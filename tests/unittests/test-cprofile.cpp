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

	// フォルダを削除
	p = ::PathFindFileNameW( szIniName );
	p[0] = L'\0';
	std::filesystem::remove( szIniName );

	// フォルダを削除
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

	ASSERT_TRUE(cProfile.IOProfileData(L"Test", L"szTest", nValue));
	ASSERT_EQ(0, nValue);
}
