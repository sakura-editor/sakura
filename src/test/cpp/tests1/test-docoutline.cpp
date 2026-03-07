/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "doc/CDocOutline.h"

#include "outline/CFuncInfo.h"
#include "outline/CFuncInfoArr.h"

#include "testing/CrtAllocHook.hpp"

namespace outline {

#if defined(_MSC_VER) && defined(_DEBUG)
TEST(CFuncInfoArr, AppendData1_101)
{
	// テスト対象
	CFuncInfoArr arr;

	// 追加するデータ
	CFuncInfo func1{
		CLogicInt(1), CLogicInt(1),
		CLayoutInt(1), CLayoutInt(1),
		L"func1",
		L"test.cpp",
		0
	};

	// mallocを失敗させる
	const testing::CrtAllocHook<CFuncInfo*, _HOOK_ALLOC> allocHook;

	// 実行するとクラッシュします。
	EXPECT_DEATH({ arr.AppendData(&func1); }, "");	// 👈バグです。クラッシュではなく、std::bad_alloc例外を投げるべき。
}


TEST(CFuncInfoArr, DISABLED_AppendData1_102)
{
	// テスト対象
	CFuncInfoArr arr;

	// 追加するデータ
	CFuncInfo func1{
		CLogicInt(1), CLogicInt(1),
		CLayoutInt(1), CLayoutInt(1),
		L"func1",
		L"test.cpp",
		0
	};

	// 1個目は普通に追加する
	arr.AppendData(&func1);

	// 追加するデータ
	CFuncInfo func2{
		CLogicInt(1), CLogicInt(1),
		CLayoutInt(1), CLayoutInt(1),
		L"func2",
		L"test.cpp",
		0
	};

	// reallocを失敗させる
	const testing::CrtAllocHook<CFuncInfo*, _HOOK_REALLOC, 2> allocHook;

	// 実行するとクラッシュします。
	EXPECT_DEATH({ arr.AppendData(&func2); }, "");	// 👈バグです。クラッシュではなく、std::bad_alloc例外を投げるべき。

	// ここで、解放されずに紛失した確保済みヒープが原因でクラッシュします。
}

#endif // if defined(_MSC_VER) && defined(_DEBUG)

} // namespace outline
