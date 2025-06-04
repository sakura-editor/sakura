/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>

#include <stdexcept>

#include "basis/primitive.h"
#include "util/string_ex.h"

/*!
	@brief 旧コード互換用。可能であれば使わないでください。

	代替関数は strnlen か std::char_traits<char>::length です。
 */
TEST(string_ex, auto_strlenA)
{
	ASSERT_EQ(3, auto_strlen("abc"));
}

/*!
	@brief 旧コード互換用。可能であれば使わないでください。

	代替関数は wcsnlen か std::char_traits<wchar_t>::length です。
 */
TEST(string_ex, auto_strlenW)
{
	ASSERT_EQ(3, auto_strlen(L"abc"));
}

/*!
	@brief 旧コード互換用。可能であれば使わないでください。

	代替関数はないのでロジックを見直してください。
 */
TEST(string_ex, auto_strchrA)
{
	constexpr const char test_data[] = "abc";
	ASSERT_EQ(&test_data[1], auto_strchr(test_data, test_data[1]));
}

/*!
	@brief 旧コード互換用。可能であれば使わないでください。

	代替関数はないのでロジックを見直してください。
 */
TEST(string_ex, auto_strchrW)
{
	constexpr const wchar_t test_data[] = L"abc";
	ASSERT_EQ(&test_data[1], auto_strchr(test_data, test_data[1]));
}

/*!
	@brief 旧コード互換用。使わないでください。

	代替関数は snprintf_s か auto_snprintf_s です。
	可能であれば 非Unicodeな文字列 を扱うコードを書かないでください。
 */
TEST(string_ex, auto_sprintfA)
{
	char szText[_MAX_PATH];
	auto_sprintf(szText, "%s-%d", "test", 101);
	ASSERT_STREQ("test-101", szText);
}

/*!
	@brief 旧コード互換用。使わないでください。

	代替関数は strprintf か _swnprintf_s か auto_snprintf_s です。
 */
TEST(string_ex, auto_sprintfW)
{
	wchar_t szText[_MAX_PATH];
	auto_sprintf(szText, L"%s-%d", L"test", 101);
	ASSERT_STREQ(L"test-101", szText);
}

/*!
	@brief 独自定義のフォーマット関数。

	C関数をC++に移植する作業を簡便化する目的で作成。
	CRT関数sprintf_sに関する注意点が全て当てはまるので注意。
	例)
		"%b" など、未定義のフォーマット識別子を指定すると落ちます。
		フォーマットと引数の整合がとれていないとAV例外で落ちます。
 */
TEST(string_ex, strprintf)
{
	std::wstring text = strprintf(L"%s-%d", L"test", 101);
	ASSERT_STREQ(L"test-101", text.c_str());
}

/*!
	@brief 独自定義のフォーマット関数(C-Style風)。
 */
TEST(string_ex, strprintfWOutputToArg)
{
	std::wstring text(1024, L'\0');
	strprintf(text, L"%s-%d", L"test", 101);
	ASSERT_STREQ(L"test-101", text.c_str());
}

/*!
	@brief 独自定義のフォーマット関数(C-Style風)。
 */
TEST(string_ex, strprintfAOutputToArg)
{
	std::string text(1024, '\0');
	strprintf(text, "%ls-of-active-codepage-%d", L"test", 101);
	ASSERT_STREQ("test-of-active-codepage-101", text.c_str());
}

/*!
	@brief 独自定義のフォーマット関数(空文字出力テスト)。
 */
TEST(string_ex, strprintfEmpty)
{
	std::wstring text = strprintf(L"%hs", "");
	ASSERT_TRUE(text.empty());
}

/*!
	@brief 独自定義のフォーマット関数(C-Style風)。

	バッファが極端に小さい場合の確認。
	wtd::wstringのスモールバッファは7文字分。
 */
TEST(string_ex, strprintfW_small_output)
{
	std::wstring text = strprintf(L"");
	EXPECT_STREQ(L"", text.c_str());

	text = strprintf(L"%d", 1);
	EXPECT_STREQ(L"1", text.c_str());

	text = strprintf(L"%d", 12);
	EXPECT_STREQ(L"12", text.c_str());

	text = strprintf(L"%d", 123);
	EXPECT_STREQ(L"123", text.c_str());

	text = strprintf(L"%d", 1234);
	EXPECT_STREQ(L"1234", text.c_str());

	text = strprintf(L"%d", 12345);
	EXPECT_STREQ(L"12345", text.c_str());

	text = strprintf(L"%d", 123456);
	EXPECT_STREQ(L"123456", text.c_str());

	text = strprintf(L"%d", 1234567);
	EXPECT_STREQ(L"1234567", text.c_str());
}

/*!
	@brief 独自定義のフォーマット関数(C-Style風)。

	バッファが極端に小さい場合の確認
	wtd::stringのスモールバッファは15文字分。
 */
TEST(string_ex, strprintfA_small_output)
{
	std::string text = strprintf("");
	EXPECT_STREQ("", text.c_str());

	text = strprintf("%d", 1);
	EXPECT_STREQ("1", text.c_str());

	text = strprintf("%d", 12);
	EXPECT_STREQ("12", text.c_str());

	text = strprintf("%d", 123);
	EXPECT_STREQ("123", text.c_str());

	text = strprintf("%d", 1234);
	EXPECT_STREQ("1234", text.c_str());

	text = strprintf("%d", 12345);
	EXPECT_STREQ("12345", text.c_str());

	text = strprintf("%d", 123456);
	EXPECT_STREQ("123456", text.c_str());

	text = strprintf("%d", 1234567);
	EXPECT_STREQ("1234567", text.c_str());

	text = strprintf("%d", 12345678);
	EXPECT_STREQ("12345678", text.c_str());

	text = strprintf("%d", 123456789);
	EXPECT_STREQ("123456789", text.c_str());

	text = strprintf("%d", 1234567890);
	EXPECT_STREQ("1234567890", text.c_str());

	text = strprintf("1234567890%d", 1);
	EXPECT_STREQ("12345678901", text.c_str());

	text = strprintf("1234567890%d", 12);
	EXPECT_STREQ("123456789012", text.c_str());

	text = strprintf("1234567890%d", 123);
	EXPECT_STREQ("1234567890123", text.c_str());

	text = strprintf("1234567890%d", 1234);
	EXPECT_STREQ("12345678901234", text.c_str());

	text = strprintf("1234567890%d", 12345);
	EXPECT_STREQ("123456789012345", text.c_str());

	text = strprintf("1234567890%d", 123456);
	EXPECT_STREQ("1234567890123456", text.c_str());
}

/*!
	@brief 独自定義の文字列比較関数。
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
	@brief 独自定義の文字列比較関数。
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
	@brief 独自定義の文字列比較関数。
 */
TEST(string_ex, strnicmp_literal)
{
	constexpr const char test_data1[] = "abc";
	constexpr const char test_data2[] = "ABC";
	constexpr const char test_data3[] = "XYZ";
	ASSERT_EQ(0, strnicmp_literal(test_data1, test_data2));
	ASSERT_GT(0, strnicmp_literal(test_data1, test_data3));
	ASSERT_LT(0, strnicmp_literal(test_data3, test_data1));
}

/*!
	@brief 独自定義の文字列比較関数。
 */
TEST(string_ex, wcsnicmp_literal)
{
	constexpr const wchar_t test_data1[] = L"abc";
	constexpr const wchar_t test_data2[] = L"ABC";
	constexpr const wchar_t test_data3[] = L"XYZ";
	ASSERT_EQ(0, wcsnicmp_literal(test_data1, test_data2));
	ASSERT_GT(0, wcsnicmp_literal(test_data1, test_data3));
	ASSERT_LT(0, wcsnicmp_literal(test_data3, test_data1));
}
