/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
	constexpr const wchar_t format4[] = L"%Y-%y-%m-%d\0%H:%M:%S";
	auto result4 = GetDateTimeFormat( std::wstring_view(format4, _countof(format4) - 1), time );
	ASSERT_STREQ( L"12345-45-12-23", result4.c_str() );
}

/*!
 * @brief CompareVersionのテスト
 * 戻り値が 0ならば: バージョンは等しい
 */
TEST(format, CompareVersion_SameVersion)
{
	// 2つのバージョン文字列を比較します。
	// バージョンは4個までの数字を「.」「-」「_」「+」でつなげたものとし、ほかに以下の記号を認識します。
	// 「a」「alpha」 ＜ 「b」「beta」 ＜ 「rc」「RC」 ＜ （記号なし） ＜ 「p」「pl」
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2.4.1.0"));

	// 1つ目の区切り文字マトリックス
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2.4.1.0"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2-4.1.0"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2_4.1.0"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2+4.1.0"));

	// 2つ目の区切り文字マトリックス
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2.4.1.0"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2.4-1.0"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2.4_1.0"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2.4+1.0"));

	// 3つ目の区切り文字マトリックス
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2.4.1.0"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2.4.1-0"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2.4.1_0"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0", L"2.4.1+0"));

	// 5つ目の値が異なっていても影響を受けない
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0.1", L"2.4.1.0.2"));

	// リビジョン記号は数値扱いなので数値4つ+記号だと影響を受けない
	ASSERT_EQ(0, CompareVersion(L"2.4.1.0alpha", L"2.4.1.0beta"));

	// 数値に指定できるのは2桁までで、区切り文字を省略できる
	ASSERT_EQ(0, CompareVersion(L"2.4.1", L"2.0401"));

	// 数値とリビジョン記号のあ緯度の区切り文字はあってもなくてもよい
	ASSERT_EQ(0, CompareVersion(L"2.4.1alpha", L"2.4.1.alpha"));

	// リビジョン記号の2文字目以降にtypoがあっても先頭文字に一致するレビジョンとして認識される
	ASSERT_EQ(0, CompareVersion(L"2.4.1a", L"2.4.1alfa"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1b", L"2.4.1bete"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1r", L"2.4.1rp"));
	ASSERT_EQ(0, CompareVersion(L"2.4.1p", L"2.4.1pp"));

	// リビジョン記号の一覧にない文字を指定した場合はalpha未満と看做す
	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0x", L"2.4.0a"));

	// リビジョン記号の一覧にない文字は文字が異なっても同一と看做す
	ASSERT_EQ(0, CompareVersion(L"2.4.1x", L"2.4.1y"));
}

/*!
 * @brief CompareVersionのテスト
 * 戻り値が 1以上ならば: Aが新しい
 */
TEST(format, CompareVersion_NewerIsA)
{
	ASSERT_TRUE(1 <= CompareVersion(L"2.4.1.0", L"2.4.0.0"));
}

/*!
 * @brief CompareVersionのテスト
 * 戻り値が -1以下ならば: Bが新しい
 */
TEST(format, CompareVersion_NewerIsB)
{
	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0.0", L"2.4.1.0"));

	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0x", L"2.4.0a"));
	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0x", L"2.4.0alpha"));

	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0a", L"2.4.0b"));
	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0alpha", L"2.4.0b"));
	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0a", L"2.4.0beta"));
	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0alpha", L"2.4.0beta"));

	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0b", L"2.4.0rc"));
	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0beta", L"2.4.0rc"));
	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0b", L"2.4.0RC"));
	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0beta", L"2.4.0RC"));

	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0rc", L"2.4.0"));
	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0RC", L"2.4.0"));

	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0", L"2.4.0p"));
	ASSERT_TRUE(-1 >= CompareVersion(L"2.4.0", L"2.4.0pl"));
}
