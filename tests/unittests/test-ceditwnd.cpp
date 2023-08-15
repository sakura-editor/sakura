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
#include "window/CEditWnd.h"

#include "MockShareDataAccessor.hpp"

/*!
 * 編集ウインドウ、構築するだけ。
 */
TEST(CEditWnd, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CEditDoc doc(pShareDataAccessor);
	EXPECT_NO_THROW({ CEditWnd wnd(std::move(pShareDataAccessor)); });
}

TEST(CEditWnd, GetEditWnd)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CEditDoc doc(pShareDataAccessor);
	CEditWnd wnd(std::move(pShareDataAccessor));
	EXPECT_THAT(&GetEditWnd(), ::testing::Eq(&wnd));
}

TEST(CEditWnd, GetEditWnd_fail)
{
	EXPECT_ANY_THROW({ GetEditWnd(); });
}

TEST(CEditWnd, GetLogfont)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CEditDoc doc(pShareDataAccessor);
	CEditWnd wnd(std::move(pShareDataAccessor));

	doc.m_blfCurTemp = true;
	EXPECT_EQ(&doc.m_lfCur, &wnd.GetLogfont(true));

	auto& typeConfig = doc.m_cDocType.GetDocumentAttributeWrite();

	typeConfig.m_bUseTypeFont = true;
	doc.m_blfCurTemp = false;
	EXPECT_EQ(&doc.m_cDocType.GetDocumentAttribute().m_lf, &wnd.GetLogfont(true));

	EXPECT_EQ(&doc.m_cDocType.GetDocumentAttribute().m_lf, &wnd.GetLogfont(false));

	typeConfig.m_bUseTypeFont = false;
	EXPECT_EQ(&pDllShareData->m_Common.m_sView.m_lf, &wnd.GetLogfont(false));
}

TEST(CEditWnd, GetFontPointSize)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CEditDoc doc(pShareDataAccessor);
	CEditWnd wnd(std::move(pShareDataAccessor));

	doc.m_blfCurTemp = true;
	EXPECT_EQ(doc.m_nPointSizeCur, wnd.GetFontPointSize(true));

	auto& typeConfig = doc.m_cDocType.GetDocumentAttributeWrite();

	typeConfig.m_bUseTypeFont = true;
	doc.m_blfCurTemp = false;
	EXPECT_EQ(doc.m_cDocType.GetDocumentAttribute().m_nPointSize, wnd.GetFontPointSize(true));

	EXPECT_EQ(doc.m_cDocType.GetDocumentAttribute().m_nPointSize, wnd.GetFontPointSize(false));

	typeConfig.m_bUseTypeFont = false;
	EXPECT_EQ(pDllShareData->m_Common.m_sView.m_nPointSize, wnd.GetFontPointSize(false));
}

TEST(CEditWnd, GetLogfontCacheMode)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CEditDoc doc(pShareDataAccessor);
	CEditWnd wnd(std::move(pShareDataAccessor));

	doc.m_blfCurTemp = true;
	EXPECT_EQ(CWM_CACHE_LOCAL, wnd.GetLogfontCacheMode());

	auto& typeConfig = doc.m_cDocType.GetDocumentAttributeWrite();

	doc.m_blfCurTemp = false;
	typeConfig.m_bUseTypeFont = true;
	EXPECT_EQ(CWM_CACHE_LOCAL, wnd.GetLogfontCacheMode());

	typeConfig.m_bUseTypeFont = false;
	EXPECT_EQ(CWM_CACHE_SHARE, wnd.GetLogfontCacheMode());
}
