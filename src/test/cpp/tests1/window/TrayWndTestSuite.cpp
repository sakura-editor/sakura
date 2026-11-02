/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "window/TrayWndTestSuite.hpp"

#include "env/CPropertyManager.h"

namespace window {

/*!
 * テストスイートの開始前に1回だけ呼ばれる関数
 */
/* static */ void TrayWndTestSuite::SetUpTrayWnd()
{
	// コマンドラインオブジェクトを用意する
	pCommandLine = std::make_unique<CCommandLine>();
	pCommandLine->ParseCommandLine(L"-NOWIN -PROF=", false);

	SetUpShareData();

	// トレイウィンドウをインスタンス化する
	pcTrayWnd = std::make_unique<CControlTray>();

	pcTrayWnd->m_hIcons.Create(G_AppInstance());

	pcTrayWnd->m_cMenuDrawer.Create(
		CSelectLang::getLangRsrcInstance(),
		pcTrayWnd->GetTrayHwnd(),
		&pcTrayWnd->m_hIcons
	);

	//プロパティ管理
	pcTrayWnd->m_pcPropertyManager = new CPropertyManager();
	pcTrayWnd->m_pcPropertyManager->Create(
		pcTrayWnd->GetTrayHwnd(),
		&pcTrayWnd->m_hIcons,
		&pcTrayWnd->m_cMenuDrawer
	);
}

/*!
 * テストスイートの終了後に1回だけ呼ばれる関数
 */
/* static */ void TrayWndTestSuite::TearDownTrayWnd()
{

	// トレイウィンドウのインスタンスを破棄する
	pcTrayWnd = nullptr;

	TearDownShareData();

	pCommandLine = nullptr;
}

} // namespace window
