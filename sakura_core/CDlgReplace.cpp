//	$Id$
/************************************************************************

	CDlgReplace.cpp
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/
//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgReplace.h"
#include "CJre.h"
#include "debug.h"
#include "CEditView.h"
#include "etc_uty.h"
#include "global.h"
#include "CWaitCursor.h"


CDlgReplace::CDlgReplace()
{
	m_bLoHiCase = FALSE;	/* 英大文字と英小文字を区別する */
	m_bWordOnly = FALSE;	/* 一致する単語のみ検索する */
	m_bRegularExp = FALSE;	/* 正規表現 */
	m_bSelectedArea = FALSE;	/* 選択範囲内置換 */
	m_szText[0] = '\0';		/* 検索文字列 */
	m_szText2[0] = '\0';		/* 置換後文字列 */
	return;
}

/* モーダルダイアログの表示 */
HWND CDlgReplace::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam, BOOL bSelected )
{
	m_bRegularExp = m_pShareData->m_Common.m_bRegularExp;	/* 1==正規表現 */
	m_bLoHiCase = m_pShareData->m_Common.m_bLoHiCase;		/* 1==英大文字小文字の区別 */
	m_bWordOnly = m_pShareData->m_Common.m_bWordOnly;		/* 1==単語のみ検索 */
	m_bSelectedArea = m_pShareData->m_Common.m_bSelectedArea;	/* 選択範囲内置換 */
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_bNOTIFYNOTFOUND;	/* 検索／置換  見つからないときメッセージを表示 */
	m_bSelected = bSelected;
	return CDialog::DoModeless( hInstance, hwndParent, IDD_REPLACE, lParam, SW_SHOW );
}


/* モードレス時：置換・検索対象となるビューの変更 */
void CDlgReplace::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}




/* ダイアログデータの設定 */
void CDlgReplace::SetData( void )
{
	int		i;
	HWND	hwndCombo;
//	m_hWnd = hwndDlg;	/* このダイアログのハンドル */

	/* 検索文字列 */
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT );
	for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[i] );
	}

	/* 置換後文字列 */
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT2, m_szText2 );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 );
	for( i = 0; i < m_pShareData->m_nREPLACEKEYArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szREPLACEKEYArr[i] );
	}

	/* 英大文字と英小文字を区別する */
	::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, m_bLoHiCase );
//	/* 一致する単語のみ検索する */
//	::CheckDlgButton( m_hWnd, IDC_CHK_WORDONLY, m_bWordOnly );
	/* 正規表現 */
	::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, m_bRegularExp );

	/* 選択範囲内置換 */
	if( m_pShareData->m_Common.m_bSelectedArea ){
		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, TRUE );
	}else{
		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, TRUE );
	}

	if( m_bRegularExp ){
		/* CJreクラスの初期化 */
		CJre	cJre;
		cJre.Init();
		if( FALSE == cJre.IsExist() ){
			::MessageBeep( MB_ICONEXCLAMATION );
			::MessageBox( m_hWnd, "jre32.dllが見つかりません。\n正規表現を利用するには、jre32.dllが必要です。\n", "情報", MB_OK | MB_ICONEXCLAMATION );
			::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
		}else{
			/* 英大文字と英小文字を区別する */
			::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );
		}
	}
	/* 検索／置換  見つからないときメッセージを表示 */
	::CheckDlgButton( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND );
	
	
	/* 置換 ダイアログを自動的に閉じる */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgReplace, m_pShareData->m_Common.m_bAutoCloseDlgReplace );
	
	return;
}




/* ダイアログデータの取得 */
/* 　0==条件未入力 　0より大きい==正常　 0より小さい==入力エラー  */
int CDlgReplace::GetData( void )
{
	int			i;
	int			j;
	CMemory*	pcmWork;
	CJre	cJre;

	/* 置換 ダイアログを自動的に閉じる */
	m_pShareData->m_Common.m_bAutoCloseDlgReplace = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgReplace );


	/* 英大文字と英小文字を区別する */
	m_bLoHiCase = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_LOHICASE );
//	/* 一致する単語のみ検索する */
//	m_bWordOnly = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_WORDONLY );
	/* 正規表現 */
	m_bRegularExp = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP );
	/* 選択範囲内置換 */
	m_bSelectedArea = ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_SELECTEDAREA );
	/* 検索／置換  見つからないときメッセージを表示 */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND );

	m_pShareData->m_Common.m_bRegularExp = m_bRegularExp;		/* 1==正規表現 */
	m_pShareData->m_Common.m_bLoHiCase = m_bLoHiCase;			/* 1==英大文字小文字の区別 */
	m_pShareData->m_Common.m_bWordOnly = m_bWordOnly;			/* 1==単語のみ検索 */
	m_pShareData->m_Common.m_bSelectedArea = m_bSelectedArea;	/* 選択範囲内置換 */
	m_pShareData->m_Common.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	/* 検索／置換  見つからないときメッセージを表示 */


	/* 検索文字列 */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText, _MAX_PATH - 1 );
	/* 置換後文字列 */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT2, m_szText2, _MAX_PATH - 1 );



	if( 0 < lstrlen( m_szText ) ){
		/* 正規表現？ */
		if( m_bRegularExp ){
			/* CJreクラスの初期化 */
			cJre.Init();
			/* jre32.dllの存在チェック */
			if( FALSE == cJre.IsExist() ){
				::MessageBox( m_hWnd, "jre32.dllが見つかりません。\n正規表現を利用するには、jre32.dllが必要です。\n", "情報", MB_OK | MB_ICONEXCLAMATION );
				return -1;
			}
			/* 検索パターンのコンパイル */
			if( !cJre.Compile( m_szText ) ){
				char	szMsg[512];
				cJre.GetJreMessage( GJM_JPN, szMsg );
				::MessageBox( m_hWnd, szMsg, "正規表現エラー情報", MB_OK | MB_ICONSTOP );
				return -1;
			}
		}

		/* 検索文字列 */
		pcmWork = new CMemory( m_szText, lstrlen( m_szText ) );
		for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
			if( 0 == strcmp( m_szText, m_pShareData->m_szSEARCHKEYArr[i] ) ){
				break;
			}
		}
		if( i < m_pShareData->m_nSEARCHKEYArrNum ){
			for( j = i; j > 0; j-- ){
				strcpy( m_pShareData->m_szSEARCHKEYArr[j], m_pShareData->m_szSEARCHKEYArr[j - 1] );
			}
		}else{
			for( j = MAX_SEARCHKEY - 1; j > 0; j-- ){
				strcpy( m_pShareData->m_szSEARCHKEYArr[j], m_pShareData->m_szSEARCHKEYArr[j - 1] );
			}
			++m_pShareData->m_nSEARCHKEYArrNum;
			if( m_pShareData->m_nSEARCHKEYArrNum > MAX_SEARCHKEY ){
				m_pShareData->m_nSEARCHKEYArrNum = MAX_SEARCHKEY;
			}
		}
		strcpy( m_pShareData->m_szSEARCHKEYArr[0], pcmWork->GetPtr( NULL ) );
		delete pcmWork;



		/* 置換後文字列 */
		pcmWork = new CMemory( m_szText2, lstrlen( m_szText2 ) );
		for( i = 0; i < m_pShareData->m_nREPLACEKEYArrNum; ++i ){
			if( 0 == strcmp( m_szText2, m_pShareData->m_szREPLACEKEYArr[i] ) ){
				break;
			}
		}
		if( i < m_pShareData->m_nREPLACEKEYArrNum ){
			for( j = i; j > 0; j-- ){
				strcpy( m_pShareData->m_szREPLACEKEYArr[j], m_pShareData->m_szREPLACEKEYArr[j - 1] );
			}
		}else{
			for( j = MAX_REPLACEKEY - 1; j > 0; j-- ){
				strcpy( m_pShareData->m_szREPLACEKEYArr[j], m_pShareData->m_szREPLACEKEYArr[j - 1] );
			}
			++m_pShareData->m_nREPLACEKEYArrNum;
			if( m_pShareData->m_nREPLACEKEYArrNum > MAX_REPLACEKEY ){
				m_pShareData->m_nREPLACEKEYArrNum = MAX_REPLACEKEY;
			}
		}
		strcpy( m_pShareData->m_szREPLACEKEYArr[0], pcmWork->GetPtr( NULL ) );
		delete pcmWork;


		return 1;
	}else{
		return 0;
	}
}


BOOL CDlgReplace::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;
	if( CJre::IsExist() ){	// jre.dllがあるかどうかを判定
		CJre	cJre;
		char	szMsg[256];
		WORD	wJreVersion;
		cJre.Init();
		/* JRE32.DLLのバージョン */
		wJreVersion = cJre.GetVersion();
		wsprintf( szMsg, "jre32.dll Ver%x.%x", wJreVersion / 0x100, wJreVersion % 0x100 );
		::SetDlgItemText( hwndDlg, IDC_STATIC_JRE32VER, szMsg );
	}

	/* ユーザーがコンボ ボックスのエディット コントロールに入力できるテキストの長さを制限する */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );

	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );

	
	/* テキスト選択中か */
	if( m_bSelected ){
//		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHPREV ), FALSE );
//		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHNEXT ), FALSE );
//		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_REPALCE ), FALSE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, TRUE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_SELECTEDAREA ), FALSE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, FALSE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, TRUE );
	}
	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );

}









BOOL CDlgReplace::OnBnClicked( int wID )
{
	CEditView*	pcEditView = (CEditView*)m_lParam;
	int			nNewPos;
	int			nReplaceNum;
	char		szLabel[64];
	int			nAllLineNum;
	HWND		hwndProgress;
	HWND		hwndStatic;
//	MSG			msg;
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「置換」のヘルプ */
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, 62 );
		return TRUE;
//	case IDC_CHK_LOHICASE:	/* 大文字と小文字を区別する */
//		MYTRACE( "IDC_CHK_LOHICASE\n" );
//		return TRUE;
//	case IDC_CHK_WORDONLY:	/* 一致する単語のみ検索 */
//		MYTRACE( "IDC_CHK_WORDONLY\n" );
//		break;
	case IDC_CHK_REGULAREXP:	/* 正規表現 */
//		MYTRACE( "IDC_CHK_REGULAREXP ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) = %d\n", ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) ){
			/* CJreクラスの初期化 */
			CJre	cJre;
			cJre.Init();
			if( FALSE == CJre::IsExist() ){
				/* JRE32.DLLのバージョン */
				::SetDlgItemText( m_hWnd, IDC_STATIC_JRE32VER, "" );

				::MessageBeep( MB_ICONEXCLAMATION );
				::MessageBox( m_hWnd, "jre32.dllが見つかりません。\n正規表現を利用するには、jre32.dllが必要です。\n", "情報", MB_OK | MB_ICONEXCLAMATION );
				::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );

			}else{
				CJre	cJre;
				char	szMsg[256];
				WORD	wJreVersion;
				cJre.Init();
				/* JRE32.DLLのバージョン */
				wJreVersion = cJre.GetVersion();
				wsprintf( szMsg, "jre32.dll Ver%x.%x", wJreVersion / 0x100, wJreVersion % 0x100 );
				::SetDlgItemText( m_hWnd, IDC_STATIC_JRE32VER, szMsg );

				/* 英大文字と英小文字を区別する */
				::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );
			}
		}else{
			/* 英大文字と英小文字を区別する */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
			::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 0 );
		}
		return TRUE;;
//	case IDOK:			/* 下検索 */
//		/* ダイアログデータの取得 */
//		nRet = GetData();
//		if( 0 < nRet ){
//			::EndDialog( hwndDlg, 2 );
//		}else
//		if( 0 == nRet ){
//			::EndDialog( hwndDlg, 0 );
//		}
//		return TRUE;


	case IDC_BUTTON_SEARCHPREV:
		if( 0 < GetData() ){
			/* コマンドコードによる処理振り分け */
			/* 前を検索 */
			pcEditView->HandleCommand( F_SEARCH_PREV, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
			/* 再描画 */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			::MYMESSAGEBOX(	m_hWnd,	MB_OK , GSTR_APPNAME,
				"文字列を指定してください。"
			);
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:
		if( 0 < GetData() ){
			/* コマンドコードによる処理振り分け */
			/* 次を検索 */
			pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
			/* 再描画 */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			::MYMESSAGEBOX(	m_hWnd,	MB_OK , GSTR_APPNAME,
				"文字列を指定してください。"
			);
		}		
		return TRUE;

	case IDC_BUTTON_REPALCE:	/* 置換 */
		if( 0 < GetData() ){
			/* カーソル左移動 */
			pcEditView->HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );
			
			/* テキスト選択解除 */
			/* 現在の選択範囲を非選択状態に戻す */
			pcEditView->DisableSelectArea( TRUE );

			/* 次を検索 */
			pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );

			/* テキストが選択されているか */
			if( pcEditView->IsTextSelected() ){
				/* コマンドコードによる処理振り分け */
				/* テキストを貼り付け */
				pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)m_szText2, FALSE, 0, 0 );
				/* 次を検索 */
				pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, (LPARAM)"最後まで置換しました。", 0, 0 );
			}
			/* 再描画 */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			::MYMESSAGEBOX(	m_hWnd,	MB_OK , GSTR_APPNAME,
				"文字列を指定してください。"
			);
		}
		return TRUE;
	case IDC_BUTTON_REPALCEALL:
		if( 0 < GetData() ){

			
			/* 表示処理ON/OFF */			
			BOOL bDisplayUpdate = FALSE;


			pcEditView->m_bDrawSWITCH = bDisplayUpdate;
			CDlgCancel	cDlgCancel;
			HWND		hwndCancel;
			nAllLineNum = pcEditView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();

			/* 進捗表示&中止ダイアログの作成 */
			hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hWnd, IDD_REPLACERUNNING );
			::EnableWindow( m_hWnd, FALSE );
			::EnableWindow( ::GetParent( m_hWnd ), FALSE );
			::EnableWindow( ::GetParent( ::GetParent( m_hWnd ) ), FALSE );

			/* プログレスバー初期化 */
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS_REPLACE );
			::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
			nNewPos = 0;
 			::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );

			/* 置換個数初期化 */
			nReplaceNum = 0;
			hwndStatic = ::GetDlgItem( hwndCancel, IDC_STATIC_KENSUU );
//			wsprintf( szLabel, "%d", nReplaceNum );
			_itoa( nReplaceNum, szLabel, 10 );
			::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );
//			::SetWindowText( hwndStatic, szLabel );

			/* ファイルの先頭に移動 */
			pcEditView->HandleCommand( F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0 );
			/* テキスト選択解除 */
			/* 現在の選択範囲を非選択状態に戻す */
			pcEditView->DisableSelectArea( FALSE );
			/* 次を検索 */
			pcEditView->HandleCommand( F_SEARCH_NEXT, bDisplayUpdate, 0, 0, 0, 0 );
			/* テキストが選択されているか */
			while( pcEditView->IsTextSelected() ){
				/* キャンセルされたか */
//				if( cDlgCancel.IsCanceled() ){
				if( cDlgCancel.m_bCANCEL ){
					break;
				}
//				if( 0 == ( nReplaceNum % 8 ) ){
					/* 処理中のユーザー操作を可能にする */
					if( !::BlockingHook( hwndCancel ) ){
						return -1;
					}
//					if( ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ){
//						if ( msg.message == WM_QUIT ){
//							return -1;
//						}
//						if( !IsDialogMessage(hwndCancel, &msg)) {
//							::TranslateMessage(&msg);
//							::DispatchMessage(&msg);
//						}
//					}
//				}
				if( 0 == ( nReplaceNum % 8 ) ){

					
//					if( 0 < nAllLineNum ){
						nNewPos = (pcEditView->m_nSelectLineFrom * 100) / nAllLineNum;
						::PostMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
//					}
					_itoa( nReplaceNum, szLabel, 10 );
					::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)(const char*)szLabel );
				}
				
//#ifdef _DEBUG
//				{
//					CRunningTimer* pcRunningTimer = new CRunningTimer( (const char*)"F_INSTEXT" );
//					gm_ProfileOutput = FALSE;
//#endif
					/* コマンドコードによる処理振り分け */
					/* テキストを貼り付け */
					pcEditView->HandleCommand( F_INSTEXT, bDisplayUpdate, (LPARAM)m_szText2, TRUE, 0, 0 );
//#ifdef _DEBUG
//					gm_ProfileOutput = TRUE;
//					delete pcRunningTimer;
//					gm_ProfileOutput = FALSE;
//				}
//#endif
				++nReplaceNum;

//#ifdef _DEBUG
//				{
//					CRunningTimer* pcRunningTimer = new CRunningTimer( (const char*)"F_SEARCH_NEXT" );
//					gm_ProfileOutput = FALSE;
//#endif
				
					/* 次を検索 */
					pcEditView->HandleCommand( F_SEARCH_NEXT, bDisplayUpdate, 0, 0, 0, 0 );
//#ifdef _DEBUG
//					gm_ProfileOutput = TRUE;
//					delete pcRunningTimer;
//					gm_ProfileOutput = FALSE;
//				}
//#endif
			}
			if( 0 < nAllLineNum ){
				nNewPos = (pcEditView->m_nSelectLineFrom * 100) / nAllLineNum;
				::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
			}
			_itoa( nReplaceNum, szLabel, 10 );
			::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

			//再描画
			pcEditView->m_bDrawSWITCH = TRUE;
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
			

			if( !cDlgCancel.IsCanceled() ){
				nNewPos = 100;
				::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
			}
//			cDlgCancel.Close();	 
			cDlgCancel.CloseDialog( 0 );	 
			::EnableWindow( m_hWnd, TRUE );
			::EnableWindow( ::GetParent( m_hWnd ), TRUE );
			::EnableWindow( ::GetParent( ::GetParent( m_hWnd ) ), TRUE );

			
			/* 再描画 */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
			
			/* アクティブにする */
			ActivateFrameWindow( m_hWnd );

			::MYMESSAGEBOX(	m_hWnd,	MB_OK | MB_TOPMOST, GSTR_APPNAME,
				"%d箇所を置換しました。", nReplaceNum
			);

//			nNewPos = 100;
// 			::SendMessage( ::GetDlgItem( m_hWnd, IDC_PROGRESS_REPLACE ), PBM_SETPOS, nNewPos, 0 );

//			::ShowWindow( ::GetDlgItem( m_hWnd, IDC_PROGRESS_REPLACE ), SW_HIDE );

			if( !cDlgCancel.IsCanceled() ){
				if(	m_bModal ){		/* モーダル　ダイアログか */
					/* 置換ダイアログを閉じる */
					::EndDialog( m_hWnd, 0 );
				}else{
					/* 置換 ダイアログを自動的に閉じる */
					if( m_pShareData->m_Common.m_bAutoCloseDlgReplace ){
						::DestroyWindow( m_hWnd );
					}
				}
			}

			
			return TRUE;
		}else{
			::MYMESSAGEBOX(	m_hWnd,	MB_OK , GSTR_APPNAME,
				"置換条件を指定してください。"
			);
		}
		return TRUE;
//	case IDCANCEL:
//		::EndDialog( hwndDlg, 0 );
//		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}
/*[EOF]*/
