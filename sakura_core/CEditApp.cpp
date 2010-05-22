#include "stdafx.h"
#include "CEditApp.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "CLoadAgent.h"
#include "CSaveAgent.h"
#include "CVisualProgress.h"
#include "recent/CMruListener.h"
#include "macro/CSMacroMgr.h"
#include "CPropertyManager.h"
#include "CGrepAgent.h"
#include "CAppMode.h"
#include "util/module.h"
#include "util/shell.h"
#include "CCommandLine.h"

CEditApp::CEditApp(HINSTANCE hInst)
: m_hInst(hInst)
{
	//ヘルパ作成
	m_cIcons.Create( m_hInst );	//	CreateImage List

	//ドキュメントの作成
	m_pcEditDoc = new CEditDoc(this);
	if( !m_pcEditDoc->Create( &m_cIcons ) ){
		ErrorMessage( NULL, _T("ドキュメントの作成に失敗しました") );
	}

	//IO管理
	m_pcLoadAgent = new CLoadAgent();
	m_pcSaveAgent = new CSaveAgent();
	m_pcVisualProgress = new CVisualProgress();

	//GREPモード管理
	m_pcGrepAgent = new CGrepAgent();

	//編集モード
	CAppMode::Instance();	//ウィンドウよりも前にイベントを受け取るためにここでインスタンス作成

	//ウィンドウの作成
	m_pcEditWnd = new CEditWnd();
	m_pcEditWnd->Create(
		CCommandLine::Instance()->GetGroupId()
	);

	//MRU管理
	m_pcMruListener = new CMruListener();

	//マクロ
	m_pcSMacroMgr = new CSMacroMgr();

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
			CEditApp::Instance()->GetHelpFilePath(),
			HELP_CONTEXT,
			nHelpContextID
		);
	}
}
