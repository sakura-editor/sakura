/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "window/EditorTestSuite.hpp"

namespace window {

/*!
 * テストスイートの開始前に1回だけ呼ばれる関数
 */
/* static */ void EditorTestSuite::SetUpEditor()
{
	// OLEを初期化する
	if (FAILED(::OleInitialize(nullptr)))
		FAIL();

	SetUpShareData();

	// CanBeMoveリージョンをテストケースに分割する。（すぐ対応できないのでコメント残し）

	// ドキュメントの初期化前に文字幅キャッシュの生成が必要
	SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_SHARE);
	InitCharWidthCache(GetDllShareData().m_Common.m_sView.m_lf);

#pragma region CanBeMove
	// ドキュメントがなくてもエラーにならない
	EXPECT_THAT(GetDocument(), IsNull());

	// ドキュメントがないのでエラー
	EXPECT_ANY_THROW(GetEditDoc());

#pragma endregion CanBeMove

	// CEditViewをインスタンス化するにはドキュメントのインスタンスが必要
	pcEditDoc = std::make_unique<CEditDoc>(nullptr);

#pragma region CanBeMove
	// ドキュメントがあるので値を返す
	EXPECT_THAT(GetDocument(), pcEditDoc.get());

	// ドキュメントがあるのでエラーにならない
	EXPECT_NO_THROW([] { GetEditDoc(); });

	EXPECT_THAT(&GetEditDoc(), GetDocument());

	// 編集ウインドウがなくてもエラーにならない
	EXPECT_THAT(GetEditWndPtr(), IsNull());

	// 編集ウインドウがないのでエラー
	EXPECT_ANY_THROW(GetEditWnd());

#pragma endregion CanBeMove

	// CEditWndを用意する
	pcEditWnd = std::make_unique<CEditWnd>();

	// SMacroMgrを用意する
	pcSMacroMgr = std::make_unique<CSMacroMgr>();

#pragma region CanBeMove
	// 編集ウインドウがあるので値を返す
	EXPECT_THAT(GetEditWndPtr(), pcEditWnd.get());

	// 編集ウインドウがあるのでエラーにならない
	EXPECT_NO_THROW([] { GetEditWnd(); });

#pragma endregion CanBeMove
}

/*!
 * テストスイートの終了後に1回だけ呼ばれる関数
 */
/* static */ void EditorTestSuite::TearDownEditor()
{
	pcSMacroMgr = nullptr;

	pcEditWnd = nullptr;

	pcEditDoc = nullptr;

	TearDownShareData();

	// OLEをシャットダウンする
	::OleUninitialize();
}

} // namespace window
