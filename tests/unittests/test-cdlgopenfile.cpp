/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#ifndef STRICT
#define STRICT 1
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */
#include <vector>
#include <tchar.h>
#include <Windows.h>
#include "util/design_template.h"
#include "dlg/CDlgOpenFile.h"

extern std::shared_ptr<IDlgOpenFile> New_CDlgOpenFile_CommonFileDialog();
extern std::shared_ptr<IDlgOpenFile> New_CDlgOpenFile_CommonItemDialog();

TEST(CDlgOpenFile, Construct)
{
	CDlgOpenFile cDlgOpenFile;
}

TEST(CDlgOpenFile, DISABLED_CommonItemDialogCreate)
{
	std::shared_ptr<IDlgOpenFile>impl = New_CDlgOpenFile_CommonItemDialog();
	impl->Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.txt",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST(CDlgOpenFile, DISABLED_CommonFileDialogCreate)
{
	std::shared_ptr<IDlgOpenFile>impl = New_CDlgOpenFile_CommonFileDialog();
	impl->Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.txt",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST(CDlgOpenFile, DISABLED_CommonItemDialogDefaltFilterLong)
{
	std::shared_ptr<IDlgOpenFile>impl = New_CDlgOpenFile_CommonItemDialog();
	// 落ちたり例外にならないこと
	impl->Create(
		GetModuleHandle(nullptr),
		nullptr,
		L".extension_250_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_LONG",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST(CDlgOpenFile, DISABLED_CommonFileDialogDefaltFilterLong)
{
	std::shared_ptr<IDlgOpenFile>impl = New_CDlgOpenFile_CommonFileDialog();
	// 落ちたり例外にならないこと
	impl->Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.extension_250_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_LONG",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST(CDlgOpenFile, DISABLED_CommonFileDialogDefaltFilterMany)
{
	std::shared_ptr<IDlgOpenFile>impl = New_CDlgOpenFile_CommonFileDialog();
	// 落ちたり例外にならないこと
	impl->Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.extension_50_0_long_long_long_long_long_long_LONG;*.extension_50_1_long_long_long_long_long_long_LONG;*.extension_50_2_long_long_long_long_long_long_LONG;*.extension_50_3_long_long_long_long_long_long_LONG;*.extension_50_4_long_long_long_long_long_long_LONG;*.extension_50_5_long_long_long_long_long_long_LONG;*.extension_50_6_long_long_long_long_long_long_LONG;*.extension_50_7_long_long_long_long_long_long_LONG;*.extension_50_8_long_long_long_long_long_long_LONG;*.extension_50_9_long_long_long_long_long_long_LONG",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST(CDlgOpenFile, DISABLED_CommonItemDialogDefaltFilterMany)
{
	std::shared_ptr<IDlgOpenFile>impl = New_CDlgOpenFile_CommonItemDialog();
	// 落ちたり例外にならないこと
	impl->Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.extension_50_0_long_long_long_long_long_long_LONG;*.extension_50_1_long_long_long_long_long_long_LONG;*.extension_50_2_long_long_long_long_long_long_LONG;*.extension_50_3_long_long_long_long_long_long_LONG;*.extension_50_4_long_long_long_long_long_long_LONG;*.extension_50_5_long_long_long_long_long_long_LONG;*.extension_50_6_long_long_long_long_long_long_LONG;*.extension_50_7_long_long_long_long_long_long_LONG;*.extension_50_8_long_long_long_long_long_long_LONG;*.extension_50_9_long_long_long_long_long_long_LONG",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}
