/*! @file */
/*
	Copyright (C) 2018-2020 Sakura Editor Organization

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
#include "charset/charcode.h"
#include <gtest/gtest.h>
#include <cstring>
#include <Windows.h>

class CharWidthCache : public testing::Test {
protected:
	void SetUp() override {
		lf1 = LOGFONT();
		lf1.lfCharSet = DEFAULT_CHARSET;
		std::wcscpy(lf1.lfFaceName, L"MS Gothic");
		lf2 = LOGFONT();
		lf2.lfCharSet = DEFAULT_CHARSET;
		std::wcscpy(lf2.lfFaceName, L"Meiryo");

		dc = GetDC(nullptr);
		font = CreateFontIndirect(&lf1);
		oldFont = (HFONT)SelectObject(dc, font);
	}

	void TearDown() override {
		SelectObject(dc, oldFont);
		DeleteObject(font);
		ReleaseDC(nullptr, dc);
	}

	LOGFONT lf1;
	LOGFONT lf2;
	HDC dc;
	HFONT font;
	HFONT oldFont;
};

TEST_F(CharWidthCache, CalcHankakuByFont)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);

	EXPECT_TRUE(WCODE::CalcHankakuByFont(L'a'));
	EXPECT_FALSE(WCODE::CalcHankakuByFont(L'あ'));
}

TEST_F(CharWidthCache, CalcPxWidthByFont)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);

	SIZE size;
	GetTextExtentPoint32(dc, L"a", 1, &size);
	EXPECT_EQ(WCODE::CalcPxWidthByFont(L'a'), size.cx);
	GetTextExtentPoint32(dc, L"あ", 1, &size);
	EXPECT_EQ(WCODE::CalcPxWidthByFont(L'あ'), size.cx);
}

TEST_F(CharWidthCache, CalcPxWidthByFont2)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);

	SIZE size;
	GetTextExtentPoint32(dc, L"\xd83c\xdf38", 2, &size);
	EXPECT_EQ(WCODE::CalcPxWidthByFont2(L"\xd83c\xdf38"), size.cx);
}

TEST_F(CharWidthCache, FontNo)
{
	// 文字幅キャッシュには半角と全角とで異なるフォントを指定することができる。
	// ある文字に対して使用するべきフォントを問い合わせたとき、
	// 結果はフォント番号によって表現され、0ならば半角、1ならば全角を意味する。

	// InitCharWidthCache を使って初期化すると、
	// すべての文字に対して同一フォントを使用することになる。
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);

	EXPECT_EQ(WCODE::GetFontNo(L'a'), 0);
	EXPECT_EQ(WCODE::GetFontNo(L'あ'), 0);
	EXPECT_EQ(WCODE::GetFontNo2(0xd83c, 0xdf38), 0);

	// InitCharWidthCacheFromDC に2つの異なるLOGFONTを与えた場合、
	// それぞれ半角用と全角用として設定される。
	SelectCharWidthCache(CWM_FONT_PRINT, CWM_CACHE_LOCAL);
	LOGFONT lfs[] = {lf1, lf2};
	InitCharWidthCacheFromDC(lfs, CWM_FONT_PRINT, dc);

	EXPECT_EQ(WCODE::GetFontNo(L'a'), 0);
	EXPECT_EQ(WCODE::GetFontNo(0xe0), 1);  // 「à」。Latin-1 は全角扱いらしい…。
	EXPECT_EQ(WCODE::GetFontNo(L'あ'), 1);
	EXPECT_EQ(WCODE::GetFontNo2(0xd83c, 0xdf38), 1);
}
