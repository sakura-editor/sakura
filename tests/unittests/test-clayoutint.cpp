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

#ifndef USE_STRICT_INT
#define USE_STRICT_INT
#endif /* #ifndef USE_STRICT_INT */

#include "basis/SakuraBasis.h"

/*!
 * @brief 加算演算子のテスト
 */
TEST( CLayoutInt, OperatorAddition )
{
	CLayoutInt a( 1 ), b( 2 ), c( 3 );

	EXPECT_TRUE( c == a + b );
	EXPECT_TRUE( c == a + 2 );
	EXPECT_TRUE( c == 1 + b );
}

/*!
 * @brief 減算演算子のテスト
 */
TEST( CLayoutInt, OperatorSubtraction )
{
	CLayoutInt a( 1 ), b( 2 ), c( 3 );

	EXPECT_TRUE( a == c - b );
	EXPECT_TRUE( a == c - 2 );
	EXPECT_TRUE( a == 3 - b );
}

/*!
 * @brief 乗算演算子のテスト
 */
TEST( CLayoutInt, OperatorMultiplication )
{
	// コメントアウトの組み合わせは未実装。
	CLayoutInt a( 2 ), b( 3 ), c( 6 );

	//EXPECT_TRUE( c == a * b );
	EXPECT_TRUE( c == a * 3 );
	EXPECT_TRUE( c == 2 * b );
}

/*!
 * @brief 除算演算子のテスト
 */
TEST( CLayoutInt, OperatorDivision )
{
	// コメントアウトの組み合わせは未実装。
	CLayoutInt a( 2 ), b( 3 ), c( 6 );

	//EXPECT_TRUE( a == c / b );
	EXPECT_TRUE( a == c / 3 );
	//EXPECT_TRUE( a == 6 / b );
}

/*!
 * @brief 剰余算演算子のテスト
 * 剰余算＝割り算で割った余りを求める計算のこと
 * nPos + 4 - (nPos % 4) で式の結果は4の倍数になる、とか。 
 */
TEST( CLayoutInt, OperatorRemainder )
{
	// コメントアウトの組み合わせは未実装。
	CLayoutInt a( 2 ), b( 3 ), c( 8 );

	EXPECT_TRUE( a == c % b );
	EXPECT_TRUE( a == c % 3 );
	//EXPECT_TRUE( a == 6 % b );
}

/*!
 * @brief 前置インクリメント演算子のテスト
 */
TEST( CLayoutInt, OperatorPrefixIncrement )
{
	CLayoutInt a( 1 );

	// 前置インクリメントはインクリメントした後の値を返す
	EXPECT_TRUE( 2 == ++a );
	EXPECT_TRUE( 2 == a );
}

/*!
 * @brief 後置インクリメント演算子のテスト
 */
TEST( CLayoutInt, OperatorPostfixIncrement )
{
	CLayoutInt a( 1 );

	// 後置インクリメントはインクリメントする前の値を返す
	EXPECT_TRUE( 1 == a++ );
	EXPECT_TRUE( 2 == a );
}

/*!
 * @brief 後置インクリメント演算子のテスト
 */
TEST( CLayoutInt, OperatorPrefixDecrement )
{
	CLayoutInt a( 1 );

	// 後置デクリメントはデクリメントした後の値を返す
	EXPECT_TRUE( 0 == --a );
	EXPECT_TRUE( 0 == a );
}

/*!
 * @brief 後置デクリメント演算子のテスト
 */
TEST( CLayoutInt, OperatorPostfixDecrement )
{
	CLayoutInt a( 1 );

	// 後置デクリメントはデクリメントする前の値を返す
	EXPECT_TRUE( 1 == a-- );
	EXPECT_TRUE( 0 == a );
}

/*!
 * @brief 単項マイナス演算子のテスト
 */
TEST( CLayoutInt, OperatorUnaryMinus )
{
	CLayoutInt a( 1 );

	// 値の正負を引っくり返す
	EXPECT_TRUE( -1 == -a );

	// 元の値は変わらない
	EXPECT_TRUE( 1 == a );

	// 裏表なので2回引っくり返すと元に戻る
	EXPECT_TRUE( 1 == -(-a) );
}

/*!
 * @brief 代入演算子のテスト
 * 等価比較演算子/否定の等価比較演算子のテストも兼ねる
 */
TEST( CLayoutInt, OperatorAssignment )
{
	CLayoutInt a( 1 ), b( 2 );

	EXPECT_TRUE( a != 2 );
	EXPECT_FALSE( a == 2 );
	EXPECT_TRUE( a != b );
	EXPECT_FALSE( a == b );

	a = b;

	EXPECT_TRUE( a == b );
	EXPECT_FALSE( a != b );
	EXPECT_TRUE( a == 2 );
	EXPECT_FALSE( a != 2 );
}

/*!
 * @brief 比較演算子のテスト
 */
TEST( CLayoutInt, OperatorCompareGreaterThan )
{
	CLayoutInt a;

	EXPECT_TRUE( a > -1 );
	EXPECT_FALSE( a > 0 );
	EXPECT_FALSE( 0 < a );
	EXPECT_TRUE( -1 < a );

	EXPECT_FALSE( a > a );
	EXPECT_TRUE( a > a - 1 );
}

/*!
 * @brief 比較演算子のテスト
 */
TEST( CLayoutInt, OperatorCompareGreaterOrEqual )
{
	CLayoutInt a;

	EXPECT_TRUE( a >= 0 );
	EXPECT_FALSE( a >= 1 );
	EXPECT_FALSE( 1 <= a );
	EXPECT_TRUE( 0 <= a );

	EXPECT_TRUE( a >= a );
	EXPECT_FALSE( a >= a + 1);
}

/*!
 * @brief 比較演算子のテスト
 */
TEST( CLayoutInt, OperatorCompareLessThan )
{
	CLayoutInt a;

	EXPECT_TRUE( a < 1 );
	EXPECT_FALSE( a < 0 );
	EXPECT_FALSE( 0 > a );
	EXPECT_TRUE( 1 > a );

	EXPECT_FALSE( a < a );
	EXPECT_TRUE( a < a + 1 );
}

/*!
 * @brief 比較演算子のテスト
 */
TEST( CLayoutInt, OperatorCompareLessOrEqual )
{
	CLayoutInt a;

	EXPECT_FALSE( a <= -1 );
	EXPECT_TRUE( a <= 0 );
	EXPECT_TRUE( 0 >= a );
	EXPECT_FALSE( -1 >= a );

	EXPECT_TRUE( a <= a );
	EXPECT_FALSE( a <= a - 1 );
}

/*!
 * @brief 暗黙変換演算子のテスト
 */
TEST( CLayoutInt, OperatorCastToCLaxInteger )
{
	CLayoutInt a( 1 );
	CLaxInteger la( 0 );

	//CLayoutInt ⇒ CLaxIntegerはキャストなしで変換できる
	la = a;
	EXPECT_TRUE( la == 1 );

	// リセット
	la = 0;
	EXPECT_TRUE( la == 0 );

	// もちろん、キャストしても変換できる
	la = (Int) a;

	EXPECT_TRUE( la == 1 );
}
