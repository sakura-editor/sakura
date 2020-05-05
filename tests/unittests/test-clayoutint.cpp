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

#include "basis/SakuraBasis.h"

/*!
 * @brief 比較演算子のテスト
 */
TEST( CLayoutInt, OperatorCompareGreaterThan )
{
	CLayoutInt a( 1 );

	EXPECT_TRUE( a > -1 );
	EXPECT_TRUE( a > 0 );
	EXPECT_FALSE( a > 1 );
	EXPECT_FALSE( a > 2 );
}

/*!
 * @brief 比較演算子のテスト
 */
TEST( CLayoutInt, OperatorCompareGreaterOrEqual )
{
	CLayoutInt a( 1 );

	EXPECT_TRUE( a >= -1 );
	EXPECT_TRUE( a >= 0 );
	EXPECT_TRUE( a >= 1 );
	EXPECT_FALSE( a >= 2 );
}

/*!
 * @brief 比較演算子のテスト
 */
TEST( CLayoutInt, OperatorCompareLessThan )
{
	CLayoutInt a( 1 );

	EXPECT_FALSE( a < -1 );
	EXPECT_FALSE( a < 0 );
	EXPECT_FALSE( a < 1 );
	EXPECT_TRUE( a < 2 );
}

/*!
 * @brief 比較演算子のテスト
 */
TEST( CLayoutInt, OperatorCompareLessOrEqual )
{
	CLayoutInt a( 1 );

	EXPECT_FALSE( a <= -1 );
	EXPECT_FALSE( a <= 0 );
	EXPECT_TRUE( a <= 1 );
	EXPECT_TRUE( a <= 2 );
}

/*!
 * @brief グローバル比較演算子のテスト
 */
TEST( CLayoutInt, GlobalOperatorCompareGreaterThan )
{
	CLayoutInt a( 1 );

	EXPECT_TRUE( -1 > a );
	EXPECT_TRUE( 0 > a );
	EXPECT_FALSE( 1 > a );
	EXPECT_FALSE( 2 > a );
}

/*!
 * @brief グローバル比較演算子のテスト
 */
TEST( CLayoutInt, GlobalOperatorCompareGreaterOrEqual )
{
	CLayoutInt a( 1 );

	EXPECT_TRUE( -1 >= a );
	EXPECT_TRUE( 0 >= a );
	EXPECT_TRUE( 1 >= a );
	EXPECT_FALSE( 2 >= a );
}

/*!
 * @brief グローバル比較演算子のテスト
 */
TEST( CLayoutInt, GlobalOperatorCompareLessThan )
{
	CLayoutInt a( 1 );

	EXPECT_FALSE( -1 < a );
	EXPECT_FALSE( 0 < a );
	EXPECT_FALSE( 1 < a );
	EXPECT_TRUE( 2 < a );
}

/*!
 * @brief グローバル比較演算子のテスト
 */
TEST( CLayoutInt, GlobalOperatorCompareLessOrEqual )
{
	CLayoutInt a( 1 );

	EXPECT_FALSE( -1 <= a );
	EXPECT_FALSE( 0 <= a );
	EXPECT_TRUE( 1 <= a );
	EXPECT_TRUE( 2 <= a );
}

