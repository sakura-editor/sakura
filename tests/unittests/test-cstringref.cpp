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
#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>

#include "basis/CStringRef.h"
#include <stdexcept>
#include <gtest/gtest.h>

/*!
 * @brief コンストラクタ(パラメータなし)の仕様
 * @remark 参照先は無効なポインタになる
 * @remark 文字列長はゼロになる
 */
TEST(CStringRefW, ConstructWithoutParam)
{
	CStringRefW value;
	ASSERT_EQ(NULL, value.c_str());
	EXPECT_EQ(0, value.length());
}

/*!
 * @brief コンストラクタ(文字列と文字列長指定)の仕様
 * @remark 参照先は指定した文字列になる
 * @remark 文字列長は指定した文字列長になる
 */
TEST(CStringRefW, ConstructWithStringWithLength)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CStringRefW value(sz, cch);
	ASSERT_STREQ(sz, value.c_str());
	EXPECT_EQ(cch, value.length());
}

/*!
 * @brief コンストラクタ(文字配列指定)の仕様
 * @remark 参照先は指定した文字配列になる
 * @remark 文字列長は指定した文字配列の配列長-1になる
 */
TEST(CStringRefW, ConstructWithString)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CStringRefW value(sz);
	ASSERT_STREQ(sz, value.c_str());
	EXPECT_EQ(cch, value.length());
}

/*!
 * @brief コンストラクタ(空文字列)の仕様
 * @remark 普通の文字列指定と同じ挙動になる
 */
TEST(CStringRefW, ConstructWithStringEmpty)
{
	constexpr const wchar_t sz[] = L"";
	CStringRefW value(sz);
	ASSERT_STREQ(sz, value.c_str());
	EXPECT_EQ(0, value.length());
}

/*!
 * @brief コピーコンストラクタの仕様
 * @remark 新しいインスタンスの属性はコピー元と同じになる
 */
TEST(CStringRefW, ConstructFromOtherByCopy)
{
	constexpr const wchar_t sz[] = L"test";
	CStringRefW other(sz);
	CStringRefW value(other);
	ASSERT_STREQ(other.c_str(), value.c_str());
	EXPECT_EQ(other.length(), value.length());

	// コピー元と同じ参照になる
	ASSERT_EQ( other, value );
}

/*!
 * @brief ムーブコンストラクタの仕様
 * @remark 新しいインスタンスの属性はムーブ元と同じになる
 * @remark ムーブ元は抜け殻にならない
 */
TEST(CStringRefW, ConstructFromOtherByMove)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CStringRefW other(sz);
	CStringRefW value(std::move(other));
	ASSERT_STREQ(sz, value.c_str());
	EXPECT_EQ(cch, value.length());

	// ムーブ元と同じ参照になる
	ASSERT_EQ( other, value );
}

/*!
 * @brief コピー代入演算子の仕様
 */
TEST(CStringRefW, CopyFromOther)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CStringRefW value;
	CStringRefW other(sz);
	value = other;
	ASSERT_STREQ(sz, value.c_str());
	EXPECT_EQ(cch, value.length());

	// コピー元と同じ参照になる
	ASSERT_EQ( other, value );
}

/*!
 * @brief ムーブ代入演算子の仕様
 * @remark ムーブ元は抜け殻にならない
 */
TEST(CStringRefW, MoveFromOther)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CStringRefW value;
	CStringRefW other(sz);
	value = std::move(other);
	ASSERT_STREQ(sz, value.c_str());
	EXPECT_EQ(cch, value.length());

	// ムーブ元と同じ参照になる
	ASSERT_EQ( other, value );
}

/*!
 * @brief 添字演算子の仕様
 * @remark 指定した位置にあるwchar_tを返す
 * @remark インデックス指定が範囲外の場合 NUL を返す
 * @remark 確保領域を超える位置を指定してもエラーにならない
 */
TEST(CStringRefW, GetCharAtIndex)
{
	constexpr const wchar_t sz[] = L"森鷗外";
	constexpr const size_t cch = _countof(sz) - 1;
	CStringRefW value(sz, cch);
	EXPECT_EQ(cch, value.length());
	for (size_t index = 0; index < cch; ++index) {
		EXPECT_EQ( sz[index], value[index] );
	}
	ASSERT_EQ(0, value[cch]);

	CStringRefW nulValue;
	ASSERT_EQ( 0, nulValue[0] );
}

/*!
 * @brief 代入演算子(nullptr指定)の仕様
 * @remark バッファを確保している場合は解放される
 * @remark 文字列長はゼロになる
 */
TEST( CStringRefW, AssignStringNullPointer )
{
	CStringRefW value( L"test" );
	value = nullptr;
	EXPECT_EQ( 0, value.length() );
	EXPECT_EQ( nullptr, value.c_str() );
}

/*!
 * @brief 等価比較演算子のテスト
 *  値あり同士の等価比較を行う
 */
TEST( CStringRefW, operatorEqualSame )
{
	CStringRefW value( L"これはテストです。" );
	CStringRefW other( value );
	ASSERT_EQ( value, other );
}

/*!
 * @brief 等価比較演算子のテスト
 *  自分自身との等価比較を行う
 */
TEST( CStringRefW, operatorEqualBySelf )
{
	CStringRefW value1( L"これはテストです。" );
	ASSERT_EQ( value1, value1 );

	CStringRefW value2;
	ASSERT_EQ( value2, value2 );
}

/*!
 * @brief 等価比較演算子のテスト
 *  nullptrとの等価比較を行う
 */
TEST(CStringRefW, operatorEqualNullptr)
{
	CStringRefW value;
	ASSERT_EQ(value, nullptr);
}

/*!
 * @brief 等価比較演算子のテスト
 *  ポインタ(値がNULL)との等価比較を行う
 */
TEST(CStringRefW, operatorEqualNullLiteral)
{
	CStringRefW value;
	LPCWSTR str = NULL;
	ASSERT_EQ(value, str);
}

/*!
 * @brief 等価比較演算子のテスト
 */
TEST( CStringRefW, operatorEqualMatrix )
{
	constexpr const wchar_t pszA[] = L"abc";

	CStringRefW crefA( pszA );
	CStringRefW crefN;

	EXPECT_TRUE( crefA == crefA );
	EXPECT_FALSE( crefA != crefA );

	EXPECT_TRUE( crefA != crefN );
	EXPECT_FALSE( crefA == crefN );
}

/*!
 * @brief グローバル等価比較演算子のテスト
 */
TEST(CStringRefW, globalOperatorEqualMatrix)
{
	constexpr const wchar_t *pszA = L"abc";
	constexpr const wchar_t *pszB = L"xyz";

	CStringRefW crefA( pszA );
	CStringRefW crefB( pszB );

	EXPECT_TRUE( crefA == pszA );
	EXPECT_TRUE( pszA == crefA );
	EXPECT_FALSE( crefA != pszA );
	EXPECT_FALSE( pszA != crefA );

	EXPECT_TRUE( crefA != pszB );
	EXPECT_TRUE( pszB != crefA );
	EXPECT_FALSE( crefA == pszB );
	EXPECT_FALSE( pszB == crefA );
}

/*!
 * @brief 互換関数の仕様
 */
TEST( CStringRefW, LegacyFunctions )
{
	constexpr const wchar_t sz[] = L"森鷗外";
	constexpr const size_t cch = _countof( sz ) - 1;
	CStringRefW value( sz, cch );
	EXPECT_EQ( sz, value.GetPtr() );
	EXPECT_EQ( cch, value.GetLength() );
	for (size_t index = 0; index < cch; ++index) {
		EXPECT_EQ( value[index], value.At( index ) );
	}
}


/*!
 * 同型との比較のテスト
 *
 * @remark < 0 自身がメモリ未確保、かつ、比較対象はメモリ確保済み
 * @remark < 0 データ値が比較対象より小さい
 * @remark < 0 データが比較対象の先頭部分と一致する、かつ、データ長が比較対象より小さい
 * @remark == 0 比較対象が自分自身の参照
 * @remark == 0 自身がメモリ未確保、かつ、比較対象がメモリ未確保
 * @remark > 0 自身が確保済み、かつ、比較対象がメモリ未確保
 * @remark > 0 データ値が比較対象より大きい
 * @remark > 0 データの先頭部分が比較対象と一致する、かつ、データ長が比較対象より大きい
 */
TEST(CStringRefW, CompareWithCStringRefW)
{
	//互いに値の異なる文字列定数を定義する
	constexpr const wchar_t szS0[]	= L"a\0b\0c";
	constexpr const wchar_t szM0[]	= L"a\0a\0c\0";
	constexpr const wchar_t szM1[]	= L"a\0b\0c\0";
	constexpr const wchar_t szM2[]	= L"a\0c\0c\0";
	constexpr const wchar_t szL0[]	= L"a\0b\0c\0d";

	// 値なしの変数と文字列定数に対応するCStringRefWのインスタンスを用意する
	CStringRefW cN0, cN1
		, cS0(szS0, _countof(szS0))
		, cM0(szM0, _countof(szM0))
		, cM1(szM1, _countof(szM1))
		, cM2(szM2, _countof(szM2))
		, cL0(szL0, _countof(szL0));

	// 比較
	// ASSERT_GTの判定仕様は v1 > v2
	// ASSERT_EQの判定仕様は v1 == v2(expected, actual)
	// ASSERT_LTの判定仕様は v1 < v2
	ASSERT_GT(0, cN0.compare(cS0));
	ASSERT_GT(0, cM1.compare(cM2));
	ASSERT_GT(0, cM1.compare(cL0));
	ASSERT_EQ(0, cN0.compare(cN0));
	ASSERT_EQ(0, cS0.compare(cS0));
	ASSERT_EQ(0, cN0.compare(cN1));
	ASSERT_LT(0, cS0.compare(cN0));
	ASSERT_LT(0, cM1.compare(cM0));
	ASSERT_LT(0, cM1.compare(cS0));
}

/*!
 * 文字列ポインタ型との比較のテスト
 *
 * @remark < 0 自身がメモリ未確保、かつ、比較対象がnullptr以外
 * @remark < 0 文字列値が比較対象より小さい
 * @remark == 0 自身がメモリ未確保、かつ、比較対象がnullptr
 * @remark > 0 自身がメモリ確保済み、かつ、比較対象がnullptr
 * @remark > 0 文字列値が比較対象より大きい
 */
TEST(CStringRefW, CompareWithStringPtr)
{
	//互いに値の異なる文字列定数を定義する
	constexpr const wchar_t* pcN0 = nullptr;
	constexpr const wchar_t szS0[] = L"ab";
	constexpr const wchar_t szM0[] = L"aac";
	constexpr const wchar_t szM1[] = L"abc";
	constexpr const wchar_t szM2[] = L"acc";
	constexpr const wchar_t szL0[] = L"abcd";

	// 定数に対応するCStringRefWのインスタンスを用意する
	CStringRefW cN0(pcN0), cM1(szM1);

	// 比較
	// ASSERT_GTの判定仕様は v1 > v2
	// ASSERT_EQの判定仕様は v1 == v2(expected, actual)
	// ASSERT_LTの判定仕様は v1 < v2
	ASSERT_GT(0, cN0.compare(szM1));
	ASSERT_GT(0, cM1.compare(szM2));
	ASSERT_GT(0, cM1.compare(szL0));
	ASSERT_EQ(0, cN0.compare(pcN0));
	ASSERT_EQ(0, cM1.compare(szM1));
	ASSERT_LT(0, cM1.compare(pcN0));
	ASSERT_LT(0, cM1.compare(szM0));
	ASSERT_LT(0, cM1.compare(szS0));
}
