/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "window/EditorTestSuite.hpp"

#include "view/colors/CColor_Comment.h"
#include "view/colors/CColor_Numeric.h"
#include "view/colors/CColor_Url.h"

namespace view::color {

struct TestCase {
	const wchar_t* text;
	int length;
	int end;

	TestCase(
		std::wstring_view text_,
		int end
	)
		: text(text_.data())
		, length(static_cast<int>(text_.size()))
		, end(end)
	{
	}
};

struct ColorStrategyTest : public ::testing::Test, public window::EditorTestSuite {
	static void SetUpTestSuite()
	{
		SetUpEditor();
	}

	static void TearDownTestSuite()
	{
		TearDownEditor();
	}

	void SetUp() override
	{
		const auto& type = pcEditDoc->m_cDocType.GetDocumentAttribute();
		m_digitDisp = type.m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp;
		m_urlDisp = type.m_ColorInfoArr[COLORIDX_URL].m_bDisp;
		m_commentDisp = type.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp;
		m_lineComment = type.m_cLineComment;
		m_blockComments[0] = type.m_cBlockComments[0];
		m_blockComments[1] = type.m_cBlockComments[1];
	}

	void TearDown() override
	{
		auto& type = pcEditDoc->m_cDocType.GetDocumentAttributeWrite();
		type.m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = m_digitDisp;
		type.m_ColorInfoArr[COLORIDX_URL].m_bDisp = m_urlDisp;
		type.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp = m_commentDisp;
		type.m_cLineComment = m_lineComment;
		type.m_cBlockComments[0] = m_blockComments[0];
		type.m_cBlockComments[1] = m_blockComments[1];
	}

	STypeConfig& GetTypeConfig()
	{
		return pcEditDoc->m_cDocType.GetDocumentAttributeWrite();
	}

private:
	bool m_digitDisp = false;
	bool m_urlDisp = false;
	bool m_commentDisp = false;
	CLineComment m_lineComment;
	CBlockComment m_blockComments[2];
};

TEST_F(ColorStrategyTest, NumericProperties)
{
	CColor_Numeric strategy;
	strategy.Update();

	EXPECT_THAT(strategy.GetStrategyColor(), Eq(COLORIDX_DIGIT));

	GetTypeConfig().m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;
	EXPECT_THAT(strategy.Disp(), IsTrue());

	GetTypeConfig().m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = false;
	EXPECT_THAT(strategy.Disp(), IsFalse());
}

TEST_F(ColorStrategyTest, NumericRecognizesSupportedFormats)
{
	const std::array testCases = {
		TestCase{ L"123 rest",		3 },
		TestCase{ L"0xABcd rest",	6 },
		TestCase{ L"0.5 rest",		3 },
		TestCase{ L"1e+2 rest",		4 },
	};

	for (const auto& testCase : testCases) {
		SCOPED_TRACE(testing::Message() << "text=" << testCase.text);
		CColor_Numeric strategy;
		const CStringRef line(testCase.text, testCase.length);

		EXPECT_THAT(strategy.BeginColor(line, 0), IsTrue());
		EXPECT_THAT(strategy.EndColor(line, testCase.end - 1), IsFalse());
		EXPECT_THAT(strategy.EndColor(line, testCase.end), IsTrue());
	}
}

TEST_F(ColorStrategyTest, NumericRejectsInvalidStartsAndHandlesBoundaries)
{
	CColor_Numeric strategy;

	EXPECT_THAT(strategy.BeginColor(CStringRef(), 0), IsFalse());

	const CStringRef word{ L"abc123", 6 };
	EXPECT_THAT(strategy.BeginColor(word, 3), IsFalse());

	const CStringRef sign{ L"-123", 4 };
	EXPECT_THAT(strategy.BeginColor(sign, 0), IsTrue());
	EXPECT_THAT(strategy.EndColor(sign, 4), IsTrue());

	const CStringRef dotOnly{ L".", 1 };
	EXPECT_THAT(strategy.BeginColor(dotOnly, 0), IsFalse());

	const CStringRef leadingDot{ L".5", 2 };
	EXPECT_THAT(strategy.BeginColor(leadingDot, 0), IsTrue());
	EXPECT_THAT(strategy.EndColor(leadingDot, 2), IsTrue());

	const CStringRef hexPrefix{ L"0x", 2 };
	EXPECT_THAT(strategy.BeginColor(hexPrefix, 0), IsTrue());
	EXPECT_THAT(strategy.EndColor(hexPrefix, 1), IsTrue());

	const CStringRef trailingDot{ L"123.", 4 };
	EXPECT_THAT(strategy.BeginColor(trailingDot, 0), IsTrue());
	EXPECT_THAT(strategy.EndColor(trailingDot, 3), IsTrue());
}

TEST_F(ColorStrategyTest, UrlProperties)
{
	CColor_Url strategy;
	strategy.Update();

	EXPECT_THAT(strategy.GetStrategyColor(), Eq(COLORIDX_URL));

	GetTypeConfig().m_ColorInfoArr[COLORIDX_URL].m_bDisp = true;
	EXPECT_THAT(strategy.Disp(), IsTrue());

	GetTypeConfig().m_ColorInfoArr[COLORIDX_URL].m_bDisp = false;
	EXPECT_THAT(strategy.Disp(), IsFalse());
}

TEST_F(ColorStrategyTest, UrlRecognizesUrlAndMailAddress)
{
	const std::array testCases = {
		TestCase{ L"http://example.com/path rest",	23 },
		TestCase{ L"user@example.com rest",			16 },
	};

	for (const auto& testCase : testCases) {
		SCOPED_TRACE(testing::Message() << "text=" << testCase.text);
		CColor_Url strategy;
		const CStringRef line(testCase.text, testCase.length);

		EXPECT_THAT(strategy.BeginColor(line, 0), IsTrue());
		EXPECT_THAT(strategy.EndColor(line, testCase.end - 1), IsFalse());
		EXPECT_THAT(strategy.EndColor(line, testCase.end), IsTrue());
	}
}

TEST_F(ColorStrategyTest, UrlRejectsInvalidCandidates)
{
	CColor_Url strategy;

	EXPECT_THAT(strategy.BeginColor(CStringRef(), 0), IsFalse());

	const CStringRef headerOnly{ L"http://", 7 };
	EXPECT_THAT(strategy.BeginColor(headerOnly, 0), IsFalse());

	const CStringRef middleOfWord{ L"xhttp://example.com", 19 };
	EXPECT_THAT(strategy.BeginColor(middleOfWord, 1), IsFalse());
}

TEST_F(ColorStrategyTest, LineCommentProperties)
{
	auto& type = GetTypeConfig();
	for (int index = 0; index < COMMENT_DELIMITER_NUM; ++index) {
		type.m_cLineComment.CopyTo(index, L"", -1);
	}

	CColor_LineComment strategy;
	strategy.Update();
	EXPECT_THAT(strategy.GetStrategyColor(), Eq(COLORIDX_COMMENT));

	type.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp = true;
	EXPECT_THAT(strategy.Disp(), IsFalse());

	type.m_cLineComment.CopyTo(0, L"//", -1);
	EXPECT_THAT(strategy.Disp(), IsTrue());

	type.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp = false;
	EXPECT_THAT(strategy.Disp(), IsFalse());
}

TEST_F(ColorStrategyTest, LineCommentRecognizesConfiguredDelimiter)
{
	auto& type = GetTypeConfig();
	for (int index = 0; index < COMMENT_DELIMITER_NUM; ++index) {
		type.m_cLineComment.CopyTo(index, L"", -1);
	}
	type.m_cLineComment.CopyTo(0, L"//", -1);

	CColor_LineComment strategy;
	strategy.Update();
	const CStringRef line{ L"code // comment", 15 };

	EXPECT_THAT(strategy.BeginColor(line, 5), IsTrue());
	EXPECT_THAT(strategy.BeginColor(line, 4), IsFalse());
	EXPECT_THAT(strategy.BeginColor(CStringRef(), 0), IsFalse());
}

TEST_F(ColorStrategyTest, LineCommentHonorsConfiguredColumn)
{
	auto& type = GetTypeConfig();
	for (int index = 0; index < COMMENT_DELIMITER_NUM; ++index) {
		type.m_cLineComment.CopyTo(index, L"", -1);
	}
	type.m_cLineComment.CopyTo(0, L"REM", 2);

	CColor_LineComment strategy;
	strategy.Update();
	const CStringRef line{ L"  REM comment", 13 };

	EXPECT_THAT(strategy.BeginColor(line, 2), IsTrue());
	EXPECT_THAT(strategy.BeginColor(line, 3), IsFalse());
}

TEST_F(ColorStrategyTest, LineCommentEndsAtLineDelimiterOrLineEnd)
{
	CColor_LineComment strategy;
	const CStringRef line{ L"// comment\r\n", 12 };

	EXPECT_THAT(strategy.EndColor(line, 5), IsFalse());
	EXPECT_THAT(strategy.EndColor(line, 10), IsTrue());
	EXPECT_THAT(strategy.EndColor(line, 12), IsTrue());
}

TEST_F(ColorStrategyTest, BlockCommentProperties)
{
	auto& type = GetTypeConfig();
	type.m_cBlockComments[0].SetBlockCommentRule(L"", L"");
	type.m_cBlockComments[1].SetBlockCommentRule(L"", L"");
	type.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp = true;

	CColor_BlockComment block1(COLORIDX_BLOCK1);
	CColor_BlockComment block2(COLORIDX_BLOCK2);
	block1.Update();
	block2.Update();

	EXPECT_THAT(block1.GetStrategyColor(), Eq(COLORIDX_BLOCK1));
	EXPECT_THAT(block2.GetStrategyColor(), Eq(COLORIDX_BLOCK2));
	EXPECT_THAT(block1.Disp(), IsFalse());
	EXPECT_THAT(block2.Disp(), IsFalse());

	type.m_cBlockComments[0].SetBlockCommentRule(L"/*", L"*/");
	EXPECT_THAT(block1.Disp(), IsTrue());
	EXPECT_THAT(block2.Disp(), IsFalse());

	type.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp = false;
	EXPECT_THAT(block1.Disp(), IsFalse());
}

TEST_F(ColorStrategyTest, BlockCommentUsesCorrespondingDelimiter)
{
	auto& type = GetTypeConfig();
	type.m_cBlockComments[0].SetBlockCommentRule(L"/*", L"*/");
	type.m_cBlockComments[1].SetBlockCommentRule(L"<!--", L"-->");

	CColor_BlockComment block1(COLORIDX_BLOCK1);
	CColor_BlockComment block2(COLORIDX_BLOCK2);
	block1.Update();
	block2.Update();
	const CStringRef cStyle{ L"/*x*/", 5 };
	const CStringRef htmlStyle{ L"<!--x-->", 8 };

	EXPECT_THAT(block1.BeginColor(cStyle, 0), IsTrue());
	EXPECT_THAT(block2.BeginColor(cStyle, 0), IsFalse());
	EXPECT_THAT(block2.BeginColor(htmlStyle, 0), IsTrue());
	EXPECT_THAT(block1.BeginColor(CStringRef(), 0), IsFalse());
}

TEST_F(ColorStrategyTest, BlockCommentEndsAfterClosingDelimiter)
{
	auto& type = GetTypeConfig();
	type.m_cBlockComments[0].SetBlockCommentRule(L"/*", L"*/");

	CColor_BlockComment strategy(COLORIDX_BLOCK1);
	strategy.Update();
	const CStringRef line{ L"/*x*/", 5 };

	EXPECT_THAT(strategy.BeginColor(line, 0), IsTrue());
	EXPECT_THAT(strategy.EndColor(line, 4), IsFalse());
	EXPECT_THAT(strategy.EndColor(line, 5), IsTrue());
}

TEST_F(ColorStrategyTest, BlockCommentFindsEndOnContinuationLine)
{
	auto& type = GetTypeConfig();
	type.m_cBlockComments[0].SetBlockCommentRule(L"/*", L"*/");

	CColor_BlockComment strategy(COLORIDX_BLOCK1);
	strategy.Update();
	strategy.InitStrategyStatus();
	const CStringRef line{ L"continued */ rest", 17 };

	EXPECT_THAT(strategy.EndColor(line, 0), IsFalse());
	EXPECT_THAT(strategy.EndColor(line, 12), IsTrue());

	strategy.InitStrategyStatus();
	const CStringRef noDelimiter{ L"continued", 9 };
	EXPECT_THAT(strategy.EndColor(noDelimiter, 0), IsFalse());
	EXPECT_THAT(strategy.EndColor(noDelimiter, 9), IsTrue());
}

} // namespace view::color
