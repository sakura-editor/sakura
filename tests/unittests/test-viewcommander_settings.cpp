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

int ChoosePointSize( const int currentSize, const int originalSize, const int* sizeTable, const size_t sizeTableCount, const int shift );

/*!
 * @brief ChoosePointSizeのテスト
 */
TEST( viewcommander_settings, ChoosePointSize )
{
	// 入力チェック

	EXPECT_EQ( ChoosePointSize( 1, 10, NULL,  1, 1 ), 1 );
	EXPECT_EQ( ChoosePointSize( 1, 10, { 0 }, 0, 1 ), 1 );
	EXPECT_EQ( ChoosePointSize( 1, 10, { 0 }, 1, 0 ), 1 );

	// テーブル要素数:1
	const int sizeTable1[] = { 10 };

	EXPECT_EQ( ChoosePointSize( 10, 10, sizeTable1, _countof(sizeTable1),  1 ), 10 );
	EXPECT_EQ( ChoosePointSize( 10, 10, sizeTable1, _countof(sizeTable1),  2 ), 10 );
	EXPECT_EQ( ChoosePointSize( 10, 10, sizeTable1, _countof(sizeTable1), -1 ), 10 );
	EXPECT_EQ( ChoosePointSize( 10, 10, sizeTable1, _countof(sizeTable1), -2 ), 10 );

	EXPECT_EQ( ChoosePointSize( 10, 15, sizeTable1, _countof(sizeTable1),  1 ), 15 );
	EXPECT_EQ( ChoosePointSize( 10, 15, sizeTable1, _countof(sizeTable1),  2 ), 15 );
	EXPECT_EQ( ChoosePointSize( 10,  5, sizeTable1, _countof(sizeTable1), -1 ),  5 );
	EXPECT_EQ( ChoosePointSize( 10,  5, sizeTable1, _countof(sizeTable1), -2 ),  5 );

	// テーブル要素数：2
	const int sizeTable2[] = { 10, 20 };

	// 現在サイズ：半端ではない 基準サイズ：半端ではない

	EXPECT_EQ( ChoosePointSize( 10, 10, sizeTable2, _countof(sizeTable2),  1 ), 20 );
	EXPECT_EQ( ChoosePointSize( 10, 10, sizeTable2, _countof(sizeTable2),  2 ), 20 );
	EXPECT_EQ( ChoosePointSize( 20, 10, sizeTable2, _countof(sizeTable2), -1 ), 10 );
	EXPECT_EQ( ChoosePointSize( 20, 10, sizeTable2, _countof(sizeTable2), -2 ), 10 );

	// 現在サイズ：半端ではない 基準サイズ：半端

	EXPECT_EQ( ChoosePointSize( 10, 15, sizeTable2, _countof(sizeTable2), 1 ), 15 );
	EXPECT_EQ( ChoosePointSize( 10, 15, sizeTable2, _countof(sizeTable2), 2 ), 20 );
	EXPECT_EQ( ChoosePointSize( 10, 15, sizeTable2, _countof(sizeTable2), 3 ), 20 );
	EXPECT_EQ( ChoosePointSize( 10, 25, sizeTable2, _countof(sizeTable2), 3 ), 25 );
	EXPECT_EQ( ChoosePointSize( 10, 25, sizeTable2, _countof(sizeTable2), 4 ), 25 );

	EXPECT_EQ( ChoosePointSize( 20, 15, sizeTable2, _countof(sizeTable2), -1 ), 15 );
	EXPECT_EQ( ChoosePointSize( 20, 15, sizeTable2, _countof(sizeTable2), -2 ), 10 );
	EXPECT_EQ( ChoosePointSize( 20, 15, sizeTable2, _countof(sizeTable2), -3 ), 10 );
	EXPECT_EQ( ChoosePointSize( 20,  5, sizeTable2, _countof(sizeTable2), -3 ),  5 );
	EXPECT_EQ( ChoosePointSize( 20,  5, sizeTable2, _countof(sizeTable2), -4 ),  5 );

	// 現在サイズ：半端 基準サイズ：半端

	EXPECT_EQ( ChoosePointSize(  5, 15, sizeTable2, _countof(sizeTable2), 1 ), 10 );
	EXPECT_EQ( ChoosePointSize( 11, 12, sizeTable2, _countof(sizeTable2), 1 ), 12 );
	EXPECT_EQ( ChoosePointSize( 11, 12, sizeTable2, _countof(sizeTable2), 2 ), 20 );
	EXPECT_EQ( ChoosePointSize( 15, 15, sizeTable2, _countof(sizeTable2), 1 ), 20 );
	EXPECT_EQ( ChoosePointSize( 25, 15, sizeTable2, _countof(sizeTable2), 1 ), 25 );

	EXPECT_EQ( ChoosePointSize(  5, 15, sizeTable2, _countof(sizeTable2), -1 ),  5 );
	EXPECT_EQ( ChoosePointSize( 12, 11, sizeTable2, _countof(sizeTable2), -1 ), 11 );
	EXPECT_EQ( ChoosePointSize( 12, 11, sizeTable2, _countof(sizeTable2), -2 ), 10 );
	EXPECT_EQ( ChoosePointSize( 15, 15, sizeTable2, _countof(sizeTable2), -1 ), 10 );
	EXPECT_EQ( ChoosePointSize( 25, 15, sizeTable2, _countof(sizeTable2), -1 ), 20 );

	// 現在サイズと基準サイズがともにテーブル最小最大の範囲外

	EXPECT_EQ( ChoosePointSize( 30, 30, sizeTable2, _countof(sizeTable2), 1 ), 30 );
	EXPECT_EQ( ChoosePointSize( 25, 30, sizeTable2, _countof(sizeTable2), 1 ), 30 );
	EXPECT_EQ( ChoosePointSize( 25, 30, sizeTable2, _countof(sizeTable2), 2 ), 30 );

	EXPECT_EQ( ChoosePointSize( 30, 30, sizeTable2, _countof(sizeTable2), -1 ), 20 );
	EXPECT_EQ( ChoosePointSize( 30, 25, sizeTable2, _countof(sizeTable2), -1 ), 25 );
	EXPECT_EQ( ChoosePointSize( 30, 25, sizeTable2, _countof(sizeTable2), -2 ), 20 );

	EXPECT_EQ( ChoosePointSize( 5, 5, sizeTable2, _countof(sizeTable2), 1 ), 10 );
	EXPECT_EQ( ChoosePointSize( 1, 5, sizeTable2, _countof(sizeTable2), 1 ), 5 );
	EXPECT_EQ( ChoosePointSize( 1, 5, sizeTable2, _countof(sizeTable2), 2 ), 10 );

	EXPECT_EQ( ChoosePointSize( 5, 5, sizeTable2, _countof(sizeTable2), -1 ), 5 );
	EXPECT_EQ( ChoosePointSize( 5, 1, sizeTable2, _countof(sizeTable2), -1 ), 1 );
	EXPECT_EQ( ChoosePointSize( 5, 1, sizeTable2, _countof(sizeTable2), -2 ), 1 );

	// テーブル要素数：3
	const int sizeTable3[] = { 10, 20, 30 };

	// 真ん中から先頭/末尾へ
	EXPECT_EQ( ChoosePointSize( 20, 25, sizeTable3, _countof(sizeTable3),  1 ), 25 );
	EXPECT_EQ( ChoosePointSize( 20, 25, sizeTable3, _countof(sizeTable3),  2 ), 30 );
	EXPECT_EQ( ChoosePointSize( 20, 25, sizeTable3, _countof(sizeTable3),  3 ), 30 );
	EXPECT_EQ( ChoosePointSize( 20, 15, sizeTable3, _countof(sizeTable3), -1 ), 15 );
	EXPECT_EQ( ChoosePointSize( 20, 15, sizeTable3, _countof(sizeTable3), -2 ), 10 );
	EXPECT_EQ( ChoosePointSize( 20, 15, sizeTable3, _countof(sizeTable3), -3 ), 10 );

	// 先頭範囲外から末尾へ
	EXPECT_EQ( ChoosePointSize( 1, 5, sizeTable3, _countof(sizeTable3), 1 ),  5 );
	EXPECT_EQ( ChoosePointSize( 1, 5, sizeTable3, _countof(sizeTable3), 2 ), 10 );
	EXPECT_EQ( ChoosePointSize( 1, 5, sizeTable3, _countof(sizeTable3), 3 ), 20 );
	EXPECT_EQ( ChoosePointSize( 1, 5, sizeTable3, _countof(sizeTable3), 4 ), 30 );
	EXPECT_EQ( ChoosePointSize( 1, 5, sizeTable3, _countof(sizeTable3), 5 ), 30 );

	// 末尾範囲外から先頭へ
	EXPECT_EQ( ChoosePointSize( 40, 35, sizeTable3, _countof(sizeTable3), -1 ), 35 );
	EXPECT_EQ( ChoosePointSize( 40, 35, sizeTable3, _countof(sizeTable3), -2 ), 30 );
	EXPECT_EQ( ChoosePointSize( 40, 35, sizeTable3, _countof(sizeTable3), -3 ), 20 );
	EXPECT_EQ( ChoosePointSize( 40, 35, sizeTable3, _countof(sizeTable3), -4 ), 10 );
	EXPECT_EQ( ChoosePointSize( 40, 35, sizeTable3, _countof(sizeTable3), -5 ), 10 );
}
