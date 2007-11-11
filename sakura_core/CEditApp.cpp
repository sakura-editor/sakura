#include "stdafx.h"
#include "CEditApp.h"
#include "util/module.h"
#include "CEditWnd.h"
#include "util/shell.h"
#include "CCommandLine.h"

#pragma warning(disable:4355) //「thisポインタが初期化リストで使用されました」の警告を無効化

CEditApp::CEditApp(HINSTANCE hInst)
: m_hInst(hInst)
, m_cEditDoc(this)
{
	//ヘルパ作成
	m_cIcons.Create( m_hInst );	//	CreateImage List

	//ドキュメントの作成
	if( !m_cEditDoc.Create( m_hInst, &m_cIcons ) ){
		ErrorMessage( NULL, _T("ドキュメントの作成に失敗しました") );
	}

	//ウィンドウの作成
	m_pcEditWnd = new CEditWnd();
	m_pcEditWnd->Create(
		hInst,
		CCommandLine::Instance()->GetGroupId()
	);
}

CEditApp::~CEditApp()
{
	delete m_pcEditWnd;
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
