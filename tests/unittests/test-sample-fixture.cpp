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
}

/*!
	Fixture テストのサンプル
*/
TEST_F(FixtureSample, Test2)
{
}
