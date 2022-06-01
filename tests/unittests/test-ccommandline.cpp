﻿/*! @file */
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
#include <gtest/gtest.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>

#include "_main/CCommandLine.h"
#include "env/CSakuraEnvironment.h"
#include "util/string_ex.h"

#include <cstdlib>
#include <fstream>

bool operator == (const EditInfo& lhs, const EditInfo& rhs) noexcept;
bool operator != (const EditInfo& lhs, const EditInfo& rhs) noexcept;

bool operator == (const GrepInfo& lhs, const GrepInfo& rhs) noexcept;
bool operator != (const GrepInfo& lhs, const GrepInfo& rhs) noexcept;

/*!
 * ローカルパスをフルパスに変換する
 *
 * コマンドラインで指定されたパスは、フルパスに変換して格納される。
 * フルパスに変換するルールが特殊なので、実際に利用する関数を使って変換する。
 */
std::wstring GetLocalPath(const std::wstring_view& filename)
{
	constexpr size_t cchBufSize = 4096;
	auto pathBuf = std::make_unique<WCHAR[]>(cchBufSize);
	if (!pathBuf) throw std::bad_alloc();

	LPWSTR pszResolvedPath = pathBuf.get();
	::wcscpy_s(pszResolvedPath, cchBufSize, filename.data());
	CSakuraEnvironment::ResolvePath(pszResolvedPath);
	return pszResolvedPath;
}

/*!
 * @brief コンストラクタ(パラメータなし)の仕様
 * @remark パラメータを何も指定しなかった状態になる
 */
TEST(CCommandLine, ConstructWithoutParam)
{
	CCommandLine cCommandLine;
	EXPECT_FALSE(cCommandLine.IsNoWindow());
	EXPECT_FALSE(cCommandLine.IsGrepMode());
	EXPECT_FALSE(cCommandLine.IsGrepDlg());
	EXPECT_FALSE(cCommandLine.IsDebugMode());
	EXPECT_FALSE(cCommandLine.IsViewMode());

	// 解析前の値はEditInfoの構築直後の値と一致する
	EXPECT_EQ(EditInfo(), cCommandLine.GetEditInfoRef());

	// 解析前の値はGrepInfoの構築直後の値と一致する
	EXPECT_EQ(GrepInfo(), cCommandLine.GetGrepInfoRef());

	EXPECT_EQ(-1, cCommandLine.GetGroupId());
	EXPECT_EQ(NULL, cCommandLine.GetMacro());
	EXPECT_EQ(NULL, cCommandLine.GetMacroType());
	EXPECT_STREQ(L"", cCommandLine.GetProfileName());	//不自然
	EXPECT_FALSE(cCommandLine.IsSetProfile());
	EXPECT_FALSE(cCommandLine.IsProfileMgr());
	EXPECT_EQ(0, cCommandLine.GetFileNum());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
}

/*!
 * @brief パラメータ解析(-NOWIN)の仕様
 * @remark -NOWINが指定されていなければFALSE
 * @remark -NOWINが指定されていたらTRUE
 */
TEST(CCommandLine, ParseNoWin)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.IsNoWindow());
	cCommandLine.ParseCommandLine(L"-NOWIN", false);
	ASSERT_TRUE(cCommandLine.IsNoWindow());
}

/*!
 * @brief パラメータ解析(-GREPMODE)の仕様
 * @remark -GREPMODEが指定されていなければFALSE
 * @remark -GREPMODEが指定されていたらTRUE
 * @remark Grepモード時は文書タイプが"grepmode"になる
 */
TEST(CCommandLine, ParseGrepMode)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.IsGrepMode());
	cCommandLine.ParseCommandLine(L"-GREPMODE", false);
	ASSERT_TRUE(cCommandLine.IsGrepMode());

	//Grepモード時は文書タイプが"grepout"になる
	ASSERT_STREQ(L"grepout", cCommandLine.GetDocType());
}

/*!
 * @brief パラメータ解析(-GREPDLG)の仕様
 * @remark -GREPDLGが指定されていなければFALSE
 * @remark -GREPDLGが指定されていたらTRUE
 */
TEST(CCommandLine, ParseGrepDialog)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.IsGrepDlg());
	cCommandLine.ParseCommandLine(L"-GREPDLG", false);
	ASSERT_TRUE(cCommandLine.IsGrepDlg());
	//FIXME: Grepダイアログ指定時にGrepモードにならないのは不自然な気がする
}

/*!
 * @brief パラメータ解析(-DEBUGMODE)の仕様
 * @remark -DEBUGMODEが指定されていなければFALSE
 * @remark -DEBUGMODEが指定されていたらTRUE
 * @remark Debugモード時は文書タイプが"output"になる
 */
TEST(CCommandLine, ParseDebugMode)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.IsDebugMode());
	cCommandLine.ParseCommandLine(L"-DEBUGMODE", false);
	ASSERT_TRUE(cCommandLine.IsDebugMode());

	//Debugモード時は文書タイプが"output"になる
	ASSERT_STREQ(L"output", cCommandLine.GetDocType());
}

/*!
 * @brief パラメータ解析(-R)の仕様
 * @remark -Rが指定されていなければFALSE
 * @remark -Rが指定されていたらTRUE
 */
TEST(CCommandLine, ParseViewMode)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.IsViewMode());
	cCommandLine.ParseCommandLine(L"-R", false);
	ASSERT_TRUE(cCommandLine.IsViewMode());
}

/*!
 * @brief パラメータ解析(-GROUP)の仕様
 * @remark -GROUPが指定されていなければ-1
 * @remark -GROUPが指定されていたら指定された数値
 */
TEST(CCommandLine, ParseGroup)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(-1, cCommandLine.GetGroupId());
	cCommandLine.ParseCommandLine(L"-GROUP=2", false);
	EXPECT_EQ(2, cCommandLine.GetGroupId());
}

/*!
 * @brief パラメータ解析(-M)の仕様
 * @remark -Mが指定されていなければNULL
 * @remark -Mが指定されていたら指定された文字列
 */
TEST(CCommandLine, ParseMacroFileName)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(NULL, cCommandLine.GetMacro());
#define TESTLOCAL_MACRO_NAME L"真っ黒.mac"
	cCommandLine.ParseCommandLine(L"-M=" TESTLOCAL_MACRO_NAME, false);
	ASSERT_STREQ(TESTLOCAL_MACRO_NAME, cCommandLine.GetMacro());
#undef TESTLOCAL_MACRO_NAME
}

/*!
 * @brief パラメータ解析(-MTYPE)の仕様
 * @remark -MTYPEが指定されていなければNULL
 * @remark -MTYPEが指定されていたら指定された文字列
 * @remark MacroTypeには任意の文字列を指定できる
 */
TEST(CCommandLine, ParseMacroType)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(NULL, cCommandLine.GetMacroType());
#define TESTLOCAL_MACRO_TYPE L"PascalScript"
	cCommandLine.ParseCommandLine(L"-MTYPE=" TESTLOCAL_MACRO_TYPE, false);
	ASSERT_STREQ(TESTLOCAL_MACRO_TYPE, cCommandLine.GetMacroType());
#undef TESTLOCAL_MACRO_TYPE
}

/*!
 * @brief パラメータ解析(-PROF)の仕様
 * @remark -PROFが指定されていなければ空文字列
 * @remark -PROFが指定されていたら指定された文字列
 */
TEST(CCommandLine, ParseProfileName)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_STREQ(L"", cCommandLine.GetProfileName());
	EXPECT_FALSE(cCommandLine.IsSetProfile());
#define TESTLOCAL_PROFILE_NAME L"執筆用"
	cCommandLine.ParseCommandLine(L"-PROF=" TESTLOCAL_PROFILE_NAME, false);
	ASSERT_STREQ(TESTLOCAL_PROFILE_NAME, cCommandLine.GetProfileName());
	EXPECT_TRUE(cCommandLine.IsSetProfile());
#undef TESTLOCAL_PROFILE_NAME
}

/*!
 * @brief パラメータ解析(-PROFMGR)の仕様
 * @remark -PROFMGRが指定されていなければFALSE
 * @remark -PROFMGRが指定されていたらTRUE
 */
TEST(CCommandLine, ParseProfileManager)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.IsProfileMgr());
	cCommandLine.ParseCommandLine(L"-PROFMGR", false);
	ASSERT_TRUE(cCommandLine.IsProfileMgr());
}

/*!
 * @brief パラメータ解析(-X)の仕様
 * @remark -Xが指定されていなければ-1
 * @remark -Xが指定されていたら指定された数値
 */
TEST(CCommandLine, ParseCaretLocationX)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	ASSERT_EQ(-1, cCommandLine.GetCaretLocation().x);
	cCommandLine.ParseCommandLine(L"-X=123", false);
	ASSERT_EQ(122, cCommandLine.GetCaretLocation().x);
}

/*!
 * @brief パラメータ解析(-Y)の仕様
 * @remark -Yが指定されていなければ-1
 * @remark -Yが指定されていたら指定された数値
 */
TEST(CCommandLine, ParseCaretLocationY)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	ASSERT_EQ(-1, cCommandLine.GetCaretLocation().y);
	cCommandLine.ParseCommandLine(L"-Y=123", false);
	ASSERT_EQ(122, cCommandLine.GetCaretLocation().y);
}

/*!
 * @brief パラメータ解析(-VX)の仕様
 * @remark -VXが指定されていなければ-1
 * @remark -VXが指定されていたら指定された数値
 */
TEST(CCommandLine, ParseViewLeftCol)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	ASSERT_EQ(-1, (Int)cCommandLine.GetViewLocation().x);
	cCommandLine.ParseCommandLine(L"-VX=123", false);
	ASSERT_EQ(122, (Int)cCommandLine.GetViewLocation().x);
}

/*!
 * @brief パラメータ解析(-VY)の仕様
 * @remark -VYが指定されていなければ-1
 * @remark -VYが指定されていたら指定された数値
 */
TEST(CCommandLine, ParseViewTopLine)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	ASSERT_EQ(-1, (Int)cCommandLine.GetViewLocation().y);
	cCommandLine.ParseCommandLine(L"-VY=123", false);
	ASSERT_EQ(122, (Int)cCommandLine.GetViewLocation().y);
}

/*!
 * @brief パラメータ解析(-SX)の仕様
 * @remark -SXが指定されていなければ-1
 * @remark -SXが指定されていたら指定された数値
 */
TEST(CCommandLine, ParseWindowSizeX)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	ASSERT_EQ(-1, cCommandLine.GetWindowSize().cx);
	cCommandLine.ParseCommandLine(L"-SX=123", false);
	ASSERT_EQ(122, cCommandLine.GetWindowSize().cx);
}

/*!
 * @brief パラメータ解析(-SY)の仕様
 * @remark -SYが指定されていなければ-1
 * @remark -SYが指定されていたら指定された数値
 */
TEST(CCommandLine, ParseWindowSizeY)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	ASSERT_EQ(-1, cCommandLine.GetWindowSize().cy);
	cCommandLine.ParseCommandLine(L"-SY=123", false);
	ASSERT_EQ(122, cCommandLine.GetWindowSize().cy);
}

/*!
 * @brief パラメータ解析(-WX)の仕様
 * @remark -WXが指定されていなければCW_USEDEFAULT
 * @remark -WXが指定されていたら指定された数値
 */
TEST(CCommandLine, ParseWindowOriginX)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	ASSERT_EQ(CW_USEDEFAULT, cCommandLine.GetWindowOrigin().x);
	cCommandLine.ParseCommandLine(L"-WX=123", false);
	ASSERT_EQ(123, cCommandLine.GetWindowOrigin().x);
}

/*!
 * @brief パラメータ解析(-WY)の仕様
 * @remark -WYが指定されていなければCW_USEDEFAULT
 * @remark -WYが指定されていたら指定された数値
 */
TEST(CCommandLine, ParseWindowOriginY)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	ASSERT_EQ(CW_USEDEFAULT, cCommandLine.GetWindowOrigin().y);
	cCommandLine.ParseCommandLine(L"-WY=123", false);
	ASSERT_EQ(123, cCommandLine.GetWindowOrigin().y);
}

/*!
 * @brief パラメータ解析(-TYPE)の仕様
 * @remark -TYPEが指定されていなければNULL
 * @remark -TYPEが指定されていたら指定された文字列
 * @remark DocTypeには任意の文字列を指定できる
 * @remark DocTypeにMAX_DOCTYPE_LENを超える文字列を指定した場合、切り捨てられる
 */
TEST(CCommandLine, ParseDocType)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_STREQ(L"", cCommandLine.GetDocType());
#define TESTLOCAL_DOC_TYPE L"C/C++"
	cCommandLine.ParseCommandLine(L"-TYPE=" TESTLOCAL_DOC_TYPE, false);
	ASSERT_STREQ(TESTLOCAL_DOC_TYPE, cCommandLine.GetDocType());
#undef TESTLOCAL_DOC_TYPE

	// MAX_DOCTYPE_LENより長いタイプ名は切り捨てられる
	cCommandLine.ParseCommandLine(L"-TYPE=TooLongTypeName", false);
	ASSERT_STREQ(L"TooLong", cCommandLine.GetDocType());
}

/*!
 * @brief パラメータ解析(-CODE)の仕様
 * @remark -CODEが指定されていたら指定された数値
 * @remark -CODEが指定されていない場合、実行ファイル名に数字が含まれていなければ99(自動選択)
 */
TEST(CCommandLine, ParseDocCode)
{
	CCommandLine cCommandLine;
	// 初期値は99(自動選択)
	EXPECT_EQ(CODE_AUTODETECT, cCommandLine.GetDocCode());

	// コマンドラインで数値を指定すれば指定した値になる
	cCommandLine.ParseCommandLine(L"-CODE=4", false);
	EXPECT_EQ(CODE_UTF8, cCommandLine.GetDocCode());

	// 実行ファイル名に1(=JIS)を含む場合の例
	wchar_t szExeFileName[MAX_PATH] = L"sakura1.exe";
	const int cchExeFileName = ::wcsnlen(szExeFileName, _countof(szExeFileName));
	cCommandLine.ParseKanjiCodeFromFileName(szExeFileName, cchExeFileName);
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(CODE_JIS, cCommandLine.GetDocCode());

	// オプション指定で上書きされる
	cCommandLine.ParseCommandLine(L"-CODE=2", false);
	EXPECT_EQ(CODE_EUC, cCommandLine.GetDocCode());
}

/*!
 * @brief プロファイル指定済みフラグの仕様
 * @remark SetProfileNameを呼び出したらTRUE
 */
TEST(CCommandLine, SetProfileName)
{
	CCommandLine cCommandLine;
	EXPECT_FALSE(cCommandLine.IsSetProfile());
	cCommandLine.SetProfileName(L"");
	ASSERT_TRUE(cCommandLine.IsSetProfile());
}

/*!
 * @brief パラメータ解析(-GKEY)の仕様
 * @remark -GKEYが指定されていなければNULL
 * @remark -GKEYが指定されていたら指定された文字列
 */
TEST(CCommandLine, ParseGrepKey)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(NULL, cCommandLine.GetGrepInfoRef().cmGrepKey.GetStringPtr());
#define TESTLOCAL_GREP_KEY L"\\w+"
	cCommandLine.ParseCommandLine(L"-GKEY=" TESTLOCAL_GREP_KEY, false);
	ASSERT_STREQ(TESTLOCAL_GREP_KEY, cCommandLine.GetGrepInfoRef().cmGrepKey.GetStringPtr());
#undef TESTLOCAL_GREP_KEY
}

/*!
 * @brief パラメータ解析(-GREPR)の仕様
 * @remark -GREPRが指定されていなければNULL
 * @remark -GREPRが指定されていたら指定された文字列
 */
TEST(CCommandLine, ParseGrepReplaceKey)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(NULL, cCommandLine.GetGrepInfoRef().cmGrepRep.GetStringPtr());
#define TESTLOCAL_GREP_REPR L"$1。"
	cCommandLine.ParseCommandLine(L"-GREPR=" TESTLOCAL_GREP_REPR, false);
	ASSERT_STREQ(TESTLOCAL_GREP_REPR, cCommandLine.GetGrepInfoRef().cmGrepRep.GetStringPtr());
#undef TESTLOCAL_GREP_REPR
}

/*!
 * @brief パラメータ解析(-GFILE)の仕様
 * @remark -GFILEが指定されていなければNULL
 * @remark -GFILEが指定されていたら指定された文字列
 */
TEST(CCommandLine, ParseGrepFile)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(NULL, cCommandLine.GetGrepInfoRef().cmGrepFile.GetStringPtr());
#define TESTLOCAL_GREP_FILE L"#.git"
	cCommandLine.ParseCommandLine(L"-GFILE=" TESTLOCAL_GREP_FILE, false);
	ASSERT_STREQ(TESTLOCAL_GREP_FILE, cCommandLine.GetGrepInfoRef().cmGrepFile.GetStringPtr());
#undef TESTLOCAL_GREP_FILE
}

/*!
 * @brief パラメータ解析(-GFOLDER)の仕様
 * @remark -GFOLDERが指定されていなければNULL
 * @remark -GFOLDERが指定されていたら指定された文字列
 */
TEST(CCommandLine, ParseGrepFolder)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(NULL, cCommandLine.GetGrepInfoRef().cmGrepFolder.GetStringPtr());
#define TESTLOCAL_GREP_FOLDER L"C:\\work\\sakura"
	cCommandLine.ParseCommandLine(L"-GFOLDER=" TESTLOCAL_GREP_FOLDER, false);
	ASSERT_STREQ(TESTLOCAL_GREP_FOLDER, cCommandLine.GetGrepInfoRef().cmGrepFolder.GetStringPtr());
#undef TESTLOCAL_GREP_FOLDER
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければFALSE
* @remark -GOPTが指定されていたらTRUE
*/
TEST(CCommandLine, ParseGrepCurFolder)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.GetGrepInfoRef().bGrepCurFolder);
	cCommandLine.ParseCommandLine(L"-GOPT=X", false);
	EXPECT_TRUE(cCommandLine.GetGrepInfoRef().bGrepCurFolder);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければFALSE
* @remark -GOPTが指定されていたらTRUE
*/
TEST(CCommandLine, ParseGrepStdout)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.GetGrepInfoRef().bGrepStdout);
	cCommandLine.ParseCommandLine(L"-GOPT=U", false);
	EXPECT_TRUE(cCommandLine.GetGrepInfoRef().bGrepStdout);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければTRUE
* @remark -GOPTが指定されていたらFALSE
*/
TEST(CCommandLine, ParseGrepHeader)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(!cCommandLine.GetGrepInfoRef().bGrepHeader);
	cCommandLine.ParseCommandLine(L"-GOPT=H", false);
	EXPECT_TRUE(!cCommandLine.GetGrepInfoRef().bGrepHeader);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければFALSE
* @remark -GOPTが指定されていたらTRUE
*/
TEST(CCommandLine, ParseGrepSubFolder)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.GetGrepInfoRef().bGrepSubFolder);
	cCommandLine.ParseCommandLine(L"-GOPT=S", false);
	EXPECT_TRUE(cCommandLine.GetGrepInfoRef().bGrepSubFolder);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければFALSE
* @remark -GOPTが指定されていたらTRUE
*/
TEST(CCommandLine, ParseGrepCaseSensitive)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.GetGrepInfoRef().sGrepSearchOption.bLoHiCase);
	cCommandLine.ParseCommandLine(L"-GOPT=L", false);
	EXPECT_TRUE(cCommandLine.GetGrepInfoRef().sGrepSearchOption.bLoHiCase);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければFALSE
* @remark -GOPTが指定されていたらTRUE
*/
TEST(CCommandLine, ParseGrepUseRegularExpressions)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.GetGrepInfoRef().sGrepSearchOption.bRegularExp);
	cCommandLine.ParseCommandLine(L"-GOPT=R", false);
	EXPECT_TRUE(cCommandLine.GetGrepInfoRef().sGrepSearchOption.bRegularExp);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければSJIS
* @remark -GOPTが指定されていたら自動検知
* @note このオプションは特殊
*/
TEST(CCommandLine, ParseGrepCodeAutoDetect)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(CODE_SJIS, cCommandLine.GetGrepInfoRef().nGrepCharSet);
	cCommandLine.ParseCommandLine(L"-GOPT=K", false);
	EXPECT_EQ(CODE_AUTODETECT, cCommandLine.GetGrepInfoRef().nGrepCharSet);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @note このオプションは特殊
*/
TEST(CCommandLine, ParseGrepOutputLineType)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(0, cCommandLine.GetGrepInfoRef().nGrepOutputLineType);
	cCommandLine.ParseCommandLine(L"-GOPT=P", false); //Positive?
	EXPECT_EQ(1, cCommandLine.GetGrepInfoRef().nGrepOutputLineType);
	cCommandLine.ParseCommandLine(L"-GOPT=N", false); //Negative?
	EXPECT_EQ(2, cCommandLine.GetGrepInfoRef().nGrepOutputLineType);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければFALSE
* @remark -GOPTが指定されていたらTRUE
*/
TEST(CCommandLine, ParseGrepUseWordParse)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.GetGrepInfoRef().sGrepSearchOption.bWordOnly);
	cCommandLine.ParseCommandLine(L"-GOPT=W", false);
	EXPECT_TRUE(cCommandLine.GetGrepInfoRef().sGrepSearchOption.bWordOnly);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @note このオプションは特殊
*/
TEST(CCommandLine, ParseGrepOutputStyle)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(1, cCommandLine.GetGrepInfoRef().nGrepOutputStyle);
	cCommandLine.ParseCommandLine(L"-GOPT=1", false);
	EXPECT_EQ(1, cCommandLine.GetGrepInfoRef().nGrepOutputStyle);
	cCommandLine.ParseCommandLine(L"-GOPT=2", false);
	EXPECT_EQ(2, cCommandLine.GetGrepInfoRef().nGrepOutputStyle);
	cCommandLine.ParseCommandLine(L"-GOPT=3", false);
	EXPECT_EQ(3, cCommandLine.GetGrepInfoRef().nGrepOutputStyle);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければFALSE
* @remark -GOPTが指定されていたらTRUE
*/
TEST(CCommandLine, ParseGrepListFileNameOnly)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.GetGrepInfoRef().bGrepOutputFileOnly);
	cCommandLine.ParseCommandLine(L"-GOPT=F", false);
	EXPECT_TRUE(cCommandLine.GetGrepInfoRef().bGrepOutputFileOnly);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければFALSE
* @remark -GOPTが指定されていたらTRUE
*/
TEST(CCommandLine, ParseGrepDisplayRoot)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.GetGrepInfoRef().bGrepOutputBaseFolder);
	cCommandLine.ParseCommandLine(L"-GOPT=B", false);
	EXPECT_TRUE(cCommandLine.GetGrepInfoRef().bGrepOutputBaseFolder);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければFALSE
* @remark -GOPTが指定されていたらTRUE
*/
TEST(CCommandLine, ParseGrepSplitResultPerFolder)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.GetGrepInfoRef().bGrepSeparateFolder);
	cCommandLine.ParseCommandLine(L"-GOPT=D", false);
	EXPECT_TRUE(cCommandLine.GetGrepInfoRef().bGrepSeparateFolder);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければFALSE
* @remark -GOPTが指定されていたらTRUE
*/
TEST(CCommandLine, ParseGrepReplacePasteFromClipBoard)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.GetGrepInfoRef().bGrepPaste);
	cCommandLine.ParseCommandLine(L"-GOPT=C", false);
	EXPECT_TRUE(cCommandLine.GetGrepInfoRef().bGrepPaste);
}

/*!
* @brief パラメータ解析(-GOPT)の仕様
* @remark -GOPTが指定されていなければFALSE
* @remark -GOPTが指定されていたらTRUE
*/
TEST(CCommandLine, ParseGrepReplaceCreateBackupFiles)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.GetGrepInfoRef().bGrepBackup);
	cCommandLine.ParseCommandLine(L"-GOPT=O", false);
	EXPECT_TRUE(cCommandLine.GetGrepInfoRef().bGrepBackup);
}

/*!
 * @brief パラメータ解析(-GCODE)の仕様
 * @remark -GCODEが指定されていなければSJIS
 * @remark -GCODEが指定されていたら指定された数値
 */
TEST(CCommandLine, ParseGrepCode)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(CODE_SJIS, cCommandLine.GetGrepInfoRef().nGrepCharSet);
	cCommandLine.ParseCommandLine(L"-GCODE=99", false);
	EXPECT_EQ(CODE_AUTODETECT, cCommandLine.GetGrepInfoRef().nGrepCharSet);
}

/*!
 * @brief パラメータ解析(-@)の仕様
 * @remark -@が指定されていなければ何もしない
 * @remark -@が指定されていたら指定されたファイルを読み込む
 */
TEST(CCommandLine, ParseFromResponseFile)
{
	// レスポンスファイルを作る
	{
		std::ofstream resp("test.response");
		resp << "-R" << std::endl;
	}

	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"-@=test.response", true);
	EXPECT_TRUE(cCommandLine.IsViewMode());

	// ついで。存在するファイル名を指定した場合の挙動チェック
	cCommandLine.ParseCommandLine(L"test.response", false);

	std::remove("test.response");
}

/*!
 * @brief オプションの仕様
 * @remark オプションはダブルクォートで囲んでもよい
 */
TEST(CCommandLine, QuotedOption)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"\"-GROUP=1\"", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief オプションの仕様
 * @remark ダブルクォートは終端記号がなくてもよい
 */
TEST(CCommandLine, QuotedOptionWithMissingEndQuote)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"\"-GROUP=1", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark '='または':'に続けて指定する
 */
TEST(CCommandLine, OptionWithArgumentAssign)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP=1", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark '='または':'に続けて指定する
 */
TEST(CCommandLine, OptionWithArgumentColon)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP:1", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark 引数はダブルクォートまたはシングルクォートで囲ってもよい
 */
TEST(CCommandLine, OptionWithDoubleQuotedArgument)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP=\"1\"", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark 引数はダブルクォートまたはシングルクォートで囲ってもよい
 */
TEST(CCommandLine, OptionWithSingleQuotedArgument)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP=\'1\'", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark 引数を指定しなかった場合、無視される
 */
TEST(CCommandLine, OptionWithoutNeededArgument)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP", false);
	EXPECT_EQ(-1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark 無効な引数を指定した場合、無視される
 */
TEST(CCommandLine, OptionWithInvalidArgumentEmpty)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP=", false);
	EXPECT_EQ(-1, cCommandLine.GetGroupId());
	cCommandLine.ParseCommandLine(L"-GROUP:", false);
	EXPECT_EQ(-1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark 数値引数に非数を指定した場合、0を指定したものと看做される
 */
TEST(CCommandLine, OptionWithInvalidArgumentNAN)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP=Admin", false);
	EXPECT_EQ(0, cCommandLine.GetGroupId());
	cCommandLine.ParseCommandLine(L"-GROUP:Admin", false);
	EXPECT_EQ(0, cCommandLine.GetGroupId());
}

/*!
 * @brief パラメータ終端指定の仕様
 * @remark "-" で始まるファイルを扱うための仕様
 * @remark コマンドラインに "--" を含めると、
 *   以降の"-"で始まる文字列をオプション指定と看做さなくなる。
 */
TEST(CCommandLine, EndOfOptionMark)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"-- -GROUP=2", false);
	EXPECT_EQ(-1, cCommandLine.GetGroupId());
	EXPECT_STREQ(GetLocalPath(L"-GROUP=2").data(), cCommandLine.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());
}

/*!
 * @brief ファイル名の指定に関する仕様
 * @remark オプションでない引数はファイル名と解釈する
 * @remark ファイル名を複数指定した場合、1つ目のファイル名をオープン対象とする
 */
TEST(CCommandLine, ParseOpenFile)
{
	CCommandLine cCommandLine1;
	std::wstring strCmdLine1 = L"test.txt";
	cCommandLine1.ParseCommandLine(strCmdLine1.data(), false);
	EXPECT_STREQ(GetLocalPath(L"test.txt").data(), cCommandLine1.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine1.GetFileName(0));
	EXPECT_EQ(0, cCommandLine1.GetFileNum());

	CCommandLine cCommandLine2;
	std::wstring strCmdLine2 = L"test1.txt test2.txt";
	cCommandLine2.ParseCommandLine(strCmdLine2.data(), false);
	EXPECT_STREQ(GetLocalPath(L"test1.txt").data(), cCommandLine2.GetOpenFile());
	EXPECT_STREQ(GetLocalPath(L"test2.txt").data(), cCommandLine2.GetFileName(0));
	EXPECT_EQ(NULL, cCommandLine1.GetFileName(1));
	EXPECT_EQ(1, cCommandLine2.GetFileNum());
}

/*!
 * @brief 終端されない二重引用符の仕様
 */
TEST(CCommandLine, UnterminatedQuotedFilename)
{
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"\"", false);
	EXPECT_STREQ(L"", cCommandLine.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());
}

/*!
 * @brief ファイルパスに「ファイルに使えない文字」を含めた場合の仕様
 * @remark 無視される
 */
TEST(CCommandLine, ParseFileNameIncludesInvalidFilenameChars)
{
	// ファイル名に使えない文字 = "\\/:*?\"<>|"
	// このうち、\\と/はパス区切りのため実質対象外になる。
	// このうち、:は代替データストリーム(ADS)の識別記号のため対象外とする。
	const std::wstring_view badNames[] = {
		L"test*.txt",
		L"test?.txt",
		L"test\".txt",
		L"test<.txt",
		L"test>.txt",
		L"test|.txt",
	};

	// ファイル名に使えない文字を含んでいたら、ファイル名としては認識されない。
	CCommandLine cCommandLine;
	for (const auto& badName : badNames) {
		cCommandLine.ParseCommandLine( badName.data(), false );
		EXPECT_STREQ(L"", cCommandLine.GetOpenFile());
		EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
		EXPECT_EQ(0, cCommandLine.GetFileNum());
	}
}

/*!
 * @brief 長過ぎるファイルパスに関する仕様
 * @remark _MAX_PATH - 1を超えるファイル名は利用できない
 */
TEST(CCommandLine, ParseTooLongFilePath)
{
	// _MAX_PATH - 1を超えるパスは無視される
	CCommandLine cCommandLine;
	std::wstring strCmdLine;
	std::wstring strPath(_MAX_PATH, L'a');
	strprintf(strCmdLine, L"%s test.txt", strPath.c_str());
	cCommandLine.ParseCommandLine(strCmdLine.data(), false);
	// 以下のチェックはMinGWで動作しないため、コメントアウトしておく
	//EXPECT_STREQ(GetLocalPath(L"test.txt").data(), cCommandLine.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());
}

// 以下のチェックはMinGWで動作しないため、コメントアウトしておく
#ifndef __MINGW32__

/*!
 * @brief ファイルパスに指定できる上限文字列長に関する仕様
 * @remark _MAX_PATH - 1までのパスは利用できる
 */
TEST(CCommandLine, ParseMaxFilePath)
{
	// 絶対パスへの変換処理の影響を受けないように、事前に絶対パス化しておく
	std::wstring strPath = GetLocalPath(L"a");
	strPath.resize(_MAX_PATH - 1, L'a');

	// _MAX_PATH - 1までのパスは受け付けられる
	CCommandLine cCommandLine;
	std::wstring strCmdLine;
	strprintf(strCmdLine, L"%s test.txt", strPath.c_str());
	cCommandLine.ParseCommandLine(strCmdLine.data(), false);
	EXPECT_STREQ(strPath.data(), cCommandLine.GetOpenFile());
	EXPECT_STREQ(GetLocalPath(L"test.txt").data(), cCommandLine.GetFileName(0));
	EXPECT_EQ(NULL, cCommandLine.GetFileName(1));
	EXPECT_EQ(1, cCommandLine.GetFileNum());
}

#endif //ifndef __MINGW32__
