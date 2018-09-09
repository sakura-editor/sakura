#include <gtest/gtest.h>

#include <limits>
#define NOMINMAX
#include <Windows.h>
#include <tchar.h>
#include "basis/primitive.h"
#include "util/string_ex2.h"

template <typename T>
constexpr void test_constilog2_once(T n, size_t expected)
{
	//std::cout << n << ","  << expected << std::endl;
	EXPECT_EQ(constilog2(n), expected);
}

template <typename T>
constexpr void test_constilog2(T n, size_t expected)
{
	test_constilog2_once(n - 1, expected - 1);
	test_constilog2_once(n, expected);
	if (n < std::numeric_limits<T>::max()) {
		test_constilog2_once(n + 1, expected);
	}
	if (n <= std::numeric_limits<T>::max() / 2)
		test_constilog2(n * 2, expected + 1);
	else
		test_constilog2_once(n * 2 - 1, expected);
}

template <typename T>
constexpr void test_constilog2()
{
	test_constilog2_once<T>(T(0), 0);
	test_constilog2<T>(T(2), 1);
}


TEST(constilog2_test, test)
{
	test_constilog2<int32_t>();
	test_constilog2<int64_t>();
}

