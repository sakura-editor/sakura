/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

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
#include "env/CHelpManager.h"

#include "MockShareDataAccessor.hpp"

/*!
 * 構築するだけ。
 */
TEST(CHelpManager, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	EXPECT_NO_THROW({ CHelpManager mgr(std::move(pShareDataAccessor)); });
}

TEST(CHelpManager, ExtWinHelpIsSet)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CHelpManager mgr(std::move(pShareDataAccessor));

	wcscpy_s(pDllShareData->m_Common.m_sHelper.m_szExtHelp, L"");
	EXPECT_FALSE(mgr.ExtWinHelpIsSet(nullptr));

	wcscpy_s(pDllShareData->m_Common.m_sHelper.m_szExtHelp, L"何か");
	EXPECT_TRUE(mgr.ExtWinHelpIsSet(nullptr));
	wcscpy_s(pDllShareData->m_Common.m_sHelper.m_szExtHelp, L"");

	auto typeConfig = std::make_unique<STypeConfig>();
	typeConfig->m_szExtHelp = L"何か";
	EXPECT_TRUE(mgr.ExtWinHelpIsSet(typeConfig.get()));

	typeConfig->m_szExtHelp = L"";
	EXPECT_FALSE(mgr.ExtWinHelpIsSet(typeConfig.get()));
}

TEST(CHelpManager, GetExtWinHelp)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CHelpManager mgr(std::move(pShareDataAccessor));

	wcscpy_s(pDllShareData->m_Common.m_sHelper.m_szExtHelp, L"何か1");
	EXPECT_EQ(pDllShareData->m_Common.m_sHelper.m_szExtHelp, mgr.GetExtWinHelp(nullptr));

	auto typeConfig = std::make_unique<STypeConfig>();
	typeConfig->m_szExtHelp = L"何か2";
	EXPECT_EQ(typeConfig->m_szExtHelp, mgr.GetExtWinHelp(typeConfig.get()));

	typeConfig->m_szExtHelp = L"";
	EXPECT_EQ(pDllShareData->m_Common.m_sHelper.m_szExtHelp, mgr.GetExtWinHelp(typeConfig.get()));
}

TEST(CHelpManager, ExtHTMLHelpIsSet)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CHelpManager mgr(std::move(pShareDataAccessor));

	wcscpy_s(pDllShareData->m_Common.m_sHelper.m_szExtHtmlHelp, L"");
	EXPECT_FALSE(mgr.ExtHTMLHelpIsSet(nullptr));

	wcscpy_s(pDllShareData->m_Common.m_sHelper.m_szExtHtmlHelp, L"何か");
	EXPECT_TRUE(mgr.ExtHTMLHelpIsSet(nullptr));
	wcscpy_s(pDllShareData->m_Common.m_sHelper.m_szExtHtmlHelp, L"");

	auto typeConfig = std::make_unique<STypeConfig>();

	typeConfig->m_szExtHtmlHelp = L"何か";
	EXPECT_TRUE(mgr.ExtHTMLHelpIsSet(typeConfig.get()));

	typeConfig->m_szExtHtmlHelp = L"";
	EXPECT_FALSE(mgr.ExtHTMLHelpIsSet(typeConfig.get()));
}

TEST(CHelpManager, GetExtHTMLHelp)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CHelpManager mgr(std::move(pShareDataAccessor));

	wcscpy_s(pDllShareData->m_Common.m_sHelper.m_szExtHtmlHelp, L"何か1");
	EXPECT_EQ(pDllShareData->m_Common.m_sHelper.m_szExtHtmlHelp, mgr.GetExtHTMLHelp(nullptr));

	auto typeConfig = std::make_unique<STypeConfig>();
	typeConfig->m_szExtHtmlHelp = L"何か2";
	EXPECT_EQ(typeConfig->m_szExtHtmlHelp, mgr.GetExtHTMLHelp(typeConfig.get()));

	typeConfig->m_szExtHtmlHelp = L"";
	EXPECT_EQ(pDllShareData->m_Common.m_sHelper.m_szExtHtmlHelp, mgr.GetExtHTMLHelp(typeConfig.get()));
}

TEST(CHelpManager, HTMLHelpIsSingle)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CHelpManager mgr(std::move(pShareDataAccessor));

	auto typeConfig = std::make_unique<STypeConfig>();
	typeConfig->m_bHtmlHelpIsSingle = true;

	pDllShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle = false;
	EXPECT_EQ(pDllShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle, mgr.HTMLHelpIsSingle(nullptr));

	typeConfig->m_szExtHtmlHelp = L"何か2";
	EXPECT_EQ(typeConfig->m_bHtmlHelpIsSingle, mgr.HTMLHelpIsSingle(typeConfig.get()));

	typeConfig->m_szExtHtmlHelp = L"";
	EXPECT_EQ(pDllShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle, mgr.HTMLHelpIsSingle(typeConfig.get()));
}
