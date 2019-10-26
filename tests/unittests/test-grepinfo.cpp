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

/*!
 * @brief 等価比較演算子のテスト
 *  初期値同士の等価比較を行う
 */
TEST(GrepInfo, operatorEqualSame)
{
	GrepInfo value, other;

	EXPECT_TRUE(value == other);
	EXPECT_FALSE(value != other);
	ASSERT_EQ(value, other);
}

/*!
 * @brief 等価比較演算子のテスト
 *  自分自身との等価比較を行う
 */
TEST(GrepInfo, operatorEqualBySelf)
{
	GrepInfo value;

	EXPECT_TRUE(value == value);
	EXPECT_FALSE(value != value);
	ASSERT_EQ(value, value);
}

/*!
 * @brief 否定の等価比較演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *
 *  合格条件：メンバの値が1つでも違ったら不一致を検出できること。
 */
TEST(GrepInfo, operatorNotEqual)
{
	GrepInfo value, other;

	value.cmGrepKey = L"ぐれっぷ";
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.cmGrepKey = other.cmGrepKey;

	value.cmGrepRep = L"ちかん";
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.cmGrepRep = other.cmGrepRep;

	value.cmGrepFile = L"#.git;*.*";
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.cmGrepFile = other.cmGrepFile;

	value.cmGrepFolder = L"C:\\work\\sakura";
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.cmGrepFolder = other.cmGrepFolder;

	value.cmExcludeFile = L"除外ファイルの仕様がなんでここに残ってんだっけ？";
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.cmExcludeFile = other.cmExcludeFile;

	value.cmExcludeFolder = L"除外フォルダの仕様がなんで(ry";
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.cmExcludeFolder = other.cmExcludeFolder;

	value.sGrepSearchOption.bRegularExp = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.sGrepSearchOption = other.sGrepSearchOption;

	value.sGrepSearchOption.bLoHiCase = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.sGrepSearchOption = other.sGrepSearchOption;

	value.sGrepSearchOption.bWordOnly = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.sGrepSearchOption = other.sGrepSearchOption;

	value.bGrepCurFolder = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.bGrepCurFolder = other.bGrepCurFolder;

	value.bGrepStdout = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.bGrepStdout = other.bGrepStdout;

	value.bGrepHeader = false;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.bGrepHeader = other.bGrepHeader;

	value.bGrepSubFolder = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.bGrepSubFolder = other.bGrepSubFolder;

	value.nGrepCharSet = CODE_EUC;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.nGrepCharSet = other.nGrepCharSet;

	value.nGrepOutputStyle = 2;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.nGrepOutputStyle = other.nGrepOutputStyle;

	value.nGrepOutputLineType = 2;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.nGrepOutputLineType = other.nGrepOutputLineType;

	value.bGrepOutputFileOnly = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.bGrepOutputFileOnly = other.bGrepOutputFileOnly;

	value.bGrepOutputBaseFolder = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.bGrepOutputBaseFolder = other.bGrepOutputBaseFolder;

	value.bGrepSeparateFolder = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.bGrepSeparateFolder = other.bGrepSeparateFolder;

	value.bGrepReplace = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.bGrepReplace = other.bGrepReplace;

	value.bGrepPaste = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.bGrepPaste = other.bGrepPaste;

	value.bGrepBackup = true;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.bGrepBackup = other.bGrepBackup;
}
