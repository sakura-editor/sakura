/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

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
	CSakuraEnvironment::ExpandParameter(L"$S", szExeFile, _countof2(szExeFile));
	ASSERT_STREQ(GetExeFileName().c_str(), szExeFile.c_str());
}

/*!
 * @brief iniファイルパスの取得
 */
TEST(CSakuraEnvironment, ExpandParameter_IniFileName)
{
	SFilePath szIniFile;
	CSakuraEnvironment::ExpandParameter(L"$I", szIniFile, _countof2(szIniFile));
	ASSERT_STREQ(GetIniFileName().c_str(), szIniFile.c_str());
}
