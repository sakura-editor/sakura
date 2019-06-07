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

/*!
	`test fixture class` のサンプルコード
*/
#include <gtest/gtest.h>
#include <cstdio>
#include <exception>


/*!
	`test fixture class` のサンプル

	以下を参照
	- http://opencv.jp/googletestdocs/primer.html#primer-test-fixtures
	- https://github.com/google/googletest/blob/ea9c49b9cd410cca4df39b376f7da7cddf2dcf6d/googletest/docs/primer.md#basic-concepts
	- https://github.com/google/googletest/blob/ea9c49b9cd410cca4df39b376f7da7cddf2dcf6d/googletest/docs/primer.md#test-fixtures-using-the-same-data-configuration-for-multiple-tests
*/
class FixtureSample : public ::testing::Test
{
protected:
	/*!
		テスト前の準備
	*/
	virtual void SetUp()
	{
		printf("%s\n", __FUNCTION__);
	}

	/*!
		テスト後の後始末
	*/
	virtual void TearDown()
	{
		printf("%s\n", __FUNCTION__);
	}
};

/*!
	Fixture テストのサンプル
*/
TEST_F(FixtureSample, Test1)
{
	/* テストケース名を出力する */
	printf("%s::%s\n", ::testing::UnitTest::GetInstance()->current_test_case()->name(), ::testing::UnitTest::GetInstance()->current_test_info()->name());
}

/*!
	Fixture テストのサンプル
*/
TEST_F(FixtureSample, Test2)
{
	/* テストケース名を出力する */
	printf("%s::%s\n", ::testing::UnitTest::GetInstance()->current_test_case()->name(), ::testing::UnitTest::GetInstance()->current_test_info()->name());
}
