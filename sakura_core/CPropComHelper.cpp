//	$Id$
/************************************************************************

	CPropComHelp.cpp

	共通設定：支援
	Copyright (C) 1998-2000, Norio Nakatani


************************************************************************/
#include "sakura_rc.h"
#include "CPropCommon.h"
#include "debug.h"
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "etc_uty.h"
#include "CDlgInput1.h"
#include "global.h"



/* p10 メッセージ処理 */
BOOL CPropCommon::DispatchEvent_p10(
    HWND	hwndDlg,	// handle to dialog box
    UINT	uMsg,	// message
    WPARAM	wParam,	// first message parameter
    LPARAM	lParam 	// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 p10 */
		SetData_p10( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		/* 入力補完　単語ファイル */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_HOKANFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );
		/* キーワードヘルプ　辞書ファイル */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		/* 外部ヘルプ１ */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHELP1 ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );
		/* 外部HTMLヘルプ */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHTMLHELP ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );


		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID         = LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl     = (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			/* ダイアログデータの取得 p10 */
			GetData_p10( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_HOKANFILE_REF:	/* 入力補完　単語ファイルの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					char*			pszMRU = NULL;;
					char*			pszOPENFOLDER = NULL;;
					char			szPath[_MAX_PATH + 1];
					strcpy( szPath, m_Common.m_szHokanFile );
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create( 
						m_hInstance, 
						hwndDlg, 
						"*.*", 
						m_Common.m_szHokanFile, 
						(const char **)&pszMRU, 
						(const char **)&pszOPENFOLDER
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Common.m_szHokanFile, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Common.m_szHokanFile );
					}
				}
				return TRUE;

			//	From Here Sept. 12, 2000 JEPRO	
			case IDC_CHECK_USEKEYWORDHELP:	/* キーワードヘルプ機能を使う時だけ辞書ファイル指定と参照ボタンをEnableにする */
				::CheckDlgButton( hwndDlg, IDC_CHECK_USEKEYWORDHELP, m_Common.m_bUseKeyWordHelp );
				if( BST_CHECKED == m_Common.m_bUseKeyWordHelp ){
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), TRUE );
				}else{
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), FALSE );
				}
				return TRUE;
			//	To Here Sept. 12, 2000
		
			case IDC_BUTTON_KEYWORDHELPFILE_REF:	/* キーワードヘルプ　辞書ファイルの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					char*			pszMRU = NULL;;
					char*			pszOPENFOLDER = NULL;;
					char			szPath[_MAX_PATH + 1];
					strcpy( szPath, m_Common.m_szKeyWordHelpFile );
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create( 
						m_hInstance, 
						hwndDlg, 
						"*.*", 
						m_Common.m_szKeyWordHelpFile, 
						(const char **)&pszMRU, 
						(const char **)&pszOPENFOLDER
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Common.m_szKeyWordHelpFile, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_KEYWORDHELPFILE, m_Common.m_szKeyWordHelpFile );
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENHELP1:	/* 外部ヘルプ１の「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					char*			pszMRU = NULL;;
					char*			pszOPENFOLDER = NULL;;
					char			szPath[_MAX_PATH + 1];
					strcpy( szPath, m_Common.m_szExtHelp1 );
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create( 
						m_hInstance, 
						hwndDlg, 
						"*.hlp", 
						m_Common.m_szExtHelp1, 
						(const char **)&pszMRU, 
						(const char **)&pszOPENFOLDER
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Common.m_szExtHelp1, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1 );
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENEXTHTMLHELP:	/* 外部HTMLヘルプの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					char*			pszMRU = NULL;;
					char*			pszOPENFOLDER = NULL;;
					char			szPath[_MAX_PATH + 1];
					strcpy( szPath, m_Common.m_szExtHtmlHelp );
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create( 
						m_hInstance, 
						hwndDlg, 
						"*.chm;*.col", 
						m_Common.m_szExtHtmlHelp, 
						(const char **)&pszMRU, 
						(const char **)&pszOPENFOLDER
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Common.m_szExtHtmlHelp, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp );
					}
				}
				return TRUE;
			}
		}
		break;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		case ???????:
//			return 0L;
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_HELPER );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( "p10 PSN_KILLACTIVE\n" );
				/* ダイアログデータの取得 p10 */
				GetData_p10( hwndDlg );
				return TRUE;
			}
			break;
//		}

//		MYTRACE( "pNMHDR->hwndFrom=%xh\n", pNMHDR->hwndFrom );
//		MYTRACE( "pNMHDR->idFrom  =%xh\n", pNMHDR->idFrom   );
//		MYTRACE( "pNMHDR->code    =%xh\n", pNMHDR->code     );
//		MYTRACE( "pMNUD->iPos    =%d\n", pMNUD->iPos      );
//		MYTRACE( "pMNUD->iDelta  =%d\n", pMNUD->iDelta    );
		break;
	}
	return FALSE;
}

/* ダイアログデータの設定 p10 */
void CPropCommon::SetData_p10( HWND hwndDlg )
{
	/*  入力補完機能を使用する */
	::CheckDlgButton( hwndDlg, IDC_CHECK_USEHOKAN, m_Common.m_bUseHokan );

	/* 入力補完機能：英大文字小文字を同一視する */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANLOHICASE, m_Common.m_bHokanLoHiCase );

	/* 入力補完 単語ファイル */
	::SetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Common.m_szHokanFile );

	/* キーワードヘルプを使用する  */
	::CheckDlgButton( hwndDlg, IDC_CHECK_USEKEYWORDHELP, m_Common.m_bUseKeyWordHelp );
//	From Here Sept. 12, 2000 JEPRO キーワードヘルプ機能を使う時だけ辞書ファイル指定と参照ボタンをEnableにする
	if( BST_CHECKED == m_Common.m_bUseKeyWordHelp ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), FALSE );
	}
//	To Here Sept. 12, 2000

	/* キーワードヘルプ　辞書ファイル */
	::SetDlgItemText( hwndDlg, IDC_EDIT_KEYWORDHELPFILE, m_Common.m_szKeyWordHelpFile );

	/* 外部ヘルプ１ */
	::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1 );

	/* 外部HTMLヘルプ */
	::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp );

	/* HtmlHelpビューアはひとつ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HTMLHELPISSINGLE, m_Common.m_bHtmlHelpIsSingle );

	/* 補完候補決定キー */
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN, m_Common.m_bHokanKey_RETURN );//VK_RETURN 補完決定キーが有効/無効
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_TAB, m_Common.m_bHokanKey_TAB );//VK_TAB    補完決定キーが有効/無効
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT, m_Common.m_bHokanKey_RIGHT );//VK_RIGHT  補完決定キーが有効/無効
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_SPACE, m_Common.m_bHokanKey_SPACE );//VK_SPACE  補完決定キーが有効/無効

	return;
}


/* ダイアログデータの取得 p10 */
int CPropCommon::GetData_p10( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_HELPER;

	/*  入力補完機能を使用する */
	m_Common.m_bUseHokan = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USEHOKAN );

	/* 入力補完機能：英大文字小文字を同一視する */
	m_Common.m_bHokanLoHiCase = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANLOHICASE );

	/* 入力補完 単語ファイル */
	::GetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Common.m_szHokanFile, MAX_PATH - 1 );

	
	/* キーワードヘルプを使用する  */
	m_Common.m_bUseKeyWordHelp = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USEKEYWORDHELP );
	
	/* キーワードヘルプ　辞書ファイル */
	::GetDlgItemText( hwndDlg, IDC_EDIT_KEYWORDHELPFILE, m_Common.m_szKeyWordHelpFile, MAX_PATH - 1 );

	/* 外部ヘルプ１ */
	::GetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1, MAX_PATH - 1 );

	/* 外部HTMLヘルプ */
	::GetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp, MAX_PATH - 1 );

	/* HtmlHelpビューアはひとつ */
	m_Common.m_bHtmlHelpIsSingle = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HTMLHELPISSINGLE );

	
	/* 補完候補決定キー */
	m_Common.m_bHokanKey_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN );//VK_RETURN 補完決定キーが有効/無効
	m_Common.m_bHokanKey_TAB = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_TAB );//VK_TAB    補完決定キーが有効/無効
	m_Common.m_bHokanKey_RIGHT = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT );//VK_RIGHT  補完決定キーが有効/無効
	m_Common.m_bHokanKey_SPACE = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_SPACE );//VK_SPACE  補完決定キーが有効/無効

	return TRUE;
}
