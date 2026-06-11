/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

/*!
 * @file test-grep.cpp
 * @brief Grep 機能の回帰テスト
 *
 * Issue #1689 のテスト基盤と PR #2459 の拡張（マルチスレッド・除外ファイルの正規表現）を
 * 対象として、実ファイルシステムを経由した動作確認を行う。
 *
 *  1. 検索プリミティブ（文字列検索・正規表現）
 *  2. CGrepEnumKeys のキー解釈
 *  3. CCodeMediator の文字コード自動判定
 *  4. 実ファイルに対する CGrepEnumFilterFiles / CGrepEnumFilterFolders の列挙
 *  5. CGrepAgent::DoGrepFileWorker による実ファイル内検索
 *  6. PR #2459 のマルチスレッド経路（シングルスレッドとの結果一致）
 *  7. PR #2459 の除外ファイルの正規表現適用
 *  8. サブフォルダー再帰、32000 行ファイル、キャンセル伝播 等
 *
 * 一時ファイル群は RAII クラス GrepTempDir で生成・破棄を行う。
 */

#include "pch.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <mutex>
#include <random>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "agent/CGrepAgent.h"
#include "agent/CSearchAgent.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeMediator.h"
#include "extmodule/CBregexp.h"
#include "grep/CGrepEnumFileBase.h"
#include "grep/CGrepEnumFiles.h"
#include "grep/CGrepEnumFolders.h"
#include "grep/CGrepEnumFilterFiles.h"
#include "grep/CGrepEnumFilterFolders.h"
#include "grep/CGrepEnumKeys.h"
#include "io/CFileLoad.h"
#include "util/file.h"

#include "window/EditorTestSuite.hpp"

namespace {

// =============================================================================
// 一時ディレクトリ RAII
// =============================================================================

/*!
 * テスト用の一時ディレクトリを生成・自動削除する RAII クラス
 *
 *  - コンストラクタで一意な一時ディレクトリを作る
 *  - デストラクタで再帰削除する
 *  - WriteEncodedTextFile / WriteRawBytes 等のヘルパで配下にファイルを書き込む
 */
class GrepTempDir
{
public:
	explicit GrepTempDir(std::wstring_view prefix = L"grp")
	{
		// GetTempFilePath は一時ファイルを生成するので、それを消してから
		// 同名のディレクトリを掘り直すことで一意性を担保する。
		auto candidate = GetTempFilePath(prefix);
		std::filesystem::remove(candidate);
		std::filesystem::create_directories(candidate);
		m_root = candidate;
	}

	GrepTempDir(const GrepTempDir&) = delete;
	GrepTempDir& operator=(const GrepTempDir&) = delete;

	~GrepTempDir()
	{
		std::error_code ec;
		std::filesystem::remove_all(m_root, ec);
	}

	const std::filesystem::path& Root() const noexcept { return m_root; }

	std::filesystem::path Sub(std::wstring_view relative) const
	{
		return m_root / std::filesystem::path(relative);
	}

	void EnsureDir(std::wstring_view relative) const
	{
		std::filesystem::create_directories(Sub(relative));
	}

	/*!
	 * 任意の文字コードでテキストファイルを書き出す
	 *
	 * BOM 付き UTF-8 / UTF-16LE / UTF-16BE に対応するために
	 * withBom を指定可能にしている（自動判定経路のテストで使用）。
	 */
	std::filesystem::path WriteEncodedTextFile(
		std::wstring_view relative,
		ECodeType codeType,
		std::wstring_view text,
		bool withBom = false) const
	{
		const auto path = Sub(relative);
		std::filesystem::create_directories(path.parent_path());

		const auto encoded = CCodeFactory::ConvertToCode(codeType, std::wstring(text));
		if (encoded.result != RESULT_COMPLETE) {
			ADD_FAILURE() << "CCodeFactory::ConvertToCode failed for codeType="
				<< static_cast<int>(codeType);
			return path;
		}

		std::ofstream os(path, std::ios::binary | std::ios::trunc);
		if (!os) {
			ADD_FAILURE() << "failed to open file for write: " << path.string();
			return path;
		}

		if (withBom) {
			switch (codeType) {
			case CODE_UTF8:
				os.write("\xEF\xBB\xBF", 3);
				break;
			case CODE_UNICODE:
				os.write("\xFF\xFE", 2);
				break;
			case CODE_UNICODEBE:
				os.write("\xFE\xFF", 2);
				break;
			default:
				break;
			}
		}
		os.write(encoded.destination.data(),
			static_cast<std::streamsize>(encoded.destination.size()));
		return path;
	}

	std::filesystem::path WriteRawBytes(
		std::wstring_view relative, std::string_view bytes) const
	{
		const auto path = Sub(relative);
		std::filesystem::create_directories(path.parent_path());
		std::ofstream os(path, std::ios::binary | std::ios::trunc);
		os.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
		return path;
	}

private:
	std::filesystem::path m_root;
};

// =============================================================================
// 補助ヘルパ
// =============================================================================

SSearchOption MakeSearchOption(bool regex, bool caseSensitive, bool wordOnly = false)
{
	SSearchOption opt;
	opt.Reset();
	opt.bRegularExp = regex;
	opt.bLoHiCase = caseSensitive;
	opt.bWordOnly = wordOnly;
	return opt;
}

SGrepOption MakeGrepOption(ECodeType charSet = CODE_AUTODETECT)
{
	// テストで共通に使う Grep 条件をまとめ、個別ケースでは必要な項目だけ上書きする。
	SGrepOption gopt;
	gopt.nGrepCharSet = charSet;
	gopt.nGrepOutputStyle = 1; // Normal
	gopt.nGrepOutputLineType = 1; // ヒット行を出力
	gopt.bGrepHeader = false;
	return gopt;
}

/*!
 * 1 ファイルに対する Grep ワーカーを呼び出してヒット数を返す
 *
 * CGrepAgent::DoGrepFileWorker は PR #2459 で並列 Grep の単位として
 * 切り出された関数。ここで直接呼ぶことで、ファイル列挙以降の検索処理を
 * 実ファイルに対して検証する。
 *
 *  @retval -1 キャンセル
 *  @retval >=0 ヒット数
 */
int RunGrepFileWorker(
	CGrepAgent& agent,
	const std::filesystem::path& path,
	std::wstring_view key,
	const SSearchOption& sSearchOption,
	const SGrepOption& sGrepOption,
	std::atomic<bool>& cancel)
{
	const std::wstring keyStr(key);
	const std::wstring fullPath = path.wstring();
	const std::wstring fileName = path.filename().wstring();
	const std::wstring baseFolder = path.parent_path().wstring();

	SGrepFileTask task;
	task.fullPath = fullPath;
	task.fileName = fileName;
	task.baseFolder = baseFolder;
	task.folder = baseFolder;
	task.relPath = fileName;

	CBregexp regexp;
	if (sSearchOption.bRegularExp) {
		if (!InitRegexp(nullptr, regexp, false)) {
			ADD_FAILURE() << "InitRegexp failed (bregonig.dll missing?)";
			return -1;
		}
		const DWORD flags = sSearchOption.bLoHiCase
			? CBregexp::optCaseSensitive
			: 0;
		if (!regexp.Compile(keyStr.c_str(), flags)) {
			ADD_FAILURE() << "regexp.Compile failed: " << path.string();
			return -1;
		}
	}

	CSearchStringPattern pattern;
	if (!pattern.SetPattern(nullptr, keyStr.c_str(), keyStr.size(),
			sSearchOption, sSearchOption.bRegularExp ? &regexp : nullptr)) {
		ADD_FAILURE() << "SetPattern failed";
		return -1;
	}

	CNativeW cmemMessage;
	CNativeW cUnicodeBuffer;
	cmemMessage.AllocStringBuffer(4000);
	cUnicodeBuffer.AllocStringBuffer(4000);

	return agent.DoGrepFileWorker(
		task, keyStr.c_str(),
		sSearchOption, sGrepOption,
		sSearchOption.bRegularExp ? &regexp : nullptr,
		pattern,
		cmemMessage, cUnicodeBuffer,
		cancel);
}

// 同上だが、外側でキャンセル管理しない単純版
int RunGrepFileWorker(
	CGrepAgent& agent,
	const std::filesystem::path& path,
	std::wstring_view key,
	const SSearchOption& sSearchOption,
	const SGrepOption& sGrepOption)
{
	std::atomic<bool> cancel{ false };
	return RunGrepFileWorker(agent, path, key, sSearchOption, sGrepOption, cancel);
}

/*!
 * 1 行分のテキストを EOL 付きで反復した文字列を作る
 *
 * 大規模ファイルやヒット間隔を制御したテストデータを作るためのもの。
 *
 * @param lineCount 行数
 * @param hitInterval 1 以上ならその倍数行に key を挿入し、それ以外は dummy を挿入する。0 以下なら常に dummy。
 */
std::wstring BuildLineSequence(
	std::wstring_view key,
	std::wstring_view dummy,
	int lineCount,
	int hitInterval)
{
	std::wstring out;
	out.reserve(static_cast<size_t>(lineCount) * 16);
	for (int i = 1; i <= lineCount; ++i) {
		if (hitInterval > 0 && (i % hitInterval) == 0) {
			out.append(key);
		} else {
			out.append(dummy);
		}
		out.push_back(L'\n');
	}
	return out;
}

} // namespace

// =============================================================================
// テストフィクスチャ
// =============================================================================

namespace {

/*!
 * 実ファイル系 Grep テストの共通フィクスチャ
 *
 *  - SetUpTestSuite で DLLSHAREDATA を一度だけ初期化（CDocTypeManager 経由で
 *    DoGrepFileWorker が参照する）
 *  - 各テストの SetUp で一意な一時ディレクトリを掘り、TearDown で破棄
 */
struct GrepRealFileTest
	: public ::testing::Test
	, public window::EditorTestSuite
{
	static void SetUpTestSuite()
	{
		SetUpEditor();
	}

	static void TearDownTestSuite()
	{
		TearDownEditor();
	}

	void SetUp() override
	{
		m_temp = std::make_unique<GrepTempDir>();
	}

	void TearDown() override
	{
		m_temp.reset();
	}

	std::unique_ptr<GrepTempDir> m_temp;
};

} // namespace

// =============================================================================
// 1. 検索プリミティブ（in-memory）
// =============================================================================

/*!
 * @brief 大小文字区別 ON/OFF の文字列検索
 * @remark 大小文字区別なし(OFF)では小文字パターンで大文字混じりにヒットし、区別あり(ON)ではヒットしないことを確認する。
 */
TEST(GrepSearchEngine, LiteralSearchCaseSensitivity)
{
	const std::wstring line = L"Needle in a haystack";

	{
		const auto opt = MakeSearchOption(false, /*caseSensitive=*/false);
		CSearchStringPattern pattern;
		ASSERT_TRUE(pattern.SetPattern(nullptr, L"needle", 6, opt, nullptr));
		EXPECT_NE(CSearchAgent::SearchString(
			line.c_str(), static_cast<int>(line.size()), 0, pattern), nullptr);
	}
	{
		const auto opt = MakeSearchOption(false, /*caseSensitive=*/true);
		CSearchStringPattern pattern;
		ASSERT_TRUE(pattern.SetPattern(nullptr, L"needle", 6, opt, nullptr));
		EXPECT_EQ(CSearchAgent::SearchString(
			line.c_str(), static_cast<int>(line.size()), 0, pattern), nullptr);
	}
}

/*!
 * @brief 日本語検索語の複数文字コード in-memory 検索
 * @remark SJIS / JIS / EUC / UTF-8 / UTF-16(LE/BE) / UTF-7 / CESU-8 にエンコード・デコードした後でも
 *         in-memory 検索で漏れがないことを確認する。ファイル経由の検索は GrepRealFileTest 側で別途確認する。
 */
TEST_F(GrepRealFileTest, JapaneseLiteralSearchAcrossMixedEncodings)
{
	const std::wstring key = L"検索キー";
	const std::vector<ECodeType> codeTypes = {
												CODE_SJIS,
												CODE_JIS,
												CODE_EUC,
												CODE_UNICODE,
												CODE_UNICODEBE,
												CODE_UTF8,
												CODE_UTF7,
												CODE_CESU8,
											  };

	const auto opt = MakeSearchOption(false, false);
	CSearchStringPattern pattern;
	ASSERT_TRUE(pattern.SetPattern(nullptr, key.c_str(), key.size(), opt, nullptr));

	for (const auto codeType : codeTypes) {
		const auto encoded = CCodeFactory::ConvertToCode(codeType,
			std::wstring(L"先頭\n検索キー\n末尾\n"));
		ASSERT_EQ(encoded.result, RESULT_COMPLETE)
			<< "codeType=" << static_cast<int>(codeType);

		const auto decoded = CCodeFactory::LoadFromCode(codeType, encoded.destination);
		ASSERT_EQ(decoded.result, RESULT_COMPLETE)
			<< "codeType=" << static_cast<int>(codeType);

		const std::wstring& text = decoded.destination;
		EXPECT_NE(CSearchAgent::SearchString(
			text.c_str(), static_cast<int>(text.size()), 0, pattern), nullptr)
			<< "codeType=" << static_cast<int>(codeType);
	}
}

/*!
 * @brief 正規表現エンジンの基本動作
 * @remark コンパイル・マッチ・大小文字オプションが正しく機能することを確認する。
 */
TEST_F(GrepRealFileTest, RegexCompileAndMatch)
{
	CBregexp regexp;
	ASSERT_TRUE(InitRegexp(nullptr, regexp, false));
	ASSERT_TRUE(regexp.Compile(L"^needle$", CBregexp::optCaseSensitive));
	EXPECT_TRUE(regexp.Match(L"needle", 6, 0));								// 一致する
	EXPECT_FALSE(regexp.Match(L"Needle", 6, 0));							// 大文字小文字区別で不一致
}

// =============================================================================
// 2. CGrepEnumKeys 解析
// =============================================================================

/*!
 * @brief `!` プレフィックスを除外正規表現として保存 (PR #2459)
 * @remark `!` で始まるパターンが除外正規表現リストに格納され、GetExcludeFiles にも反映されることを確認する。
 */
TEST(CGrepEnumKeys, ParseRegexExcludePattern)
{
	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.txt;!.*skip.*\\.txt$"));

	ASSERT_EQ(keys.m_vecExceptFileRegexPatterns.size(), 1u);
	EXPECT_EQ(keys.m_vecExceptFileRegexPatterns[0], L".*skip.*\\.txt$");	// ! を除いた文字列で格納

	const auto excludeFiles = keys.GetExcludeFiles();
	ASSERT_EQ(excludeFiles.size(), 1u);
	EXPECT_EQ(excludeFiles[0], L".*skip.*\\.txt$");							// GetExcludeFiles にも同値が反映
}

/*!
 * @brief ファイルパターンと除外フォルダーの同時解析
 * @remark 検索対象ファイルパターンと除外フォルダー指定が正しく振り分けられ、
 *         フォルダー未指定時は *.* を補完することを確認する。
 */
TEST(CGrepEnumKeys, ParseFileAndFolderKeysWithDefaults)
{
	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.cpp;#build"));

	ASSERT_EQ(keys.m_vecSearchFileKeys.size(), 1u);
	EXPECT_STREQ(keys.m_vecSearchFileKeys[0], L"*.cpp");		// *.cpp が検索対象ファイルに振り分けられる

	const auto excludeFolders = keys.GetExcludeFolders();
	ASSERT_EQ(excludeFolders.size(), 1u);
	EXPECT_STREQ(excludeFolders[0], L"build");					// # を除いたフォルダー名で除外に振り分けられる

	ASSERT_EQ(keys.m_vecSearchFolderKeys.size(), 1u);
	EXPECT_STREQ(keys.m_vecSearchFolderKeys[0], L"*.*");		// フォルダー未指定時は *.* を補完
}

/*!
 * @brief 絶対パス混在の検索キーはエラー
 * @remark 検索パターンに絶対パスが含まれる場合、SetFileKeys が戻り値 2 を返して拒否することを確認する。
 */
TEST(CGrepEnumKeys, AbsolutePathInSearchKeyIsRejected)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(2, keys.SetFileKeys(L"C:\\foo\\*.cpp"));
}

// =============================================================================
// 3. CCodeMediator 自動判定
// =============================================================================

/*!
 * @brief BOM 検出と空バッファ時の既定コードフォールバック
 * @remark 空バッファは既定コードを返し、BOM 付きの UTF-8 / UTF-16LE / UTF-16BE は
 *         それぞれ正しい文字コードを返すことを確認する。
 */
TEST(CCodeMediator, AutoDetectBomAndDefaultCode)
{
	SEncodingConfig encodingConfig{};
	encodingConfig.m_bPriorCesu8 = false;
	encodingConfig.m_eDefaultCodetype = CODE_UTF8;

	CCodeMediator mediator(encodingConfig);

	EXPECT_EQ(mediator.CheckKanjiCode(nullptr, 0), CODE_UTF8);									// 空入力は既定コード

	const auto utf8Body = CCodeFactory::ConvertToCode(CODE_UTF8, std::wstring(L"検索キー"));
	ASSERT_EQ(utf8Body.result, RESULT_COMPLETE);
	const std::string utf8Bom = std::string("\xEF\xBB\xBF", 3) + utf8Body.destination;
	EXPECT_EQ(mediator.CheckKanjiCode(utf8Bom.data(), utf8Bom.size()), CODE_UTF8);				// UTF-8 BOM を検出

	const auto utf16leBody = CCodeFactory::ConvertToCode(CODE_UNICODE, std::wstring(L"検索キー"));
	ASSERT_EQ(utf16leBody.result, RESULT_COMPLETE);
	const std::string utf16leBom = std::string("\xFF\xFE", 2) + utf16leBody.destination;
	EXPECT_EQ(mediator.CheckKanjiCode(utf16leBom.data(), utf16leBom.size()), CODE_UNICODE);		// UTF-16LE BOM を検出

	const auto utf16beBody = CCodeFactory::ConvertToCode(CODE_UNICODEBE, std::wstring(L"検索キー"));
	ASSERT_EQ(utf16beBody.result, RESULT_COMPLETE);
	const std::string utf16beBom = std::string("\xFE\xFF", 2) + utf16beBody.destination;
	EXPECT_EQ(mediator.CheckKanjiCode(utf16beBom.data(), utf16beBom.size()), CODE_UNICODEBE);	// UTF-16BE BOM を検出
}

// =============================================================================
// 4. 実ファイルに対する CGrepEnumFilterFiles / Folders の列挙
// =============================================================================

/*!
 * @brief 拡張子フィルターと除外キーによるファイル列挙
 * @remark `*.cpp` 指定で .cpp ファイルのみが列挙され、AddExceptFile で追加した .bak が除外されることを確認する。
 */
TEST_F(GrepRealFileTest, EnumerateFiles_FiltersByExtensionAndExcludeKey)
{
	m_temp->WriteEncodedTextFile(L"main.cpp", CODE_UTF8, L"int main(){}\n");
	m_temp->WriteEncodedTextFile(L"util.cpp", CODE_UTF8, L"void util(){}\n");
	m_temp->WriteEncodedTextFile(L"readme.md", CODE_UTF8, L"# readme\n");
	m_temp->WriteEncodedTextFile(L"main.bak", CODE_UTF8, L"old\n");

	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.cpp"));
	ASSERT_EQ(0, keys.AddExceptFile(L"*.bak"));

	CGrepEnumFilterFiles enumFiles;
	CGrepEnumOptions enumOpts;
	CGrepEnumFiles exceptAbs;
	enumFiles.Enumerates(m_temp->Root().wstring().c_str(), keys, enumOpts, exceptAbs);

	std::vector<std::wstring> found;
	for (int i = 0; i < enumFiles.GetCount(); ++i) {
		found.emplace_back(enumFiles.GetFileName(i));
	}
	std::sort(found.begin(), found.end());

	ASSERT_EQ(found.size(), 2u);
	EXPECT_EQ(found[0], L"main.cpp");
	EXPECT_EQ(found[1], L"util.cpp");
}

/*!
 * @brief `#` プレフィックスによるサブフォルダー除外
 * @remark `#excluded` や `#node_modules` で指定したフォルダーが列挙結果から除外され、
 *         指定外のフォルダーのみ残ることを確認する。
 */
TEST_F(GrepRealFileTest, EnumerateFolders_ExcludesByHashKey)
{
	m_temp->EnsureDir(L"keep");
	m_temp->EnsureDir(L"excluded");
	m_temp->EnsureDir(L"node_modules");

	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.cpp;#excluded;#node_modules"));

	CGrepEnumFilterFolders enumFolders;
	CGrepEnumOptions enumOpts;
	CGrepEnumFolders exceptAbs;
	enumFolders.Enumerates(m_temp->Root().wstring().c_str(), keys, enumOpts, exceptAbs);

	std::vector<std::wstring> found;
	for (int i = 0; i < enumFolders.GetCount(); ++i) {
		found.emplace_back(enumFolders.GetFileName(i));
	}
	std::sort(found.begin(), found.end());

	ASSERT_EQ(found.size(), 1u);
	EXPECT_EQ(found[0], L"keep");
}

/*!
 * @brief サブフォルダー再帰列挙
 * @remark サブフォルダーを再帰的に検索し、下位フォルダーのファイルも列挙されることを確認する。
 *         列挙順序はファイルシステム依存のため、順序ではなく存在のみを検証する。
 */
TEST_F(GrepRealFileTest, EnumerateFiles_SubfolderRecursion)
{
	m_temp->WriteEncodedTextFile(L"root.txt", CODE_UTF8, L"a\n");
	m_temp->WriteEncodedTextFile(L"sub1/child.txt", CODE_UTF8, L"b\n");
	m_temp->WriteEncodedTextFile(L"sub1/sub2/grandchild.txt", CODE_UTF8, L"c\n");

	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.txt"));

	std::vector<std::wstring> visited;

	auto walk = [&](auto& self, const std::filesystem::path& folder) -> void {
		CGrepEnumFilterFiles enumFiles;
		CGrepEnumOptions enumOpts;
		CGrepEnumFiles exceptAbs;
		enumFiles.Enumerates(folder.wstring().c_str(), keys, enumOpts, exceptAbs);
		for (int i = 0; i < enumFiles.GetCount(); ++i) {
			visited.emplace_back((folder / enumFiles.GetFileName(i)).wstring());
		}

		CGrepEnumFilterFolders enumFolders;
		CGrepEnumFolders exceptAbsFolders;
		enumFolders.Enumerates(folder.wstring().c_str(), keys, enumOpts, exceptAbsFolders);
		for (int i = 0; i < enumFolders.GetCount(); ++i) {
			self(self, folder / enumFolders.GetFileName(i));
		}
	};
	walk(walk, m_temp->Root());

	ASSERT_EQ(visited.size(), 3u);
	// ファイルシステムの列挙順序は保証されないため、順序に依存しない検証をする
	bool foundGrandchild = false, foundChild = false, foundRoot = false;
	for (const auto& v : visited) {
		if (v.find(L"grandchild.txt") != std::wstring::npos) foundGrandchild = true;
		if (v.find(L"child.txt")      != std::wstring::npos) foundChild      = true;
		if (v.find(L"root.txt")       != std::wstring::npos) foundRoot       = true;
	}
	EXPECT_TRUE(foundGrandchild);
	EXPECT_TRUE(foundChild);
	EXPECT_TRUE(foundRoot);
}

// =============================================================================
// 5. CGrepAgent::DoGrepFileWorker による実ファイル内検索
// =============================================================================

/*!
 * @brief 単一 UTF-8 ファイルへの基本ヒット数
 * @remark 50 行のファイルに 10 行ごと key を埋め込み、DoGrepFileWorker が 5 件を返すことを確認する。
 */
TEST_F(GrepRealFileTest, FileWorker_BasicLiteralHitsInUtf8File)
{
	const auto path = m_temp->WriteEncodedTextFile(L"basic.txt", CODE_UTF8,
		BuildLineSequence(L"needle", L"abc", 50, 10));

	CGrepAgent agent;
	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();

	EXPECT_EQ(5, RunGrepFileWorker(agent, path, L"needle", sOpt, gOpt));
}

/*!
 * @brief 大小文字オプションの実ファイル経路での反映
 * @remark 大小文字区別なしで 3 件、区別ありで 1 件ヒットすることを実ファイル経由で確認する。
 */
TEST_F(GrepRealFileTest, FileWorker_CaseSensitivityIsRespected)
{
	const auto path = m_temp->WriteEncodedTextFile(L"case.txt", CODE_UTF8,
		std::wstring(L"Needle\nneedle\nNEEDLE\n"));

	CGrepAgent agent;
	const auto gOpt = MakeGrepOption();

	EXPECT_EQ(3, RunGrepFileWorker(agent, path, L"needle",
				MakeSearchOption(false, /*caseSensitive=*/false), gOpt));	// 大小文字区別なし: 3 件
	EXPECT_EQ(1, RunGrepFileWorker(agent, path, L"needle",
				MakeSearchOption(false, /*caseSensitive=*/true), gOpt));	// 大小文字区別あり: 1 件
}

/*!
 * @brief 正規表現検索の実ファイル経路での動作
 * @remark `^id=\d+$` パターンで数字行のみ 3 件ヒットすることを実ファイル経由で確認する。
 */
TEST_F(GrepRealFileTest, FileWorker_RegexHitsInUtf8File)
{
	const auto path = m_temp->WriteEncodedTextFile(L"regex.txt", CODE_UTF8,
		std::wstring(L"id=001\nid=042\nname=alice\nid=999\n"));

	CGrepAgent agent;
	const auto sOpt = MakeSearchOption(/*regex=*/true, /*caseSensitive=*/true);
	const auto gOpt = MakeGrepOption();

	EXPECT_EQ(3, RunGrepFileWorker(agent, path, L"^id=\\d+$", sOpt, gOpt));
}

/*!
 * @brief 日本語検索の複数エンコード対応
 * @remark SJIS / BOM 付き UTF-8 / BOM 付き UTF-16LE のファイルを自動判定経由で検索し、
 *          いずれも 2 件を返すことを確認する。
 */
TEST_F(GrepRealFileTest, FileWorker_JapaneseAcrossEncodings)
{
	const std::wstring body = L"先頭\n検索キー\n中間\n検索キー\n末尾\n";

	struct Entry { const wchar_t* name; ECodeType code; bool bom; };
	const std::vector<Entry> entries = {
											{ L"jp_sjis.txt",  CODE_SJIS,      false },
											{ L"jp_utf8.txt",  CODE_UTF8,      true },
											{ L"jp_utf16.txt", CODE_UNICODE,   true },
										};

	CGrepAgent agent;
	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();

	for (const auto& e : entries) {
		const auto path = m_temp->WriteEncodedTextFile(e.name, e.code, body, e.bom);
		EXPECT_EQ(2, RunGrepFileWorker(agent, path, L"検索キー", sOpt, gOpt))
			<< "encoding=" << static_cast<int>(e.code);
	}
}

/*!
 * @brief 32000 行ファイルの全件走査
 * @remark 1000 行ごとに 1 件 key を埋め込んだ 32000 行ファイルを走査し、
 *         ヒット数が 32 件で壊れないことを確認する。
 */
TEST_F(GrepRealFileTest, FileWorker_32000LinesIsSearchable)
{
	const auto path = m_temp->WriteEncodedTextFile(L"huge.txt", CODE_UTF8,
		BuildLineSequence(L"needle", L"abc", 32000, 1000));

	CGrepAgent agent;
	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();

	EXPECT_EQ(32, RunGrepFileWorker(agent, path, L"needle", sOpt, gOpt));
}

/*!
 * @brief atomic キャンセルフラグによる走査の中断
 * @remark 50000 行ファイルの走査開始直後にキャンセルフラグを立て、ワーカーが
 *         -1 または全件未満で終了することを確認する。
 *         32 行ごとのチェックポイントで即時抜けることを期待する。
 */
TEST_F(GrepRealFileTest, FileWorker_CancelTerminatesScan)
{
	const auto path = m_temp->WriteEncodedTextFile(L"long.txt", CODE_UTF8,
		BuildLineSequence(L"needle", L"abc", 50000, 1000));

	CGrepAgent agent;
	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();

	std::atomic<bool> cancel{ false };
	std::promise<int> resultPromise;
	auto resultFuture = resultPromise.get_future();

	std::thread worker([&]() {
		resultPromise.set_value(
			RunGrepFileWorker(agent, path, L"needle", sOpt, gOpt, cancel));
	});

	// ワーカーがファイルを開いて走り出すための猶予
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	cancel.store(true, std::memory_order_release);

	// wait_for の結果を変数に受けてから join し、ASSERT でテスト中断しても
	// スレッドがダングリング参照を踏まないようにする。
	const bool ready = resultFuture.wait_for(std::chrono::seconds(5))
		== std::future_status::ready;
	worker.join();
	ASSERT_TRUE(ready) << "worker did not finish within 5s";
	const int hits = resultFuture.get();

	// -1 (キャンセル) もしくは中途半端なヒット数で終わる。
	// いずれにせよ、全件 (50) が出ないこと、もしくは -1 であることを確認する。
	EXPECT_TRUE(hits == -1 || (hits >= 0 && hits < 50))
		<< "hits=" << hits << " (50 means cancel was not honored)";
}

// =============================================================================
// 6. PR #2459 マルチスレッド経路の検証
// =============================================================================

namespace {

/*!
 * ファイル群をワーカー数 nThreads で並列走査して合計ヒット数を返す
 *
 * DoGrepFileWorker を直接呼ぶテスト専用ヘルパ。RunParallelGrep とは独立した実装で、
 * production のスレッドプール経路（RunParallelGrep 本体）はカバーしない。
 * production 本体の検証は RunParallelGrep_* テストを参照。
 */
int RunWorkersParallel(
	const std::vector<std::filesystem::path>& files,
	std::wstring_view key,
	const SSearchOption& sSearchOption,
	const SGrepOption& sGrepOption,
	unsigned int nThreads)
{
	std::atomic<int> total{ 0 };
	std::atomic<size_t> next{ 0 };
	std::atomic<bool> cancel{ false };

	CGrepAgent agent; // ワーカー本体はステートレスにふるまうので共有してよい

	auto worker = [&]() {
		while (!cancel.load(std::memory_order_acquire)) {
			const size_t idx = next.fetch_add(1, std::memory_order_acq_rel);
			if (idx >= files.size()) break;
			const int hits = RunGrepFileWorker(
				agent, files[idx], key, sSearchOption, sGrepOption, cancel);
			if (hits < 0) {
				cancel.store(true, std::memory_order_release);
				break;
			}
			total.fetch_add(hits, std::memory_order_relaxed);
		}
	};

	std::vector<std::thread> workers;
	workers.reserve(nThreads);
	for (unsigned int i = 0; i < nThreads; ++i) {
		workers.emplace_back(worker);
	}
	for (auto& t : workers) t.join();
	return total.load();
}

} // namespace

/*!
 * @brief マルチスレッドとシングルスレッドの合計ヒット数一致 (PR #2459)
 * @remark スレッド数 2 / 4 / 8 の並列走査が、シングルスレッドと同じ合計ヒット数を返すことを確認する。
 */
TEST_F(GrepRealFileTest, MultiThread_HitCountMatchesSingleThread)
{
	std::vector<std::filesystem::path> files;
	for (int i = 0; i < 12; ++i) {
		const std::wstring name = L"file_" + std::to_wstring(i) + L".txt";
		files.push_back(m_temp->WriteEncodedTextFile(name, CODE_UTF8,
			BuildLineSequence(L"needle", L"abc", 100, 10)));
	}

	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();

	const int singleThreaded = RunWorkersParallel(files, L"needle", sOpt, gOpt, 1);
	EXPECT_EQ(singleThreaded, 12 * 10);							// 100 行に 10 件 × 12 ファイル

	for (unsigned int n : { 2u, 4u, 8u }) {
		const int parallel = RunWorkersParallel(files, L"needle", sOpt, gOpt, n);
		EXPECT_EQ(parallel, singleThreaded) << "threads=" << n;
	}
}

/*!
 * @brief 並列走査の繰り返しによるデッドロック・件数ぶれ検証 (PR #2459)
 * @remark 4 スレッドで同一入力を 5 回繰り返し、デッドロックが発生せず件数が毎回一致することを確認する。
 */
TEST_F(GrepRealFileTest, MultiThread_StressNoDeadlockAcrossRepeats)
{
	std::vector<std::filesystem::path> files;
	for (int i = 0; i < 8; ++i) {
		const std::wstring name = L"stress_" + std::to_wstring(i) + L".txt";
		files.push_back(m_temp->WriteEncodedTextFile(name, CODE_UTF8,
			BuildLineSequence(L"needle", L"abc", 200, 20)));
	}

	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();
	const int expected = 8 * (200 / 20);

	for (int round = 0; round < 5; ++round) {
		const int hits = RunWorkersParallel(files, L"needle", sOpt, gOpt, 4);
		EXPECT_EQ(hits, expected) << "round=" << round;
	}
}

// =============================================================================
// 7. PR #2459 除外正規表現の適用
// =============================================================================

/*!
 * @brief 除外正規表現の列挙結果への適用 (PR #2459)
 * @remark `!.*\.obj$` / `!.*\.exe$` を SetFileKeys に与え、production と同じ経路（CBregexp.Compile → Match）で
 *         列挙結果をフィルタリングした結果、.cpp ファイルのみが残ることを確認する。
 */
TEST_F(GrepRealFileTest, RegexExclude_AppliedToEnumeratedFiles)
{
	m_temp->WriteEncodedTextFile(L"main.cpp", CODE_UTF8, L"int main(){}\n");
	m_temp->WriteEncodedTextFile(L"util.cpp", CODE_UTF8, L"void util(){}\n");
	m_temp->WriteEncodedTextFile(L"main.obj", CODE_UTF8, L"binary-ish\n");
	m_temp->WriteEncodedTextFile(L"app.exe",  CODE_UTF8, L"binary-ish\n");

	CGrepEnumKeys keys;
	// 探索対象は *.* だが、.obj と .exe は正規表現で除外
	ASSERT_EQ(0, keys.SetFileKeys(L"*.*;!.*\\.obj$;!.*\\.exe$"));
	ASSERT_EQ(keys.m_vecExceptFileRegexPatterns.size(), 2u);

	// production と同じく、テストドライバが各パターンを CBregexp でコンパイル
	std::vector<CBregexp> excludeRegexps(keys.m_vecExceptFileRegexPatterns.size());
	for (size_t i = 0; i < keys.m_vecExceptFileRegexPatterns.size(); ++i) {
		ASSERT_TRUE(InitRegexp(nullptr, excludeRegexps[i], false));
		ASSERT_TRUE(excludeRegexps[i].Compile(
			keys.m_vecExceptFileRegexPatterns[i].c_str(),
			CBregexp::optCaseSensitive));
	}

	// 列挙
	CGrepEnumFilterFiles enumFiles;
	CGrepEnumOptions enumOpts;
	CGrepEnumFiles exceptAbs;
	enumFiles.Enumerates(m_temp->Root().wstring().c_str(), keys, enumOpts, exceptAbs);

	// 列挙結果に対して production と同じく除外正規表現を当てる
	std::vector<std::wstring> survivors;
	for (int i = 0; i < enumFiles.GetCount(); ++i) {
		const std::wstring fullPath = (m_temp->Root() / enumFiles.GetFileName(i)).wstring();
		bool excluded = false;
		for (auto& re : excludeRegexps) {
			if (re.Match(fullPath.c_str(), static_cast<int>(fullPath.size()), 0)) {
				excluded = true;
				break;
			}
		}
		if (!excluded) {
			survivors.emplace_back(enumFiles.GetFileName(i));
		}
	}
	std::sort(survivors.begin(), survivors.end());

	ASSERT_EQ(survivors.size(), 2u);
	EXPECT_EQ(survivors[0], L"main.cpp");
	EXPECT_EQ(survivors[1], L"util.cpp");
}

/*!
 * @brief 不正な除外正規表現はコンパイル失敗
 * @remark 不正な構文の除外正規表現パターンは CBregexp::Compile が false を返し、
 *         production での上流バリデーション（DoGrep 側）で弾けることを確認する。
 */
TEST_F(GrepRealFileTest, RegexExclude_InvalidPatternFailsToCompile)
{
	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.cpp;!*invalid("));
	ASSERT_EQ(keys.m_vecExceptFileRegexPatterns.size(), 1u);

	CBregexp regexp;
	ASSERT_TRUE(InitRegexp(nullptr, regexp, false));
	EXPECT_FALSE(regexp.Compile(
		keys.m_vecExceptFileRegexPatterns[0].c_str(),
		CBregexp::optCaseSensitive));
}

// =============================================================================
// Phase 2-B: CGrepEnumKeys coverage for SplitPattern / AddExcept* (PR #2459)
// =============================================================================

// ----- CGrepEnumKeys::SplitPattern 区切り 3 種 -----

/*!
 * @brief 文字列分割(SplitPattern)の仕様：セミコロンによる区切り
 * @remark セミコロン(;)で区切った場合、正しく複数の要素に分割することを確認する。
 */
TEST(CGrepEnumKeys, SplitPattern_SemicolonSeparator)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.cpp;*.h");
	EXPECT_EQ(2, keys.m_vecSearchFileKeys.size());
	if (keys.m_vecSearchFileKeys.size() == 2) {
		EXPECT_STREQ(L"*.cpp", keys.m_vecSearchFileKeys[0]);
		EXPECT_STREQ(L"*.h", keys.m_vecSearchFileKeys[1]);
	}
}

/*!
 * @brief 文字列分割(SplitPattern)の仕様：スペースによる区切り
 * @remark スペースで区切った場合も、正しく複数の要素に分割することを確認する。
 */
TEST(CGrepEnumKeys, SplitPattern_SpaceSeparator)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.cpp *.h");
	EXPECT_EQ(2, keys.m_vecSearchFileKeys.size());
}

/*!
 * @brief 文字列分割(SplitPattern)の仕様：カンマによる区切り
 * @remark カンマ(,)で区切った場合も、正しく複数の要素に分割することを確認する。
 */
TEST(CGrepEnumKeys, SplitPattern_CommaSeparator)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.cpp,*.h");
	EXPECT_EQ(2, keys.m_vecSearchFileKeys.size());
}

/*!
 * @brief 文字列分割(SplitPattern)の仕様：複数種類の区切り文字の混在
 * @remark セミコロン、スペース、カンマが混在している場合でも、すべての区切り文字が有効に機能することを確認する。
 */
TEST(CGrepEnumKeys, SplitPattern_MixedSeparators)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.cpp;*.h *.txt,*.md");
	EXPECT_EQ(4, keys.m_vecSearchFileKeys.size());
}

/*!
 * @brief 文字列分割(SplitPattern)の仕様：連続する区切り文字
 * @remark 区切り文字が連続している場合、空の要素を生成せず無視する（my_strtokの仕様）ことを確認する。
 */
TEST(CGrepEnumKeys, SplitPattern_ConsecutiveSeparators)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.cpp;;;*.h");
	EXPECT_EQ(2, keys.m_vecSearchFileKeys.size());
}

/*!
 * @brief 文字列分割(SplitPattern)の仕様：末尾の区切り文字
 * @remark 文字列の末尾に区切り文字がある場合でも、余分な空要素を生成しないことを確認する。
 */
TEST(CGrepEnumKeys, SplitPattern_TrailingSeparator)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.cpp;");
	EXPECT_EQ(1, keys.m_vecSearchFileKeys.size());
}

/*!
 * @brief 文字列分割(SplitPattern)の仕様：ダブルクォートの除去
 * @remark 要素がダブルクォートで囲まれている場合、分割後にダブルクォートが取り除かれることを確認する。
 */
TEST(CGrepEnumKeys, SplitPattern_QuotesRemoved)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"\"*.cpp\";\"*.h\"");
	EXPECT_EQ(2, keys.m_vecSearchFileKeys.size());
	if (keys.m_vecSearchFileKeys.size() == 2) {
		EXPECT_STREQ(L"*.cpp", keys.m_vecSearchFileKeys[0]);
		EXPECT_STREQ(L"*.h", keys.m_vecSearchFileKeys[1]);
	}
}

/*!
 * @brief 文字列分割(SplitPattern)の仕様：要素途中のダブルクォート
 * @remark 要素の途中にダブルクォートが含まれている場合、文字列から
 *         そのダブルクォートを除去して連結することを確認する。
 */
TEST(CGrepEnumKeys, SplitPattern_QuotesInMiddle)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"a\"b\"c");
	EXPECT_EQ(1, keys.m_vecSearchFileKeys.size());
	if (keys.m_vecSearchFileKeys.size() == 1) {
		EXPECT_STREQ(L"abc", keys.m_vecSearchFileKeys[0]);
	}
}

// ----- CGrepEnumKeys::SetFileKeys 戻り値・分岐 -----

/*!
 * @brief 検索対象キー設定(SetFileKeys)の仕様：空入力のフォールバック
 * @remark 空文字列が入力された場合、デフォルトの "*.*" が検索対象・除外対象の双方に設定され、
 *         戻り値が0になることを確認する。
 */
TEST(CGrepEnumKeys, SetFileKeys_EmptyInputFallsBackToWildcard)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L""));						// 受理する
	EXPECT_EQ(1, keys.m_vecSearchFileKeys.size());				// *.* が 1 件補完される
	if (keys.m_vecSearchFileKeys.size() == 1) {
		EXPECT_STREQ(L"*.*", keys.m_vecSearchFileKeys[0]);
	}
}

/*!
 * @brief 検索対象キー設定(SetFileKeys)の仕様：パス中のワイルドカード
 * @remark パス文字列内にワイルドカードが含まれる場合、戻り値が1になることを確認する。
 */
TEST(CGrepEnumKeys, SetFileKeys_WildcardInPathReturns1)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(1, keys.SetFileKeys(L"*\\file.exe"));
}

/*!
 * @brief 検索対象キー設定(SetFileKeys)の仕様：絶対パス指定
 * @remark 検索対象として絶対パスが指定された場合、戻り値が2になることを確認する。
 */
TEST(CGrepEnumKeys, SetFileKeys_AbsolutePathInSearchReturns2)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(2, keys.SetFileKeys(L"C:\\foo\\*.cpp"));
}

/*!
 * @brief 検索対象キー設定(SetFileKeys)の仕様：除外フォルダー（相対パス）
 * @remark `#` から始まる相対パスが、除外フォルダーのリスト（m_vecExceptFolderKeys）に正しく追加され、
 *         戻り値が0になることを確認する。
 */
TEST(CGrepEnumKeys, SetFileKeys_ExcludeFolderHashRelative)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L"#build"));					// 受理する
	EXPECT_EQ(1, keys.m_vecExceptFolderKeys.size());			// 相対除外フォルダーに 1 件追加
	if (keys.m_vecExceptFolderKeys.size() == 1) {
		EXPECT_STREQ(L"build", keys.m_vecExceptFolderKeys[0]);
	}
}

/*!
 * @brief 検索対象キー設定(SetFileKeys)の仕様：除外フォルダー（絶対パス）
 * @remark `#` から始まる絶対パスが、絶対パス用の除外フォルダーリスト（m_vecExceptAbsFolderKeys）に追加され、
 *         戻り値が0になることを確認する。
 */
TEST(CGrepEnumKeys, SetFileKeys_ExcludeFolderHashAbsolute)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L"#C:\\build"));				// 受理する
	EXPECT_EQ(1, keys.m_vecExceptAbsFolderKeys.size());			// 絶対除外フォルダーに 1 件追加
	if (keys.m_vecExceptAbsFolderKeys.size() == 1) {
		EXPECT_STREQ(L"C:\\build", keys.m_vecExceptAbsFolderKeys[0]);
	}
}

/*!
 * @brief 検索対象キー設定(SetFileKeys)の仕様：正規表現の事前バリデーションスキップ
 * @remark `!` から始まる正規表現パターンは `ValidateKey` の事前チェックをスキップするため、
 *         不正な文法でも戻り値0（受理）になることを確認する。
 */
TEST(CGrepEnumKeys, SetFileKeys_RegexExcludeNoValidation)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L"!*invalid\\("));
}

/*!
 * @brief 検索対象キー設定(SetFileKeys)の仕様：重複キーの排除
 * @remark まったく同じ検索対象キーが複数指定された場合、内部で重複が排除(Deduplication)され、
 *         1つにまとめられることを確認する。
 */
TEST(CGrepEnumKeys, SetFileKeys_DuplicateKeyDeduplicated)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.cpp;*.cpp;*.cpp");
	EXPECT_EQ(1, keys.m_vecSearchFileKeys.size());
}

// ----- AddExceptFile / AddExceptFolder -----

/*!
 * @brief 個別除外指定(AddExceptFile)の仕様：相対パスの複数追加
 * @remark AddExceptFileを使用して相対パスを渡した際、区切り文字で分割して
 *         除外ファイルリスト（相対）に追加することを確認する。
 */
TEST(CGrepEnumKeys, AddExceptFile_RelativePath)
{
	CGrepEnumKeys keys;
	keys.AddExceptFile(L"*.obj;*.tmp");
	EXPECT_EQ(2, keys.m_vecExceptFileKeys.size());				// セミコロンで 2 件に分割して相対リストに追加
}

/*!
 * @brief 個別除外指定(AddExceptFile)の仕様：絶対パスの追加
 * @remark 絶対パスが渡された場合は、絶対パス用の除外ファイルリスト（m_vecExceptAbsFileKeys）に追加することを確認する。
 */
TEST(CGrepEnumKeys, AddExceptFile_AbsolutePath)
{
	CGrepEnumKeys keys;
	keys.AddExceptFile(L"C:\\tmp\\foo.obj");
	EXPECT_EQ(1, keys.m_vecExceptAbsFileKeys.size());
	if (keys.m_vecExceptAbsFileKeys.size() == 1) {
		EXPECT_STREQ(L"C:\\tmp\\foo.obj", keys.m_vecExceptAbsFileKeys[0]);
	}
}

/*!
 * @brief デフォルト除外フォルダの追加(AddExceptFolder)の仕様
 * @remark デフォルトで用意されている複数の相対除外パターン（.git, .svn, .vs等）を一括で正しく追加することを確認する。
 */
TEST(CGrepEnumKeys, AddExceptFolder_DefaultPattern)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.AddExceptFolder(L".git;.svn;.vs"));		// 追加を受け付ける
	EXPECT_EQ(3, keys.m_vecExceptFolderKeys.size());			// 相対フォルダーが 3 件
	EXPECT_EQ(0, keys.m_vecExceptAbsFolderKeys.size());			// 絶対側は増えない
}

/*!
 * @brief 除外ファイルの追加(AddExceptFile)の仕様：#記号の扱い
 * @remark AddExceptFileでは `#` が特別な意味（フォルダー指定）を持たず、通常の文字列としてファイル名に処理することを確認する。
 */
TEST(CGrepEnumKeys, AddExceptFile_HashIsNotSpecial)
{
	CGrepEnumKeys keys;
	keys.AddExceptFile(L"#foo");
	EXPECT_EQ(1, keys.m_vecExceptFileKeys.size());
	if (keys.m_vecExceptFileKeys.size() == 1) {
		EXPECT_STREQ(L"#foo", keys.m_vecExceptFileKeys[0]);
	}
}

// ----- GetExcludeFiles / GetExcludeFolders 集約 -----

/*!
 * @brief 除外ファイルの集約取得(GetExcludeFiles)の仕様
 * @remark SetFileKeys と AddExceptFile の両方で登録された除外ファイルが
 *         GetExcludeFiles で集約されることを確認する。
 *         集約対象は m_vecExceptFileKeys, m_vecExceptAbsFileKeys,
 *         m_vecExceptFileRegexPatterns の 3 配列。
 *         ただし実装によっては一部配列が含まれない場合がある（件数は実装に合わせて凍結）。
 */
TEST(CGrepEnumKeys, GetExcludeFiles_MergesAllThreeArrays)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.cpp;!.*\\.obj$");
	keys.AddExceptFile(L"*.tmp;C:\\foo.bak");
	auto files = keys.GetExcludeFiles();
	EXPECT_EQ(3, files.size());	// 正規表現・相対・絶対の 3 系統を集約した合計
}

/*!
 * @brief 除外フォルダーの集約取得(GetExcludeFolders)の仕様
 * @remark SetFileKeysで設定された相対パス(`#`)および絶対パス(`#C:\...`)の除外フォルダーを
 *         正しく1つに集約することを確認する。
 */
TEST(CGrepEnumKeys, GetExcludeFolders_MergesRelAndAbs)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.cpp;#build;#C:\\sys");
	auto folders = keys.GetExcludeFolders();
	EXPECT_EQ(2, folders.size());	// 相対 1 件 + 絶対 1 件の合計
}

// =============================================================================
// 9. RunParallelGrep 経路（UI非依存でテスト可能）
// =============================================================================

/*!
 * @brief RunParallelGrep が pcDlgCancel/pcViewDst=nullptr で動作し、ヒット数が正しい (PR #2459)
 * @remark UI 非依存（nullptr 渡し）でスレッドプール経路を走らせ、直列経路と同じ件数になることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_HitCountMatchesSingleThread)
{
	for (int i = 0; i < 8; ++i) {
		m_temp->WriteEncodedTextFile(
			L"rp_" + std::to_wstring(i) + L".txt",
			CODE_UTF8,
			BuildLineSequence(L"needle", L"abc", 100, 10));
	}

	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();
	const std::wstring rootPath = m_temp->Root().wstring();

	CGrepAgent agent;
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	CGrepEnumFiles cGrepExceptAbsFiles;
	CGrepEnumFolders cGrepExceptAbsFolders;
	CNativeW cmemMessage;
	int nHitCount = 0;

	const std::vector<std::wstring> vPaths = { rootPath };
	const int result = agent.RunParallelGrep(
		nullptr, nullptr,
		L"needle", sOpt, gOpt, vPaths,
		keys, cGrepExceptAbsFiles, cGrepExceptAbsFolders,
		cmemMessage, nHitCount);

	EXPECT_EQ(0, result);           // 0 = 完了（-1 はキャンセル）
	EXPECT_EQ(8 * 10, nHitCount);   // 8 ファイル × 10 件
}

/*!
 * @brief RunParallelGrep がサブフォルダーを再帰して合計ヒット数を返す (1-1: サブフォルダー分岐経路)
 * @remark ルート + sub1/ + sub1/sub2/ に配置したファイルを bGrepSubFolder=true で検索し、
 *         RunParallelGrep のバッチ1..N（cTopFolders 列挙 → サブフォルダーバッチ）経路を踏む。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_SubfolderRecursion)
{
	// ルート直下: 50行・10行おきにヒット → 5件
	m_temp->WriteEncodedTextFile(L"root.txt", CODE_UTF8,
		BuildLineSequence(L"needle", L"abc", 50, 10));
	// sub1/: 30行・10行おきにヒット → 3件
	m_temp->WriteEncodedTextFile(L"sub1/s1.txt", CODE_UTF8,
		BuildLineSequence(L"needle", L"abc", 30, 10));
	// sub1/sub2/: 20行・10行おきにヒット → 2件
	m_temp->WriteEncodedTextFile(L"sub1/sub2/s2.txt", CODE_UTF8,
		BuildLineSequence(L"needle", L"abc", 20, 10));

	const auto sOpt = MakeSearchOption(false, false);
	auto gOpt = MakeGrepOption();
	gOpt.bGrepSubFolder = true;

	CGrepAgent agent;
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	CGrepEnumFiles cExAbsFiles;
	CGrepEnumFolders cExAbsFolders;
	CNativeW cmemMessage;
	int nHit = 0;

	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };
	const int rc = agent.RunParallelGrep(
		nullptr, nullptr,
		L"needle", sOpt, gOpt, vPaths,
		keys, cExAbsFiles, cExAbsFolders,
		cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(10, nHit);    // 5 + 3 + 2
}

/*!
 * @brief RunParallelGrep のワーカー内除外正規表現が適用される (1-2: excludeRegexps Compile/Match 経路)
 * @remark .cpp/.obj/.exe の4ファイルを配置し、.obj/.exe を除外正規表現で除外した結果
 *         .cpp 2ファイル分のヒットのみが返ることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_ExcludeRegexFiltersWorker)
{
	// 4ファイルに各 2 件の needle を含む
	m_temp->WriteEncodedTextFile(L"main.cpp",  CODE_UTF8, L"needle\nneedle\n");
	m_temp->WriteEncodedTextFile(L"util.cpp",  CODE_UTF8, L"needle\nneedle\n");
	m_temp->WriteEncodedTextFile(L"main.obj",  CODE_UTF8, L"needle\nneedle\n");
	m_temp->WriteEncodedTextFile(L"app.exe",   CODE_UTF8, L"needle\nneedle\n");

	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();

	CGrepAgent agent;
	CGrepEnumKeys keys;
	// *.*;!.*\.obj$;!.*\.exe$ → .obj/.exe を除外
	keys.SetFileKeys(L"*.*;!.*\\.obj$;!.*\\.exe$");
	CGrepEnumFiles cExAbsFiles;
	CGrepEnumFolders cExAbsFolders;
	CNativeW cmemMessage;
	int nHit = 0;

	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };
	const int rc = agent.RunParallelGrep(
		nullptr, nullptr,
		L"needle", sOpt, gOpt, vPaths,
		keys, cExAbsFiles, cExAbsFolders,
		cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(4, nHit);     // .cpp 2 ファイル × 2 件（.obj/.exe は除外）
}

/*!
 * @brief RunParallelGrep のベースフォルダーヘッダーが重複しない (2: ヘッダー出力一致検証)
 * @remark bGrepOutputBaseFolder=true, bGrepSeparateFolder=false で複数ファイルを検索したとき、
 *         ベースフォルダーヘッダー行（■"..."）が出力に 1 回だけ現れることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_BaseFolderHeader_NoDuplicate)
{
	m_temp->WriteEncodedTextFile(L"a.txt", CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"b.txt", CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"c.txt", CODE_UTF8, L"needle\n");

	const auto sOpt = MakeSearchOption(false, false);
	auto gOpt = MakeGrepOption();
	gOpt.bGrepOutputBaseFolder = true;
	gOpt.bGrepSeparateFolder   = false;

	CGrepAgent agent;
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	CGrepEnumFiles cExAbsFiles;
	CGrepEnumFolders cExAbsFolders;
	CNativeW cmemMessage;
	int nHit = 0;

	const std::wstring rootPath = m_temp->Root().wstring();
	const std::vector<std::wstring> vPaths = { rootPath };
	const int rc = agent.RunParallelGrep(
		nullptr, nullptr,
		L"needle", sOpt, gOpt, vPaths,
		keys, cExAbsFiles, cExAbsFolders,
		cmemMessage, nHit);

	ASSERT_EQ(0, rc);
	EXPECT_EQ(3, nHit);

	// ベースフォルダーヘッダー（■"<root>"）は 1 回だけ出力されること
	const std::wstring output(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());
	const std::wstring baseFolderHeader = L"■\"" + rootPath + L"\"";
	int headerCount = 0;
	size_t pos = 0;
	while ((pos = output.find(baseFolderHeader, pos)) != std::wstring::npos) {
		++headerCount;
		pos += baseFolderHeader.size();
	}
	EXPECT_EQ(1, headerCount) << "ベースフォルダーヘッダーが重複して出力されている";
}

/*!
 * @brief RunParallelGrep のベース/フォルダーヘッダーが重複しない (2: bGrepSeparateFolder=true)
 * @remark bGrepOutputBaseFolder=true, bGrepSeparateFolder=true で複数ファイルを検索したとき、
 *         ◎"..."（ベース）と ■\r\n（ルート直下フォルダー）が各 1 回だけ出ることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_SeparateFolderHeader_NoDuplicate)
{
	m_temp->WriteEncodedTextFile(L"x.txt", CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"y.txt", CODE_UTF8, L"needle\n");

	const auto sOpt = MakeSearchOption(false, false);
	auto gOpt = MakeGrepOption();
	gOpt.bGrepOutputBaseFolder = true;
	gOpt.bGrepSeparateFolder   = true;

	CGrepAgent agent;
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	CGrepEnumFiles cExAbsFiles;
	CGrepEnumFolders cExAbsFolders;
	CNativeW cmemMessage;
	int nHit = 0;

	const std::wstring rootPath = m_temp->Root().wstring();
	const std::vector<std::wstring> vPaths = { rootPath };
	const int rc = agent.RunParallelGrep(
		nullptr, nullptr,
		L"needle", sOpt, gOpt, vPaths,
		keys, cExAbsFiles, cExAbsFolders,
		cmemMessage, nHit);

	ASSERT_EQ(0, rc);
	EXPECT_EQ(2, nHit);

	const std::wstring output(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());

	// ◎"<root>" は 1 回だけ（ベースフォルダーヘッダー、bGrepSeparateFolder=true 時）
	const std::wstring baseHeader = L"◎\"" + rootPath + L"\"";
	int baseCount = 0;
	size_t pos = 0;
	while ((pos = output.find(baseHeader, pos)) != std::wstring::npos) {
		++baseCount;
		pos += baseHeader.size();
	}
	EXPECT_EQ(1, baseCount) << "ベースフォルダーヘッダー（◎）が重複して出力されている";

	// ■\r\n（ルート直下 = 空フォルダー名）は 1 回だけ
	const std::wstring folderHeader = L"■\r\n";
	int folderCount = 0;
	pos = 0;
	while ((pos = output.find(folderHeader, pos)) != std::wstring::npos) {
		++folderCount;
		pos += folderHeader.size();
	}
	EXPECT_EQ(1, folderCount) << "フォルダーヘッダー（■\\r\\n）が重複して出力されている";
}
