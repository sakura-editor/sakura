/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "window/EditorTestSuite.hpp"

#include "view/CEditView.h"

namespace view {

class EditViewISearchTest : public ::testing::Test, public window::EditorTestSuite {
public:
	static void SetUpTestSuite() { SetUpEditor(); }
	static void TearDownTestSuite() { TearDownEditor(); }

	void SetUp() override
	{
		// 共有データの検索オプションを、ビューの値と全部違う値にしておく（文字列検索・区別する・単語単位）
		auto& sharedOption = GetDllShareData().m_Common.m_sSearch.m_sSearchOption;
		m_sharedBackup = sharedOption;
		sharedOption = SSearchOption(false, true, true);
	}

	void TearDown() override
	{
		GetDllShareData().m_Common.m_sSearch.m_sSearchOption = m_sharedBackup;
	}

	/*!
	 * インクリメンタルサーチを始められないとき、ビューの検索オプションが変わらないことを確かめる
	 *
	 * CEditView::Create() を呼ばないので、正規表現のDLL(m_CurRegexp)は読み込まれていない。
	 * 正規表現・Migemoのどちらも「正規表現を使えない」で戻る。
	 */
	void ExpectNotEntered(int nCommand) const
	{
		CEditView view;
		EXPECT_THAT(view.m_CurRegexp.IsAvailable(), IsFalse());	// 前提

		const SSearchOption viewOption(true, false, false);	// 正規表現（共有データと全部違う）
		view.m_sCurSearchOption = viewOption;

		EXPECT_THAT(view.ProcessCommand_isearch(nCommand, false, 0, 0, 0, 0), IsTrue());

		// 始まらない
		EXPECT_THAT(view.m_nISearchMode, Eq(SEARCH_NONE));

		// ビューの検索オプションは書き換わらない
		EXPECT_THAT(view.m_sCurSearchOption, Eq(viewOption));
	}

private:
	SSearchOption m_sharedBackup;
};

/*!
 * @brief 正規表現インクリメンタルサーチを始められないとき、ビューの検索オプションが変わらないこと(Issue #2867)
 */
TEST_F(EditViewISearchTest, RegexpUnavailableKeepsCurSearchOption)
{
	ExpectNotEntered(F_ISEARCH_REGEXP_NEXT);
}

/*!
 * @brief MIGEMOインクリメンタルサーチを始められないとき、ビューの検索オプションが変わらないこと(Issue #2867)
 */
TEST_F(EditViewISearchTest, MigemoUnavailableKeepsCurSearchOption)
{
	ExpectNotEntered(F_ISEARCH_MIGEMO_NEXT);
}

} // namespace view
