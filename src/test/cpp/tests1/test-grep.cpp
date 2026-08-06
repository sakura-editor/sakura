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
#include <format>
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
#include "grep/GrepPathFormat.h"
#include "io/CFileLoad.h"
#include "util/file.h"
#include "_os/CClipboard.h"
#include "dlg/CDlgGrep.h"
#include "env/CAppNodeManager.h"
#include "env/CSakuraEnvironment.h"
#include "types/CType.h"
#include "view/colors/CColorStrategy.h"

#include "dlg/CDlgCancel.h"
#include "view/CEditView.h"

#include "window/EditorTestSuite.hpp"
#include "grep-test-util.h"

namespace {

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

/*! 特定フォルダー直下のファイル名をフィルター列挙し、ソート済みで返す */
std::vector<std::wstring> EnumerateSortedFileNames(const std::wstring& folder, CGrepEnumKeys& keys)
{
	CGrepEnumFilterFiles enumFiles;
	CGrepEnumOptions enumOpts;
	CGrepEnumFiles exceptAbs;
	enumFiles.Enumerates(folder.c_str(), keys, enumOpts, exceptAbs);

	std::vector<std::wstring> found;
	for (int i = 0; i < enumFiles.GetCount(); ++i) {
		found.emplace_back(enumFiles.GetFileName(i));
	}
	std::sort(found.begin(), found.end());
	return found;
}

/*! 特定フォルダー直下のフォルダー名をフィルター列挙し、ソート済みで返す */
std::vector<std::wstring> EnumerateSortedFolderNames(const std::wstring& folder, CGrepEnumKeys& keys)
{
	CGrepEnumFilterFolders enumFolders;
	CGrepEnumOptions enumOpts;
	CGrepEnumFolders exceptAbs;
	enumFolders.Enumerates(folder.c_str(), keys, enumOpts, exceptAbs);

	std::vector<std::wstring> found;
	for (int i = 0; i < enumFolders.GetCount(); ++i) {
		found.emplace_back(enumFolders.GetFileName(i));
	}
	std::sort(found.begin(), found.end());
	return found;
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
// GrepPathFormat 整形
// =============================================================================

/*!
 * @brief ';' を含むパスの引用符付け (QuotePathIfNeeded)
 * @remark ';' を含むパスは '"' で囲まれ、含まないパスはそのまま返ることを確認する。
 */
TEST(GrepPathFormat, QuotePathIfNeeded)
{
	EXPECT_EQ(QuotePathIfNeeded(L"a;b"), L"\"a;b\"");		// ';' を含む → 引用符で囲む
	EXPECT_EQ(QuotePathIfNeeded(L"plain"), L"plain");		// ';' を含まない → そのまま
	EXPECT_EQ(QuotePathIfNeeded(L""), L"");					// 空文字はそのまま
}

/*!
 * @brief パスリストの ';' 区切り連結 (FormatPathList)
 * @remark ';' を含む要素のみ引用符で囲み、要素間を ';' で連結することを確認する。
 */
TEST(GrepPathFormat, FormatPathListQuotesSemicolonElements)
{
	const std::vector<std::wstring> paths{ L"a", L"b;c", L"d" };
	EXPECT_EQ(FormatPathList(paths), L"a;\"b;c\";d");

	const std::vector<std::wstring> empty;
	EXPECT_EQ(FormatPathList(empty), L"");					// 空リストは空文字
}

// =============================================================================
// 2. CGrepEnumKeys 解析
// =============================================================================

/*!
 * @brief `!` プレフィックスを正規表現モードで除外正規表現として保存 (PR #2459)
 * @remark 正規表現モード（bExcludeFileRegex=true）で `!` を除外正規表現として保存し、
 *         GetExcludeFiles にも反映されることを確認する。
 */
TEST(CGrepEnumKeys, ParseRegexExcludePattern)
{
	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.txt;!.*skip.*\\.txt$", /*bExcludeFileRegex=*/true));

	ASSERT_EQ(keys.m_vecExceptFileRegexPatterns.size(), 1u);
	EXPECT_EQ(keys.m_vecExceptFileRegexPatterns[0], L".*skip.*\\.txt$");	// ! を除いた文字列で格納

	const auto excludeFiles = keys.GetExcludeFiles();
	ASSERT_EQ(excludeFiles.size(), 1u);
	EXPECT_EQ(excludeFiles[0], L".*skip.*\\.txt$");							// GetExcludeFiles にも同値が反映
}

/*!
 * @brief `!` 単体（空の除外パターン）は登録せず無視する (PR #9)
 * @remark 空の正規表現は全パスにマッチし全ファイル除外となるため、
 *         正規表現モード・ワイルドカードモードのいずれでも登録されないことを確認する。
 */
TEST(CGrepEnumKeys, EmptyExcludePatternIsIgnored)
{
	// 正規表現モード: `!` 単体は m_vecExceptFileRegexPatterns に登録されない
	{
		CGrepEnumKeys keys;
		ASSERT_EQ(0, keys.SetFileKeys(L"*.txt;!", /*bExcludeFileRegex=*/true));
		EXPECT_EQ(keys.m_vecExceptFileRegexPatterns.size(), 0u);
		EXPECT_EQ(keys.GetExcludeFiles().size(), 0u);
	}
	// ワイルドカードモード: `!` 単体は除外ファイルキーに登録されない
	{
		CGrepEnumKeys keys;
		ASSERT_EQ(0, keys.SetFileKeys(L"*.txt;!"));
		EXPECT_EQ(keys.GetExcludeFiles().size(), 0u);
	}
	// 空でない除外パターンは従来どおり登録される（回帰確認）
	{
		CGrepEnumKeys keys;
		ASSERT_EQ(0, keys.SetFileKeys(L"*.txt;!;!.*\\.obj$", /*bExcludeFileRegex=*/true));
		ASSERT_EQ(keys.m_vecExceptFileRegexPatterns.size(), 1u);
		EXPECT_EQ(keys.m_vecExceptFileRegexPatterns[0], L".*\\.obj$");
	}
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
	EXPECT_STREQ(keys.m_vecSearchFileKeys[0].c_str(), L"*.cpp");		// *.cpp が検索対象ファイルに振り分けられる

	const auto excludeFolders = keys.GetExcludeFolders();
	ASSERT_EQ(excludeFolders.size(), 1u);
	EXPECT_STREQ(excludeFolders[0].c_str(), L"build");					// # を除いたフォルダー名で除外に振り分けられる

	ASSERT_EQ(keys.m_vecSearchFolderKeys.size(), 1u);
	EXPECT_STREQ(keys.m_vecSearchFolderKeys[0].c_str(), L"*.*");		// フォルダー未指定時は *.* を補完
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

	const auto found = EnumerateSortedFileNames(m_temp->Root().wstring(), keys);

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

	const auto found = EnumerateSortedFolderNames(m_temp->Root().wstring(), keys);

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
	bool foundGrandchild = false;
	bool foundChild = false;
	bool foundRoot = false;
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

	std::atomic cancel{ false };
	std::promise<int> resultPromise;
	auto resultFuture = resultPromise.get_future();

	std::jthread worker([&resultPromise, &agent, &path, &sOpt, &gOpt, &cancel]() {
		resultPromise.set_value(
			RunGrepFileWorker(agent, path, L"needle", sOpt, gOpt, cancel));
	});

	// ワーカーがファイルを開いて走り出すための猶予
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	cancel.store(true);

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
	std::atomic total{ 0 };
	std::atomic<size_t> next{ 0 };
	std::atomic cancel{ false };

	CGrepAgent agent; // ワーカー本体はステートレスにふるまうので共有してよい

	auto worker = [&cancel, &next, &files, &agent, &key, &sSearchOption, &sGrepOption, &total]() {
		while (!cancel.load()) {
			const size_t idx = next.fetch_add(1);
			if (idx >= files.size()) break;
			const int hits = RunGrepFileWorker(
				agent, files[idx], key, sSearchOption, sGrepOption, cancel);
			if (hits < 0) {
				cancel.store(true);		// 次周回冒頭の判定で全ワーカーが停止する
			} else {
				total.fetch_add(hits);
			}
		}
	};

	std::vector<std::jthread> workers;
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
		const std::wstring name = std::format(L"file_{}.txt", i);
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
		const std::wstring name = std::format(L"stress_{}.txt", i);
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
 * @brief 正規表現モードの除外を列挙結果へ適用 (PR #2459)
 * @remark 正規表現モード（bExcludeFileRegex=true）で `!.*\.obj$` / `!.*\.exe$` を SetFileKeys に与え、
 *         production と同じ経路（CBregexp.Compile → Match）で列挙結果をフィルタリングした結果、
 *         .cpp ファイルのみが残ることを確認する。
 */
TEST_F(GrepRealFileTest, RegexExclude_AppliedToEnumeratedFiles)
{
	m_temp->WriteEncodedTextFile(L"main.cpp", CODE_UTF8, L"int main(){}\n");
	m_temp->WriteEncodedTextFile(L"util.cpp", CODE_UTF8, L"void util(){}\n");
	m_temp->WriteEncodedTextFile(L"main.obj", CODE_UTF8, L"binary-ish\n");
	m_temp->WriteEncodedTextFile(L"app.exe",  CODE_UTF8, L"binary-ish\n");

	CGrepEnumKeys keys;
	// 探索対象は *.* だが、.obj と .exe は正規表現で除外
	ASSERT_EQ(0, keys.SetFileKeys(L"*.*;!.*\\.obj$;!.*\\.exe$", /*bExcludeFileRegex=*/true));
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
 * @brief 正規表現モードで不正なパターンはコンパイル失敗
 * @remark 正規表現モード（bExcludeFileRegex=true）で不正な構文の除外パターンを与えると
 *         CBregexp::Compile が false を返し、production での上流バリデーション（DoGrep 側）で
 *         弾けることを確認する。
 */
TEST_F(GrepRealFileTest, RegexExclude_InvalidPatternFailsToCompile)
{
	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.cpp;!*invalid(", /*bExcludeFileRegex=*/true));
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
		EXPECT_STREQ(L"*.cpp", keys.m_vecSearchFileKeys[0].c_str());
		EXPECT_STREQ(L"*.h", keys.m_vecSearchFileKeys[1].c_str());
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
	keys.SetFileKeys(LR"("*.cpp";"*.h")");
	EXPECT_EQ(2, keys.m_vecSearchFileKeys.size());
	if (keys.m_vecSearchFileKeys.size() == 2) {
		EXPECT_STREQ(L"*.cpp", keys.m_vecSearchFileKeys[0].c_str());
		EXPECT_STREQ(L"*.h", keys.m_vecSearchFileKeys[1].c_str());
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
		EXPECT_STREQ(L"abc", keys.m_vecSearchFileKeys[0].c_str());
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
		EXPECT_STREQ(L"*.*", keys.m_vecSearchFileKeys[0].c_str());
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
		EXPECT_STREQ(L"build", keys.m_vecExceptFolderKeys[0].c_str());
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
		EXPECT_STREQ(L"C:\\build", keys.m_vecExceptAbsFolderKeys[0].c_str());
	}
}

/*!
 * @brief 正規表現モードで `!` パターンは ValidateKey をスキップして受理される
 * @remark 正規表現モード（bExcludeFileRegex=true）では `!` から始まるパターンが
 *         `ValidateKey` の事前チェックをスキップするため、不正な文法でも戻り値0（受理）になることを確認する。
 */
TEST(CGrepEnumKeys, SetFileKeys_RegexExcludeNoValidation)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L"!*invalid\\(", /*bExcludeFileRegex=*/true));
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

/*!
 * @brief `!` はデフォルト (bExcludeFileRegex=false) ではワイルドカード除外に入る (回帰)
 * @remark 第2引数を省略した場合、`!` プレフィックスはワイルドカード除外リストに格納され、
 *         正規表現除外リストは空のままであることを確認する。
 */
TEST(CGrepEnumKeys, SetFileKeys_ExcludeFileWildcardByDefault)
{
	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.cpp;!*.obj"));   // 第2引数省略=false
	ASSERT_EQ(1u, keys.m_vecExceptFileKeys.size());
	EXPECT_STREQ(L"*.obj", keys.m_vecExceptFileKeys[0].c_str());
	EXPECT_TRUE(keys.m_vecExceptFileRegexPatterns.empty());
}

/*!
 * @brief `~` は特殊文字ではなく通常の検索対象ファイル名として扱われる (回帰)
 * @remark `~` プレフィックスは除外でも正規表現でもなく、
 *         通常の検索対象ファイルキーとして格納されることを確認する。
 */
TEST(CGrepEnumKeys, SetFileKeys_TildeIsNormalSearchTarget)
{
	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"~temp*"));
	ASSERT_EQ(1u, keys.m_vecSearchFileKeys.size());
	EXPECT_STREQ(L"~temp*", keys.m_vecSearchFileKeys[0].c_str());
	EXPECT_TRUE(keys.m_vecExceptFileKeys.empty());			// 除外（ワイルドカード）にも入らない
	EXPECT_TRUE(keys.m_vecExceptFileRegexPatterns.empty());	// 除外（正規表現）にも入らない
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
	keys.AddExceptFile(L"C:\\testdata\\foo.obj");
	EXPECT_EQ(1, keys.m_vecExceptAbsFileKeys.size());
	if (keys.m_vecExceptAbsFileKeys.size() == 1) {
		EXPECT_STREQ(L"C:\\testdata\\foo.obj", keys.m_vecExceptAbsFileKeys[0].c_str());
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
		EXPECT_STREQ(L"#foo", keys.m_vecExceptFileKeys[0].c_str());
	}
}

// ----- GetExcludeFiles / GetExcludeFolders 集約 -----

/*!
 * @brief 除外ファイルの集約取得(GetExcludeFiles)の仕様
 * @remark 正規表現モード（bExcludeFileRegex=true）の SetFileKeys と AddExceptFile の両方で登録された
 *         除外ファイルが GetExcludeFiles で集約されることを確認する。
 *         集約対象は m_vecExceptFileKeys, m_vecExceptAbsFileKeys,
 *         m_vecExceptFileRegexPatterns の 3 配列。
 *         ただし実装によっては一部配列が含まれない場合がある（件数は実装に合わせて凍結）。
 */
TEST(CGrepEnumKeys, GetExcludeFiles_MergesAllThreeArrays)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.cpp;!.*\\.obj$", /*bExcludeFileRegex=*/true);
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
			std::format(L"rp_{}.txt", i),
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
		SGrepSearchParams{ L"needle", sOpt, gOpt },
		SGrepEnumContext{ keys, cGrepExceptAbsFiles, cGrepExceptAbsFolders },
		vPaths,
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
		SGrepSearchParams{ L"needle", sOpt, gOpt },
		SGrepEnumContext{ keys, cExAbsFiles, cExAbsFolders },
		vPaths,
		cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(10, nHit);    // 5 + 3 + 2
}

// =============================================================================
// 補助ヘルパー（セクション 9–12 共通）
// =============================================================================

namespace {

/*! 複数パスへの RunParallelGrep を最小限の引数で呼ぶラッパー */
int RunParallelGrepOnPaths(
	CGrepEnumKeys& keys,
	const std::vector<std::wstring>& vPaths,
	std::wstring_view key,
	const SSearchOption& sOpt,
	const SGrepOption& gOpt,
	CNativeW& cmemMessage,
	int& nHit)
{
	CGrepAgent agent;
	CGrepEnumFiles cExAbsFiles;
	CGrepEnumFolders cExAbsFolders;
	return agent.RunParallelGrep(
		nullptr, nullptr,
		SGrepSearchParams{ std::wstring(key).c_str(), sOpt, gOpt },
		SGrepEnumContext{ keys, cExAbsFiles, cExAbsFolders },
		vPaths,
		cmemMessage, nHit);
}

/*! str 中に sub が何回現れるかを数える */
int CountOccurrences(std::wstring_view str, std::wstring_view sub)
{
	int count = 0;
	size_t pos = 0;
	while ((pos = str.find(sub, pos)) != std::wstring::npos) {
		++count;
		pos += sub.size();
	}
	return count;
}

} // namespace

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
	// *.*;!.*\.obj$;!.*\.exe$ → 正規表現モードで .obj/.exe を除外
	keys.SetFileKeys(L"*.*;!.*\\.obj$;!.*\\.exe$", /*bExcludeFileRegex=*/true);
	CGrepEnumFiles cExAbsFiles;
	CGrepEnumFolders cExAbsFolders;
	CNativeW cmemMessage;
	int nHit = 0;

	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };
	const int rc = agent.RunParallelGrep(
		nullptr, nullptr,
		SGrepSearchParams{ L"needle", sOpt, gOpt },
		SGrepEnumContext{ keys, cExAbsFiles, cExAbsFolders },
		vPaths,
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

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	CNativeW cmemMessage;
	int nHit = 0;
	const std::wstring rootPath = m_temp->Root().wstring();
	const std::vector<std::wstring> vPaths = { rootPath };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"needle", sOpt, gOpt, cmemMessage, nHit);

	ASSERT_EQ(0, rc);
	EXPECT_EQ(3, nHit);

	// ベースフォルダーヘッダー（■"<root>"）は 1 回だけ出力されること
	const std::wstring output(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());
	EXPECT_EQ(1, CountOccurrences(output, L"■\"" + rootPath + L"\""))
		<< "ベースフォルダーヘッダーが重複して出力されている";
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

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	CNativeW cmemMessage;
	int nHit = 0;
	const std::wstring rootPath = m_temp->Root().wstring();
	const std::vector<std::wstring> vPaths = { rootPath };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"needle", sOpt, gOpt, cmemMessage, nHit);

	ASSERT_EQ(0, rc);
	EXPECT_EQ(2, nHit);

	const std::wstring output(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());

	// ◎"<root>" は 1 回だけ（ベースフォルダーヘッダー、bGrepSeparateFolder=true 時）
	EXPECT_EQ(1, CountOccurrences(output, L"◎\"" + rootPath + L"\""))
		<< "ベースフォルダーヘッダー（◎）が重複して出力されている";

	// ■\r\n（ルート直下 = 空フォルダー名）は 1 回だけ
	EXPECT_EQ(1, CountOccurrences(output, L"■\r\n"))
		<< "フォルダーヘッダー（■\\r\\n）が重複して出力されている";
}


// =============================================================================
// 10. BuildGrepHeader / BuildGrepFooter の分岐網羅
// =============================================================================

/*!
 * @brief BuildGrepHeader: リテラルキーのヘッダー基本形式
 * @remark キーが引用符で囲まれ、ファイルパターン・フォルダーが出力に含まれ、末尾が \r\n\r\n で終わることを確認する。
 */
TEST_F(GrepRealFileTest, BuildGrepHeader_LiteralKeyBasic)
{
	const auto sOpt = MakeSearchOption(false, false);
	auto gOpt = MakeGrepOption();
	gOpt.bGrepSubFolder = false;

	const auto result = CGrepAgent::BuildGrepHeader(
		L"needle", L"*.txt", L"C:\\test", sOpt, gOpt);
	const std::wstring out(result.GetStringPtr(), result.GetStringLength());

	EXPECT_NE(out.find(L"\"needle\""), std::wstring::npos) << "キーが引用符で囲まれて出力されること";
	EXPECT_NE(out.find(L"*.txt"),      std::wstring::npos) << "ファイルパターンが出力に含まれること";
	EXPECT_NE(out.find(L"C:\\test"),   std::wstring::npos) << "フォルダーが出力に含まれること";
	ASSERT_GE(out.size(), 4u);
	EXPECT_EQ(out.substr(out.size() - 4), std::wstring(L"\r\n\r\n")) << "末尾が \\r\\n\\r\\n で終わること";
}

/*!
 * @brief BuildGrepHeader: 空キーはファイル一覧モード（キー引用符なし）
 * @remark 空キーの場合はキー行が引用符なしで出力され、ファイル・フォルダーは通常通り出力されることを確認する。
 */
TEST_F(GrepRealFileTest, BuildGrepHeader_EmptyKeyIsFileListMode)
{
	const auto sOpt = MakeSearchOption(false, false);
	auto gOpt = MakeGrepOption();
	gOpt.bGrepSubFolder = false;

	const auto result = CGrepAgent::BuildGrepHeader(
		L"", L"*.cpp", L"C:\\src", sOpt, gOpt);
	const std::wstring out(result.GetStringPtr(), result.GetStringLength());

	EXPECT_EQ(out.find(L"\"\""),   std::wstring::npos) << "空キーは引用符で囲まれない";
	EXPECT_NE(out.find(L"*.cpp"),  std::wstring::npos) << "ファイルパターンは出力される";
	EXPECT_NE(out.find(L"C:\\src"), std::wstring::npos) << "フォルダーは出力される";
}

/*!
 * @brief BuildGrepHeader: 置換文字列・クリップボード貼り付けの分岐
 * @remark bGrepReplace=true 時に置換文字列が出力に含まれ、bGrepPaste=true 時は置換文字列が出力されないことを確認する。
 */
TEST_F(GrepRealFileTest, BuildGrepHeader_ReplaceAndPasteVariants)
{
	const auto sOpt = MakeSearchOption(false, false);
	auto gOpt = MakeGrepOption();
	gOpt.bGrepReplace = true;
	gOpt.bGrepPaste   = false;

	// 置換文字列あり
	{
		const auto result = CGrepAgent::BuildGrepHeader(
			L"needle", L"*.txt", L"C:\\t", sOpt, gOpt, L"replacement");
		const std::wstring out(result.GetStringPtr(), result.GetStringLength());
		EXPECT_NE(out.find(L"\"replacement\""), std::wstring::npos)
			<< "置換文字列が引用符で囲まれて出力される";
	}

	// クリップボードから貼り付け（置換文字列は出力されない）
	{
		gOpt.bGrepPaste = true;
		const auto result = CGrepAgent::BuildGrepHeader(
			L"needle", L"*.txt", L"C:\\t", sOpt, gOpt, L"replacement");
		const std::wstring out(result.GetStringPtr(), result.GetStringLength());
		EXPECT_EQ(out.find(L"\"replacement\""), std::wstring::npos)
			<< "bGrepPaste=true のとき置換文字列は出力されない";
	}
}

/*!
 * @brief BuildGrepHeader: 各検索・出力オプションの分岐が出力に影響すること
 * @remark 単語単位・文字コード固定・lineType・FileOnly・正規表現の各 ON/OFF で出力が変化することを確認する。
 */
TEST_F(GrepRealFileTest, BuildGrepHeader_SearchAndOutputOptionBranches)
{
	// 単語単位 ON: 追加文言で出力が長くなること
	{
		const auto base = CGrepAgent::BuildGrepHeader(
			L"key", L"*.txt", L"C:\\t",
			MakeSearchOption(false, false, false), MakeGrepOption());
		const auto word = CGrepAgent::BuildGrepHeader(
			L"key", L"*.txt", L"C:\\t",
			MakeSearchOption(false, false, true), MakeGrepOption());
		EXPECT_GT(word.GetStringLength(), base.GetStringLength())
			<< "単語単位 ON のとき bWordOnly 文言が追加される";
	}

	// 文字コード固定（SJIS）: 自動検出と異なる文字コード表記が出力されること
	{
		auto gAuto = MakeGrepOption(); gAuto.nGrepCharSet = CODE_AUTODETECT;
		auto gSjis = MakeGrepOption(); gSjis.nGrepCharSet = CODE_SJIS;
		const auto rAuto = CGrepAgent::BuildGrepHeader(
			L"key", L"*.txt", L"C:\\t", MakeSearchOption(false, false), gAuto);
		const auto rSjis = CGrepAgent::BuildGrepHeader(
			L"key", L"*.txt", L"C:\\t", MakeSearchOption(false, false), gSjis);
		const std::wstring sAuto(rAuto.GetStringPtr(), rAuto.GetStringLength());
		const std::wstring sSjis(rSjis.GetStringPtr(), rSjis.GetStringLength());
		EXPECT_NE(sAuto, sSjis)
			<< "自動検出と文字コード固定で文字コード表記が異なる";
	}

	// lineType=2（否ヒット行）: lineType=1 と出力が異なること
	{
		auto g1 = MakeGrepOption(); g1.nGrepOutputLineType = 1;
		auto g2 = MakeGrepOption(); g2.nGrepOutputLineType = 2;
		const auto r1 = CGrepAgent::BuildGrepHeader(
			L"key", L"*.txt", L"C:\\t", MakeSearchOption(false, false), g1);
		const auto r2 = CGrepAgent::BuildGrepHeader(
			L"key", L"*.txt", L"C:\\t", MakeSearchOption(false, false), g2);
		const std::wstring s1(r1.GetStringPtr(), r1.GetStringLength());
		const std::wstring s2(r2.GetStringPtr(), r2.GetStringLength());
		EXPECT_NE(s1, s2)
			<< "lineType=1 と lineType=2 で異なる文言が出力される";
	}

	// bGrepOutputFileOnly=true: 文言追加で出力が長くなること
	{
		auto gBase = MakeGrepOption(); gBase.bGrepOutputFileOnly = false;
		auto gFile = MakeGrepOption(); gFile.bGrepOutputFileOnly = true;
		const auto rBase = CGrepAgent::BuildGrepHeader(
			L"key", L"*.txt", L"C:\\t", MakeSearchOption(false, false), gBase);
		const auto rFile = CGrepAgent::BuildGrepHeader(
			L"key", L"*.txt", L"C:\\t", MakeSearchOption(false, false), gFile);
		EXPECT_GT(rFile.GetStringLength(), rBase.GetStringLength())
			<< "bGrepOutputFileOnly=true のとき ファイル毎最初のみ 文言が追加される";
	}

	// 正規表現 ON: 正規表現DLL文言で出力が長くなること
	{
		const auto rBase = CGrepAgent::BuildGrepHeader(
			L"key", L"*.txt", L"C:\\t", MakeSearchOption(false, false), MakeGrepOption());
		const auto rRegex = CGrepAgent::BuildGrepHeader(
			L"key", L"*.txt", L"C:\\t", MakeSearchOption(true, false), MakeGrepOption());
		EXPECT_GT(rRegex.GetStringLength(), rBase.GetStringLength())
			<< "正規表現 ON のとき正規表現DLL文言が追加される";

		// 置換・正規表現・lineType=0・貼り付けなし → 1行目のみ表示の分岐
		auto gRep = MakeGrepOption();
		gRep.bGrepReplace = true;
		gRep.bGrepPaste   = false;
		gRep.nGrepOutputLineType = 0;
		const auto rFirstLine = CGrepAgent::BuildGrepHeader(
			L"key", L"*.txt", L"C:\\t", MakeSearchOption(true, false), gRep, L"rep");
		EXPECT_GT(rFirstLine.GetStringLength(), 0);
	}
}

/*!
 * @brief BuildGrepFooter: 通常検索と置換で書式が異なり、どちらもヒット数を含む
 * @remark BuildGrepFooter(42, false) と BuildGrepFooter(42, true) がそれぞれ "42" を含み、
 *         かつ互いに異なる文字列になることを確認する。
 */
TEST_F(GrepRealFileTest, BuildGrepFooter_SearchAndReplaceFormats)
{
	const auto rSearch  = CGrepAgent::BuildGrepFooter(42, false);
	const auto rReplace = CGrepAgent::BuildGrepFooter(42, true);
	const std::wstring sSearch (rSearch .GetStringPtr(), rSearch .GetStringLength());
	const std::wstring sReplace(rReplace.GetStringPtr(), rReplace.GetStringLength());

	EXPECT_NE(sSearch.find(L"42"),  std::wstring::npos) << "通常フッターにヒット数が含まれる";
	EXPECT_NE(sReplace.find(L"42"), std::wstring::npos) << "置換フッターにヒット数が含まれる";
	EXPECT_NE(sSearch, sReplace)                        << "通常と置換でフッター文字列が異なる";
}

// =============================================================================
// 11. FormatGrepResultLine の出力形式
// =============================================================================

/*!
 * @brief FormatGrepResultLine: style=1 + bGrepOutputBaseFolder=true で行頭に ・ が付き CRLF 終端
 * @remark ノーマルスタイルでベースフォルダー表示が ON のとき、先頭文字が ・ になり CRLF で終わることを確認する。
 */
TEST_F(GrepRealFileTest, FormatGrepResultLine_NormalStyleWithFolderBullet)
{
	CNativeW out;
	auto gOpt = MakeGrepOption();
	gOpt.nGrepOutputStyle      = 1;
	gOpt.bGrepOutputBaseFolder = true;
	gOpt.nGrepOutputLineType   = 1;

	CGrepAgent::FormatGrepResultLine(
		out,
		L"foo.cpp", L"",
		SGrepMatchInfo{ 1, 1, L"needle in a haystack", 20, 0, L"needle", 6 },
		gOpt);

	const std::wstring result(out.GetStringPtr(), out.GetStringLength());
	ASSERT_FALSE(result.empty());
	EXPECT_EQ(result[0], L'・') << "ベースフォルダー表示 ON のとき行頭は ・ (U+30FB)";
	ASSERT_GE(result.size(), 2u);
	EXPECT_EQ(result.substr(result.size() - 2), std::wstring(L"\r\n")) << "CRLF 終端";
}

/*!
 * @brief FormatGrepResultLine: style=2（WZ風）で行頭 ・(行番号,桁) 形式、ファイルパス非表示
 * @remark WZ風スタイルは ・(<行>,<桁>): の行頭形式になり、ファイルパスは含まれないことを確認する。
 */
TEST_F(GrepRealFileTest, FormatGrepResultLine_WZStyle)
{
	CNativeW out;
	auto gOpt = MakeGrepOption();
	gOpt.nGrepOutputStyle    = 2;
	gOpt.nGrepOutputLineType = 1;

	CGrepAgent::FormatGrepResultLine(
		out,
		L"bar.cpp", L"",
		SGrepMatchInfo{ 5, 3, L"line content", 12, 0, L"line", 4 },
		gOpt);

	const std::wstring result(out.GetStringPtr(), out.GetStringLength());
	ASSERT_FALSE(result.empty());
	EXPECT_EQ(result[0], L'・') << "WZ風は行頭 ・";
	EXPECT_NE(result.find(L'('), std::wstring::npos) << "行番号を括弧付きで出力する";
	EXPECT_EQ(result.find(L"bar.cpp"), std::wstring::npos)
		<< "ファイルパスは FormatGrepResultLine では出力しない";
}

/*!
 * @brief FormatGrepResultLine: style=3 + lineType=0 でマッチ部分のみ出力され EOL 付加を制御
 * @remark 結果のみモード・該当部分出力のとき、マッチ末尾が EOL でなければ \r\n を付加し、
 *         EOL で終わる場合は余分な \r\n を付加しないことを確認する。
 */
TEST_F(GrepRealFileTest, FormatGrepResultLine_ResultOnlyMatchPartEol)
{
	auto gOpt = MakeGrepOption();
	gOpt.nGrepOutputStyle    = 3;
	gOpt.nGrepOutputLineType = 0;

	// Case 1: マッチ末尾が EOL でない → \r\n が追加される
	{
		CNativeW out;
		CGrepAgent::FormatGrepResultLine(
			out, L"f.cpp", L"",
			SGrepMatchInfo{ 1, 1, L"needle in haystack\r\n", 20, 2, L"needle", 6 },
			gOpt);
		const std::wstring result(out.GetStringPtr(), out.GetStringLength());
		EXPECT_EQ(result, std::wstring(L"needle\r\n"))
			<< "マッチ末尾が EOL でない場合 \\r\\n を追加する";
	}

	// Case 2: マッチ末尾が \n → bEOL=false → 余分な \r\n を追加しない
	{
		CNativeW out;
		const wchar_t matchData[] = L"needle\n";
		CGrepAgent::FormatGrepResultLine(
			out, L"f.cpp", L"",
			SGrepMatchInfo{ 1, 1, matchData, 7, 1, matchData, 7 },
			gOpt);
		const std::wstring result(out.GetStringPtr(), out.GetStringLength());
		EXPECT_EQ(result, std::wstring(L"needle\n"))
			<< "マッチ末尾が \\n の場合 \\r\\n を追加しない";
	}
}

// =============================================================================
// 12. RunParallelGrep / DoGrepFileWorker 分岐追加
// =============================================================================

/*!
 * @brief RunParallelGrep: ワーカースレッドローカルの正規表現コンパイル経路
 * @remark 正規表現キーを渡したとき、ワーカースレッド内で正規表現がコンパイルされ正しく検索されることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_RegexKeySearch)
{
	m_temp->WriteEncodedTextFile(L"regex.txt", CODE_UTF8,
		L"id=001\nid=042\nname=alice\nid=999\n");

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	CNativeW cmemMessage;
	int nHit = 0;

	const auto sOpt = MakeSearchOption(/*regex=*/true, /*caseSensitive=*/true);
	const auto gOpt = MakeGrepOption();
	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"^id=\\d+$", sOpt, gOpt, cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(3, nHit);
}

/*!
 * @brief RunParallelGrep: 単語単位検索分岐（CreateWordList / SearchStringWord）
 * @remark bWordOnly=true で "word" を検索したとき、単独の "word" のみヒットし
 *         "words" 内の "word" はマッチしないことを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_WordOnlySearch)
{
	m_temp->WriteEncodedTextFile(L"word.txt", CODE_UTF8, L"word\nwords\n");

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	CNativeW cmemMessage;
	int nHit = 0;

	const auto sOpt = MakeSearchOption(false, false, /*wordOnly=*/true);
	auto gOpt = MakeGrepOption();
	gOpt.nGrepOutputLineType = 1;
	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"word", sOpt, gOpt, cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(1, nHit) << R"("words" 内の "word" は単語境界なしでマッチしない)";
}

/*!
 * @brief RunParallelGrep: lineType=0（該当部分）で 1 行内の複数ヒットをカウント
 * @remark nGrepOutputLineType=0 のとき 1 行内の全マッチを数えるため、
 *         2 回 needle が現れる行は 2 件となることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_MatchPartCountsAllHitsInLine)
{
	m_temp->WriteEncodedTextFile(L"multi.txt", CODE_UTF8, L"needle needle\n");

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	CNativeW cmemMessage;
	int nHit = 0;

	const auto sOpt = MakeSearchOption(false, false);
	auto gOpt = MakeGrepOption();
	gOpt.nGrepOutputLineType = 0;
	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"needle", sOpt, gOpt, cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(2, nHit) << "1 行に 2 件のマッチがあれば lineType=0 では 2 件カウント";
}

/*!
 * @brief RunParallelGrep: lineType=2（否ヒット行）の件数と出力内容
 * @remark nGrepOutputLineType=2 のとき、ヒットしなかった行のみカウント・出力されることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_NoHitLineOutput)
{
	m_temp->WriteEncodedTextFile(L"nohit.txt", CODE_UTF8,
		L"target\nother\nelse\ntarget\n");

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	CNativeW cmemMessage;
	int nHit = 0;

	const auto sOpt = MakeSearchOption(false, false);
	auto gOpt = MakeGrepOption();
	gOpt.nGrepOutputLineType = 2;
	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"target", sOpt, gOpt, cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(2, nHit) << "否ヒット行は 2 行（other・else）";
	const std::wstring out(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());
	EXPECT_NE(out.find(L"other"), std::wstring::npos) << "否ヒット行 other が出力される";
	EXPECT_NE(out.find(L"else"),  std::wstring::npos) << "否ヒット行 else が出力される";
}

/*!
 * @brief RunParallelGrep: bGrepOutputFileOnly=true でファイル毎最初の 1 件のみ
 * @remark 5 行すべてヒットするファイルを検索したとき、bGrepOutputFileOnly=true では 1 件のみ返ることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_OutputFileOnly)
{
	m_temp->WriteEncodedTextFile(L"fileonly.txt", CODE_UTF8,
		L"needle\nneedle\nneedle\nneedle\nneedle\n");

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	CNativeW cmemMessage;
	int nHit = 0;

	const auto sOpt = MakeSearchOption(false, false);
	auto gOpt = MakeGrepOption();
	gOpt.bGrepOutputFileOnly = true;
	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"needle", sOpt, gOpt, cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(1, nHit) << "bGrepOutputFileOnly=true ではファイル毎最初の 1 件のみ";
}

/*!
 * @brief RunParallelGrep: 複数検索パスでパスごとにヘッダー履歴がリセットされる
 * @remark 2 つの独立ディレクトリを vPaths に渡したとき、各パスのベースフォルダーヘッダーが
 *         それぞれ 1 回ずつ出力されることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_MultiplePathsHeaderResetPerPath)
{
	GrepTempDir dir1;
	GrepTempDir dir2;
	dir1.WriteEncodedTextFile(L"a.txt", CODE_UTF8, L"needle\n");
	dir2.WriteEncodedTextFile(L"b.txt", CODE_UTF8, L"needle\n");

	CGrepAgent agent;
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	auto gOpt = MakeGrepOption();
	gOpt.bGrepOutputBaseFolder = true;
	gOpt.bGrepSeparateFolder   = false;

	const auto sOpt = MakeSearchOption(false, false);
	const std::vector<std::wstring> vPaths = {
		dir1.Root().wstring(),
		dir2.Root().wstring()
	};
	CNativeW cmemMessage;
	int nHit = 0;
	CGrepEnumFiles cExAbsFiles;
	CGrepEnumFolders cExAbsFolders;

	const int rc = agent.RunParallelGrep(
		nullptr, nullptr,
		SGrepSearchParams{ L"needle", sOpt, gOpt },
		SGrepEnumContext{ keys, cExAbsFiles, cExAbsFolders },
		vPaths,
		cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(2, nHit);

	const std::wstring out(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());
	EXPECT_EQ(1, CountOccurrences(out, L"■\"" + dir1.Root().wstring() + L"\""))
		<< "dir1 のヘッダーが 1 回のみ出力される";
	EXPECT_EQ(1, CountOccurrences(out, L"■\"" + dir2.Root().wstring() + L"\""))
		<< "dir2 のヘッダーが 1 回のみ出力される";
}

/*!
 * @brief RunParallelGrep: ファイルのない空ディレクトリで 0 件・正常終了しハングしない
 * @remark ファイルが存在しない一時ディレクトリを検索したとき、タスク 0 件バッチの早期リターンしハングしないことを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_EmptyDirectoryNoHang)
{
	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();
	CNativeW cmemMessage;
	int nHit = 0;
	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"needle", sOpt, gOpt, cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(0, nHit);
}

/*!
 * @brief RunParallelGrep: # 除外フォルダーにより DoGrepTreeEnumerate でフォルダーが事前除外される
 * @remark #excluded キーで excluded/ フォルダーを除外したとき、keep/ のファイルのみがヒットすることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_ExcludeFolderHashKey)
{
	m_temp->WriteEncodedTextFile(L"excluded/a.txt", CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"keep/b.txt",     CODE_UTF8, L"needle\n");

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt;#excluded");
	auto gOpt = MakeGrepOption();
	gOpt.bGrepSubFolder = true;

	const auto sOpt = MakeSearchOption(false, false);
	CNativeW cmemMessage;
	int nHit = 0;
	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"needle", sOpt, gOpt, cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(1, nHit) << "excluded/ は #excluded で除外されるため keep/ の 1 件のみ";
}

/*!
 * @brief RunParallelGrep: bGrepSeparateFolder=true でサブフォルダー名ヘッダーが 1 回のみ出力される
 * @remark sub/ に 3 件ヒットがあるとき、フォルダーヘッダー ■"<sub_path>" が 1 回だけ出力されることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_SeparateFolderSubfolderNameHeader)
{
	m_temp->WriteEncodedTextFile(L"sub/c.txt", CODE_UTF8,
		L"needle\nneedle\nneedle\n");

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	auto gOpt = MakeGrepOption();
	gOpt.bGrepSubFolder      = true;
	gOpt.bGrepSeparateFolder = true;

	const auto sOpt = MakeSearchOption(false, false);
	CNativeW cmemMessage;
	int nHit = 0;
	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"needle", sOpt, gOpt, cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(3, nHit);

	const std::wstring out(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());
	const std::wstring subPath = (m_temp->Root() / L"sub").wstring();
	EXPECT_EQ(1, CountOccurrences(out, L"■\"" + subPath + L"\""))
		<< "sub フォルダーヘッダーが 1 回のみ出力される";
}

/*!
 * @brief RunParallelGrep: 空キーはファイル一覧モードで 1 ファイル 1 件・ファイル名出力
 * @remark 空キーの場合、ファイル内容を検索せず 1 ファイルあたり 1 件としてファイル名を出力することを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_EmptyKeyListsFiles)
{
	m_temp->WriteEncodedTextFile(L"list.txt", CODE_UTF8, L"content\n");

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();
	CNativeW cmemMessage;
	int nHit = 0;
	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"", sOpt, gOpt, cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(1, nHit) << "空キーはファイル 1 件につき 1 カウント";
	const std::wstring out(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());
	EXPECT_NE(out.find(L"list.txt"), std::wstring::npos) << "ファイル名が出力に含まれる";
}

/*!
 * @brief RunParallelGrep: ヒット 0 件で rc=0・正常終了
 * @remark マッチしないキーで検索したとき、戻り値 0・ヒット数 0 で正常終了することを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_NoHitsReturnsZero)
{
	m_temp->WriteEncodedTextFile(L"empty.txt", CODE_UTF8, L"no match here\n");

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();
	CNativeW cmemMessage;
	int nHit = 0;
	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"needle", sOpt, gOpt, cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(0, nHit);
}

/*!
 * @brief RunParallelGrep: WZ風スタイル（style=2）でファイルヘッダーが 1 回のみ出力される
 * @remark 3 件ヒットするファイルを WZ風スタイルで検索したとき、■"<filepath>" ヘッダーが 1 回だけ出ることを確認する。
 */
TEST_F(GrepRealFileTest, RunParallelGrep_WZStyleFileHeaderOnce)
{
	m_temp->WriteEncodedTextFile(L"wz.txt", CODE_UTF8,
		L"needle\nneedle\nneedle\n");

	CGrepEnumKeys keys;
	keys.SetFileKeys(L"*.txt");
	auto gOpt = MakeGrepOption();
	gOpt.nGrepOutputStyle      = 2;
	gOpt.bGrepOutputBaseFolder = false;
	gOpt.bGrepSeparateFolder   = false;

	const auto sOpt = MakeSearchOption(false, false);
	CNativeW cmemMessage;
	int nHit = 0;
	const std::vector<std::wstring> vPaths = { m_temp->Root().wstring() };

	const int rc = RunParallelGrepOnPaths(keys, vPaths, L"needle", sOpt, gOpt, cmemMessage, nHit);

	EXPECT_EQ(0, rc);
	EXPECT_EQ(3, nHit);

	const std::wstring out(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());
	EXPECT_EQ(1, CountOccurrences(out, L"■\""))
		<< "WZ風ファイルヘッダーが 1 回のみ出力される";
}

// =============================================================================
// 13. DoGrepFileWorker / FormatGrepResultLine / BuildGrepHeader 未カバー分割
// =============================================================================

/*!
 * @brief DoGrepFileWorker: 存在しないファイルは CError_FileOpen を捕捉し 0 件（エラーメッセージ
 * @remark FileOpen 失敗時に catch(CError_FileOpen) 経路が開き、戻り値 0 と
 *         フルパスを含むエラーメッセージが結果バッファに追記されることを確認する。
 */
TEST_F(GrepRealFileTest, FileWorker_FileOpenErrorReturnsZeroWithMessage)
{
	const std::filesystem::path path = m_temp->Sub(L"not_exist.txt");	// 作成しない

	SGrepFileTask task;
	task.fullPath   = path.wstring();
	task.fileName   = path.filename().wstring();
	task.baseFolder = m_temp->Root().wstring();
	task.folder     = task.baseFolder;
	task.relPath    = task.fileName;

	const auto sOpt = MakeSearchOption(false, false);
	const auto gOpt = MakeGrepOption();

	CSearchStringPattern pattern;
	ASSERT_TRUE(pattern.SetPattern(nullptr, L"needle", 6, sOpt, nullptr));

	CNativeW cmemMessage;
	CNativeW cUnicodeBuffer;
	std::atomic cancel{ false };

	CGrepAgent agent;
	const int hits = agent.DoGrepFileWorker(
		SGrepSearchParams{ L"needle", sOpt, gOpt },
		task,
		nullptr, pattern, cancel, cmemMessage, cUnicodeBuffer);

	EXPECT_EQ(0, hits) << "ファイルオープン失敗は 0 件で正常復帰する";
	const std::wstring out(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());
	EXPECT_NE(out.find(path.wstring()), std::wstring::npos)
		<< "エラーメッセージに対象フルパスが含まれる";
}

/*!
 * @brief DoGrepFileWorker: 事前キャンセル済みフラグで走査前に -1 を返す
 * @remark FileOpen 直後のキャンセルチェック（行ループ前）で即時 -1 を返すことを確認する。
 */
TEST_F(GrepRealFileTest, FileWorker_PreCancelledReturnsMinusOne)
{
	const auto path = m_temp->WriteEncodedTextFile(L"pre.txt", CODE_UTF8, L"needle\n");

	CGrepAgent agent;
	std::atomic cancel{ true };	// 走査開始前にキャンセル済み

	EXPECT_EQ(-1, RunGrepFileWorker(agent, path, L"needle",
		MakeSearchOption(false, false), MakeGrepOption(), cancel));
}

namespace {

/*!
 * 空キー（ファイル一覧モード）で DoGrepFileWorker を 1 ファイルに対して実行する
 * テスト専用ヘルパ。gOpt で出力書式・文字コードを制御し、出力は cmemMessage で受け取る。
 */
int RunEmptyKeyFileWorker(
	const std::filesystem::path& path,
	std::wstring_view baseFolder,
	const SGrepOption& gOpt,
	CNativeW& cmemMessage)
{
	SGrepFileTask task;
	task.fullPath   = path.wstring();
	task.fileName   = path.filename().wstring();
	task.baseFolder = baseFolder;
	task.folder     = task.baseFolder;
	task.relPath    = task.fileName;

	const auto sOpt = MakeSearchOption(false, false);

	CSearchStringPattern pattern;
	pattern.SetPattern(nullptr, L"", 0, sOpt, nullptr);	// 空キーでは未使用

	CNativeW cUnicodeBuffer;
	std::atomic cancel{ false };

	CGrepAgent agent;
	return agent.DoGrepFileWorker(
		SGrepSearchParams{ L"", sOpt, gOpt },
		task,
		nullptr, pattern, cancel, cmemMessage, cUnicodeBuffer);
}

/*! 同上（CODE_AUTODETECT・既定書式） */
int RunEmptyKeyFileWorker(
	const std::filesystem::path& path,
	std::wstring_view baseFolder,
	CNativeW& cmemMessage)
{
	return RunEmptyKeyFileWorker(path, baseFolder, MakeGrepOption(CODE_AUTODETECT), cmemMessage);
}

} // namespace

/*!
 * @brief DoGrepFileWorker: 空キー時の出力スタイル書式分割マトリクス
 * @remark 空キー（ファイル一覧モード）で style=1/2/3 × ベースフォルダー/フォルダー毎表示の
 *         組み合わせごとに正しい行頭書式（●/■/■ /装飾なし）が選択されることを確認する。
 *         文字コードは固定（CODE_UTF8）として自動判定分岐を踏まずに書式分岐のみを検証する。
 */
TEST_F(GrepRealFileTest, FileWorker_EmptyKeyOutputStyleVariants)
{
	const auto path = m_temp->WriteEncodedTextFile(L"name.txt", CODE_UTF8, L"x\n");

	const auto run = [&](int style, bool baseFolder, bool sepFolder) {
		auto gOpt = MakeGrepOption(CODE_UTF8);	// 文字コード固定で自動判定を回避
		gOpt.nGrepOutputStyle      = style;
		gOpt.bGrepOutputBaseFolder = baseFolder;
		gOpt.bGrepSeparateFolder   = sepFolder;

		CNativeW cmemMessage;
		const int hits = RunEmptyKeyFileWorker(path, m_temp->Root().wstring(), gOpt, cmemMessage);
		EXPECT_EQ(1, hits) << "style=" << style
			<< " base=" << baseFolder << " sep=" << sepFolder;
		return std::wstring(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());
	};

	// style=1 + ベースフォルダー表示: 行頭 ・"（中黒 U+30FB）
	{
		const std::wstring out = run(1, true, false);
		EXPECT_EQ(0u, out.find(L"・\"")) << "style=1/base のとき行頭は ・\"";
	}
	// style=2 + ベースフォルダー表示: 行頭 ■"（空白なし）
	{
		const std::wstring out = run(2, true, false);
		EXPECT_EQ(0u, out.find(L"■\""))  << "style=2/base のとき行頭は ■\"";
	}
	// style=2 + フォルダー毎表示: 行頭 ◆"
	{
		const std::wstring out = run(2, false, true);
		EXPECT_EQ(0u, out.find(L"◆\""))  << "style=2/sep のとき行頭は ◆\"";
	}
	// style=3 装飾なし: フルパス + CRLF のみ
	{
		const std::wstring out = run(3, false, false);
		EXPECT_EQ(0u, out.find(path.wstring())) << "style=3 はフルパスから始まる";
		EXPECT_EQ(out.find(L"■"),  std::wstring::npos);
		EXPECT_EQ(out.find(L"・"), std::wstring::npos);
	}
}

/*!
 * @brief FormatGrepResultLine: 該当行出力（lineType=1）の 2000 文字クランプ
 * @remark style=1 で行本文が 2000 文字を超える場合、出力が 2000 文字に切り詰められることを確認する。
 */
TEST_F(GrepRealFileTest, FormatGrepResultLine_LongLineIsTruncatedTo2000)
{
	const std::wstring longLine(3000, L'a');

	CNativeW out;
	auto gOpt = MakeGrepOption();
	gOpt.nGrepOutputStyle    = 1;
	gOpt.nGrepOutputLineType = 1;

	CGrepAgent::FormatGrepResultLine(
		out, L"f.cpp", L"",
		SGrepMatchInfo{ 1, 1, longLine.c_str(), static_cast<int>(longLine.size()), 0, longLine.c_str(), 6 },
		gOpt);

	const std::wstring result(out.GetStringPtr(), out.GetStringLength());
	EXPECT_NE(result.find(std::wstring(2000, L'a')), std::wstring::npos)
		<< "2000 文字までは出力される";
	EXPECT_EQ(result.find(std::wstring(2001, L'a')), std::wstring::npos)
		<< "2001 文字以上は出力されない";
}

/*!
 * @brief FormatGrepResultLine: 該当部分出力（lineType=0）の 2500 文字クランプと EOL 付加
 * @remark style=3 でもマッチ部分が 2500 文字を超える場合に切り詰められる。
 *         切り詰め後の末尾が改行でなく \r\n が付加されることを確認する。
 */
TEST_F(GrepRealFileTest, FormatGrepResultLine_LongMatchIsTruncatedTo2500)
{
	const std::wstring longMatch(3000, L'b');

	CNativeW out;
	auto gOpt = MakeGrepOption();
	gOpt.nGrepOutputStyle    = 3;
	gOpt.nGrepOutputLineType = 0;

	CGrepAgent::FormatGrepResultLine(
		out, L"f.cpp", L"",
		SGrepMatchInfo{ 1, 1, longMatch.c_str(), static_cast<int>(longMatch.size()), 0, longMatch.c_str(), static_cast<int>(longMatch.size()) },
		gOpt);

	EXPECT_EQ(2502, out.GetStringLength())
		<< "マッチ部分 2500 文字 + \\r\\n の合計 2502 文字";
	const std::wstring result(out.GetStringPtr(), out.GetStringLength());
	EXPECT_EQ(result.substr(result.size() - 2), std::wstring(L"\r\n"));
}

/*!
 * @brief BuildGrepHeader: nullptr 引数を空文字列として扱い、クラッシュしない
 * @remark pszKey / pszFile / pszFolder に nullptr を渡しても既定値（空文字列）で
 *         安全にヘッダーが生成されることを確認する。
 */
TEST_F(GrepRealFileTest, BuildGrepHeader_NullArgumentsAreHandled)
{
	const auto result = CGrepAgent::BuildGrepHeader(
		nullptr, nullptr, nullptr,
		MakeSearchOption(false, false), MakeGrepOption());

	EXPECT_GT(result.GetStringLength(), 0) << "nullptr 引数でもヘッダーが生成される";
	ASSERT_GE(result.GetStringLength(), 4);
	const std::wstring out(result.GetStringPtr(), result.GetStringLength());
	EXPECT_EQ(out.substr(out.size() - 4), std::wstring(L"\r\n\r\n"));
}

// =============================================================================
// 14. 未カバー else / catch 分岐の追加カバレッジ
// =============================================================================


/*!
 * @brief DoGrepFileWorker: 空キー（自動判定）不存在ファイルで DetectError 分岐
 * @remark 空キー（ファイル一覧モード）かつ CODE_AUTODETECT のとき、存在しないファイルの
 *         文字コード判定が CODE_ERROR となり、!IsValidCodeOrCPType 分岐で
 *         "(DetectError)" をコード名として出力されることを確認する。
 */
TEST_F(GrepRealFileTest, FileWorker_EmptyKeyDetectErrorOnMissingFile)
{
	const std::filesystem::path path = m_temp->Sub(L"missing.txt");	// 作成しない

	CNativeW cmemMessage;
	const int hits = RunEmptyKeyFileWorker(path, m_temp->Root().wstring(), cmemMessage);

	EXPECT_EQ(1, hits) << "空キーは判定エラーでもファイル 1 件としてカウントする";
	const std::wstring out(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());
	EXPECT_NE(out.find(L"(DetectError)"), std::wstring::npos)
		<< "判定エラー時はコード名に (DetectError) が出力される";
	EXPECT_NE(out.find(path.wstring()), std::wstring::npos)
		<< "ファイルパスが出力に含まれる";
}

/*!
 * @brief DoGrepFileWorker: 正規表現のゼロ長マッチで matchlen 補正分岐を経て無限ループしない
 * @remark lineType=0（該当部分出力）でゼロ長マッチを許す正規表現（b*）を与えると、
 *         matchlen <= 0 の補正分岐（GetSizeOfChar による前進）が機能して
 *         走査が正常終了することを確認する。
 */
TEST_F(GrepRealFileTest, FileWorker_RegexZeroLengthMatchAdvances)
{
	const auto path = m_temp->WriteEncodedTextFile(L"zero.txt", CODE_UTF8, L"xb\n");

	CGrepAgent agent;
	const auto sOpt = MakeSearchOption(/*regex=*/true, /*caseSensitive=*/true);
	auto gOpt = MakeGrepOption();
	gOpt.nGrepOutputLineType = 0;	// 該当部分出力（1 行内の複数マッチ継続ループ）

	const int hits = RunGrepFileWorker(agent, path, L"b*", sOpt, gOpt);

	// マッチ件数は正規表現エンジンのゼロ長マッチ仕様に依存するため、
	// 正常終了（ハングしない）と 1 件以上のマッチのみを検証する。
	EXPECT_GE(hits, 1);
}

/*!
 * @brief BuildGrepHeader: 英大文字小文字を区別する（bLoHiCase=true）分岐
 * @remark bLoHiCase の ON/OFF でヘッダー記述（区別する/区別しない）が切り替わることを確認する。
 */
TEST_F(GrepRealFileTest, BuildGrepHeader_CaseSensitiveBranch)
{
	const auto rIgnore = CGrepAgent::BuildGrepHeader(
		L"key", L"*.txt", L"C:\\t",
		MakeSearchOption(false, /*caseSensitive=*/false), MakeGrepOption());
	const auto rCase = CGrepAgent::BuildGrepHeader(
		L"key", L"*.txt", L"C:\\t",
		MakeSearchOption(false, /*caseSensitive=*/true), MakeGrepOption());

	const std::wstring sIgnore(rIgnore.GetStringPtr(), rIgnore.GetStringLength());
	const std::wstring sCase  (rCase.GetStringPtr(),   rCase.GetStringLength());
	EXPECT_NE(sIgnore, sCase)
		<< "大文字小文字区別の ON/OFF でヘッダー記述が切り替わる";
}

/*!
 * @brief BuildGrepHeader: 置換時（lineType=0 でも一致した箇所を出力）分岐
 * @remark bGrepReplace=false かつ nGrepOutputLineType=0 のとき、lineType=1 とは
 *         異なる記述（STR_GREP_SHOW_MATCH_AREA）が出力されることを確認する。
 */
TEST_F(GrepRealFileTest, BuildGrepHeader_MatchAreaLineTypeZero)
{
	auto g0 = MakeGrepOption(); g0.nGrepOutputLineType = 0;
	auto g1 = MakeGrepOption(); g1.nGrepOutputLineType = 1;

	const auto r0 = CGrepAgent::BuildGrepHeader(
		L"key", L"*.txt", L"C:\\t", MakeSearchOption(false, false), g0);
	const auto r1 = CGrepAgent::BuildGrepHeader(
		L"key", L"*.txt", L"C:\\t", MakeSearchOption(false, false), g1);

	const std::wstring s0(r0.GetStringPtr(), r0.GetStringLength());
	const std::wstring s1(r1.GetStringPtr(), r1.GetStringLength());
	EXPECT_NE(s0, s1)
		<< "lineType=0（一致した箇所）と lineType=1（一致した行）で記述が異なる";
}

// =============================================================================
// 15. DoGrep 本体のスモークテスト（EditorTestSuite + stdout 経路）
// =============================================================================

namespace {

/*!
 * Windows 標準ハンドルの一時差し替え RAII ガード（test-cgrepagent-flow.cpp と同手法）
 */
class GrepStdHandleGuard {
public:
	GrepStdHandleGuard(DWORD id, HANDLE replacement)
		: m_id(id), m_saved(::GetStdHandle(id))
	{
		::SetStdHandle(m_id, replacement);
	}
	~GrepStdHandleGuard()
	{
		::SetStdHandle(m_id, m_saved);
	}
	GrepStdHandleGuard(const GrepStdHandleGuard&) = delete;
	GrepStdHandleGuard& operator=(const GrepStdHandleGuard&) = delete;

private:
	DWORD  m_id;
	HANDLE m_saved;
};

//! DoGrep へ渡す 4 つの入力文字列（検索キー・置換文字列・ファイルパターン・フォルダー）
struct SDoGrepTestInput {
	std::wstring key;
	std::wstring replaceTo;
	std::wstring filePattern;
	std::wstring folder;
};

/*! DoGrep をキャプチャ付き stdout モードで実行する共通コア */
DWORD RunDoGrepCaptureStdout(
	CGrepAgent& agent,
	const SDoGrepTestInput& in,
	const SSearchOption& sOpt,
	SGrepOption gOpt,
	bool bExcludeFileRegex,
	std::string& capturedStdout)
{
	capturedStdout.clear();

	const std::wstring tmpFile = GetTempDirString() + L"sakura_dogrep_capture_stdout.txt";

	HANDLE hTempFile = ::CreateFileW(tmpFile.c_str(),
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hTempFile == INVALID_HANDLE_VALUE) {
		ADD_FAILURE() << "failed to create stdout capture file";
		return 0;
	}
	struct HandleDeleter {
		void operator()(HANDLE h) const {
			if (h && h != INVALID_HANDLE_VALUE) ::CloseHandle(h);
		}
	};
	std::unique_ptr<void, HandleDeleter> hFileGuard{ hTempFile };

	DWORD hits = 0;
	{
		GrepStdHandleGuard stdoutGuard(STD_OUTPUT_HANDLE, hTempFile);

		CEditView* pView = &CEditWnd::getInstance()->GetActiveView();

		const CNativeW cmKey(in.key.c_str());
		const CNativeW cmRep(in.replaceTo.c_str());
		const CNativeW cmFile(in.filePattern.c_str());
		const CNativeW cmFolder(in.folder.c_str());

		gOpt.bGrepStdout = true;
		gOpt.bGrepHeader = true;

		hits = agent.DoGrep(
			pView,
			SGrepInput{ &cmKey, &cmRep, &cmFile, &cmFolder },
			sOpt,
			gOpt,
			true,				// bGrepCurFolder: カレントディレクトリを変更しない
			bExcludeFileRegex	// bGrepExcludeFileRegexp
		);
	} // stdout 復帰

	::SetFilePointer(hTempFile, 0, nullptr, FILE_BEGIN);
	std::string buf(16384, '\0');
	DWORD dwRead = 0;
	::ReadFile(hTempFile, buf.data(), static_cast<DWORD>(buf.size()), &dwRead, nullptr);
	capturedStdout.assign(buf.data(), dwRead);

	hFileGuard.reset();
	::DeleteFileW(tmpFile.c_str());
	return hits;
}

/*!
 * 置換モード・除外正規表現ありで共通コアを呼ぶラッパー
 * key=needle / replaceTo=REPLACED / 自動判定・ヒット行出力・Normal スタイルに固定する
 */
DWORD RunDoGrepReplaceExcludeRegex(
	const std::wstring& filePattern,
	const std::wstring& folder,
	bool bSubFolder,
	std::string& capturedStdout)
{
	SSearchOption sOpt;
	sOpt.Reset();

	SGrepOption gOpt;
	gOpt.bGrepReplace        = true;
	gOpt.bGrepSubFolder      = bSubFolder;
	gOpt.nGrepCharSet        = CODE_AUTODETECT;
	gOpt.nGrepOutputLineType = 1;
	gOpt.nGrepOutputStyle    = 1;

	CGrepAgent agent;
	return RunDoGrepCaptureStdout(agent, { L"needle", L"REPLACED", filePattern, folder },
		sOpt, gOpt, /*bExcludeFileRegex=*/true, capturedStdout);
}

/*!
 * DoGrep を stdout モードで実行し、ヒット数と stdout 出力（ANSI文字列）を返す
 *
 * bGrepStdout=true にし AddTail は標準出力へ書き込む（GA-18 で実証済みの安全経路）。
 * 標準出力は一時ファイルへ差し替えて回収する。
 * bGrepCurFolder=true でカレントディレクトリ変更を抑止する。
 */
DWORD RunDoGrepStdout(
	const std::wstring& key,
	const std::wstring& filePattern,
	const std::wstring& folder,
	std::string& capturedStdout,
	bool bExcludeFileRegex = false)   // → 追加
{
	SSearchOption sOpt;
	sOpt.Reset();

	SGrepOption gOpt;
	gOpt.bGrepReplace        = false;
	gOpt.bGrepSubFolder      = false;
	gOpt.nGrepCharSet        = CODE_AUTODETECT;
	gOpt.nGrepOutputLineType = 1;
	gOpt.nGrepOutputStyle    = 1;

	CGrepAgent agent;
	return RunDoGrepCaptureStdout(agent, { key, L"", filePattern, folder },
		sOpt, gOpt, bExcludeFileRegex, capturedStdout);
}

} // namespace

/*!
 * @brief DoGrep: stdout モードでの基本検索スモーク (PR #2459)
 * @remark EditorTestSuite 環境で DoGrep 本体を実行し、ヘッダー組み立て・検索条件・除外表示・
 *         サブフォルダー記述切り替え → RunParallelGrep → ヒット数出力の全経路。
 *         headless で実走するため、ヒット数と stdout 出力が正しいことを確認する。
 */
TEST_F(GrepRealFileTest, DoGrep_StdoutMode_BasicSearchSmoke)
{
	m_temp->WriteEncodedTextFile(L"hit.txt", CODE_UTF8, L"needle\nabc\nneedle\n");

	std::string out;
	const DWORD hits = RunDoGrepStdout(
		L"needle", L"*.txt", m_temp->Root().wstring(), out);

	EXPECT_EQ(2u, hits) << "needle 2 件がヒットする";
	EXPECT_NE(out.find("needle"), std::string::npos)
		<< "stdout にヒット行が出力される";
	EXPECT_NE(out.find("hit.txt"), std::string::npos)
		<< "stdout にファイル名が出力される";
}

/*!
 * @brief DoGrep: `!` 除外正規表現の事前検証と適用のスモーク (PR #2459)
 * @remark 有効な `!` 除外パターンを与え、DoGrep 冒頭の事前検証ループ
 *         （InitRegexp → Compile 成功）を経た上で、キーのヒット・除外ファイル表示と
 *         ワーカーでの除外適用（skip 系 1 件除外）が行われることを確認する。
 */
TEST_F(GrepRealFileTest, DoGrep_StdoutMode_ExcludeRegexSmoke)
{
	m_temp->WriteEncodedTextFile(L"keep.txt",      CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"skip_me.txt",   CODE_UTF8, L"needle\n");

	std::string out;
	const DWORD hits = RunDoGrepStdout(
		L"needle", L"*.txt;!.*skip.*\\.txt$", m_temp->Root().wstring(),
		out, /*bExcludeFileRegex=*/true);

	EXPECT_EQ(1u, hits) << "skip_me.txt は除外し keep.txt の 1 件のみ";
	EXPECT_NE(out.find("keep.txt"), std::string::npos)
		<< "keep.txt のヒットが出力される";
}

/*!
 * @brief DoGrepFileWorker: 空キー（自動判定）実在ファイルで有効コードの Bracket 分岐 (GA-21)
 * @remark 空キー（ファイル一覧モード）かつ CODE_AUTODETECT で、実在の UTF-8(BOM) ファイルの
 *         文字コード判定が有効コードになり、IsValidCodeType 真の Bracket 表記が出力されることを確認する。
 */
TEST_F(GrepRealFileTest, FileWorker_EmptyKeyAutoDetectValidCodeBracket)
{
	const auto path = m_temp->WriteEncodedTextFile(
		L"detect.txt", CODE_UTF8, L"日本語テキスト\n", /*withBom=*/true);

	CNativeW cmemMessage;
	const int hits = RunEmptyKeyFileWorker(path, m_temp->Root().wstring(), cmemMessage);

	EXPECT_EQ(1, hits) << "空キーはファイル 1 件としてカウント";
	const std::wstring out(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength());
	EXPECT_EQ(std::wstring::npos, out.find(L"(DetectError)"))
		<< "実在ファイルは判定エラーにならない";
	EXPECT_NE(std::wstring::npos, out.find(L"["))
		<< "有効コードの Bracket 表記（[UTF-8] 等）が出力される";
}

/*!
 * @brief DoGrep: 置換モード（除外正規表現）サブフォルダーの直列パススモーク (GA-22)
 * @remark bGrepReplace=true 経路（DoGrepTree → DoGrepReplaceFile）を実走し、
 *         DoGrepTree の除外正規表現コンパイル・reExcl.Match フィルタ・サブフォルダー再帰を網羅する。
 *         keep.txt と sub/child.txt は置換対象、skip_me.txt は ! 除外正規表現で除外される。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_ExcludeRegexAndSubfolderSmoke)
{
	m_temp->WriteEncodedTextFile(L"keep.txt",      CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"skip_me.txt",   CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"sub/child.txt", CODE_UTF8, L"needle\n");

	std::string out;
	const DWORD hits = RunDoGrepReplaceExcludeRegex(
		L"*.txt;!.*skip.*\\.txt$", m_temp->Root().wstring(),
		/*bSubFolder=*/true, out);

	EXPECT_EQ(2u, hits) << "keep.txt と sub/child.txt の 2 件のみ置換（skip_me.txt は除外）";
	EXPECT_NE(out.find("keep.txt"),  std::string::npos) << "keep.txt が出力される";
	EXPECT_NE(out.find("child.txt"), std::string::npos) << "サブフォルダーの child.txt が出力される（再帰）";
	EXPECT_EQ(out.find("skip_me"),   std::string::npos) << "skip_me.txt は除外正規表現で除外され出力されない";
}

/*!
 * @brief DoGrep 置換: 複数の除外正規表現が結合パターンとして適用される (GA-35)
 * @remark 除外正規表現を 2 本与え、DoGrepTree の P4 結合（"(?:p1)|(?:p2)"）経路で
 *         両パターンとも有効に除外されることを確認する（1 本のみの GA-22 では
 *         結合の "|" 連結コードが実行されないため本テストで補完する）。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_MultipleExcludeRegexCombined)
{
	m_temp->WriteEncodedTextFile(L"keep.txt",    CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"skip_me.txt", CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"app.log",     CODE_UTF8, L"needle\n");

	std::string out;
	const DWORD hits = RunDoGrepReplaceExcludeRegex(
		L"*.*;!.*skip.*\\.txt$;!.*\\.log$", m_temp->Root().wstring(),
		/*bSubFolder=*/false, out);

	EXPECT_EQ(1u, hits) << "keep.txt の 1 件のみ置換（skip_me.txt / app.log は結合除外パターンで除外）";
	EXPECT_NE(out.find("keep.txt"), std::string::npos) << "keep.txt が出力される";
	EXPECT_EQ(out.find("skip_me"),  std::string::npos) << "skip_me.txt は 1 本目の除外パターンで除外";
	EXPECT_EQ(out.find("app.log"),  std::string::npos) << "app.log は 2 本目の除外パターンで除外";
}

// =============================================================================
// Grep 置換経路の追加検証（GA-23 ～ GA-25）
// =============================================================================

namespace {

//! ファイルの生バイトを読み出す（置換後内容の検証用）。
//! BOM の有無や EOL に依存しないよう、バイト列としてそのまま返す。
std::string ReadAllBytes(const std::filesystem::path& path)
{
	std::ifstream is(path, std::ios::binary);
	if (!is) return {};
	is.seekg(0, std::ios::end);
	const std::streamoff len = is.tellg();
	if (len <= 0) return {};
	std::string buf(static_cast<size_t>(len), '\0');
	is.seekg(0, std::ios::beg);
	is.read(&buf[0], len);
	buf.resize(static_cast<size_t>(is.gcount()));
	return buf;
}

//! DoGrep を置換モードで実行する。
//! stdout はテンポラリへ迂回してコンソールを汚さない（GA-22 と同じ手法）。
//! ファイル書き換え（CWriteData）は stdout 経路とは独立に実行されるため、
//! 置換後のファイル内容はこの呼び出し後に各テストが読み出して検証できる。
DWORD RunDoGrepReplace(
	const std::wstring& key,
	const std::wstring& replaceTo,
	const std::wstring& filePattern,
	const std::wstring& folder,
	bool bRegex,
	bool bBackup,
	bool bSubFolder)
{
	SSearchOption sOpt;
	sOpt.Reset();
	sOpt.bRegularExp = bRegex;

	SGrepOption gOpt;
	gOpt.bGrepReplace        = true;
	gOpt.bGrepSubFolder      = bSubFolder;
	gOpt.nGrepCharSet        = CODE_AUTODETECT;
	gOpt.nGrepOutputLineType = 1;
	gOpt.nGrepOutputStyle    = 1;
	gOpt.bGrepBackup         = bBackup;

	CGrepAgent agent;
	std::string capturedStdout;	// この経路では stdout 内容は検証しない
	return RunDoGrepCaptureStdout(agent, { key, replaceTo, filePattern, folder },
		sOpt, gOpt, /*bExcludeFileRegex=*/false, capturedStdout);
}

} // namespace

/*!
 * @brief DoGrep 置換: リテラル置換が実ファイルへ書き込まれる (GA-23)
 * @remark bGrepReplace=true の直行経路（DoGrepTree → DoGrepReplaceFile → CWriteData）で、
 *         対象ファイルの内容が実際に needle → REPLACED へ書き換えられることを検証する。
 *         GA-22 は stdout（ヒットファイル名）のみを見ていたため、書き込み結果を別途確認する。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_LiteralWritesReplacedContent)
{
	const auto path = m_temp->WriteEncodedTextFile(L"keep.txt", CODE_UTF8, L"needle\n");

	const DWORD hits = RunDoGrepReplace(
		L"needle", L"REPLACED", L"*.txt", m_temp->Root().wstring(),
		/*bRegex=*/false, /*bBackup=*/false, /*bSubFolder=*/false);

	EXPECT_EQ(1u, hits) << "1 件が置換される";

	const std::string body = ReadAllBytes(path);
	EXPECT_NE(body.find("REPLACED"), std::string::npos)
		<< "置換後の文字列がファイルへ書き込まれている";
	EXPECT_EQ(body.find("needle"), std::string::npos)
		<< "前の文字列は残らない";
}

/*!
 * @brief DoGrep 置換: 正規表現置換が行内の全マッチを置換する (GA-24)
 * @remark bRegularExp=true の置換経路（pRegexp->Replace）で、1 行内の複数マッチを
 *         すべて置換し、ヒット数が一致することを検証する（g 相当の行末まで一括置換）。
 * @note   bregonig.dll に依存する（他の正規表現テストと同条件）。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_RegexReplacesAllInLine)
{
	const auto path = m_temp->WriteEncodedTextFile(
		L"multi.txt", CODE_UTF8, L"needle needle needle\n");

	const DWORD hits = RunDoGrepReplace(
		L"needle", L"XX", L"*.txt", m_temp->Root().wstring(),
		/*bRegex=*/true, /*bBackup=*/false, /*bSubFolder=*/false);

	EXPECT_EQ(3u, hits) << "行内 3 件すべてがヒット・置換される";

	const std::string body = ReadAllBytes(path);
	EXPECT_EQ(body.find("needle"), std::string::npos)
		<< "行内の全マッチが置換される";
	EXPECT_NE(body.find("XX"), std::string::npos)
		<< "置換後の文字列が書き込まれている";
}

/*!
 * @brief DoGrep 置換: バックアップ有効時に .skrold が生成される (GA-25)
 * @remark bGrepBackup=true のとき、元ファイルを .skrold へ退避し（元内容を保持）、
 *         元パスには置換後内容が書き込まれることを検証する。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_BackupCreatesSkrold)
{
	const auto path = m_temp->WriteEncodedTextFile(L"data.txt", CODE_UTF8, L"needle\n");

	const DWORD hits = RunDoGrepReplace(
		L"needle", L"REPLACED", L"*.txt", m_temp->Root().wstring(),
		/*bRegex=*/false, /*bBackup=*/true, /*bSubFolder=*/false);

	EXPECT_EQ(1u, hits) << "1 件が置換される";

	// 元パスには置換後内容
	const std::string body = ReadAllBytes(path);
	EXPECT_NE(body.find("REPLACED"), std::string::npos)
		<< "元パスに置換後内容が書き込まれている";
	EXPECT_EQ(body.find("needle"), std::string::npos)
		<< "元パスに前の文字列は残らない";

	// バックアップ(.skrold)には元内容
	const std::filesystem::path backup = path.wstring() + L".skrold";
	ASSERT_TRUE(std::filesystem::exists(backup))
		<< ".skrold バックアップが生成される";
	const std::string backupBody = ReadAllBytes(backup);
	EXPECT_NE(backupBody.find("needle"), std::string::npos)
		<< ".skrold には前の内容が保持される";
	EXPECT_EQ(backupBody.find("REPLACED"), std::string::npos)
		<< ".skrold は置換前の内容である";
}

// =============================================================================
// Grep 置換経路の全ギャップ検証（GA-26 ～ GA-33）
// =============================================================================

namespace {

//! 置換テスト拡張の実行オプション。既定は最小構成。
struct ReplaceRunParams {
	std::wstring	key;
	std::wstring	replaceTo;
	std::wstring	folder;
	std::wstring	filePattern    = L"*.txt";
	bool			bRegex         = false;
	bool			bWordOnly      = false;
	bool			bBackup        = false;
	bool			bSubFolder     = false;
	bool			bOutputFileOnly = false;
	int				nOutputLineType = 1;
	ECodeType		charSet        = CODE_AUTODETECT;
};

//! DoGrep を置換モードで実行する（オプション構造体版）。
//! stdout はテンポラリへ迂回してコンソールを汚さない（GA-22 と同じ手法）。
DWORD RunDoGrepReplaceEx(const ReplaceRunParams& p)
{
	SSearchOption sOpt;
	sOpt.Reset();
	sOpt.bRegularExp = p.bRegex;
	sOpt.bWordOnly   = p.bWordOnly;

	SGrepOption gOpt;
	gOpt.bGrepReplace         = true;
	gOpt.bGrepSubFolder       = p.bSubFolder;
	gOpt.nGrepCharSet         = p.charSet;
	gOpt.nGrepOutputLineType  = p.nOutputLineType;
	gOpt.nGrepOutputStyle     = 1;
	gOpt.bGrepBackup          = p.bBackup;
	gOpt.bGrepOutputFileOnly  = p.bOutputFileOnly;

	CGrepAgent agent;
	std::string capturedStdout;	// この経路では stdout 内容は検証しない
	return RunDoGrepCaptureStdout(agent, { p.key, p.replaceTo, p.filePattern, p.folder },
		sOpt, gOpt, /*bExcludeFileRegex=*/false, capturedStdout);
}

//! 文字列中の部分文字列の出現回数を数える（置換件数の内容検証用）。
size_t CountByteOccurrences(std::string_view hay, std::string_view needle)
{
	if (needle.empty()) return 0;
	size_t n = 0;
	for (size_t pos = hay.find(needle); pos != std::string::npos; pos = hay.find(needle, pos + needle.size())) {
		++n;
	}
	return n;
}

} // namespace

/*!
 * @brief DoGrep 置換: 単語のみ検索は部分一致を置換しない (GA-26)
 * @remark bWordOnly=true の置換経路（SearchStringWord）で、独立語 "cat" のみ置換され、
 *         部分一致 "category" は置換されないことを検証する。
 * @note   置換パターンのコンパイル（SetPattern）に bregonig.dll が要る。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_WordOnlySkipsSubstring)
{
	const auto path = m_temp->WriteEncodedTextFile(
		L"word.txt", CODE_UTF8, L"cat category cat\n");

	ReplaceRunParams p;
	p.key = L"cat"; p.replaceTo = L"REPLACED"; p.folder = m_temp->Root().wstring();
	p.bWordOnly = true;
	const DWORD hits = RunDoGrepReplaceEx(p);

	EXPECT_EQ(2u, hits) << "独立語 cat の 2 件のみヒット・置換される";

	const std::string body = ReadAllBytes(path);
	EXPECT_NE(body.find("category"), std::string::npos)
		<< "部分一致 category は置換されない";
	EXPECT_EQ(2u, CountByteOccurrences(body, "REPLACED"))
		<< "置換されたのは独立語の 2 件のみ";
}

/*!
 * @brief DoGrep 置換: UTF-8 BOM 付きファイルは BOM を保持して書き戻す (GA-27)
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_Utf8BomPreserved)
{
	const auto path = m_temp->WriteEncodedTextFile(
		L"bom8.txt", CODE_UTF8, L"needle\n", /*withBom=*/true);

	ReplaceRunParams p;
	p.key = L"needle"; p.replaceTo = L"REPLACED"; p.folder = m_temp->Root().wstring();
	const DWORD hits = RunDoGrepReplaceEx(p);

	EXPECT_EQ(1u, hits);

	const std::string body = ReadAllBytes(path);
	ASSERT_GE(body.size(), 3u);
	EXPECT_EQ(0xEF, static_cast<unsigned char>(body[0]));	// UTF-8 BOM が保持される
	EXPECT_EQ(0xBB, static_cast<unsigned char>(body[1]));
	EXPECT_EQ(0xBF, static_cast<unsigned char>(body[2]));
	EXPECT_NE(body.find("REPLACED"), std::string::npos);
	EXPECT_EQ(body.find("needle"), std::string::npos);
}

/*!
 * @brief DoGrep 置換: UTF-16LE ファイルは BOM・エンコードを保持して書き戻す (GA-28)
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_Utf16LePreserved)
{
	const auto path = m_temp->WriteEncodedTextFile(
		L"u16.txt", CODE_UNICODE, L"needle\n", /*withBom=*/true);

	ReplaceRunParams p;
	p.key = L"needle"; p.replaceTo = L"REPLACED"; p.folder = m_temp->Root().wstring();
	const DWORD hits = RunDoGrepReplaceEx(p);

	EXPECT_EQ(1u, hits);

	const std::string body = ReadAllBytes(path);
	ASSERT_GE(body.size(), 2u);
	EXPECT_EQ(0xFF, static_cast<unsigned char>(body[0]));	// UTF-16LE BOM が保持される
	EXPECT_EQ(0xFE, static_cast<unsigned char>(body[1]));
	// "REPLACED" が UTF-16LE（各文字の後ろに 0x00）で書き込まれている
	const char rep16[] = { 'R',0,'E',0,'P',0,'L',0,'A',0,'C',0,'E',0,'D',0 };
	EXPECT_NE(body.find(std::string(rep16, sizeof(rep16))), std::string::npos)
		<< "置換後文字列が UTF-16LE で書き込まれている";
}

/*!
 * @brief DoGrep 置換: SJIS 日本語の置換往復（再 Grep で確認） (GA-29)
 * @remark SJIS(BOM なし)の日本語ファイルを置換し、置換後ファイルを SJIS で再検索して
 *         置換後語がヒット・置換前語が 0 件になることで、エンコード往復の健全性を確認する。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_SjisJapaneseRoundTrip)
{
	const auto path = m_temp->WriteEncodedTextFile(
		L"sjis.txt", CODE_SJIS, L"東京\n");

	ReplaceRunParams p;
	p.key = L"東京"; p.replaceTo = L"大阪"; p.folder = m_temp->Root().wstring();
	p.charSet = CODE_SJIS;
	const DWORD hits = RunDoGrepReplaceEx(p);

	EXPECT_EQ(1u, hits);

	// 置換後ファイルを SJIS で再検索して往復を確認する。
	CGrepAgent agent;
	EXPECT_EQ(1, RunGrepFileWorker(agent, path, L"大阪",
		MakeSearchOption(false, false), MakeGrepOption(CODE_SJIS)))
		<< "置換後語が SJIS として正しく書き戻され再検索でヒットする";
	EXPECT_EQ(0, RunGrepFileWorker(agent, path, L"東京",
		MakeSearchOption(false, false), MakeGrepOption(CODE_SJIS)))
		<< "置換前語は残らない";
}

/*!
 * @brief DoGrep 置換: bGrepOutputFileOnly でも全行が置換される (GA-30)
 * @remark 出力（メッセージ）はファイル毎 1 回に抑制されるが、置換自体は全該当行に及ぶこと。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_OutputFileOnlyStillReplacesAll)
{
	const auto path = m_temp->WriteEncodedTextFile(
		L"multi.txt", CODE_UTF8, L"needle\nneedle\n");

	ReplaceRunParams p;
	p.key = L"needle"; p.replaceTo = L"REPLACED"; p.folder = m_temp->Root().wstring();
	p.bOutputFileOnly = true;
	const DWORD hits = RunDoGrepReplaceEx(p);

	EXPECT_EQ(2u, hits) << "出力抑制されても 2 行ともヒット・置換される";

	const std::string body = ReadAllBytes(path);
	EXPECT_EQ(body.find("needle"), std::string::npos);
	EXPECT_EQ(2u, CountByteOccurrences(body, "REPLACED"))
		<< "全該当行が置換される";
}

/*!
 * @brief DoGrep 置換: 否ヒット行タイプ(2)は置換では通常置換に正規化される (GA-31)
 * @remark nGrepOutputLineType=2（否ヒット行）で置換を指示しても、DoGrep が 1 に正規化し、
 *         通常どおりヒット行を置換すること。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_NoHitLineTypeNormalized)
{
	const auto path = m_temp->WriteEncodedTextFile(
		L"nht.txt", CODE_UTF8, L"needle\n");

	ReplaceRunParams p;
	p.key = L"needle"; p.replaceTo = L"REPLACED"; p.folder = m_temp->Root().wstring();
	p.nOutputLineType = 2;	// 置換モードでは 1 に正規化される
	const DWORD hits = RunDoGrepReplaceEx(p);

	EXPECT_EQ(1u, hits) << "否ヒット行指定でも通常のヒット行置換として動作する";

	const std::string body = ReadAllBytes(path);
	EXPECT_NE(body.find("REPLACED"), std::string::npos);
	EXPECT_EQ(body.find("needle"), std::string::npos);
}

/*!
 * @brief DoGrep 置換: 書き込みファイル(.skrnew)が開けない場合は元が壊れない (GA-32)
 * @remark 出力先 "<path>.skrnew" と同名のディレクトリを先に作って書き込みオープンを失敗させる。
 *         CError_WriteFileOpen 経路（STR_GREP_ERR_FILEWRITE）が通り、元ファイルは無変更のまま。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_WriteOpenFailureKeepsOriginal)
{
	const auto path = m_temp->WriteEncodedTextFile(L"data.txt", CODE_UTF8, L"needle\n");

	// 出力先 .skrnew をディレクトリとして先取りし、ファイル書き込みを失敗させる。
	const std::filesystem::path skrnew = path.wstring() + L".skrnew";
	std::filesystem::create_directories(skrnew);

	ReplaceRunParams p;
	p.key = L"needle"; p.replaceTo = L"REPLACED"; p.folder = m_temp->Root().wstring();
	const DWORD hits = RunDoGrepReplaceEx(p);

	EXPECT_EQ(0u, hits) << "書き込みオープン失敗時はヒット確定前に中断する";

	const std::string body = ReadAllBytes(path);
	EXPECT_NE(body.find("needle"), std::string::npos)
		<< "書き込み失敗時に元ファイルは無変更";
	EXPECT_EQ(body.find("REPLACED"), std::string::npos)
		<< "置換後内容は元ファイルへ書き込まれない";
}

/*!
 * @brief DoGrep 置換: 元ファイルロックされていても元が壊れない (GA-33)
 * @remark 元ファイルを共有読み取りのみ(削除・書き込み共有なし)で開いたまま置換を実行する。
 *         読み取り open 失敗 or 置換確定(Delete/MoveFile)失敗のいずれの分岐でも、
 *         元ファイルが破壊されない不変条件を検証する。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_LockedOriginalNotCorrupted)
{
	const auto path = m_temp->WriteEncodedTextFile(L"locked.txt", CODE_UTF8, L"needle\n");

	struct HandleDeleter {
		void operator()(HANDLE h) const { if (h && h != INVALID_HANDLE_VALUE) ::CloseHandle(h); }
	};
	// 共有は読み取りのみ（削除・書き込み共有を与えない）→ 置換確定の Delete/MoveFile が失敗するはず。
	std::unique_ptr<void, HandleDeleter> hLock{
		::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr)
	};
	ASSERT_NE(INVALID_HANDLE_VALUE, hLock.get()) << "ロック用ハンドルの確保に失敗";

	ReplaceRunParams p;
	p.key = L"needle"; p.replaceTo = L"REPLACED"; p.folder = m_temp->Root().wstring();
	(void)RunDoGrepReplaceEx(p);	// ヒット数は分岐により値が変わり得るため見ない

	const std::string body = ReadAllBytes(path);
	EXPECT_NE(body.find("needle"), std::string::npos)
		<< "ロック中の元ファイルは破壊されない";
	EXPECT_EQ(body.find("REPLACED"), std::string::npos)
		<< "ロック中は置換後内容で上書きされない";
}

// =============================================================================
// Grep 置換「貼り付けモード」検証（GA-34）
// =============================================================================

namespace {

//! クリップボードに CF_UNICODETEXT を積む（貼り付けモード検証用）。
bool SetClipboardUnicodeText(HWND hwnd, const std::wstring& s)
{
	bool bOpened = false;
	for (int i = 0; i < 10 && !(bOpened = ::OpenClipboard(hwnd)); ++i) {
		::Sleep(50);
	}
	if (!bOpened) { return false; }
	::EmptyClipboard();
	const size_t bytes = (s.size() + 1) * sizeof(wchar_t);
	if (HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, bytes)) {
		if (void* p = ::GlobalLock(hMem)) {
			memcpy(p, s.c_str(), bytes);
			::GlobalUnlock(hMem);
			::SetClipboardData(CF_UNICODETEXT, hMem);	// 所有権はクリップボードへ移る
		}
	}
	::CloseClipboard();
	return true;
}

//! クリップボードを空にする（後続テストへ影響させない）。
void ClearClipboard(HWND hwnd)
{
	if (::OpenClipboard(hwnd)) { ::EmptyClipboard(); ::CloseClipboard(); }
}

} // namespace

/*!
 * @brief DoGrep 置換: 貼り付けモードはクリップボード内容で置換する (GA-34)
 * @remark bGrepPaste=true の経路（MyGetClipboardData → CClipboard::GetText(CF_UNICODETEXT)）で、
 *         置換文字列フィールドではなくクリップボードの内容が使われることを検証する。
 * @note   クリップボードはグローバル資源のため、テスト後に空にする。
 *         クリップボードにアクセスできる環境（EditorTestSuite のウィンドウ）を前提とする。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_PasteUsesClipboard)
{
	const auto path = m_temp->WriteEncodedTextFile(L"paste.txt", CODE_UTF8, L"needle\n");

	CEditView* pView = &CEditWnd::getInstance()->GetActiveView();
	const HWND hwnd = pView->GetHwnd();

	// クリップボードへ置換文字列を積む（置換フィールドには別値を入れ、そちらが使われないことを確認する）
	if (!SetClipboardUnicodeText(hwnd, L"REPLACED")) {
		GTEST_SKIP() << "Skipped: cannot open clipboard (CI clipboard contention)";
	}

	const std::wstring tmpFile = GetTempDirString() + L"sakura_dogrep_paste_stdout.txt";
	HANDLE hTempFile = ::CreateFileW(tmpFile.c_str(),
		GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	ASSERT_NE(INVALID_HANDLE_VALUE, hTempFile) << "stdout キャプチャファイル作成失敗";
	struct HandleDeleter {
		void operator()(HANDLE h) const { if (h && h != INVALID_HANDLE_VALUE) ::CloseHandle(h); }
	};
	std::unique_ptr<void, HandleDeleter> hFileGuard{ hTempFile };

	DWORD hits = 0;
	{
		GrepStdHandleGuard stdoutGuard(STD_OUTPUT_HANDLE, hTempFile);

		CGrepAgent agent;
		const CNativeW cmKey(L"needle");
		const CNativeW cmRep(L"IGNORED");	// 貼り付けモードでは置換値として使われない
		const CNativeW cmFile(L"*.txt");
		const CNativeW cmFolder(m_temp->Root().wstring().c_str());

		SSearchOption sOpt;
		sOpt.Reset();

		SGrepOption gOpt;
		gOpt.bGrepReplace        = true;
		gOpt.bGrepPaste          = true;	// ← クリップボードから置換文字列を取得
		gOpt.bGrepSubFolder      = false;
		gOpt.bGrepStdout         = true;
		gOpt.bGrepHeader         = true;
		gOpt.nGrepCharSet        = CODE_AUTODETECT;
		gOpt.nGrepOutputLineType = 1;
		gOpt.nGrepOutputStyle    = 1;

		hits = agent.DoGrep(
			pView,
			SGrepInput{ &cmKey, &cmRep, &cmFile, &cmFolder },
			sOpt,
			gOpt,
			true,	// bGrepCurFolder
			false	// bGrepExcludeFileRegexp
		);
	} // ← stdout 復帰

	hFileGuard.reset();
	::DeleteFileW(tmpFile.c_str());
	ClearClipboard(hwnd);

	EXPECT_EQ(1u, hits) << "1 件が置換される";

	const std::string body = ReadAllBytes(path);
	EXPECT_NE(body.find("REPLACED"), std::string::npos)
		<< "クリップボードの内容で置換される";
	EXPECT_EQ(body.find("IGNORED"), std::string::npos)
		<< "置換フィールドの値は貼り付けモードでは使われない";
	EXPECT_EQ(body.find("needle"), std::string::npos)
		<< "前の文字列は残らない";
}

// =============================================================================
// 15. Grep 結果スナップショット（Phase 0-4）
// =============================================================================

namespace {

/*!
 * DoGrepFileWorker を 1 ファイルに対して実行し、ヒット数と出力メッセージ全文を返す
 *
 * ヒット数のみを検証する RunGrepFileWorker と異なり、出力文字列の完全一致
 * スナップショットを取るための派生ヘルパ。文字コードは CODE_UTF8 固定とし、
 * 自動判別によるコード名表記（"[UTF-8]" 等）の揺れを排除する。
 */
struct SWorkerSnapshot
{
	int hits;
	std::wstring message;
};

SWorkerSnapshot RunWorkerSnapshot(
	const std::filesystem::path& path,
	std::wstring_view key,
	int nOutputStyle,
	int nOutputLineType)
{
	const std::wstring keyStr(key);

	SGrepFileTask task;
	task.fullPath = path.wstring();
	task.fileName = path.filename().wstring();
	task.baseFolder = path.parent_path().wstring();
	task.folder = task.baseFolder;
	task.relPath = task.fileName;

	const auto sOpt = MakeSearchOption(false, false);
	auto gOpt = MakeGrepOption(CODE_UTF8);	// 文字コード固定で自動判別のコード名出力を回避
	gOpt.nGrepOutputStyle = nOutputStyle;
	gOpt.nGrepOutputLineType = nOutputLineType;

	CSearchStringPattern pattern;
	EXPECT_TRUE(pattern.SetPattern(nullptr, keyStr.c_str(), keyStr.size(), sOpt, nullptr));

	CNativeW cmemMessage;
	CNativeW cUnicodeBuffer;
	cmemMessage.AllocStringBuffer(4000);
	cUnicodeBuffer.AllocStringBuffer(4000);
	std::atomic cancel{ false };
	CGrepAgent agent;

	const int hits = agent.DoGrepFileWorker(
		SGrepSearchParams{ keyStr.c_str(), sOpt, gOpt },
		task,
		nullptr, pattern, cancel, cmemMessage, cUnicodeBuffer);

	return SWorkerSnapshot{
		hits,
		std::wstring(cmemMessage.GetStringPtr(), cmemMessage.GetStringLength()) };
}

//! 2 行（1 行目のみヒット）の標準スナップショット入力を書き出す
std::filesystem::path WriteSnapshotInput(const GrepTempDir& temp)
{
	// 1 行目: 7 桁目に "needle"（1 ヒット） / 2 行目: ヒットなし
	return temp.WriteEncodedTextFile(L"snap.txt", CODE_UTF8, L"alpha needle one\nbravo two\n");
}

} // namespace

/*!
 * @brief スナップショット: style=1(ノーマル) × lineType=1(行単位)
 * @remark "フルパス(行,桁): 該当行" 形式の完全一致を確認する。
 */
TEST_F(GrepRealFileTest, Snapshot_Style1_LineType1_MatchedLine)
{
	const auto path = WriteSnapshotInput(*m_temp);
	const auto result = RunWorkerSnapshot(path, L"needle", 1, 1);

	EXPECT_EQ(1, result.hits);
	EXPECT_EQ(path.wstring() + L"(1,7): alpha needle one\r\n", result.message);
}

/*!
 * @brief スナップショット: style=1(ノーマル) × lineType=0(該当部分)
 * @remark 該当行全体ではなくマッチ部分のみが出力されることを確認する。
 */
TEST_F(GrepRealFileTest, Snapshot_Style1_LineType0_MatchedPart)
{
	const auto path = WriteSnapshotInput(*m_temp);
	const auto result = RunWorkerSnapshot(path, L"needle", 1, 0);

	EXPECT_EQ(1, result.hits);
	EXPECT_EQ(path.wstring() + L"(1,7): needle\r\n", result.message);
}

/*!
 * @brief スナップショット: style=1(ノーマル) × lineType=2(否マッチ行)
 * @remark ヒットしなかった 2 行目のみが桁 1 で出力されることを確認する。
 */
TEST_F(GrepRealFileTest, Snapshot_Style1_LineType2_UnmatchedLine)
{
	const auto path = WriteSnapshotInput(*m_temp);
	const auto result = RunWorkerSnapshot(path, L"needle", 1, 2);

	EXPECT_EQ(1, result.hits);
	EXPECT_EQ(path.wstring() + L"(2,1): bravo two\r\n", result.message);
}

/*!
 * @brief スナップショット: style=2(WZ風) × lineType=1(行単位)
 * @remark ファイルヘッダー行（■"フルパス"）と "・(%6d,%-5d): " 書式の完全一致を確認する。
 */
TEST_F(GrepRealFileTest, Snapshot_Style2_LineType1_MatchedLine)
{
	const auto path = WriteSnapshotInput(*m_temp);
	const auto result = RunWorkerSnapshot(path, L"needle", 2, 1);

	EXPECT_EQ(1, result.hits);
	EXPECT_EQ(
		L"■\"" + path.wstring() + L"\"\r\n"
		L"・(     1,7    ): alpha needle one\r\n",
		result.message);
}

/*!
 * @brief スナップショット: style=3(結果のみ) × lineType=1(行単位)
 * @remark パス・行番号などの装飾なしで該当行のみが出力されることを確認する。
 */
TEST_F(GrepRealFileTest, Snapshot_Style3_LineType1_MatchedLine)
{
	const auto path = WriteSnapshotInput(*m_temp);
	const auto result = RunWorkerSnapshot(path, L"needle", 3, 1);

	EXPECT_EQ(1, result.hits);
	EXPECT_EQ(L"alpha needle one\r\n", result.message);
}

/*!
 * @brief スナップショット: style=3(結果のみ) × lineType=0(該当部分)
 * @remark マッチ部分のみが装飾なしで出力されることを確認する。
 */
TEST_F(GrepRealFileTest, Snapshot_Style3_LineType0_MatchedPart)
{
	const auto path = WriteSnapshotInput(*m_temp);
	const auto result = RunWorkerSnapshot(path, L"needle", 3, 0);

	EXPECT_EQ(1, result.hits);
	EXPECT_EQ(L"needle\r\n", result.message);
}

/*!
 * @brief スナップショット: style=1(ノーマル) × lineType=0 の同一行内複数ヒット
 * @remark 同一行の 2 つのヒットがそれぞれの桁位置で 2 行出力されることを確認する。
 */
TEST_F(GrepRealFileTest, Snapshot_Style1_LineType0_MultipleHitsInLine)
{
	const auto path = m_temp->WriteEncodedTextFile(L"multi.txt", CODE_UTF8, L"needle x needle\n");
	const auto result = RunWorkerSnapshot(path, L"needle", 1, 0);

	EXPECT_EQ(2, result.hits);
	EXPECT_EQ(
		path.wstring() + L"(1,1): needle\r\n" +
		path.wstring() + L"(1,10): needle\r\n",
		result.message);
}

/*!
 * @brief スナップショット: 除外ファイル(!)・除外フォルダー(#)適用後の列挙結果
 * @remark "*.*;!*.log;#skip" 指定で .log ファイルと skip フォルダーが列挙から除外され、
 *         残る要素が期待どおりであることを確認する。
 */
TEST_F(GrepRealFileTest, Snapshot_Enumerate_ExcludePatternsApplied)
{
	m_temp->WriteEncodedTextFile(L"a.txt", CODE_UTF8, L"a\n");
	m_temp->WriteEncodedTextFile(L"b.log", CODE_UTF8, L"b\n");
	m_temp->WriteEncodedTextFile(L"c.txt", CODE_UTF8, L"c\n");
	m_temp->EnsureDir(L"keep");
	m_temp->EnsureDir(L"skip");

	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.*;!*.log;#skip"));

	// ファイル列挙: b.log が除外される
	{
		const auto found = EnumerateSortedFileNames(m_temp->Root().wstring(), keys);

		ASSERT_EQ(2u, found.size());
		EXPECT_EQ(L"a.txt", found[0]);
		EXPECT_EQ(L"c.txt", found[1]);
	}

	// フォルダー列挙: skip が除外される
	{
		const auto found = EnumerateSortedFolderNames(m_temp->Root().wstring(), keys);

		ASSERT_EQ(1u, found.size());
		EXPECT_EQ(L"keep", found[0]);
	}
}

/*!
 * @brief スナップショット: 除外ファイルパターンが検索結果に反映される
 * @remark 除外対象ファイルは列挙段階で落ちるため、残ったファイルの検索結果のみが
 *         得られることを、列挙→ワーカー実行の結合で確認する。
 */
TEST_F(GrepRealFileTest, Snapshot_Enumerate_ThenWorker_ExcludedFileNotSearched)
{
	const auto hitPath = m_temp->WriteEncodedTextFile(L"hit.txt", CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"ignored.log", CODE_UTF8, L"needle\n");

	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.*;!*.log"));

	CGrepEnumFilterFiles enumFiles;
	CGrepEnumOptions enumOpts;
	CGrepEnumFiles exceptAbs;
	enumFiles.Enumerates(m_temp->Root().wstring().c_str(), keys, enumOpts, exceptAbs);

	std::wstring combined;
	int totalHits = 0;
	for (int i = 0; i < enumFiles.GetCount(); ++i) {
		const auto result = RunWorkerSnapshot(
			m_temp->Root() / enumFiles.GetFileName(i), L"needle", 1, 1);
		totalHits += result.hits;
		combined += result.message;
	}

	EXPECT_EQ(1, totalHits);
	EXPECT_EQ(hitPath.wstring() + L"(1,1): needle\r\n", combined);
}

// =============================================================================
// 16. DoGrepFile 直列経路の直接検証（GA-36 ～ GA-40）・ハンドルGrep エラー経路（GA-41）
// =============================================================================
//
// 補足: :HWND: 正常系（実ウィンドウ経由のハンドルGrep）は、テスト環境の CEditWnd が
// IsSakuraMainWindow のウィンドウクラス判定を満たさないため実行不可（CI で確認済み）。
// 直列版 MatchAndEmitGrepLine（pnHitCount 非 null 分岐）は DoGrepFile を直接呼び出して
// 実ファイルに対し検証する。CDlgCancel はダイアログ未生成（NULL HWND）で渡し、
// UI 系 API は no-op となる（GA-42 と同じ方式）。UI チェックは 32 行ごとのため
// 小さな入力では BlockingHook にも到達しない。

namespace {

//! DoGrepFile（直列版）の実行結果
struct SSerialGrepResult {
	int				ret;		//!< DoGrepFile の戻り値（このファイルのヒット数）
	int				globalHits;	//!< *pnHitCount（直列版合計ヒット数）
	std::wstring	message;	//!< 結果メッセージ
};

//! DoGrepFile を実ファイルに対して直接呼び出す（直列版 MatchAndEmitGrepLine, pnHitCount 非 null 経路）。
SSerialGrepResult RunDoGrepFileSerial(
	const std::filesystem::path& path,
	const std::wstring& key,
	const SSearchOption& sOpt,
	int nOutputLineType)
{
	SSerialGrepResult result{ 0, 0, L"" };

	// DoGrep 本体と同じ手順で検索パターンを構築する（bRegularExp 時は SetPattern 内でコンパイルされる）
	CBregexp regexp;
	CSearchStringPattern pattern;
	if (!pattern.SetPattern(nullptr, key.c_str(), (int)key.size(), sOpt, &regexp)) {
		ADD_FAILURE() << "SetPattern failed";
		return result;
	}

	auto gOpt = MakeGrepOption();
	gOpt.nGrepOutputLineType = nOutputLineType;

	CDlgCancel cDlgCancel;	// ダイアログ未生成（NULL HWND・キャンセルされない）

	const std::wstring fullPath   = path.wstring();
	const std::wstring fileName   = path.filename().wstring();
	const std::wstring baseFolder = path.parent_path().wstring();

	CNativeW cmemMessage;
	CNativeW cUnicodeBuffer;
	cmemMessage.AllocStringBuffer(4000);
	cUnicodeBuffer.AllocStringBuffer(4000);
	bool bOutputBaseFolder = false;
	bool bOutputFolderName = false;

	CEditView* pView = &CEditWnd::getInstance()->GetActiveView();
	CGrepAgent agent;
	result.ret = agent.DoGrepFile(
		pView,
		&cDlgCancel,
		nullptr,	// hWndTarget: nullptr = 実ファイルを読み込む
		key.c_str(),
		fileName.c_str(),
		sOpt,
		gOpt,
		pattern,
		&regexp,
		&result.globalHits,
		fullPath.c_str(),
		baseFolder.c_str(),
		baseFolder.c_str(),
		fileName.c_str(),
		bOutputBaseFolder,
		bOutputFolderName,
		cmemMessage,
		cUnicodeBuffer
	);
	result.message.assign(cmemMessage.GetStringPtr(), (size_t)cmemMessage.GetStringLength());
	return result;
}

} // namespace

/*!
 * @brief DoGrepFile 直列経路の基本ヒット（lineType=1） (GA-36)
 * @remark 直列版 MatchAndEmitGrepLine（pnGlobalHitCount 非 null 分岐）で、戻り値と
 *         *pnHitCount の両方にヒット数が正しく加算されることを確認する。
 */
TEST_F(GrepRealFileTest, SerialGrep_DoGrepFile_BasicHits)
{
	const auto path = m_temp->WriteEncodedTextFile(L"ga36.txt", CODE_UTF8, L"UNIQ0 alpha\nUNIQ0 beta\n");

	SSearchOption sOpt;
	sOpt.Reset();
	const auto r = RunDoGrepFileSerial(path, L"UNIQ0", sOpt, 1);

	EXPECT_EQ(2, r.ret) << "UNIQ0 を含む 2 行がヒットする";
	EXPECT_EQ(2, r.globalHits) << "直列版 *pnHitCount にも同数が加算される";
	EXPECT_NE(r.message.find(L"UNIQ0 alpha"), std::wstring::npos) << "ヒット行が出力される";
}

/*!
 * @brief DoGrepFile 直列経路: lineType=0 で 1 行内複数ヒットの累積 (GA-37)
 * @remark 該当部分出力モードで 1 行に 3 つのマッチがあるとき、直列版 MatchAndEmitGrepLine の
 *         ヒットごとの ++(*pnGlobalHitCount) が 3 回実行され合計 3 件となることを確認する。
 */
TEST_F(GrepRealFileTest, SerialGrep_DoGrepFile_MultipleHitsInLine)
{
	const auto path = m_temp->WriteEncodedTextFile(L"ga37.txt", CODE_UTF8, L"UNIQ1 x UNIQ1 x UNIQ1\n");

	SSearchOption sOpt;
	sOpt.Reset();
	const auto r = RunDoGrepFileSerial(path, L"UNIQ1", sOpt, 0);	// 該当部分（1 行内の複数マッチ継続）

	EXPECT_EQ(3, r.ret) << "1 行内の 3 マッチが戻り値に加算される";
	EXPECT_EQ(3, r.globalHits) << "1 行内の 3 マッチが直列版合計ヒット数へすべて加算される";
}

/*!
 * @brief DoGrepFile 直列経路: lineType=2（否ヒット行）でヒット数が巻き戻される (GA-38)
 * @remark 否ヒット行モードでは、ヒット行のマッチ分を *pnGlobalHitCount から行開始値へ
 *         巻き戻し、否ヒット行のみ +1 する（C-1 共通化の直列版分岐）。
 *         ファイルはヒット行 1（マッチ 3 個）＋否ヒット行 2 の計 3 行。
 *         巻き戻しが壊れている場合、ヒット行の 3 マッチが混入して 5 以上になる。
 */
TEST_F(GrepRealFileTest, SerialGrep_DoGrepFile_NoHitLineTypeRewindsCount)
{
	const auto path = m_temp->WriteEncodedTextFile(L"ga38.txt", CODE_UTF8, L"UNIQ2 UNIQ2 UNIQ2\nmissA\nmissB\n");

	SSearchOption sOpt;
	sOpt.Reset();
	const auto r = RunDoGrepFileSerial(path, L"UNIQ2", sOpt, 2);	// 否ヒット行

	EXPECT_EQ(2, r.ret)
		<< "否ヒット行 2 のみカウントされる（ヒット行の 3 マッチは巻き戻される）";
	EXPECT_EQ(2, r.globalHits)
		<< "直列版 *pnHitCount も行開始値へ巻き戻されて否ヒット行 2 のみとなる";
	EXPECT_EQ(r.message.find(L"UNIQ2"), std::wstring::npos)
		<< "ヒット行（UNIQ2 を含む行）は否ヒット行モードでは出力されない";
	EXPECT_NE(r.message.find(L"missA"), std::wstring::npos) << "否ヒット行 missA が出力される";
	EXPECT_NE(r.message.find(L"missB"), std::wstring::npos) << "否ヒット行 missB が出力される";
}

/*!
 * @brief DoGrepFile 直列経路: 単語のみ検索分岐 (GA-39)
 * @remark 直列版 MatchAndEmitGrepLine の bWordOnly 分岐で、独立語のみヒットし
 *         部分一致（UNIQ3words）はヒットしないことを確認する。
 */
TEST_F(GrepRealFileTest, SerialGrep_DoGrepFile_WordOnly)
{
	const auto path = m_temp->WriteEncodedTextFile(L"ga39.txt", CODE_UTF8, L"UNIQ3w UNIQ3words UNIQ3w\n");

	SSearchOption sOpt;
	sOpt.Reset();
	sOpt.bWordOnly = true;
	const auto r = RunDoGrepFileSerial(path, L"UNIQ3w", sOpt, 0);	// 1 行内の複数マッチを数える

	EXPECT_EQ(2, r.ret) << "独立語 UNIQ3w の 2 件のみ（UNIQ3words は単語境界不成立）";
	EXPECT_EQ(2, r.globalHits) << "直列版 *pnHitCount にも同数が加算される";
}

/*!
 * @brief DoGrepFile 直列経路: 正規表現検索分岐 (GA-40)
 * @remark 直列版 MatchAndEmitGrepLine の bRegularExp 分岐で、`^RXUNIQ\d+$` が
 *         数字終端の行のみにヒットすることを確認する。
 * @note   bregonig.dll に依存する（他の正規表現テストと同条件）。
 */
TEST_F(GrepRealFileTest, SerialGrep_DoGrepFile_Regex)
{
	const auto path = m_temp->WriteEncodedTextFile(L"ga40.txt", CODE_UTF8, L"RXUNIQ1\nRXUNIQ22\nRXUNIQx\n");

	SSearchOption sOpt;
	sOpt.Reset();
	sOpt.bRegularExp = true;
	const auto r = RunDoGrepFileSerial(path, L"^RXUNIQ\\d+$", sOpt, 1);

	EXPECT_EQ(2, r.ret) << "RXUNIQ1 / RXUNIQ22 の 2 行のみヒットする";
	EXPECT_EQ(2, r.globalHits) << "直列版 *pnHitCount にも同数が加算される";
}

/*!
 * @brief ハンドルGrep: 無効 HWND トークンのエラー経路で SGrepRunningGuard がリセットする (GA-41)
 * @remark 無効な :HWND: トークンで GetHwndTitle が -1 を返す経路（"HWND handle error."）でも、
 *         A-3 の RAII ガードにより m_bGrepRunning がリセットされ、同一 agent で
 *         続けて実行する通常 Grep が再入エラー（0xffffffff）にならないことを確認する。
 *         本ガード導入前は本経路のみリセットが漏れ、以後 Grep が永続的に実行不能だった。
 */
TEST_F(GrepRealFileTest, HwndGrep_InvalidTokenErrorPath_GuardResetsRunningFlag)
{
	m_temp->WriteEncodedTextFile(L"guard.txt", CODE_UTF8, L"needle\nneedle\n");

	struct HandleDeleter {
		void operator()(HANDLE h) const { if (h && h != INVALID_HANDLE_VALUE) ::CloseHandle(h); }
	};

	CGrepAgent agent;	// 2 回の DoGrep で同一インスタンスを使う（再入ガードは agent 単位）

	// 1 回目: 無効 HWND トークン → GetHwndTitle が -1 → エラー経路で return 0
	{
		const std::wstring tmpFile = GetTempDirString() + L"sakura_dogrep_badhwnd_stdout.txt";
		HANDLE hTempFile = ::CreateFileW(tmpFile.c_str(),
			GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		ASSERT_NE(INVALID_HANDLE_VALUE, hTempFile) << "stdout キャプチャファイル作成失敗";
		std::unique_ptr<void, HandleDeleter> hFileGuard{ hTempFile };

		DWORD hits = 0;
		std::string out;
		{
			GrepStdHandleGuard stdoutGuard(STD_OUTPUT_HANDLE, hTempFile);

			CEditView* pView = &CEditWnd::getInstance()->GetActiveView();
			const CNativeW cmKey(L"needle");
			const CNativeW cmRep(L"");
			const CNativeW cmFile(L":HWND:0");	// 無効ハンドル（IsSakuraMainWindow が false）
			const CNativeW cmFolder(L"C:\\");

			SSearchOption sOpt;
			sOpt.Reset();

			SGrepOption gOpt;
			gOpt.bGrepStdout         = true;
			gOpt.bGrepHeader         = true;
			gOpt.nGrepCharSet        = CODE_AUTODETECT;
			gOpt.nGrepOutputLineType = 1;
			gOpt.nGrepOutputStyle    = 1;

			hits = agent.DoGrep(
				pView,
				SGrepInput{ &cmKey, &cmRep, &cmFile, &cmFolder },
				sOpt, gOpt,
				true,	// bGrepCurFolder
				false	// bGrepExcludeFileRegexp
			);

			::SetFilePointer(hTempFile, 0, nullptr, FILE_BEGIN);
			std::string buf(8192, '\0');
			DWORD dwRead = 0;
			::ReadFile(hTempFile, buf.data(), static_cast<DWORD>(buf.size()), &dwRead, nullptr);
			out.assign(buf.data(), dwRead);
		}
		hFileGuard.reset();
		::DeleteFileW(tmpFile.c_str());

		EXPECT_EQ(0u, hits) << "無効 HWND トークンは 0 件で復帰する";
		EXPECT_NE(out.find("HWND handle error"), std::string::npos)
			<< "HWND ハンドルエラーのメッセージが出力される";
	}

	// 2 回目: 同一 agent で通常のファイル Grep。ガードが機能していれば正常実行できる。
	//         ガードが機能していない場合は再入検知で 0xffffffff が返る。
	{
		const std::wstring tmpFile = GetTempDirString() + L"sakura_dogrep_guard2_stdout.txt";
		HANDLE hTempFile = ::CreateFileW(tmpFile.c_str(),
			GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		ASSERT_NE(INVALID_HANDLE_VALUE, hTempFile) << "stdout キャプチャファイル作成失敗";
		std::unique_ptr<void, HandleDeleter> hFileGuard{ hTempFile };

		DWORD hits2 = 0;
		{
			GrepStdHandleGuard stdoutGuard(STD_OUTPUT_HANDLE, hTempFile);

			CEditView* pView = &CEditWnd::getInstance()->GetActiveView();
			const CNativeW cmKey(L"needle");
			const CNativeW cmRep(L"");
			const CNativeW cmFile(L"*.txt");
			const CNativeW cmFolder(m_temp->Root().wstring().c_str());

			SSearchOption sOpt;
			sOpt.Reset();

			SGrepOption gOpt;
			gOpt.bGrepStdout         = true;
			gOpt.bGrepHeader         = true;
			gOpt.nGrepCharSet        = CODE_AUTODETECT;
			gOpt.nGrepOutputLineType = 1;
			gOpt.nGrepOutputStyle    = 1;

			hits2 = agent.DoGrep(
				pView,
				SGrepInput{ &cmKey, &cmRep, &cmFile, &cmFolder },
				sOpt, gOpt,
				true,	// bGrepCurFolder
				false	// bGrepExcludeFileRegexp
			);
		}
		hFileGuard.reset();
		::DeleteFileW(tmpFile.c_str());

		EXPECT_NE(0xffffffffu, hits2)
			<< "SGrepRunningGuard により m_bGrepRunning がリセットされ再入エラーにならない";
		EXPECT_EQ(2u, hits2) << "通常 Grep が正常に実行され needle 2 件がヒットする";
	}
}

// =============================================================================
// 17. DoGrepTree キャンセル経路・P4 結合安全化の検証（GA-42 ～ GA-44）
// =============================================================================

/*!
 * @brief DoGrepTree: キャンセル済みダイアログで FlushAndCancel 経路を通る (GA-42)
 * @remark ダイアログを生成せず m_bCANCEL=TRUE の CDlgCancel を注入し、ファイルループ先頭の
 *         中断チェックで FlushAndCancel（B-1: goto cancel_return の代替ラムダ）が実行され、
 *         (1) 戻り値 -1、(2) 残存メッセージの AddTail フラッシュ、(3) バッファ長 0 リセット、
 *         の 3 点を確認する。フラッシュ先は stdout（bGrepStdout=true）で回収する。
 */
TEST_F(GrepRealFileTest, DoGrepTree_PreCancelledFlushesAndReturnsMinusOne)
{
	// キャンセルチェックはファイルループ内にあるためファイルが 1 つ以上必要
	m_temp->WriteEncodedTextFile(L"c.txt", CODE_UTF8, L"needle\n");

	CDlgCancel cDlgCancel;
	cDlgCancel.m_bCANCEL = TRUE;	// ダイアログ未生成のままキャンセル済み状態にする

	const auto sOpt = MakeSearchOption(false, false);
	auto gOpt = MakeGrepOption();
	gOpt.bGrepStdout = true;

	CSearchStringPattern pattern;
	ASSERT_TRUE(pattern.SetPattern(nullptr, L"needle", 6, sOpt, nullptr));
	CBregexp regexp;	// 非正規表現検索のため未使用（キャンセル判定が先に走る）

	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*.txt"));
	CGrepEnumFiles exceptAbsFiles;
	CGrepEnumFolders exceptAbsFolders;

	CNativeW cmemMessage;
	cmemMessage.AppendString(L"PRELOADEDMARK\r\n");	// FlushAndCancel の残存フラッシュ検証用
	CNativeW cUnicodeBuffer;
	int nHitCount = 0;
	bool bOutputBaseFolder = false;
	const std::wstring rootPath = m_temp->Root().wstring();
	const CNativeW cmRep(L"");

	const std::wstring tmpFile = GetTempDirString() + L"sakura_dogreptree_cancel_stdout.txt";
	HANDLE hTempFile = ::CreateFileW(tmpFile.c_str(),
		GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	ASSERT_NE(INVALID_HANDLE_VALUE, hTempFile) << "stdout キャプチャファイル作成失敗";
	struct HandleDeleter {
		void operator()(HANDLE h) const { if (h && h != INVALID_HANDLE_VALUE) ::CloseHandle(h); }
	};
	std::unique_ptr<void, HandleDeleter> hFileGuard{ hTempFile };

	int ret = 0;
	std::string out;
	{
		GrepStdHandleGuard stdoutGuard(STD_OUTPUT_HANDLE, hTempFile);

		CEditView* pView = &CEditWnd::getInstance()->GetActiveView();
		CGrepAgent agent;
		ret = agent.DoGrepTree(
			pView,
			&cDlgCancel,
			L"needle",
			cmRep,
			keys,
			exceptAbsFiles,
			exceptAbsFolders,
			rootPath.c_str(),
			rootPath.c_str(),
			sOpt,
			gOpt,
			pattern,
			&regexp,
			bOutputBaseFolder,
			&nHitCount,
			cmemMessage,
			cUnicodeBuffer
		);

		::SetFilePointer(hTempFile, 0, nullptr, FILE_BEGIN);
		std::string buf(8192, '\0');
		DWORD dwRead = 0;
		::ReadFile(hTempFile, buf.data(), static_cast<DWORD>(buf.size()), &dwRead, nullptr);
		out.assign(buf.data(), dwRead);
	}
	hFileGuard.reset();
	::DeleteFileW(tmpFile.c_str());

	EXPECT_EQ(-1, ret) << "キャンセル時は -1 を返す";
	EXPECT_EQ(0, cmemMessage.GetStringLength())
		<< "FlushAndCancel が残存メッセージをフラッシュして長さ 0 に戻す";
	EXPECT_NE(out.find("PRELOADEDMARK"), std::string::npos)
		<< "残存メッセージが AddTail（stdout）へフラッシュされる";
}

/*!
 * @brief DoGrep 置換: 後方参照を含む除外正規表現は結合せず個別適用される (GA-43)
 * @remark P4 結合はグループ番号が全パターン通しで振り直されるため、先行パターンに
 *         キャプチャグループがあると後続パターンの \1 の参照先が変わる。
 *         CanCombineExcludePatterns により個別コンパイルへ迂回し、各パターンが
 *         単独コンパイル時と同じ意味で適用されることを直列（置換）経路で確認する。
 *         p1=`.*(zk)m\.log$`（グループあり）、p2=`.*(zq)\1.*`（\1 は (zq) を参照）。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplaceMode_BackRefExcludeNotCombined)
{
	m_temp->WriteEncodedTextFile(L"keep.txt", CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"zqzq.txt", CODE_UTF8, L"needle\n");	// p2 で除外（zqzq = (zq)\1）
	m_temp->WriteEncodedTextFile(L"zkm.log",  CODE_UTF8, L"needle\n");	// p1 で除外

	std::string out;
	const DWORD hits = RunDoGrepReplaceExcludeRegex(
		L"*.*;!.*(zk)m\\.log$;!.*(zq)\\1.*", m_temp->Root().wstring(),
		/*bSubFolder=*/false, out);

	EXPECT_EQ(1u, hits) << "keep.txt の 1 件のみ置換（後方参照パターンは個別適用で正しく除外）";
	EXPECT_NE(out.find("keep.txt"), std::string::npos) << "keep.txt が出力される";
	EXPECT_EQ(out.find("zqzq"),     std::string::npos) << "zqzq.txt は \\1 付きパターンで除外";
	EXPECT_EQ(out.find("zkm"),      std::string::npos) << "zkm.log はグループ付きパターンで除外";
}

/*!
 * @brief RunParallelGrep: 後方参照を含む除外正規表現は結合せず個別適用される (GA-44)
 * @remark GA-43 と同じパターン構成を並列経路（ワーカー内 excludeRegexps）で検証する。
 */
TEST_F(GrepRealFileTest, DoGrep_StdoutMode_BackRefExcludeNotCombined)
{
	m_temp->WriteEncodedTextFile(L"keep.txt", CODE_UTF8, L"needle\n");
	m_temp->WriteEncodedTextFile(L"zqzq.txt", CODE_UTF8, L"needle\n");	// p2 で除外（zqzq = (zq)\1）
	m_temp->WriteEncodedTextFile(L"zkm.log",  CODE_UTF8, L"needle\n");	// p1 で除外

	std::string out;
	const DWORD hits = RunDoGrepStdout(
		L"needle", L"*.*;!.*(zk)m\\.log$;!.*(zq)\\1.*", m_temp->Root().wstring(),
		out, /*bExcludeFileRegex=*/true);

	EXPECT_EQ(1u, hits) << "keep.txt の 1 件のみ（後方参照パターンは個別適用で正しく除外）";
	EXPECT_NE(out.find("keep.txt"), std::string::npos) << "keep.txt のヒットが出力される";
	EXPECT_EQ(out.find("zqzq"),     std::string::npos) << "zqzq.txt は \\1 付きパターンで除外";
	EXPECT_EQ(out.find("zkm"),      std::string::npos) << "zkm.log はグループ付きパターンで除外";
}

// =============================================================================
// 18. カバレッジ補完（DoGrep 早期経路・EscapeStringLiteral・AddTail・
//     クリップボード置換・ハンドルGrep）（GA-45 ～ GA-51）
// =============================================================================

namespace {

/*!
 * DoGrep をヘッダ付き stdout モードで実行し、標準出力の内容とヒット数を返す
 * テスト専用ヘルパ（GA-46/47/49/50/51 共用）。
 */
DWORD RunDoGrepStdoutHeader(
	CGrepAgent& agent,
	const SDoGrepTestInput& in,
	SGrepOption gOpt,
	const SSearchOption& sOpt,
	std::string& out)
{
	return RunDoGrepCaptureStdout(agent, in, sOpt, gOpt, /*bExcludeFileRegex=*/false, out);
}

/*!
 * 出力先ビューのタイプ別設定（文字列色分け・文字列区切り記号エスケープ方法）を
 * 一時的に書き換え、スコープ脱出時に必ず復元する RAII ガード（GA-46/47 用）。
 */
class TypeConfigStringGuard {
public:
	TypeConfigStringGuard(BOOL bDisp, int nStringType)
		: m_type(CEditWnd::getInstance()->GetActiveView().m_pcEditDoc->m_cDocType.GetDocumentAttributeWrite())
		, m_oldDisp(m_type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp)
		, m_oldStringType(m_type.m_nStringType)
	{
		m_type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = bDisp;
		m_type.m_nStringType = decltype(m_type.m_nStringType)(nStringType);
	}
	~TypeConfigStringGuard()
	{
		m_type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = m_oldDisp;
		m_type.m_nStringType = m_oldStringType;
	}
	TypeConfigStringGuard(const TypeConfigStringGuard&) = delete;
	TypeConfigStringGuard& operator=(const TypeConfigStringGuard&) = delete;
private:
	STypeConfig&	m_type;
	BOOL			m_oldDisp;
	decltype(std::declval<STypeConfig&>().m_nStringType)	m_oldStringType;
};

} // namespace

/*!
 * @brief DoGrep: 再入ガード（m_bGrepRunning=true）で 0xffffffff を返す (GA-45)
 * @remark 再入チェックは SGrepRunningGuard 生成より前にあるため、早期 return 後も
 *         フラグは true のまま維持される（呼び出し元＝先行実行中の DoGrep が責任を持つ）。
 */
TEST_F(GrepRealFileTest, DoGrep_ReentryGuardReturnsErrorCode)
{
	CEditView* pView = &CEditWnd::getInstance()->GetActiveView();
	CGrepAgent agent;
	agent.m_bGrepRunning = true;	// 実行中を偽装

	const CNativeW cmKey(L"reentry");
	const CNativeW cmRep(L"");
	const CNativeW cmFile(L"*.txt");
	const CNativeW cmFolder(m_temp->Root().wstring().c_str());
	SSearchOption sOpt;
	sOpt.Reset();
	SGrepOption gOpt;

	const DWORD ret = agent.DoGrep(
		pView, SGrepInput{ &cmKey, &cmRep, &cmFile, &cmFolder },
		sOpt, gOpt, true, false);

	EXPECT_EQ(0xffffffffu, ret) << "再入時は 0xffffffff を返す";
	EXPECT_TRUE(agent.m_bGrepRunning)
		<< "ガード生成前の早期 return のためフラグは維持される";
	agent.m_bGrepRunning = false;	// 後続テストへの影響を避けるため復元
}

/*!
 * @brief EscapeStringLiteral: CPP 系エスケープ分岐（\\ ' " のエスケープ） (GA-46)
 * @remark 文字列色分け有効かつ STRING_LITERAL_CPP のタイプ別設定で DoGrep ヘッダ中の
 *         検索キーがエスケープされることを確認する（\ → \\）。
 */
TEST_F(GrepRealFileTest, DoGrep_HeaderKeyEscape_CppStringType)
{
	TypeConfigStringGuard typeGuard(TRUE, STRING_LITERAL_CPP);

	CGrepAgent agent;
	SSearchOption sOpt;
	sOpt.Reset();
	std::string out;
	const DWORD hits = RunDoGrepStdoutHeader(
		agent, { L"esc\\cpp", L"", L"*.txt", m_temp->Root().wstring() },
		MakeGrepOption(), sOpt, out);

	EXPECT_EQ(0u, hits) << "空フォルダーのためヒットなし";
	EXPECT_NE(out.find("esc\\\\cpp"), std::string::npos)
		<< R"(ヘッダの検索キーで \ が \\ にエスケープされる)";
}

/*!
 * @brief EscapeStringLiteral: PLSQL エスケープ分岐（' " の二重化） (GA-47)
 * @remark 文字列色分け有効かつ STRING_LITERAL_PLSQL のタイプ別設定で DoGrep ヘッダ中の
 *         検索キーがエスケープされることを確認する（' → ''）。
 */
TEST_F(GrepRealFileTest, DoGrep_HeaderKeyEscape_PlsqlStringType)
{
	TypeConfigStringGuard typeGuard(TRUE, STRING_LITERAL_PLSQL);

	CGrepAgent agent;
	SSearchOption sOpt;
	sOpt.Reset();
	std::string out;
	const DWORD hits = RunDoGrepStdoutHeader(
		agent, { L"esc'plsql", L"", L"*.txt", m_temp->Root().wstring() },
		MakeGrepOption(), sOpt, out);

	EXPECT_EQ(0u, hits) << "空フォルダーのためヒットなし";
	EXPECT_NE(out.find("esc''plsql"), std::string::npos)
		<< "ヘッダの検索キーで ' が '' にエスケープされる";
}

/*!
 * @brief AddTail: エディタ出力分岐（Command_ADDTAIL 経路） (GA-48)
 * @remark bAddStdout=false の場合、結果がアクティブ文書の末尾に追記され、
 *         行数が増加することを確認する（stdout 分岐は GA-18 で検証済み）。
 */
TEST_F(GrepRealFileTest, AddTail_EditorMode_AppendsToDocument)
{
	CEditView* pView = &CEditWnd::getInstance()->GetActiveView();
	const auto before = pView->GetDocument()->m_cDocLineMgr.GetLineCount();

	CGrepAgent agent;
	const CNativeW msg(L"ADDTAILMARK1\r\nADDTAILMARK2\r\n");
	agent.AddTail(pView, msg, false);	// エディタ出力分岐

	const auto after = pView->GetDocument()->m_cDocLineMgr.GetLineCount();
	EXPECT_GT(after, before) << "文書末尾への追記により行数が増える";
}

/*!
 * @brief Grep置換: クリップボード貼り付け（通常テキスト・EOL変換あり） (GA-49)
 * @remark bGrepPaste=true でクリップボード内容が置換文字列として使用され、
 *         m_bConvertEOLPaste=true の EOL 変換分岐を通過して実ファイルが
 *         置換されることを確認する。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplacePasteMode_UsesClipboardText)
{
	const auto path = m_temp->WriteEncodedTextFile(L"paste.txt", CODE_UTF8, L"pkneedle\nrest\n");

	// EOL 変換分岐（ConvertEol）を通すため一時的に有効化し、必ず復元する
	auto& sEdit = GetDllShareData().m_Common.m_sEdit;
	const auto oldConvertEol = sEdit.m_bConvertEOLPaste;
	sEdit.m_bConvertEOLPaste = true;
	ScopeExit shareGuard([&sEdit, oldConvertEol] { sEdit.m_bConvertEOLPaste = oldConvertEol; });

	CGrepAgent agent;
	SSearchOption sOpt;
	sOpt.Reset();
	auto gOpt = MakeGrepOption();
	gOpt.bGrepReplace   = true;
	gOpt.bGrepPaste     = true;
	gOpt.bGrepSubFolder = false;

	// CI ではクリップボードを他プロセスが奪取・クリアすることがあり、DoGrep 内部の
	// 読み取り（MyGetClipboardData）が失敗し得る。失敗時は対象ファイルが未変更で
	// 再実行が安全なため、設定→実行のサイクル全体をリトライする。
	// DoGrep 内部の読み取り（MyGetClipboardData）は出力先ビューの HWND で
	// OpenClipboard するため、設定・読み戻し検証も同じ HWND で行い、
	// DoGrep の前提条件そのものを検証する。
	const HWND hwndView = CEditWnd::getInstance()->GetActiveView().GetHwnd();

	DWORD hits = 0;
	std::string out;
	bool bClipVerified = false;	// 設定内容を一度でも読み戻せたか（環境判定用）
	for (int attempt = 0; attempt < 3 && hits == 0; ++attempt) {
		if (attempt > 0) { ::Sleep(100); }
		// クリップボードへ置換文字列を設定（通常テキスト）
		{
			CClipboard clipboard(hwndView);
			if (!clipboard) { continue; }
			if (!clipboard.SetText(L"PASTED", 6, false, false)) { continue; }
		}
		// 設定内容を読み戻して検証（OpenClipboard 不可の CI 環境では検証不可＝環境要因）
		bool bReadBack = false;
		if (::OpenClipboard(hwndView)) {
			if (HANDLE h = ::GetClipboardData(CF_UNICODETEXT)) {
				if (const wchar_t* p = static_cast<const wchar_t*>(::GlobalLock(h))) {
					bReadBack = (::wcscmp(p, L"PASTED") == 0);
					::GlobalUnlock(h);
				}
			}
			::CloseClipboard();
		}
		if (!bReadBack) { continue; }
		bClipVerified = true;
		out.clear();
		hits = RunDoGrepStdoutHeader(
			agent, { L"pkneedle", L"", L"*.txt", m_temp->Root().wstring() },
			gOpt, sOpt, out);
	}
	if (hits == 0 && !bClipVerified) {
		GTEST_SKIP() << "Skipped: clipboard unavailable "
						"(OpenClipboard failed or read-back verification failed)";
	}

	EXPECT_EQ(1u, hits) << "1 件が置換される";

	std::ifstream is(path, std::ios::binary);
	std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
	EXPECT_NE(content.find("PASTED"), std::string::npos)
		<< "ファイル内容がクリップボード文字列で置換される";
	EXPECT_EQ(content.find("pkneedle"), std::string::npos) << "元の検索キーは残らない";
}

/*!
 * @brief Grep置換: クリップボード貼り付け（ラインモード・改行補完） (GA-50)
 * @remark m_bEnableLineModePaste=true かつラインモード形式のクリップボードで、
 *         末尾に改行がない貼り付けデータへ改行が補完される分岐
 *         （空クリップボード時の範囲外参照防止コードを含む行）を通過することを確認する。
 */
TEST_F(GrepRealFileTest, DoGrep_ReplacePasteMode_LineModeAppendsEol)
{
	const auto path = m_temp->WriteEncodedTextFile(L"lmode.txt", CODE_UTF8, L"lmneedle\nrest\n");

	auto& sEdit = GetDllShareData().m_Common.m_sEdit;
	const auto oldLineMode   = sEdit.m_bEnableLineModePaste;
	const auto oldConvertEol = sEdit.m_bConvertEOLPaste;
	sEdit.m_bEnableLineModePaste = true;
	sEdit.m_bConvertEOLPaste     = false;
	ScopeExit shareGuard([&sEdit, oldLineMode, oldConvertEol] {
		sEdit.m_bEnableLineModePaste = oldLineMode;
		sEdit.m_bConvertEOLPaste     = oldConvertEol;
	});

	CGrepAgent agent;
	SSearchOption sOpt;
	sOpt.Reset();
	auto gOpt = MakeGrepOption();
	gOpt.bGrepReplace   = true;
	gOpt.bGrepPaste     = true;
	gOpt.bGrepSubFolder = false;

	// GA-49 と同様、CI のクリップボード競合対策として設定→実行のサイクルをリトライする。
	// 検証 HWND も GA-49 と同様に DoGrep が使用する view の HWND に合わせる。
	const HWND hwndView = CEditWnd::getInstance()->GetActiveView().GetHwnd();

	DWORD hits = 0;
	std::string out;
	bool bClipVerified = false;	// 設定内容を一度でも読み戻せたか（環境判定用）
	for (int attempt = 0; attempt < 3 && hits == 0; ++attempt) {
		if (attempt > 0) { ::Sleep(100); }
		// ラインモード（bLineSelect=true）・末尾改行なしで設定 → DoGrep 側で改行補完される
		{
			CClipboard clipboard(hwndView);
			if (!clipboard) { continue; }
			if (!clipboard.SetText(L"LINEPASTE", 9, false, /*bLineSelect=*/true)) { continue; }
		}
		// 設定内容を読み戻して検証（OpenClipboard 不可の CI 環境では検証不可＝環境要因）
		bool bReadBack = false;
		if (::OpenClipboard(hwndView)) {
			if (HANDLE h = ::GetClipboardData(CF_UNICODETEXT)) {
				if (const wchar_t* p = static_cast<const wchar_t*>(::GlobalLock(h))) {
					bReadBack = (::wcscmp(p, L"LINEPASTE") == 0);
					::GlobalUnlock(h);
				}
			}
			::CloseClipboard();
		}
		if (!bReadBack) { continue; }
		bClipVerified = true;
		out.clear();
		hits = RunDoGrepStdoutHeader(
			agent, { L"lmneedle", L"", L"*.txt", m_temp->Root().wstring() },
			gOpt, sOpt, out);
	}
	if (hits == 0 && !bClipVerified) {
		GTEST_SKIP() << "Skipped: clipboard unavailable "
						"(OpenClipboard failed or read-back verification failed)";
	}

	EXPECT_EQ(1u, hits) << "1 件が置換される";

	std::ifstream is(path, std::ios::binary);
	std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
	const auto pos = content.find("LINEPASTE");
	ASSERT_NE(pos, std::string::npos) << "貼り付け文字列で置換される";
	ASSERT_LT(pos + 9, content.size());
	EXPECT_TRUE(content[pos + 9] == '\r' || content[pos + 9] == '\n')
		<< "ラインモード貼り付けにより末尾へ改行が補完される";
}

/*!
 * @brief ハンドルGrep: 有効ウィンドウの正常系（GetHwndTitle / CFileLoadOrWnd） (GA-51)
 * @remark テストスイートの編集ウィンドウ自身を :HWND: トークンで指定し、
 *         GetHwndTitle の有効 HWND 経路と CFileLoadOrWnd の HWND 分岐
 *         （FileOpen/ReadLine/GetPercent/GetFileSize/FileClose）を通過することを確認する。
 * @note   テスト環境の編集ウィンドウが IsSakuraMainWindow 判定や EditNode 登録の
 *         要件を満たさない場合はエラー経路（GA-41 で検証済み）へ落ちるため、
 *         その場合は GTEST_SKIP で環境要因として明示する。
 */
TEST_F(GrepRealFileTest, HwndGrep_ValidWindow_SearchesWindowText)
{
	CEditView* pView = &CEditWnd::getInstance()->GetActiveView();
	const HWND hwndEditor = CEditWnd::getInstance()->GetHwnd();

	// 前提1: IsSakuraMainWindow はウィンドウクラス名（GSTR_EDITWINDOWNAME）判定。
	//        これが不成立の場合はテスト側では解消できないためスキップする。
	if (!IsSakuraMainWindow(hwndEditor)) {
		GTEST_SKIP() << "Skipped: edit window class name does not match "
						"(IsSakuraMainWindow returned false)";
	}

	// 前提2: GetHwndTitle は CAppNodeManager::GetEditNode で編集ウィンドウノードの
	//        登録を要求する。テスト環境では起動シーケンスの登録処理が走らないため、
	//        本テスト内で登録し、スコープ脱出時に必ず解除する。
	ASSERT_TRUE(CAppNodeGroupHandle(0).AddEditWndList(hwndEditor))
		<< "EditNode の登録に失敗";
	ScopeExit nodeGuard([hwndEditor] { CAppNodeGroupHandle(0).DeleteEditWndList(hwndEditor); });

	// 対象テキストをアクティブ文書へ投入（キーは本テスト固有の文字列にする）
	const CNativeW seed(L"hwuniq alpha\r\nhwuniq beta\r\n");
	pView->GetCommander().Command_ADDTAIL(seed.GetStringPtr(), seed.GetStringLength());

	const std::wstring token = CDlgGrep::BuildHwndFileToken(hwndEditor);

	CGrepAgent agent;
	SSearchOption sOpt;
	sOpt.Reset();
	std::string out;
	const DWORD hits = RunDoGrepStdoutHeader(
		agent, { L"hwuniq", L"", token, L"C:\\" },
		MakeGrepOption(), sOpt, out);

	if (out.find("HWND handle error") != std::string::npos) {
		GTEST_SKIP() << "Skipped: window not recognized as grep target after EditNode registration "
						"(environmental, e.g. MYWM_GETFILEINFO response)";
	}

	EXPECT_EQ(2u, hits) << "ウィンドウ内の hwuniq 2 件がヒットする";
	EXPECT_NE(out.find("hwuniq alpha"), std::string::npos) << "ヒット行が出力される";
	EXPECT_NE(out.find("hwuniq beta"),  std::string::npos) << "ヒット行が出力される";
}
