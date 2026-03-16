/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "dlg/CDlgOpenFile.h"

#include "window/EditorTestSuite.hpp"

struct DlgOpenFileTest : public ::testing::Test, public window::EditorTestSuite {
	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpEditor();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		TearDownEditor();
	}
};

TEST_F(DlgOpenFileTest, Construct)
{
	CDlgOpenFile cDlgOpenFile;
}

TEST_F(DlgOpenFileTest, CommonItemDialogCreate)
{
	GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = true;

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

TEST_F(DlgOpenFileTest, CommonFileDialogCreate)
{
	GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = false;

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

TEST_F(DlgOpenFileTest, CommonItemDialogDefaltFilterLong)
{
	GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = true;

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

TEST_F(DlgOpenFileTest, CommonFileDialogDefaltFilterLong)
{
	GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = false;

	// 落ちたり例外にならないこと
	CDlgOpenFile cDlgOpenFile;
	cDlgOpenFile.Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.extension_250_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_LONG",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST_F(DlgOpenFileTest, CommonFileDialogDefaltFilterMany)
{
	GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = false;

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

TEST_F(DlgOpenFileTest, ommonItemDialogDefaltFilterMany)
{
	GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = true;

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
