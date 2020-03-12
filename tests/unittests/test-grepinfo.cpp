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

#include "GrepInfo.h"

/*!
 * 同型との等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しい
 * @retval false 等しくない
 */
bool operator == (const GrepInfo& lhs, const GrepInfo& rhs) noexcept {
	if (&lhs == &rhs) return true;
	return lhs.cmGrepKey == rhs.cmGrepKey
		&& lhs.cmGrepRep == rhs.cmGrepRep
		&& lhs.cmGrepFile == rhs.cmGrepFile
		&& lhs.cmGrepFolder == rhs.cmGrepFolder
		&& lhs.sGrepSearchOption == rhs.sGrepSearchOption
		&& lhs.bGrepCurFolder == rhs.bGrepCurFolder
		&& lhs.bGrepStdout == rhs.bGrepStdout
		&& lhs.bGrepHeader == rhs.bGrepHeader
		&& lhs.bGrepSubFolder == rhs.bGrepSubFolder
		&& lhs.nGrepCharSet == rhs.nGrepCharSet
		&& lhs.nGrepOutputStyle == rhs.nGrepOutputStyle
		&& lhs.nGrepOutputLineType == rhs.nGrepOutputLineType
		&& lhs.bGrepOutputFileOnly == rhs.bGrepOutputFileOnly
		&& lhs.bGrepOutputBaseFolder == rhs.bGrepOutputBaseFolder
		&& lhs.bGrepSeparateFolder == rhs.bGrepSeparateFolder
		&& lhs.bGrepReplace == rhs.bGrepReplace
		&& lhs.bGrepPaste == rhs.bGrepPaste
		&& lhs.bGrepBackup == rhs.bGrepBackup;
}

/*!
 * 同型との否定の等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しくない
 * @retval false 等しい
 */
bool operator != (const GrepInfo& lhs, const GrepInfo& rhs) noexcept
{
	return !(lhs == rhs);
}

/*!
 * @brief 等価比較演算子のテスト
 *  初期値同士の等価比較を行う
 */
TEST(GrepInfo, operatorEqualSame)
{
	GrepInfo value, other;
	ASSERT_EQ(value, other);
}

/*!
 * @brief 等価比較演算子のテスト
 *  自分自身との等価比較を行う
 */
TEST(GrepInfo, operatorEqualBySelf)
{
	GrepInfo value;
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
	ASSERT_NE(value, other);
	value.cmGrepKey = other.cmGrepKey;

	value.cmGrepRep = L"ちかん";
	ASSERT_NE(value, other);
	value.cmGrepRep = other.cmGrepRep;

	value.cmGrepFile = L"#.git;*.*";
	ASSERT_NE(value, other);
	value.cmGrepFile = other.cmGrepFile;

	value.cmGrepFolder = L"C:\\work\\sakura";
	ASSERT_NE(value, other);
	value.cmGrepFolder = other.cmGrepFolder;

	value.sGrepSearchOption.bRegularExp = true;
	ASSERT_NE(value, other);
	value.sGrepSearchOption = other.sGrepSearchOption;

	value.sGrepSearchOption.bLoHiCase = true;
	ASSERT_NE(value, other);
	value.sGrepSearchOption = other.sGrepSearchOption;

	value.sGrepSearchOption.bWordOnly = true;
	ASSERT_NE(value, other);
	value.sGrepSearchOption = other.sGrepSearchOption;

	value.bGrepCurFolder = true;
	ASSERT_NE(value, other);
	value.bGrepCurFolder = other.bGrepCurFolder;

	value.bGrepStdout = true;
	ASSERT_NE(value, other);
	value.bGrepStdout = other.bGrepStdout;

	value.bGrepHeader = false;
	ASSERT_NE(value, other);
	value.bGrepHeader = other.bGrepHeader;

	value.bGrepSubFolder = true;
	ASSERT_NE(value, other);
	value.bGrepSubFolder = other.bGrepSubFolder;

	value.nGrepCharSet = CODE_EUC;
	ASSERT_NE(value, other);
	value.nGrepCharSet = other.nGrepCharSet;

	value.nGrepOutputStyle = 2;
	ASSERT_NE(value, other);
	value.nGrepOutputStyle = other.nGrepOutputStyle;

	value.nGrepOutputLineType = 2;
	ASSERT_NE(value, other);
	value.nGrepOutputLineType = other.nGrepOutputLineType;

	value.bGrepOutputFileOnly = true;
	ASSERT_NE(value, other);
	value.bGrepOutputFileOnly = other.bGrepOutputFileOnly;

	value.bGrepOutputBaseFolder = true;
	ASSERT_NE(value, other);
	value.bGrepOutputBaseFolder = other.bGrepOutputBaseFolder;

	value.bGrepSeparateFolder = true;
	ASSERT_NE(value, other);
	value.bGrepSeparateFolder = other.bGrepSeparateFolder;

	value.bGrepReplace = true;
	ASSERT_NE(value, other);
	value.bGrepReplace = other.bGrepReplace;

	value.bGrepPaste = true;
	ASSERT_NE(value, other);
	value.bGrepPaste = other.bGrepPaste;

	value.bGrepBackup = true;
	ASSERT_NE(value, other);
	value.bGrepBackup = other.bGrepBackup;
}

/*!
 * @brief 等価比較演算子のテスト
 *  期待結果EQ,期待結果NEでは判定できない、逆条件のテストを行う
 */
TEST(GrepInfo, operatorEqualAndNotEqual)
{
	// 初期値同士の比較(等価になる)
	GrepInfo v1, v2;

	EXPECT_TRUE(v1 == v2);
	EXPECT_FALSE(v1 != v2);

	// 初期値と値を変えた値の比較(不一致になる)
	v2.bGrepBackup = true;
	EXPECT_FALSE(v1 == v2);
	EXPECT_TRUE(v1 != v2);
}
