/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "dlg/CDlgOpenFile.h"

#include "window/EditorTestSuite.hpp"

#include <fstream>

namespace uia {

cxx::com_pointer<IUIAutomationElement> GetFocusedElement(
	IUIAutomation* pAutomation
);

} // namespace uia

namespace window {

/*!
 * @brief ファイルダイアログテストのパラメーター
 *
 * @param bVistaStyleFileDialog Vistaスタイルのファイルダイアログを使うかどうか
 *
 * @note 単独パラメーターなので、std::tuple<bool> でなく bool としている。
 */
using FileDialogTestParam = bool;

/*!
 * ファイルダイアログテストのためのフィクスチャクラス
 *
 */
struct FileDialogTest : public ::testing::TestWithParam<FileDialogTestParam>, public window::EditorTestSuite, public window::UiaTestSuite {
	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpUiaTestSuite();

		SetUpEditor();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		TearDownEditor();

		TearDownUiaTestSuite();
	}

	/*!
	 * テストが実行される直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		// テスト設定を反映する
		GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = GetParam();

		const auto unusedArg1 = G_AppInstance();

		auto& cDlgOpenFile = *CDlgOpenFile::getInstance();
		cDlgOpenFile.Create(
			unusedArg1,
			nullptr,
			L"*.txt",
			LR"(C:\Windows\System32)",
			std::vector<LPCWSTR>(),
			std::vector<LPCWSTR>()
		);
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override
	{
		// 設定を元に戻す
		GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = true;

		TearDownUia();
	}

	/*!
	 * ファイルを開くダイアログのタイトルを取得する
	 */
	std::wstring GetOpenFileNameDialogTitle() const { return GetParam() ? L"開く" : L"ファイルを開く"; }
};

TEST_P(FileDialogTest, Create001)
{
	// 落ちたり例外にならないこと
	auto& cDlgOpenFile = *CDlgOpenFile::getInstance();
	cDlgOpenFile.Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.txt",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST_P(FileDialogTest, Create002_LongFilter)
{
	// 落ちたり例外にならないこと
	auto& cDlgOpenFile = *CDlgOpenFile::getInstance();
	cDlgOpenFile.Create(
		GetModuleHandle(nullptr),
		nullptr,
		L".extension_250_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_LONG",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST_P(FileDialogTest, Create003_ManyFiltersy)
{
	// 落ちたり例外にならないこと
	auto& cDlgOpenFile = *CDlgOpenFile::getInstance();
	cDlgOpenFile.Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.extension_50_0_long_long_long_long_long_long_LONG;*.extension_50_1_long_long_long_long_long_long_LONG;*.extension_50_2_long_long_long_long_long_long_LONG;*.extension_50_3_long_long_long_long_long_long_LONG;*.extension_50_4_long_long_long_long_long_long_LONG;*.extension_50_5_long_long_long_long_long_long_LONG;*.extension_50_6_long_long_long_long_long_long_LONG;*.extension_50_7_long_long_long_long_long_long_LONG;*.extension_50_8_long_long_long_long_long_long_LONG;*.extension_50_9_long_long_long_long_long_long_LONG",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, DoModalOpenDlg001)
{
	const auto path = GetExeFileName().replace_filename(L"test.txt");

	std::error_code ec;
	std::filesystem::remove(path, ec);

	// ファイルが存在しない、のメッセージが出ないようにファイルを作る
	std::ofstream ofs{ path };
	ofs.close();

	// 表示されたファイルダイアログを閉じるためのスレッドを起動する
	auto t = StartDialogCloser(GetOpenFileNameDialogTitle(), [path] (IUIAutomation* pUIAutomation, HWND, std::stop_token) {
		auto pItem = uia::GetFocusedElement(pUIAutomation);
		ASSERT_THAT(pItem, NotNull());

		// ファイル名を入力する
		EmulateSetValue(pItem, path.c_str());

		// Enterキー押下で閉じる
		EmulateHitEnter();
	});

	SLoadInfo loadInfo{};
	std::vector<std::wstring> files;
	bool bOptions = true;
	CDlgOpenFile::getInstance()->DoModalOpenDlg(&loadInfo, &files, bOptions);

	// 作成したファイルを削除する
	std::filesystem::remove(path, ec);
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, DoModalOpenDlg101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(std::nullopt);

	// コマンドコードで、無理矢理動かす
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_FILEOPEN, nullptr, 0, pcEditWnd->DispatchEvent);
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, DoModalOpenDlg102)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(std::nullopt);

	SLoadInfo loadInfo{};
	bool bOptions = false;
	CDlgOpenFile::getInstance()->DoModalOpenDlg(&loadInfo, nullptr, bOptions);
}

/*!
 * 名前を付けて保存ダイアログの表示テスト
 */
TEST_P(FileDialogTest, DoModalSaveDlg001)
{
	const auto path = GetExeFileName().replace_filename(L"test.txt");

	std::error_code ec;
	std::filesystem::remove(path, ec);

	// 表示されたファイルダイアログを閉じるためのスレッドを起動する
	auto t = StartDialogCloser(L"名前を付けて保存", [path](IUIAutomation* pUIAutomation, HWND, std::stop_token) {
		auto pItem = uia::GetFocusedElement(pUIAutomation);
		ASSERT_THAT(pItem, NotNull());

		// ファイル名を入力する
		EmulateSetValue(pItem, path.c_str());

		// Enterキー押下で閉じる
		EmulateHitEnter();
	});

	SSaveInfo saveInfo{};
	bool bSimpleMode = true;
	CDlgOpenFile::getInstance()->DoModalSaveDlg(&saveInfo, bSimpleMode);
}

/*!
 * 名前を付けて保存ダイアログの表示テスト
 */
TEST_P(FileDialogTest, DoModalSaveDlg101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(std::nullopt);

	// コマンドコードで、無理矢理動かす
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_FILESAVEAS_DIALOG, nullptr, 0, pcEditWnd->DispatchEvent);
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, GetOpenFileName001)
{
	const auto path = GetExeFileName().replace_filename(L"test.txt");

	std::error_code ec;
	std::filesystem::remove(path, ec);

	// ファイルが存在しない、のメッセージが出ないようにファイルを作る
	std::ofstream ofs{ path };
	ofs.close();

	// 表示されたファイルダイアログを閉じるためのスレッドを起動する
	auto t = StartDialogCloser(L"開く", [path] (IUIAutomation* pUIAutomation, HWND, std::stop_token) {
		auto pItem = uia::GetFocusedElement(pUIAutomation);
		ASSERT_THAT(pItem, NotNull());

		// ファイル名を入力する
		EmulateSetValue(pItem, path.c_str());

		// Enterキー押下で閉じる
		EmulateHitEnter();
	});

	SFilePath szPath{ path.filename().c_str() };
	CDlgOpenFile::getInstance()->DoModal_GetOpenFileName(szPath, EFilter::EFITER_TEXT);

	// 作成したファイルを削除する
	std::filesystem::remove(path, ec);
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, GetOpenFileName101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(std::nullopt);

	// コマンドコードで、無理矢理動かす
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_LOADKEYMACRO, nullptr, 0, pcEditWnd->DispatchEvent);
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, GetOpenFileName102)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(std::nullopt);

	SFilePath szPath{};
	CDlgOpenFile::getInstance()->DoModal_GetOpenFileName(szPath, EFilter::EFITER_NONE);
}

#if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * 名前を付けて保存ダイアログの表示テスト
 */
TEST_P(FileDialogTest, GetSaveFileName001)
{
	if (!GetParam()) {
		GTEST_SUCCESS_("Legacy Common File Dialog may cause error on GitHub Actions.");
		return;
	}

	const auto path = GetExeFileName().replace_filename(L"test.txt");
	const auto dummyPath = GetExeFileName().replace_filename(L"dummy.txt");

	std::error_code ec;

	// 上書き確認メッセージが出ないように、事前にパスを削除しておく
	std::filesystem::remove(path, ec);

	// パス解決でｋるようにファイルを作る
	std::filesystem::remove(dummyPath, ec);
	std::ofstream ofs{ dummyPath };
	ofs.close();

	// 表示されたファイルダイアログを閉じるためのスレッドを起動する
	auto t = StartDialogCloser(L"名前を付けて保存", [path](IUIAutomation* pUIAutomation, HWND, std::stop_token) {
		auto pItem = uia::GetFocusedElement(pUIAutomation);
		ASSERT_THAT(pItem, NotNull());

		// ファイル名を入力する
		EmulateSetValue(pItem, path.c_str());

		// Enterキー押下で閉じる
		EmulateHitEnter();
	});

	SFilePath szPath{ dummyPath.filename().native() };
	CDlgOpenFile::getInstance()->DoModal_GetSaveFileName(szPath);

	std::filesystem::remove(dummyPath, ec);
}

#endif // if defined(_MSC_VER) &&  defined(_DEBUG)

/*!
 * 名前を付けて保存ダイアログの表示テスト
 */
TEST_P(FileDialogTest, GetSaveFileName101)
{
	// 保存先のパスを作る
	const auto path = GetExeFileName().replace_filename(L"test-save-file.txt");

	// 上書き確認メッセージが出ないように、事前にパスを削除しておく
	std::error_code ec;
	std::filesystem::remove(path, ec);

	// キーマクロ保存が使えるようにダミーマクロを登録する
	LPARAM lParams = 0L;
	pcSMacroMgr->Append(STAND_KEYMACRO, F_0, &lParams, &pcEditWnd->GetView(0));

	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	dialog::ModalDialogCloser closer(std::nullopt);

	// コマンドコードで、無理矢理動かす
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_SAVEKEYMACRO, nullptr, 0, pcEditWnd->DispatchEvent);

	// 保存したファイルを削除する
	std::filesystem::remove(path, ec);
}

/*!
 * @brief パラメータテストをインスタンス化する
 *  Vistaスタイル有効／無効の2パターンで実体化させる
 */
INSTANTIATE_TEST_SUITE_P(FileDialog
	, FileDialogTest
	, ::testing::Values(
		true,
		false
	)
);

//! ファイル選択テストのためのフィクスチャクラス
struct SelectFileTest : public ::testing::Test, public window::EditorTestSuite, public window::UiaTestSuite {
	static constexpr auto& text = L"test.ini";
	static inline auto path = GetExeFileName().replace_filename(text);

	static inline HWND hWnd = nullptr;
	static inline HWND hWndDlg = nullptr;
	static inline HWND hWndFolder = nullptr;

	static inline std::unique_ptr<CDialog> pcDlg = nullptr;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpUiaTestSuite();

		SetUpEditor();

		// ファイルが存在しない、のメッセージが出ないようにファイルを作る
		std::ofstream ofs{ path };
		ofs.close();

		constexpr HINSTANCE unusedArg1 = nullptr;
		hWnd = pcEditWnd->GetHwnd();

		// テスト用ダミーダイアログを作る
		pcDlg = std::make_unique<CDialog>();
		hWndDlg = pcDlg->DoModeless(unusedArg1, hWnd, IDD_GREP, 0L, SW_SHOW);
		EXPECT_THAT(hWndDlg, NotNull());

		// ファイルパスを入力する項目のハンドルを取得する
		hWndFolder = ::GetDlgItem(hWndDlg, IDC_COMBO_FOLDER);
		EXPECT_THAT(hWndFolder, NotNull());

		// ファイルパスの初期値に相対パスを入れる
		apiwrap::SetDlgItemTextW(hWndDlg, IDC_COMBO_FOLDER, text);
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		// テスト用ダミーダイアログを閉じる
		pcDlg->CloseDialog(0);

		// 作成したファイルを削除する
		std::error_code ec;
		std::filesystem::remove(path, ec);

		TearDownEditor();

		TearDownUiaTestSuite();
	}

	/*!
	 * テストが実行される直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		CDlgOpenFile::setInstance<MockCDlgOpenFile>();

		MockCDlgOpenFile::gm_Files.emplace_back(path.native());
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override
	{
		CDlgOpenFile::resetInstance();

		TearDownUia();
	}
};

/*!
 * ファイル選択のテスト
 */
TEST_F(SelectFileTest, SelectFile001)
{
	constexpr bool resolvePath = true;	// パス解決する場合のテスト

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	EXPECT_THAT(CDlgOpenFile::SelectFile(hWndDlg, hWndFolder, L"*.ini", resolvePath, EFITER_NONE), IsTrue());

	const auto ret = apiwrap::GetDlgItemTextW(hWndDlg, IDC_COMBO_FOLDER);
	EXPECT_THAT(ret.c_str(), StrEq(text));	// 相対パスが設定される
}


/*!
 * ファイル選択のテスト
 */
TEST_F(SelectFileTest, SelectFile002)
{
	constexpr bool resolvePath = false;	// パス解決しない場合のテスト

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	EXPECT_THAT(CDlgOpenFile::SelectFile(hWndDlg, hWndFolder, L"*.ini", resolvePath, EFITER_NONE), IsTrue());

	const auto ret = apiwrap::GetDlgItemTextW(hWndDlg, IDC_COMBO_FOLDER);
	EXPECT_THAT(ret.c_str(), StrEq(path.c_str()));	// 絶対パスが設定される
}

/*!
 * ファイル選択のテスト
 */
TEST_F(SelectFileTest, SelectFile101)
{
	constexpr bool resolvePath = true;

	MockCDlgOpenFile::gm_Files.clear();

	auto& cDlgOpenFile = static_cast<MockCDlgOpenFile&>(*CDlgOpenFile::getInstance());
	EXPECT_CALL(cDlgOpenFile, DoModal_GetOpenFileName(_, _))
		.Times(1)
		.WillOnce(testing::DoDefault());

	EXPECT_THAT(CDlgOpenFile::SelectFile(hWndDlg, hWndFolder, L"*.ini", resolvePath, EFITER_NONE), IsFalse());
}

} // namespace window
