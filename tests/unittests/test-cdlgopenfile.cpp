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
#include <gtest/gtest.h>
#ifndef STRICT
#define STRICT 1
#endif
#include <vector>
#include <tchar.h>
#include <Windows.h>
#include "util/design_template.h"
#include "dlg/CDlgOpenFile.h"

#include "doc/CEditDoc.h"
#include "view/CEditView.h"

#include "MockShareDataAccessor.hpp"

extern std::shared_ptr<IDlgOpenFile> New_CDlgOpenFile_CommonFileDialog(std::shared_ptr<ShareDataAccessor> ShareDataAccessor_ = std::make_shared<ShareDataAccessor>());
extern std::shared_ptr<IDlgOpenFile> New_CDlgOpenFile_CommonItemDialog(std::shared_ptr<ShareDataAccessor> ShareDataAccessor_ = std::make_shared<ShareDataAccessor>());

/*!
 * ファイルを開くダイアログ、構築するだけ。
 */
TEST(CDlgOpenFile, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	EXPECT_NO_THROW({ CDlgOpenFile dlg(std::move(pShareDataAccessor)); });
}

TEST(CDlgOpenFile_CommonFileDialog, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	pDllShareData->m_Common.m_sEdit.m_bVistaStyleFileDialog = false;
	CDlgOpenFile dlg(std::move(pShareDataAccessor));
	EXPECT_FALSE(dlg.IsItemDialog());
}

TEST(CDlgOpenFile_CommonItemDialog, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	pDllShareData->m_Common.m_sEdit.m_bVistaStyleFileDialog = true;
	CDlgOpenFile dlg(std::move(pShareDataAccessor));
	EXPECT_TRUE(dlg.IsItemDialog());
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
