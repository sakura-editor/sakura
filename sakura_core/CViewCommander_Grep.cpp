#include "stdafx.h"
#include "CViewCommander.h"
#include "sakura_rc.h"
#include "view/CEditView.h"
#include "CEditDoc.h"
#include "CControlTray.h"
#include "charcode.h"
#include "CEditWnd.h"

/*! GREPダイアログの表示

	@date 2005.01.10 genta CEditView_Commandより移動
	@author Yazaki
*/
void CViewCommander::Command_GREP_DIALOG( void )
{
	CNativeW	cmemCurText;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji ダイアログ専用関数に変更

	/* キーがないなら、履歴からとってくる */
	if( 0 == cmemCurText.GetStringLength() ){
//		cmemCurText.SetData( GetShareData()->m_aSearchKeys[0], lstrlen( GetShareData()->m_aSearchKeys[0] ) );
		cmemCurText.SetString( GetShareData()->m_aSearchKeys[0] );
	}
	wcscpy( GetEditWindow()->m_cDlgGrep.m_szText, cmemCurText.GetStringPtr() );

	/* Grepダイアログの表示 */
	int nRet = GetEditWindow()->m_cDlgGrep.DoModal( GetInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath() );
//	MYTRACE_A( "nRet=%d\n", nRet );
	if( FALSE == nRet ){
		return;
	}
	HandleCommand(F_GREP, TRUE, 0, 0, 0, 0);	//	GREPコマンドの発行
}

/*! GREP実行

	@date 2005.01.10 genta CEditView_Commandより移動
*/
void CViewCommander::Command_GREP( void )
{
	CNativeW		cmWork1;
	CNativeT		cmWork2;
	CNativeT		cmWork3;

	/* 編集ウィンドウの上限チェック */
	if( GetShareData()->m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
		TCHAR szMsg[512];
		auto_sprintf( szMsg, _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
		::MessageBox( m_pCommanderView->GetHwnd(), szMsg, GSTR_APPNAME, MB_OK );
		return;
	}
	cmWork1.SetString( GetEditWindow()->m_cDlgGrep.m_szText );
	cmWork2.SetString( GetEditWindow()->m_cDlgGrep.m_szFile );
	cmWork3.SetString( GetEditWindow()->m_cDlgGrep.m_szFolder );

	/*	今のEditViewにGrep結果を表示する。
		Grepモードのとき。または、変更フラグがオフで、ファイルを読み込んでいない場合。
	*/
	if( CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode ||
		( !GetDocument()->m_cDocEditor.IsModified() &&
		  !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() )		/* 現在編集中のファイルのパス */
	){
		CEditApp::Instance()->m_pcGrepAgent->DoGrep(
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
		/*======= Grepの実行 =============*/
		/* Grep結果ウィンドウの表示 */
		cmWork1.Replace( L"\"", L"\"\"" );
		cmWork2.Replace( _T("\""), _T("\"\"") );
		cmWork3.Replace( _T("\""), _T("\"\"") );

		// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		CCommandLineString cCmdLine;
		cCmdLine.AppendF(
			_T("-GREPMODE -GKEY=\"%ls\" -GFILE=\"%ls\" -GFOLDER=\"%ts\" -GCODE=%d"),
			cmWork1.GetStringPtr(),
			cmWork2.GetStringPtr(),
			cmWork3.GetStringPtr(),
			GetEditWindow()->m_cDlgGrep.m_nGrepCharSet
		);

		//GOPTオプション
		TCHAR	pOpt[64];
		pOpt[0] = _T('\0');
		if( GetEditWindow()->m_cDlgGrep.m_bSubFolder				)_tcscat( pOpt, _T("S") );	// サブフォルダからも検索する
		if( GetEditWindow()->m_cDlgGrep.m_sSearchOption.bWordOnly	)_tcscat( pOpt, _T("W") );	// 単語単位で探す
		if( GetEditWindow()->m_cDlgGrep.m_sSearchOption.bLoHiCase	)_tcscat( pOpt, _T("L") );	// 英大文字と英小文字を区別する
		if( GetEditWindow()->m_cDlgGrep.m_sSearchOption.bRegularExp	)_tcscat( pOpt, _T("R") );	// 正規表現
		if( GetEditWindow()->m_cDlgGrep.m_bGrepOutputLine			)_tcscat( pOpt, _T("P") );	// 行を出力するか該当部分だけ出力するか
		if( 1 == GetEditWindow()->m_cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("1") );	// Grep: 出力形式
		if( 2 == GetEditWindow()->m_cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("2") );	// Grep: 出力形式
		if( 0 < _tcslen( pOpt ) ){
			cCmdLine.AppendF( _T(" -GOPT="), pOpt );
		}

//		MYTRACE_A( "pCmdLine=[%ls]\n", pCmdLine );
		/* 新規編集ウィンドウの追加 ver 0 */
		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = _T("");
		sLoadInfo.eCharCode = CODE_DEFAULT;
		sLoadInfo.bViewMode = false;
		CControlTray::OpenNewEditor( GetInstance(), m_pCommanderView->GetHwnd(), sLoadInfo, cCmdLine.c_str() );
		/*======= Grepの実行 =============*/
		/* Grep結果ウィンドウの表示 */
	}
	return;
}
/*[EOF]*/
