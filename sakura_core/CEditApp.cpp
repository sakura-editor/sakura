/*
	Copyright (C) 2007, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "CEditApp.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "CLoadAgent.h"
#include "CSaveAgent.h"
#include "uiparts/CVisualProgress.h"
#include "recent/CMruListener.h"
#include "macro/CSMacroMgr.h"
#include "CPropertyManager.h"
#include "CGrepAgent.h"
#include "_main/CAppMode.h"
#include "_main/CCommandLine.h"
#include "util/module.h"
#include "util/shell.h"

void CEditApp::Create(HINSTANCE hInst, int nGroupId)
{
	m_hInst = hInst;

	//ヘルパ作成
	m_cIcons.Create( m_hInst );	//	CreateImage List

	//ドキュメントの作成
	m_pcEditDoc = new CEditDoc(this);

	//IO管理
	m_pcLoadAgent = new CLoadAgent();
	m_pcSaveAgent = new CSaveAgent();
	m_pcVisualProgress = new CVisualProgress();

	//GREPモード管理
	m_pcGrepAgent = new CGrepAgent();

	//編集モード
	CAppMode::getInstance();	//ウィンドウよりも前にイベントを受け取るためにここでインスタンス作成

	//マクロ
	m_pcSMacroMgr = new CSMacroMgr();

	//ウィンドウの作成
	m_pcEditWnd = CEditWnd::getInstance();

	m_pcEditDoc->Create( m_pcEditWnd );
	m_pcEditWnd->Create( m_pcEditDoc, &m_cIcons, nGroupId );

	//MRU管理
	m_pcMruListener = new CMruListener();

	//プロパティ管理
	m_pcPropertyManager = new CPropertyManager();
	m_pcPropertyManager->Create(
		m_pcEditWnd->GetHwnd(),
		&GetIcons(),
		&m_pcEditWnd->GetMenuDrawer()
	);
}

CEditApp::~CEditApp()
{
	delete m_pcSMacroMgr;
	delete m_pcPropertyManager;
	delete m_pcMruListener;
	delete m_pcGrepAgent;
	delete m_pcVisualProgress;
	delete m_pcSaveAgent;
	delete m_pcLoadAgent;
	delete m_pcEditDoc;
}

/*! 共通設定 プロパティシート */
bool CEditApp::OpenPropertySheet( int nPageNum )
{
	/* プロパティシートの作成 */
	bool bRet = m_pcPropertyManager->OpenPropertySheet( m_pcEditWnd->GetHwnd(), nPageNum, false );
	if( bRet ){
		// 2007.10.19 genta マクロ登録変更を反映するため，読み込み済みのマクロを破棄する
		m_pcSMacroMgr->UnloadAll();
	}

	return bRet;
}

/*! タイプ別設定 プロパティシート */
bool CEditApp::OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType )
{
	bool bRet = m_pcPropertyManager->OpenPropertySheetTypes( m_pcEditWnd->GetHwnd(), nPageNum, nSettingType );

	return bRet;
}

