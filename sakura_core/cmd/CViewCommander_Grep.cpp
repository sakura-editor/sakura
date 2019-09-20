﻿/*!	@file
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
#include "CViewCommander_inline.h"

#include "_main/CControlTray.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "plugin/CPlugin.h"
#include "plugin/CJackManager.h"

/*! GREPダイアログの表示

	@date 2005.01.10 genta CEditView_Commandより移動
	@author Yazaki
*/
void CViewCommander::Command_GREP_DIALOG( void )
{
	CNativeW	cmemCurText;
	// 2014.07.01 複数Grepウィンドウを使い分けている場合などに影響しないように、未設定のときだけHistoryを見る
	bool bGetHistory = GetEditWindow()->m_cDlgGrep.m_bSetText == false;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	bool bSet = m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText, bGetHistory );	// 2006.08.23 ryoji ダイアログ専用関数に変更

	if( bSet ){
		GetEditWindow()->m_cDlgGrep.m_strText = cmemCurText.GetStringPtr();
		GetEditWindow()->m_cDlgGrep.m_bSetText = true;
	}

	/* Grepダイアログの表示 */
	int nRet = GetEditWindow()->m_cDlgGrep.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath() );
//	MYTRACE( L"nRet=%d\n", nRet );
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
	CNativeW		cmWork2;
	CNativeW		cmWork3;
	CNativeW		cmWork4;
	CNativeW		cmWorkExcludeFile;
	CNativeW		cmWorkExcludeFolder;
	cmWork1.SetString( GetEditWindow()->m_cDlgGrep.m_strText.c_str() );
	cmWork2.SetString( GetEditWindow()->m_cDlgGrep.m_szFile );
	cmWork3.SetString( GetEditWindow()->m_cDlgGrep.m_szFolder );
	cmWorkExcludeFile.SetString(GetEditWindow()->m_cDlgGrep.m_szExcludeFile);
	cmWorkExcludeFolder.SetString(GetEditWindow()->m_cDlgGrep.m_szExcludeFolder);

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
			CTypeConfig cTypeGrep = CDocTypeManager().GetDocumentTypeOfExt( L"grepout" );
			const STypeConfigMini* pConfig;
			CDocTypeManager().GetTypeConfigMini( cTypeGrep, &pConfig );
			GetDocument()->m_cDocType.SetDocumentTypeIdx( pConfig->m_id );
			GetDocument()->m_cDocType.LockDocumentType();
			GetDocument()->OnChangeType();
		}
		
		CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
			m_pCommanderView,
			false,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			&cmWorkExcludeFile,
			&cmWorkExcludeFolder,
			false,
			GetEditWindow()->m_cDlgGrep.m_bSubFolder,
			false,
			true, // Header
			GetEditWindow()->m_cDlgGrep.m_sSearchOption,
			GetEditWindow()->m_cDlgGrep.m_nGrepCharSet,
			GetEditWindow()->m_cDlgGrep.m_nGrepOutputLineType,
			GetEditWindow()->m_cDlgGrep.m_nGrepOutputStyle,
			GetEditWindow()->m_cDlgGrep.m_bGrepOutputFileOnly,
			GetEditWindow()->m_cDlgGrep.m_bGrepOutputBaseFolder,
			GetEditWindow()->m_cDlgGrep.m_bGrepSeparateFolder,
			false,
			false
		);

		//プラグイン：DocumentOpenイベント実行
		CJackManager::getInstance()->InvokePlugins( PP_DOCUMENT_OPEN, &GetEditWindow()->GetActiveView() );
	}
	else{
		// 編集ウィンドウの上限チェック
		if( GetDllShareData().m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
			OkMessage( m_pCommanderView->GetHwnd(), LS(STR_MAXWINDOW), MAX_EDITWINDOWS );
			return;
		}

		/*======= Grepの実行 =============*/
		/* Grep結果ウィンドウの表示 */
		CControlTray::DoGrepCreateWindow(G_AppInstance(), m_pCommanderView->GetHwnd(), GetEditWindow()->m_cDlgGrep);
	}
	return;
}

/*! GREP置換ダイアログの表示
*/
void CViewCommander::Command_GREP_REPLACE_DLG( void )
{
	CNativeW	cmemCurText;
	CDlgGrepReplace& cDlgGrepRep = GetEditWindow()->m_cDlgGrepReplace;

	// 複数Grepウィンドウを使い分けている場合などに影響しないように、未設定のときだけHistoryを見る
	bool bGetHistory = cDlgGrepRep.m_bSetText == false;

	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText, bGetHistory );

	if( 0 < cmemCurText.GetStringLength() ){
		cDlgGrepRep.m_strText = cmemCurText.GetStringPtr();
		cDlgGrepRep.m_bSetText = true;
	}
	if( 0 < GetDllShareData().m_sSearchKeywords.m_aReplaceKeys.size() ){
		if( cDlgGrepRep.m_nReplaceKeySequence < GetDllShareData().m_Common.m_sSearch.m_nReplaceKeySequence ){
			cDlgGrepRep.m_strText2 = GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0];
		}
	}

	int nRet = cDlgGrepRep.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath(), (LPARAM)m_pCommanderView );
	if( !nRet ){
		return;
	}
	HandleCommand(F_GREP_REPLACE, TRUE, 0, 0, 0, 0);	//	GREPコマンドの発行
}

/*! GREP置換実行
*/
void CViewCommander::Command_GREP_REPLACE( void )
{
	CNativeW		cmWork1;
	CNativeW		cmWork2;
	CNativeW		cmWork3;
	CNativeW		cmWork4;
	CNativeW		cmWorkExcludeFile;
	CNativeW		cmWorkExcludeFolder;

	CDlgGrepReplace& cDlgGrepRep = GetEditWindow()->m_cDlgGrepReplace;
	cmWork1.SetString( cDlgGrepRep.m_strText.c_str() );
	cmWork2.SetString( cDlgGrepRep.m_szFile );
	cmWork3.SetString( cDlgGrepRep.m_szFolder );
	cmWork4.SetString( cDlgGrepRep.m_strText2.c_str() );
	cmWorkExcludeFile.SetString(cDlgGrepRep.m_szExcludeFile);
	cmWorkExcludeFolder.SetString(cDlgGrepRep.m_szExcludeFolder);

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
		CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
			m_pCommanderView,
			true,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			&cmWorkExcludeFile,
			&cmWorkExcludeFolder,
			false,
			cDlgGrepRep.m_bSubFolder,
			false, // Stdout
			true, // Header
			cDlgGrepRep.m_sSearchOption,
			cDlgGrepRep.m_nGrepCharSet,
			cDlgGrepRep.m_nGrepOutputLineType,
			cDlgGrepRep.m_nGrepOutputStyle,
			cDlgGrepRep.m_bGrepOutputFileOnly,
			cDlgGrepRep.m_bGrepOutputBaseFolder,
			cDlgGrepRep.m_bGrepSeparateFolder,
			cDlgGrepRep.m_bPaste,
			cDlgGrepRep.m_bBackup
		);
	}
	else{
		// 編集ウィンドウの上限チェック
		if( GetDllShareData().m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
			OkMessage( m_pCommanderView->GetHwnd(), L"編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。", MAX_EDITWINDOWS );
			return;
		}
		/*======= Grepの実行 =============*/
		/* Grep結果ウィンドウの表示 */
		cmWork1.Replace( L"\"", L"\"\"" );
		cmWork2.Replace( L"\"", L"\"\"" );
		cmWork3.Replace( L"\"", L"\"\"" );
		cmWork4.Replace( L"\"", L"\"\"" );

		// -GREPMODE -GKEY="1" -GREPR="2" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		CNativeW cCmdLine;
		WCHAR szTemp[20];
		cCmdLine.AppendString(L"-GREPMODE -GKEY=\"");
		cCmdLine.AppendString(cmWork1.GetStringPtr());
		cCmdLine.AppendString(L"\" -GREPR=\"");
		cCmdLine.AppendString(cmWork4.GetStringPtr());
		cCmdLine.AppendString(L"\" -GFILE=\"");
		cCmdLine.AppendString(cmWork2.GetStringPtr());
		cCmdLine.AppendString(L"\" -GFOLDER=\"");
		cCmdLine.AppendString(cmWork3.GetStringPtr());
		cCmdLine.AppendString(L"\" -GCODE=");
		auto_sprintf( szTemp, L"%d", cDlgGrepRep.m_nGrepCharSet );
		cCmdLine.AppendString(szTemp);

		//GOPTオプション
		WCHAR	pOpt[64];
		pOpt[0] = L'\0';
		if( cDlgGrepRep.m_bSubFolder				)wcscat( pOpt, L"S" );	// サブフォルダからも検索する
		if( cDlgGrepRep.m_sSearchOption.bWordOnly	)wcscat( pOpt, L"W" );	// 単語単位で探す
		if( cDlgGrepRep.m_sSearchOption.bLoHiCase	)wcscat( pOpt, L"L" );	// 英大文字と英小文字を区別する
		if( cDlgGrepRep.m_sSearchOption.bRegularExp	)wcscat( pOpt, L"R" );	// 正規表現
		if( cDlgGrepRep.m_nGrepOutputLineType == 1     )wcscat( pOpt, L"P" );	// 行を出力する
		// if( cDlgGrepRep.m_nGrepOutputLineType == 2     )wcscat( pOpt, L"N" );	// 否ヒット行を出力する 2014.09.23
		if( 1 == cDlgGrepRep.m_nGrepOutputStyle		)wcscat( pOpt, L"1" );	// Grep: 出力形式
		if( 2 == cDlgGrepRep.m_nGrepOutputStyle		)wcscat( pOpt, L"2" );	// Grep: 出力形式
		if( 3 == cDlgGrepRep.m_nGrepOutputStyle		)wcscat( pOpt, L"3" );
		if( cDlgGrepRep.m_bGrepOutputFileOnly		)wcscat( pOpt, L"F" );
		if( cDlgGrepRep.m_bGrepOutputBaseFolder		)wcscat( pOpt, L"B" );
		if( cDlgGrepRep.m_bGrepSeparateFolder		)wcscat( pOpt, L"D" );
		if( cDlgGrepRep.m_bPaste					)wcscat( pOpt, L"C" );	// クリップボードから貼り付け
		if( cDlgGrepRep.m_bBackup					)wcscat( pOpt, L"O" );	// バックアップ作成
		if( 0 < wcslen( pOpt ) ){
			cCmdLine.AppendString( L" -GOPT=" );
			cCmdLine.AppendString( pOpt );
		}

		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = L"";
		sLoadInfo.eCharCode = CODE_NONE;
		sLoadInfo.bViewMode = false;
		CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo, cCmdLine.GetStringPtr(),
			false, NULL, GetDllShareData().m_Common.m_sTabBar.m_bNewWindow? true : false );
	}
	return;
}

