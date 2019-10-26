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

#include "EditInfo.h"

/*!
 * @brief 等価演算子のテスト
 *  初期値同士の等価比較を行う
 */
TEST(EditInfo, operatorEqualSame)
{
	EditInfo value, other;

	EXPECT_TRUE(value == other);
	EXPECT_FALSE(value != other);
	ASSERT_EQ(value, other);
}

/*!
 * @brief 等価演算子のテスト
 *  自分自身との等価比較を行う
 */
TEST(EditInfo, operatorEqualBySelf)
{
	EditInfo value;

	EXPECT_TRUE(value == value);
	EXPECT_FALSE(value != value);
	ASSERT_EQ(value, value);
}

/*!
 * @brief 否定の等価演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *
 *  合格条件：メンバの値が1つでも違ったら不一致を検出できること。
 */
TEST(EditInfo, operatorNotEqual)
{
	EditInfo value, other;

	wcscpy_s(value.m_szPath, L"test");
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_szPath[0] = 0;
	
	value.m_nCharCode = CODE_JIS;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_nCharCode = other.m_nCharCode;

	value.m_bBom = !other.m_bBom;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_bBom = other.m_bBom;

	wcscpy_s(value.m_szDocType, L"test");
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_szDocType[0] = 0;

	value.m_nTypeId = 1234;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_nTypeId = other.m_nTypeId;

	value.m_nViewTopLine = 1234;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_nViewTopLine = other.m_nViewTopLine;

	value.m_nViewLeftCol = 1234;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_nViewLeftCol = other.m_nViewLeftCol;

	value.m_ptCursor = CLogicPoint(1234, 5678);
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_ptCursor = other.m_ptCursor;

	value.m_bIsModified = !other.m_bIsModified;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_bIsModified = other.m_bIsModified;

	value.m_bIsGrep = !other.m_bIsGrep;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_bIsGrep = other.m_bIsGrep;

	wcscpy_s(value.m_szGrepKey, L"test");
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_szGrepKey[0] = 0;

	value.m_bIsDebug = !other.m_bIsDebug;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_bIsDebug = other.m_bIsDebug;

	wcscpy_s(value.m_szMarkLines, L"test");
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_szMarkLines[0] = 0;

	value.m_nWindowSizeX = 1234;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_nWindowSizeX = other.m_nWindowSizeX;

	value.m_nWindowSizeY = 1234;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_nWindowSizeY = other.m_nWindowSizeY;

	value.m_nWindowOriginX = 1234;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_nWindowOriginX = other.m_nWindowOriginX;

	value.m_nWindowOriginY = 1234;
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
	value.m_nWindowOriginY = other.m_nWindowOriginY;
}
