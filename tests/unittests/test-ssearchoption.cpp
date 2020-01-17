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

#include "_main/global.h"

/*!
 * @brief 等価演算子のテスト
 *  初期値同士の等価比較を行う
 */
TEST(SSearchOption, operatorEqualSame)
{
	SSearchOption value, other;
	ASSERT_EQ(value, other);
}

/*!
 * @brief 等価演算子のテスト
 *  自分自身との等価比較を行う
 */
TEST(SSearchOption, operatorEqualBySelf)
{
	SSearchOption value;
	ASSERT_EQ(value, value);
}

/*!
 * @brief 否定の等価演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *
 *  合格条件：メンバの値が1つでも違ったら不一致を検出できること。
 */
TEST(SSearchOption, operatorNotEqual)
{
	SSearchOption value, other;

	value.bRegularExp = true;
	ASSERT_NE(value, other);
	value.bRegularExp = false;

	value.bLoHiCase = true;
	ASSERT_NE(value, other);
	value.bLoHiCase = false;

	value.bWordOnly = true;
	ASSERT_NE(value, other);
	value.bWordOnly = false;

	ASSERT_EQ(value, other);
}

/*!
 * @brief 等価比較演算子のテスト
 *  期待結果EQ,期待結果NEでは判定できない、逆条件のテストを行う
 */
TEST(SSearchOption, operatorEqualAndNotEqual)
{
	// 初期値同士の比較(等価になる)
	SSearchOption v1, v2;

	EXPECT_TRUE(v1 == v2);
	EXPECT_FALSE(v1 != v2);

	// 初期値と値を変えた値の比較(不一致になる)
	v2.bWordOnly = true;
	EXPECT_FALSE(v1 == v2);
	EXPECT_TRUE(v1 != v2);
}

/*!
 * @brief リセットのテスト
 *  値を初期値（＝全部false）に戻す。
 */
TEST(SSearchOption, Reset)
{
	SSearchOption value{ true,true,true }, other;
	EXPECT_NE(value, other);
	value.Reset();
	ASSERT_EQ(value, other);

}

