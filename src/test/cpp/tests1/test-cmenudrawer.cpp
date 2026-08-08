/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "env/ShareDataTestSuite.hpp"

#include "uiparts/CMenuDrawer.h"

/*!
 * @brief CMenuDrawerのテスト
 *
 * CMenuDrawerはenv::ShareDataClientを継承しているので共有メモリが要る。
 * ただしコンストラクタはツールバーボタンの配列を組み立てるだけでGDIには触れないため、
 * ウィンドウやデバイスコンテキストは不要（それらを使うのはCreate）。
 */
struct CMenuDrawerTest : public ::testing::Test, public env::ShareDataTestSuite {
	//! ツールバー番号の最大値。CMenuDrawerのコンストラクタが持つtbd[]の要素数と一致する。
	//! 0番はセパレータで、1番以降がtbd[]の各要素に対応する。
	static constexpr int TOOLBAR_NO_MAX = 481;

	static void SetUpTestSuite()
	{
		SetUpShareData();
	}

	static void TearDownTestSuite()
	{
		TearDownShareData();
	}
};

/*!
 * ツールバーボタンの有効・無効は、そのボタン自身のコマンドで決まる。
 *
 * 修正前は隣の要素tbd[i]を見ていた（コマンドはtbd[i-INDEX_GAP]）ため、
 * 実コマンドとF_DISABLEが隣り合う箇所で有効・無効が入れ替わっていた。
 * 全481ボタンのうち33ボタンが該当する。
 * 最後の481番はtbd[]の範囲外を読んでおり、こちらは値が保証されない。
 */
TEST_F(CMenuDrawerTest, ToolbarButtonStateComesFromItsOwnFuncCode)
{
	const CMenuDrawer drawer;

	struct TestData {
		int				toolbarNo;
		EFunctionCode	funcCode;
		bool			enabled;
	};
	constexpr TestData testData[] = {
		{  16, F_OPEN_HfromtoC,		true  },	//!< 次がF_DISABLEなので修正前は無効になっていた
		{  17, F_DISABLE,			false },
		{  18, F_DISABLE,			false },	//!< 次が実コマンドなので修正前は有効になっていた
		{  19, F_ACTIVATE_SQLPLUS,	true  },
		{  51, F_UNINDENT_SPACE,	true  },	//!< 修正前は無効になっていた
		{  52, F_DISABLE,			false },	//!< 修正前は有効になっていた
		{  53, F_LTRIM,				true  },
		{ 403, EFunctionCode(F_USERMACRO_0 + 49),
									true  },	//!< 実コマンドの最後(外部マクロ50)。修正前は無効になっていた
		{ 481, F_DISABLE,			false },	//!< tbd[]の最終要素。修正前はtbd[481]という範囲外を読んでいた
	};

	for (const auto& data : testData) {
		const auto button = drawer.getButton(data.toolbarNo);
		EXPECT_EQ(static_cast<EFunctionCode>(button.idCommand), data.funcCode) << "toolbarNo = " << data.toolbarNo;
		EXPECT_EQ(button.fsState, data.enabled ? BYTE(TBSTATE_ENABLED) : BYTE(0)) << "toolbarNo = " << data.toolbarNo;
	}
}

/*!
 * すべてのツールバーボタンについて、無効なのはF_DISABLEのときだけである。
 *
 * 0番はセパレータで、F_DISABLEではないが状態も0なので対象から外す。
 */
TEST_F(CMenuDrawerTest, ToolbarButtonIsDisabledOnlyWhenItIsF_DISABLE)
{
	const CMenuDrawer drawer;

	for (int toolbarNo = 1; toolbarNo <= TOOLBAR_NO_MAX; ++toolbarNo) {
		const auto button = drawer.getButton(toolbarNo);
		const bool disabled = (button.fsState == 0);
		EXPECT_EQ(button.idCommand == F_DISABLE, disabled) << "toolbarNo = " << toolbarNo;
	}
}

/*!
 * ダミーとして置かれているボタンの内容を確認する。
 *
 * セパレータと折り返し用の仮想ボタンは、tbd[]の値をそのまま使わない特別扱いになっている。
 */
TEST_F(CMenuDrawerTest, SeparatorAndWrapButtonAreDummies)
{
	const CMenuDrawer drawer;

	const auto separator = drawer.getButton(CMenuDrawer::TOOLBAR_BUTTON_F_SEPARATOR);
	EXPECT_EQ(static_cast<EFunctionCode>(separator.idCommand), F_SEPARATOR);
	EXPECT_EQ(separator.fsStyle, BYTE(TBSTYLE_SEP));
	EXPECT_EQ(separator.iBitmap, -1);

	// tbd[]ではF_TOOLBARWRAPだが、ボタンには別のコマンドが入る
	const auto wrap = drawer.getButton(CMenuDrawer::TOOLBAR_BUTTON_F_TOOLBARWRAP);
	EXPECT_EQ(static_cast<EFunctionCode>(wrap.idCommand), F_MENU_NOT_USED_FIRST);
	EXPECT_EQ(wrap.fsState, BYTE(TBSTATE_ENABLED | TBSTATE_WRAP));
	EXPECT_EQ(wrap.fsStyle, BYTE(TBSTYLE_SEP));
	EXPECT_EQ(wrap.iBitmap, -1);
}

/*!
 * アイコン番号は、ダミー以外のコマンドにのみ割り当てられる。
 */
TEST_F(CMenuDrawerTest, IconIndexIsAssignedOnlyToRealCommands)
{
	const CMenuDrawer drawer;

	for (int toolbarNo = 1; toolbarNo <= TOOLBAR_NO_MAX; ++toolbarNo) {
		if (toolbarNo == CMenuDrawer::TOOLBAR_BUTTON_F_TOOLBARWRAP) {
			continue;	// 折り返し用の仮想ボタンは対象外
		}
		const auto button = drawer.getButton(toolbarNo);
		if (F_DUMMY_MAX_CODE < button.idCommand) {
			// アイコン番号はツールバー番号から、セパレータの分だけずれる
			EXPECT_EQ(button.iBitmap, toolbarNo - 1) << "toolbarNo = " << toolbarNo;
		}
		else {
			EXPECT_EQ(button.iBitmap, -1) << "toolbarNo = " << toolbarNo;
		}
	}
}
