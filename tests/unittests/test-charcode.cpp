/*! @file */
/*
	Copyright (C) 2021, Sakura Editor Organization

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
#define NOMINMAX
#include "charset/charcode.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cstring>
#include <string>
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

TEST_F(CharWidthCache, IsHankaku)
{
	// IsHankaku は文字が半角であることが既知であれば即trueを返す。
	// 既知の文字の判定がキャッシュに依存していないことを確認するため、
	// ここではキャッシュを初期化しない。

	// Basic Latin と Control Codes (C0, C1)
	for (wchar_t ch = 0x00; ch <= 0xa0; ++ch) {
		EXPECT_TRUE(WCODE::IsHankaku(ch));
	}
	// 半角カタカナ
	for (wchar_t ch = 0xff61; ch <= 0xff9f; ++ch) {
		EXPECT_TRUE(WCODE::IsHankaku(ch));
	}

	// ここからは実行時のフォントを基に計算する文字。
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);

	// 漢字・ハングル・外字の場合、コード表の一番目の文字の幅がすべての文字に適用される。
	bool kanjiWidth = WCODE::CalcHankakuByFont(L'一');

	// CJK統合漢字。Unicode 5.1 以降の文字には未対応。
	for (wchar_t ch = 0x4e00; ch <= 0x9fbb; ++ch) {
		EXPECT_EQ(WCODE::IsHankaku(ch), kanjiWidth);
	}
	// CJK統合漢字拡張A。Unicode 13.0 の追加分には対応していない。
	for (wchar_t ch = 0x3400; ch <= 0x4d85; ++ch) {
		EXPECT_EQ(WCODE::IsHankaku(ch), kanjiWidth);
	}

	// ハングル
	bool hangulWidth = WCODE::CalcHankakuByFont(L'가');
	for (wchar_t ch = 0xac00; ch <= 0xd7a3; ++ch) {
		EXPECT_EQ(WCODE::IsHankaku(ch), hangulWidth);
	}

	// 外字
	bool privateUseWidth = WCODE::CalcHankakuByFont(0xe000);
	for (wchar_t ch = 0xe000; ch <= 0xe8ff; ++ch) {
		EXPECT_EQ(WCODE::IsHankaku(ch), privateUseWidth);
	}
}

TEST_F(CharWidthCache, IsZenkaku) {
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_LOCAL);
	InitCharWidthCache(lf1);

	EXPECT_EQ(WCODE::IsZenkaku(L'a'), !WCODE::IsHankaku(L'a'));
	EXPECT_EQ(WCODE::IsZenkaku(L'あ'), !WCODE::IsHankaku(L'あ'));
}

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

	// コントロールコードの幅を普通に計算すると1pxになることがあるため、
	// スペース・中黒の幅と比較して大きい方をとることになっている。
	SIZE sizeOfSpace;
	GetTextExtentPoint32(dc, L" ", 1, &sizeOfSpace);
	SIZE sizeOfNakaguro;
	GetTextExtentPoint32(dc, L"･", 1, &sizeOfNakaguro);

	// 理由はわからないが、NULとそれ以外で違う文字の幅を採用している。
	GetTextExtentPoint32(dc, L"\0", 1, &size);
	EXPECT_EQ(WCODE::CalcPxWidthByFont('\0'), std::max(size.cx, sizeOfSpace.cx));
	GetTextExtentPoint32(dc, L"\x01", 1, &size);
	EXPECT_EQ(WCODE::CalcPxWidthByFont('\x01'), std::max(size.cx, sizeOfNakaguro.cx));
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

TEST(charcode, IS_KEYWORD_CHAR)
{
	for (wchar_t ch = 0; ch < gm_keyword_char.size(); ++ch) {
		EXPECT_EQ(IS_KEYWORD_CHAR(ch),
			gm_keyword_char[ch] == CK_CSYM || gm_keyword_char[ch] == CK_UDEF);
	}
	EXPECT_FALSE(IS_KEYWORD_CHAR(static_cast<wchar_t>(-1)));
}

// 以下、関数が判定している文字がすべてASCII範囲内であれば総当たりテストを実施する。
// ASCII範囲外の文字が含まれる関数については適宜テストケースを手書きして対応。

TEST(charcode, IsAZ)
{
	const std::wstring chars = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	for (wchar_t ch = 0; ch < 128; ++ch) {
		EXPECT_EQ(WCODE::IsAZ(ch), chars.find(ch) != std::wstring::npos);
	}
}

TEST(charcode, Is09)
{
	const std::wstring chars = L"0123456789";
	for (wchar_t ch = 0; ch < 128; ++ch) {
		EXPECT_EQ(WCODE::Is09(ch), chars.find(ch) != std::wstring::npos);
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

	EXPECT_FALSE(WCODE::IsLineDelimiter(L'a', false));
	EXPECT_FALSE(WCODE::IsLineDelimiter(L'a', true));
}

TEST(charcode, IsLineDelimiterBasic)
{
	EXPECT_TRUE(WCODE::IsLineDelimiterBasic(0x0d));
	EXPECT_TRUE(WCODE::IsLineDelimiterBasic(0x0a));
	EXPECT_FALSE(WCODE::IsLineDelimiterBasic(0x85));
	EXPECT_FALSE(WCODE::IsLineDelimiterBasic(0x2028));
	EXPECT_FALSE(WCODE::IsLineDelimiterBasic(0x2029));
	EXPECT_FALSE(WCODE::IsLineDelimiterBasic(L'a'));
}

TEST(charcode, IsLineDelimiterExt) {
	EXPECT_TRUE(WCODE::IsLineDelimiterExt(0x0d));
	EXPECT_TRUE(WCODE::IsLineDelimiterExt(0x0a));
	EXPECT_TRUE(WCODE::IsLineDelimiterExt(0x85));
	EXPECT_TRUE(WCODE::IsLineDelimiterExt(0x2028));
	EXPECT_TRUE(WCODE::IsLineDelimiterExt(0x2029));
	EXPECT_FALSE(WCODE::IsLineDelimiterExt(L'a'));
}

TEST(charcode, IsWordDelimiter)
{
	EXPECT_TRUE(WCODE::IsWordDelimiter(L' '));
	EXPECT_TRUE(WCODE::IsWordDelimiter(L'\t'));
	EXPECT_TRUE(WCODE::IsWordDelimiter(L'　'));
	EXPECT_FALSE(WCODE::IsWordDelimiter(L'a'));
	EXPECT_FALSE(WCODE::IsWordDelimiter(L'あ'));
}

TEST(charcode, IsIndentChar)
{
	EXPECT_TRUE(WCODE::IsIndentChar(L' ', false));
	EXPECT_TRUE(WCODE::IsIndentChar(L'\t', false));
	EXPECT_FALSE(WCODE::IsIndentChar(L'　', false));

	EXPECT_TRUE(WCODE::IsIndentChar(L' ', true));
	EXPECT_TRUE(WCODE::IsIndentChar(L'\t', true));
	EXPECT_TRUE(WCODE::IsIndentChar(L'　', true));

	EXPECT_FALSE(WCODE::IsIndentChar(L'a', false));
	EXPECT_FALSE(WCODE::IsIndentChar(L'あ', false));
	EXPECT_FALSE(WCODE::IsIndentChar(L'a', true));
	EXPECT_FALSE(WCODE::IsIndentChar(L'あ', true));
}

TEST(charcode, IsBlank)
{
	EXPECT_TRUE(WCODE::IsBlank(L' '));
	EXPECT_TRUE(WCODE::IsBlank(L'\t'));
	EXPECT_TRUE(WCODE::IsBlank(L'　'));
	EXPECT_FALSE(WCODE::IsBlank(L'a'));
	EXPECT_FALSE(WCODE::IsBlank(L'あ'));
}

TEST(charcode, IsValidFileNameChar)
{
	const std::wstring chars(L"\0<>?\"|*", 7);
	for (wchar_t ch = 0; ch < 128; ++ch) {
		EXPECT_EQ(WCODE::IsValidFilenameChar(ch), chars.find(ch) == std::wstring::npos);
	}
}

TEST(charcode, IsTabAvailableCode)
{
	const std::wstring chars(L"\0\r\n\t", 4);
	for (wchar_t ch = 0; ch < 128; ++ch) {
		EXPECT_EQ(WCODE::IsTabAvailableCode(ch), chars.find(ch) == std::wstring::npos);
	}
}
