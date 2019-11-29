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
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>

#include "basis/primitive.h"
#include "print/CPrint.h"

#include <cassert>
#include <limits>
#include <string>

/*! テストのベースとなる値 */
static constexpr MYDEVMODE myDevMode = {
	FALSE, //BOOL	m_bPrinterNotFound;
	{L"m_szPrinterDriverName"}, //WCHAR	m_szPrinterDriverName[_MAX_PATH + 1];
	{L"m_szPrinterDeviceName"}, //WCHAR	m_szPrinterDeviceName[_MAX_PATH + 1];
	{L"m_szPrinterOutputName"}, //WCHAR	m_szPrinterOutputName[_MAX_PATH + 1];
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
	{L"dmFormName"}, //BCHAR	dmFormName[CCHFORMNAME];
	std::numeric_limits<WORD>::min(), //WORD	dmLogPixels;
	std::numeric_limits<DWORD>::min(), //DWORD	dmBitsPerPel;
	std::numeric_limits<DWORD>::min(), //DWORD	dmPelsWidth;
	std::numeric_limits<DWORD>::min(), //DWORD	dmPelsHeight;
	std::numeric_limits<DWORD>::min(), //DWORD	dmDisplayFlags;
	std::numeric_limits<DWORD>::min(), //DWORD	dmDisplayFrequency;
};

/*!
 * @brief 等価演算子のテスト
 */
TEST(MYDEVMODETest, operatorEqual)
{
	ASSERT_EQ(myDevMode, MYDEVMODE(myDevMode)); // コピーと比較
	ASSERT_EQ(myDevMode, myDevMode); // 自分自身と比較
}


/*!
 * @brief 否定の等価演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *
 *  合格条件：メンバの値が1つでも違ったら不一致を検出できること。
 */
TEST(MYDEVMODETest, operatorNotEqual)
{
	MYDEVMODE value = myDevMode;
	ASSERT_EQ(myDevMode, value);

	value.m_bPrinterNotFound = ! myDevMode.m_bPrinterNotFound;
	ASSERT_NE(myDevMode, value);
	value.m_bPrinterNotFound = myDevMode.m_bPrinterNotFound;

	value.m_szPrinterDriverName[0] = myDevMode.m_szPrinterDriverName[0]+1;
	ASSERT_NE(myDevMode, value);
	value.m_szPrinterDriverName[0] = myDevMode.m_szPrinterDriverName[0];

	value.m_szPrinterDeviceName[0] = myDevMode.m_szPrinterDeviceName[0]+1;
	ASSERT_NE(myDevMode, value);
	value.m_szPrinterDeviceName[0] = myDevMode.m_szPrinterDeviceName[0];

	value.m_szPrinterOutputName[0] = myDevMode.m_szPrinterOutputName[0]+1;
	ASSERT_NE(myDevMode, value);
	value.m_szPrinterOutputName[0] = myDevMode.m_szPrinterOutputName[0];

	value.dmFields = myDevMode.dmFields+1;
	ASSERT_NE(myDevMode, value);
	value.dmFields = myDevMode.dmFields;

	value.dmOrientation = myDevMode.dmOrientation+1;
	ASSERT_NE(myDevMode, value);
	value.dmOrientation = myDevMode.dmOrientation;

	value.dmPaperSize = myDevMode.dmPaperSize+1;
	ASSERT_NE(myDevMode, value);
	value.dmPaperSize = myDevMode.dmPaperSize;

	value.dmPaperLength = myDevMode.dmPaperLength+1;
	ASSERT_NE(myDevMode, value);
	value.dmPaperLength = myDevMode.dmPaperLength;

	value.dmPaperWidth = myDevMode.dmPaperWidth+1;
	ASSERT_NE(myDevMode, value);
	value.dmPaperWidth = myDevMode.dmPaperWidth;

	value.dmScale = myDevMode.dmScale+1;
	ASSERT_NE(myDevMode, value);
	value.dmScale = myDevMode.dmScale;

	value.dmCopies = myDevMode.dmCopies+1;
	ASSERT_NE(myDevMode, value);
	value.dmCopies = myDevMode.dmCopies;

	value.dmDefaultSource = myDevMode.dmDefaultSource+1;
	ASSERT_NE(myDevMode, value);
	value.dmDefaultSource = myDevMode.dmDefaultSource;

	value.dmPrintQuality = myDevMode.dmPrintQuality+1;
	ASSERT_NE(myDevMode, value);
	value.dmPrintQuality = myDevMode.dmPrintQuality;

	value.dmColor = myDevMode.dmColor+1;
	ASSERT_NE(myDevMode, value);
	value.dmColor = myDevMode.dmColor;

	value.dmDuplex = myDevMode.dmDuplex+1;
	ASSERT_NE(myDevMode, value);
	value.dmDuplex = myDevMode.dmDuplex;

	value.dmYResolution = myDevMode.dmYResolution+1;
	ASSERT_NE(myDevMode, value);
	value.dmYResolution = myDevMode.dmYResolution;

	value.dmTTOption = myDevMode.dmTTOption+1;
	ASSERT_NE(myDevMode, value);
	value.dmTTOption = myDevMode.dmTTOption;

	value.dmCollate = myDevMode.dmCollate+1;
	ASSERT_NE(myDevMode, value);
	value.dmCollate = myDevMode.dmCollate;

	value.dmFormName[0] = myDevMode.dmFormName[0]+1;
	ASSERT_NE(myDevMode, value);
	value.dmFormName[0] = myDevMode.dmFormName[0];

	value.dmLogPixels = myDevMode.dmLogPixels+1;
	ASSERT_NE(myDevMode, value);
	value.dmLogPixels = myDevMode.dmLogPixels;

	value.dmBitsPerPel = myDevMode.dmBitsPerPel+1;
	ASSERT_NE(myDevMode, value);
	value.dmBitsPerPel = myDevMode.dmBitsPerPel;

	value.dmPelsWidth = myDevMode.dmPelsWidth+1;
	ASSERT_NE(myDevMode, value);
	value.dmPelsWidth = myDevMode.dmPelsWidth;

	value.dmPelsHeight = myDevMode.dmPelsHeight+1;
	ASSERT_NE(myDevMode, value);
	value.dmPelsHeight = myDevMode.dmPelsHeight;

	value.dmDisplayFlags = myDevMode.dmDisplayFlags+1;
	ASSERT_NE(myDevMode, value);
	value.dmDisplayFlags = myDevMode.dmDisplayFlags;

	value.dmDisplayFrequency = myDevMode.dmDisplayFrequency+1;
	ASSERT_NE(myDevMode, value);
	value.dmDisplayFrequency = myDevMode.dmDisplayFrequency;
}

/*!
 * @brief 文字列メンバに関する比較演算子のテスト
 *  バッファ長と文字列長が一致するようなエッジケースの確認
 */
TEST(MYDEVMODETest, stringMemberComparisonEdgeCase)
{
	// memset で初期化する(手抜き)。
	// value と other はアドレス以外同一。
	// ４つの文字列メンバにヌル終端はない。
	MYDEVMODE value, other;
	memset(&value, 'a', sizeof value);
	memset(&other, 'a', sizeof other);
	ASSERT_EQ(value, other); // 下手な文字列比較をしているとアクセス違反を起こすかも(起こしてはいけない)。

	// 一方の文字列バッファの末尾をNUL終端する。
	value.m_szPrinterDriverName[_countof(value.m_szPrinterDriverName)-1] =
	value.m_szPrinterDeviceName[_countof(value.m_szPrinterDeviceName)-1] =
	value.m_szPrinterOutputName[_countof(value.m_szPrinterOutputName)-1] =
	value.dmFormName[_countof(value.dmFormName)-1] = L'0';

	// ヌル終端があるべき場所にある文字の違いにより
	// 構造体が異なると判断されるなら、比較演算子は読み過ぎている。
	// 逆に、そこが無視できているならアクセス違反を起こすようなミスはない。
	ASSERT_EQ(value, other);
}
