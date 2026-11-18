/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "window/EditorTestSuite.hpp"

#include "view/CEditView.h"
#include "extmodule/CMigemo.h"

#include "tests1_rc.h"

void extract_zip_resource(WORD id, const std::optional<std::filesystem::path>& optOutDir);

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

		const auto& sHelper = GetDllShareData().m_Common.m_sHelper;
		m_migemoDllBackup = sHelper.m_szMigemoDll;
		m_migemoDictBackup = sHelper.m_szMigemoDict;

		m_dictPath = GetIniFileName().replace_filename(L"dict");
		RemoveDict();
	}

	void TearDown() override
	{
		m_pcMigemo = nullptr;
		RemoveDict();

		auto& sHelper = GetDllShareData().m_Common.m_sHelper;
		sHelper.m_szMigemoDll = m_migemoDllBackup;
		sHelper.m_szMigemoDict = m_migemoDictBackup;

		GetDllShareData().m_Common.m_sSearch.m_sSearchOption = m_sharedBackup;
	}

	//! ビューの正規表現を使えるようにする（CEditView::Create() と同じ読み込み方）
	static void InitRegexp(CEditView& view)
	{
		view.m_CurRegexp.InitDll(GetDllShareData().m_Common.m_sSearch.m_szRegexpLib);
	}

	/*!
	 * Migemoのインスタンスを用意する
	 *
	 * @param bDict true: 辞書を置く / false: 辞書を置かない（Migemoを使えない）
	 */
	void PrepareMigemo(bool bDict)
	{
		auto& sHelper = GetDllShareData().m_Common.m_sHelper;
		sHelper.m_szMigemoDll = L"migemo.dll";
		sHelper.m_szMigemoDict = L"dict";
		if (bDict) {
			ExtractDict();
		}
		m_pcMigemo = std::make_unique<CMigemo>();
	}

	//! Migemoの辞書を置く
	void ExtractDict() const
	{
		extract_zip_resource(IDR_ZIPRES2, m_dictPath);
	}

	/*!
	 * インクリメンタルサーチを始められないとき、モードもビューの検索オプションも変わらないことを確かめる
	 */
	static void ExpectNotEntered(CEditView& view, int nCommand)
	{
		const ESearchMode modeBefore = view.m_nISearchMode;
		const SSearchOption optionBefore = view.m_sCurSearchOption;

		EXPECT_THAT(view.ProcessCommand_isearch(nCommand, false, 0, 0, 0, 0), IsTrue());

		// 始まらない（前のモードのまま）
		EXPECT_THAT(view.m_nISearchMode, Eq(modeBefore));

		// ビューの検索オプションは書き換わらない
		EXPECT_THAT(view.m_sCurSearchOption, Eq(optionBefore));
	}

	/*!
	 * インクリメンタルサーチが始まり、ビューの検索オプションが共有データから作った値になることを確かめる
	 */
	static void ExpectEntered(CEditView& view, int nCommand, ESearchMode mode, const SSearchOption& expected)
	{
		EXPECT_THAT(view.ProcessCommand_isearch(nCommand, false, 0, 0, 0, 0), IsTrue());

		EXPECT_THAT(view.m_nISearchMode, Eq(mode));
		EXPECT_THAT(view.m_sCurSearchOption, Eq(expected));
	}

private:
	void RemoveDict() const
	{
		std::error_code ec;
		std::filesystem::remove_all(m_dictPath, ec);
	}

	SSearchOption m_sharedBackup;
	SFilePath m_migemoDllBackup;
	SFilePath m_migemoDictBackup;
	std::filesystem::path m_dictPath;
	std::unique_ptr<CMigemo> m_pcMigemo;
};

//! ビューに入れておく検索オプション（共有データとも、始まったときの値とも違う）
const SSearchOption viewOption(true, true, false);

/*!
 * @brief 通常のインクリメンタルサーチ(前方)が始まり、正規表現・大小区別・単語単位を外した値になること
 */
TEST_F(EditViewISearchTest, NormalForwardEnters)
{
	CEditView view;
	view.m_sCurSearchOption = viewOption;

	ExpectEntered(view, F_ISEARCH_NEXT, SEARCH_NORMAL, SSearchOption(false, false, false));
	EXPECT_THAT(view.m_nISearchDirection, Eq(SEARCH_FORWARD));
}

/*!
 * @brief 通常のインクリメンタルサーチ(後方)が始まること
 */
TEST_F(EditViewISearchTest, NormalBackwardEnters)
{
	CEditView view;
	view.m_sCurSearchOption = viewOption;

	ExpectEntered(view, F_ISEARCH_PREV, SEARCH_NORMAL, SSearchOption(false, false, false));
	EXPECT_THAT(view.m_nISearchDirection, Eq(SEARCH_BACKWARD));
}

/*!
 * @brief 正規表現を使えるとき、正規表現インクリメンタルサーチが始まり、単語単位は共有データのままになること
 */
TEST_F(EditViewISearchTest, RegexpEntersWhenAvailable)
{
	CEditView view;
	InitRegexp(view);
	EXPECT_THAT(view.m_CurRegexp.IsAvailable(), IsTrue());	// 前提
	view.m_sCurSearchOption = viewOption;

	ExpectEntered(view, F_ISEARCH_REGEXP_NEXT, SEARCH_REGEXP, SSearchOption(true, false, true));
}

/*!
 * @brief 正規表現を使えないとき、正規表現インクリメンタルサーチが始まらず、ビューの検索オプションが変わらないこと(Issue #2867)
 *
 * CEditView::Create() を呼ばないので、正規表現のDLL(m_CurRegexp)は読み込まれていない。
 */
TEST_F(EditViewISearchTest, RegexpUnavailableKeepsCurSearchOption)
{
	CEditView view;
	EXPECT_THAT(view.m_CurRegexp.IsAvailable(), IsFalse());	// 前提
	view.m_sCurSearchOption = viewOption;

	ExpectNotEntered(view, F_ISEARCH_REGEXP_NEXT);
	EXPECT_THAT(view.m_nISearchMode, Eq(SEARCH_NONE));
}

/*!
 * @brief 正規表現とMigemoを使えるとき、MIGEMOインクリメンタルサーチが始まること
 */
TEST_F(EditViewISearchTest, MigemoEntersWhenAvailable)
{
	PrepareMigemo(true);

	CEditView view;
	InitRegexp(view);
	EXPECT_THAT(view.m_CurRegexp.IsAvailable(), IsTrue());	// 前提
	view.m_sCurSearchOption = viewOption;

	ExpectEntered(view, F_ISEARCH_MIGEMO_NEXT, SEARCH_MIGEMO, SSearchOption(true, false, true));
}

/*!
 * @brief 正規表現を使えないとき、MIGEMOインクリメンタルサーチが始まらず、ビューの検索オプションが変わらないこと(Issue #2867)
 */
TEST_F(EditViewISearchTest, MigemoUnavailableKeepsCurSearchOption)
{
	CEditView view;
	EXPECT_THAT(view.m_CurRegexp.IsAvailable(), IsFalse());	// 前提
	view.m_sCurSearchOption = viewOption;

	ExpectNotEntered(view, F_ISEARCH_MIGEMO_NEXT);
	EXPECT_THAT(view.m_nISearchMode, Eq(SEARCH_NONE));
}

/*!
 * @brief Migemoの辞書が無いとき、MIGEMOインクリメンタルサーチが始まらず、ビューの検索オプションが変わらないこと(Issue #2867)
 *
 * 正規表現は使える。Issue の再現手順と同じ「Migemoだけ使えない」経路。
 */
TEST_F(EditViewISearchTest, MigemoDictUnavailableKeepsCurSearchOption)
{
	PrepareMigemo(false);

	CEditView view;
	InitRegexp(view);
	EXPECT_THAT(view.m_CurRegexp.IsAvailable(), IsTrue());	// 前提
	view.m_sCurSearchOption = viewOption;

	ExpectNotEntered(view, F_ISEARCH_MIGEMO_NEXT);
	EXPECT_THAT(view.m_nISearchMode, Eq(SEARCH_NONE));
}

/*!
 * @brief インクリメンタルサーチ中に使えないモードへ切り替えると、前のモードと検索オプションのまま続くこと(Issue #2867)
 */
TEST_F(EditViewISearchTest, SwitchToUnavailableModeKeepsCurrentMode)
{
	CEditView view;
	EXPECT_THAT(view.m_CurRegexp.IsAvailable(), IsFalse());	// 前提
	view.m_sCurSearchOption = viewOption;

	ExpectEntered(view, F_ISEARCH_NEXT, SEARCH_NORMAL, SSearchOption(false, false, false));

	ExpectNotEntered(view, F_ISEARCH_MIGEMO_NEXT);
	EXPECT_THAT(view.m_nISearchMode, Eq(SEARCH_NORMAL));
}

/*!
 * @brief インクリメンタルサーチ中に同じモードを選ぶと、再実行になり、方向だけが変わること
 *
 * 再実行の枝は検索オプションに触らない。共有データを変えておき、写し直さないことも確かめる。
 */
TEST_F(EditViewISearchTest, ReenterSameModeChangesDirectionOnly)
{
	CEditView view;
	view.m_sCurSearchOption = viewOption;

	ExpectEntered(view, F_ISEARCH_NEXT, SEARCH_NORMAL, SSearchOption(false, false, false));
	EXPECT_THAT(view.m_bISearchFirst, IsTrue());

	GetDllShareData().m_Common.m_sSearch.m_sSearchOption = SSearchOption(true, true, true);

	EXPECT_THAT(view.ProcessCommand_isearch(F_ISEARCH_PREV, false, 0, 0, 0, 0), IsTrue());

	EXPECT_THAT(view.m_nISearchMode, Eq(SEARCH_NORMAL));
	EXPECT_THAT(view.m_nISearchDirection, Eq(SEARCH_BACKWARD));
	EXPECT_THAT(view.m_bISearchFirst, IsFalse());
	EXPECT_THAT(view.m_sCurSearchOption, Eq(SSearchOption(false, false, false)));
}

/*!
 * @brief インクリメンタルサーチ中に使えるモードへ切り替えると、検索オプションが変わり、検索開始位置は変わらないこと
 *
 * 検索開始位置は、インクリメンタルサーチでないところから入ったときだけ記録する。
 */
TEST_F(EditViewISearchTest, SwitchBetweenAvailableModesKeepsStartPos)
{
	CEditView view;
	InitRegexp(view);
	EXPECT_THAT(view.m_CurRegexp.IsAvailable(), IsTrue());	// 前提
	view.m_sCurSearchOption = viewOption;

	ExpectEntered(view, F_ISEARCH_NEXT, SEARCH_NORMAL, SSearchOption(false, false, false));

	const CLogicPoint startPos(5, 3);	// 入ったときの位置(0, 0)と違う値
	view.m_ptSrchStartPos_PHY = startPos;

	ExpectEntered(view, F_ISEARCH_REGEXP_PREV, SEARCH_REGEXP, SSearchOption(true, false, true));
	EXPECT_THAT(view.m_nISearchDirection, Eq(SEARCH_BACKWARD));
	EXPECT_THAT(view.m_ptSrchStartPos_PHY, Eq(startPos));
}

/*!
 * @brief Migemoの辞書が無くて始められなかった後、辞書を置いてもう一度選ぶと始まること(Issue #2867)
 *
 * 1 回目で、ビューの検索オプションもMigemoの状態も壊れていないことを確かめる。
 */
TEST_F(EditViewISearchTest, MigemoRetryAfterDictPlaced)
{
	PrepareMigemo(false);

	CEditView view;
	InitRegexp(view);
	EXPECT_THAT(view.m_CurRegexp.IsAvailable(), IsTrue());	// 前提
	view.m_sCurSearchOption = viewOption;

	ExpectNotEntered(view, F_ISEARCH_MIGEMO_NEXT);
	EXPECT_THAT(view.m_nISearchMode, Eq(SEARCH_NONE));

	ExtractDict();

	ExpectEntered(view, F_ISEARCH_MIGEMO_PREV, SEARCH_MIGEMO, SSearchOption(true, false, true));
	EXPECT_THAT(view.m_nISearchDirection, Eq(SEARCH_BACKWARD));
}

/*!
 * @brief 文字列を選んだ状態で始めると、選択が解除されること
 *
 * HWND の無いビューで選択の描画に進まないよう、描画スイッチを切っておく。
 */
TEST_F(EditViewISearchTest, EnterClearsSelection)
{
	CEditView view;
	view.SetDrawSwitch(false);
	view.m_sCurSearchOption = viewOption;

	view.GetSelectionInfo().SetSelectArea(CLayoutRange(CLayoutPoint(0, 0), CLayoutPoint(1, 0)));
	EXPECT_THAT(view.GetSelectionInfo().IsTextSelected(), IsTrue());	// 前提

	ExpectEntered(view, F_ISEARCH_NEXT, SEARCH_NORMAL, SSearchOption(false, false, false));
	EXPECT_THAT(view.GetSelectionInfo().IsTextSelected(), IsFalse());
}

/*!
 * @brief インクリメンタルサーチのコマンドでなければ処理せず、モードも検索オプションも変わらないこと
 */
TEST_F(EditViewISearchTest, OtherCommandIsNotProcessed)
{
	CEditView view;
	view.m_sCurSearchOption = viewOption;

	EXPECT_THAT(view.ProcessCommand_isearch(F_0, false, 0, 0, 0, 0), IsFalse());

	EXPECT_THAT(view.m_nISearchMode, Eq(SEARCH_NONE));
	EXPECT_THAT(view.m_sCurSearchOption, Eq(viewOption));
}

} // namespace view
