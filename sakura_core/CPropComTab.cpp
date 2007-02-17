/*!	@file
	@brief 共通設定ダイアログボックス、「タブバー」ページ

	@author Norio Nakatani
	@date 2007.02.11 genta 共通設定に新規タブを追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2007, genta
	Copyright (C) 2001, MIK, genta
	Copyright (C) 2002, YAZAKI, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta, ryoji
*/

#include "stdafx.h"
#include "CPropCommon.h"

#include "sakura.hh"
static const DWORD p_helpids[] = {
	IDC_CHECK_DispTabWnd,			HIDC_CHECK_DispTabWnd,			//タブウインドウ表示	//@@@ 2003.05.31 MIK
	IDC_CHECK_SameTabWidth,			HIDC_CHECK_SameTabWidth,		//等幅	// 2006.08.06 ryoji
	IDC_CHECK_DispTabIcon,			HIDC_CHECK_DispTabIcon,			//アイコン表示	// 2006.08.06 ryoji
	IDC_CHECK_SortTabList,			HIDC_CHECK_SortTabList,			//タブ一覧ソート	// 2006.08.06 ryoji
	IDC_CHECK_DispTabWndMultiWin,	HIDC_CHECK_DispTabWndMultiWin,	//タブウインドウ表示	//@@@ 2003.05.31 MIK
	IDC_TABWND_CAPTION,				HIDC_TABWND_CAPTION,			//タブウインドウキャプション	//@@@ 2003.06.15 MIK
	IDC_CHECK_RetainEmptyWindow,	HIDC_CHECK_RetainEmptyWindow,	//最後のファイルを閉じたとき(無題)文書を残す	// 2007.02.13 ryoji
	IDC_CHECK_CloseOneWin,			HIDC_CHECK_CloseOneWin,			//ウィンドウの閉じるボタンは現在のファイルのみ閉じる	// 2007.02.13 ryoji
	0, 0
};

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg[in] ダイアログボックスのWindow Handle
	@param uMsg[in] メッセージ
	@param wParam[in] パラメータ1
	@param lParam[in] パラメータ2
*/
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_TAB(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_PROP_TAB, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* メッセージ処理 */
INT_PTR CPropCommon::DispatchEvent_PROP_TAB( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 p1 */
		SetData_PROP_TAB( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */

		return TRUE;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_TAB );
				return TRUE;
			case PSN_KILLACTIVE:
				/* ダイアログデータの取得 p1 */
				GetData_PROP_TAB( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_TAB;
				return TRUE;
			}
//			break;	/* default */
//		}
		break;	/* WM_NOTIFY */

	case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);	/* 通知コード */
			WORD wID = LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
			if( wNotifyCode == BN_CLICKED &&
				(wID == IDC_CHECK_DispTabWnd || wID == IDC_CHECK_DispTabWndMultiWin ) ){
				
				EnableTabPropInput( hwndDlg );
			}
		}
		break;
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


/* ダイアログデータの設定 */
void CPropCommon::SetData_PROP_TAB( HWND hwndDlg )
{

	//	Feb. 11, 2007 genta「ウィンドウ」シートより移動
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispTabWnd, m_Common.m_bDispTabWnd );	//@@@ 2003.05.31 MIK
	::CheckDlgButton( hwndDlg, IDC_CHECK_SameTabWidth, m_Common.m_bSameTabWidth );	//@@@ 2006.01.28 ryoji
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispTabIcon, m_Common.m_bDispTabIcon );	//@@@ 2006.01.28 ryoji
	::CheckDlgButton( hwndDlg, IDC_CHECK_SortTabList, m_Common.m_bSortTabList );			//@@@ 2006.03.23 fon
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispTabWndMultiWin, ! m_Common.m_bDispTabWndMultiWin ); //@@@ 2003.05.31 MIK
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_TABWND_CAPTION ), EM_LIMITTEXT, (WPARAM)(sizeof( m_Common.m_szTabWndCaption ) - 1 ), (LPARAM)0 );
	::SetDlgItemText( hwndDlg, IDC_TABWND_CAPTION, m_Common.m_szTabWndCaption );

	//	Feb. 11, 2007 genta 新規作成
	::CheckDlgButton( hwndDlg, IDC_CHECK_RetainEmptyWindow, m_Common.m_bTab_RetainEmptyWin );
	::CheckDlgButton( hwndDlg, IDC_CHECK_CloseOneWin, m_Common.m_bTab_CloseOneWin );
	
	EnableTabPropInput(hwndDlg);
	return;
}

/* ダイアログデータの取得 */
int CPropCommon::GetData_PROP_TAB( HWND hwndDlg )
{
	//	Feb. 11, 2007 genta「ウィンドウ」シートより移動
	m_Common.m_bDispTabWnd = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTabWnd );
	m_Common.m_bSameTabWidth = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SameTabWidth );		// 2006.01.28 ryoji
	m_Common.m_bDispTabIcon = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTabIcon );		// 2006.01.28 ryoji
	m_Common.m_bSortTabList = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SortTabList );		// 2006.03.23 fon
	m_Common.m_bDispTabWndMultiWin =
		( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTabWndMultiWin ) == BST_CHECKED ) ? FALSE : TRUE;
	::GetDlgItemText( hwndDlg, IDC_TABWND_CAPTION, m_Common.m_szTabWndCaption, sizeof( m_Common.m_szTabWndCaption ) );

	//	Feb. 11, 2007 genta 新規作成
	m_Common.m_bTab_RetainEmptyWin = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_RetainEmptyWindow );
	m_Common.m_bTab_CloseOneWin = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CloseOneWin );

	return TRUE;
}

/*! 「タブバー」シート上のアイテムの有効・無効を適切に設定する

	@date 2007.02.12 genta 新規作成
*/
void CPropCommon::EnableTabPropInput(HWND hwndDlg)
{
	if( !::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTabWnd ) )
	{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_SameTabWidth       ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DispTabIcon        ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_SortTabList        ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_TABWND_CAPTION           ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DispTabWndMultiWin ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_RetainEmptyWindow  ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_CloseOneWin        ), FALSE );
	}
	else
	{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_SameTabWidth       ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DispTabIcon        ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_SortTabList        ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_TABWND_CAPTION           ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DispTabWndMultiWin ), TRUE );
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTabWndMultiWin ) )
		{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_RetainEmptyWindow ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_CloseOneWin       ), TRUE );
		}
		else {
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_RetainEmptyWindow ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_CloseOneWin       ), FALSE );
		}
	}
}
/*[EOF]*/
