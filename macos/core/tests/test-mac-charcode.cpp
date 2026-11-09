/*! @file
	@brief Mac 版：文字宽度缓存（charset/charcode.cpp 的 Mac 分支）的测试

	对应原版 test-charcode.cpp 中用 GDI 测宽度的 CharWidthCache 测试。
	Mac 版通过 sakura_platform_hooks.h 的测量函数取得宽度，这里换成固定宽度的假测量函数来确认计算规则。
*/
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "charset/charcode.h"
#include "sakura_platform_hooks.h"

#include <cstring>

namespace {

//! 假的测量函数：ASCII 为 6，字号 10 的“あ”为 11（不是半角的 2 倍），其余为 12，控制字符为 1
int FakeMeasure(const LOGFONT& font, const WCHAR* text, size_t length)
{
	int w = 0;
	for (size_t i = 0; i < length; ++i) {
		const WCHAR c = text[i];
		if (c < 0x20) w += 1;
		else if (c < 0x80) w += 6;
		else if (c == u'あ' && font.lfHeight == 10) w += 11;
		else w += 12;
	}
	return w;
}

class MacCharWidthCache : public testing::Test {
protected:
	void SetUp() override {
		sakura::platform::SetTextMeasureFunction(&FakeMeasure);
		lf1 = LOGFONT();
		lf1.lfCharSet = DEFAULT_CHARSET;
		std::copy_n(u"Osaka", 6, lf1.lfFaceName);
		lf2 = lf1;
		std::copy_n(u"Hiragino Sans", 14, lf2.lfFaceName);
		lf3 = lf1;
		lf3.lfPitchAndFamily = FIXED_PITCH;
		lf3.lfHeight = 10;
	}
	void TearDown() override {
		sakura::platform::SetTextMeasureFunction(nullptr);
	}
	LOGFONT lf1;
	LOGFONT lf2;
	LOGFONT lf3;
};

} // namespace

TEST_F(MacCharWidthCache, CalcHankakuByFont)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);
	CCharWidthCache& cache = GetCharWidthCache();
	EXPECT_TRUE(cache.CalcHankakuByFont(LCHAR('a')));
	EXPECT_FALSE(cache.CalcHankakuByFont(LCHAR('あ')));
}

TEST_F(MacCharWidthCache, CalcPxWidthByFont)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);
	CCharWidthCache& cache = GetCharWidthCache();
	EXPECT_EQ(cache.CalcPxWidthByFont(LCHAR('a')), 6);
	EXPECT_EQ(cache.CalcPxWidthByFont(LCHAR('あ')), 12);
	// 控制字符取空格（NUL）或中点（其他）的宽度中较大的一方
	EXPECT_EQ(cache.CalcPxWidthByFont(LCHAR('\0')), 6);
	EXPECT_EQ(cache.CalcPxWidthByFont(LCHAR('\x01')), 12);
}

TEST_F(MacCharWidthCache, CalcPxWidthByFont2)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);
	CCharWidthCache& cache = GetCharWidthCache();
	EXPECT_EQ(cache.CalcPxWidthByFont2(LTEXT("\xd83c\xdf38")), 24);
}

TEST_F(MacCharWidthCache, CalcPxWidthByFont3)
{
	// 等宽字体中比半角宽的字符，宽度统一为半角的 2 倍
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf3);
	CCharWidthCache& cache = GetCharWidthCache();
	EXPECT_EQ(cache.CalcPxWidthByFont(LCHAR('a')), 6);
	EXPECT_EQ(cache.CalcPxWidthByFont(LCHAR('あ')), 12);
}

TEST_F(MacCharWidthCache, FontNo)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);
	EXPECT_EQ(WCODE::GetFontNo(LCHAR('a')), 0);
	EXPECT_EQ(WCODE::GetFontNo(LCHAR('あ')), 0);
	EXPECT_EQ(WCODE::GetFontNo2(0xd83c, 0xdf38), 0);

	SelectCharWidthCache(CWM_FONT_PRINT, CWM_CACHE_LOCAL);
	LOGFONT lfs[] = { lf1, lf2 };
	InitCharWidthCacheFromDC(lfs, CWM_FONT_PRINT, nullptr);
	EXPECT_EQ(WCODE::GetFontNo(LCHAR('a')), 0);
	EXPECT_EQ(WCODE::GetFontNo(0xe0), 1);
	EXPECT_EQ(WCODE::GetFontNo(LCHAR('あ')), 1);
	EXPECT_EQ(WCODE::GetFontNo2(0xd83c, 0xdf38), 1);
}

TEST_F(MacCharWidthCache, GetCharWidthCache)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	CCharWidthCache& edit1 = GetCharWidthCache();
	SelectCharWidthCache(CWM_FONT_MINIMAP, CWM_CACHE_LOCAL);
	CCharWidthCache& minimap = GetCharWidthCache();
	EXPECT_NE(&edit1, &minimap);
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	EXPECT_EQ(&edit1, &GetCharWidthCache());
}
