/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "dlg/CDlgOpenFile.h"

#include "window/EditorTestSuite.hpp"

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
		SetUpUia();

		SetUpEditor();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		TearDownEditor();

		TearDownUia();
	}

	/*!
	 * テストが実行された直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		// テスト設定を反映する
		GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = GetParam();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override
	{
		// 設定を元に戻す
		GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = true;
	}
};

TEST_P(FileDialogTest, Create001)
{
	// 落ちたり例外にならないこと
	CDlgOpenFile cDlgOpenFile;
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
	CDlgOpenFile cDlgOpenFile;
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
	CDlgOpenFile cDlgOpenFile;
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
TEST_P(FileDialogTest, DoModalOpenDlg101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	// コマンドコードで、無理矢理動かす
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_FILEOPEN, nullptr, 0, pcEditWnd->DispatchEvent);
}

/*!
 * 名前を付けて保存ダイアログの表示テスト
 */
TEST_P(FileDialogTest, DoModalSaveDlg101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	// コマンドコードで、無理矢理動かす
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_FILESAVEAS_DIALOG, nullptr, 0, pcEditWnd->DispatchEvent);
}

/*!
 * ファイルを開くダイアログの表示テスト
 */
TEST_P(FileDialogTest, GetOpenFileName101)
{
	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

	// コマンドコードで、無理矢理動かす
	const auto hWnd = pcEditWnd->GetHwnd();
	FORWARD_WM_COMMAND(hWnd, F_LOADKEYMACRO, nullptr, 0, pcEditWnd->DispatchEvent);
}

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

	// 表示されたモーダルダイアログをキャンセルボタンで閉じるようにする
	dialog::ModalDialogCloser closer;

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

} // namespace window
