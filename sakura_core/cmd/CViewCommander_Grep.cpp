/*!	@file
@brief CViewCommanderクラスのコマンド(Grep)関数群

*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
#include "_main/CControlTray.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "window/CEditWnd.h"

/*! GREPダイアログの表示

	@date 2005.01.10 genta CEditView_Commandより移動
	@author Yazaki
*/
void CViewCommander::Command_GREP_DIALOG( void )
{
	CNativeW	cmemCurText;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji ダイアログ専用関数に変更

	if( 0 < cmemCurText.GetStringLength() ){
		GetEditWindow()->m_cDlgGrep.m_strText = cmemCurText.GetStringPtr();
	}

	/* Grepダイアログの表示 */
	int nRet = GetEditWindow()->m_cDlgGrep.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath() );
//	MYTRACE_A( "nRet=%d\n", nRet );
	if( !nRet ){
		return;
	}
	HandleCommand(F_GREP, true, 0, 0, 0, 0);	//	GREPコマンドの発行
}

/*! GREP実行

	@date 2005.01.10 genta CEditView_Commandより移動
*/
void CViewCommander::Command_GREP( void )
{
	CNativeW		cmWork1;
	CNativeT		cmWork2;
	CNativeT		cmWork3;

	cmWork1.SetString( GetEditWindow()->m_cDlgGrep.m_strText.c_str() );
	cmWork2.SetString( GetEditWindow()->m_cDlgGrep.m_szFile );
	cmWork3.SetString( GetEditWindow()->m_cDlgGrep.m_szFolder );

	/*	今のEditViewにGrep結果を表示する。
		Grepモードのとき、または未編集で無題かつアウトプットでない場合。
		自ウィンドウがGrep実行中も、(異常終了するので)別ウィンドウにする
	*/
	if( (  CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode &&
		  !CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning ) ||
		( !GetDocument()->m_cDocEditor.IsModified() &&
		  !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() &&		/* 現在編集中のファイルのパス */
		  !CAppMode::getInstance()->IsDebugMode()
		)
	){
		// 2011.01.23 Grepタイプ別適用
		if( !GetDocument()->m_cDocEditor.IsModified() && GetDocument()->m_cDocLineMgr.GetLineCount() == 0 ){
			CTypeConfig cTypeGrep = CDocTypeManager().GetDocumentTypeOfExt( _T("grepout") );
			GetDocument()->m_cDocType.SetDocumentType( cTypeGrep, false );
			GetDocument()->m_cDocType.LockDocumentType();
			GetDocument()->OnChangeType();
		}
		
		CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
			m_pCommanderView,
			&cmWork1,
			&cmWork2,
			&cmWork3,
			GetEditWindow()->m_cDlgGrep.m_bSubFolder,
			GetEditWindow()->m_cDlgGrep.m_sSearchOption,
			GetEditWindow()->m_cDlgGrep.m_nGrepCharSet,
			GetEditWindow()->m_cDlgGrep.m_bGrepOutputLine,
			GetEditWindow()->m_cDlgGrep.m_nGrepOutputStyle
		);
	}
	else{
		// 編集ウィンドウの上限チェック
		if( GetDllShareData().m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
			OkMessage( m_pCommanderView->GetHwnd(), _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
			return;
		}

		/*======= Grepの実行 =============*/
		/* Grep結果ウィンドウの表示 */
		CControlTray::DoGrepCreateWindow(G_AppInstance(), m_pCommanderView->GetHwnd(), GetEditWindow()->m_cDlgGrep);
	}
	return;
}

