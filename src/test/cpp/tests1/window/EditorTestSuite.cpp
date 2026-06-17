/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "window/EditorTestSuite.hpp"

#include "_main/CAppMode.h"

namespace window {

/*!
 * テストスイートの開始前に1回だけ呼ばれる関数
 */
/* static */ void EditorTestSuite::SetUpEditor()
{
	// OLEを初期化する
	if (FAILED(::OleInitialize(nullptr)))
		FAIL();

	// 言語環境を初期化する
	CSelectLang::InitializeLanguageEnvironment();

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

	const auto hInst = G_AppInstance();

	CEditApp::getInstance()->m_hInst = hInst;

	//ヘルパ作成
	CEditApp::getInstance()->m_cIcons.Create(hInst);

	// CEditViewをインスタンス化するにはドキュメントのインスタンスが必要
	pcEditDoc = std::make_unique<CEditDoc>(nullptr);
	CEditApp::getInstance()->m_pcEditDoc = pcEditDoc.get();

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

	//IO管理
	pcLoadAgent = std::make_unique<CLoadAgent>();
	CEditApp::getInstance()->m_pcLoadAgent = pcLoadAgent.get();
	pcSaveAgent = std::make_unique<CSaveAgent>();
	CEditApp::getInstance()->m_pcSaveAgent = pcSaveAgent.get();
	pcVisualProgress = std::make_unique<CVisualProgress>();
	CEditApp::getInstance()->m_pcVisualProgress = pcVisualProgress.get();

	//GREPモード管理
	pcGrepAgent = std::make_unique<CGrepAgent>();
	CEditApp::getInstance()->m_pcGrepAgent = pcGrepAgent.get();

	//編集モード
	CAppMode::getInstance();	//ウィンドウよりも前にイベントを受け取るためにここでインスタンス作成

	// SMacroMgrを用意する
	pcSMacroMgr = std::make_unique<CSMacroMgr>();

	CEditApp::getInstance()->m_pcSMacroMgr = pcSMacroMgr.get();

	//ドキュメントの作成
	pcEditDoc->Create();

	// CEditWndを用意する
	pcEditWnd = std::make_unique<CEditWnd>();
	CEditApp::getInstance()->m_pcEditWnd = pcEditWnd.get();

#pragma region CanBeMove
	// 編集ウインドウがあるので値を返す
	EXPECT_THAT(GetEditWndPtr(), pcEditWnd.get());

	// 編集ウインドウがあるのでエラーにならない
	EXPECT_NO_THROW([] { GetEditWnd(); });

#pragma endregion CanBeMove

	//MRU管理
	pcMruListener = std::make_unique<CMruListener>();
	CEditApp::getInstance()->m_pcMruListener = pcMruListener.get();

	//プロパティ管理
	pcPropertyManager = std::make_unique<CPropertyManager>();
	CEditApp::getInstance()->m_pcPropertyManager = pcPropertyManager.get();
	CEditApp::getInstance()->m_pcPropertyManager->Create(
		pcEditWnd->GetHwnd(),
		&CEditApp::getInstance()->m_cIcons,
		&pcEditWnd->GetMenuDrawer()
	);
}

/*!
 * テストスイートの終了後に1回だけ呼ばれる関数
 */
/* static */ void EditorTestSuite::TearDownEditor()
{
	CEditApp::getInstance()->m_pcSMacroMgr = nullptr;
	CEditApp::getInstance()->m_pcPropertyManager = nullptr;
	CEditApp::getInstance()->m_pcMruListener = nullptr;
	CEditApp::getInstance()->m_pcGrepAgent = nullptr;
	CEditApp::getInstance()->m_pcVisualProgress = nullptr;
	CEditApp::getInstance()->m_pcSaveAgent = nullptr;
	CEditApp::getInstance()->m_pcLoadAgent = nullptr;
	CEditApp::getInstance()->m_pcEditWnd = nullptr;
	CEditApp::getInstance()->m_pcEditDoc = nullptr;

	pcPropertyManager = nullptr;
	pcMruListener = nullptr;
	pcGrepAgent = nullptr;
	pcVisualProgress = nullptr;
	pcSaveAgent = nullptr;
	pcLoadAgent = nullptr;

	pcSMacroMgr = nullptr;

	pcEditWnd = nullptr;

	pcEditDoc = nullptr;

	TearDownShareData();

	// OLEをシャットダウンする
	::OleUninitialize();
}

} // namespace window
