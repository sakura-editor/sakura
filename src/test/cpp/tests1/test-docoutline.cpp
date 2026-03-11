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

TEST(CFuncInfo, test001)
{
	CFuncInfo info(
		CLogicInt(10),
		CLogicInt(3),
		CLayoutInt(12),
		CLayoutInt(4),
		L"funcA",
		L"test.cpp",
		FL_OBJ_FUNCTION
	);

	EXPECT_THAT(info.m_nFuncLineCRLF, Eq(CLogicInt(10)));
	EXPECT_THAT(info.m_nFuncColCRLF, Eq(CLogicInt(3)));
	EXPECT_THAT(info.m_nFuncLineLAYOUT, Eq(CLayoutInt(12)));
	EXPECT_THAT(info.m_nFuncColLAYOUT, Eq(CLayoutInt(4)));
	EXPECT_THAT(info.m_cmemFuncName.GetStringPtr(), StrEq(L"funcA"));
	EXPECT_THAT(info.m_cmemFileName.GetStringPtr(), StrEq(L"test.cpp"));
	EXPECT_THAT(info.m_nInfo, Eq(FL_OBJ_FUNCTION));
	EXPECT_THAT(info.m_nDepth, Eq(0));
	EXPECT_THAT(info.IsAddClipText(), Eq(true));
}

TEST(CFuncInfo, test002)
{
	CFuncInfo info(
		CLogicInt(20),
		CLogicInt(1),
		CLayoutInt(21),
		CLayoutInt(1),
		L"funcB",
		nullptr,
		FL_OBJ_CLASS | FUNCINFO_NOCLIPTEXT
	);

	EXPECT_THAT(info.m_cmemFuncName.GetStringPtr(), StrEq(L"funcB"));
	EXPECT_THAT(info.m_cmemFileName.GetStringPtr(), IsNull());
	EXPECT_THAT(info.m_nInfo & FUNCINFO_INFOMASK, Eq(FL_OBJ_CLASS));
	EXPECT_THAT(info.IsAddClipText(), Eq(false));
}

TEST(CFuncInfoArr, test001)
{
	CFuncInfoArr arr;

	// 初期状態
	EXPECT_THAT(arr.GetNum(), Eq(0));
	EXPECT_THAT(arr.GetAt(0), IsNull());
	EXPECT_THAT(arr.GetAppendText(1), StrEq(L""));
	EXPECT_THAT(arr.AppendTextLenMax(), Eq(0));

	// 1件追加して内容を確認
	arr.AppendData(
		CLogicInt(10),
		CLogicInt(3),
		CLayoutInt(12),
		CLayoutInt(4),
		L"funcA",
		L"test.cpp",
		FL_OBJ_FUNCTION,
		1
	);
	EXPECT_THAT(arr.GetNum(), Eq(1));

	CFuncInfo* item = arr.GetAt(0);
	ASSERT_THAT(item, NotNull());
	EXPECT_THAT(item->m_nFuncLineCRLF, Eq(CLogicInt(10)));
	EXPECT_THAT(item->m_nFuncColCRLF, Eq(CLogicInt(3)));
	EXPECT_THAT(item->m_nFuncLineLAYOUT, Eq(CLayoutInt(12)));
	EXPECT_THAT(item->m_nFuncColLAYOUT, Eq(CLayoutInt(4)));
	EXPECT_THAT(item->m_cmemFuncName.GetStringPtr(), StrEq(L"funcA"));
	EXPECT_THAT(item->m_cmemFileName.GetStringPtr(), StrEq(L"test.cpp"));
	EXPECT_THAT(item->m_nInfo, Eq(FL_OBJ_FUNCTION));
	EXPECT_THAT(item->m_nDepth, Eq(1));

	// 5引数版のAppendData呼び出しを確認（桁は既定値1、ファイル名はnullptr）
	arr.AppendData(CLogicInt(20), CLayoutInt(22), L"funcB", FL_OBJ_CLASS, 2);
	EXPECT_THAT(arr.GetNum(), Eq(2));

	CFuncInfo* item2 = arr.GetAt(1);
	ASSERT_THAT(item2, NotNull());
	EXPECT_THAT(item2->m_nFuncLineCRLF, Eq(CLogicInt(20)));
	EXPECT_THAT(item2->m_nFuncColCRLF, Eq(CLogicInt(1)));
	EXPECT_THAT(item2->m_nFuncLineLAYOUT, Eq(CLayoutInt(22)));
	EXPECT_THAT(item2->m_nFuncColLAYOUT, Eq(CLayoutInt(1)));
	EXPECT_THAT(item2->m_cmemFuncName.GetStringPtr(), StrEq(L"funcB"));
	EXPECT_THAT(item2->m_cmemFileName.GetStringPtr(), IsNull());
	EXPECT_THAT(item2->m_nInfo, Eq(FL_OBJ_CLASS));
	EXPECT_THAT(item2->m_nDepth, Eq(2));

	// 追加テキストの管理を確認
	arr.SetAppendText(FL_OBJ_FUNCTION, L"[F]", false);
	EXPECT_THAT(arr.GetAppendText(FL_OBJ_FUNCTION), StrEq(L"[F]"));
	EXPECT_THAT(arr.AppendTextLenMax(), Eq(3));

	arr.SetAppendText(FL_OBJ_FUNCTION, L"[FUNC]", false);
	EXPECT_THAT(arr.GetAppendText(FL_OBJ_FUNCTION), StrEq(L"[F]"));

	arr.SetAppendText(FL_OBJ_FUNCTION, L"[FUNC]", true);
	EXPECT_THAT(arr.GetAppendText(FL_OBJ_FUNCTION), StrEq(L"[FUNC]"));

	// 呼ぶだけ
	arr.DUMP();

	// 空に戻ることを確認
	arr.Empty();
	EXPECT_THAT(arr.GetNum(), Eq(0));
	EXPECT_THAT(arr.GetAt(0), IsNull());
	EXPECT_THAT(arr.GetAppendText(FL_OBJ_FUNCTION), StrEq(L""));
	EXPECT_THAT(arr.AppendTextLenMax(), Eq(0));
}

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
