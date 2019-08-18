#include "doctest.h"

#include <limits>
#include <stdint.h>
#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <Windows.h>
#include <tchar.h>
#include "basis/primitive.h"
#include "util/string_ex2.h"

template <typename T>
void test_int2dec(T value, ptrdiff_t lenExpected, const wchar_t* strExpected)
{
	wchar_t buff[int2dec_destBufferSufficientLength<T>()];
	ptrdiff_t len = int2dec(value, buff);
	CHECK(len == lenExpected);
	CHECK(wcscmp(buff, strExpected) == 0);
}

template <typename T>
void test_plusminus(T plusValue, ptrdiff_t lenExpected, const wchar_t* strExpected)
{
	test_int2dec(plusValue, lenExpected, strExpected);
	test_int2dec(-plusValue, 1+lenExpected, (std::wstring(L"-")+strExpected).c_str());
}

static
void test_32_64_plus_minus(int value, ptrdiff_t lenExpected, const wchar_t* strExpected)
{
	test_plusminus<int32_t>(value, lenExpected, strExpected);
	test_plusminus<int64_t>(value, lenExpected, strExpected);
}

TEST_CASE("zero")
{
	test_int2dec<int32_t>(0, 1, L"0");
	test_int2dec<int64_t>(0, 1, L"0");
}

TEST_CASE("digits")
{
	test_32_64_plus_minus(2, 1, L"2");
	test_32_64_plus_minus(3, 1, L"3");
	test_32_64_plus_minus(4, 1, L"4");
	test_32_64_plus_minus(5, 1, L"5");
	test_32_64_plus_minus(6, 1, L"6");
	test_32_64_plus_minus(7, 1, L"7");
	test_32_64_plus_minus(8, 1, L"8");
	test_32_64_plus_minus(9, 1, L"9");
}

TEST_CASE("max")
{
	test_int2dec<int32_t>(std::numeric_limits<int32_t>::max(), 10, L"2147483647");
	test_int2dec<int64_t>(std::numeric_limits<int64_t>::max(), 19, L"9223372036854775807");
}

TEST_CASE("min")
{
	test_int2dec<int32_t>(std::numeric_limits<int32_t>::min(), 11, L"-2147483648");
	test_int2dec<int64_t>(std::numeric_limits<int64_t>::min(), 20, L"-9223372036854775808");
}

TEST_CASE("group_sequence")
{
	test_32_64_plus_minus(1, 1, L"1");
	test_32_64_plus_minus(12, 2, L"12");
	test_32_64_plus_minus(123, 3, L"123");
	test_32_64_plus_minus(1234, 4, L"1234");
	test_32_64_plus_minus(12345, 5, L"12345");
	test_32_64_plus_minus(123456, 6, L"123456");
	test_32_64_plus_minus(1234567, 7, L"1234567");
	test_32_64_plus_minus(12345678, 8, L"12345678");
	test_32_64_plus_minus(123456789, 9, L"123456789");
	test_32_64_plus_minus(1234567890, 10, L"1234567890");
}
