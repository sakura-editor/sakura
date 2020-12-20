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
 * @brief GetDateTimeFormatのテスト
 */
TEST( format, GetDateTimeFormat )
{
	SYSTEMTIME time = {};

	// 変換指定子の解釈
	auto result1 = GetDateTimeFormat( L"%%-%1-%", time );
	ASSERT_STREQ( L"%-1-", result1.c_str() );

	// 数字桁数少
	time.wYear = 1;
	time.wMonth = 2;
	time.wDay = 3;
	time.wHour = 1;
	time.wMinute = 2;
	time.wSecond = 3;
	auto result2 = GetDateTimeFormat( L"%Y-%y-%m-%d %H:%M:%S", time );
	ASSERT_STREQ( L"1-01-02-03 01:02:03", result2.c_str() );

	// 数字桁数多
	time.wYear = 12345;
	time.wMonth = 12;
	time.wDay = 23;
	time.wHour = 12;
	time.wMinute = 34;
	time.wSecond = 56;
	auto result3 = GetDateTimeFormat( L"%Y-%y-%m-%d %H:%M:%S", time );
	ASSERT_STREQ( L"12345-45-12-23 12:34:56", result3.c_str() );

	// 途中にnull文字
	auto result4 = GetDateTimeFormat( L"%Y-%y-%m-%d\0%H:%M:%S", time );
	ASSERT_STREQ( L"12345-45-12-23", result4.c_str() );
}
