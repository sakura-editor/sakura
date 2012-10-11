/*!	@file
	@brief CEditViewクラスのgrep関連コマンド処理系関数群

	@author genta
	@date	2005/01/10 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "CEditApp.h"
#include "charcode.h"
#include "Debug.h"
#include "sakura_rc.h"

/*!
	コマンドコードの変換(grep mode時)
*/
void CEditView::TranslateCommand_grep(
	int&	nCommand,
	BOOL&	bRedraw,
	LPARAM&	lparam1,
	LPARAM&	lparam2,
	LPARAM&	lparam3,
	LPARAM&	lparam4
)
{
	if( ! m_pcEditDoc->m_bGrepMode )
		return;

	if( nCommand == F_CHAR ){
		//	Jan. 23, 2005 genta 文字判定忘れ
		if(( lparam1 == CR || lparam1 == LF ) && m_pShareData->m_Common.m_bGTJW_RETURN ){
			nCommand = F_TAGJUMP;
			lparam1 = GetKeyState_Control();
		}
	}
}

/*! GREPダイアログの表示

	@date 2005.01.10 genta CEditView_Commandより移動
	@author Yazaki
*/
void CEditView::Command_GREP_DIALOG( void )
{
	CMemory		cmemCurText;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji ダイアログ専用関数に変更

	/* キーがないなら、履歴からとってくる */
	if( 0 == cmemCurText.GetStringLength() ){
//		cmemCurText.SetData( m_pShareData->m_szSEARCHKEYArr[0], lstrlen( m_pShareData->m_szSEARCHKEYArr[0] ) );
		cmemCurText.SetString( m_pShareData->m_szSEARCHKEYArr[0] );
	}
	_tcscpy( m_pcEditDoc->m_cDlgGrep.m_szText, cmemCurText.GetStringPtr() );

	/* Grepダイアログの表示 */
	int nRet = m_pcEditDoc->m_cDlgGrep.DoModal( m_hInstance, m_hWnd, m_pcEditDoc->GetFilePath() );
//	MYTRACE_A( "nRet=%d\n", nRet );
	if( !nRet ){
		return;
	}
	HandleCommand(F_GREP, TRUE, 0, 0, 0, 0);	//	GREPコマンドの発行
}

/*! GREP実行

	@date 2005.01.10 genta CEditView_Commandより移動
*/
void CEditView::Command_GREP( void )
{
	CMemory		cmWork1;
	CMemory		cmWork2;
	CMemory		cmWork3;
	CMemory		cmemCurText;

	/* 編集ウィンドウの上限チェック */
	if( m_pShareData->m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
		::MYMESSAGEBOX( m_hWnd, MB_OK, GSTR_APPNAME, _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );

		return;
	}
	cmWork1.SetString( m_pcEditDoc->m_cDlgGrep.m_szText );
	cmWork2.SetString( m_pcEditDoc->m_cDlgGrep.m_szFile );
	cmWork3.SetString( m_pcEditDoc->m_cDlgGrep.m_szFolder );

	/*	今のEditViewにGrep結果を表示する。
		Grepモードのとき。または、変更フラグがオフで、ファイルを読み込んでいない場合。
		Grep中とアウトプットウィンドウも除外する
	*/
	if( (  m_pcEditDoc->m_bGrepMode &&
		   !m_pcEditDoc->m_bGrepRunning ) ||
	    ( !m_pcEditDoc->m_bGrepMode &&
		   !m_pcEditDoc->IsModified() &&
		   !m_pcEditDoc->IsFilePathAvailable() &&		/* 現在編集中のファイルのパス */
		   !m_pcEditDoc->m_bDebugMode
		)
	){
		DoGrep(
			&cmWork1,
			&cmWork2,
			&cmWork3,
			m_pcEditDoc->m_cDlgGrep.m_bSubFolder,
			m_pcEditDoc->m_cDlgGrep.m_sSearchOption,
			m_pcEditDoc->m_cDlgGrep.m_nGrepCharSet,
			m_pcEditDoc->m_cDlgGrep.m_bGrepOutputLine,
			m_pcEditDoc->m_cDlgGrep.m_nGrepOutputStyle
		);
	}
	else{
		/*======= Grepの実行 =============*/
		/* Grep結果ウィンドウの表示 */
		char*	pCmdLine = new char[1024];
		char*	pOpt = new char[64];
		cmWork1.Replace( "\"", "\"\"" );
		cmWork2.Replace( _T("\""), _T("\"\"") );
		cmWork3.Replace( _T("\""), _T("\"\"") );

		// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		wsprintf( pCmdLine,
			_T("-GREPMODE -GKEY=\"%s\" -GFILE=\"%s\" -GFOLDER=\"%s\" -GCODE=%d"),
			cmWork1.GetStringPtr(),
			cmWork2.GetStringPtr(),
			cmWork3.GetStringPtr(),
			m_pcEditDoc->m_cDlgGrep.m_nGrepCharSet
		);

		//GOPTオプション
		pOpt[0] = _T('\0');
		if( m_pcEditDoc->m_cDlgGrep.m_bSubFolder				)_tcscat( pOpt, _T("S") );	// サブフォルダからも検索する
		if( m_pcEditDoc->m_cDlgGrep.m_sSearchOption.bWordOnly	)_tcscat( pOpt, _T("W") );	// 単語単位で探す
		if( m_pcEditDoc->m_cDlgGrep.m_sSearchOption.bLoHiCase	)_tcscat( pOpt, _T("L") );	// 英大文字と英小文字を区別する
		if( m_pcEditDoc->m_cDlgGrep.m_sSearchOption.bRegularExp	)_tcscat( pOpt, _T("R") );	// 正規表現
		if( m_pcEditDoc->m_cDlgGrep.m_bGrepOutputLine			)_tcscat( pOpt, _T("P") );	// 行を出力するか該当部分だけ出力するか
		if( 1 == m_pcEditDoc->m_cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("1") );	// Grep: 出力形式
		if( 2 == m_pcEditDoc->m_cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("2") );	// Grep: 出力形式
		if( 0 < _tcslen( pOpt ) ){
			_tcscat( pCmdLine, _T(" -GOPT=") );
			_tcscat( pCmdLine, pOpt );
		}

//		MYTRACE_A( "pCmdLine=[%s]\n", pCmdLine );
		/* 新規編集ウィンドウの追加 ver 0 */
		CEditApp::OpenNewEditor( m_hInstance, m_hWnd, pCmdLine, 0, FALSE );
		delete [] pCmdLine;
		delete [] pOpt;
		/*======= Grepの実行 =============*/
		/* Grep結果ウィンドウの表示 */
	}
	return;
}
/*[EOF]*/
