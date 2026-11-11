/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "outline/CFuncInfoArr.h"
#include "outline/CFuncInfo.h"

/*!
	@brief AppendData adds elements and GetNum reflects count.
 */
TEST(CFuncInfoArr, AppendData_IncreasesCount)
{
	CFuncInfoArr arr;
	ASSERT_EQ(0, arr.GetNum());

	auto* pInfo1 = new CFuncInfo(
		CLogicInt(1), CLogicInt(0),
		CLayoutInt(1), CLayoutInt(0),
		L"func1", L"file.cpp", 0);
	arr.AppendData(pInfo1);
	ASSERT_EQ(1, arr.GetNum());

	auto* pInfo2 = new CFuncInfo(
		CLogicInt(2), CLogicInt(0),
		CLayoutInt(2), CLayoutInt(0),
		L"func2", L"file.cpp", 0);
	arr.AppendData(pInfo2);
	ASSERT_EQ(2, arr.GetNum());
}

/*!
	@brief AppendData stores retrievable elements via GetAt.
 */
TEST(CFuncInfoArr, AppendData_GetAtReturnsCorrectElement)
{
	CFuncInfoArr arr;

	auto* pInfo = new CFuncInfo(
		CLogicInt(10), CLogicInt(5),
		CLayoutInt(10), CLayoutInt(5),
		L"myFunc", L"test.cpp", FL_OBJ_FUNCTION);
	arr.AppendData(pInfo);

	CFuncInfo* pResult = arr.GetAt(0);
	ASSERT_NE(nullptr, pResult);
	ASSERT_EQ(pInfo, pResult);
}
