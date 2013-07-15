/*!	@file
	@brief 検索ダイアログボックス

	@author Norio Nakatani
	@date	1998/12/12 再作成
	@date 2001/06/23 N.Nakatani 単語単位で検索する機能を実装
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, JEPRO, hor, Stonee
	Copyright (C) 2002, MIK, hor, YAZAKI, genta
	Copyright (C) 2005, zenryaku
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CDlgFind.h"
#include "Funccode.h"
//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
#include "CBregexp.h"
#include "CEditView.h"
#include "etc_uty.h"	//Stonee, 2001/03/12
#include "Debug.h"// 2002/2/10 aroka ヘッダ整理
#include "sakura_rc.h"
#include "sakura.hh"

//検索 CDlgFind.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//11800
	IDC_BUTTON_SEARCHNEXT,			HIDC_FIND_BUTTON_SEARCHNEXT,		//次を検索
	IDC_BUTTON_SEARCHPREV,			HIDC_FIND_BUTTON_SEARCHPREV,		//前を検索
	IDCANCEL,						HIDCANCEL_FIND,						//キャンセル
	IDC_BUTTON_HELP,				HIDC_FIND_BUTTON_HELP,				//ヘルプ
	IDC_CHK_WORD,					HIDC_FIND_CHK_WORD,					//単語単位
	IDC_CHK_LOHICASE,				HIDC_FIND_CHK_LOHICASE,				//大文字小文字
	IDC_CHK_REGULAREXP,				HIDC_FIND_CHK_REGULAREXP,			//正規表現
	IDC_CHECK_NOTIFYNOTFOUND,		HIDC_FIND_CHECK_NOTIFYNOTFOUND,		//見つからないときに通知
	IDC_CHECK_bAutoCloseDlgFind,	HIDC_FIND_CHECK_bAutoCloseDlgFind,	//自動的に閉じる
	IDC_COMBO_TEXT,					HIDC_FIND_COMBO_TEXT,				//検索文字列
	IDC_STATIC_JRE32VER,			HIDC_FIND_STATIC_JRE32VER,			//正規表現バージョン
	IDC_BUTTON_SETMARK,				HIDC_FIND_BUTTON_SETMARK,			//2002.01.16 hor 検索該当行をマーク
	IDC_CHECK_SEARCHALL,			HIDC_FIND_CHECK_SEARCHALL,			//2002.01.26 hor 先頭（末尾）から再検索
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgFind::CDlgFind()
{
	m_sSearchOption.Reset();
	m_szText[0] = '\0';		/* 検索文字列 */
	return;
}


/*!
	標準以外のメッセージを捕捉する

	@date 2013.03.24 novice 新規作成
*/
INT_PTR CDlgFind::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_COMMAND:
		WORD wID = LOWORD(wParam);
		switch( wID ){
		case IDC_COMBO_TEXT:
			if ( HIWORD(wParam) == CBN_DROPDOWN ) {
				HWND hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT );
				if ( ::SendMessage(hwndCombo, CB_GETCOUNT, 0L, 0L) == 0) {
					int i;
					for (i = 0; i < m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum; ++i) {
						::SendMessage( hwndCombo, CB_ADDSTRING, 0L, (LPARAM)m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[i] );
					}
				}
			}
			break;
		}
		break;
	}
	return result;
}

/* モードレスダイアログの表示 */
HWND CDlgFind::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	m_sSearchOption = m_pShareData->m_Common.m_sSearch.m_sSearchOption;		// 検索オプション
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND;	// 検索／置換  見つからないときメッセージを表示
	m_ptEscCaretPos_PHY.x = ((CEditView*)lParam)->m_ptCaretPos_PHY.x;	// 検索開始時のカーソル位置退避	02/07/28 ai
	m_ptEscCaretPos_PHY.y = ((CEditView*)lParam)->m_ptCaretPos_PHY.y;	// 検索開始時のカーソル位置退避	02/07/28 ai
	((CEditView*)lParam)->m_bSearch = TRUE;							// 検索開始位置の登録有無		02/07/28 ai
	return CDialog::DoModeless( hInstance, hwndParent, IDD_FIND, lParam, SW_SHOW );
}

/* モードレス時：検索対象となるビューの変更 */
void CDlgFind::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}



/* ダイアログデータの設定 */
void CDlgFind::SetData( void )
{
//	MYTRACE( _T("CDlgFind::SetData()") );

	/*****************************
	*           初期化           *
	*****************************/
	// Here Jun. 26, 2001 genta
	// 正規表現ライブラリの差し替えに伴う処理の見直しによりjre.dll判定を削除

	/* ユーザーがコンボ ボックスのエディット コントロールに入力できるテキストの長さを制限する */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );


	/*****************************
	*         データ設定         *
	*****************************/
	/* 検索文字列 */
	// 検索文字列リストの設定(関数化)	2010/5/28 Uchi
	SetCombosList();

	/* 英大文字と英小文字を区別する */
	::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, m_sSearchOption.bLoHiCase );

	// 2001/06/23 Norio Nakatani
	/* 単語単位で検索 */
	::CheckDlgButton( m_hWnd, IDC_CHK_WORD, m_sSearchOption.bWordOnly );

	/* 検索／置換  見つからないときメッセージを表示 */
	::CheckDlgButton( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND );

	// From Here Jun. 29, 2001 genta
	// 正規表現ライブラリの差し替えに伴う処理の見直し
	// 処理フロー及び判定条件の見直し。必ず正規表現のチェックと
	// 無関係にCheckRegexpVersionを通過するようにした。
	if( CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, false )
		&& m_sSearchOption.bRegularExp){
		/* 英大文字と英小文字を区別する */
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 1 );
//正規表現がONでも、大文字小文字を区別する／しないを選択できるように。
//		::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
//		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

		// 2001/06/23 N.Nakatani
		/* 単語単位で探す */
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );
	}
	else {
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
	}
	// To Here Jun. 29, 2001 genta

	/* 検索ダイアログを自動的に閉じる */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFind, m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind );

	/* 先頭（末尾）から再検索 2002.01.26 hor */
	::CheckDlgButton( m_hWnd, IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_sSearch.m_bSearchAll );

	return;
}


// 検索文字列リストの設定
//	2010/5/28 Uchi
void CDlgFind::SetCombosList( void )
{
	HWND	hwndCombo;
	TCHAR	szBuff[_MAX_PATH+1];

	/* 検索文字列 */
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT );
	while (::SendMessage(hwndCombo, CB_GETCOUNT, 0L, 0L) > 0) {
		::SendMessage(hwndCombo, CB_DELETESTRING, 0L, 0L);
	}
	::GetWindowText( hwndCombo, szBuff, _MAX_PATH );
	if (_tcscmp( szBuff, m_szText ) != 0) {
		SetDlgItemText(m_hWnd, IDC_COMBO_TEXT, m_szText);
	}
}


/* ダイアログデータの取得 */
int CDlgFind::GetData( void )
{
//	MYTRACE( _T("CDlgFind::GetData()") );

	/* 英大文字と英小文字を区別する */
	m_sSearchOption.bLoHiCase = (0!=IsDlgButtonChecked( m_hWnd, IDC_CHK_LOHICASE ));

	// 2001/06/23 Norio Nakatani
	/* 単語単位で検索 */
	m_sSearchOption.bWordOnly = (0!=IsDlgButtonChecked( m_hWnd, IDC_CHK_WORD ));

	/* 一致する単語のみ検索する */
	/* 正規表現 */
	m_sSearchOption.bRegularExp = (0!=IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ));

	/* 検索／置換  見つからないときメッセージを表示 */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND );

	m_pShareData->m_Common.m_sSearch.m_sSearchOption = m_sSearchOption;		// 検索オプション
	m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	// 検索／置換  見つからないときメッセージを表示

	/* 検索文字列 */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText, _countof( m_szText ));

	/* 検索ダイアログを自動的に閉じる */
	m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgFind );

	/* 先頭（末尾）から再検索 2002.01.26 hor */
	m_pShareData->m_Common.m_sSearch.m_bSearchAll = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_SEARCHALL );

	if( m_szText[0] != '\0' ){
		/* 正規表現？ */
		// From Here Jun. 26, 2001 genta
		//	正規表現ライブラリの差し替えに伴う処理の見直し
		int nFlag = 0x00;
		nFlag |= m_sSearchOption.bLoHiCase ? 0x01 : 0x00;
		if( m_sSearchOption.bRegularExp && !CheckRegexpSyntax( m_szText, m_hWnd, true, nFlag ) ){
			return -1;
		}
		// To Here Jun. 26, 2001 genta 正規表現ライブラリ差し替え

		/* 検索文字列 */
		//@@@ 2002.2.2 YAZAKI CShareDataに移動
		CShareData::getInstance()->AddToSearchKeyArr( (const char*)m_szText );
		if( !m_bModal ){
			/* ダイアログデータの設定 */
			//SetData();
			SetCombosList();		//	コンボのみの初期化	2010/5/28 Uchi
		}
		return 1;
	}else{
		return 0;
	}
}



BOOL CDlgFind::OnBnClicked( int wID )
{
	int			nRet;
	CEditView*	pcEditView = (CEditView*)m_lParam;
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「検索」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp( m_hWnd, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_SEARCH_DIALOG) );	//Apr. 5, 2001 JEPRO 修正漏れを追加	// 2006.10.10 ryoji MyWinHelpに変更に変更
		break;
	case IDC_CHK_REGULAREXP:	/* 正規表現 */
//		MYTRACE( _T("IDC_CHK_REGULAREXP ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) = %d\n"), ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) ){

			// From Here Jun. 26, 2001 genta
			//	正規表現ライブラリの差し替えに伴う処理の見直し
			if( !CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
			}else{
			// To Here Jun. 26, 2001 genta

				/* 英大文字と英小文字を区別する */
				//	Jan. 31, 2002 genta
				//	大文字・小文字の区別は正規表現の設定に関わらず保存する
				//::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
				//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

				// 2001/06/23 Norio Nakatani
				/* 単語単位で検索 */
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );
			}
		}else{
			/* 英大文字と英小文字を区別する */
			//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
			//	Jan. 31, 2002 genta
			//	大文字・小文字の区別は正規表現の設定に関わらず保存する
			//::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 0 );

			// 2001/06/23 Norio Nakatani
			/* 単語単位で検索 */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), TRUE );
		}
		break;
	case IDC_BUTTON_SEARCHPREV:	/* 上検索 */	//Feb. 13, 2001 JEPRO ボタン名を[IDC_BUTTON1]→[IDC_BUTTON_SERACHPREV]に変更
		/* ダイアログデータの取得 */
		nRet = GetData();
		if( 0 < nRet ){
			if( m_bModal ){		/* モーダルダイアログか */
				CloseDialog( 1 );
			}else{
				/* 前を検索 */
				pcEditView->HandleCommand( F_SEARCH_PREV, true, (LPARAM)m_hWnd, 0, 0, 0 );

				/* 再描画 2005.04.06 zenryaku 0文字幅マッチでキャレットを表示するため */
				pcEditView->Redraw();	// 前回0文字幅マッチの消去にも必要	// HandleCommand(F_REDRAW) -> Redraw() 非マッチ時に「見つからなかった」ステータスバーメッセージを消さない

				// 02/06/26 ai Start
				// 検索開始位置を登録
				if( TRUE == pcEditView->m_bSearch ){
					// 検索開始時のカーソル位置登録条件変更 02/07/28 ai start
					pcEditView->m_ptSrchStartPos_PHY.x = m_ptEscCaretPos_PHY.x;
					pcEditView->m_ptSrchStartPos_PHY.y = m_ptEscCaretPos_PHY.y;
					pcEditView->m_bSearch = FALSE;
					// 02/07/28 ai end
				}//  02/06/26 ai End

				/* 検索ダイアログを自動的に閉じる */
				if( m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
			}
		}
		else{
			OkMessage( m_hWnd, _T("検索条件を指定してください。") );
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:		/* 下検索 */	//Feb. 13, 2001 JEPRO ボタン名を[IDOK]→[IDC_BUTTON_SERACHNEXT]に変更
		/* ダイアログデータの取得 */
		nRet = GetData();
		if( 0 < nRet ){
			if( m_bModal ){		/* モーダルダイアログか */
				CloseDialog( 2 );
			}
			else{
				/* 次を検索 */
				pcEditView->HandleCommand( F_SEARCH_NEXT, true, (LPARAM)m_hWnd, 0, 0, 0 );

				/* 再描画 2005.04.06 zenryaku 0文字幅マッチでキャレットを表示するため */
				pcEditView->Redraw();	// 前回0文字幅マッチの消去にも必要	// HandleCommand(F_REDRAW) -> Redraw() 非マッチ時に「見つからなかった」ステータスバーメッセージを消さない

				// 検索開始位置を登録
				if( TRUE == pcEditView->m_bSearch ){
					// 検索開始時のカーソル位置登録条件変更 02/07/28 ai start
					pcEditView->m_ptSrchStartPos_PHY.x = m_ptEscCaretPos_PHY.x;
					pcEditView->m_ptSrchStartPos_PHY.y = m_ptEscCaretPos_PHY.y;
					pcEditView->m_bSearch = FALSE;
				}

				/* 検索ダイアログを自動的に閉じる */
				if( m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
			}
		}
		else{
			OkMessage( m_hWnd, _T("検索条件を指定してください。") );
		}
		return TRUE;
	case IDC_BUTTON_SETMARK:	//2002.01.16 hor 該当行マーク
		if( 0 < GetData() ){
			if( m_bModal ){		/* モーダルダイアログか */
				CloseDialog( 2 );
			}else{
				pcEditView->HandleCommand( F_BOOKMARK_PATTERN, false, 0, 0, 0, 0 );
				/* 検索ダイアログを自動的に閉じる */
				if( m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
				else{
					::SendMessage(m_hWnd,WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDC_COMBO_TEXT ),TRUE);
				}
			}
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}

BOOL CDlgFind::OnActivate( WPARAM wParam, LPARAM lParam )
{
	// 0文字幅マッチ描画のON/OFF	// 2009.11.29 ryoji
	CEditView*	pcEditView = (CEditView*)m_lParam;
	if( pcEditView->IsTextSelected()
		&& pcEditView->m_nSelectLineFrom == pcEditView->m_nSelectLineTo
		&& pcEditView->m_nSelectColmFrom == pcEditView->m_nSelectColmTo
	){
		::InvalidateRect( pcEditView->m_hWnd, NULL, TRUE );	// アクティブ化／非アクティブ化が完了してから再描画
	}

	return CDialog::OnActivate( wParam, lParam );
}

//@@@ 2002.01.18 add start
LPVOID CDlgFind::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
