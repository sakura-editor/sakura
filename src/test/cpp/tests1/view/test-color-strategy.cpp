/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "window/EditorTestSuite.hpp"

#include "view/colors/CColor_Comment.h"
#include "view/colors/CColor_Heredoc.h"
#include "view/colors/CColor_Numeric.h"
#include "view/colors/CColor_Quote.h"
#include "view/colors/CColor_Url.h"

namespace view::color {

using namespace std::literals::string_view_literals;

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
		m_heredocDisp = type.m_ColorInfoArr[COLORIDX_HEREDOC].m_bDisp;
		m_singleQuoteDisp = type.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp;
		m_doubleQuoteDisp = type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp;
		m_stringType = type.m_nStringType;
		m_stringLineOnly = type.m_bStringLineOnly;
		m_stringEndLine = type.m_bStringEndLine;
		m_heredocType = type.m_nHeredocType;
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
		type.m_ColorInfoArr[COLORIDX_HEREDOC].m_bDisp = m_heredocDisp;
		type.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = m_singleQuoteDisp;
		type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = m_doubleQuoteDisp;
		type.m_nStringType = m_stringType;
		type.m_bStringLineOnly = m_stringLineOnly;
		type.m_bStringEndLine = m_stringEndLine;
		type.m_nHeredocType = m_heredocType;
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
	bool m_heredocDisp = false;
	bool m_singleQuoteDisp = false;
	bool m_doubleQuoteDisp = false;
	int m_stringType = STRING_LITERAL_CPP;
	bool m_stringLineOnly = false;
	bool m_stringEndLine = false;
	int m_heredocType = HEREDOC_PHP;
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

TEST_F(ColorStrategyTest, QuoteProperties)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_CPP;

	CColor_SingleQuote singleQuote;
	CColor_DoubleQuote doubleQuote;
	singleQuote.Update();
	doubleQuote.Update();

	EXPECT_THAT(singleQuote.GetStrategyColor(), Eq(COLORIDX_SSTRING));
	EXPECT_THAT(doubleQuote.GetStrategyColor(), Eq(COLORIDX_WSTRING));

	type.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = true;
	type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;
	EXPECT_THAT(singleQuote.Disp(), IsTrue());
	EXPECT_THAT(doubleQuote.Disp(), IsFalse());

	type.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;
	type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = true;
	EXPECT_THAT(singleQuote.Disp(), IsFalse());
	EXPECT_THAT(doubleQuote.Disp(), IsTrue());
}

TEST_F(ColorStrategyTest, QuoteRecognizesBasicStrings)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_CPP;
	type.m_bStringLineOnly = false;

	CColor_SingleQuote singleQuote;
	CColor_DoubleQuote doubleQuote;
	singleQuote.Update();
	doubleQuote.Update();

	const CStringRef singleLine{ L"'x' rest", 8 };
	EXPECT_THAT(singleQuote.BeginColor(singleLine, 0), IsTrue());
	EXPECT_THAT(singleQuote.EndColor(singleLine, 2), IsFalse());
	EXPECT_THAT(singleQuote.EndColor(singleLine, 3), IsTrue());

	const CStringRef doubleLine{ L"\"x\" rest", 8 };
	EXPECT_THAT(doubleQuote.BeginColor(doubleLine, 0), IsTrue());
	EXPECT_THAT(doubleQuote.EndColor(doubleLine, 2), IsFalse());
	EXPECT_THAT(doubleQuote.EndColor(doubleLine, 3), IsTrue());

	EXPECT_THAT(singleQuote.BeginColor(doubleLine, 0), IsFalse());
	EXPECT_THAT(doubleQuote.BeginColor(singleLine, 0), IsFalse());
	EXPECT_THAT(singleQuote.BeginColor(CStringRef(), 0), IsFalse());
}

TEST_F(ColorStrategyTest, QuoteIdentifiesCppRawStringPrefixes)
{
	struct RawStringTestCase {
		std::wstring_view text;
		int quotePos;
		bool expected;
	};
	const std::array testCases = {
		RawStringTestCase{ L"R\"(",       1, true },
		RawStringTestCase{ L"u8R\"tag(",  3, true },
		RawStringTestCase{ L"uR\"tag(",   2, true },
		RawStringTestCase{ L"UR\"tag(",   2, true },
		RawStringTestCase{ L"LR\"tag(",   2, true },
		RawStringTestCase{ L"xR\"tag(",   2, false },
		RawStringTestCase{ L"R\"",        1, false },
		RawStringTestCase{ L"R\"(",       0, false },
	};

	for (const auto& testCase : testCases) {
		SCOPED_TRACE(testing::Message() << "quotePos=" << testCase.quotePos);
		const CStringRef line(testCase.text.data(), static_cast<int>(testCase.text.size()));

		EXPECT_THAT(CColor_Quote::IsCppRawString(line, testCase.quotePos), Eq(testCase.expected));
	}
}

TEST_F(ColorStrategyTest, QuoteHelpersHonorEscapeRules)
{
	const CStringRef cppLine{ L"\"a\\\"b\" rest", 11 };
	EXPECT_THAT(CColor_Quote::Match_Quote(L'"', 1, cppLine, STRING_LITERAL_CPP), Eq(6));
	EXPECT_THAT(CColor_Quote::Match_Quote(L'"', 1, cppLine, STRING_LITERAL_HTML), Eq(4));

	const CStringRef plsqlLine{ L"'a''b' rest", 11 };
	EXPECT_THAT(CColor_Quote::Match_Quote(L'\'', 1, plsqlLine, STRING_LITERAL_PLSQL), Eq(6));

	const CStringRef rawLine{ L"body)tag\" rest", 14 };
	EXPECT_THAT(CColor_Quote::Match_QuoteStr(L")tag\"", 5, 0, rawLine, false), Eq(9));
}

TEST_F(ColorStrategyTest, QuoteUsesCppBackslashEscapes)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_CPP;
	type.m_bStringLineOnly = false;

	CColor_DoubleQuote strategy;
	strategy.Update();
	const CStringRef line{ L"\"a\\\"b\" rest", 11 };

	EXPECT_THAT(strategy.BeginColor(line, 0), IsTrue());
	EXPECT_THAT(strategy.EndColor(line, 4), IsFalse());
	EXPECT_THAT(strategy.EndColor(line, 5), IsFalse());
	EXPECT_THAT(strategy.EndColor(line, 6), IsTrue());
}

TEST_F(ColorStrategyTest, QuoteUsesPlsqlDoubledQuotes)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_PLSQL;
	type.m_bStringLineOnly = false;

	CColor_SingleQuote strategy;
	strategy.Update();
	const CStringRef line{ L"'a''b' rest", 11 };

	EXPECT_THAT(strategy.BeginColor(line, 0), IsTrue());
	EXPECT_THAT(strategy.EndColor(line, 3), IsFalse());
	EXPECT_THAT(strategy.EndColor(line, 5), IsFalse());
	EXPECT_THAT(strategy.EndColor(line, 6), IsTrue());
}

TEST_F(ColorStrategyTest, QuoteInHtmlRequiresAttributeAssignment)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_HTML;
	type.m_bStringLineOnly = true;
	type.m_bStringEndLine = false;

	CColor_DoubleQuote strategy;
	strategy.Update();

	const CStringRef attribute{ L"name = \"value\"", 14 };
	EXPECT_THAT(strategy.BeginColor(attribute, 7), IsTrue());
	EXPECT_THAT(strategy.EndColor(attribute, 13), IsFalse());
	EXPECT_THAT(strategy.EndColor(attribute, 14), IsTrue());

	const CStringRef text{ L"prefix \"value\"", 14 };
	EXPECT_THAT(strategy.BeginColor(text, 7), IsFalse());
}

TEST_F(ColorStrategyTest, QuoteRecognizesCSharpVerbatimStrings)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_CSHARP;

	CColor_DoubleQuote strategy;
	strategy.Update();
	const CStringRef line{ L"@\"a\"\"b\" rest", 12 };

	EXPECT_THAT(strategy.BeginColor(line, 1), IsTrue());
	std::unique_ptr<CLayoutColorInfo> colorInfo(strategy.GetStrategyColorInfo());
	EXPECT_THAT(colorInfo.get(), NotNull());
	EXPECT_THAT(strategy.EndColor(line, 4), IsFalse());
	EXPECT_THAT(strategy.EndColor(line, 7), IsTrue());
}

TEST_F(ColorStrategyTest, QuoteRecognizesPythonTripleQuotes)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_PYTHON;

	CColor_DoubleQuote strategy;
	strategy.Update();
	const CStringRef line{ L"\"\"\"body\"\"\" rest", 15 };

	EXPECT_THAT(strategy.BeginColor(line, 0), IsTrue());
	std::unique_ptr<CLayoutColorInfo> colorInfo(strategy.GetStrategyColorInfo());
	EXPECT_THAT(colorInfo.get(), NotNull());
	EXPECT_THAT(strategy.EndColor(line, 9), IsFalse());
	EXPECT_THAT(strategy.EndColor(line, 10), IsTrue());
}

TEST_F(ColorStrategyTest, QuoteRecognizesCppRawStrings)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_CPP;

	CColor_DoubleQuote strategy;
	strategy.Update();
	const CStringRef line{ L"R\"tag(body)tag\" rest", 20 };

	EXPECT_THAT(strategy.BeginColor(line, 1), IsTrue());
	std::unique_ptr<CLayoutColorInfo> colorInfo(strategy.GetStrategyColorInfo());
	EXPECT_THAT(colorInfo.get(), NotNull());
	EXPECT_THAT(strategy.EndColor(line, 14), IsFalse());
	EXPECT_THAT(strategy.EndColor(line, 15), IsTrue());
}

TEST_F(ColorStrategyTest, QuoteHonorsLineOnlySettings)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_CPP;
	type.m_bStringLineOnly = true;

	CColor_DoubleQuote strategy;
	type.m_bStringEndLine = false;
	strategy.Update();
	const CStringRef unterminated{ L"\"open", 5 };
	EXPECT_THAT(strategy.BeginColor(unterminated, 0), IsFalse());

	type.m_bStringEndLine = true;
	strategy.Update();
	const CStringRef withCrLf{ L"\"open\r\n", 7 };
	EXPECT_THAT(strategy.BeginColor(withCrLf, 0), IsTrue());
	EXPECT_THAT(strategy.EndColor(withCrLf, 4), IsFalse());
	EXPECT_THAT(strategy.EndColor(withCrLf, 5), IsTrue());
}

TEST_F(ColorStrategyTest, QuoteCarriesRawStringStateAcrossLines)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_CPP;

	CColor_DoubleQuote firstLineStrategy;
	firstLineStrategy.Update();
	const CStringRef firstLine{ L"R\"tag(body", 10 };
	EXPECT_THAT(firstLineStrategy.BeginColor(firstLine, 1), IsTrue());
	std::unique_ptr<CLayoutColorInfo> colorInfo(firstLineStrategy.GetStrategyColorInfo());
	ASSERT_THAT(colorInfo.get(), NotNull());

	CColor_DoubleQuote nextLineStrategy;
	nextLineStrategy.Update();
	nextLineStrategy.SetStrategyColorInfo(colorInfo.get());
	nextLineStrategy.InitStrategyStatus();
	const CStringRef nextLine{ L"tail)tag\" rest", 14 };
	EXPECT_THAT(nextLineStrategy.EndColor(nextLine, 0), IsFalse());
	EXPECT_THAT(nextLineStrategy.EndColor(nextLine, 8), IsFalse());
	EXPECT_THAT(nextLineStrategy.EndColor(nextLine, 9), IsTrue());
}

TEST_F(ColorStrategyTest, QuoteCarriesPythonTripleQuoteStateAcrossLines)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_PYTHON;

	CColor_SingleQuote firstLineStrategy;
	firstLineStrategy.Update();
	const CStringRef firstLine{ L"'''body", 7 };
	EXPECT_THAT(firstLineStrategy.BeginColor(firstLine, 0), IsTrue());
	std::unique_ptr<CLayoutColorInfo> colorInfo(firstLineStrategy.GetStrategyColorInfo());
	ASSERT_THAT(colorInfo.get(), NotNull());

	CColor_SingleQuote nextLineStrategy;
	nextLineStrategy.Update();
	nextLineStrategy.SetStrategyColorInfo(colorInfo.get());
	nextLineStrategy.InitStrategyStatus();
	const CStringRef nextLine{ L"tail''' rest", 12 };
	EXPECT_THAT(nextLineStrategy.EndColor(nextLine, 0), IsFalse());
	EXPECT_THAT(nextLineStrategy.EndColor(nextLine, 6), IsFalse());
	EXPECT_THAT(nextLineStrategy.EndColor(nextLine, 7), IsTrue());
}

TEST_F(ColorStrategyTest, QuoteContinuesCppStringAfterEscapedLineEnd)
{
	auto& type = GetTypeConfig();
	type.m_nStringType = STRING_LITERAL_CPP;
	type.m_bStringLineOnly = true;
	type.m_bStringEndLine = false;

	CColor_DoubleQuote strategy;
	strategy.Update();
	const CStringRef firstLine{ L"\"open\\\n", 7 };
	EXPECT_THAT(strategy.BeginColor(firstLine, 0), IsTrue());
	std::unique_ptr<CLayoutColorInfo> colorInfo(strategy.GetStrategyColorInfo());
	EXPECT_THAT(colorInfo.get(), IsNull());

	strategy.InitStrategyStatus();
	const CStringRef nextLine{ L"tail\" rest", 10 };
	EXPECT_THAT(strategy.EndColor(nextLine, 0), IsFalse());
	EXPECT_THAT(strategy.EndColor(nextLine, 4), IsFalse());
	EXPECT_THAT(strategy.EndColor(nextLine, 5), IsTrue());
}

TEST_F(ColorStrategyTest, HeredocProperties)
{
	CColor_Heredoc strategy;
	strategy.Update();

	EXPECT_THAT(strategy.GetStrategyColor(), Eq(COLORIDX_HEREDOC));

	GetTypeConfig().m_ColorInfoArr[COLORIDX_HEREDOC].m_bDisp = true;
	EXPECT_THAT(strategy.Disp(), IsTrue());

	GetTypeConfig().m_ColorInfoArr[COLORIDX_HEREDOC].m_bDisp = false;
	EXPECT_THAT(strategy.Disp(), IsFalse());
}

TEST_F(ColorStrategyTest, HeredocRecognizesPhpDeclarations)
{
	GetTypeConfig().m_nHeredocType = HEREDOC_PHP;
	const std::array declarations = {
		L"<<<ID\n"sv,
		L"<<<  ID\r\n"sv,
		L"<<<'ID'\n"sv,
		L"<<<\t\"ID_2\"\n"sv,
	};

	for (const auto declaration : declarations) {
		SCOPED_TRACE(testing::Message() << "declaration length=" << declaration.size());
		CColor_Heredoc strategy;
		strategy.Update();
		const CStringRef line(declaration.data(), static_cast<int>(declaration.size()));

		EXPECT_THAT(strategy.BeginColor(line, 0), IsTrue());
		EXPECT_THAT(strategy.EndColor(line, static_cast<int>(declaration.size()) - 1), IsFalse());
		EXPECT_THAT(strategy.EndColor(line, static_cast<int>(declaration.size())), IsTrue());
	}
}

TEST_F(ColorStrategyTest, HeredocRejectsInvalidDeclarations)
{
	GetTypeConfig().m_nHeredocType = HEREDOC_PHP;
	const std::array invalidDeclarations = {
		L"<<<\n"sv,
		L"<<<'ID\n"sv,
		L"<<<ID suffix\n"sv,
		L"<<<ID"sv,
	};

	CColor_Heredoc strategy;
	strategy.Update();
	EXPECT_THAT(strategy.BeginColor(CStringRef(), 0), IsFalse());
	for (const auto declaration : invalidDeclarations) {
		SCOPED_TRACE(testing::Message() << "declaration length=" << declaration.size());
		const CStringRef line(declaration.data(), static_cast<int>(declaration.size()));
		EXPECT_THAT(strategy.BeginColor(line, 0), IsFalse());
	}

	GetTypeConfig().m_nHeredocType = HEREDOC_PERL;
	const CStringRef unsupported{ L"<<<ID\n", 6 };
	EXPECT_THAT(strategy.BeginColor(unsupported, 0), IsFalse());
}

TEST_F(ColorStrategyTest, HeredocCarriesStateAcrossLinesAndRecognizesTerminators)
{
	GetTypeConfig().m_nHeredocType = HEREDOC_PHP;
	CColor_Heredoc declarationStrategy;
	declarationStrategy.Update();
	const CStringRef declaration{ L"<<<ID\n", 6 };
	ASSERT_THAT(declarationStrategy.BeginColor(declaration, 0), IsTrue());
	std::unique_ptr<CLayoutColorInfo> colorInfo(declarationStrategy.GetStrategyColorInfo());
	ASSERT_THAT(colorInfo.get(), NotNull());

	const auto expectEndPosition = [&](std::wstring_view text, int end) {
		CColor_Heredoc strategy;
		strategy.Update();
		strategy.SetStrategyColorInfo(colorInfo.get());
		strategy.InitStrategyStatus();
		const CStringRef line(text.data(), static_cast<int>(text.size()));

		EXPECT_THAT(strategy.EndColor(line, 0), IsFalse());
		EXPECT_THAT(strategy.EndColor(line, end - 1), IsFalse());
		EXPECT_THAT(strategy.EndColor(line, end), IsTrue());
	};

	expectEndPosition(L"body\n", 5);
	expectEndPosition(L"ID\n", 2);
	expectEndPosition(L"ID;\n", 2);
	expectEndPosition(L"IDsuffix\n", 9);
}

} // namespace view::color
