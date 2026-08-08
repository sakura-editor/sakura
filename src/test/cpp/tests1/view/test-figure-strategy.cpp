/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "window/EditorTestSuite.hpp"

#include "view/figures/CFigure_Comma.h"
#include "view/figures/CFigure_CtrlCode.h"
#include "view/figures/CFigure_Eol.h"
#include "view/figures/CFigure_HanSpace.h"
#include "view/figures/CFigure_Tab.h"
#include "view/figures/CFigure_ZenSpace.h"

namespace view::figure {

class FigureStrategyTest : public ::testing::Test, public window::EditorTestSuite {
public:
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
		for (size_t index = 0; index < m_colorIndexes.size(); ++index) {
			m_displaySettings[index] = type.m_ColorInfoArr[m_colorIndexes[index]].m_bDisp;
		}
		m_tsvMode = type.m_nTsvMode;
	}

	void TearDown() override
	{
		auto& type = GetTypeConfig();
		for (size_t index = 0; index < m_colorIndexes.size(); ++index) {
			type.m_ColorInfoArr[m_colorIndexes[index]].m_bDisp = m_displaySettings[index];
		}
		type.m_nTsvMode = m_tsvMode;
	}

protected:
	STypeConfig& GetTypeConfig()
	{
		return pcEditDoc->m_cDocType.GetDocumentAttributeWrite();
	}

private:
	static constexpr std::array m_colorIndexes = {
		COLORIDX_TAB,
		COLORIDX_SPACE,
		COLORIDX_ZENSPACE,
		COLORIDX_CTRLCODE,
	};
	std::array<bool, m_colorIndexes.size()> m_displaySettings{};
	int m_tsvMode = TSV_MODE_NONE;
};

TEST_F(FigureStrategyTest, TextAlwaysMatchesAndDisplays)
{
	CFigure_Text strategy;

	EXPECT_THAT(strategy.Match(L"text", 4), IsTrue());
	EXPECT_THAT(strategy.Match(L"", 0), IsTrue());
	EXPECT_THAT(strategy.Disp(), IsTrue());
}

TEST_F(FigureStrategyTest, TabMatchesOnlyTabAndAlwaysDisplays)
{
	CFigure_Tab strategy;

	EXPECT_THAT(strategy.Match(L"\t", 1), IsTrue());
	EXPECT_THAT(strategy.Match(L" ", 1), IsFalse());
	EXPECT_THAT(strategy.Disp(), IsTrue());
}

TEST_F(FigureStrategyTest, CommaMatchesCommaAndDisplaysOnlyInCsvMode)
{
	auto& type = GetTypeConfig();
	CFigure_Comma strategy;

	EXPECT_THAT(strategy.Match(L",", 1), IsTrue());
	EXPECT_THAT(strategy.Match(L";", 1), IsFalse());

	type.m_nTsvMode = TSV_MODE_NONE;
	strategy.Update();
	EXPECT_THAT(strategy.Disp(), IsFalse());

	type.m_nTsvMode = TSV_MODE_TSV;
	EXPECT_THAT(strategy.Disp(), IsFalse());

	type.m_nTsvMode = TSV_MODE_CSV;
	EXPECT_THAT(strategy.Disp(), IsTrue());
}

TEST_F(FigureStrategyTest, HanSpaceMatchesOnlyAsciiSpace)
{
	CFigure_HanSpace strategy;

	EXPECT_THAT(strategy.Match(L" ", 1), IsTrue());
	EXPECT_THAT(strategy.Match(L"\t", 1), IsFalse());
	EXPECT_THAT(strategy.Match(L"　", 1), IsFalse());
}

TEST_F(FigureStrategyTest, HanSpaceFollowsDisplaySetting)
{
	auto& type = GetTypeConfig();
	CFigure_HanSpace strategy;

	type.m_ColorInfoArr[COLORIDX_SPACE].m_bDisp = true;
	strategy.Update();
	EXPECT_THAT(strategy.Disp(), IsTrue());

	type.m_ColorInfoArr[COLORIDX_SPACE].m_bDisp = false;
	strategy.Update();
	EXPECT_THAT(strategy.Disp(), IsFalse());
}

TEST_F(FigureStrategyTest, ZenSpaceMatchesOnlyIdeographicSpace)
{
	CFigure_ZenSpace strategy;

	EXPECT_THAT(strategy.Match(L"　", 1), IsTrue());
	EXPECT_THAT(strategy.Match(L" ", 1), IsFalse());
}

TEST_F(FigureStrategyTest, ZenSpaceFollowsDisplaySetting)
{
	auto& type = GetTypeConfig();
	CFigure_ZenSpace strategy;

	type.m_ColorInfoArr[COLORIDX_ZENSPACE].m_bDisp = true;
	strategy.Update();
	EXPECT_THAT(strategy.Disp(), IsTrue());

	type.m_ColorInfoArr[COLORIDX_ZENSPACE].m_bDisp = false;
	strategy.Update();
	EXPECT_THAT(strategy.Disp(), IsFalse());
}

TEST_F(FigureStrategyTest, EolMatchesOnlyCompleteLineDelimiters)
{
	CFigure_Eol strategy;

	EXPECT_THAT(strategy.Match(L"\r\n", 2), IsTrue());
	EXPECT_THAT(strategy.Match(L"\r", 1), IsTrue());
	EXPECT_THAT(strategy.Match(L"\n", 1), IsTrue());
	EXPECT_THAT(strategy.Match(L"\r\n", 1), IsTrue());
	EXPECT_THAT(strategy.Match(L"\rX", 2), IsFalse());
	EXPECT_THAT(strategy.Match(L"X", 1), IsFalse());
	EXPECT_THAT(strategy.Disp(), IsTrue());
}

TEST_F(FigureStrategyTest, CtrlCodeRecognizesAsciiControlCodesAndFollowsDisplaySetting)
{
	auto& type = GetTypeConfig();
	CFigure_CtrlCode strategy;

	EXPECT_THAT(strategy.Match(L"\x0001", 1), IsTrue());
	EXPECT_THAT(strategy.Match(L"A", 1), IsFalse());
	EXPECT_THAT(strategy.GetAlternateChar(), Eq(L'･'));

	type.m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp = true;
	strategy.Update();
	EXPECT_THAT(strategy.Disp(), IsTrue());

	type.m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp = false;
	strategy.Update();
	EXPECT_THAT(strategy.Disp(), IsFalse());
}

} // namespace view::figure
