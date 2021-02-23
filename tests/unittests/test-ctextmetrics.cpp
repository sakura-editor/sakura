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
#include <gtest/gtest.h>
#include "view/CTextMetrics.h"
#include <vector>
#include <Windows.h>

class CTextMetricsWithGDI : public testing::Test {
protected:
	CTextMetricsWithGDI() {
		lf1.lfCharSet = DEFAULT_CHARSET;
		std::wcscpy(lf1.lfFaceName, L"MS Gothic");

		dc = GetDC(nullptr);
		font = CreateFontIndirect(&lf1);
		oldFont = (HFONT)SelectObject(dc, font);

		GetTextExtentPoint32(dc, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size);
		GetTextMetrics(dc, &tm);
	}
	~CTextMetricsWithGDI() {
		SelectObject(dc, oldFont);
		DeleteObject(font);
		ReleaseDC(nullptr, dc);
	}
	SIZE size;
	TEXTMETRIC tm;
	LOGFONT lf1{};
	HDC dc;
	HFONT font;
	HFONT oldFont;
};

TEST_F(CTextMetricsWithGDI, Update1)
{
	// 引数に0を設定
	CTextMetrics metrics;
	metrics.Update(dc, font, 0, 0);
	EXPECT_EQ(metrics.GetHankakuWidth(), (size.cx / 26 + 1) / 2);
	EXPECT_EQ(metrics.GetHankakuHeight(), size.cy);
	EXPECT_EQ(metrics.GetCharHeightMarginByFontNo(0), 0);
	EXPECT_EQ(metrics.GetHankakuDx(), metrics.GetHankakuWidth());
	EXPECT_EQ(metrics.GetHankakuDy(), metrics.GetHankakuHeight());
	for (int i = 0; i < 64; ++i)
		EXPECT_EQ(metrics.GetDxArray_AllHankaku()[i], metrics.GetHankakuDx());

	// Updateに依存するその他の関数のテスト
	EXPECT_EQ(metrics.GetCharSpacing(), metrics.GetHankakuDx() - metrics.GetHankakuWidth());
	EXPECT_EQ(metrics.GetCharPxWidth(), 1);
	EXPECT_EQ(metrics.GetCharPxWidth(CLayoutInt(0)), 0);
	EXPECT_EQ(metrics.GetCharPxWidth(CLayoutInt(42)), 42);
	EXPECT_EQ((Int)metrics.GetLayoutXDefault(), metrics.GetHankakuDx());
	EXPECT_EQ((Int)metrics.GetLayoutXDefault(0), 0);
	EXPECT_EQ((Int)metrics.GetLayoutXDefault(42), metrics.GetHankakuDx() * 42);
	CTextMetrics metrics2;
	metrics.CopyTextMetricsStatus(&metrics2);
	EXPECT_EQ(metrics2.GetHankakuWidth(), metrics.GetHankakuWidth());
	EXPECT_EQ(metrics2.GetHankakuHeight(), metrics.GetHankakuHeight());
	EXPECT_EQ(metrics2.GetCharHeightMarginByFontNo(0), metrics.GetCharHeightMarginByFontNo(0));

	class FakeCache : public CCharWidthCache {
	public:
		int CalcPxWidthByFont(wchar_t ch) override { return 1; }
	} cache;
	EXPECT_EQ(metrics.CalcTextWidth3(L"a", 1, cache), 1);
	std::vector<int> v;
	metrics.GenerateDxArray2(&v, L"a", 1, cache);
	EXPECT_EQ(v[0], 1);
}

TEST_F(CTextMetricsWithGDI, Update2)
{
	// nLineSpaceに正の数を設定
	CTextMetrics metrics;
	metrics.Update(dc, font, 1000, 0);
	EXPECT_EQ(metrics.GetHankakuWidth(), (size.cx / 26 + 1) / 2);
	EXPECT_EQ(metrics.GetHankakuHeight(), size.cy);
	EXPECT_EQ(metrics.GetCharHeightMarginByFontNo(0), 0);
	EXPECT_EQ(metrics.GetHankakuDx(), metrics.GetHankakuWidth());
	EXPECT_EQ(metrics.GetHankakuDy(), metrics.GetHankakuHeight() + 1000);
}

TEST_F(CTextMetricsWithGDI, Update3)
{
	// nLineSpaceに負の数を設定
	CTextMetrics metrics;
	metrics.Update(dc, font, -1000, 0);
	EXPECT_EQ(metrics.GetHankakuWidth(), (size.cx / 26 + 1) / 2);
	EXPECT_EQ(metrics.GetHankakuHeight(), 1);
	EXPECT_EQ(metrics.GetCharHeightMarginByFontNo(0), 0);
	EXPECT_EQ(metrics.GetHankakuDx(), metrics.GetHankakuWidth());
	EXPECT_EQ(metrics.GetHankakuDy(), 1);
}

TEST_F(CTextMetricsWithGDI, Update4)
{
	// nColmSpaceに正の数を設定
	CTextMetrics metrics;
	metrics.Update(dc, font, 0, 1000);
	EXPECT_EQ(metrics.GetHankakuWidth(), (size.cx / 26 + 1) / 2);
	EXPECT_EQ(metrics.GetHankakuHeight(), size.cy);
	EXPECT_EQ(metrics.GetCharHeightMarginByFontNo(0), 0);
	EXPECT_EQ(metrics.GetHankakuDx(), metrics.GetHankakuWidth() + 1000);
	EXPECT_EQ(metrics.GetHankakuDy(), metrics.GetHankakuHeight());
}

TEST_F(CTextMetricsWithGDI, Update5)
{
	// nColmSpaceに負の数を設定
	CTextMetrics metrics;
	metrics.Update(dc, font, 0, -1000);
	EXPECT_EQ(metrics.GetHankakuWidth(), (size.cx / 26 + 1) / 2);
	EXPECT_EQ(metrics.GetHankakuHeight(), size.cy);
	EXPECT_EQ(metrics.GetCharHeightMarginByFontNo(0), 0);
	EXPECT_EQ(metrics.GetHankakuDx(), metrics.GetHankakuWidth() - 1000);
	EXPECT_EQ(metrics.GetHankakuDy(), metrics.GetHankakuHeight());
}

class FakeCache1 : public CCharWidthCache {
	int i = 0;
public:
	int CalcPxWidthByFont(wchar_t ch) override {
		return ++i;
	}
	int CalcPxWidthByFont2(const wchar_t* p) const override {
		return 10000;
	}
};

TEST(CTextMetrics, GenerateDxArray1)
{
	// 各文字の幅を CalcPxWidthByFont で計算して返す
	std::vector<int> v;
	FakeCache1 cache;
	const int* p = CTextMetrics::GenerateDxArray(&v, L"ab", 2, 0, 0, 0, 0, cache);
	EXPECT_EQ(p, v.data());
	EXPECT_EQ(v[0], 1);
	EXPECT_EQ(v[1], 2);
}

TEST(CTextMetrics, GenerateDxArray2)
{
	// 各文字の幅に nCharSpacing を足して返す
	std::vector<int> v;
	FakeCache1 cache;
	CTextMetrics::GenerateDxArray(&v, L"ab", 2, 0, 0, 0, 10, cache);
	EXPECT_EQ(v[0], 11);
	EXPECT_EQ(v[1], 12);
}

TEST(CTextMetrics, GenerateDxArray3)
{
	// サロゲートペアの幅は CalcPxWidthByFont2 で計算する
	std::vector<int> v;
	FakeCache1 cache;
	CTextMetrics::GenerateDxArray(&v, L"\xd83c\xdf38", 2, 0, 0, 0, 0, cache);
	EXPECT_EQ(v[0], 10000);
}

TEST(CTextMetrics, GenerateDxArray4)
{
	// サロゲートペアの幅に nCharSpacing を足して返す
	std::vector<int> v;
	FakeCache1 cache;
	CTextMetrics::GenerateDxArray(&v, L"\xd83c\xdf38", 2, 0, 0, 0, 10, cache);
	EXPECT_EQ(v[0], 10020);
}

TEST(CTextMetrics, GenerateDxArray5)
{
	// 対応する下位サロゲートのない上位サロゲートの幅は CalcPxWidthByFont を使って計算する
	std::vector<int> v;
	FakeCache1 cache;
	CTextMetrics::GenerateDxArray(&v, L"\xd83c,", 2, 0, 0, 0, 0, cache);
	EXPECT_EQ(v[0], 1);
	EXPECT_EQ(v[1], 2);
}

TEST(CTextMetrics, GenerateDxArray6)
{
	// 上位サロゲート片 + nCharSpacing の組み合わせ
	std::vector<int> v;
	FakeCache1 cache;
	CTextMetrics::GenerateDxArray(&v, L"\xd83c,", 2, 0, 0, 0, 10, cache);
	EXPECT_EQ(v[0], 21);
	EXPECT_EQ(v[1], 12);
}

TEST(CTextMetrics, GenerateDxArray7)
{
	// タブ幅計算のテスト
	std::vector<int> v;
	FakeCache1 cache;
	CTextMetrics::GenerateDxArray(&v, L"\t\t \t", 4, 10, 100, 1000, 0, cache);
	EXPECT_EQ(v[0], 100);
	EXPECT_EQ(v[1], 100);
	EXPECT_EQ(v[2], 1);
	EXPECT_EQ(v[3], 99);
}

TEST(CTextMetrics, CalcTextWidth)
{
	int dx[] = {1, 2, 3};
	EXPECT_EQ(CTextMetrics::CalcTextWidth(nullptr, 3, dx), 6);
}

TEST(CTextMetrics, CalcTextWidth2)
{
	class FakeCache : public CCharWidthCache {
		int i = 0;
	public:
		int CalcPxWidthByFont(wchar_t ch) override {
			return ++i;
		}
	};
	std::vector<int> v;
	FakeCache cache;
	EXPECT_EQ(CTextMetrics::CalcTextWidth2(L"abc", 3, 0, 0, v, cache), 6);
}
