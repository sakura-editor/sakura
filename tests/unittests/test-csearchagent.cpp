/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "CSaveAgent.h"

#include <array>
#include <initializer_list>
#include <string_view>
#include <utility>
#include "doc/logic/CDocLineMgr.h"

namespace {

template <typename T> void SetLines(CDocLineMgr& m, int seq, T begin, T end)
{
	for (auto it = begin; it != end; ++it) {
		CDocLine* line = m.AddNewLine();
		line->SetDocLineString(it->data(), it->length());
		line->m_sMark.m_cModified = seq;
	}
}

void SetLines(CDocLineMgr& m, int seq, std::initializer_list<std::wstring_view> args)
{
	SetLines(m, seq, args.begin(), args.end());
}

struct RawLineData {
	const wchar_t* line;
	int seq;
};

COpeLineData MakeOpeLineData(std::initializer_list<RawLineData> lines)
{
	COpeLineData data;
	for (RawLineData rawLine : lines) {
		CLineData line;
		line.cmemLine = rawLine.line;
		line.nSeq = rawLine.seq;
		data.push_back(line);
	}
	return data;
}

}

/*!
	CSearchAgent::ReplaceData のテスト

	行の一部を置き換える。
 */
TEST(CSearchAgent, ReplaceData1)
{
	CDocLineMgr m;
	SetLines(m, 1, {L"AAA\n", L"BBB\n", L"CCC\n"});

	COpeLineData insData = MakeOpeLineData({{L"DDD", 2}});
	COpeLineData delData;

	DocLineReplaceArg arg;
	arg.sDelRange = CLogicRange(CLogicPoint(0, 1), CLogicPoint(3, 1));
	arg.pcmemDeleted = &delData;
	arg.pInsData = &insData;
	arg.nDelSeq = 1;
	arg.nInsSeq = -1;
	CSearchAgent(&m).ReplaceData(&arg, false);

	EXPECT_EQ(m.GetLineCount(), 3);
	EXPECT_STREQ(m.GetLine(CLogicInt(0))->GetPtr(), L"AAA\n");
	EXPECT_EQ(m.GetLine(CLogicInt(0))->m_sMark.m_cModified.GetSeq(), 1);
	EXPECT_STREQ(m.GetLine(CLogicInt(1))->GetPtr(), L"DDD\n");
	EXPECT_EQ(m.GetLine(CLogicInt(1))->m_sMark.m_cModified.GetSeq(), 2);
	EXPECT_STREQ(m.GetLine(CLogicInt(2))->GetPtr(), L"CCC\n");
	EXPECT_EQ(m.GetLine(CLogicInt(2))->m_sMark.m_cModified.GetSeq(), 1);

	EXPECT_EQ(arg.nInsSeq, 1);
	EXPECT_EQ(arg.pcmemDeleted->size(), 1);
	EXPECT_STREQ(arg.pcmemDeleted->at(0).cmemLine.GetStringPtr(), L"BBB");
	EXPECT_EQ(arg.pcmemDeleted->at(0).nSeq, 1);
	EXPECT_EQ(arg.nDeletedLineNum, 0);
	EXPECT_EQ(arg.nInsLineNum, 0);
	EXPECT_EQ(arg.ptNewPos, CLogicPoint(3, 1));
}

/*!
	CSearchAgent::ReplaceData のテスト

	行全体を置き換える。
 */
TEST(CSearchAgent, ReplaceData2)
{
	CDocLineMgr m;
	SetLines(m, 1, {L"AAA\n", L"BBB\n", L"CCC\n"});

	COpeLineData insData = MakeOpeLineData({{L"DDD\n", 2}});
	COpeLineData delData;

	DocLineReplaceArg arg;
	arg.sDelRange = CLogicRange(CLogicPoint(0, 1), CLogicPoint(4, 1));
	arg.pcmemDeleted = &delData;
	arg.pInsData = &insData;
	arg.nDelSeq = 1;
	arg.nInsSeq = -1;
	CSearchAgent(&m).ReplaceData(&arg, false);

	EXPECT_EQ(m.GetLineCount(), 3);
	EXPECT_STREQ(m.GetLine(CLogicInt(0))->GetPtr(), L"AAA\n");
	EXPECT_EQ(m.GetLine(CLogicInt(0))->m_sMark.m_cModified.GetSeq(), 1);
	EXPECT_STREQ(m.GetLine(CLogicInt(1))->GetPtr(), L"DDD\n");
	EXPECT_EQ(m.GetLine(CLogicInt(1))->m_sMark.m_cModified.GetSeq(), 2);
	EXPECT_STREQ(m.GetLine(CLogicInt(2))->GetPtr(), L"CCC\n");
	EXPECT_EQ(m.GetLine(CLogicInt(2))->m_sMark.m_cModified.GetSeq(), 1);

	EXPECT_EQ(arg.nInsSeq, 0);
	EXPECT_EQ(arg.pcmemDeleted->size(), 1);
	EXPECT_STREQ(arg.pcmemDeleted->at(0).cmemLine.GetStringPtr(), L"BBB\n");
	EXPECT_EQ(arg.pcmemDeleted->at(0).nSeq, 1);
	EXPECT_EQ(arg.nDeletedLineNum, 1);
	EXPECT_EQ(arg.nInsLineNum, 1);
	EXPECT_EQ(arg.ptNewPos, CLogicPoint(0, 2));
}

/*!
	CSearchAgent::ReplaceData のテスト

	行末の改行を削除する。
 */
TEST(CSearchAgent, ReplaceData3)
{
	CDocLineMgr m;
	SetLines(m, 1, {L"AAA\n", L"BBB\n", L"CCC\n"});

	COpeLineData delData;

	DocLineReplaceArg arg;
	arg.sDelRange = CLogicRange(CLogicPoint(3, 1), CLogicPoint(4, 1));
	arg.pcmemDeleted = &delData;
	arg.pInsData = nullptr;
	arg.nDelSeq = 1;
	arg.nInsSeq = -1;
	CSearchAgent(&m).ReplaceData(&arg, false);

	EXPECT_EQ(m.GetLineCount(), 2);
	EXPECT_STREQ(m.GetLine(CLogicInt(0))->GetPtr(), L"AAA\n");
	EXPECT_EQ(m.GetLine(CLogicInt(0))->m_sMark.m_cModified.GetSeq(), 1);
	EXPECT_STREQ(m.GetLine(CLogicInt(1))->GetPtr(), L"BBBCCC\n");
	EXPECT_EQ(m.GetLine(CLogicInt(1))->m_sMark.m_cModified.GetSeq(), 1);

	EXPECT_EQ(arg.nInsSeq, 0);
	EXPECT_EQ(arg.pcmemDeleted->size(), 2);
	EXPECT_STREQ(arg.pcmemDeleted->at(0).cmemLine.GetStringPtr(), L"\n");
	EXPECT_EQ(arg.pcmemDeleted->at(0).nSeq, 1);
	EXPECT_STREQ(arg.pcmemDeleted->at(1).cmemLine.GetStringPtr(), L"");
	EXPECT_EQ(arg.pcmemDeleted->at(1).nSeq, 1);
	EXPECT_EQ(arg.nDeletedLineNum, 1);
	EXPECT_EQ(arg.nInsLineNum, 0);
	EXPECT_EQ(arg.ptNewPos, CLogicPoint(3, 1));
}

/*!
	CSearchAgent::ReplaceData のテスト

	行末の改行を削除する。削除するデータ長が長いケース。
 */
TEST(CSearchAgent, ReplaceData4)
{
	CDocLineMgr m;
	SetLines(m, 1, {L"AAA\n", L"BBB\n", L"CCC\n"});

	COpeLineData delData;

	DocLineReplaceArg arg;
	arg.sDelRange = CLogicRange(CLogicPoint(1, 1), CLogicPoint(4, 1));
	arg.pcmemDeleted = &delData;
	arg.pInsData = nullptr;
	arg.nDelSeq = 1;
	arg.nInsSeq = -1;
	CSearchAgent(&m).ReplaceData(&arg, false);

	EXPECT_EQ(m.GetLineCount(), 2);
	EXPECT_STREQ(m.GetLine(CLogicInt(0))->GetPtr(), L"AAA\n");
	EXPECT_EQ(m.GetLine(CLogicInt(0))->m_sMark.m_cModified.GetSeq(), 1);
	EXPECT_STREQ(m.GetLine(CLogicInt(1))->GetPtr(), L"BCCC\n");
	EXPECT_EQ(m.GetLine(CLogicInt(1))->m_sMark.m_cModified.GetSeq(), 1);

	EXPECT_EQ(arg.nInsSeq, 0);
	EXPECT_EQ(arg.pcmemDeleted->size(), 2);
	EXPECT_STREQ(arg.pcmemDeleted->at(0).cmemLine.GetStringPtr(), L"BB\n");
	EXPECT_EQ(arg.pcmemDeleted->at(0).nSeq, 1);
	EXPECT_STREQ(arg.pcmemDeleted->at(1).cmemLine.GetStringPtr(), L"");
	EXPECT_EQ(arg.pcmemDeleted->at(1).nSeq, 1);
	EXPECT_EQ(arg.nDeletedLineNum, 1);
	EXPECT_EQ(arg.nInsLineNum, 0);
	EXPECT_EQ(arg.ptNewPos, CLogicPoint(1, 1));
}

/*!
	CSearchAgent::ReplaceData のテスト

	行末の改行を削除する。対象行がデータ末尾であるケース。
 */
TEST(CSearchAgent, ReplaceData5)
{
	CDocLineMgr m;
	SetLines(m, 1, {L"AAA\n"});

	COpeLineData delData;

	DocLineReplaceArg arg;
	arg.sDelRange = CLogicRange(CLogicPoint(3, 0), CLogicPoint(4, 0));
	arg.pcmemDeleted = &delData;
	arg.pInsData = nullptr;
	arg.nDelSeq = 1;
	arg.nInsSeq = -1;
	CSearchAgent(&m).ReplaceData(&arg, false);

	EXPECT_EQ(m.GetLineCount(), 1);
	EXPECT_STREQ(m.GetLine(CLogicInt(0))->GetPtr(), L"AAA");
	EXPECT_EQ(m.GetLine(CLogicInt(0))->m_sMark.m_cModified.GetSeq(), 1);

	EXPECT_EQ(arg.nInsSeq, 0);
	EXPECT_EQ(arg.pcmemDeleted->size(), 1);
	EXPECT_STREQ(arg.pcmemDeleted->at(0).cmemLine.GetStringPtr(), L"\n");
	EXPECT_EQ(arg.pcmemDeleted->at(0).nSeq, 1);
	EXPECT_EQ(arg.nDeletedLineNum, 0);
	EXPECT_EQ(arg.nInsLineNum, 0);
	EXPECT_EQ(arg.ptNewPos, CLogicPoint(3, 0));
}

/*!
	CSearchAgent::ReplaceData のテスト

	文字を挿入して複数行を連結する。
 */
TEST(CSearchAgent, ReplaceData6)
{
	CDocLineMgr m;
	SetLines(m, 1, {L"AAA\n", L"BBB\n", L"CCC\n"});

	COpeLineData insData = MakeOpeLineData({{L" ", 2}});
	COpeLineData delData;

	DocLineReplaceArg arg;
	arg.sDelRange = CLogicRange(CLogicPoint(2, 1), CLogicPoint(1, 2));
	arg.pcmemDeleted = &delData;
	arg.pInsData = &insData;
	arg.nDelSeq = 1;
	arg.nInsSeq = -1;
	CSearchAgent(&m).ReplaceData(&arg, false);

	EXPECT_EQ(m.GetLineCount(), 2);
	EXPECT_STREQ(m.GetLine(CLogicInt(0))->GetPtr(), L"AAA\n");
	EXPECT_EQ(m.GetLine(CLogicInt(0))->m_sMark.m_cModified.GetSeq(), 1);
	EXPECT_STREQ(m.GetLine(CLogicInt(1))->GetPtr(), L"BB CC\n");
	EXPECT_EQ(m.GetLine(CLogicInt(1))->m_sMark.m_cModified.GetSeq(), 2);

	EXPECT_EQ(arg.nInsSeq, 1);
	EXPECT_EQ(arg.pcmemDeleted->size(), 2);
	EXPECT_STREQ(arg.pcmemDeleted->at(0).cmemLine.GetStringPtr(), L"B\n");
	EXPECT_EQ(arg.pcmemDeleted->at(0).nSeq, 1);
	EXPECT_STREQ(arg.pcmemDeleted->at(1).cmemLine.GetStringPtr(), L"C");
	EXPECT_EQ(arg.pcmemDeleted->at(1).nSeq, 1);
	EXPECT_EQ(arg.nDeletedLineNum, 1);
	EXPECT_EQ(arg.nInsLineNum, 0);
	EXPECT_EQ(arg.ptNewPos, CLogicPoint(3, 1));
}

/*!
	CSearchAgent::ReplaceData のテスト

	既存行の間に行を挿入し、次の行の先頭に文字を挿入する。
	先頭に文字を挿入した行がデータの末尾であるケース。
 */
TEST(CSearchAgent, ReplaceData7)
{
	CDocLineMgr m;
	SetLines(m, 1, {L"AAA\n", L"BBB\n"});

	COpeLineData insData = MakeOpeLineData({{L"CCC\n", 2}, {L"DDD", 3}});
	COpeLineData delData;

	DocLineReplaceArg arg;
	arg.sDelRange = CLogicRange(CLogicPoint(0, 1), CLogicPoint(0, 1));
	arg.pcmemDeleted = &delData;
	arg.pInsData = &insData;
	arg.nDelSeq = 0;
	arg.nInsSeq = -1;
	CSearchAgent(&m).ReplaceData(&arg, false);

	EXPECT_EQ(m.GetLineCount(), 3);
	EXPECT_STREQ(m.GetLine(CLogicInt(0))->GetPtr(), L"AAA\n");
	EXPECT_EQ(m.GetLine(CLogicInt(0))->m_sMark.m_cModified.GetSeq(), 1);
	EXPECT_STREQ(m.GetLine(CLogicInt(1))->GetPtr(), L"CCC\n");
	EXPECT_EQ(m.GetLine(CLogicInt(1))->m_sMark.m_cModified.GetSeq(), 2);
	EXPECT_STREQ(m.GetLine(CLogicInt(2))->GetPtr(), L"DDDBBB\n");
	EXPECT_EQ(m.GetLine(CLogicInt(2))->m_sMark.m_cModified.GetSeq(), 3);

	EXPECT_EQ(arg.nInsSeq, 1);
	EXPECT_TRUE(arg.pcmemDeleted->empty());
	EXPECT_EQ(arg.nDeletedLineNum, 0);
	EXPECT_EQ(arg.nInsLineNum, 1);
	EXPECT_EQ(arg.ptNewPos, CLogicPoint(3, 2));
}

/*!
	CSearchAgent::ReplaceData のテスト

	既存行の間に行を挿入し、次の行の先頭に文字を挿入する。
	先頭に文字を挿入した行がデータの末尾ではないケース。
 */
TEST(CSearchAgent, ReplaceData8)
{
	CDocLineMgr m;
	SetLines(m, 1, {L"AAA\n", L"BBB\n", L"CCC\n"});

	COpeLineData insData = MakeOpeLineData({{L"DDD\n", 2}, {L"EEE", 3}});
	COpeLineData delData;

	DocLineReplaceArg arg;
	arg.sDelRange = CLogicRange(CLogicPoint(0, 1), CLogicPoint(0, 1));
	arg.pcmemDeleted = &delData;
	arg.pInsData = &insData;
	arg.nDelSeq = 0;
	arg.nInsSeq = -1;
	CSearchAgent(&m).ReplaceData(&arg, false);

	EXPECT_EQ(m.GetLineCount(), 4);
	EXPECT_STREQ(m.GetLine(CLogicInt(0))->GetPtr(), L"AAA\n");
	EXPECT_EQ(m.GetLine(CLogicInt(0))->m_sMark.m_cModified.GetSeq(), 1);
	EXPECT_STREQ(m.GetLine(CLogicInt(1))->GetPtr(), L"DDD\n");
	EXPECT_EQ(m.GetLine(CLogicInt(1))->m_sMark.m_cModified.GetSeq(), 2);
	EXPECT_STREQ(m.GetLine(CLogicInt(2))->GetPtr(), L"EEEBBB\n");
	EXPECT_EQ(m.GetLine(CLogicInt(2))->m_sMark.m_cModified.GetSeq(), 3);
	EXPECT_STREQ(m.GetLine(CLogicInt(3))->GetPtr(), L"CCC\n");
	EXPECT_EQ(m.GetLine(CLogicInt(3))->m_sMark.m_cModified.GetSeq(), 1);

	EXPECT_EQ(arg.nInsSeq, 1);
	EXPECT_TRUE(arg.pcmemDeleted->empty());
	EXPECT_EQ(arg.nDeletedLineNum, 0);
	EXPECT_EQ(arg.nInsLineNum, 1);
	EXPECT_EQ(arg.ptNewPos, CLogicPoint(3, 2));
}

/*!
	CSearchAgent::ReplaceData のテスト

	既存行の末尾に新しい行を追加する。
 */
TEST(CSearchAgent, ReplaceData9)
{
	CDocLineMgr m;
	SetLines(m, 1, {L"AAA\n", L"BBB\n"});

	COpeLineData insData = MakeOpeLineData({{L"CCC\n", 2}});
	COpeLineData delData;

	DocLineReplaceArg arg;
	arg.sDelRange = CLogicRange(CLogicPoint(0, 2), CLogicPoint(0, 2));
	arg.pcmemDeleted = &delData;
	arg.pInsData = &insData;
	arg.nDelSeq = 0;
	arg.nInsSeq = -1;
	CSearchAgent(&m).ReplaceData(&arg, false);

	EXPECT_EQ(m.GetLineCount(), 3);
	EXPECT_STREQ(m.GetLine(CLogicInt(0))->GetPtr(), L"AAA\n");
	EXPECT_EQ(m.GetLine(CLogicInt(0))->m_sMark.m_cModified.GetSeq(), 1);
	EXPECT_STREQ(m.GetLine(CLogicInt(1))->GetPtr(), L"BBB\n");
	EXPECT_EQ(m.GetLine(CLogicInt(1))->m_sMark.m_cModified.GetSeq(), 1);
	EXPECT_STREQ(m.GetLine(CLogicInt(2))->GetPtr(), L"CCC\n");
	EXPECT_EQ(m.GetLine(CLogicInt(2))->m_sMark.m_cModified.GetSeq(), 2);

	EXPECT_EQ(arg.nInsSeq, 0);
	EXPECT_TRUE(arg.pcmemDeleted->empty());
	EXPECT_EQ(arg.nDeletedLineNum, 0);
	EXPECT_EQ(arg.nInsLineNum, 1);
	EXPECT_EQ(arg.ptNewPos, CLogicPoint(0, 3));
}

/*!
	CSearchAgent::ReplaceData のテスト

	置換後の文字列が既存の行バッファの有効長に収まる場合の最適化済みコードパスの検査。
 */
TEST(CSearchAgent, ReplaceData10)
{
	CDocLineMgr m;
	SetLines(m, 1, {L"0123456789\n"});
	m.GetLine(CLogicInt(0))->_GetDocLineData().AllocStringBuffer(15);

	COpeLineData insData = MakeOpeLineData({{L"0123", 2}});
	COpeLineData delData;

	DocLineReplaceArg arg;
	arg.sDelRange = CLogicRange(CLogicPoint(9, 0), CLogicPoint(10, 0));
	arg.pcmemDeleted = &delData;
	arg.pInsData = &insData;
	arg.nDelSeq = 0;
	arg.nInsSeq = -1;
	CSearchAgent(&m).ReplaceData(&arg, false);

	EXPECT_EQ(m.GetLineCount(), 1);
	EXPECT_STREQ(m.GetLine(CLogicInt(0))->GetPtr(), L"0123456780123\n");
	EXPECT_EQ(m.GetLine(CLogicInt(0))->m_sMark.m_cModified.GetSeq(), 2);

	EXPECT_EQ(arg.nInsSeq, 1);
	EXPECT_EQ(arg.pcmemDeleted->size(), 1);
	EXPECT_STREQ(arg.pcmemDeleted->at(0).cmemLine.GetStringPtr(), L"9");
	EXPECT_EQ(arg.pcmemDeleted->at(0).nSeq, 1);
	EXPECT_EQ(arg.nDeletedLineNum, 0);
	EXPECT_EQ(arg.nInsLineNum, 0);
	EXPECT_EQ(arg.ptNewPos, CLogicPoint(13, 0));
}
