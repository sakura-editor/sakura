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
	//assert(sizeof(value) == sizeof(myDevMode));
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
	//assert(sizeof(value) == sizeof(myDevMode));
	::memcpy_s(&value, sizeof(value), &myDevMode, sizeof(myDevMode));

	EXPECT_TRUE(value == myDevMode);
	EXPECT_EQ(myDevMode, value);

	value.m_bPrinterNotFound = TRUE;
	EXPECT_NE(myDevMode, value);
	value.m_bPrinterNotFound = myDevMode.m_bPrinterNotFound;
	EXPECT_EQ(myDevMode, value);

	::wcscpy_s(value.m_szPrinterDriverName, L"PrinterDriverName");
	EXPECT_NE(myDevMode, value);
	::wcscpy_s(value.m_szPrinterDriverName, myDevMode.m_szPrinterDriverName);
	EXPECT_EQ(myDevMode, value);

	::wcscpy_s(value.m_szPrinterDeviceName, L"PrinterDeviceName");
	EXPECT_NE(myDevMode, value);
	::wcscpy_s(value.m_szPrinterDeviceName, myDevMode.m_szPrinterDeviceName);
	EXPECT_EQ(myDevMode, value);

	::wcscpy_s(value.m_szPrinterOutputName, L"PrinterOutputName");
	EXPECT_NE(myDevMode, value);
	::wcscpy_s(value.m_szPrinterOutputName, myDevMode.m_szPrinterOutputName);
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
}

/*!
 * @brief 否定の等価演算子のテスト
 *  文字列メンバの末尾(通常はNUL文字)が異なるパターンを検出できるかチェックする
 */
TEST(MYDEVMODETest, operatorNotEqualAntiLazyCode)
{
	// デフォルトで初期化
	MYDEVMODE value, other;

	// スタック変数のアドレスをchar*にキャストしてデータを書き替える
	char* buf1 = reinterpret_cast<char*>(&value);
	::memset(buf1, 'a', sizeof(MYDEVMODE));
	char* buf2 = reinterpret_cast<char*>(&other);
	::memset(buf2, 'a', sizeof(MYDEVMODE));

	// まったく同じなので等価になる
	EXPECT_TRUE(value == other);
	EXPECT_FALSE(value != other);
	EXPECT_EQ(other, value);

	// 文字列メンバをNUL終端する
	value.m_szPrinterDriverName[_countof(value.m_szPrinterDriverName) - 1] = 0;

	// NUL終端された文字列 != NUL終端されてない文字列、となるはず。
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	EXPECT_NE(other, value);
}

/*!
 * @brief 等価比較演算子が一般保護違反を犯さないことを保証する非機能要件テスト
 *
 *  通常、ここまでやる必要はないが、修正の理由が「安全のため」なので、
 *  実際にどういうケースで一般保護例外違反となるか、コード的に発生させる方法の共有を兼ねて実装したもの。
 */
TEST(MYDEVMODETest, StrategyForSegmentationFault)
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
	EXPECT_TRUE(memBlock1 != NULL);

	// 仮想メモリ全域をコミット(=確保)する。
	wchar_t* buf1 = static_cast<wchar_t*>(::VirtualAlloc(memBlock1, allocSize, MEM_COMMIT, PAGE_READWRITE));
	EXPECT_TRUE(buf1 != NULL);

	// 確保したメモリ全域をASCII文字'a'で埋める
	::wmemset(buf1, L'a', pageSize);

	// 2ページ目の保護モードをNOACCESSにする。
	DWORD flOldProtect = 0;
	volatile BOOL retVirtualProtect = ::VirtualProtect((char*)buf1 + pageSize, pageSize, PAGE_NOACCESS, &flOldProtect);
	EXPECT_TRUE(retVirtualProtect);

	// メモリデータをテスト対象型にマップする。実態として配列のように扱えるポインタを取得している。
	MYDEVMODE* pValues = reinterpret_cast<MYDEVMODE*>(buf1);

	// 例外判定用の巨大な文字列を作る。これは2ページ分のサイズを持つ巨大データ。
	std::wstring largeString(pageSize, L'a');
	const auto pLargeStr = largeString.c_str();

	/* DEATHテストで例外ケースの判定を行う。
	 * pLargeStrには、コミットサイズの倍のデータが入っているので、
	 * 単純にstrcmpするとreserveしただけの領域にアクセスしてしまい一般保護違反(access violation)が起きる。
	 *
	 * 想定結果：一般保護違反で落ちる
	 * 備考：例外メッセージは無視する(例外が起きたことが検知できればよいから。)
	 */
	volatile int ret = 0;
	ASSERT_DEATH({ ret = ::wcscmp(pValues[0].m_szPrinterDeviceName, pLargeStr); }, ".*");
	(void)ret;

	// 等価比較演算子を使った場合には落ちないことを確認する
	EXPECT_TRUE(pValues[0] == pValues[1]);
	EXPECT_FALSE(pValues[0] != pValues[1]);

	// 仮想メモリをデコミット(=解放)する。
	::VirtualFree((LPVOID)buf1, pageSize, MEM_DECOMMIT);
	// 仮想メモリ範囲を解放する。
	::VirtualFree(memBlock1, 0, MEM_RELEASE);
}
