/*! @file */
/*
	Copyright (C) 2018-2019 Sakura Editor Organization

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

#ifndef NOMINMAX
#define NOMINMAX
#endif NOMINMAX

#include <tchar.h>
#include <Windows.h>

#include "basis/primitive.h"
#include "print/CPrint.h"

#include <cassert>
#include <limits>

/*! テストのベースとなる値 */
static constexpr MYDEVMODE myDevMode = {
	FALSE, //BOOL	m_bPrinterNotFound;
	{_T("m_szPrinterDriverName")}, //TCHAR	m_szPrinterDriverName[_MAX_PATH + 1];
	{_T("m_szPrinterDeviceName")}, //TCHAR	m_szPrinterDeviceName[_MAX_PATH + 1];
	{_T("m_szPrinterOutputName")}, //TCHAR	m_szPrinterOutputName[_MAX_PATH + 1];
	std::numeric_limits<DWORD>::min(), //DWORD	dmFields;
	std::numeric_limits<short>::min(), //short	dmOrientation;
	std::numeric_limits<short>::min(), //short	dmPaperSize;
	std::numeric_limits<short>::min(), //short	dmPaperLength;
	std::numeric_limits<short>::min(), //short	dmPaperWidth;
	std::numeric_limits<short>::min(), //short	dmScale;
	std::numeric_limits<short>::min(), //short	dmCopies;
	std::numeric_limits<short>::min(), //short	dmDefaultSource;
	std::numeric_limits<short>::min(), //short	dmPrintQuality;
	std::numeric_limits<short>::min(), //short	dmColor;
	std::numeric_limits<short>::min(), //short	dmDuplex;
	std::numeric_limits<short>::min(), //short	dmYResolution;
	std::numeric_limits<short>::min(), //short	dmTTOption;
	std::numeric_limits<short>::min(), //short	dmCollate;
	{_T("dmFormName")}, //BCHAR	dmFormName[CCHFORMNAME];
	std::numeric_limits<WORD>::min(), //WORD	dmLogPixels;
	std::numeric_limits<DWORD>::min(), //DWORD	dmBitsPerPel;
	std::numeric_limits<DWORD>::min(), //DWORD	dmPelsWidth;
	std::numeric_limits<DWORD>::min(), //DWORD	dmPelsHeight;
	std::numeric_limits<DWORD>::min(), //DWORD	dmDisplayFlags;
	std::numeric_limits<DWORD>::min(), //DWORD	dmDisplayFrequency;
};

/*!
 * @brief 等価演算子のテスト
 *  コピーコンストラクタ(trivial)でインスタンスを生成し、等価比較を行う
 */
TEST(MYDEVMODETest, operatorEqualByCopy)
{
	// コピーコンストラクタ経由で初期化
	MYDEVMODE value = myDevMode;

	EXPECT_TRUE(value == myDevMode);
	EXPECT_FALSE(value != myDevMode);
	ASSERT_EQ(myDevMode, value);
}

/*!
 * @brief 等価演算子のテスト
 *  自分自身との等価比較を行う
 */
TEST(MYDEVMODETest, operatorEqualBySelf)
{
	// 初期値はなんでもよいが一応指定しておく
	MYDEVMODE value = myDevMode;

	EXPECT_TRUE(value == value);
	EXPECT_FALSE(value != value);
	ASSERT_EQ(value, value);
}

/*!
 * @brief 等価演算子のテスト
 *  memcpyでメンバが割当たっていない領域を含めてコピーし、等価比較を行う
 */
TEST(MYDEVMODETest, operatorEqualByMemCpy)
{
	// デフォルトで初期化
	MYDEVMODE value;

	// メモリ領域全体をコピーしてまったく同じにする
	assert(sizeof(value) == sizeof(myDevMode));
	::memcpy_s(&value, sizeof(value), &myDevMode, sizeof(myDevMode));

	EXPECT_TRUE(value == myDevMode);
	EXPECT_FALSE(value != myDevMode);
	ASSERT_EQ(myDevMode, value);
}

/*!
 * @brief 否定の等価演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *
 *  合格条件：メンバの値が1つでも違ったら不一致を検出できること。
 */
TEST(MYDEVMODETest, operatorNotEqual)
{
	// デフォルトで初期化
	MYDEVMODE value;

	// メモリ領域全体をコピーしてまったく同じにする
	assert(sizeof(value) == sizeof(myDevMode));
	::memcpy_s(&value, sizeof(value), &myDevMode, sizeof(myDevMode));

	EXPECT_TRUE(value == myDevMode);
	EXPECT_EQ(myDevMode, value);

	value.m_bPrinterNotFound = TRUE;
	EXPECT_NE(myDevMode, value);
	value.m_bPrinterNotFound = myDevMode.m_bPrinterNotFound;
	EXPECT_EQ(myDevMode, value);

	::_tcscpy_s(value.m_szPrinterDriverName, _T("PrinterDriverName"));
	EXPECT_NE(myDevMode, value);
	::_tcscpy_s(value.m_szPrinterDriverName, myDevMode.m_szPrinterDriverName);
	EXPECT_EQ(myDevMode, value);

	::_tcscpy_s(value.m_szPrinterDeviceName, _T("PrinterDeviceName"));
	EXPECT_NE(myDevMode, value);
	::_tcscpy_s(value.m_szPrinterDeviceName, myDevMode.m_szPrinterDeviceName);
	EXPECT_EQ(myDevMode, value);

	::_tcscpy_s(value.m_szPrinterOutputName, _T("PrinterOutputName"));
	EXPECT_NE(myDevMode, value);
	::_tcscpy_s(value.m_szPrinterOutputName, myDevMode.m_szPrinterOutputName);
	EXPECT_EQ(myDevMode, value);

	value.dmFields = std::numeric_limits<decltype(value.dmFields)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmFields = myDevMode.dmFields;
	EXPECT_EQ(myDevMode, value);

	value.dmOrientation = std::numeric_limits<decltype(value.dmOrientation)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmOrientation = myDevMode.dmOrientation;
	EXPECT_EQ(myDevMode, value);

	value.dmPaperSize = std::numeric_limits<decltype(value.dmPaperSize)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmPaperSize = myDevMode.dmPaperSize;
	EXPECT_EQ(myDevMode, value);

	value.dmPaperLength = std::numeric_limits<decltype(value.dmPaperLength)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmPaperLength = myDevMode.dmPaperLength;
	EXPECT_EQ(myDevMode, value);

	value.dmPaperWidth = std::numeric_limits<decltype(value.dmPaperWidth)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmPaperWidth = myDevMode.dmPaperWidth;
	EXPECT_EQ(myDevMode, value);

	value.dmScale = std::numeric_limits<decltype(value.dmScale)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmScale = myDevMode.dmScale;
	EXPECT_EQ(myDevMode, value);

	value.dmCopies = std::numeric_limits<decltype(value.dmCopies)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmCopies = myDevMode.dmCopies;
	EXPECT_EQ(myDevMode, value);

	value.dmDefaultSource = std::numeric_limits<decltype(value.dmDefaultSource)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmDefaultSource = myDevMode.dmDefaultSource;
	EXPECT_EQ(myDevMode, value);

	value.dmPrintQuality = std::numeric_limits<decltype(value.dmPrintQuality)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmPrintQuality = myDevMode.dmPrintQuality;
	EXPECT_EQ(myDevMode, value);

	value.dmColor = std::numeric_limits<decltype(value.dmColor)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmColor = myDevMode.dmColor;
	EXPECT_EQ(myDevMode, value);

	value.dmDuplex = std::numeric_limits<decltype(value.dmDuplex)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmDuplex = myDevMode.dmDuplex;
	EXPECT_EQ(myDevMode, value);

	value.dmYResolution = std::numeric_limits<decltype(value.dmYResolution)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmYResolution = myDevMode.dmYResolution;
	EXPECT_EQ(myDevMode, value);

	value.dmTTOption = std::numeric_limits<decltype(value.dmTTOption)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmTTOption = myDevMode.dmTTOption;
	EXPECT_EQ(myDevMode, value);

	value.dmCollate = std::numeric_limits<decltype(value.dmCollate)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmCollate = myDevMode.dmCollate;
	EXPECT_EQ(myDevMode, value);

	::_tcscpy_s(value.dmFormName, _T("FormName"));
	EXPECT_NE(myDevMode, value);
	::_tcscpy_s(value.dmFormName, myDevMode.dmFormName);
	EXPECT_EQ(myDevMode, value);

	value.dmLogPixels = std::numeric_limits<decltype(value.dmLogPixels)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmLogPixels = myDevMode.dmLogPixels;
	EXPECT_EQ(myDevMode, value);

	value.dmBitsPerPel = std::numeric_limits<decltype(value.dmBitsPerPel)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmBitsPerPel = myDevMode.dmBitsPerPel;
	EXPECT_EQ(myDevMode, value);

	value.dmPelsWidth = std::numeric_limits<decltype(value.dmPelsWidth)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmPelsWidth = myDevMode.dmPelsWidth;
	EXPECT_EQ(myDevMode, value);

	value.dmPelsHeight = std::numeric_limits<decltype(value.dmPelsHeight)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmPelsHeight = myDevMode.dmPelsHeight;
	EXPECT_EQ(myDevMode, value);

	value.dmDisplayFlags = std::numeric_limits<decltype(value.dmDisplayFlags)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmDisplayFlags = myDevMode.dmDisplayFlags;
	EXPECT_EQ(myDevMode, value);

	value.dmDisplayFrequency = std::numeric_limits<decltype(value.dmDisplayFrequency)>::max();
	EXPECT_NE(myDevMode, value);
	value.dmDisplayFrequency = myDevMode.dmDisplayFrequency;
	EXPECT_EQ(myDevMode, value);
}
