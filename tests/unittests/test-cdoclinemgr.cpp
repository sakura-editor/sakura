/*
	Copyright (C) 2021-2022, Sakura Editor Organization

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

#include "pch.h"
#include "doc/logic/CDocLineMgr.h"

#include <array>

TEST(CDocLineMgr, ListManipulations)
{
	CDocLineMgr m;

	// 初期状態。行数0。
	EXPECT_EQ(m.GetLineCount(), 0);
	EXPECT_EQ(m.GetLine(CLogicInt(0)), nullptr);
	EXPECT_EQ(m.GetLine(CLogicInt(1)), nullptr);
	EXPECT_EQ(m.GetDocLineTop(), nullptr);
	EXPECT_EQ(m.GetDocLineBottom(), nullptr);

	// 1行追加する。行数1。
	CDocLine* a = m.AddNewLine();
	ASSERT_NE(a, nullptr);
	a->SetDocLineString(L"A", 1, false);
	EXPECT_EQ(m.GetLineCount(), 1);
	EXPECT_EQ(m.GetLine(CLogicInt(0)), a);
	EXPECT_EQ(m.GetLine(CLogicInt(1)), nullptr);
	EXPECT_EQ(m.GetDocLineTop(), a);
	EXPECT_EQ(m.GetDocLineBottom(), a);

	// 先頭に1行入れる。行数2。
	CDocLine* b = m.InsertNewLine(m.GetDocLineTop());
	ASSERT_NE(b, nullptr);
	b->SetDocLineString(L"B", 1, false);
	EXPECT_EQ(m.GetLineCount(), 2);
	//EXPECT_EQ(m.GetLine(CLogicInt(0)), b);
	//EXPECT_EQ(m.GetLine(CLogicInt(1)), a);
	EXPECT_EQ(m.GetLine(CLogicInt(2)), nullptr);
	EXPECT_EQ(m.GetDocLineTop(), b);
	EXPECT_EQ(m.GetDocLineBottom(), a);

	// 中間に1行入れる。行数3。
	CDocLine* c = m.InsertNewLine(m.GetDocLineBottom());
	ASSERT_NE(c, nullptr);
	c->SetDocLineString(L"C", 1, false);
	EXPECT_EQ(m.GetLineCount(), 3);
	//EXPECT_EQ(m.GetLine(CLogicInt(0)), b);
	//EXPECT_EQ(m.GetLine(CLogicInt(1)), c);
	EXPECT_EQ(m.GetLine(CLogicInt(2)), a);
	EXPECT_EQ(m.GetDocLineTop(), b);
	EXPECT_EQ(m.GetDocLineBottom(), a);

	// 末尾に1行追加する。行数4。
	CDocLine* d = m.InsertNewLine(nullptr);
	ASSERT_NE(d, nullptr);
	d->SetDocLineString(L"D", 1, false);
	EXPECT_EQ(m.GetLineCount(), 4);
	EXPECT_EQ(m.GetLine(CLogicInt(0)), b);
	EXPECT_EQ(m.GetLine(CLogicInt(1)), c);
	EXPECT_EQ(m.GetLine(CLogicInt(2)), a);
	EXPECT_EQ(m.GetLine(CLogicInt(3)), d);
	EXPECT_EQ(m.GetDocLineTop(), b);
	EXPECT_EQ(m.GetDocLineBottom(), d);

	// 末尾に1行追加する。行数5。
	CDocLine* e = m.AddNewLine();
	ASSERT_NE(e, nullptr);
	e->SetDocLineString(L"E", 1, false);
	EXPECT_EQ(m.GetLineCount(), 5);
	EXPECT_EQ(m.GetLine(CLogicInt(0)), b);
	EXPECT_EQ(m.GetLine(CLogicInt(1)), c);
	EXPECT_EQ(m.GetLine(CLogicInt(2)), a);
	EXPECT_EQ(m.GetLine(CLogicInt(4)), e);
	EXPECT_EQ(m.GetLine(CLogicInt(3)), d);
	EXPECT_EQ(m.GetDocLineTop(), b);
	EXPECT_EQ(m.GetDocLineBottom(), e);

	// 先頭の1行を削除する。行数4。
	m.DeleteLine(b);
	EXPECT_EQ(m.GetLineCount(), 4);
	EXPECT_EQ(m.GetLine(CLogicInt(0)), c);
	EXPECT_EQ(m.GetLine(CLogicInt(1)), a);
	EXPECT_EQ(m.GetLine(CLogicInt(2)), d);
	EXPECT_EQ(m.GetLine(CLogicInt(3)), e);
	EXPECT_EQ(m.GetDocLineTop(), c);
	EXPECT_EQ(m.GetDocLineBottom(), e);

	// 中間の1行を削除する。行数3。
	m.DeleteLine(a);
	EXPECT_EQ(m.GetLineCount(), 3);
	EXPECT_EQ(m.GetLine(CLogicInt(0)), c);
	EXPECT_EQ(m.GetLine(CLogicInt(1)), d);
	EXPECT_EQ(m.GetLine(CLogicInt(2)), e);
	EXPECT_EQ(m.GetDocLineTop(), c);
	EXPECT_EQ(m.GetDocLineBottom(), e);

	// 末尾の1行を削除する。行数2。
	m.DeleteLine(e);
	EXPECT_EQ(m.GetLineCount(), 2);
	EXPECT_EQ(m.GetLine(CLogicInt(0)), c);
	EXPECT_EQ(m.GetLine(CLogicInt(1)), d);
	EXPECT_EQ(m.GetDocLineTop(), c);
	EXPECT_EQ(m.GetDocLineBottom(), d);

	// すべて削除する。行数0。
	m.DeleteAllLine();
	EXPECT_EQ(m.GetLineCount(), 0);
	EXPECT_EQ(m.GetLine(CLogicInt(0)), nullptr);
	EXPECT_EQ(m.GetLine(CLogicInt(1)), nullptr);
	EXPECT_EQ(m.GetDocLineTop(), nullptr);
	EXPECT_EQ(m.GetDocLineBottom(), nullptr);

	// 最後の行を DeleteLine で削除するケース。
	m.DeleteLine(m.AddNewLine());
	EXPECT_EQ(m.GetLineCount(), 0);
	EXPECT_EQ(m.GetLine(CLogicInt(0)), nullptr);
	EXPECT_EQ(m.GetLine(CLogicInt(1)), nullptr);
	EXPECT_EQ(m.GetDocLineTop(), nullptr);
	EXPECT_EQ(m.GetDocLineBottom(), nullptr);
}

TEST(CDocLineMgr, RandomAccess)
{
	std::array<CDocLine*, 100> lines;
	CDocLineMgr m;
	for (int i = 0; i < lines.size(); ++i)
		lines[i] = m.AddNewLine();

	EXPECT_EQ(m.GetLine(CLogicInt(10)), lines[10]);
	EXPECT_EQ(m.GetLine(CLogicInt(90)), lines[90]);
	EXPECT_EQ(m.GetLine(CLogicInt(60)), lines[60]);
	EXPECT_EQ(m.GetLine(CLogicInt(70)), lines[70]);
}
