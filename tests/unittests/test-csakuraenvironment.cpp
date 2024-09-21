/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

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

#include "env/CSakuraEnvironment.h"

#include "CEditorProcessInitTest.hpp"

#include "_main/CNormalProcess.h"

#include "util/file.h"

struct CSakuraEnvironmentTest : public CEditorProcessInitTest {};

/*!
 * @brief exeファイルパスの取得
 */
TEST_F(CSakuraEnvironmentTest, ExpandParameter_ExeFileName)
{
	SFilePath szExeFile;
	CSakuraEnvironment::ExpandParameter(L"$S", szExeFile, _countof2(szExeFile));
	ASSERT_STREQ(GetExeFileName().c_str(), szExeFile.c_str());
}

/*!
 * @brief iniファイルパスの取得
 */
TEST_F(CSakuraEnvironmentTest, ExpandParameter_IniFileName)
{
	SFilePath szIniFile;
	CSakuraEnvironment::ExpandParameter(L"$I", szIniFile, _countof2(szIniFile));
	ASSERT_STREQ(GetIniFileName().c_str(), szIniFile.c_str());
}

/*!
 * @brief GREP検索キーの取得(32文字以内)
 */
TEST_F(CSakuraEnvironmentTest, ExpandParameter_GetGrepKey001)
{
	constexpr auto& grepKey = L"1234567890ABCDEF1234567890abcdef";
	SFilePath szBuff;
	CAppMode::getInstance()->SetGrepKey(grepKey);
	CSakuraEnvironment::ExpandParameter(L"$h", szBuff, _countof2(szBuff));
	ASSERT_STREQ(grepKey, szBuff.c_str());
}

/*!
 * @brief GREP検索キーの取得(32文字超過)
 */
TEST_F(CSakuraEnvironmentTest, ExpandParameter_GetGrepKey002)
{
	SFilePath szBuff;
	CAppMode::getInstance()->SetGrepKey(L"1234567890ABCDEF1234567890abcdef0"sv);
	CSakuraEnvironment::ExpandParameter(L"$h", szBuff, _countof2(szBuff));
	ASSERT_STREQ(L"1234567890ABCDEF1234567890abcdef...", szBuff.c_str());
}

/*!
 * @brief GREP検索キーの取得(出力バッファサイズピッタリ)
 */
TEST_F(CSakuraEnvironmentTest, ExpandParameter_GetGrepKey003)
{
	StaticString<WCHAR, 30 + 1> szBuff;
	CAppMode::getInstance()->SetGrepKey(L"1234567890ABCDEF1234567890abcd"sv);
	CSakuraEnvironment::ExpandParameter(L"$h", szBuff, _countof2(szBuff));
	ASSERT_STREQ(L"1234567890ABCDEF1234567890abcd", szBuff.c_str());
}

/*!
 * @brief GREP検索キーの取得(出力バッファサイズが足りない)
 */
TEST_F(CSakuraEnvironmentTest, ExpandParameter_GetGrepKey004)
{
	StaticString<WCHAR, 30 + 1> szBuff;
	CAppMode::getInstance()->SetGrepKey(L"1234567890ABCDEF1234567890abcdef0"sv);
	CSakuraEnvironment::ExpandParameter(L"$h", szBuff, _countof2(szBuff));
	ASSERT_STREQ(L"1234567890ABCDEF1234567890a...", szBuff.c_str());
}

/*!
 * @brief GREP検索キーの取得(設定なし)
 */
TEST_F(CSakuraEnvironmentTest, ExpandParameter_GetGrepKey005)
{
	SFilePath szBuff;
	CAppMode::getInstance()->SetGrepKey(L""sv);
	CSakuraEnvironment::ExpandParameter(L"$h", szBuff, _countof2(szBuff));
	ASSERT_STREQ(L"", szBuff.c_str());
}
