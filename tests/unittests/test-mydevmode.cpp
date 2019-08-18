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

#include "doctest.h"

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
TEST_CASE("operatorEqualByCopy")
{
	// コピーコンストラクタ経由で初期化
	MYDEVMODE value = myDevMode;

	CHECK(value == myDevMode);
	CHECK_FALSE(value != myDevMode);
	REQUIRE(myDevMode == value);
}

/*!
 * @brief 等価演算子のテスト
 *  自分自身との等価比較を行う
 */
TEST_CASE("operatorEqualBySelf")
{
	// 初期値はなんでもよいが一応指定しておく
	MYDEVMODE value = myDevMode;

	CHECK(value == value);
	CHECK_FALSE(value != value);
	REQUIRE(value == value);
}

/*!
 * @brief 等価演算子のテスト
 *  memcpyでメンバが割当たっていない領域を含めてコピーし、等価比較を行う
 */
TEST_CASE("operatorEqualByMemCpy")
{
	// デフォルトで初期化
	MYDEVMODE value;

	// メモリ領域全体をコピーしてまったく同じにする
	assert(sizeof(value) == sizeof(myDevMode));
	::memcpy_s(&value, sizeof(value), &myDevMode, sizeof(myDevMode));

	CHECK(value == myDevMode);
	CHECK_FALSE(value != myDevMode);
	REQUIRE(myDevMode == value);
}

/*!
 * @brief 否定の等価演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *
 *  合格条件：メンバの値が1つでも違ったら不一致を検出できること。
 */
TEST_CASE("operatorNotEqual")
{
	// デフォルトで初期化
	MYDEVMODE value;

	// メモリ領域全体をコピーしてまったく同じにする
	assert(sizeof(value) == sizeof(myDevMode));
	::memcpy_s(&value, sizeof(value), &myDevMode, sizeof(myDevMode));

	CHECK(value == myDevMode);
	CHECK(myDevMode == value);

	value.m_bPrinterNotFound = TRUE;
	CHECK(myDevMode != value);
	value.m_bPrinterNotFound = myDevMode.m_bPrinterNotFound;
	CHECK(myDevMode == value);

	::_tcscpy_s(value.m_szPrinterDriverName, _T("PrinterDriverName"));
	CHECK(myDevMode != value);
	::_tcscpy_s(value.m_szPrinterDriverName, myDevMode.m_szPrinterDriverName);
	CHECK(myDevMode == value);

	::_tcscpy_s(value.m_szPrinterDeviceName, _T("PrinterDeviceName"));
	CHECK(myDevMode != value);
	::_tcscpy_s(value.m_szPrinterDeviceName, myDevMode.m_szPrinterDeviceName);
	CHECK(myDevMode == value);

	::_tcscpy_s(value.m_szPrinterOutputName, _T("PrinterOutputName"));
	CHECK(myDevMode != value);
	::_tcscpy_s(value.m_szPrinterOutputName, myDevMode.m_szPrinterOutputName);
	CHECK(myDevMode == value);

	value.dmFields = std::numeric_limits<decltype(value.dmFields)>::max();
	CHECK(myDevMode != value);
	value.dmFields = myDevMode.dmFields;
	CHECK(myDevMode == value);

	value.dmOrientation = std::numeric_limits<decltype(value.dmOrientation)>::max();
	CHECK(myDevMode != value);
	value.dmOrientation = myDevMode.dmOrientation;
	CHECK(myDevMode == value);

	value.dmPaperSize = std::numeric_limits<decltype(value.dmPaperSize)>::max();
	CHECK(myDevMode != value);
	value.dmPaperSize = myDevMode.dmPaperSize;
	CHECK(myDevMode == value);

	value.dmPaperLength = std::numeric_limits<decltype(value.dmPaperLength)>::max();
	CHECK(myDevMode != value);
	value.dmPaperLength = myDevMode.dmPaperLength;
	CHECK(myDevMode == value);

	value.dmPaperWidth = std::numeric_limits<decltype(value.dmPaperWidth)>::max();
	CHECK(myDevMode != value);
	value.dmPaperWidth = myDevMode.dmPaperWidth;
	CHECK(myDevMode == value);

	value.dmScale = std::numeric_limits<decltype(value.dmScale)>::max();
	CHECK(myDevMode != value);
	value.dmScale = myDevMode.dmScale;
	CHECK(myDevMode == value);

	value.dmCopies = std::numeric_limits<decltype(value.dmCopies)>::max();
	CHECK(myDevMode != value);
	value.dmCopies = myDevMode.dmCopies;
	CHECK(myDevMode == value);

	value.dmDefaultSource = std::numeric_limits<decltype(value.dmDefaultSource)>::max();
	CHECK(myDevMode != value);
	value.dmDefaultSource = myDevMode.dmDefaultSource;
	CHECK(myDevMode == value);

	value.dmPrintQuality = std::numeric_limits<decltype(value.dmPrintQuality)>::max();
	CHECK(myDevMode != value);
	value.dmPrintQuality = myDevMode.dmPrintQuality;
	CHECK(myDevMode == value);

	value.dmColor = std::numeric_limits<decltype(value.dmColor)>::max();
	CHECK(myDevMode != value);
	value.dmColor = myDevMode.dmColor;
	CHECK(myDevMode == value);

	value.dmDuplex = std::numeric_limits<decltype(value.dmDuplex)>::max();
	CHECK(myDevMode != value);
	value.dmDuplex = myDevMode.dmDuplex;
	CHECK(myDevMode == value);

	value.dmYResolution = std::numeric_limits<decltype(value.dmYResolution)>::max();
	CHECK(myDevMode != value);
	value.dmYResolution = myDevMode.dmYResolution;
	CHECK(myDevMode == value);

	value.dmTTOption = std::numeric_limits<decltype(value.dmTTOption)>::max();
	CHECK(myDevMode != value);
	value.dmTTOption = myDevMode.dmTTOption;
	CHECK(myDevMode == value);

	value.dmCollate = std::numeric_limits<decltype(value.dmCollate)>::max();
	CHECK(myDevMode != value);
	value.dmCollate = myDevMode.dmCollate;
	CHECK(myDevMode == value);

	::_tcscpy_s(value.dmFormName, _T("FormName"));
	CHECK(myDevMode != value);
	::_tcscpy_s(value.dmFormName, myDevMode.dmFormName);
	CHECK(myDevMode == value);

	value.dmLogPixels = std::numeric_limits<decltype(value.dmLogPixels)>::max();
	CHECK(myDevMode != value);
	value.dmLogPixels = myDevMode.dmLogPixels;
	CHECK(myDevMode == value);

	value.dmBitsPerPel = std::numeric_limits<decltype(value.dmBitsPerPel)>::max();
	CHECK(myDevMode != value);
	value.dmBitsPerPel = myDevMode.dmBitsPerPel;
	CHECK(myDevMode == value);

	value.dmPelsWidth = std::numeric_limits<decltype(value.dmPelsWidth)>::max();
	CHECK(myDevMode != value);
	value.dmPelsWidth = myDevMode.dmPelsWidth;
	CHECK(myDevMode == value);

	value.dmPelsHeight = std::numeric_limits<decltype(value.dmPelsHeight)>::max();
	CHECK(myDevMode != value);
	value.dmPelsHeight = myDevMode.dmPelsHeight;
	CHECK(myDevMode == value);

	value.dmDisplayFlags = std::numeric_limits<decltype(value.dmDisplayFlags)>::max();
	CHECK(myDevMode != value);
	value.dmDisplayFlags = myDevMode.dmDisplayFlags;
	CHECK(myDevMode == value);

	value.dmDisplayFrequency = std::numeric_limits<decltype(value.dmDisplayFrequency)>::max();
	CHECK(myDevMode != value);
	value.dmDisplayFrequency = myDevMode.dmDisplayFrequency;
	CHECK(myDevMode == value);
}

/*!
 * @brief 等価比較演算子が一般保護違反を犯さないことを保証する非機能要件テスト
 *
 *  通常、ここまでやる必要はないが、修正の理由が「安全のため」なので、
 *  実際にどういうケースで一般保護例外違反となるか、コード的に発生させる方法の共有を兼ねて実装したもの。
 */
TEST_CASE("StrategyForSegmentationFault")
{
	// システムのページサイズを取得する
	SYSTEM_INFO systemInfo = { 0 };
	::GetSystemInfo(&systemInfo);

	// システムページサイズ
	const auto pageSize = systemInfo.dwPageSize;
	// 確保領域サイズ(2ページ分)
	const auto allocSize = pageSize * 2;

	// 仮想メモリ範囲を予約する。予約時点では全体をNOACCESS指定にしておく。
	LPVOID memBlock1 = ::VirtualAlloc(NULL, allocSize, MEM_RESERVE, PAGE_NOACCESS);
	CHECK(memBlock1 != nullptr);

	// 仮想メモリ全域をコミット(=確保)する。
	wchar_t* buf1 = static_cast<wchar_t*>(::VirtualAlloc(memBlock1, allocSize, MEM_COMMIT, PAGE_READWRITE));
	CHECK(buf1 != nullptr);

	// 確保したメモリ全域をASCII文字'a'で埋める
	::wmemset(buf1, L'a', pageSize);

	// 2ページ目の保護モードをNOACCESSにする。
	DWORD flOldProtect = 0;
	volatile BOOL retVirtualProtect = ::VirtualProtect((char*)buf1 + pageSize, pageSize, PAGE_NOACCESS, &flOldProtect);
	CHECK(retVirtualProtect != FALSE);

	// メモリデータをテスト対象型にマップする。実態として配列のように扱えるポインタを取得している。
	MYDEVMODE* pValues = reinterpret_cast<MYDEVMODE*>(buf1);

	// 例外判定用の巨大な文字列を作る。これは2ページ分のサイズを持つ巨大データ。
	std::wstring largeString(pageSize, L'a');
	const auto pLargeStr = largeString.c_str();

	// doctest はまだ death tests に未対応な為コメントアウト
#if 0
	/* DEATHテストで例外ケースの判定を行う。
	 * pLargeStrには、コミットサイズの倍のデータが入っているので、
	 * 単純にstrcmpするとreserveしただけの領域にアクセスしてしまい一般保護違反(access violation)が起きる。
	 *
	 * 想定結果：一般保護違反で落ちる
	 * 備考：例外メッセージは無視する(例外が起きたことが検知できればよいから。)
	 */
	volatile int ret = 0;
	ASSERT_DEATH({ ret = ::_tcscmp(pValues[0].m_szPrinterDeviceName, pLargeStr); }, ".*");
	(void)ret;
#endif

	// 等価比較演算子を使った場合には落ちないことを確認する
	CHECK(pValues[0] == pValues[1]);
	CHECK_FALSE(pValues[0] != pValues[1]);

	// 仮想メモリをデコミット(=解放)する。
	::VirtualFree((LPVOID)buf1, pageSize, MEM_DECOMMIT);
	// 仮想メモリ範囲を解放する。
	::VirtualFree(memBlock1, 0, MEM_RELEASE);
}
