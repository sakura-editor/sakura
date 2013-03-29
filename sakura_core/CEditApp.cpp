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

CEditApp::CEditApp(HINSTANCE hInst, int nGroupId)
: m_hInst(hInst)
{
	//ヘルパ作成
	m_cIcons.Create( m_hInst );	//	CreateImage List

	//ドキュメントの作成
	m_pcEditDoc = new CEditDoc(this);
	m_pcEditDoc->Create( &m_cIcons );

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
	m_pcEditWnd = new CEditWnd();
	m_pcEditWnd->Create( nGroupId );

	//MRU管理
	m_pcMruListener = new CMruListener();

	//プロパティ管理
	m_pcPropertyManager = new CPropertyManager();
}

CEditApp::~CEditApp()
{
	delete m_pcSMacroMgr;
	delete m_pcPropertyManager;
	delete m_pcMruListener;
	delete m_pcEditWnd;
	delete m_pcGrepAgent;
	delete m_pcVisualProgress;
	delete m_pcSaveAgent;
	delete m_pcLoadAgent;
	delete m_pcEditDoc;
}


/*! ヘルプファイルのフルパスを返す
 
    @return パスを格納したバッファのポインタ
 
    @note 実行ファイルと同じ位置の sakura.chm ファイルを返す。
        パスが UNC のときは _MAX_PATH に収まらない可能性がある。
 
    @date 2002/01/19 aroka ；nMaxLen 引数追加
	@date 2007/10/23 kobake 引数説明の誤りを修正(in→out)
	@date 2007/10/23 kobake CEditAppのメンバ関数に変更
	@date 2007/10/23 kobake シグニチャ変更。constポインタを返すだけのインターフェースにしました。
*/
LPCTSTR CEditApp::GetHelpFilePath() const
{
	static TCHAR szHelpFile[_MAX_PATH] = _T("");
	if(szHelpFile[0]==_T('\0')){
		GetExedir( szHelpFile, _T("sakura.chm") );
	}
	return szHelpFile;
}


/* メニューアイテムに対応するヘルプを表示 */
void CEditApp::ShowFuncHelp( HWND hwndParent, EFunctionCode nFuncID ) const
{
	/* 機能IDに対応するヘルプコンテキスト番号を返す */
	int		nHelpContextID = FuncID_To_HelpContextID( nFuncID );
	if( 0 != nHelpContextID ){
		// 2006.10.10 ryoji MyWinHelpに変更に変更
		MyWinHelp(
			hwndParent,
			CEditApp::getInstance()->GetHelpFilePath(),
			HELP_CONTEXT,
			nHelpContextID
		);
	}
}
