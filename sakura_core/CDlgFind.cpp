//	$Id$
/*!	@file
	@brief 検索ダイアログボックス

	@author Norio Nakatani
	@date	1998/12/12 再作成
	@date 2001/06/23 N.Nakatani 単語単位で検索する機能を実装
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, JEPRO, hor, Stonee
	Copyright (C) 2002, MIK, hor

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CDlgFind.h"
#include "funccode.h"
#include "sakura_rc.h"
//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
#include "CBregexp.h"
#include "CEditView.h"
#include "etc_uty.h"	//Stonee, 2001/03/12
#include "debug.h"// 2002/2/10 aroka ヘッダ整理

//検索 CDlgFind.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
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
//	MYTRACE( "CDlgFind::CDlgFind()\n" );
	m_bLoHiCase = FALSE;	/* 英大文字と英小文字を区別する */
	m_bWordOnly = FALSE;	/* 一致する単語のみ検索する */
	m_bRegularExp = FALSE;	/* 正規表現 */
	m_szText[0] = '\0';		/* 検索文字列 */
	return;
}



//	/* モーダルダイアログの表示 */
//	int CDlgFind::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
//	{
//		m_bRegularExp = m_pShareData->m_Common.m_bRegularExp;			/* 1==正規表現 */
//		m_bLoHiCase = m_pShareData->m_Common.m_bLoHiCase;				/* 1==英大文字小文字の区別 */
//		m_bWordOnly = m_pShareData->m_Common.m_bWordOnly;				/* 1==単語のみ検索 */
//		m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_bNOTIFYNOTFOUND;	/* 検索／置換  見つからないときメッセージを表示 */
//		return CDialog::DoModal( hInstance, hwndParent, IDD_FIND, lParam );
//	}


/* モードレスダイアログの表示 */
HWND CDlgFind::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	m_bRegularExp = m_pShareData->m_Common.m_bRegularExp;			/* 1==正規表現 */
	m_bLoHiCase = m_pShareData->m_Common.m_bLoHiCase;				/* 1==英大文字小文字の区別 */
	m_bWordOnly = m_pShareData->m_Common.m_bWordOnly;				/* 1==単語のみ検索 */
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_bNOTIFYNOTFOUND;	/* 検索／置換  見つからないときメッセージを表示 */
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
//	MYTRACE( "CDlgFind::SetData()" );
	int		i;
	HWND	hwndCombo;

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
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText );
	for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[i] );
	}
	/* 英大文字と英小文字を区別する */
	::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, m_bLoHiCase );

	// 2001/06/23 Norio Nakatani
	/* 単語単位で検索 */
	::CheckDlgButton( m_hWnd, IDC_CHK_WORD, m_bWordOnly );

	/* 検索／置換  見つからないときメッセージを表示 */
	::CheckDlgButton( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND );

	// From Here Jun. 29, 2001 genta
	// 正規表現ライブラリの差し替えに伴う処理の見直し
	// 処理フロー及び判定条件の見直し。必ず正規表現のチェックと
	// 無関係にCheckRegexpVersionを通過するようにした。
	if( CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, false )
		&& m_bRegularExp){
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
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFind, m_pShareData->m_Common.m_bAutoCloseDlgFind );

	/* 先頭（末尾）から再検索 2002.01.26 hor */
	::CheckDlgButton( m_hWnd, IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_bSearchAll );

	return;
}




/* ダイアログデータの取得 */
int CDlgFind::GetData( void )
{
//	MYTRACE( "CDlgFind::GetData()" );
//	int			i;
//	int			j;
//	CMemory*	pcmWork;
	//

	/* 英大文字と英小文字を区別する */
	m_bLoHiCase = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_LOHICASE );

	// 2001/06/23 Norio Nakatani
	/* 単語単位で検索 */
	m_bWordOnly = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_WORD );

	/* 一致する単語のみ検索する */
	/* 正規表現 */
	m_bRegularExp = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP );

	/* 検索／置換  見つからないときメッセージを表示 */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND );

	m_pShareData->m_Common.m_bRegularExp = m_bRegularExp;			/* 1==正規表現 */
	m_pShareData->m_Common.m_bLoHiCase = m_bLoHiCase;				/* 1==英大文字小文字の区別 */
	m_pShareData->m_Common.m_bWordOnly = m_bWordOnly;				/* 1==単語のみ検索 */
	m_pShareData->m_Common.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	/* 検索／置換  見つからないときメッセージを表示 */

	/* 検索文字列 */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText, _MAX_PATH - 1 );

	/* 検索ダイアログを自動的に閉じる */
	m_pShareData->m_Common.m_bAutoCloseDlgFind = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgFind );

	/* 先頭（末尾）から再検索 2002.01.26 hor */
	m_pShareData->m_Common.m_bSearchAll = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_SEARCHALL );

	if( 0 < lstrlen( m_szText ) ){
		/* 正規表現？ */
		// From Here Jun. 26, 2001 genta
		//	正規表現ライブラリの差し替えに伴う処理の見直し
		int nFlag = 0x00;
		nFlag |= m_bLoHiCase ? 0x01 : 0x00;
		if( m_bRegularExp && !CheckRegexpSyntax( m_szText, m_hWnd, true, nFlag ) ){
			return -1;
		}
		// To Here Jun. 26, 2001 genta 正規表現ライブラリ差し替え

		/* 検索文字列 */
		//@@@ 2002.2.2 YAZAKI CShareDataに移動
		CShareData::getInstance()->AddToSearchKeyArr( (const char*)m_szText );
		if( FALSE == m_bModal ){
			/* ダイアログデータの設定 */
			SetData();
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
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_SEARCH_DIALOG) );	//Apr. 5, 2001 JEPRO 修正漏れを追加
		break;
	case IDC_CHK_REGULAREXP:	/* 正規表現 */
//		MYTRACE( "IDC_CHK_REGULAREXP ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) = %d\n", ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) );
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
				pcEditView->HandleCommand( F_SEARCH_PREV, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
//				/* 再描画 */
//				pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
				/* 検索ダイアログを自動的に閉じる */
				if( m_pShareData->m_Common.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
			}
//From Here Feb. 20, 2001 JEPRO 「置換」ダイアログと同じように警告メッセージを表示するように変更
//		}else
//		if( 0 == nRet ){
//			::MessageBeep( MB_ICONHAND );
//			CloseDialog( 0 );
//		}
//		return TRUE;
//ここまでコメントアウトし、代わりに以下を追加
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"検索条件を指定してください。"
			);
		}
		return TRUE;
//To Here Feb. 20, 2001
	case IDC_BUTTON_SEARCHNEXT:		/* 下検索 */	//Feb. 13, 2001 JEPRO ボタン名を[IDOK]→[IDC_BUTTON_SERACHNEXT]に変更
		/* ダイアログデータの取得 */
		nRet = GetData();
		if( 0 < nRet ){
			if( m_bModal ){		/* モーダルダイアログか */
				CloseDialog( 2 );
			}else{
				/* 次を検索 */
				pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
//				/* 再描画 */
//				pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
				/* 検索ダイアログを自動的に閉じる */
				if( m_pShareData->m_Common.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
				// 2001.12.03 hor
				//	ダイアログを閉じないとき、IDC_COMBO_TEXT 上で Enter した場合に
				//	キャレットが表示されなくなるのを回避する
				else{
				::SendMessage(m_hWnd,WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDC_COMBO_TEXT ),TRUE);
				}
			}
//From Here Feb. 20, 2001 JEPRO 「置換」ダイアログと同じように警告メッセージを表示するように変更
//		}else
//		if( 0 == nRet ){
//			::MessageBeep( MB_ICONHAND );
//			CloseDialog( 0 );
//		}
//		return TRUE;
//ここまでコメントアウトし、代わりに以下を追加
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"検索条件を指定してください。"
			);
		}
		return TRUE;
//To Here Feb. 20, 2001
	case IDC_BUTTON_SETMARK:	//2002.01.16 hor 該当行マーク
		if( 0 < GetData() ){
			if( m_bModal ){		/* モーダルダイアログか */
				CloseDialog( 2 );
			}else{
				pcEditView->HandleCommand( F_BOOKMARK_PATTERN, FALSE, 0, 0, 0, 0 );
				/* 検索ダイアログを自動的に閉じる */
				if( m_pShareData->m_Common.m_bAutoCloseDlgFind ){
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



//@@@ 2002.01.18 add start
LPVOID CDlgFind::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
