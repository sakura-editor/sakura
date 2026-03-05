/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "config/maxdata.h"
#include "basis/primitive.h"
#include "debug/Debug2.h"
#include "basis/CMyString.h"
#include "mem/CNativeW.h"
#include "env/CSakuraEnvironment.h"
#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"
#include "util/file.h"

/*!
 * @brief exeファイルパスの取得
 */
TEST(CSakuraEnvironment, ExpandParameter_ExeFileName)
{
	SFilePath szExeFile;
	CSakuraEnvironment::ExpandParameter(L"$S", szExeFile, std::size(szExeFile));
	ASSERT_STREQ(GetExeFileName().c_str(), szExeFile.c_str());
}

/*!
 * @brief iniファイルパスの取得
 */
TEST(CSakuraEnvironment, ExpandParameter_IniFileName)
{
	SFilePath szIniFile;
	CSakuraEnvironment::ExpandParameter(L"$I", szIniFile, std::size(szIniFile));
	ASSERT_STREQ(GetIniFileName().c_str(), szIniFile.c_str());
}

/*!
 * @brief プロファイル名の取得
 */
TEST(CSakuraEnvironment, ExpandParameter_ProfileName)
{
	// 受け取り用バッファを用意する
	std::wstring buffer(_MAX_PATH, L'\0');

	CSakuraEnvironment::ExpandParameter(L"$<profile>", std::data(buffer), int(std::size(buffer)));
	buffer.resize(::wcsnlen(std::data(buffer), std::size(buffer)));
	EXPECT_THAT(buffer, StrEq(L""));

	// コマンドラインのインスタンスを用意する
	auto pCommandLine = std::make_unique<CCommandLine>();
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// バッファを再確保する
	buffer = std::wstring(_MAX_PATH, L'\0');

	CSakuraEnvironment::ExpandParameter(L"$<profile>", std::data(buffer), int(std::size(buffer)));
	buffer.resize(::wcsnlen(std::data(buffer), std::size(buffer)));
	EXPECT_THAT(buffer, StrEq(L"profile1"));

	// コマンドラインのインスタンスを用意する
	pCommandLine = std::make_unique<CCommandLine>();
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// バッファを再確保する
	buffer = std::wstring(_MAX_PATH, L'\0');

	CSakuraEnvironment::ExpandParameter(L"$<profile>", std::data(buffer), int(std::size(buffer)));
	buffer.resize(::wcsnlen(std::data(buffer), std::size(buffer)));
	EXPECT_THAT(buffer, StrEq(L""));
}
