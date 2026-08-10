/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "dlg/CDlgOpenFile.h"

#include "window/EditorTestSuite.hpp"

#include "eval_outputs.hpp"

#include <fstream>

void CallDlgOpenFail();

namespace dialog {

HWND FindFileNameEdit(HWND hFileDialog);

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
	static constexpr auto& openDialogTitle = L"開く";
	static constexpr auto& saveDialogTitle0 = L"開く";
	static constexpr auto& saveDialogTitle = L"名前を付けて保存";

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpUiaTestSuite();

		SetUpEditor();

		Comdlg32::setInstance<MockComdlg32>();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		Comdlg32::resetInstance();

		TearDownEditor();

		TearDownUiaTestSuite();
	}

	/*!
	 * テストが実行される直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		MockComdlg32::gm_Files.clear();

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
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	std::unique_ptr<dialog::ModalDialogCloser> closer = nullptr;

	const auto testAction = [] () {
		SLoadInfo loadInfo{};
		std::vector<std::wstring> files;
		bool bOptions = true;
		CDlgOpenFile::getInstance()->DoModalOpenDlg(&loadInfo, &files, bOptions);
	};

	const auto path = GetExeFileName().replace_filename(L"test.txt");

	std::error_code ec;
	std::filesystem::remove(path, ec);

	// ファイルが存在しない、のメッセージが出ないようにファイルを作る
	std::ofstream ofs{ path };
	ofs.close();

	if (GetParam()) {
		// 表示されたモーダルダイアログをキャンセルボタンで閉じる
		closer = std::make_unique<dialog::ModalDialogCloser>(openDialogTitle, [path] (HWND hWndDlg) {
			// 現状の値を決め打ち
			// ある日突然機能しなくなる可能性あるので注意
			apiwrap::SetDlgItemTextW(hWndDlg, 1148, path.c_str());
			SendDlgCommand(hWndDlg, IDOK);
		});

	} else {
		auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
		EXPECT_CALL(*pComdlg32, GetOpenFileNameW(_))
			.Times(1)
			.WillOnce(testing::DoDefault());
		EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
			.Times(0);

		MockComdlg32::gm_Files.emplace_back(path.native());
	}

	testAction();

	// 作成したファイルを削除する
	std::filesystem::remove(path, ec);
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, DoModalOpenDlg101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	std::unique_ptr<dialog::ModalDialogCloser> closer = nullptr;

	const auto testAction = [] () {
		// コマンドコードで、無理矢理動かす
		const auto hWnd = pcEditWnd->GetHwnd();
		FORWARD_WM_COMMAND(hWnd, F_FILEOPEN, nullptr, 0, pcEditWnd->DispatchEvent);
	};

	if (GetParam()) {
		// 表示されたモーダルダイアログをキャンセルボタンで閉じる
		closer = std::make_unique<dialog::ModalDialogCloser>();

	} else {
		auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
		EXPECT_CALL(*pComdlg32, GetOpenFileNameW(_))
			.Times(1)
			.WillOnce(Return(FALSE));
		EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
			.Times(2)
			.WillRepeatedly(Return(0));
	}

	testAction();
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, DoModalOpenDlg102)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	std::unique_ptr<dialog::ModalDialogCloser> closer = nullptr;

	const auto testAction = [] () {
		SLoadInfo loadInfo{};
		bool bOptions = false;
		CDlgOpenFile::getInstance()->DoModalOpenDlg(&loadInfo, nullptr, bOptions);
	};

	if (GetParam()) {
		// 表示されたモーダルダイアログをキャンセルボタンで閉じる
		closer = std::make_unique<dialog::ModalDialogCloser>();

	} else {
		auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
		EXPECT_CALL(*pComdlg32, GetOpenFileNameW(_))
			.Times(1)
			.WillOnce(Return(FALSE));
		EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
			.Times(2)
			.WillRepeatedly(Return(0));
	}

	testAction();
}

/*!
 * 名前を付けて保存ダイアログの表示テスト
 */
TEST_P(FileDialogTest, DoModalSaveDlg001)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	std::unique_ptr<dialog::ModalDialogCloser> closer = nullptr;

	const auto testAction = [] () {
		SSaveInfo saveInfo{};
		bool bSimpleMode = true;
		CDlgOpenFile::getInstance()->DoModalSaveDlg(&saveInfo, bSimpleMode);
	};

	const auto path = GetExeFileName().replace_filename(L"test.txt");

	std::error_code ec;
	std::filesystem::remove(path, ec);

	if (GetParam()) {
		// 表示されたモーダルダイアログをキャンセルボタンで閉じる
		closer = std::make_unique<dialog::ModalDialogCloser>(saveDialogTitle0, [path] (HWND hWndDlg) {
			ASSERT_THAT(apiwrap::GetWindowTextW(hWndDlg), StrEq(saveDialogTitle));

			// 現状の値を決め打ち
			// ある日突然機能しなくなる可能性あるので注意
			const auto hWndEdit = dialog::FindFileNameEdit(hWndDlg);
			apiwrap::SetWindowTextW(hWndEdit, path.c_str());
			SendDlgCommand(hWndDlg, IDOK);
		});

	} else {
		auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
		EXPECT_CALL(*pComdlg32, GetSaveFileNameW(_))
			.Times(1)
			.WillOnce(testing::DoDefault());
		EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
			.Times(0);

		MockComdlg32::gm_Files.emplace_back(path.native());
	}

	testAction();

	// 保存したファイルを削除する
	std::filesystem::remove( path, ec );
}

/*!
 * 名前を付けて保存ダイアログの表示テスト
 */
TEST_P(FileDialogTest, DoModalSaveDlg101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	std::unique_ptr<dialog::ModalDialogCloser> closer = nullptr;

	const auto testAction = [] () {
		// コマンドコードで、無理矢理動かす
		const auto hWnd = pcEditWnd->GetHwnd();
		FORWARD_WM_COMMAND(hWnd, F_FILESAVEAS_DIALOG, nullptr, 0, pcEditWnd->DispatchEvent);
	};

	if (GetParam()) {
		// 表示されたモーダルダイアログをキャンセルボタンで閉じる
		closer = std::make_unique<dialog::ModalDialogCloser>();

	} else {
		auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
		EXPECT_CALL(*pComdlg32, GetSaveFileNameW(_))
			.Times(1)
			.WillOnce(Return(FALSE));
		EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
			.Times(2)
			.WillRepeatedly(Return(0));
	}

	testAction();
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, GetOpenFileName001)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	std::unique_ptr<dialog::ModalDialogCloser> closer = nullptr;

	const auto testAction = [] () {
		SFilePath szPath{};
		CDlgOpenFile::getInstance()->DoModal_GetOpenFileName(szPath, EFilter::EFITER_TEXT);
	};

	const auto path = GetExeFileName().replace_filename(L"test.txt");

	std::error_code ec;
	std::filesystem::remove(path, ec);

	// ファイルが存在しない、のメッセージが出ないようにファイルを作る
	std::ofstream ofs{ path };
	ofs.close();

	if (GetParam()) {
		// 表示されたモーダルダイアログを閉じる
		closer = std::make_unique<dialog::ModalDialogCloser>(openDialogTitle, [path] (HWND hWndDlg) {
			// 現状の値を決め打ち
			// ある日突然機能しなくなる可能性あるので注意
			apiwrap::SetDlgItemTextW(hWndDlg, 1148, path.c_str());
			SendDlgCommand(hWndDlg, IDOK);
		});

	} else {
		auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
		EXPECT_CALL(*pComdlg32, GetOpenFileNameW(_))
			.Times(1)
			.WillOnce(testing::DoDefault());
		EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
			.Times(0);

		MockComdlg32::gm_Files.emplace_back(path.native());
	}

	testAction();

	// 作成したファイルを削除する
	std::filesystem::remove(path, ec);
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, GetOpenFileName101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	std::unique_ptr<dialog::ModalDialogCloser> closer = nullptr;

	const auto testAction = [] () {
		// コマンドコードで、無理矢理動かす
		const auto hWnd = pcEditWnd->GetHwnd();
		FORWARD_WM_COMMAND(hWnd, F_LOADKEYMACRO, nullptr, 0, pcEditWnd->DispatchEvent);
	};

	if (GetParam()) {
		// 表示されたモーダルダイアログをキャンセルボタンで閉じる
		closer = std::make_unique<dialog::ModalDialogCloser>();

	} else {
		auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
		EXPECT_CALL(*pComdlg32, GetOpenFileNameW(_))
			.Times(1)
			.WillOnce(Return(FALSE));
		EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
			.Times(2)
			.WillRepeatedly(Return(0));
	}

	testAction();
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, GetOpenFileName102)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	std::unique_ptr<dialog::ModalDialogCloser> closer = nullptr;

	const auto testAction = [] () {
		SFilePath szPath{};
		CDlgOpenFile::getInstance()->DoModal_GetOpenFileName(szPath, EFilter::EFITER_NONE);
	};

	if (GetParam()) {
		// 表示されたモーダルダイアログをキャンセルボタンで閉じる
		closer = std::make_unique<dialog::ModalDialogCloser>();

	} else {
		auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
		EXPECT_CALL(*pComdlg32, GetOpenFileNameW(_))
			.Times(2)
			.WillOnce(Return(FALSE))
			.WillOnce(Return(FALSE));
		EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
			.Times(2)
			.WillOnce(Return(FNERR_INVALIDFILENAME))
			.WillOnce(Return(0));
	}

	testAction();
}

/*!
 * 名前を付けて保存ダイアログの表示テスト
 */
TEST_P(FileDialogTest, GetSaveFileName001)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	std::unique_ptr<dialog::ModalDialogCloser> closer = nullptr;

	const auto testAction = [] () {
		SFilePath szPath{};
		CDlgOpenFile::getInstance()->DoModal_GetSaveFileName(szPath);
	};

	const auto path = GetExeFileName().replace_filename(L"test.txt");
	const auto dummyPath = GetExeFileName().replace_filename(L"dummy.txt");

	std::error_code ec;

	// 上書き確認メッセージが出ないように、事前にパスを削除しておく
	std::filesystem::remove(path, ec);

	// パス解決できるようにファイルを作る
	std::filesystem::remove(dummyPath, ec);
	std::ofstream ofs{ dummyPath };
	ofs.close();

	if (GetParam()) {
		// 表示されたモーダルダイアログをキャンセルボタンで閉じる
		closer = std::make_unique<dialog::ModalDialogCloser>(saveDialogTitle0, [path] (HWND hWndDlg) {
			ASSERT_THAT(apiwrap::GetWindowTextW(hWndDlg), StrEq(saveDialogTitle));

			// 現状の値を決め打ち
			// ある日突然機能しなくなる可能性あるので注意
			const auto hWndEdit = dialog::FindFileNameEdit(hWndDlg);
			apiwrap::SetWindowTextW(hWndEdit, path.c_str());
			SendDlgCommand(hWndDlg, IDOK);
		});

	} else {
		auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
		EXPECT_CALL(*pComdlg32, GetSaveFileNameW(_))
			.Times(1)
			.WillOnce(testing::DoDefault());
		EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
			.Times(0);

		MockComdlg32::gm_Files.emplace_back(dummyPath.native());
	}

	testAction();

	// 保存したファイルを削除する
	std::filesystem::remove(dummyPath, ec);
}

/*!
 * 名前を付けて保存ダイアログの表示テスト
 */
TEST_P(FileDialogTest, GetSaveFileName101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じる
	std::unique_ptr<dialog::ModalDialogCloser> closer = nullptr;

	const auto testAction = [] () {
		// キーマクロ保存が使えるようにダミーマクロを登録する
		LPARAM lParams = 0L;
		pcSMacroMgr->Append(STAND_KEYMACRO, F_0, &lParams, &pcEditWnd->GetView(0));

		// コマンドコードで、無理矢理動かす
		const auto hWnd = pcEditWnd->GetHwnd();
		FORWARD_WM_COMMAND(hWnd, F_SAVEKEYMACRO, nullptr, 0, pcEditWnd->DispatchEvent);
	};

	if (GetParam()) {
		// 表示されたモーダルダイアログをキャンセルボタンで閉じる
		closer = std::make_unique<dialog::ModalDialogCloser>();

	} else {
		auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
		EXPECT_CALL(*pComdlg32, GetSaveFileNameW(_))
			.Times(2)
			.WillOnce(Return(FALSE))
			.WillOnce(Return(FALSE));
		EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
			.Times(2)
			.WillOnce(Return(FNERR_INVALIDFILENAME))
			.WillOnce(Return(0));
	}

	testAction();
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

/*!
 * @brief DlgOpenFailテストのパラメーター
 *
 * @param code CommDlgのエラーコード
 * @param name CommDlgのエラー識別子
 */
using DlgOpenFailTestParam = std::tuple<DWORD, std::wstring_view>;

/*!
 * DlgOpenFailテストのためのフィクスチャクラス
 */
struct DlgOpenFailTest : public ::testing::TestWithParam<DlgOpenFailTestParam> {

	/*!
	 * テストが実行される直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		User32::setInstance<MockUser32>();
		Comdlg32::setInstance<MockComdlg32>();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override
	{
		Comdlg32::resetInstance();
		User32::resetInstance();
	}
};

/*!
 * @brief DlgOpenFailのテスト
 */
TEST_P(DlgOpenFailTest, test)
{
	const auto code = std::get<0>(GetParam());
	const auto name = std::get<1>(GetParam());

	const auto expected = std::format(L"ダイアログが開けません。\n\nエラー:{:<21s}", name);

	auto pComdlg32 = static_cast<MockComdlg32*>(Comdlg32::getInstance());
	EXPECT_CALL(*pComdlg32, CommDlgExtendedError())
		.Times(1)
		.WillOnce(Return(code));

	auto pUser32 = (MockUser32*)User32::getInstance();
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(expected), _, _, _))
		.Times(1)
		.WillOnce(Return(IDOK));

	CallDlgOpenFail();
}

#pragma push_macro("CD_ERR_ENTRY")

#define CD_ERR_ENTRY(code)	DlgOpenFailTestParam{ code, L ## #code }

/*!
 * @brief パラメータテストをインスタンス化する
 */
INSTANTIATE_TEST_SUITE_P(CommDlgCodes
	, DlgOpenFailTest
	, ::testing::Values(
		CD_ERR_ENTRY(CDERR_DIALOGFAILURE),
		CD_ERR_ENTRY(CDERR_FINDRESFAILURE),
		CD_ERR_ENTRY(CDERR_NOHINSTANCE),
		CD_ERR_ENTRY(CDERR_INITIALIZATION),
		CD_ERR_ENTRY(CDERR_NOHOOK),
		CD_ERR_ENTRY(CDERR_LOCKRESFAILURE),
		CD_ERR_ENTRY(CDERR_NOTEMPLATE),
		CD_ERR_ENTRY(CDERR_LOADRESFAILURE),
		CD_ERR_ENTRY(CDERR_STRUCTSIZE),
		CD_ERR_ENTRY(CDERR_LOADSTRFAILURE),
		CD_ERR_ENTRY(FNERR_BUFFERTOOSMALL),
		CD_ERR_ENTRY(CDERR_MEMALLOCFAILURE),
		CD_ERR_ENTRY(FNERR_INVALIDFILENAME),
		CD_ERR_ENTRY(CDERR_MEMLOCKFAILURE),
		CD_ERR_ENTRY(FNERR_SUBCLASSFAILURE),

		// 未定義のエラーコードは以下固定値。
		DlgOpenFailTestParam{ 0x6000, L"UNKNOWN_ERRORCODE" }
	)
);

#pragma pop_macro("CD_ERR_ENTRY")

} // namespace dialog
