/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
// ====================================================================================
// Phase 3-B: CGrepAgent Unit Tests for Result Formatting & Flow Helpers (PR #2459)
// ====================================================================================

#include "pch.h"
#include <gtest/gtest.h>
#include <Windows.h>
#include <functional>
#include <memory>

#include "agent/CGrepAgent.h"
#include "env/ShareDataTestSuite.hpp"
#include "window/EditorTestSuite.hpp"

// ----- FormatGrepResultLine -----

namespace {
// FormatGrepResultLine は内部で GetDllShareData() を参照するため ShareData を初期化するフィクスチャを使用する
struct CGrepAgentTest
	: public ::testing::Test
	, public env::ShareDataTestSuite
{
	// BuildGrepHeader/FormatGrepResultLine は内部で共有データを参照するため、ここで初期化する。
	static void SetUpTestSuite()  { SetUpShareData(); }
	static void TearDownTestSuite() { TearDownShareData(); }
};
} // namespace

/*!
 * @brief 検索結果行のフォーマット(Normalスタイル) (GA-01)
 * @remark パス、行番号、桁番号、文字コード、およびヒットした行全体を指定の書式で出力することを確認する。
 */
TEST_F(CGrepAgentTest, FormatGrepResultLine_NormalStyle1_ProducesPathLineColContent)
{
	CNativeW cmem;
	SGrepOption opt;
	opt.nGrepOutputStyle = 1;
	opt.nGrepOutputLineType = 1;

	CGrepAgent::FormatGrepResultLine(
		cmem, L"C:\\test.cpp", L" [UTF-8]",
		SGrepMatchInfo{ 42, 5, L"  return true;\n", 15, 1, L"true", 4 }, opt
	);
	EXPECT_STREQ(L"C:\\test.cpp(42,5) [UTF-8]:   return true;\r\n", cmem.GetStringPtr());	// 正常形式の出力
}

/*!
 * @brief 検索結果行のフォーマット(WZ風スタイル) (GA-02)
 * @remark ファイル名出力がグループ化するスタイルにおいて、ファイルパス情報を含めずに行番号・桁情報から出力することを確認する。
 */
TEST_F(CGrepAgentTest, FormatGrepResultLine_WzStyle2_FileGrouped)
{
	CNativeW cmem;
	SGrepOption opt;
	opt.nGrepOutputStyle = 2;
	opt.nGrepOutputLineType = 1;

	CGrepAgent::FormatGrepResultLine(
		cmem, L"C:\\test.cpp", L"",
		SGrepMatchInfo{ 42, 5, L"  return true;\n", 15, 1, L"true", 4 }, opt
	);
	std::wstring s(cmem.GetStringPtr());
	EXPECT_NE(std::wstring::npos, s.find(L"42"));				// 行番号
	EXPECT_NE(std::wstring::npos, s.find(L"5"));				// 桁番号
	EXPECT_NE(std::wstring::npos, s.find(L"return true;"));		// 行内容
	// WZ風の場合、行頭に "・" が付く
	EXPECT_EQ(L'・', s[0]);	// グループ先頭マーカー
}

/*!
 * @brief 検索結果行のフォーマット(結果のみスタイル) (GA-03)
 * @remark パスや行番号情報を出力せず、マッチした行のコンテンツのみを出力することを確認する。
 */
TEST_F(CGrepAgentTest, FormatGrepResultLine_ResultOnlyStyle3_NoPath)
{
	CNativeW cmem;
	SGrepOption opt;
	opt.nGrepOutputStyle = 3;
	opt.nGrepOutputLineType = 1;

	CGrepAgent::FormatGrepResultLine(
		cmem, L"C:\\test.cpp", L"",
		SGrepMatchInfo{ 42, 5, L"  return true;\n", 15, 1, L"true", 4 }, opt
	);
	EXPECT_STREQ(L"  return true;\r\n", cmem.GetStringPtr());
}

/*!
 * @brief 検索結果行のフォーマット(該当部分のみ出力) (GA-04)
 * @remark 行全体ではなく、マッチした部分の文字列だけを出力することを確認する。
 * @note nGrepOutputLineType=0 は内部で GetDllShareData() を参照するため ShareData 初期化が必要。
 */
TEST_F(CGrepAgentTest, FormatGrepResultLine_OutputLineType0_OnlyMatchPart)
{
	CNativeW cmem;
	SGrepOption opt;
	opt.nGrepOutputStyle = 3;
	opt.nGrepOutputLineType = 0; // 該当部分のみ

	CGrepAgent::FormatGrepResultLine(
		cmem, L"C:\\test.cpp", L"",
		SGrepMatchInfo{ 42, 5, L"  return true;\n", 15, 1, L"true", 4 }, opt
	);
	EXPECT_STREQ(L"true\r\n", cmem.GetStringPtr());
}

/*!
 * @brief 検索結果行のフォーマット(行全体出力) (GA-05)
 * @remark マッチした文字列を含む行全体を出力することを確認する。
 */
TEST_F(CGrepAgentTest, FormatGrepResultLine_OutputLineType1_FullLine)
{
	CNativeW cmem;
	SGrepOption opt;
	opt.nGrepOutputStyle = 3;
	opt.nGrepOutputLineType = 1;

	CGrepAgent::FormatGrepResultLine(
		cmem, L"C:\\test.cpp", L"",
		SGrepMatchInfo{ 42, 5, L"  return true;\n", 15, 1, L"true", 4 }, opt
	);
	EXPECT_STREQ(L"  return true;\r\n", cmem.GetStringPtr());
}

/*!
 * @brief 検索結果行のフォーマット(否該当行出力) (GA-06)
 * @remark nGrepOutputLineType=2 でも FormatGrepResultLine 自体は行全体を出力する。
 *         非該当行の判定・フィルタリングは呼び出し側 (DoGrepFile) の責任である。
 */
TEST_F(CGrepAgentTest, FormatGrepResultLine_OutputLineType2_NegativeLine)
{
	CNativeW cmem;
	SGrepOption opt;
	opt.nGrepOutputStyle = 3;			// 結果のみ（パス情報を除外して検証を簡潔化）
	opt.nGrepOutputLineType = 2;		// 否該当行

	CGrepAgent::FormatGrepResultLine(
		cmem, L"C:\\test.cpp", L"",
		SGrepMatchInfo{ 42, 1, L"no match here", 13, 0, L"", 0 }, opt
	);
	EXPECT_STREQ(L"no match here\r\n", cmem.GetStringPtr());
}

// ----- BuildGrepHeader / BuildGrepFooter -----

/*!
 * @brief 検索ヘッダ生成(基本形) (GA-07)
 * @remark 検索キーワード、対象ファイルパターン、検索対象フォルダーの情報がヘッダに含まれることを確認する。
 */
TEST(CGrepAgent, BuildGrepHeader_BasicShape)
{
	SSearchOption sOpt;
	SGrepOption gOpt;

	CNativeW header = CGrepAgent::BuildGrepHeader(L"foo", L"*.cpp", L"C:\\src", sOpt, gOpt);
	std::wstring s(header.GetStringPtr());
	EXPECT_NE(std::wstring::npos, s.find(L"foo"));
	EXPECT_NE(std::wstring::npos, s.find(L"*.cpp"));
	EXPECT_NE(std::wstring::npos, s.find(L"C:\\src"));
}

/*!
 * @brief 検索ヘッダ生成(正規表現フラグON) (GA-08)
 * @remark 正規表現オプションが有効な場合、ヘッダ文字列に「正規表現」のマーカーが含まれることを確認する。
 */
TEST(CGrepAgent, BuildGrepHeader_WithRegexOption_IncludesMarker)
{
	SSearchOption sOpt;
	sOpt.bRegularExp = true;
	SGrepOption gOpt;

	CNativeW header = CGrepAgent::BuildGrepHeader(L"foo", L"*.cpp", L"C:\\src", sOpt, gOpt);
	std::wstring s(header.GetStringPtr());
	EXPECT_NE(std::wstring::npos, s.find(L"正規表現"));
}

/*!
 * @brief 検索ヘッダ生成(大文字小文字区別フラグON) (GA-09)
 * @remark 大文字小文字の区別オプションが有効な場合、ヘッダ文字列に「大文字小文字」のマーカーが含まれることを確認する。
 */
TEST(CGrepAgent, BuildGrepHeader_WithCaseSensitive_IncludesMarker)
{
	SSearchOption sOpt;
	sOpt.bLoHiCase = true;
	SGrepOption gOpt;

	CNativeW header = CGrepAgent::BuildGrepHeader(L"foo", L"*.cpp", L"C:\\src", sOpt, gOpt);
	std::wstring s(header.GetStringPtr());
	EXPECT_NE(std::wstring::npos, s.find(L"大文字小文字"));
}

/*!
 * @brief 検索フッタ生成(0件ヒット時) (GA-10)
 * @remark 検索結果が0件の場合、件数「0」を含むフッタ文字列を生成することを確認する。
 *         フッタの具体的な書式（「0 件」「0件」等）は実装依存のため、数字の存在のみを検証する。
 */
TEST(CGrepAgent, BuildGrepFooter_ZeroHits_HasZeroMessage)
{
	CNativeW footer = CGrepAgent::BuildGrepFooter(0, false);
	std::wstring s(footer.GetStringPtr());
	EXPECT_NE(std::wstring::npos, s.find(L"0"));
}

/*!
 * @brief 検索フッタ生成(ヒットあり・通常Grep) (GA-11)
 * @remark 通常Grepで件数が含まれること、置換Grepで件数が含まれることをそれぞれ確認する。
 */
TEST(CGrepAgent, BuildGrepFooter_PositiveHits_HasCount)
{
	{
		CNativeW footer = CGrepAgent::BuildGrepFooter(42, false);
		std::wstring s(footer.GetStringPtr());
		EXPECT_NE(std::wstring::npos, s.find(L"42"));
	}
	{
		CNativeW footer = CGrepAgent::BuildGrepFooter(42, true);
		std::wstring s(footer.GetStringPtr());
		EXPECT_NE(std::wstring::npos, s.find(L"42"));
	}
}

// ----- CreateFolders / ChopYen -----

/*!
 * @brief CreateFolders: セミコロン区切りの分割 (GA-12)
 * @remark 複数のフォルダーパスがセミコロンで区切られている場合、それぞれを独立したパス要素として抽出することを確認する。
 */
TEST(CGrepAgent, CreateFolders_SemicolonSeparated_SplitsCorrectly)
{
	std::vector<std::wstring> v;
	CGrepAgent::CreateFolders(L"C:\\a;C:\\b", v);
	ASSERT_EQ(2, v.size());
	EXPECT_STREQ(L"C:\\a", v[0].c_str());
	EXPECT_STREQ(L"C:\\b", v[1].c_str());
}

/*!
 * @brief CreateFolders: ダブルクォートで囲まれたセミコロン (GA-13)
 * @remark パスがダブルクォートで囲まれている場合、内部のセミコロンは区切り文字として扱われず、
 *         ダブルクォートが除去された1つのパスとなることを確認する。
 */
TEST(CGrepAgent, CreateFolders_QuotedSemicolon_PreservesAsOne)
{
	std::vector<std::wstring> v;
	CGrepAgent::CreateFolders(L"\"C:\\a;b\"", v);
	ASSERT_EQ(1, v.size());
	EXPECT_STREQ(L"C:\\a;b", v[0].c_str());
}

/*!
 * @brief CreateFolders: 長いファイル名の解決 (GA-14)
 * @remark 8.3形式の短いパスが指定された場合、`GetLongFileName` で元の長いパスに解決してリストへ追加することを確認する。
 * @note  8.3 形式生成が無効化されたボリューム（NTFS Win10+ 既定など）では
 *        GetShortPathName が失敗するため GTEST_SKIP でスキップする。
 */
TEST(CGrepAgent, CreateFolders_LongFileName_Resolved)
{
	WCHAR tempDir[MAX_PATH];
	::GetTempPathW(MAX_PATH, tempDir);

	const std::wstring longPath = std::wstring(tempDir) + L"sakura_grep_test_very_long_file_name.tmp";

	HANDLE hFile = ::CreateFileW(longPath.c_str(), GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(INVALID_HANDLE_VALUE, hFile)
		<< "一時ファイル作成失敗：テスト前提条件未達。GetLastError=" << ::GetLastError();
	::CloseHandle(hFile);

	auto fileGuard = std::unique_ptr<void, std::function<void(void*)>>(
		reinterpret_cast<void*>(1),
		[&longPath](void*) { ::DeleteFileW(longPath.c_str()); }
	);

	WCHAR shortPath[MAX_PATH];
	if (0 == ::GetShortPathNameW(longPath.c_str(), shortPath, MAX_PATH)) {
		GTEST_SKIP() << "8.3 形式が無効なボリュームのためスキップ。GetLastError="
					 << ::GetLastError();
	}

	std::vector<std::wstring> v;
	CGrepAgent::CreateFolders(shortPath, v);
	ASSERT_EQ(1u, v.size());
	EXPECT_GT(v[0].length(), 0u);
}

/*!
 * @brief ChopYen: 末尾の円記号削除 (GA-15)
 * @remark パスの末尾に円記号（バックスラッシュ）がある場合、それを除去することを確認する。
 */
TEST(CGrepAgent, ChopYen_LastBackslashRemoved)
{
	EXPECT_STREQ(L"C:\\foo", CGrepAgent::ChopYen(L"C:\\foo\\").c_str());
}

/*!
 * @brief ChopYen: 末尾に円記号がない場合 (GA-16)
 * @remark パスの末尾に円記号がない場合は何も変更しないことを確認する。
 */
TEST(CGrepAgent, ChopYen_NoTrailingYen_Unchanged)
{
	EXPECT_STREQ(L"C:\\foo", CGrepAgent::ChopYen(L"C:\\foo").c_str());
}

/*!
 * @brief ChopYen: ルートパスの場合 (GA-17)
 * @remark ドライブのルート(C:\ など)の場合でも、仕様として末尾の円記号を除去して(C: となる)ことを確認する。
 */
TEST(CGrepAgent, ChopYen_RootPath_AlsoTrimmed)
{
	EXPECT_STREQ(L"C:", CGrepAgent::ChopYen(L"C:\\").c_str());
}

// ----- AddTail / OnBeforeClose -----

namespace {

/*!
 * @brief Windows 標準ハンドル（STD_OUTPUT_HANDLE 等）の一時差し替えと
 *        スコープ脱出時の確実な復帰を保証する RAII ガード。
 *
 * SetStdHandle を直接呼ぶテストでは、例外発生時に元のハンドルへ復帰できず
 * テストプロセスの stdout が破壊されたまま後続テストへ波及するリスクがある。
 * 本ガードは構築時に差し替え、デストラクタで必ず元に戻す。
 */
class StdHandleGuard {
public:
	StdHandleGuard(DWORD id, HANDLE replacement)
		: m_id(id), m_saved(::GetStdHandle(id))
	{
		::SetStdHandle(m_id, replacement);
	}
	~StdHandleGuard()
	{
		::SetStdHandle(m_id, m_saved);
	}
	StdHandleGuard(const StdHandleGuard&) = delete;
	StdHandleGuard& operator=(const StdHandleGuard&) = delete;

private:
	DWORD  m_id;
	HANDLE m_saved;
};

struct GrepAgentFlowTest
	: public ::testing::Test
	, public window::EditorTestSuite
{
	static void SetUpTestSuite()  { SetUpEditor(); }
	static void TearDownTestSuite() { TearDownEditor(); }
};

} // namespace

/*!
 * @brief AddTail: 標準出力への書き込み (GA-18)
 * @remark bAddStdout が true に指定された場合、エディタへの挿入ではなく標準出力（stdout）へエンコード変換した結果を書き出すことを確認する。
 */
TEST_F(GrepAgentFlowTest, AddTail_StdoutMode_WritesToStdout)
{
	WCHAR tempDir[MAX_PATH];
	::GetTempPathW(MAX_PATH, tempDir);
	const std::wstring tmpFile = std::wstring(tempDir) + L"sakura_stdout_test.txt";

	HANDLE hTempFile = ::CreateFileW(tmpFile.c_str(),
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(INVALID_HANDLE_VALUE, hTempFile)
		<< "一時ファイル作成失敗。テスト前提条件未達。";

	struct HandleDeleter {
		void operator()(HANDLE h) const {
			if (h && h != INVALID_HANDLE_VALUE) ::CloseHandle(h);
		}
	};
	std::unique_ptr<void, HandleDeleter> hFileGuard{ hTempFile };

	{
		// stdout 差し替えの RAII（このスコープを抜けると必ず元に戻る）
		StdHandleGuard stdoutGuard(STD_OUTPUT_HANDLE, hTempFile);

		CGrepAgent agent;
		CNativeW msg(L"HelloGrepTest");

		// SetUpEditor はフィクスチャ (GrepAgentFlowTest::SetUpTestSuite) で初期化済み
		CEditView* pView = &CEditWnd::getInstance()->GetActiveView();
		agent.AddTail(pView, msg, true);
	} // ← stdout 復帰

	::SetFilePointer(hTempFile, 0, NULL, FILE_BEGIN);
	char buf[128] = {0};
	DWORD dwRead = 0;
	::ReadFile(hTempFile, buf, sizeof(buf) - 1, &dwRead, NULL);

	EXPECT_TRUE(strstr(buf, "HelloGrepTest") != nullptr);

	hFileGuard.reset();
	::DeleteFileW(tmpFile.c_str());
}

/*!
 * @brief OnBeforeClose: Grep実行中の割り込み (GA-19)
 * @remark Grep処理が実行中(m_bGrepRunningがtrue)の場合、クローズ要求に対して割り込み(CALLBACK_INTERRUPT)を返すことを確認する。
 */
TEST_F(GrepAgentFlowTest, OnBeforeClose_DuringGrepReturnsInterrupt)
{
	CGrepAgent agent;
	agent.m_bGrepRunning = true;
	EXPECT_EQ(CALLBACK_INTERRUPT, agent.OnBeforeClose());
}

/*!
 * @brief OnBeforeClose: Grep非実行時の継続 (GA-20)
 * @remark Grep処理が実行中でない場合、クローズ要求に対して継続(CALLBACK_CONTINUE)を返すことを確認する。
 */
TEST_F(GrepAgentFlowTest, OnBeforeClose_NotRunningReturnsContinue)
{
	CGrepAgent agent;
	agent.m_bGrepRunning = false;
	EXPECT_EQ(CALLBACK_CONTINUE, agent.OnBeforeClose());
}
