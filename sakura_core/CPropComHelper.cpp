/*!	@file
	@brief 共通設定ダイアログボックス、「支援」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, genta, MIK, jepro, asa-o
	Copyright (C) 2002, YAZAKI, MIK, genta
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include "sakura_rc.h"
#include "CPropCommon.h"
#include "debug.h"
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "CDlgInput1.h"
#include "global.h"
#include "util/shell.h"
#include "util/module.h"
#include "util/file.h"

//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//10600
	IDC_BUTTON_OPENHELP1,			HIDC_BUTTON_OPENHELP1,			//外部ヘルプファイル参照
	IDC_BUTTON_OPENEXTHTMLHELP,		HIDC_BUTTON_OPENEXTHTMLHELP,	//外部HTMLファイル参照
//	IDC_CHECK_USEHOKAN,				HIDC_CHECK_USEHOKAN,			//逐次入力補完
	IDC_CHECK_m_bHokanKey_RETURN,	HIDC_CHECK_m_bHokanKey_RETURN,	//候補決定キー（Enter）
	IDC_CHECK_m_bHokanKey_TAB,		HIDC_CHECK_m_bHokanKey_TAB,		//候補決定キー（Tab）
	IDC_CHECK_m_bHokanKey_RIGHT,	HIDC_CHECK_m_bHokanKey_RIGHT,	//候補決定キー（→）
//	IDC_CHECK_m_bHokanKey_SPACE,	HIDC_CHECK_m_bHokanKey_SPACE,	//候補決定キー（Space）
	IDC_CHECK_HTMLHELPISSINGLE,		HIDC_CHECK_HTMLHELPISSINGLE,	//ビューアの複数起動
	IDC_EDIT_EXTHELP1,				HIDC_EDIT_EXTHELP1,				//外部ヘルプファイル名
	IDC_EDIT_EXTHTMLHELP,			HIDC_EDIT_EXTHTMLHELP,			//外部HTMLヘルプファイル名
	//	2007.02.04 genta カーソル位置の単語の辞書検索は共通設定から外した
	//IDC_CHECK_CLICKKEYSEARCH,		HIDC_CHECK_CLICKKEYSEARCH,		//キャレット位置の単語を辞書検索	// 2006.03.24 fon
	IDC_BUTTON_KEYWORDHELPFONT,		HIDC_BUTTON_KEYWORDHELPFONT,	//キーワードヘルプのフォント
	IDC_EDIT_MIGEMO_DLL,			HIDC_EDIT_MIGEMO_DLL,			//Migemo DLLファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_OPENMDLL,			HIDC_BUTTON_OPENMDLL,			//Migemo DLLファイル参照	// 2006.08.06 ryoji
	IDC_EDIT_MIGEMO_DICT,			HIDC_EDIT_MIGEMO_DICT,			//Migemo 辞書ファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_OPENMDICT,			HIDC_BUTTON_OPENMDICT,			//Migemo 辞書ファイル参照	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//10600
//	IDC_BUTTON_HOKANFILE_REF,		10600,	//入力補完 単語ファイル参照		//Jul. 05, 2001 JEPRO タイプ別に移動
//	IDC_BUTTON_KEYWORDHELPFILE_REF,	10601,	//キーワードヘルプファイル参照	//Jul. 05, 2001 JEPRO タイプ別に移動
	IDC_BUTTON_OPENHELP1,			10602,	//外部ヘルプファイル参照
	IDC_BUTTON_OPENEXTHTMLHELP,		10603,	//外部HTMLファイル参照
//	IDC_CHECK_USEHOKAN,				10610,	//逐次入力補完
//	IDC_CHECK_HOKANLOHICASE,		10611,	//入力補完の英大文字小文字		//Jul. 05, 2001 JEPRO タイプ別に移動
	IDC_CHECK_m_bHokanKey_RETURN,	10612,	//候補決定キー（Enter）
	IDC_CHECK_m_bHokanKey_TAB,		10613,	//候補決定キー（Tab）
	IDC_CHECK_m_bHokanKey_RIGHT,	10614,	//候補決定キー（→）
//	IDC_CHECK_m_bHokanKey_SPACE,	10615,	//候補決定キー（Space）
//	IDC_CHECK_USEKEYWORDHELP,		10616,	//キーワードヘルプ機能			//Jul. 05, 2001 JEPRO タイプ別に移動
	IDC_CHECK_HTMLHELPISSINGLE,		10617,	//ビューアの複数起動
//	IDC_EDIT_HOKANFILE,				10640,	//単語ファイル名				//Jul. 05, 2001 JEPRO タイプ別に移動
//	IDC_EDIT_KEYWORDHELPFILE,		10641,	//辞書ファイル名				//Jul. 05, 2001 JEPRO タイプ別に移動
	IDC_EDIT_EXTHELP1,				10642,	//外部ヘルプファイル名
	IDC_EDIT_EXTHTMLHELP,			10643,	//外部HTMLヘルプファイル名
//	IDC_STATIC,						-1,
	0, 0
};
#endif
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_HELPER(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_p10, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* p10 メッセージ処理 */
INT_PTR CPropCommon::DispatchEvent_p10(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
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
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		/* 外部ヘルプ１ */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHELP1 ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );
		/* 外部HTMLヘルプ */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHTMLHELP ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );


		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			/* ダイアログデータの取得 p10 */
			GetData_p10( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_OPENHELP1:	/* 外部ヘルプ１の「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパス
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Common.m_sHelper.m_szExtHelp ) ){
						GetInidirOrExedir( szPath, m_Common.m_sHelper.m_szExtHelp, TRUE );
					}else{
						_tcscpy( szPath, m_Common.m_sHelper.m_szExtHelp );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.hlp"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Common.m_sHelper.m_szExtHelp, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_sHelper.m_szExtHelp );
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENEXTHTMLHELP:	/* 外部HTMLヘルプの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパス
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Common.m_sHelper.m_szExtHtmlHelp ) ){
						GetInidirOrExedir( szPath, m_Common.m_sHelper.m_szExtHtmlHelp, TRUE );
					}else{
						_tcscpy( szPath, m_Common.m_sHelper.m_szExtHtmlHelp );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.chm;*.col"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Common.m_sHelper.m_szExtHtmlHelp, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_sHelper.m_szExtHtmlHelp );
					}
				}
				return TRUE;
			// ai 02/05/21 Add S
			case IDC_BUTTON_KEYWORDHELPFONT:	/* キーワードヘルプの「フォント」ボタン */
				{
					CHOOSEFONT	cf;
					LOGFONT		lf;

					/* LOGFONTの初期化 */
					memcpy_raw(&lf, &(m_Common.m_sHelper.m_lf_kh), sizeof(lf));

					/* CHOOSEFONTの初期化 */
					memset_raw(&cf, 0, sizeof(cf));
					cf.lStructSize = sizeof(cf);
					cf.hwndOwner = hwndDlg;
					cf.hDC = NULL;
					cf.lpLogFont = &lf;
//					cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;
					cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
					if(ChooseFont(&cf))
					{
						memcpy_raw(&(m_Common.m_sHelper.m_lf_kh), &lf, sizeof(lf));
					}
				}
				return TRUE;
			// ai 02/05/21 Add E
			case IDC_BUTTON_OPENMDLL:	/* MIGEMODLL場所指定「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパス
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Common.m_sHelper.m_szMigemoDll ) ){
						GetInidirOrExedir( szPath, m_Common.m_sHelper.m_szMigemoDll, TRUE );
					}else{
						_tcscpy( szPath, m_Common.m_sHelper.m_szMigemoDll );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.dll"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Common.m_sHelper.m_szMigemoDll, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DLL, m_Common.m_sHelper.m_szMigemoDll );
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENMDICT:	/* MigemoDict場所指定「参照...」ボタン */
				{
					TCHAR	szPath[MAX_PATH];
					/* 検索フォルダ */
					// 2007.05.27 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Common.m_sHelper.m_szMigemoDict ) ){
						GetInidirOrExedir( szPath, m_Common.m_sHelper.m_szMigemoDict, TRUE );
					}else{
						_tcscpy( szPath, m_Common.m_sHelper.m_szMigemoDict );
					}
					if( SelectDir( hwndDlg, _T("検索するフォルダを選んでください"), szPath, szPath ) ){
						_tcscpy( m_Common.m_sHelper.m_szMigemoDict, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict );
					}
				}
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
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
//				MYTRACE_A( "p10 PSN_KILLACTIVE\n" );
				/* ダイアログデータの取得 p10 */
				GetData_p10( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_HELPER;
				return TRUE;
			}
//			break;	/* default */
//		}

//		MYTRACE_A( "pNMHDR->hwndFrom=%xh\n", pNMHDR->hwndFrom );
//		MYTRACE_A( "pNMHDR->idFrom  =%xh\n", pNMHDR->idFrom );
//		MYTRACE_A( "pNMHDR->code    =%xh\n", pNMHDR->code );
//		MYTRACE_A( "pMNUD->iPos    =%d\n", pMNUD->iPos );
//		MYTRACE_A( "pMNUD->iDelta  =%d\n", pMNUD->iDelta );
		break;	/* WM_NOTIFY */

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}

/* ダイアログデータの設定 p10 */
void CPropCommon::SetData_p10( HWND hwndDlg )
{
	/* 外部ヘルプ１ */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_sHelper.m_szExtHelp );

	/* 外部HTMLヘルプ */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_sHelper.m_szExtHtmlHelp );

	/* HtmlHelpビューアはひとつ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HTMLHELPISSINGLE, m_Common.m_sHelper.m_bHtmlHelpIsSingle );

	/* 補完候補決定キー */
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN, m_Common.m_sHelper.m_bHokanKey_RETURN );	//VK_RETURN 補完決定キーが有効/無効
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_TAB, m_Common.m_sHelper.m_bHokanKey_TAB );		//VK_TAB    補完決定キーが有効/無効
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT, m_Common.m_sHelper.m_bHokanKey_RIGHT );	//VK_RIGHT  補完決定キーが有効/無効

	//migemo dict 
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DLL, m_Common.m_sHelper.m_szMigemoDll);
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict);

	return;
}


/* ダイアログデータの取得 p10 */
int CPropCommon::GetData_p10( HWND hwndDlg )
{
	// Oct. 5, 2002 genta サイズ制限方法変更
	/* 外部ヘルプ１ */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_sHelper.m_szExtHelp, _countof( m_Common.m_sHelper.m_szExtHelp ));

	/* 外部HTMLヘルプ */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_sHelper.m_szExtHtmlHelp, _countof( m_Common.m_sHelper.m_szExtHtmlHelp ));

	/* HtmlHelpビューアはひとつ */
	m_Common.m_sHelper.m_bHtmlHelpIsSingle = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HTMLHELPISSINGLE );


	/* 補完候補決定キー */
	m_Common.m_sHelper.m_bHokanKey_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN );//VK_RETURN 補完決定キーが有効/無効
	m_Common.m_sHelper.m_bHokanKey_TAB = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_TAB );		//VK_TAB    補完決定キーが有効/無効
	m_Common.m_sHelper.m_bHokanKey_RIGHT = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT );	//VK_RIGHT  補完決定キーが有効/無効

	::DlgItem_GetText( hwndDlg, IDC_EDIT_MIGEMO_DLL, m_Common.m_sHelper.m_szMigemoDll, _countof( m_Common.m_sHelper.m_szMigemoDll ));
	::DlgItem_GetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict, _countof( m_Common.m_sHelper.m_szMigemoDict ));


	return TRUE;
}


/*[EOF]*/
