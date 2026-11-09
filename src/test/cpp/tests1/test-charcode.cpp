/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "charset/charcode.h"
#include "pch.h"
#include <algorithm>
#include <cstring>
#include <string>

#if defined(_WIN32)	// GDI で文字幅を測るテスト。Mac 版は macos/core/tests/test-mac-charcode.cpp で測定関数を差し替えて確認する
#include <Windows.h>

class CharWidthCache : public testing::Test {
protected:
	void SetUp() override {
		lf1 = LOGFONT();
		lf1.lfCharSet = DEFAULT_CHARSET;
		std::wcscpy(lf1.lfFaceName, LTEXT("MS Gothic"));
		lf2 = LOGFONT();
		lf2.lfCharSet = DEFAULT_CHARSET;
		std::wcscpy(lf2.lfFaceName, LTEXT("Meiryo"));
		lf3 = LOGFONT();
		lf3.lfCharSet = DEFAULT_CHARSET;
		std::wcscpy(lf3.lfFaceName, LTEXT("MS Gothic")); // UD デジタル 教科書体 N-B
		lf3.lfPitchAndFamily = FIXED_PITCH;
		lf3.lfHeight = 10;
		lf3.lfWeight = FW_BOLD;

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
	LOGFONT lf3;
	HDC dc;
	HFONT font;
	HFONT oldFont;
};

TEST_F(CharWidthCache, IsHankaku)
{
	// IsHankaku は文字が半角であることが既知であれば即trueを返す。
	// 既知の文字の判定がキャッシュに依存していないことを確認するため、
	// ここではキャッシュを初期化しない。

	// Basic Latin と Control Codes (C0, C1)
	for (WCHAR ch = 0x00; ch <= 0xa0; ++ch) {
		EXPECT_TRUE(WCODE::IsHankaku(ch));
	}
	// 半角カタカナ
	for (WCHAR ch = 0xff61; ch <= 0xff9f; ++ch) {
		EXPECT_TRUE(WCODE::IsHankaku(ch));
	}

	// ここからは実行時のフォントを基に計算する文字。
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);
	CCharWidthCache& cache = GetCharWidthCache();

	// 漢字・ハングル・外字の場合、コード表の一番目の文字の幅がすべての文字に適用される。
	bool kanjiWidth = cache.CalcHankakuByFont(LCHAR('一'));

	// CJK統合漢字。Unicode 5.1 以降の文字には未対応。
	for (WCHAR ch = 0x4e00; ch <= 0x9fbb; ++ch) {
		EXPECT_EQ(WCODE::IsHankaku(ch, cache), kanjiWidth);
	}
	// CJK統合漢字拡張A。Unicode 13.0 の追加分には対応していない。
	for (WCHAR ch = 0x3400; ch <= 0x4d85; ++ch) {
		EXPECT_EQ(WCODE::IsHankaku(ch, cache), kanjiWidth);
	}

	// ハングル
	bool hangulWidth = cache.CalcHankakuByFont(LCHAR('가'));
	for (WCHAR ch = 0xac00; ch <= 0xd7a3; ++ch) {
		EXPECT_EQ(WCODE::IsHankaku(ch, cache), hangulWidth);
	}

	// 外字
	bool privateUseWidth = cache.CalcHankakuByFont(0xe000);
	for (WCHAR ch = 0xe000; ch <= 0xe8ff; ++ch) {
		EXPECT_EQ(WCODE::IsHankaku(ch, cache), privateUseWidth);
	}
}

TEST_F(CharWidthCache, IsZenkaku) {
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);

	EXPECT_EQ(WCODE::IsZenkaku(LCHAR('a')), !WCODE::IsHankaku(LCHAR('a')));
	EXPECT_EQ(WCODE::IsZenkaku(LCHAR('あ')), !WCODE::IsHankaku(LCHAR('あ')));
}

TEST_F(CharWidthCache, CalcHankakuByFont)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);
	CCharWidthCache& cache = GetCharWidthCache();

	EXPECT_TRUE(cache.CalcHankakuByFont(LCHAR('a')));
	EXPECT_FALSE(cache.CalcHankakuByFont(LCHAR('あ')));
}

TEST_F(CharWidthCache, CalcPxWidthByFont)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);
	CCharWidthCache& cache = GetCharWidthCache();

	SIZE size;
	GetTextExtentPoint32(dc, LTEXT("a"), 1, &size);
	EXPECT_EQ(cache.CalcPxWidthByFont(LCHAR('a')), size.cx);
	GetTextExtentPoint32(dc, LTEXT("あ"), 1, &size);
	EXPECT_EQ(cache.CalcPxWidthByFont(LCHAR('あ')), size.cx);

	// コントロールコードの幅を普通に計算すると1pxになることがあるため、
	// スペース・中黒の幅と比較して大きい方をとることになっている。
	SIZE sizeOfSpace;
	GetTextExtentPoint32(dc, LTEXT(" "), 1, &sizeOfSpace);
	SIZE sizeOfNakaguro;
	GetTextExtentPoint32(dc, LTEXT("･"), 1, &sizeOfNakaguro);

	// 理由はわからないが、NULとそれ以外で違う文字の幅を採用している。
	GetTextExtentPoint32(dc, LTEXT("\0"), 1, &size);
	EXPECT_EQ(cache.CalcPxWidthByFont('\0'), std::max(size.cx, sizeOfSpace.cx));
	GetTextExtentPoint32(dc, LTEXT("\x01"), 1, &size);
	EXPECT_EQ(cache.CalcPxWidthByFont('\x01'), std::max(size.cx, sizeOfNakaguro.cx));
}

TEST_F(CharWidthCache, CalcPxWidthByFont2)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);
	CCharWidthCache& cache = GetCharWidthCache();

	SIZE size;
	GetTextExtentPoint32(dc, LTEXT("\xd83c\xdf38"), 2, &size);
	EXPECT_EQ(cache.CalcPxWidthByFont2(LTEXT("\xd83c\xdf38")), size.cx);
}

TEST_F(CharWidthCache, CalcPxWidthByFont3)
{
	// フォントを作り直して設定する
	SelectObject(dc, oldFont);
	font = CreateFontIndirect(&lf3);
	oldFont = static_cast<HFONT>(SelectObject(dc, font));

	// 半角×2≠全角となることを確認する
	SIZE size = {};
	GetTextExtentPoint32(dc, LTEXT("a"), 1, &size);
	EXPECT_EQ(6, size.cx);
	GetTextExtentPoint32(dc, LTEXT("あ"), 1, &size);
	EXPECT_EQ(11, size.cx);

	// フォントを元に戻しておく
	font = static_cast<HFONT>(SelectObject(dc, oldFont));
	DeleteObject(font);

	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf3);
	CCharWidthCache& cache = GetCharWidthCache();

	// 半角×2＝全角となることを確認する
	constexpr auto halfWidth = 6;
	constexpr auto fullWidth = halfWidth * 2;
	EXPECT_EQ(cache.CalcPxWidthByFont(LCHAR('a')), halfWidth);
	EXPECT_EQ(cache.CalcPxWidthByFont(LCHAR('あ')), fullWidth);
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

	EXPECT_EQ(WCODE::GetFontNo(LCHAR('a')), 0);
	EXPECT_EQ(WCODE::GetFontNo(LCHAR('あ')), 0);
	EXPECT_EQ(WCODE::GetFontNo2(0xd83c, 0xdf38), 0);

	// InitCharWidthCacheFromDC に2つの異なるLOGFONTを与えた場合、
	// それぞれ半角用と全角用として設定される。
	SelectCharWidthCache(CWM_FONT_PRINT, CWM_CACHE_LOCAL);
	LOGFONT lfs[] = {lf1, lf2};
	InitCharWidthCacheFromDC(lfs, CWM_FONT_PRINT, dc);

	EXPECT_EQ(WCODE::GetFontNo(LCHAR('a')), 0);
	EXPECT_EQ(WCODE::GetFontNo(0xe0), 1);  // 「à」。Latin-1 は全角扱いらしい…。
	EXPECT_EQ(WCODE::GetFontNo(LCHAR('あ')), 1);
	EXPECT_EQ(WCODE::GetFontNo2(0xd83c, 0xdf38), 1);
}

TEST_F(CharWidthCache, GetCharWidthCache)
{
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	CCharWidthCache& edit1 = GetCharWidthCache();
	SelectCharWidthCache(CWM_FONT_MINIMAP, CWM_CACHE_LOCAL);
	CCharWidthCache& minimap = GetCharWidthCache();

	// 違うキャッシュ
	EXPECT_NE(&edit1, &minimap);

	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	CCharWidthCache& edit2 = GetCharWidthCache();

	// 同じキャッシュ
	EXPECT_EQ(&edit1, &edit2);
}

#endif // defined(_WIN32)

TEST(charcode, IS_KEYWORD_CHAR)
{
	for (WCHAR ch = 0; ch < gm_keyword_char.size(); ++ch) {
		EXPECT_EQ(IS_KEYWORD_CHAR(ch),
			gm_keyword_char[ch] == CK_CSYM || gm_keyword_char[ch] == CK_UDEF);
	}
	EXPECT_FALSE(IS_KEYWORD_CHAR(static_cast<WCHAR>(-1)));
}

// 以下、関数が判定している文字がすべてASCII範囲内であれば総当たりテストを実施する。
// ASCII範囲外の文字が含まれる関数については適宜テストケースを手書きして対応。

TEST(charcode, IsAZ)
{
	const sakura::wstring chars = LTEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	for (WCHAR ch = 0; ch < 128; ++ch) {
		EXPECT_EQ(WCODE::IsAZ(ch), chars.find(ch) != sakura::wstring::npos);
	}
}

TEST(charcode, Is09)
{
	const sakura::wstring chars = LTEXT("0123456789");
	for (WCHAR ch = 0; ch < 128; ++ch) {
		EXPECT_EQ(WCODE::Is09(ch), chars.find(ch) != sakura::wstring::npos);
	}
}

TEST(charcode, IsInRange)
{
	EXPECT_FALSE(WCODE::IsInRange(1, 2, 4));
	EXPECT_TRUE(WCODE::IsInRange(2, 2, 4));
	EXPECT_TRUE(WCODE::IsInRange(3, 2, 4));
	EXPECT_TRUE(WCODE::IsInRange(4, 2, 4));
	EXPECT_FALSE(WCODE::IsInRange(5, 2, 4));
}

TEST(charcode, IsLineDelimiter)
{
	EXPECT_TRUE(WCODE::IsLineDelimiter(0x0d, false));
	EXPECT_TRUE(WCODE::IsLineDelimiter(0x0a, false));
	EXPECT_FALSE(WCODE::IsLineDelimiter(0x85, false));
	EXPECT_FALSE(WCODE::IsLineDelimiter(0x2028, false));
	EXPECT_FALSE(WCODE::IsLineDelimiter(0x2029, false));

	EXPECT_TRUE(WCODE::IsLineDelimiter(0x0d, true));
	EXPECT_TRUE(WCODE::IsLineDelimiter(0x0a, true));
	EXPECT_TRUE(WCODE::IsLineDelimiter(0x85, true));
	EXPECT_TRUE(WCODE::IsLineDelimiter(0x2028, true));
	EXPECT_TRUE(WCODE::IsLineDelimiter(0x2029, true));

	EXPECT_FALSE(WCODE::IsLineDelimiter(LCHAR('a'), false));
	EXPECT_FALSE(WCODE::IsLineDelimiter(LCHAR('a'), true));
}

TEST(charcode, IsLineDelimiterBasic)
{
	EXPECT_TRUE(WCODE::IsLineDelimiterBasic(0x0d));
	EXPECT_TRUE(WCODE::IsLineDelimiterBasic(0x0a));
	EXPECT_FALSE(WCODE::IsLineDelimiterBasic(0x85));
	EXPECT_FALSE(WCODE::IsLineDelimiterBasic(0x2028));
	EXPECT_FALSE(WCODE::IsLineDelimiterBasic(0x2029));
	EXPECT_FALSE(WCODE::IsLineDelimiterBasic(LCHAR('a')));
}

TEST(charcode, IsLineDelimiterExt) {
	EXPECT_TRUE(WCODE::IsLineDelimiterExt(0x0d));
	EXPECT_TRUE(WCODE::IsLineDelimiterExt(0x0a));
	EXPECT_TRUE(WCODE::IsLineDelimiterExt(0x85));
	EXPECT_TRUE(WCODE::IsLineDelimiterExt(0x2028));
	EXPECT_TRUE(WCODE::IsLineDelimiterExt(0x2029));
	EXPECT_FALSE(WCODE::IsLineDelimiterExt(LCHAR('a')));
}

TEST(charcode, IsWordDelimiter)
{
	EXPECT_TRUE(WCODE::IsWordDelimiter(LCHAR(' ')));
	EXPECT_TRUE(WCODE::IsWordDelimiter(LCHAR('\t')));
	EXPECT_TRUE(WCODE::IsWordDelimiter(LCHAR('　')));
	EXPECT_FALSE(WCODE::IsWordDelimiter(LCHAR('a')));
	EXPECT_FALSE(WCODE::IsWordDelimiter(LCHAR('あ')));
}

TEST(charcode, IsIndentChar)
{
	EXPECT_TRUE(WCODE::IsIndentChar(LCHAR(' '), false));
	EXPECT_TRUE(WCODE::IsIndentChar(LCHAR('\t'), false));
	EXPECT_FALSE(WCODE::IsIndentChar(LCHAR('　'), false));

	EXPECT_TRUE(WCODE::IsIndentChar(LCHAR(' '), true));
	EXPECT_TRUE(WCODE::IsIndentChar(LCHAR('\t'), true));
	EXPECT_TRUE(WCODE::IsIndentChar(LCHAR('　'), true));

	EXPECT_FALSE(WCODE::IsIndentChar(LCHAR('a'), false));
	EXPECT_FALSE(WCODE::IsIndentChar(LCHAR('あ'), false));
	EXPECT_FALSE(WCODE::IsIndentChar(LCHAR('a'), true));
	EXPECT_FALSE(WCODE::IsIndentChar(LCHAR('あ'), true));
}

TEST(charcode, IsBlank)
{
	EXPECT_TRUE(WCODE::IsBlank(LCHAR(' ')));
	EXPECT_TRUE(WCODE::IsBlank(LCHAR('\t')));
	EXPECT_TRUE(WCODE::IsBlank(LCHAR('　')));
	EXPECT_FALSE(WCODE::IsBlank(LCHAR('a')));
	EXPECT_FALSE(WCODE::IsBlank(LCHAR('あ')));
}

TEST(charcode, IsValidFileNameChar)
{
	const sakura::wstring chars(LTEXT("\0<>?\"|*"), 7);
	for (WCHAR ch = 0; ch < 128; ++ch) {
		EXPECT_EQ(WCODE::IsValidFilenameChar(ch), chars.find(ch) == sakura::wstring::npos);
	}
}

TEST(charcode, IsTabAvailableCode)
{
	const sakura::wstring chars(LTEXT("\0\r\n\t"), 4);
	for (WCHAR ch = 0; ch < 128; ++ch) {
		EXPECT_EQ(WCODE::IsTabAvailableCode(ch), chars.find(ch) == sakura::wstring::npos);
	}
}
