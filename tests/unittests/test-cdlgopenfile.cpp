/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
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
