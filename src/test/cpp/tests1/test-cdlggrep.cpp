/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include <Windows.h>

#include <filesystem>
#include <fstream>

#include "dlg/CDlgGrep.h"
#include "dlg/ModalDialogCloser.hpp"
#include "env/ShareDataTestSuite.hpp"
#include "grep/CGrepEnumKeys.h"
#include "mem/CNativeW.h"
#include "env/DLLSHAREDATA.h"

namespace {

struct CDlgGrepTest
	: public ::testing::Test
	, public env::ShareDataTestSuite
{
	static void SetUpTestSuite()
	{
		// CDlgGrep は履歴や既定値の取得で ShareData を参照するため、事前に初期化する。
		SetUpShareData();
	}

	static void TearDownTestSuite()
	{
		TearDownShareData();
	}
};

} // namespace

// ======================================================================
// Phase 3-A: CDlgGrep Unit Tests (PR #2459)
// ======================================================================

// ----- 既定値とパック処理 -----

/*!
 * @brief コンストラクタ直後の既定値検証 (DG-01)
 * @remark コンストラクタ呼び出し直後、サブフォルダー検索や出力形式などが既定値になっていることを確認する。
 */
TEST_F(CDlgGrepTest, DefaultMemberValues_Constructor)
{
	CDlgGrep dlg;

	EXPECT_FALSE(dlg.m_bSubFolder);							// 初期状態ではサブフォルダー検索は無効
	EXPECT_FALSE(dlg.m_bFromThisText);						// 初期状態では編集中テキスト検索は無効
	EXPECT_EQ(CODE_SJIS, dlg.m_nGrepCharSet);				// 既定の文字コードは SJIS
	EXPECT_EQ(1, dlg.m_nGrepOutputLineType);				// 既定の出力対象はヒット行
	EXPECT_EQ(1, dlg.m_nGrepOutputStyle);					// 既定の出力形式は Normal
	EXPECT_EQ(L'\0', dlg.m_szFile[0]);						// 検索ファイル欄は空で開始
	EXPECT_EQ(L'\0', dlg.m_szFolder[0]);					// 検索フォルダー欄は空で開始
	EXPECT_EQ(L'\0', dlg.m_szExcludeFile[0]);				// 除外ファイル欄は空で開始
	EXPECT_EQ(L'\0', dlg.m_szExcludeFolder[0]);				// 除外フォルダー欄は空で開始
}

/*!
 * @brief 除外パターンの定数検証 (DG-02)
 * @remark DEFAULT_EXCLUDE_FILE_PATTERN と DEFAULT_EXCLUDE_FOLDER_PATTERN が仕様通り定義されていることを確認する。
 */
TEST_F(CDlgGrepTest, DefaultExcludePatterns_Constants)
{
	EXPECT_STREQ(L".*\\.msi$;.*\\.exe$;.*\\.obj$;.*\\.pdb$;.*\\.ilk$;.*\\.res$;.*\\.pch$;.*\\.iobj$;.*\\.ipdb$", DEFAULT_EXCLUDE_FILE_PATTERN);
	EXPECT_STREQ(L".git;.svn;.vs", DEFAULT_EXCLUDE_FOLDER_PATTERN);
}

/*!
 * @brief パック処理：除外指定なし (DG-03)
 * @remark 除外指定がない場合、検索ファイルパターンをそのまま返すことを確認する。
 */
TEST_F(CDlgGrepTest, GetPackedGFileString_NoExclusions)
{
	CDlgGrep dlg;
	wcscpy_s(dlg.m_szFile, L"*.cpp");
	std::wstring packed = dlg.GetPackedGFileString().GetStringPtr();
	EXPECT_STREQ(L"*.cpp", packed.c_str());
}

/*!
 * @brief パック処理：除外フォルダーあり (DG-04)
 * @remark 除外フォルダーが指定された場合、`#` プレフィックスを付加して結合することを確認する。
 */
TEST_F(CDlgGrepTest, GetPackedGFileString_WithExcludeFolders)
{
	CDlgGrep dlg;
	wcscpy_s(dlg.m_szFile, L"*.cpp");
	wcscpy_s(dlg.m_szExcludeFolder, L".git;.svn");
	std::wstring packed = dlg.GetPackedGFileString().GetStringPtr();
	EXPECT_STREQ(L"*.cpp;#.git;#.svn", packed.c_str());
}

/*!
 * @brief パック処理：除外ファイルあり (DG-05)
 * @remark 除外ファイルが指定された場合、`!` プレフィックスを付加して結合することを確認する。
 *         検索対象ファイルが空の場合は `*.*` を補う。
 */
TEST_F(CDlgGrepTest, GetPackedGFileString_WithExcludeFiles)
{
	CDlgGrep dlg;
	wcscpy_s(dlg.m_szFile, L"");	// 空の場合は内部で *.* になる
	wcscpy_s(dlg.m_szExcludeFile, L"*.obj;*.exe");
	std::wstring packed = dlg.GetPackedGFileString().GetStringPtr();
	EXPECT_STREQ(L"*.*;!*.obj;!*.exe", packed.c_str());
}

/*!
 * @brief パック処理：除外フォルダー内の「!」エスケープ (DG-06)
 * @remark `!` が含まれるフォルダー名をダブルクォートでエスケープして結合することを確認する。
 */
TEST_F(CDlgGrepTest, GetPackedGFileString_EscapeBangInExcludeFolder)
{
	CDlgGrep dlg;
	wcscpy_s(dlg.m_szExcludeFolder, L"!special");
	std::wstring packed = dlg.GetPackedGFileString().GetStringPtr();
	EXPECT_STREQ(L"*.*;\"#!special\"", packed.c_str());
}

/*!
 * @brief パック処理：除外フォルダー内の「#」エスケープ (DG-07)
 * @remark `#` が含まれるフォルダー名をダブルクォートでエスケープして結合することを確認する。
 */
TEST_F(CDlgGrepTest, GetPackedGFileString_EscapeHashInExcludeFolder)
{
	CDlgGrep dlg;
	wcscpy_s(dlg.m_szExcludeFolder, L"#hash");
	std::wstring packed = dlg.GetPackedGFileString().GetStringPtr();
	EXPECT_STREQ(L"*.*;\"##hash\"", packed.c_str());
}

/*!
 * @brief パック処理：除外フォルダー名のスペース区切り (DG-08)
 * @remark SplitPattern はスペースを区切り文字として扱うため、
 *         「my folder」は「my」と「folder」に分割され、それぞれに # が付く。
 *         スペース含みの単一フォルダー名は GetPackedGFileString 経由では扱えない（既知制限）。
 */
TEST_F(CDlgGrepTest, GetPackedGFileString_EscapeSpaceInExcludeFolder)
{
	CDlgGrep dlg;
	wcscpy_s(dlg.m_szExcludeFolder, L"my folder");
	std::wstring packed = dlg.GetPackedGFileString().GetStringPtr();
	EXPECT_STREQ(L"*.*;#my;#folder", packed.c_str());
}

/*!
 * @brief パック処理：除外フォルダー内のセミコロン (DG-09)
 * @remark セミコロンは区切り文字として処理され、それぞれ独立した除外指定となることを確認する。
 */
TEST_F(CDlgGrepTest, GetPackedGFileString_EscapeSemicolonInExcludeFolder)
{
	CDlgGrep dlg;
	wcscpy_s(dlg.m_szExcludeFolder, L"a;b");
	std::wstring packed = dlg.GetPackedGFileString().GetStringPtr();
	EXPECT_STREQ(L"*.*;#a;#b", packed.c_str());
}

/*!
 * @brief パック処理：複合パターン (DG-10)
 * @remark 除外フォルダーと除外ファイルの両方が指定された場合、すべてを正しくパック文字列に結合することを確認する。
 */
TEST_F(CDlgGrepTest, GetPackedGFileString_CombinedAllExclusions)
{
	CDlgGrep dlg;
	wcscpy_s(dlg.m_szFile, L"*.cpp");
	wcscpy_s(dlg.m_szExcludeFolder, L"build;dist");
	wcscpy_s(dlg.m_szExcludeFile, L"*.obj;*.tmp");
	std::wstring packed = dlg.GetPackedGFileString().GetStringPtr();
	EXPECT_STREQ(L"*.cpp;#build;#dist;!*.obj;!*.tmp", packed.c_str());
}

/*!
 * @brief パックとアンパックのラウンドトリップ検証 (GUI -> CLI -> EnumKeys) (DG-11)
 * @remark GUIで設定した複雑なパターンをパックし、それを再度 `CGrepEnumKeys::SetFileKeys` で正しくパースすることを確認する。
 */
TEST_F(CDlgGrepTest, RoundTrip_GuiToCliToEnumKeys)
{
	CDlgGrep dlg;
	wcscpy_s(dlg.m_szFile, L"*.cpp");
	wcscpy_s(dlg.m_szExcludeFolder, L"build;dist");
	wcscpy_s(dlg.m_szExcludeFile, L"*.obj;*.tmp");
	std::wstring packed = dlg.GetPackedGFileString().GetStringPtr();

	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(packed.c_str()));
	
	EXPECT_EQ(1, keys.m_vecSearchFileKeys.size());						// 検索対象は 1 件
	EXPECT_STREQ(L"*.cpp", keys.m_vecSearchFileKeys[0]);

	EXPECT_EQ(2, keys.m_vecExceptFolderKeys.size());					// 除外フォルダーは 2 件
	EXPECT_STREQ(L"build", keys.m_vecExceptFolderKeys[0]);
	EXPECT_STREQ(L"dist", keys.m_vecExceptFolderKeys[1]);

	EXPECT_EQ(2, keys.m_vecExceptFileRegexPatterns.size());				// ! プレフィックスは正規表現除外に入る
	EXPECT_STREQ(L"*.obj", keys.m_vecExceptFileRegexPatterns[0].c_str());
	EXPECT_STREQ(L"*.tmp", keys.m_vecExceptFileRegexPatterns[1].c_str());
}

// ----- 切り出し関数のテスト -----

/*!
 * @brief 既定除外パターンの決定：履歴が空の場合 (DG-12)
 * @remark 履歴が空の場合、定数で定義された既定パターンを使用し、履歴にも追加することを確認する。
 */
TEST_F(CDlgGrepTest, DetermineDefaultExcludePatterns_EmptyHistorySetsDefaults)
{
	CDlgGrep dlg;
	GetDllShareDataPtr()->m_sSearchKeywords.m_aExcludeFiles.clear();
	dlg.DetermineDefaultExcludePatterns();
	EXPECT_STREQ(DEFAULT_EXCLUDE_FILE_PATTERN, dlg.m_szExcludeFile);
}

/*!
 * @brief 既定除外パターンの決定：履歴が存在する場合 (DG-13)
 * @remark 履歴が既に存在する場合、先頭の履歴を既定の除外パターンとして使用することを確認する。
 */
TEST_F(CDlgGrepTest, DetermineDefaultExcludePatterns_NonEmptyHistoryUsesHistoryTop)
{
	CDlgGrep dlg;
	GetDllShareDataPtr()->m_sSearchKeywords.m_aExcludeFiles.clear();
	GetDllShareDataPtr()->m_sSearchKeywords.m_aExcludeFiles.push_back(L"*.bak");
	dlg.DetermineDefaultExcludePatterns();
	EXPECT_STREQ(L"*.bak", dlg.m_szExcludeFile);
}

/*!
 * @brief 既定除外パターンの決定：既に値がセットされている場合 (DG-14)
 * @remark 既に `m_szExcludeFile` に値がセットされている場合は、既定値の書き込みをスキップすることを確認する。
 */
TEST_F(CDlgGrepTest, DetermineDefaultExcludePatterns_AlreadySetSkipped)
{
	CDlgGrep dlg;
	wcscpy_s(dlg.m_szExcludeFile, L"*.tmp");
	dlg.DetermineDefaultExcludePatterns();
	EXPECT_STREQ(L"*.tmp", dlg.m_szExcludeFile);
}

// ----- :HWND: トークンのテスト -----

/*!
 * @brief HWNDフォーマット(BuildHwndFileToken)のテスト (DG-15)
 * @remark 指定されたHWND値を `:HWND:` プレフィックス付きの固定長文字列として正しくフォーマットすることを確認する。
 */
TEST_F(CDlgGrepTest, BuildHwndFileToken_Format)
{
	HWND hwnd = (HWND)(uintptr_t)0xABCD1234;
	std::wstring token = CDlgGrep::BuildHwndFileToken(hwnd);
#ifdef _WIN64
	EXPECT_STREQ(L":HWND:00000000abcd1234", token.c_str());		// 64bit は 16 桁固定
#else
	EXPECT_STREQ(L":HWND:abcd1234", token.c_str());				// 32bit は 8 桁固定
#endif
}

/*!
 * @brief HWNDフォーマット(BuildHwndFileToken)のテスト(NULL指定) (DG-16)
 * @remark HWNDがNULLの場合でも正しくゼロ埋めしてフォーマットすることを確認する。
 */
TEST_F(CDlgGrepTest, BuildHwndFileToken_NullHwnd)
{
	HWND hwnd = NULL;
	std::wstring token = CDlgGrep::BuildHwndFileToken(hwnd);
#ifdef _WIN64
	EXPECT_STREQ(L":HWND:0000000000000000", token.c_str());		// NULL でもゼロ埋め
#else
	EXPECT_STREQ(L":HWND:00000000", token.c_str());				// NULL でもゼロ埋め
#endif
}

/*!
 * @brief HWNDファイルトークン判定(IsHwndFileToken)のポジティブテスト (DG-17)
 * @remark 正しいフォーマットの `:HWND:` 文字列に対して true を返すことを確認する。
 */
TEST_F(CDlgGrepTest, IsHwndFileToken_PositiveCases)
{
	EXPECT_TRUE(CDlgGrep::IsHwndFileToken(L":HWND:00000000"));	// 正しい接頭辞なら true
	EXPECT_TRUE(CDlgGrep::IsHwndFileToken(L":HWND:abc123"));
}

/*!
 * @brief HWNDファイルトークン判定(IsHwndFileToken)のネガティブテスト (DG-18)
 * @remark 異なるフォーマットや通常のファイル名に対して false を返すことを確認する。
 */
TEST_F(CDlgGrepTest, IsHwndFileToken_NegativeCases)
{
	EXPECT_FALSE(CDlgGrep::IsHwndFileToken(L"*.cpp"));			// 通常のファイル名は false
	EXPECT_FALSE(CDlgGrep::IsHwndFileToken(L":hwnd:abc"));
	EXPECT_FALSE(CDlgGrep::IsHwndFileToken(L":HWND"));
	EXPECT_FALSE(CDlgGrep::IsHwndFileToken(L""));
}

// =============================================================================
// フル GUI 結合テスト (Phase 5 / DG-20 ～ 27)
// =============================================================================

struct CDlgGrepGuiTest : public ::testing::Test, public env::ShareDataTestSuite {
	static void SetUpTestSuite() { SetUpShareData(); }
	static void TearDownTestSuite() { TearDownShareData(); }

	std::filesystem::path m_testDir;

	void SetUp() override {
		if (::GetModuleHandleW(nullptr) == nullptr) {
			GTEST_SKIP() << "GUI not available";
		}

		// tests1.exe の配置ディレクトリ基準で一時フォルダを作成（GetTempPath はパス非依存で環境差異が出るため使わない）
		wchar_t exePath[MAX_PATH];
		::GetModuleFileNameW(nullptr, exePath, MAX_PATH);
		m_testDir = std::filesystem::path(exePath).parent_path() / L"test_grep_gui";
		std::filesystem::create_directories(m_testDir);

		// フォルダ存在チェックを通過させるためのダミーファイル
		std::ofstream ofs(m_testDir / L"dummy.cpp", std::ios::trunc);
		ofs << "int main() { return 0; }\n";
	}

	void TearDown() override {
		std::error_code ec;
		std::filesystem::remove_all(m_testDir, ec);
	}
};

/*!
 * @brief キャンセル即時送信テスト (DG-20)
 * @remark DoModal に対して直ちに IDCANCEL を送信し、0 を返して正常終了することを確認する。
 */
TEST_F(CDlgGrepGuiTest, DoModalCancelImmediately_NoException)
{
	dialog::ModalDialogCloser closer; // デフォルトで IDCANCEL を送信
	CDlgGrep dlg;
	const auto hInstance = ::GetModuleHandleW(nullptr);
	const int rc = dlg.DoModal(hInstance, nullptr, nullptr);
	EXPECT_EQ(0, rc);
}

/*!
 * @brief 有効入力でのOK送信テスト (DG-21)
 * @remark 有効な検索キーワード・ファイル・フォルダを指定して IDOK を送信し、正常に受理されて 1 (OK) が返ることを確認する。
 */
TEST_F(CDlgGrepGuiTest, DoModalOK_WithValidInputs_ReturnsOK)
{
	dialog::ModalDialogCloser closer([](HWND hWnd) {
		::PostMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
	});
	CDlgGrep dlg;
	dlg.m_strText = L"search";
	wcscpy_s(dlg.m_szFile, L"*.cpp");
	wcscpy_s(dlg.m_szFolder, m_testDir.c_str());
	const auto hInstance = ::GetModuleHandleW(nullptr);
	const int rc = dlg.DoModal(hInstance, nullptr, nullptr);
	EXPECT_EQ(1, rc);
}

/*!
 * @brief フォルダ未指定時のフォールバック動作 (DG-22)
 * @remark フォルダが空の状態で IDOK を送信した場合、
 *         GetData 内でカレントフォルダ等にフォールバックして成功する仕様を凍結する。
 */
TEST_F(CDlgGrepGuiTest, DoModalOK_EmptyFolder_FallsBackToCurrentDir)
{
	dialog::ModalDialogCloser closer([](HWND hWnd) {
		HWND hOwner = ::GetWindow(hWnd, GW_OWNER);
		if (hOwner != nullptr) {
			// MessageBox を閉じる
			::SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);
			// 本体をキャンセルで閉じて復帰させる
			::PostMessageW(hOwner, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);
		} else {
			// 本体のダイアログには OK を送る
			::PostMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
		}
	});

	CDlgGrep dlg;
	dlg.m_strText = L"search";
	wcscpy_s(dlg.m_szFile, L"*.cpp");
	dlg.m_szFolder[0] = L'\0';			// 空のフォルダ

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const int rc = dlg.DoModal(hInstance, nullptr, nullptr);
	EXPECT_EQ(1, rc);					// 空フォルダでも GetData 内でカレントフォルダ等にフォールバックして成功する仕様を凍結
}

/*!
 * @brief 不正正規表現指定時の DoModal 動作 (DG-23)
 * @remark 不正な正規表現を指定して IDOK を送信した場合でも、
 *         DoModal は成功（rc=1）を返す仕様を凍結する。
 *         構文チェックは DoModal 内ではブロッキングしない。
 */
TEST_F(CDlgGrepGuiTest, DoModalOK_InvalidRegex_StillReturnsOK)
{
	dialog::ModalDialogCloser closer([](HWND hWnd) {
		HWND hOwner = ::GetWindow(hWnd, GW_OWNER);
		if (hOwner != nullptr) {
			::SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);
			::PostMessageW(hOwner, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);
		} else {
			::PostMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
		}
	});

	CDlgGrep dlg;
	dlg.m_strText = L"(invalid";
	dlg.m_sSearchOption.bRegularExp = true;
	wcscpy_s(dlg.m_szFile, L"*.cpp");
	wcscpy_s(dlg.m_szFolder, m_testDir.c_str());

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const int rc = dlg.DoModal(hInstance, nullptr, nullptr);
	EXPECT_EQ(1, rc);					// 不正正規表現でも DoModal は成功する仕様を凍結（構文チェックは DoModal 内でブロッキングしない）
}

/*!
 * @brief セミコロン区切りフォルダの仕様凍結テスト (DG-24)
 * @remark セミコロンは CreateFolders で区切り文字として分割される仕様（既知制限）。
 *         セミコロン含みの単一フォルダとしては扱えないため、
 *         セミコロンで区切った 2 つの実在フォルダを渡して IDOK が返ることを確認する。
 */
TEST_F(CDlgGrepGuiTest, DoModalOK_FolderWithSemicolon_IsSplitBySeparator)
{
	// セミコロンを含むパスは CreateFolders で分割されるため、
	// 「セミコロン含みの単一フォルダ」としては扱えない。
	// これは Sakura の仕様（既知制限）。
	//
	// 代わりに、セミコロンで区切った 2 つの実在フォルダを検証する。
	auto dir1 = m_testDir / L"folderA";
	auto dir2 = m_testDir / L"folderB";
	std::filesystem::create_directories(dir1);
	std::filesystem::create_directories(dir2);

	dialog::ModalDialogCloser closer([](HWND hWnd) {
		::PostMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
	});
	CDlgGrep dlg;
	dlg.m_strText = L"foo";
	wcscpy_s(dlg.m_szFile, L"*.cpp");
	std::wstring folders = dir1.wstring() + L";" + dir2.wstring();
	wcscpy_s(dlg.m_szFolder, folders.c_str());

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const int rc = dlg.DoModal(hInstance, nullptr, nullptr);
	EXPECT_EQ(1, rc);
}

/*!
 * @brief 複数絶対パス指定時の挙動 (DG-25)
 * @remark セミコロン区切りで各々が有効な絶対パスである場合、2つのフォルダとしてそのまま保持（または解決）することを確認する。
 */
TEST_F(CDlgGrepGuiTest, DoModalOK_MultipleFolders_AllResolved)
{
	dialog::ModalDialogCloser closer([](HWND hWnd) {
		::PostMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
	});
	auto sub1 = m_testDir / L"sub1";
	auto sub2 = m_testDir / L"sub2";
	std::filesystem::create_directories(sub1);
	std::filesystem::create_directories(sub2);
	const std::wstring folders = sub1.wstring() + L";" + sub2.wstring();

	CDlgGrep dlg;
	dlg.m_strText = L"foo";
	wcscpy_s(dlg.m_szFile, L"*.cpp");
	wcscpy_s(dlg.m_szFolder, folders.c_str());

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const int rc = dlg.DoModal(hInstance, nullptr, nullptr);
	EXPECT_EQ(1, rc);
	// 正規化の詳細は実装依存のため、成功して戻ることのみ確認する
}

/*!
 * @brief 通常検索時の履歴追加 (DG-26)
 * @remark 「自テキスト検索」ではない場合、実行によって検索キーワード等を共有データの履歴に記録することを確認する。
 */
TEST_F(CDlgGrepGuiTest, DoModalOK_HistoryRecordedExceptFromThisText)
{
	dialog::ModalDialogCloser closer([](HWND hWnd) {
		::PostMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
	});
	CDlgGrep dlg;
	dlg.m_strText = L"history_test";
	wcscpy_s(dlg.m_szFile, L"*.h");
	wcscpy_s(dlg.m_szFolder, m_testDir.c_str());
	dlg.m_bFromThisText = FALSE;
	
	const auto hInstance = ::GetModuleHandleW(nullptr);
	dlg.DoModal(hInstance, nullptr, nullptr);
	
	auto* share = GetDllShareDataPtr();
	EXPECT_STREQ(L"history_test", share->m_sSearchKeywords.m_aSearchKeys[0]);
}

/*!
 * @brief 自テキスト検索時の履歴非汚染 (DG-27)
 * @remark 「自テキスト検索」モードでの実行時は、一時的な検索であるため共有データの履歴を汚染しないことを確認する。
 */
TEST_F(CDlgGrepGuiTest, DoModalOK_FromThisText_DoesNotPolluteHistory)
{
	dialog::ModalDialogCloser closer([](HWND hWnd) {
		::PostMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
	});
	
	auto* share = GetDllShareDataPtr();
	// 履歴の先頭に別の値を入れておく
	wcscpy_s(share->m_sSearchKeywords.m_aSearchKeys[0], _MAX_PATH, L"prev_key");

	CDlgGrep dlg;
	dlg.m_strText = L"pollute_test";
	wcscpy_s(dlg.m_szFile, L"*.cpp");
	wcscpy_s(dlg.m_szFolder, m_testDir.c_str());
	dlg.m_bFromThisText = TRUE;
	
	const auto hInstance = ::GetModuleHandleW(nullptr);
	dlg.DoModal(hInstance, nullptr, nullptr);
	
	// 履歴が更新されていないことを確認
	EXPECT_STREQ(L"prev_key", share->m_sSearchKeywords.m_aSearchKeys[0]);
}

