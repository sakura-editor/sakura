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

#include "EditInfo.h"

/*!
 * 同型との等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しい
 * @retval false 等しくない
 */
bool operator==(const EditInfo &lhs, const EditInfo &rhs) noexcept
{
    if (&lhs == &rhs)
        return true;
    return 0 == wcsncmp(lhs.m_szPath, rhs.m_szPath, _countof(lhs.m_szPath)) && lhs.m_nCharCode == rhs.m_nCharCode && lhs.m_bBom == rhs.m_bBom && 0 == wcsncmp(lhs.m_szDocType, rhs.m_szDocType, _countof(lhs.m_szDocType)) && lhs.m_nTypeId == rhs.m_nTypeId && lhs.m_nViewTopLine == rhs.m_nViewTopLine && lhs.m_nViewLeftCol == rhs.m_nViewLeftCol && lhs.m_ptCursor == rhs.m_ptCursor && lhs.m_bIsModified == rhs.m_bIsModified && lhs.m_bIsGrep == rhs.m_bIsGrep && 0 == wcsncmp(lhs.m_szGrepKey, rhs.m_szGrepKey, _countof(lhs.m_szGrepKey)) && lhs.m_bIsDebug == rhs.m_bIsDebug && 0 == wcsncmp(lhs.m_szMarkLines, rhs.m_szMarkLines, _countof(lhs.m_szMarkLines)) && lhs.m_nWindowSizeX == rhs.m_nWindowSizeX && lhs.m_nWindowSizeY == rhs.m_nWindowSizeY && lhs.m_nWindowOriginX == rhs.m_nWindowOriginX && lhs.m_nWindowOriginY == rhs.m_nWindowOriginY;
}

/*!
 * 同型との否定の等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しくない
 * @retval false 等しい
 */
bool operator!=(const EditInfo &lhs, const EditInfo &rhs) noexcept
{
    return !(lhs == rhs);
}

/*!
 * @brief 等価比較演算子のテスト
 *  初期値同士の等価比較を行う
 */
TEST(EditInfo, operatorEqualSame)
{
    EditInfo value, other;
    ASSERT_EQ(value, other);
}

/*!
 * @brief 等価比較演算子のテスト
 *  自分自身との等価比較を行う
 */
TEST(EditInfo, operatorEqualBySelf)
{
    EditInfo value;
    ASSERT_EQ(value, value);
}

/*!
 * @brief 否定の等価比較演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *
 *  合格条件：メンバの値が1つでも違ったら不一致を検出できること。
 */
TEST(EditInfo, operatorNotEqual)
{
    EditInfo value, other;

    wcscpy_s(value.m_szPath, L"test");
    ASSERT_NE(value, other);
    value.m_szPath[0] = 0;

    value.m_nCharCode = CODE_JIS;
    ASSERT_NE(value, other);
    value.m_nCharCode = other.m_nCharCode;

    value.m_bBom = !other.m_bBom;
    ASSERT_NE(value, other);
    value.m_bBom = other.m_bBom;

    wcscpy_s(value.m_szDocType, L"test");
    ASSERT_NE(value, other);
    value.m_szDocType[0] = 0;

    value.m_nTypeId = 1234;
    ASSERT_NE(value, other);
    value.m_nTypeId = other.m_nTypeId;

    value.m_nViewTopLine = 1234;
    ASSERT_NE(value, other);
    value.m_nViewTopLine = other.m_nViewTopLine;

    value.m_nViewLeftCol = 1234;
    ASSERT_NE(value, other);
    value.m_nViewLeftCol = other.m_nViewLeftCol;

    value.m_ptCursor = CLogicPoint(1234, 5678);
    ASSERT_NE(value, other);
    value.m_ptCursor = other.m_ptCursor;

    value.m_bIsModified = !other.m_bIsModified;
    ASSERT_NE(value, other);
    value.m_bIsModified = other.m_bIsModified;

    value.m_bIsGrep = !other.m_bIsGrep;
    ASSERT_NE(value, other);
    value.m_bIsGrep = other.m_bIsGrep;

    wcscpy_s(value.m_szGrepKey, L"test");
    ASSERT_NE(value, other);
    value.m_szGrepKey[0] = 0;

    value.m_bIsDebug = !other.m_bIsDebug;
    ASSERT_NE(value, other);
    value.m_bIsDebug = other.m_bIsDebug;

    wcscpy_s(value.m_szMarkLines, L"test");
    ASSERT_NE(value, other);
    value.m_szMarkLines[0] = 0;

    value.m_nWindowSizeX = 1234;
    ASSERT_NE(value, other);
    value.m_nWindowSizeX = other.m_nWindowSizeX;

    value.m_nWindowSizeY = 1234;
    ASSERT_NE(value, other);
    value.m_nWindowSizeY = other.m_nWindowSizeY;

    value.m_nWindowOriginX = 1234;
    ASSERT_NE(value, other);
    value.m_nWindowOriginX = other.m_nWindowOriginX;

    value.m_nWindowOriginY = 1234;
    ASSERT_NE(value, other);
    value.m_nWindowOriginY = other.m_nWindowOriginY;
}

/*!
 * @brief 等価比較演算子のテスト
 *  期待結果EQ,期待結果NEでは判定できない、逆条件のテストを行う
 */
TEST(EditInfo, operatorEqualAndNotEqual)
{
    // 初期値同士の比較(等価になる)
    EditInfo v1, v2;

    EXPECT_TRUE(v1 == v2);
    EXPECT_FALSE(v1 != v2);

    // 初期値と値を変えた値の比較(不一致になる)
    v2.m_nWindowOriginY = 1234;
    EXPECT_FALSE(v1 == v2);
    EXPECT_TRUE(v1 != v2);
}
