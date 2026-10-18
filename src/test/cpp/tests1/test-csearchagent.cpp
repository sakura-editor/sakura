/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "agent/CSaveAgent.h"

#include "cmd/COpeBlk.h"

#include <array>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "doc/logic/CDocLineMgr.h"
#include "agent/CSearchAgent.h"
#include "docplus/CBookmarkManager.h"
#include "env/ShareDataTestSuite.hpp"
#include "extmodule/CBregexp.h"

using ::testing::ElementsAre;	// pch.h に using が無いのでここで宣言する

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

TEST(CDeleteOpe, Dump001)
{
	CDeleteOpe ope;

	// 呼ぶだけ
	ope.DUMP();
}

TEST(CInsertOpe, Dump001)
{
	CInsertOpe ope;

	// 呼ぶだけ
	ope.DUMP();
}

TEST(COpeBlk, AppendOpe001)
{
	COpeBlk blk;

	auto pOpe = new CReplaceOpe();
	pOpe->m_ptCaretPos_PHY_Before = CLogicPoint(0, 1);
	pOpe->m_ptCaretPos_PHY_After = CLogicPoint(0, 1);
	blk.AppendOpe(pOpe);

	// 呼ぶだけ
	blk.DUMP();
}

TEST(COpeBlk, AppendOpe101)
{
	COpeBlk blk;
	auto pOpe = new CMoveCaretOpe();
	blk.AppendOpe(pOpe);
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

/*!
	@brief 文書の中の検索とブックマークの一括設定のテスト

	CSearchAgent::SearchWord() と CBookmarkManager::MarkSearchWord() の今の動きを固定する。
	単語単位は共有データの設定を読み、正規表現は bregonig.dll を読み込むので、共有データを用意する。
 */
struct CSearchAgentSearchTest : public ::testing::Test, public env::ShareDataTestSuite {
	static void SetUpTestSuite()
	{
		SetUpShareData();
	}

	static void TearDownTestSuite()
	{
		TearDownShareData();
	}

	CDocLineMgr m_cDocLineMgr;
	CBregexp m_cRegexp;
	SSearchOption m_sSearchOption;
	std::wstring m_strKey;
	CSearchStringPattern m_cPattern;

	//! 文書を作る。行の文字列には行末を含める
	void SetDocument(std::initializer_list<std::wstring_view> lines)
	{
		for (const auto& line : lines) {
			m_cDocLineMgr.AddNewLine()->SetDocLineString(line.data(), int(line.length()), false);
		}
	}

	//! 文字列の検索に使う文書
	void SetStringDocument()
	{
		SetDocument({ L"abc abcd abc\n", L"xyz\n", L"ABC\n", L"abc" });
	}

	//! 単語単位の検索に使う文書
	void SetWordDocument()
	{
		SetDocument({ L"foo foobar foo_bar foo\n", L"bar foo\n" });
	}

	//! ブックマークの一括設定に使う文書
	void SetBookmarkDocument()
	{
		SetDocument({ L"foo\n", L"foobar\n", L"bar foo\n", L"FOO" });
	}

	//! 検索の条件を設定する
	bool SetKey(std::wstring_view key, bool bRegularExp, bool bLoHiCase, bool bWordOnly)
	{
		m_strKey = key;
		m_sSearchOption = SSearchOption(bRegularExp, bLoHiCase, bWordOnly);
		return m_cPattern.SetPattern(nullptr, m_strKey.c_str(), m_strKey.length(), m_sSearchOption, &m_cRegexp);
	}

	//! (x, y) から検索する。見つかれば { FromX, FromY, ToX, ToY }、見つからなければ空
	std::vector<int> Search(int x, int y, ESearchDirection eDirection)
	{
		CLogicRange range;
		if (const int nRet = CSearchAgent(&m_cDocLineMgr).SearchWord(CLogicPoint(x, y), eDirection, &range, m_cPattern); nRet == 0) {
			return {};
		}
		return { range.GetFrom().x, range.GetFrom().y, range.GetTo().x, range.GetTo().y };
	}

	//! ブックマークの付いている行の番号
	std::vector<int> BookmarkedLines()
	{
		std::vector<int> lines;
		for (int i = 0; i < m_cDocLineMgr.GetLineCount(); ++i) {
			if (CBookmarkGetter(m_cDocLineMgr.GetLine(CLogicInt(i))).IsBookmarked()) {
				lines.push_back(i);
			}
		}
		return lines;
	}
};

// ---------------------------------------------------------------------------
// SearchWord: 文字列
// ---------------------------------------------------------------------------

//! 前方: カーソルの位置の一致を返す
TEST_F(CSearchAgentSearchTest, StringForward)
{
	SetStringDocument();
	ASSERT_THAT(SetKey(L"abc", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(0, 0, 3, 0));
}

//! 前方: 行の途中から探す。単語の一部にも一致する
TEST_F(CSearchAgentSearchTest, StringForwardFromMiddleOfLine)
{
	SetStringDocument();
	ASSERT_THAT(SetKey(L"abc", false, true, false), IsTrue());
	EXPECT_THAT(Search(1, 0, SEARCH_FORWARD), ElementsAre(4, 0, 7, 0));
}

//! 前方: 大文字小文字を区別すると、次の行の大文字は飛ばす
TEST_F(CSearchAgentSearchTest, StringForwardMatchCase)
{
	SetStringDocument();
	ASSERT_THAT(SetKey(L"abc", false, true, false), IsTrue());
	EXPECT_THAT(Search(10, 0, SEARCH_FORWARD), ElementsAre(0, 3, 3, 3));
}

//! 前方: 大文字小文字を区別しない
TEST_F(CSearchAgentSearchTest, StringForwardIgnoreCase)
{
	SetStringDocument();
	ASSERT_THAT(SetKey(L"abc", false, false, false), IsTrue());
	EXPECT_THAT(Search(10, 0, SEARCH_FORWARD), ElementsAre(0, 2, 3, 2));
}

//! 前方: 見つからない
TEST_F(CSearchAgentSearchTest, StringForwardNotFound)
{
	SetStringDocument();
	ASSERT_THAT(SetKey(L"zzz", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
}

//! 前方: 重なる一致も見つける
TEST_F(CSearchAgentSearchTest, StringForwardOverlap)
{
	SetDocument({ L"aaa" });
	ASSERT_THAT(SetKey(L"aa", false, true, false), IsTrue());
	EXPECT_THAT(Search(1, 0, SEARCH_FORWARD), ElementsAre(1, 0, 3, 0));
}

//! 後方: カーソルより前で最後の一致を返す
TEST_F(CSearchAgentSearchTest, StringBackward)
{
	SetStringDocument();
	ASSERT_THAT(SetKey(L"abc", false, true, false), IsTrue());
	EXPECT_THAT(Search(9, 0, SEARCH_BACKWARD), ElementsAre(4, 0, 7, 0));
}

//! 後方: カーソルの位置から始まる一致は含めない
TEST_F(CSearchAgentSearchTest, StringBackwardExcludesCursorPosition)
{
	SetStringDocument();
	ASSERT_THAT(SetKey(L"abc", false, true, false), IsTrue());
	EXPECT_THAT(Search(4, 0, SEARCH_BACKWARD), ElementsAre(0, 0, 3, 0));
}

//! 後方: 前の行へさかのぼる。大文字小文字を区別すると大文字の行は飛ばす
TEST_F(CSearchAgentSearchTest, StringBackwardPrevLinesMatchCase)
{
	SetStringDocument();
	ASSERT_THAT(SetKey(L"abc", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 3, SEARCH_BACKWARD), ElementsAre(9, 0, 12, 0));
}

//! 後方: 前の行へさかのぼる。大文字小文字を区別しない
TEST_F(CSearchAgentSearchTest, StringBackwardPrevLinesIgnoreCase)
{
	SetStringDocument();
	ASSERT_THAT(SetKey(L"abc", false, false, false), IsTrue());
	EXPECT_THAT(Search(0, 3, SEARCH_BACKWARD), ElementsAre(0, 2, 3, 2));
}

//! 後方: 文書の先頭より前には無い
TEST_F(CSearchAgentSearchTest, StringBackwardNotFoundAtTop)
{
	SetStringDocument();
	ASSERT_THAT(SetKey(L"abc", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_BACKWARD), IsEmpty());
}

/*!
	後方: 行の先頭から一致の長さずつ進めて探すので、重なる一致は飛ばす

	"aaa" でカーソルが 3 のとき、1 から始まる一致は見つけず、0 から始まる一致を返す。
 */
TEST_F(CSearchAgentSearchTest, StringBackwardSkipsOverlap)
{
	SetDocument({ L"aaa" });
	ASSERT_THAT(SetKey(L"aa", false, true, false), IsTrue());
	EXPECT_THAT(Search(3, 0, SEARCH_BACKWARD), ElementsAre(0, 0, 2, 0));
}

// ---------------------------------------------------------------------------
// SearchWord: 単語単位
// ---------------------------------------------------------------------------

//! 前方: 単語に一致する
TEST_F(CSearchAgentSearchTest, WordForward)
{
	SetWordDocument();
	ASSERT_THAT(SetKey(L"foo", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(0, 0, 3, 0));
}

//! 前方: 単語の一部(foobar・foo_bar)には一致しない
TEST_F(CSearchAgentSearchTest, WordForwardSkipsPartOfWord)
{
	SetWordDocument();
	ASSERT_THAT(SetKey(L"foo", false, true, true), IsTrue());
	EXPECT_THAT(Search(3, 0, SEARCH_FORWARD), ElementsAre(19, 0, 22, 0));
}

//! 前方: 単語の途中から探すと、その単語の先頭から一致する(カーソルより前から始まる範囲を返す)
TEST_F(CSearchAgentSearchTest, WordForwardFromMiddleOfWord)
{
	SetWordDocument();
	ASSERT_THAT(SetKey(L"foo", false, true, true), IsTrue());
	EXPECT_THAT(Search(20, 0, SEARCH_FORWARD), ElementsAre(19, 0, 22, 0));
}

//! 前方: 大文字小文字を区別しない
TEST_F(CSearchAgentSearchTest, WordForwardIgnoreCase)
{
	SetWordDocument();
	ASSERT_THAT(SetKey(L"FOO", false, false, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(0, 0, 3, 0));
}

//! 前方: 大文字小文字を区別する
TEST_F(CSearchAgentSearchTest, WordForwardMatchCase)
{
	SetWordDocument();
	ASSERT_THAT(SetKey(L"FOO", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
}

//! 前方: 検索語が 2 つの単語なら、どちらかの単語に一致する
TEST_F(CSearchAgentSearchTest, WordForwardAnyWordOfKey)
{
	SetWordDocument();
	ASSERT_THAT(SetKey(L"foo bar", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 1, SEARCH_FORWARD), ElementsAre(0, 1, 3, 1));
}

//! 前方: 単語の一部にしか無ければ見つからない
TEST_F(CSearchAgentSearchTest, WordForwardNotFound)
{
	SetDocument({ L"foobar foo_bar\n" });
	ASSERT_THAT(SetKey(L"foo", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
}

//! 後方: 単語を 1 つずつさかのぼり、単語の一部は飛ばす
TEST_F(CSearchAgentSearchTest, WordBackward)
{
	SetWordDocument();
	ASSERT_THAT(SetKey(L"foo", false, true, true), IsTrue());
	EXPECT_THAT(Search(19, 0, SEARCH_BACKWARD), ElementsAre(0, 0, 3, 0));
}

//! 後方: 単語の途中から探すと、その単語に一致する(カーソルより後ろまでの範囲を返す)
TEST_F(CSearchAgentSearchTest, WordBackwardFromMiddleOfWord)
{
	SetWordDocument();
	ASSERT_THAT(SetKey(L"foo", false, true, true), IsTrue());
	EXPECT_THAT(Search(2, 0, SEARCH_BACKWARD), ElementsAre(0, 0, 3, 0));
}

//! 後方: 前の行の行末からさかのぼる
TEST_F(CSearchAgentSearchTest, WordBackwardPrevLine)
{
	SetWordDocument();
	ASSERT_THAT(SetKey(L"foo", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 1, SEARCH_BACKWARD), ElementsAre(19, 0, 22, 0));
}

//! 後方: 文書の先頭より前には無い
TEST_F(CSearchAgentSearchTest, WordBackwardNotFoundAtTop)
{
	SetWordDocument();
	ASSERT_THAT(SetKey(L"foo", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_BACKWARD), IsEmpty());
}

// ---------------------------------------------------------------------------
// SearchWord: 正規表現
// ---------------------------------------------------------------------------

//! 前方: 一致の範囲を返す
TEST_F(CSearchAgentSearchTest, RegexForward)
{
	SetDocument({ L"abbbc abc\n" });
	ASSERT_THAT(SetKey(L"b+", true, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(1, 0, 4, 0));
}

//! 前方: 行の途中から探す
TEST_F(CSearchAgentSearchTest, RegexForwardFromMiddleOfLine)
{
	SetDocument({ L"abbbc abc\n" });
	ASSERT_THAT(SetKey(L"b+", true, true, false), IsTrue());
	EXPECT_THAT(Search(2, 0, SEARCH_FORWARD), ElementsAre(2, 0, 4, 0));
}

//! 前方: 一致は行をまたがない。次の行へ進む
TEST_F(CSearchAgentSearchTest, RegexForwardNextLine)
{
	SetDocument({ L"abc\n", L"xbz" });
	ASSERT_THAT(SetKey(L"b", true, true, false), IsTrue());
	EXPECT_THAT(Search(2, 0, SEARCH_FORWARD), ElementsAre(1, 1, 2, 1));
}

//! 前方: 大文字小文字を区別しない
TEST_F(CSearchAgentSearchTest, RegexForwardIgnoreCase)
{
	SetDocument({ L"abc" });
	ASSERT_THAT(SetKey(L"B", true, false, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(1, 0, 2, 0));
}

//! 前方: 大文字小文字を区別する
TEST_F(CSearchAgentSearchTest, RegexForwardMatchCase)
{
	SetDocument({ L"abc" });
	ASSERT_THAT(SetKey(L"B", true, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
}

//! 前方: 行頭(長さ 0)。行の途中から探すと、その行の行頭には一致せず、次の行の行頭に一致する
TEST_F(CSearchAgentSearchTest, RegexForwardLineHead)
{
	SetDocument({ L"abc\n", L"def" });
	ASSERT_THAT(SetKey(L"^", true, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(0, 0, 0, 0));
	EXPECT_THAT(Search(1, 0, SEARCH_FORWARD), ElementsAre(0, 1, 0, 1));
}

//! 前方: 行末(長さ 0)は、CRLF の CR の前に一致する
TEST_F(CSearchAgentSearchTest, RegexForwardLineEnd)
{
	SetDocument({ L"abc\r\n", L"def" });
	ASSERT_THAT(SetKey(L"$", true, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(3, 0, 3, 0));
}

//! 前方: 改行文字の中で一致したときは、一致の始まりを改行文字の始まりに直す(CRLF の LF に一致しても CR から)
TEST_F(CSearchAgentSearchTest, RegexForwardMatchInsideEol)
{
	SetDocument({ L"abc\r\n", L"x\n" });
	ASSERT_THAT(SetKey(L"\\n", true, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(3, 0, 5, 0));
}

//! 前方: 改行文字の中から探し始めると、その行は探さずに次の行へ進む
TEST_F(CSearchAgentSearchTest, RegexForwardStartInsideEol)
{
	SetDocument({ L"abc\r\n", L"x\n" });
	ASSERT_THAT(SetKey(L"\\n", true, true, false), IsTrue());
	EXPECT_THAT(Search(4, 0, SEARCH_FORWARD), ElementsAre(1, 1, 2, 1));
}

//! 後方: カーソルより前で最後の一致を返す。カーソルの位置から始まる一致は含めない
TEST_F(CSearchAgentSearchTest, RegexBackward)
{
	SetDocument({ L"abcabc" });
	ASSERT_THAT(SetKey(L"b", true, true, false), IsTrue());
	EXPECT_THAT(Search(5, 0, SEARCH_BACKWARD), ElementsAre(4, 0, 5, 0));
	EXPECT_THAT(Search(4, 0, SEARCH_BACKWARD), ElementsAre(1, 0, 2, 0));
}

//! 後方: 前の行へさかのぼる
TEST_F(CSearchAgentSearchTest, RegexBackwardPrevLine)
{
	SetDocument({ L"abc\n", L"xyz" });
	ASSERT_THAT(SetKey(L"b", true, true, false), IsTrue());
	EXPECT_THAT(Search(1, 1, SEARCH_BACKWARD), ElementsAre(1, 0, 2, 0));
}

//! 後方: 長さ 0 の一致の後は 1 文字進めて探すので、止まらずに行頭の一致を返す
TEST_F(CSearchAgentSearchTest, RegexBackwardZeroLength)
{
	SetDocument({ L"abc" });
	ASSERT_THAT(SetKey(L"^", true, true, false), IsTrue());
	EXPECT_THAT(Search(2, 0, SEARCH_BACKWARD), ElementsAre(0, 0, 0, 0));
}

//! 見つからない
TEST_F(CSearchAgentSearchTest, RegexNotFound)
{
	SetDocument({ L"abc\n", L"xyz" });
	ASSERT_THAT(SetKey(L"q", true, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
	EXPECT_THAT(Search(3, 1, SEARCH_BACKWARD), IsEmpty());
}

// ---------------------------------------------------------------------------
// MarkSearchWord
// ---------------------------------------------------------------------------

//! 文字列: 一致を含む行にブックマークを付ける
TEST_F(CSearchAgentSearchTest, MarkSearchWordString)
{
	SetBookmarkDocument();
	ASSERT_THAT(SetKey(L"foo", false, true, false), IsTrue());
	CBookmarkManager(&m_cDocLineMgr).MarkSearchWord(m_cPattern);
	EXPECT_THAT(BookmarkedLines(), ElementsAre(0, 1, 2));
}

//! 単語単位: 単語の一部(foobar)の行には付けない
TEST_F(CSearchAgentSearchTest, MarkSearchWordWord)
{
	SetBookmarkDocument();
	ASSERT_THAT(SetKey(L"foo", false, true, true), IsTrue());
	CBookmarkManager(&m_cDocLineMgr).MarkSearchWord(m_cPattern);
	EXPECT_THAT(BookmarkedLines(), ElementsAre(0, 2));
}

//! 単語単位: 大文字小文字を区別しない
TEST_F(CSearchAgentSearchTest, MarkSearchWordWordIgnoreCase)
{
	SetBookmarkDocument();
	ASSERT_THAT(SetKey(L"foo", false, false, true), IsTrue());
	CBookmarkManager(&m_cDocLineMgr).MarkSearchWord(m_cPattern);
	EXPECT_THAT(BookmarkedLines(), ElementsAre(0, 2, 3));
}

//! 正規表現: 行頭から照合する
TEST_F(CSearchAgentSearchTest, MarkSearchWordRegex)
{
	SetBookmarkDocument();
	ASSERT_THAT(SetKey(L"^foo", true, true, false), IsTrue());
	CBookmarkManager(&m_cDocLineMgr).MarkSearchWord(m_cPattern);
	EXPECT_THAT(BookmarkedLines(), ElementsAre(0, 1));
}

//! すでに付いているブックマークは、一致しなくても外さない
TEST_F(CSearchAgentSearchTest, MarkSearchWordKeepsBookmarks)
{
	SetBookmarkDocument();
	CBookmarkSetter(m_cDocLineMgr.GetLine(CLogicInt(3))).SetBookmark(true);
	ASSERT_THAT(SetKey(L"foo", false, true, false), IsTrue());
	CBookmarkManager(&m_cDocLineMgr).MarkSearchWord(m_cPattern);
	EXPECT_THAT(BookmarkedLines(), ElementsAre(0, 1, 2, 3));
}

// ---------------------------------------------------------------------------
// イレギュラーケース
// ---------------------------------------------------------------------------

//! 空の検索語は、文字列でも単語単位でも見つからず、ブックマークも付けない
TEST_F(CSearchAgentSearchTest, EmptyKey)
{
	SetDocument({ L"abc\n" });
	ASSERT_THAT(SetKey(L"", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
	EXPECT_THAT(Search(3, 0, SEARCH_BACKWARD), IsEmpty());
	ASSERT_THAT(SetKey(L"", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
	CBookmarkManager(&m_cDocLineMgr).MarkSearchWord(m_cPattern);
	EXPECT_THAT(BookmarkedLines(), IsEmpty());
}

//! 単語単位: 区切り(空白・タブ・全角空白)だけの検索語は単語を含まないので見つからない。前後の区切りは無視する
TEST_F(CSearchAgentSearchTest, WordDelimiterOnlyKey)
{
	SetDocument({ L"foo bar\n", L"foo" });
	ASSERT_THAT(SetKey(L" \t　", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
	EXPECT_THAT(Search(3, 1, SEARCH_BACKWARD), IsEmpty());
	ASSERT_THAT(SetKey(L"  foo  ", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(0, 0, 3, 0));
	EXPECT_THAT(Search(3, 1, SEARCH_BACKWARD), ElementsAre(0, 1, 3, 1));
}

//! 空の文書・範囲外の行から探すと、見つからない
TEST_F(CSearchAgentSearchTest, EmptyDocumentAndOutOfRangeLine)
{
	ASSERT_THAT(SetKey(L"a", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
	EXPECT_THAT(Search(0, 0, SEARCH_BACKWARD), IsEmpty());

	SetDocument({ L"abc\n" });
	EXPECT_THAT(Search(0, -1, SEARCH_FORWARD), IsEmpty());
	EXPECT_THAT(Search(0, 1, SEARCH_FORWARD), IsEmpty());
	EXPECT_THAT(Search(0, 5, SEARCH_BACKWARD), IsEmpty());
	ASSERT_THAT(SetKey(L"a", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 1, SEARCH_FORWARD), IsEmpty());
	EXPECT_THAT(Search(0, 5, SEARCH_BACKWARD), IsEmpty());
}

//! 文字列: 検索語に改行文字を含めると、同じ行の行末までなら一致する。行はまたがない
TEST_F(CSearchAgentSearchTest, StringKeyContainsEol)
{
	SetDocument({ L"abc\r\n", L"def" });
	ASSERT_THAT(SetKey(L"c\r\n", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(2, 0, 5, 0));
	ASSERT_THAT(SetKey(L"c\n", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
	ASSERT_THAT(SetKey(L"c\r\ndef", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
}

//! 行末が CR だけの行: 文字列は CR まで一致し、単語単位は前後の行へ進む
TEST_F(CSearchAgentSearchTest, CrOnlyLineEndings)
{
	SetDocument({ L"abc\r", L"def\r", L"ghi" });
	ASSERT_THAT(SetKey(L"c\r", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(2, 0, 4, 0));
	ASSERT_THAT(SetKey(L"def", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(0, 1, 3, 1));
	EXPECT_THAT(Search(3, 2, SEARCH_BACKWARD), ElementsAre(0, 1, 3, 1));
}

//! 文字列・後方: カーソルより前から始まり、カーソルをまたぐ一致を返す
TEST_F(CSearchAgentSearchTest, StringBackwardMatchStraddlesCursor)
{
	SetDocument({ L"abcabc" });
	ASSERT_THAT(SetKey(L"abc", false, true, false), IsTrue());
	EXPECT_THAT(Search(5, 0, SEARCH_BACKWARD), ElementsAre(3, 0, 6, 0));
}

//! 単語単位・後方: 最後の行に行末が無くても、行末から単語をさかのぼる
TEST_F(CSearchAgentSearchTest, WordBackwardLastLineWithoutEol)
{
	SetDocument({ L"foo bar" });
	ASSERT_THAT(SetKey(L"bar", false, true, true), IsTrue());
	EXPECT_THAT(Search(7, 0, SEARCH_BACKWARD), ElementsAre(4, 0, 7, 0));
	EXPECT_THAT(Search(6, 0, SEARCH_BACKWARD), ElementsAre(4, 0, 7, 0));
	EXPECT_THAT(Search(4, 0, SEARCH_BACKWARD), IsEmpty());
}

/*!
	単語単位・後方: カーソルが行末より右でも、行末とみなしてその行を探す

	今はその行を飛ばすので DISABLED_ にしている(#2743 の 1)。文字列の後方検索は同じ位置からその行を探す。
 */
TEST_F(CSearchAgentSearchTest, DISABLED_WordBackwardCursorPastTextSearchesLine)
{
	SetDocument({ L"abc\n", L"abc" });
	ASSERT_THAT(SetKey(L"abc", false, true, true), IsTrue());
	EXPECT_THAT(Search(4, 0, SEARCH_BACKWARD), ElementsAre(0, 0, 3, 0));
	EXPECT_THAT(Search(100, 1, SEARCH_BACKWARD), ElementsAre(0, 1, 3, 1));
}

//! 単語単位: 検索語の記号も 1 つの単語として扱う(a.b は a・.・b のどれかに一致する)
TEST_F(CSearchAgentSearchTest, WordKeyPunctuationIsAlsoAWord)
{
	SetDocument({ L"x.y\n" });
	ASSERT_THAT(SetKey(L"a.b", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(1, 0, 2, 0));
}

//! 単語単位: 全角空白は単語の区切り
TEST_F(CSearchAgentSearchTest, WordFullWidthSpaceIsDelimiter)
{
	SetDocument({ L"foobar foo　baz\n" });
	ASSERT_THAT(SetKey(L"foo　bar", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(7, 0, 10, 0));
}

//! 単語単位: ひらがなとカタカナの境目で単語が分かれる
TEST_F(CSearchAgentSearchTest, WordKanaBoundaries)
{
	SetDocument({ L"ひらがなカタカナ\n" });
	ASSERT_THAT(SetKey(L"カタカナ", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(4, 0, 8, 0));
	EXPECT_THAT(Search(8, 0, SEARCH_BACKWARD), ElementsAre(4, 0, 8, 0));
	ASSERT_THAT(SetKey(L"がな", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
}

/*!
	単語単位: 長音「ー」は前後どちらの単語にも付く

	「あーカ」は「あ」から見ると「あー」「カ」、「カ」から見ると「あ」「ーカ」に分かれる。今の動きを固定する。
 */
TEST_F(CSearchAgentSearchTest, WordProlongedSoundMarkJoinsBothSides)
{
	SetDocument({ L"あーカ\n" });
	ASSERT_THAT(SetKey(L"あー", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(0, 0, 2, 0));
	ASSERT_THAT(SetKey(L"ーカ", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(1, 0, 3, 0));
	ASSERT_THAT(SetKey(L"カ", false, true, true), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
	EXPECT_THAT(Search(3, 0, SEARCH_BACKWARD), IsEmpty());
}

/*!
	文字列: 検索語がサロゲートペアの片方だけ(UTF-16 のコード片)なら、文字の途中では一致しない

	今はペアの途中に一致するので DISABLED_ にしている(#2743 の 2)。
 */
TEST_F(CSearchAgentSearchTest, DISABLED_StringKeyDoesNotCutCharacter)
{
	SetDocument({ L"\U00020BB7\n" });	// 𠮷
	ASSERT_THAT(SetKey(L"\xDFB7", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
	ASSERT_THAT(SetKey(L"\xD842", false, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), IsEmpty());
}

//! 正規表現: 行末(長さ 0)は、LF・CR・行末なしのどの行でも改行文字の前に一致する
TEST_F(CSearchAgentSearchTest, RegexLineEndEachEol)
{
	ASSERT_THAT(SetKey(L"$", true, true, false), IsTrue());
	SetDocument({ L"abc\n", L"abc\r", L"abc" });
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(3, 0, 3, 0));
	EXPECT_THAT(Search(0, 1, SEARCH_FORWARD), ElementsAre(3, 1, 3, 1));
	EXPECT_THAT(Search(0, 2, SEARCH_FORWARD), ElementsAre(3, 2, 3, 2));
}

//! 正規表現・後方: 行末(長さ 0)はカーソルの位置から始まるので含めず、カーソルより後ろからなら一致する
TEST_F(CSearchAgentSearchTest, RegexBackwardLineEnd)
{
	SetDocument({ L"abc" });
	ASSERT_THAT(SetKey(L"$", true, true, false), IsTrue());
	EXPECT_THAT(Search(3, 0, SEARCH_BACKWARD), IsEmpty());
	EXPECT_THAT(Search(4, 0, SEARCH_BACKWARD), ElementsAre(3, 0, 3, 0));
}

//! 正規表現: 「.」は改行文字に一致しないので、改行だけの行を飛ばす
TEST_F(CSearchAgentSearchTest, RegexDotSkipsEol)
{
	SetDocument({ L"\r\n", L"ab" });
	ASSERT_THAT(SetKey(L".", true, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(0, 1, 1, 1));
}

//! 正規表現: 「\r」は CRLF の CR だけに一致する(改行文字の途中までの範囲を返す)
TEST_F(CSearchAgentSearchTest, RegexCrInsideCrlf)
{
	SetDocument({ L"abc\r\n" });
	ASSERT_THAT(SetKey(L"\\r", true, true, false), IsTrue());
	EXPECT_THAT(Search(0, 0, SEARCH_FORWARD), ElementsAre(3, 0, 4, 0));
}
