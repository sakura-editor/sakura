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
#include <Windows.h>
#include "util/format.h"

/*!
 * @brief 入力の妥当性判定に関するテスト
 */
TEST( format, CheckInputValidity )
{
	SYSTEMTIME time = {};
	time.wYear = 123;
	WCHAR buffer[5] = {};
	EXPECT_FALSE( GetDateTimeFormat( NULL, 0, L"", time ) );
	EXPECT_FALSE( GetDateTimeFormat( buffer, 0, L"", time ) );
	EXPECT_FALSE( GetDateTimeFormat( buffer, _countof(buffer), NULL, time ) );
	EXPECT_FALSE( GetDateTimeFormat( buffer, 3, L"%Y", time ) );
	EXPECT_TRUE( GetDateTimeFormat( buffer, 4, L"%Y", time ) );
}

/*!
 * @brief バッファ書き込み範囲のテスト
 */
TEST( format, BufferWriteRange )
{
	SYSTEMTIME time = {};
	time.wYear = 1;

	WCHAR buffer1[] = { L'X', L'X', L'X' };
	EXPECT_TRUE( GetDateTimeFormat( buffer1, _countof(buffer1), L"", time ) );
	EXPECT_EQ( L'\0', buffer1[0] );
	EXPECT_EQ( L'X', buffer1[1] );

	WCHAR buffer2[] = { L'X', L'X', L'X' };
	EXPECT_TRUE( GetDateTimeFormat( buffer2, _countof(buffer2), L"%", time ) );
	EXPECT_EQ( L'\0', buffer2[0] );
	EXPECT_EQ( L'X', buffer2[1] );

	WCHAR buffer3[] = { L'X', L'X', L'X' };
	EXPECT_TRUE( GetDateTimeFormat( buffer3, _countof(buffer3), L"%Y", time ) );
	EXPECT_EQ( L'1', buffer3[0] );
	EXPECT_EQ( L'\0', buffer3[1] );
	EXPECT_EQ( L'X', buffer3[2] );

	WCHAR buffer4[] = { L'X', L'X', L'X' };
	EXPECT_FALSE( GetDateTimeFormat( buffer4, _countof(buffer4), L"_%y", time ) );
	EXPECT_EQ( L'_', buffer4[0] );
	EXPECT_EQ( L'\0', buffer4[1] );
	EXPECT_EQ( L'X', buffer4[2] );

	WCHAR buffer5[] = { L'X', L'X', L'X' };
	EXPECT_TRUE( GetDateTimeFormat( buffer5, _countof(buffer5), L"1", time ) );
	EXPECT_EQ( L'1', buffer5[0] );
	EXPECT_EQ( L'\0', buffer5[1] );
	EXPECT_EQ( L'X', buffer5[2] );

	WCHAR buffer6[] = { L'X', L'X', L'X' };
	EXPECT_TRUE( GetDateTimeFormat( buffer6, _countof(buffer6), L"11", time ) );
	EXPECT_EQ( L'1', buffer6[0] );
	EXPECT_EQ( L'1', buffer6[1] );
	EXPECT_EQ( L'\0', buffer6[2] );

	WCHAR buffer7[] = { L'X', L'X', L'X' };
	EXPECT_FALSE( GetDateTimeFormat( buffer7, _countof(buffer7), L"111", time ) );
	EXPECT_EQ( L'1', buffer7[0] );
	EXPECT_EQ( L'1', buffer7[1] );
	EXPECT_EQ( L'\0', buffer7[2] );
}

/*!
 * @brief 書式変換のテスト
 */
TEST( format, Formatting )
{
	SYSTEMTIME time = {};

	WCHAR buffer1[5] = {};
	EXPECT_TRUE( GetDateTimeFormat( buffer1, _countof(buffer1), L"%%-%1-%", time ) );
	ASSERT_STREQ( L"%-1-", buffer1 );

	time.wYear = 1;
	time.wMonth = 2;
	time.wDay = 3;
	time.wHour = 1;
	time.wMinute = 2;
	time.wSecond = 3;
	WCHAR buffer2[20] = {};
	EXPECT_TRUE( GetDateTimeFormat( buffer2, _countof(buffer2), L"%Y-%y-%m-%d %H:%M:%S", time ) );
	ASSERT_STREQ( L"1-01-02-03 01:02:03", buffer2 );

	time.wYear = 12345;
	time.wMonth = 12;
	time.wDay = 23;
	time.wHour = 12;
	time.wMinute = 34;
	time.wSecond = 56;
	WCHAR buffer3[24] = {};
	EXPECT_TRUE( GetDateTimeFormat( buffer3, _countof(buffer3), L"%Y-%y-%m-%d %H:%M:%S", time ) );
	ASSERT_STREQ( L"12345-45-12-23 12:34:56", buffer3 );
}
