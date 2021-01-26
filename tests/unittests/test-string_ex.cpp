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

#include "basis/primitive.h"
#include "util/string_ex.h"

/*!
	@breaf 旧コード互換用。可能であれば使わないでください。

	代替関数は strnlen か std::char_traits<char>::length です。
 */
TEST(string_ex, auto_strlenA)
{
	ASSERT_EQ(3, auto_strlen("abc"));
}

/*!
	@breaf 旧コード互換用。可能であれば使わないでください。

	代替関数は wcsnlen か std::char_traits<wchar_t>::length です。
 */
TEST(string_ex, auto_strlenW)
{
	ASSERT_EQ(3, auto_strlen(L"abc"));
}

/*!
	@breaf 旧コード互換用。可能であれば使わないでください。

	代替関数はないのでロジックを見直してください。
 */
TEST(string_ex, auto_strchrA)
{
	constexpr const char test_data[] = "abc";
	ASSERT_EQ(&test_data[1], auto_strchr(test_data, test_data[1]));
}

/*!
	@breaf 旧コード互換用。可能であれば使わないでください。

	代替関数はないのでロジックを見直してください。
 */
TEST(string_ex, auto_strchrW)
{
	constexpr const wchar_t test_data[] = L"abc";
	ASSERT_EQ(&test_data[1], auto_strchr(test_data, test_data[1]));
}

/*!
	@breaf 旧コード互換用。使わないでください。

	代替関数は snprintf_s か auto_sprintf_s です。
	可能であれば 非Unicodeな文字列 を扱うコードを書かないでください。
 */
TEST(string_ex, auto_sprintfA)
{
	std::string text(_MAX_PATH, L'\0');
	auto_sprintf(text.data(), "%s-%d", "test", 101);
	ASSERT_STREQ("test-101", text.data());
}

/*!
	@breaf 旧コード互換用。使わないでください。

	代替関数は strprintf か _swnprintf_s か auto_sprintf_s です。
 */
TEST(string_ex, auto_sprintfW)
{
	std::wstring text(_MAX_PATH, L'\0');
	auto_sprintf(text.data(), L"%s-%d", L"test", 101);
	ASSERT_STREQ(L"test-101", text.data());
}

TEST(string_ex, strprintf)
{
	std::wstring text;
	strprintf(text, L"%s-%d", L"test", 101);
	ASSERT_STREQ(L"test-101", text.c_str());
}

/*!
	@breaf 独自定義の文字列比較関数。
 */
TEST(string_ex, strncmp_literal)
{
	constexpr const char test_data1[] = "abc";
	constexpr const char test_data2[] = "abc";
	constexpr const char test_data3[] = "xyz";
	ASSERT_EQ(0, strncmp_literal(test_data1, test_data2));
	ASSERT_GT(0, strncmp_literal(test_data1, test_data3));
	ASSERT_LT(0, strncmp_literal(test_data3, test_data1));
}

/*!
	@breaf 独自定義の文字列比較関数。
 */
TEST(string_ex, wcsncmp_literal)
{
	constexpr const wchar_t test_data1[] = L"abc";
	constexpr const wchar_t test_data2[] = L"abc";
	constexpr const wchar_t test_data3[] = L"xyz";
	ASSERT_EQ(0, wcsncmp_literal(test_data1, test_data2));
	ASSERT_GT(0, wcsncmp_literal(test_data1, test_data3));
	ASSERT_LT(0, wcsncmp_literal(test_data3, test_data1));
}

/*!
	@breaf 独自定義の文字列比較関数。
 */
TEST(string_ex, strnicmp_literal)
{
	constexpr const char test_data1[] = "abc";
	constexpr const char test_data2[] = "ABC";
	constexpr const char test_data3[] = "xyz";
	ASSERT_EQ(0, strnicmp_literal(test_data1, test_data2));
	ASSERT_GT(0, strnicmp_literal(test_data1, test_data3));
	ASSERT_LT(0, strnicmp_literal(test_data3, test_data1));
}

/*!
	@breaf 独自定義の文字列比較関数。
 */
TEST(string_ex, wcsnicmp_literal)
{
	constexpr const wchar_t test_data1[] = L"abc";
	constexpr const wchar_t test_data2[] = L"ABC";
	constexpr const wchar_t test_data3[] = L"xyz";
	ASSERT_EQ(0, wcsnicmp_literal(test_data1, test_data2));
	ASSERT_GT(0, wcsnicmp_literal(test_data1, test_data3));
	ASSERT_LT(0, wcsnicmp_literal(test_data3, test_data1));
}
