/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

/*!
	DISABLED テストのサンプル1

	- このテスト fixture は通常実行されません
	- `--gtest_also_run_disabled_tests` オプションを指定して実行することにより明示的に実行できます。

	https://github.com/google/googletest/blob/9d4cde44a4a3952cf21861f9370b3bed9265dfd7/googletest/docs/advanced.md#temporarily-disabling-tests
*/
class DISABLED_TestsIncomplte1 : public ::testing::Test
{
protected:
	/*!
		テスト前の準備
	*/
	virtual void SetUp()
	{
	}

	/*!
		テスト後の後始末
	*/
	virtual void TearDown()
	{
	}
};

/*!
	DISABLED テストのサンプル
*/
TEST_F(DISABLED_TestsIncomplte1, Test1)
{
	/* テストに問題があって失敗している例 */
	EXPECT_EQ(4, 1 + 2);
}

/*!
	DISABLED テストのサンプル
*/
TEST_F(DISABLED_TestsIncomplte1, Test2)
{
	/* テストに問題があって失敗している例 */
	EXPECT_EQ(4, 1 + 2);
}

/*!
	DISABLED テストのサンプル2

	- このテスト fixture は通常実行されません
	- クラス名に DISABLED_ を含まない
	- typedef した名前に DISABLED_ をつけて fixture にする
	- `--gtest_also_run_disabled_tests` オプションを指定して実行することにより明示的に実行できます。

	https://github.com/google/googletest/blob/9d4cde44a4a3952cf21861f9370b3bed9265dfd7/googletest/docs/advanced.md#temporarily-disabling-tests
*/
class TestsIncomplte2 : public ::testing::Test
{
protected:
	/*!
		テスト前の準備
	*/
	virtual void SetUp()
	{
	}

	/*!
		テスト後の後始末
	*/
	virtual void TearDown()
	{
	}
};

/*!
	fixture 全体を無効化するために、DISABLED_ をつけた名前を typedef する
*/
typedef TestsIncomplte2 DISABLED_TestsIncomplte2;

/*!
	DISABLED テストのサンプル
*/
TEST_F(DISABLED_TestsIncomplte2, Test1)
{
	/* テストに問題があって失敗している例 */
	EXPECT_EQ(4, 1 + 2);
}

/*!
	DISABLED テストのサンプル
*/
TEST_F(DISABLED_TestsIncomplte2, Test2)
{
	/* テストに問題があって失敗している例 */
	EXPECT_EQ(4, 1 + 2);
}

/*!
	DISABLED テストのサンプル

	- このテストは通常実行されません
	- `--gtest_also_run_disabled_tests` オプションを指定して実行することにより明示的に実行できます。

	https://github.com/google/googletest/blob/9d4cde44a4a3952cf21861f9370b3bed9265dfd7/googletest/docs/advanced.md#temporarily-disabling-tests
*/
TEST(test, DISABLED_IncomplteTest)
{
	/* テストに問題があって失敗している例 */
	EXPECT_EQ(4, 1 + 2);
}
