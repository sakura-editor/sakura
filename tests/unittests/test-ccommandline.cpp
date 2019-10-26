/*! @file */
/*
	Copyright (C) 2018-2019 Sakura Editor Organization

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

#include <cstdlib>
#include <fstream>

/*!
 * @brief コンストラクタ(パラメータなし)の仕様
 * @remark パラメータを何も指定しなかった状態になる
 */
TEST(CCommandLine, ConstructWithoutParam)
{
	CCommandLine cCommandLine;
	EXPECT_FALSE(cCommandLine.IsNoWindow());
	EXPECT_FALSE(cCommandLine.IsWriteQuit());
	EXPECT_FALSE(cCommandLine.IsGrepMode());
	EXPECT_FALSE(cCommandLine.IsGrepDlg());
	EXPECT_FALSE(cCommandLine.IsDebugMode());
	EXPECT_FALSE(cCommandLine.IsViewMode());

	//GetEditInfo: テストしづらい上に戻り値true以外は返らない・・・仕様バグですね(^^;
	EditInfo fi;
	EXPECT_TRUE(cCommandLine.GetEditInfo(&fi));
	EXPECT_EQ(EditInfo(), fi);

	//GetGrepInfo: テストしづらい上に戻り値true以外は返らない・・・仕様バグですね(^^;
	GrepInfo gi;
	EXPECT_TRUE(cCommandLine.GetGrepInfo(&gi));
	//TODO: GrepInfoに等価比較演算子を実装する
	//EXPECT_EQ(GrepInfo(), gi);

	EXPECT_EQ(-1, cCommandLine.GetGroupId());
	EXPECT_EQ(NULL, cCommandLine.GetMacro());
	EXPECT_EQ(NULL, cCommandLine.GetMacroType());
	//1つだけNULLを返さないのはバグの気配がします(^^;
	EXPECT_STREQ(L"", cCommandLine.GetProfileName());
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
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.IsNoWindow());
	cCommandLine.ParseCommandLine(L"-NOWIN", false);
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
	CCommandLine cCommandLine;
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
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_FALSE(cCommandLine.IsGrepMode());
	cCommandLine.ParseCommandLine(L"-GREPMODE", false);
	ASSERT_TRUE(cCommandLine.IsGrepMode());

	//TODO: 現状のコードではテストを書きづらい...
	//Grepモード時は文書タイプが"grepout"になる
	//ASSERT_STREQ(L"grepout", cCommandLine.GetDocType());
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

	//TODO: 現状のコードではテストを書きづらい...
	//Debugモード時は文書タイプが"output"になる
	//ASSERT_STREQ(L"output", cCommandLine.GetDocType());
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
 * @brief パラメータ解析(ファイル名)の仕様
 * @remark GetFileNumで取得できるのは2つ目以降のファイル数
 * @remark GetFileName(0)で取得できるのは2つ目のファイル名
 */
TEST(CCommandLine, ParseOpenFilename)
{
	//TODO: 現状のコードではテストを書きづらい...
	//・ファイルは複数指定できる
	//・1つ目のファイルは EditInfo に格納される
	//・2つ目以降のファイルは vector に格納される

	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);
	EXPECT_EQ(0, cCommandLine.GetFileNum());
	EXPECT_EQ(NULL, cCommandLine.GetFileName(0));
#define TESTLOCAL_OPEN_FILENAME L"test2.txt"
	cCommandLine.ParseCommandLine(L"test1.txt " TESTLOCAL_OPEN_FILENAME, false);
	EXPECT_EQ(1, cCommandLine.GetFileNum());
	std::wstring additionalFile(cCommandLine.GetFileName(0));
	ASSERT_NE(0, additionalFile.find(TESTLOCAL_OPEN_FILENAME));
#undef TESTLOCAL_OPEN_FILENAME
	cCommandLine.ClearFile();
	EXPECT_EQ(0, cCommandLine.GetFileNum());
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
}
