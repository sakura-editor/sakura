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

#include "_main/CCommandLine.h"
#include "debug/debug2.h"

#include <wrl.h>
#include <ShlObj.h>
#include <Shlwapi.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>

bool operator == (const EditInfo& lhs, const EditInfo& rhs) noexcept;
bool operator != (const EditInfo& lhs, const EditInfo& rhs) noexcept;

bool operator == (const GrepInfo& lhs, const GrepInfo& rhs) noexcept;
bool operator != (const GrepInfo& lhs, const GrepInfo& rhs) noexcept;

/*!
 * テスト用の極薄ラッパークラス
 */
class CCommandLineWrapper : public CCommandLine
{
public:
	CCommandLineWrapper() = default;
};

/*!
 * 指定されたパスをフルパスに変換する
 */
LPCWSTR ToFullPath(LPCWSTR szFilename)
{
	static WCHAR szPath[_MAX_PATH]{ 0 };
	::_wfullpath( szPath, szFilename, _countof(szPath) );
	return szPath;
}

/*!
 * ショートカット(.lnk)の作成
 *
 * @remark この関数はテストで利用することのみを想定した簡易実装になっているので、本体に移植するなら例外処理を実装する必要があります。
 */
bool CreateShortcutLink(
	LPCWSTR pszAbsLinkPath,			//!< [in] ショートカット(.lnk)のフルパス
	LPCWSTR pszPathToBeLinked		//!< [in] リンク先ファイルのフルパス
)
{
	using namespace Microsoft::WRL;

	// 引数の前提条件(IShellLinkは_MAX_PATH以上のパスをサポートしません。)
	assert(pszAbsLinkPath && pszAbsLinkPath[0] && _MAX_PATH != ::wcsnlen(pszAbsLinkPath, _MAX_PATH));
	assert(pszPathToBeLinked && pszPathToBeLinked[0] && _MAX_PATH != ::wcsnlen(pszPathToBeLinked, _MAX_PATH));

	ComPtr<IShellLink> pShellLink;
	// Get a pointer to the IShellLink interface.
	if( SUCCEEDED( ::CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pShellLink) ) ) ){
		// Set the path to the link target.
		if( SUCCEEDED( pShellLink->SetPath( pszPathToBeLinked ) ) ){
			ComPtr<IPersistFile> pPersistFile;
			// Get a pointer to the IPersistFile interface.
			if( SUCCEEDED( pShellLink->QueryInterface( IID_PPV_ARGS(&pPersistFile) ) ) ){
				// Save the shortcut.
				if( SUCCEEDED( pPersistFile->Save( pszAbsLinkPath, TRUE ) ) ){
					return true;
				}
			}
		}
	}
	return false;
}

/*!
 * @brief コンストラクタ(パラメータなし)の仕様
 * @remark パラメータを何も指定しなかった状態になる
 */
TEST(CCommandLine, ConstructWithoutParam)
{
	CCommandLineWrapper cCommandLine;
	EXPECT_FALSE(cCommandLine.IsNoWindow());
	EXPECT_FALSE(cCommandLine.IsWriteQuit());
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
 * @remark -WQが指定された場合、-NOWINがなくてもTRUE
 */
TEST(CCommandLine, ParseNoWin)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.IsNoWindow());
	cCommandLine.ParseCommandLine(L"-NOWIN", false);
	ASSERT_TRUE(cCommandLine.IsNoWindow());
	cCommandLine.ParseCommandLine(L"-WQ", false);
	ASSERT_TRUE(cCommandLine.IsNoWindow());
}

/*!
 * @brief パラメータ解析(-WQ)の仕様
 * @remark -WQが指定されていなければFALSE
 * @remark -WQが指定されていたらTRUE
 * @remark -WQが指定された場合、-NOWINもTRUE
 */
TEST(CCommandLine, ParseWriteQuit)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.IsWriteQuit());
	cCommandLine.ParseCommandLine(L"-WQ", false);
	ASSERT_TRUE(cCommandLine.IsWriteQuit());
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
 */
TEST(CCommandLine, ParseDocType)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_STREQ(L"", cCommandLine.GetDocType());
#define TESTLOCAL_DOC_TYPE L"C/C++"
	cCommandLine.ParseCommandLine(L"-TYPE=" TESTLOCAL_DOC_TYPE, false);
	ASSERT_STREQ(TESTLOCAL_DOC_TYPE, cCommandLine.GetDocType());
#undef TESTLOCAL_DOC_TYPE
}

/*!
 * @brief パラメータ解析(-CODE)の仕様
 * @remark -CODEが指定されていたら指定された数値
 * @remark -CODEが指定されていない場合、実行ファイル名に数字が含まれていなければ99(自動選択)
 */
TEST(CCommandLine, ParseDocCode)
{
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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

	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"-@=test.response", true);
	EXPECT_TRUE(cCommandLine.IsViewMode());

	// ついで。存在するファイル名を指定した場合の挙動チェック
	cCommandLine.ParseCommandLine(L"test.response", false);

	std::remove("test.response");
}

/*!
 * @brief パラメータ解析(-@)の仕様
 * @remark -@が指定されていたら指定されたファイルが存在しない場合、無視する
 */
TEST(CCommandLine, ParseFromResponseFileMissing)
{
#define TESTLOCAL_FILE_NAME "not-found.response"

	// ファイルパスが既に存在していたら削除して作り直す
	if( fexist( _T(TESTLOCAL_FILE_NAME) ) ){
		std::filesystem::remove( _T(TESTLOCAL_FILE_NAME) );
	}

	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"-@=" _T(TESTLOCAL_FILE_NAME), true);
	EXPECT_STREQ(L"", cCommandLine.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());

	EXPECT_FALSE(cCommandLine.IsViewMode());

#undef TESTLOCAL_FILE_NAME
}

/*!
 * @brief パラメータ解析の仕様
 * @remark 未定義のオプションは無視する
 */
TEST(CCommandLine, ParseUndefinedOption)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"-UNDEFINED", false);
}

/*!
 * @brief オプションの仕様
 * @remark オプションはダブルクォートで囲んでもよい
 */
TEST(CCommandLine, QuotedOption)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"\"-GROUP=1\"", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief オプションの仕様
 * @remark ダブルクォートは終端記号がなくてもよい
 */
TEST(CCommandLine, QuotedOptionWithMissingEndQuote)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"\"-GROUP=1", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark '='または':'に続けて指定する
 */
TEST(CCommandLine, OptionWithArgumentAssign)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP=1", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark '='または':'に続けて指定する
 */
TEST(CCommandLine, OptionWithArgumentColon)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP:1", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark 引数はダブルクォートまたはシングルクォートで囲ってもよい
 */
TEST(CCommandLine, OptionWithDoubleQuotedArgument)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP=\"1\"", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark 引数はダブルクォートまたはシングルクォートで囲ってもよい
 */
TEST(CCommandLine, OptionWithSingleQuotedArgument)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP=\'1\'", false);
	ASSERT_EQ(1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark 引数を指定しなかった場合、無視される
 */
TEST(CCommandLine, OptionWithoutNeededArgument)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"-GROUP", false);
	EXPECT_EQ(-1, cCommandLine.GetGroupId());
}

/*!
 * @brief 引数付きパラメータの仕様
 * @remark 無効な引数を指定した場合、無視される
 */
TEST(CCommandLine, OptionWithInvalidArgumentEmpty)
{
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
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
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"-- -GROUP=2", false);
	EXPECT_EQ(-1, cCommandLine.GetGroupId());
	EXPECT_STREQ(ToFullPath(L"-GROUP=2"), cCommandLine.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());
}

/*!
 * @brief 終端されない二重引用符の仕様
 */
TEST(CCommandLine, UnterminatedQuotedOption)
{
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(L"\"", false);
	EXPECT_STREQ(L"", cCommandLine.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());
}

/*!
 * @brief コマンドライン先頭が「空白を含むファイルパス」である場合の仕様
 * @remark 「空白を含むファイルパス」を扱うための仕様。
 * @remark コマンドラインの先頭が "-" で始まらない場合に、
 *   存在するファイルパスと一致する先頭部分をファイルパスとする。
 */
TEST(CCommandLine, UnquotedFileIncludesSpacesAtBeginOfCommandLine)
{
#define TESTLOCAL_FILE_NAME "unquoted file path includes spaces"

	// ファイルパスが既に存在していたら削除して作り直す
	if( fexist( _T(TESTLOCAL_FILE_NAME) ) ){
		std::filesystem::remove( _T(TESTLOCAL_FILE_NAME) );
	}

	// ファイルパスにテキトーなテキストファイルを作成する
	{
		std::ofstream local_file( TESTLOCAL_FILE_NAME );
		local_file << TESTLOCAL_FILE_NAME << std::endl;
	}

	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine( _T(TESTLOCAL_FILE_NAME) L" tmp.txt -R", false );
	EXPECT_STREQ(ToFullPath(_T(TESTLOCAL_FILE_NAME)), cCommandLine.GetOpenFile());
	EXPECT_STREQ(ToFullPath(L"tmp.txt"), cCommandLine.GetFileName(0));
	EXPECT_EQ(1, cCommandLine.GetFileNum());
	EXPECT_TRUE(cCommandLine.IsViewMode());

	// テストが終わったら要らんので削除してしまう
	std::filesystem::remove( _T(TESTLOCAL_FILE_NAME) );

#undef TESTLOCAL_FILE_NAME
}

/*!
 * @brief ショートカット(.lnk)のパス解決に関する仕様
 */
TEST(CCommandLine, LinkFiles)
{
	// 埋め文字用に超長い文字列を作成する
	std::wstring strPath(_MAX_PATH, L'a');

	// カレントディレクトリに限界長のファイル名のファイルを作る
	WCHAR szAbsPath[_MAX_PATH]{ 0 };
	::_snwprintf_s(szAbsPath, _MAX_PATH - 4, _TRUNCATE, L"%s%s", ToFullPath(L"test"), strPath.c_str());
	::wcscat_s(szAbsPath, L".txt");

	// ファイルパスが既に存在していたら削除して作り直す
	if( fexist( szAbsPath ) ){
		std::filesystem::remove( szAbsPath );
	}

	// ファイルパスにテキトーなテキストファイルを作成する
	{
		std::wofstream local_file( szAbsPath );
		local_file << szAbsPath << std::endl;
	}

	// カレントディレクトリに限界長のファイル名のショートカットを作る
	WCHAR szAbsLinkPath[_MAX_PATH]{ 0 };
	::_snwprintf_s(szAbsLinkPath, _countof(szAbsLinkPath) - 4, _TRUNCATE, L"%s%s", ToFullPath(L"test"), strPath.c_str());
	::wcscat_s(szAbsLinkPath, L".lnk");

	// ローカルショートカットファイル名を取得する
	LPCWSTR pszLinkPath = ::PathFindFileName( szAbsLinkPath );

	// ショートカットをファイルと関連付ける
	EXPECT_TRUE(CreateShortcutLink(szAbsLinkPath, szAbsPath));

	// ショートカットのファイル名を8.3形式に置換する
	WCHAR szShortLinkPath[_MAX_PATH];
	::GetShortPathNameW( pszLinkPath, szShortLinkPath, _countof(szShortLinkPath) );

	CNativeW cmTestCmd;
	cmTestCmd.AppendStringF(L"\"%s\" test.txt", szShortLinkPath);
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(cmTestCmd.GetStringPtr(), false);
	EXPECT_STREQ(szAbsPath, cCommandLine.GetOpenFile());
	EXPECT_STREQ(ToFullPath(L"test.txt"), cCommandLine.GetFileName(0));
	EXPECT_EQ(1, cCommandLine.GetFileNum());

	// テストが終わったら要らんので削除してしまう
	std::filesystem::remove( szAbsLinkPath );
	std::filesystem::remove( szAbsPath );
}

/*!
 * @brief ショートカット(.lnk)のパス解決失敗に関するテスト
 */
TEST(CCommandLine, FailToResolveLink)
{
	// 埋め文字用に超長い文字列を作成する
	std::wstring strPath(_MAX_PATH, L'a');

	// カレントディレクトリに限界超のファイル名のファイルを作る
	WCHAR szAbsPath[_MAX_PATH + 1]{ 0 };
	::_snwprintf_s(szAbsPath, _countof(szAbsPath) - 4, _TRUNCATE, L"%s%s", ToFullPath(L"test"), strPath.c_str());
	::wcscat_s(szAbsPath, L".txt");

	// _MAX_PATH制限突破用にプレフィックスを付けた絶対パスを用意する
	std::wstring absPath(L"\\\\?\\");
	absPath += szAbsPath;

	// ファイルパスにテキトーなテキストファイルを作成する
	{
		std::wofstream local_file( L"test.txt" );
		local_file << szAbsPath << std::endl;
	}

	// 作ったファイルを移動する
	::MoveFileW( L"test.txt", absPath.c_str() );

	// カレントディレクトリに限界長のファイル名のショートカットを作る
	WCHAR szAbsLinkPath[_MAX_PATH - 1]{ 0 };
	::_snwprintf_s(szAbsLinkPath, _countof(szAbsLinkPath) - 4, _TRUNCATE, L"%s%s", ToFullPath(L"test"), strPath.c_str());
	::wcscat_s(szAbsLinkPath, L".lnk");

	// ファイル名を8.3形式に置換する(プレフィックスは外す)
	WCHAR szShortPath[_MAX_PATH];
	::GetShortPathNameW(absPath.c_str(), szShortPath, _countof(szShortPath));
	::wcscpy_s(szShortPath, &szShortPath[4]);

	// ショートカットをファイルと関連付ける
	EXPECT_TRUE(CreateShortcutLink(szAbsLinkPath, szShortPath));

	CNativeW cmTestCmd;
	cmTestCmd.AppendStringF(L"%s test.txt", szAbsLinkPath);
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(cmTestCmd.GetStringPtr(), false);
	EXPECT_STREQ(ToFullPath(L"test.txt"), cCommandLine.GetOpenFile());
	EXPECT_STREQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());

	// 削除するためにファイルを移動する
	::MoveFileW( absPath.c_str(), L"test.txt" );

	// テストが終わったら要らんので削除してしまう
	std::filesystem::remove( szAbsLinkPath );
	std::filesystem::remove( L"test.txt" );
}

/*!
 * @brief 長過ぎるファイルパスに関する仕様
 * @remark _MAX_PATH - 1までのファイル名は利用できる
 */
TEST(CCommandLine, QuotedMaxAbsFilePath)
{
	// 埋め文字用に超長い文字列を作成する
	std::wstring strPath(_MAX_PATH, L'a');

	// カレントディレクトリに限界長のファイル名のファイルを作る
	WCHAR szPath[_MAX_PATH]{ 0 };
	::_snwprintf_s(szPath, _MAX_PATH - 4, _TRUNCATE, L"%s%s", ToFullPath(L"test"), strPath.c_str());
	::wcscat_s(szPath, L".txt");

	CNativeW cmTestCmd;
	cmTestCmd.AppendStringF(L"\"%s\" test.txt", szPath);
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(cmTestCmd.GetStringPtr(), false);
	EXPECT_STREQ(szPath, cCommandLine.GetOpenFile());
	EXPECT_STREQ(ToFullPath(L"test.txt"), cCommandLine.GetFileName(0));
	EXPECT_EQ(1, cCommandLine.GetFileNum());
}

/*!
 * @brief 長過ぎるファイルパスに関する仕様
 * @remark _MAX_PATH - 1までのファイル名は利用できる
 */
TEST(CCommandLine, UnquotedMaxAbsFilePath)
{
	// 埋め文字用に超長い文字列を作成する
	std::wstring strPath(_MAX_PATH, L'a');

	// カレントディレクトリに限界長のファイル名のファイルを作る
	WCHAR szPath[_MAX_PATH]{ 0 };
	::_snwprintf_s(szPath, _MAX_PATH - 4, _TRUNCATE, L"%s%s", ToFullPath(L"test"), strPath.c_str());
	::wcscat_s(szPath, L".txt");

	CNativeW cmTestCmd;
	cmTestCmd.AppendStringF(L"%s test.txt", szPath);
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(cmTestCmd.GetStringPtr(), false);
	EXPECT_STREQ(szPath, cCommandLine.GetOpenFile());
	EXPECT_STREQ(ToFullPath(L"test.txt"), cCommandLine.GetFileName(0));
	EXPECT_EQ(1, cCommandLine.GetFileNum());
}

/*!
 * @brief 長過ぎるファイルパスに関する仕様
 * @remark _MAX_PATH - 1を超えるファイル名は利用できない
 */
TEST(CCommandLine, QuotedTooLongFilePath)
{
	std::wstring strPath(_MAX_PATH, L'a');
	CNativeW cmTestCmd;
	cmTestCmd.AppendStringF(L"\"%s\" test.txt", strPath.c_str());
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(cmTestCmd.GetStringPtr(), false);
	EXPECT_STREQ(ToFullPath(L"test.txt"), cCommandLine.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());
}

/*!
 * @brief 長過ぎるファイルパスに関する仕様
 * @remark _MAX_PATH - 1を超えるファイル名は利用できない
 */
TEST(CCommandLine, UnquotedTooLongFilePath)
{
	std::wstring strPath(_MAX_PATH, L'a');
	CNativeW cmTestCmd;
	cmTestCmd.AppendStringF(L"%s test.txt", strPath.c_str());
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(cmTestCmd.GetStringPtr(), false);
	EXPECT_STREQ(ToFullPath(L"test.txt"), cCommandLine.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());
}

/*!
 * @brief 長過ぎるファイルパスに関する仕様
 * @remark _MAX_PATH - 1を超えるファイル名は利用できない
 */
TEST(CCommandLine, QuotedTooLongFileName)
{
	std::wstring strPath(_MAX_PATH - 1, L'a');
	CNativeW cmTestCmd;
	cmTestCmd.AppendStringF(L"\"%s\" test.txt", strPath.c_str());
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(cmTestCmd.GetStringPtr(), false);
	EXPECT_STREQ(ToFullPath(L"test.txt"), cCommandLine.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());
}

/*!
 * @brief 長過ぎるファイルパスに関する仕様
 * @remark _MAX_PATH - 1を超えるファイル名は利用できない
 */
TEST(CCommandLine, UnquotedTooLongFileName)
{
	std::wstring strPath(_MAX_PATH - 1, L'a');
	CNativeW cmTestCmd;
	cmTestCmd.AppendStringF(L"%s test.txt", strPath.c_str());
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(cmTestCmd.GetStringPtr(), false);
	EXPECT_STREQ(ToFullPath(L"test.txt"), cCommandLine.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());
}

/*!
 * @brief ファイルパスにファイルプロトコルの接頭辞を含めた場合の仕様
 */
TEST(CCommandLine, StripFileProtocol)
{
#define TESTLOCAL_FILE_NAME "test.txt"

	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine( L"file:///" _T(TESTLOCAL_FILE_NAME), false );
	EXPECT_STREQ(ToFullPath(_T(TESTLOCAL_FILE_NAME)), cCommandLine.GetOpenFile());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());

#undef TESTLOCAL_FILE_NAME
}

/*!
 * @brief ファイルパスに「ファイルに使えない文字」を含めた場合の仕様
 */
TEST(CCommandLine, InvalidFilenameChars)
{
	// ファイル名に使えない文字('"'は除外)
	constexpr const wchar_t invalidFilenameChars[] = L"<>?|*";
	for (int n = 0; n < _countof(invalidFilenameChars); ++n ) {
		if( invalidFilenameChars[n] == L'\0' ) break;
		CNativeW cmTestCmd;
		cmTestCmd.AppendStringF( L"\"%c.txt\" test.txt", invalidFilenameChars[n] );
		CCommandLineWrapper cCommandLine;
		cCommandLine.ParseCommandLine( cmTestCmd.GetStringPtr(), false );
		EXPECT_STREQ(ToFullPath(L"test.txt"), cCommandLine.GetOpenFile());
		EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
		EXPECT_EQ(0, cCommandLine.GetFileNum());
	}
}

/*!
 * @brief CCommandLine::ClearFileの仕様
 */
TEST(CCommandLine, ClearFile)
{
	CNativeW cmTestCmd(L"test1.txt test2.txt");
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine(cmTestCmd.GetStringPtr(), false);
	EXPECT_STREQ(ToFullPath(L"test1.txt"), cCommandLine.GetOpenFile());
	EXPECT_STREQ(ToFullPath(L"test2.txt"), cCommandLine.GetFileName(0));
	EXPECT_EQ(1, cCommandLine.GetFileNum());

	cCommandLine.ClearFile(); //クリアする
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
	EXPECT_EQ(0, cCommandLine.GetFileNum());
}
