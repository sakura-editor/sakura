/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

/*!
	Parameterized Test のサンプルコード
	(このサンプルでは std::tr1 は使用しない)

	参考:
	http://d.hatena.ne.jp/nobu-q/20110103
	http://tatsyblog.sakura.ne.jp/wordpress/ci/1190/
*/
#include "pch.h"
#include <stdio.h>


typedef std::tuple<int, int>                  Parameters2; /* Parameterized Test のパラメータ */
typedef std::tuple<std::tuple<int, int>, int> Parameters3; /* Parameterized Test のパラメータ */

void PrintValue( int a, int b)
{
	printf( "a = %d, b = %d\n", a, b);
}

void PrintValue( int a, int b, int c )
{
	printf( "a = %d, b = %d, c = %d\n", a, b, c);
}

/*!
	パラメータが 2 つの Parameterized Test のクラス
*/
class ParameterizedTestSampleWith2 : public ::testing::TestWithParam<Parameters2> {
};

/*!
	パラメータが 3 つの Parameterized Test のクラス
*/
class ParameterizedTestSampleWith3 : public ::testing::TestWithParam<Parameters3> {
};

/* テストデータ */
int in1Params[] = { 1, 2, 3 };
int in2Params[] = { 2, 3, 4 };
int in3Params[] = { 3, 4, 5 };

/*!
	パラメータが 2 つの Parameterized Test のクラスのインスタンスを生成する
*/
INSTANTIATE_TEST_CASE_P(ParameterizedTestSampleWith2Instance,
                        ParameterizedTestSampleWith2,
                        ::testing::Combine(::testing::ValuesIn(in1Params), ::testing::ValuesIn(in2Params) )
);


/*!
	パラメータが 3 つの Parameterized Test のクラスのインスタンスを生成する
*/
INSTANTIATE_TEST_CASE_P(ParameterizedTestSampleWith3Instance,
                        ParameterizedTestSampleWith3,
                        ::testing::Combine(
                            ::testing::Combine(::testing::ValuesIn(in1Params), ::testing::ValuesIn(in2Params) ),
                            ::testing::ValuesIn(in3Params)
                        )
);

/*!
	パラメータが 2 つの Parameterized Test のクラスのテスト
*/
TEST_P(ParameterizedTestSampleWith2, test) {
	int a = std::get<0>(GetParam());
	int b = std::get<1>(GetParam());
	PrintValue(a, b);
}

/*!
	パラメータが 3 つの Parameterized Test のクラスのテスト
*/
TEST_P(ParameterizedTestSampleWith3, test) {
	/* tupple を取得する */
	auto pair = std::get<0>(GetParam());

	int a = std::get<0>(pair);
	int b = std::get<1>(pair);
	int c = std::get<1>(GetParam());
	PrintValue(a, b, c);
}
