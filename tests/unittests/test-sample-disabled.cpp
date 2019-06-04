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

/*!
	DISABLED テストのサンプル
	
	- このテストは通常実行されません
	- --gtest_also_run_disabled_tests オプションを指定して実行することにより明示的に実行できます。

	https://github.com/google/googletest/blob/9d4cde44a4a3952cf21861f9370b3bed9265dfd7/googletest/docs/advanced.md#temporarily-disabling-tests
*/
class DISABLED_TestsImcomplte1 : public ::testing::Test
{


};


TEST_F(DISABLED_TestsImcomplte1, Test1)
{
	/* 意図的に失敗させる */
	EXPECT_EQ(1 + 2, 4);
}


TEST_F(DISABLED_TestsImcomplte1, Test2)
{
	/* 意図的に失敗させる */
	EXPECT_EQ(1 + 2, 4);
}


/*!
	DISABLED テストのサンプル
	
	- このテストは通常実行されません
	- --gtest_also_run_disabled_tests オプションを指定して実行することにより明示的に実行できます。

	https://github.com/google/googletest/blob/9d4cde44a4a3952cf21861f9370b3bed9265dfd7/googletest/docs/advanced.md#temporarily-disabling-tests
*/
TEST(test, DISABLED_IncomplteTest)
{
	/* 意図的に失敗させる */
	EXPECT_EQ(1 + 2, 4);
}
