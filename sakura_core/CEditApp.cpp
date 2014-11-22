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
#include "CSMacroMgr.h"
#include "CEditWnd.h"

void CEditApp::Create(HINSTANCE hInst, HWND hwndParent, int nGroupId)
{
	//ヘルパ作成
	m_cIcons.Create( hInst );	//	CreateImage List

	//ドキュメントの作成
	m_pcEditDoc = new CEditDoc();

	//マクロ
	m_pcSMacroMgr = new CSMacroMgr();

	//ウィンドウの作成
	m_pcEditWnd = CEditWnd::getInstance();

	m_pcEditDoc->Create( hInst, m_pcEditWnd, &m_cIcons );
	m_pcEditWnd->Create( hInst, hwndParent, m_pcEditDoc, &m_cIcons, nGroupId );

	//プロパティ管理
	m_pcPropertyManager = new CPropertyManager();
	m_pcPropertyManager->Create(
		hInst,
		m_pcEditWnd->m_hWnd,
		&m_cIcons,
		&m_pcEditWnd->m_cMenuDrawer
	);
}

CEditApp::~CEditApp()
{
	delete m_pcSMacroMgr;
	delete m_pcPropertyManager;
	delete m_pcEditDoc;
}

/*! 共通設定 プロパティシート */
bool CEditApp::OpenPropertySheet( int nPageNum )
{
	/* プロパティシートの作成 */
	bool bRet = m_pcPropertyManager->OpenPropertySheet( m_pcEditWnd->m_hWnd, nPageNum );
	if( bRet ){
		// 2007.10.19 genta マクロ登録変更を反映するため，読み込み済みのマクロを破棄する
		m_pcSMacroMgr->UnloadAll();
	}

	return bRet;
}

/*! タイプ別設定 プロパティシート */
bool CEditApp::OpenPropertySheetTypes( int nPageNum, int nSettingType )
{
	int nTextWrapMethodOld = m_pcEditDoc->GetDocumentAttribute().m_nTextWrapMethod;

	bool bRet = m_pcPropertyManager->OpenPropertySheetTypes( m_pcEditWnd->m_hWnd, nPageNum, nSettingType );
	if( bRet ){
		// 2008.06.01 nasukoji	テキストの折り返し位置変更対応
		// タイプ別設定を呼び出したウィンドウについては、タイプ別設定が変更されたら
		// 折り返し方法の一時設定適用中を解除してタイプ別設定を有効とする。
		if( nTextWrapMethodOld != m_pcEditDoc->GetDocumentAttribute().m_nTextWrapMethod ){		// 設定が変更された
			m_pcEditDoc->m_bTextWrapMethodCurTemp = false;	// 一時設定適用中を解除
		}
	}

	return bRet;
}

/*[EOF]*/
