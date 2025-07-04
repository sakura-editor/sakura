﻿/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "CProfile.h"

#include <cstdlib>
#include <filesystem>

#include "util/file.h"
#include "CDataProfile.h"

/*!
 * @brief WriteProfileは指定されたパスに含まれるサブディレクトリを作成する
 */
TEST( CProfile, WriteProfileMakesSubDirectories )
{
	// サブディレクトリを含むパスを作成する
	WCHAR szIniName[_MAX_PATH]{ 0 };
	::_wfullpath( szIniName, L"test1\\test2\\test.ini", _countof(szIniName) );

	// プロファイルを書き出す
	CProfile cProfile;
	cProfile.WriteProfile( szIniName, L"WriteProfileのテスト" );

	ASSERT_TRUE( fexist( szIniName ) );

	WCHAR* p;

	// ファイルを削除
	std::filesystem::remove( szIniName );

	// フォルダーを削除
	p = ::PathFindFileNameW( szIniName );
	p[0] = L'\0';
	std::filesystem::remove( szIniName );

	// フォルダーを削除
	p = ::PathFindFileNameW( szIniName );
	p[0] = L'\0';
	std::filesystem::remove( szIniName );
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NoSection)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	// 初期状態は空なのでセクションが見つからない
	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"存在しないセクション名", L"szTest", value));
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NewSection)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"szTest", value));

	// セクションを追加
	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	// 追加されたセクションを取得
	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"szTest", value));
	ASSERT_STREQ(L"value", value.data());
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NoEntry)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"szTest", value));

	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"szTest", value));
	ASSERT_STREQ(L"value", value.data());

	// 該当するキーがないのでエントリを取得できない
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"nTest", value));
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NewEntry)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"szTest", value));

	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"szTest", value));
	ASSERT_STREQ(L"value", value.data());

	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"nTest", value));

	cProfile.SetProfileData(L"Test", L"nTest", L"109");

	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"nTest", value));
	ASSERT_STREQ(L"109", value.data());
}

/*!
 * @brief StringBufferWのテスト
 */
TEST(StringBufferW, ctor)
{
	WCHAR szBuf[12]{ 0 };
	StringBufferW buf1(szBuf, _countof(szBuf));
	StringBufferW buf2(szBuf);

	ASSERT_THROW({ StringBufferW buf3(nullptr, 1); }, std::invalid_argument);

	ASSERT_THROW({ StringBufferW buf4(szBuf, 0); }, std::invalid_argument);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_int)
{
	int value = 0;
	ASSERT_TRUE(profile_data::TryParse(L"109", value));
	ASSERT_EQ(109, value);

	ASSERT_FALSE(profile_data::TryParse(L"", value));
	ASSERT_EQ(109, value);

	ASSERT_FALSE(profile_data::TryParse(L"not a number", value));
	ASSERT_EQ(109, value);

	ASSERT_TRUE(profile_data::TryParse(L"888", value));
	ASSERT_EQ(888, value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_enum)
{
	enum ETest { NONE, RED, GREEN, BLUE, };
	ETest value = NONE;
	ASSERT_TRUE(profile_data::TryParse(L"1", value));
	ASSERT_EQ(RED, value);

	ASSERT_FALSE(profile_data::TryParse(L"", value));
	ASSERT_EQ(RED, value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_bool)
{
	bool value = false;
	ASSERT_TRUE(profile_data::TryParse(L"1", value));
	ASSERT_TRUE(value);

	ASSERT_FALSE(profile_data::TryParse(L"", value));
	ASSERT_TRUE(value);

	ASSERT_FALSE(profile_data::TryParse(L"false", value));
	ASSERT_TRUE(value);

	ASSERT_TRUE(profile_data::TryParse(L"0", value));
	ASSERT_FALSE(value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_WCHAR)
{
	WCHAR value = L'\0';
	ASSERT_TRUE(profile_data::TryParse(L"|", value));
	ASSERT_EQ(L'|', value);

	ASSERT_FALSE(profile_data::TryParse(L"\U0001F51E", value));
	ASSERT_EQ(L'|', value);

	ASSERT_TRUE(profile_data::TryParse(L"", value));
	ASSERT_EQ(L'\0', value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_KEYCODE)
{
	KEYCODE value = '\0';
	ASSERT_TRUE(profile_data::TryParse(L"W", value));
	ASSERT_EQ('W', value);

	ASSERT_FALSE(profile_data::TryParse(L"漢", value));
	ASSERT_EQ('W', value);

	ASSERT_TRUE(profile_data::TryParse(L"", value));
	ASSERT_EQ('\0', value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_StringBufferW)
{
	WCHAR buffer[5]{ 0 };
	StringBufferW value(buffer);
	ASSERT_TRUE(profile_data::TryParse(L"test", value));
	ASSERT_STREQ(L"test", value.c_str());

	ASSERT_FALSE(profile_data::TryParse(L"overflow", value));
	ASSERT_STREQ(L"test", value.c_str());

	ASSERT_TRUE(profile_data::TryParse(L"", value));
	ASSERT_STREQ(L"", value.c_str());
}

/*!
 * @brief ToString(KEYCODE)のテスト
 */
TEST(profile_data, ToString_KEYCODE)
{
	std::wstring str;
	KEYCODE code;

	code = 0x80;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"", str.c_str());

	code = -1;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"", str.c_str());

	code = 0;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"", str.c_str());

	code = 1;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"\x01", str.c_str());

	code = 0x61;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"a", str.c_str());

	code = 0x7f;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"\x7f", str.c_str());
}

/*!
 * @brief IOProfileDataのテスト
 */
TEST(CDataProfile, IOProfileData)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	ASSERT_FALSE(cProfile.IOProfileData(L"Test", L"szTest", value));

	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	ASSERT_TRUE(cProfile.IOProfileData(L"Test", L"szTest", value));
	ASSERT_STREQ(L"value", value.data());

	ASSERT_FALSE(cProfile.IOProfileData(L"Test", L"nTest", value));

	cProfile.SetProfileData(L"Test", L"nTest", L"109");

	ASSERT_TRUE(cProfile.IOProfileData(L"Test", L"nTest", value));
	ASSERT_STREQ(L"109", value.data());

	int nValue = 0;
	ASSERT_TRUE(cProfile.IOProfileData(L"Test", L"nTest", nValue));
	ASSERT_EQ(109, nValue);

	ASSERT_FALSE(cProfile.IOProfileData(L"Test", L"szTest", nValue));
	ASSERT_EQ(109, nValue);
}
