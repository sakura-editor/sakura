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
	Copyright (C) 2012, Moca
	Copyright (C) 2013, Uchi

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {
	IDC_CHECK_DispTabWnd,			HIDC_CHECK_DispTabWnd,			//タブウインドウ表示	//@@@ 2003.05.31 MIK
	IDC_CHECK_SameTabWidth,			HIDC_CHECK_SameTabWidth,		//等幅	// 2006.08.06 ryoji
	IDC_CHECK_DispTabIcon,			HIDC_CHECK_DispTabIcon,			//アイコン表示	// 2006.08.06 ryoji
	IDC_CHECK_DispTabClose,			HIDC_CHECK_DispTabClose,		//タブを閉じるボタン表示	// 2012.04.14 syat
	IDC_CHECK_SortTabList,			HIDC_CHECK_SortTabList,			//タブ一覧ソート	// 2006.08.06 ryoji
	IDC_CHECK_DispTabWndMultiWin,	HIDC_CHECK_DispTabWndMultiWin,	//タブウインドウ表示	//@@@ 2003.05.31 MIK
	IDC_TABWND_CAPTION,				HIDC_TABWND_CAPTION,			//タブウインドウキャプション	//@@@ 2003.06.15 MIK
	IDC_CHECK_RetainEmptyWindow,	HIDC_CHECK_RetainEmptyWindow,	//最後のファイルを閉じたとき(無題)文書を残す	// 2007.02.13 ryoji
	IDC_CHECK_CloseOneWin,			HIDC_CHECK_CloseOneWin,			//ウィンドウの閉じるボタンは現在のファイルのみ閉じる	// 2007.02.13 ryoji
	IDC_CHECK_ChgWndByWheel,		HIDC_CHECK_ChgWndByWheel,		//マウスホイールでウィンドウ切り替え 2007.04.03 ryoji
	IDC_CHECK_OpenNewWin,			HIDC_CHECK_OpenNewWin,			//外部から起動するときは新しいウインドウで開く 2009.06.19
	IDC_BUTTON_TABFONT,				HIDC_BUTTON_TABFONT,			//タブフォント
	0, 0
};

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg[in] ダイアログボックスのWindow Handle
	@param uMsg[in] メッセージ
	@param wParam[in] パラメータ1
	@param lParam[in] パラメータ2
*/
INT_PTR CALLBACK CPropTab::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropTab::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* メッセージ処理 */
INT_PTR CPropTab::DispatchEvent( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	NMHDR*		pNMHDR;
//	int			idCtrl;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 Tab */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */

		return TRUE;
	case WM_NOTIFY:
//		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
//		switch( idCtrl ){
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_TAB );
				return TRUE;
			case PSN_KILLACTIVE:
				/* ダイアログデータの取得 Tab */
				GetData( hwndDlg );
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
			if( wNotifyCode == BN_CLICKED ){
				switch( wID ){
				case IDC_CHECK_DispTabWnd:
				case IDC_CHECK_DispTabWndMultiWin:
					EnableTabPropInput( hwndDlg );
					break;
				case IDC_BUTTON_TABFONT:
					LOGFONT   lf = m_Common.m_sTabBar.m_lf;
					INT nPointSize = m_Common.m_sTabBar.m_nPointSize;

					if( MySelectFont( &lf, &nPointSize, hwndDlg, false) ){
						m_Common.m_sTabBar.m_lf = lf;
						m_Common.m_sTabBar.m_nPointSize = nPointSize;
						// タブ フォント表示	// 2013/4/24 Uchi
						if (m_hTabFont != NULL)		::DeleteObject( m_hTabFont );
						m_hTabFont = SetFontLabel( hwndDlg, IDC_STATIC_TABFONT, m_Common.m_sTabBar.m_lf, m_Common.m_sTabBar.m_nPointSize);
					}
					break;
				}
			}
		}
		break;
//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.12.22 End

	case WM_DESTROY:
		// タブ フォント破棄	// 2013/4/24 Uchi
		if (m_hTabFont != NULL) {
			::DeleteObject( m_hTabFont );
			m_hTabFont = NULL;
		}
		return TRUE;
	}
	return FALSE;
}


/* ダイアログデータの設定 */
void CPropTab::SetData( HWND hwndDlg )
{
	//	Feb. 11, 2007 genta「ウィンドウ」シートより移動
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispTabWnd, m_Common.m_sTabBar.m_bDispTabWnd );	//@@@ 2003.05.31 MIK
	::CheckDlgButton( hwndDlg, IDC_CHECK_SameTabWidth, m_Common.m_sTabBar.m_bSameTabWidth );	//@@@ 2006.01.28 ryoji
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispTabIcon, m_Common.m_sTabBar.m_bDispTabIcon );	//@@@ 2006.01.28 ryoji
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispTabClose, m_Common.m_sTabBar.m_bDispTabClose );	//@@@ 2012.04.14 syat
	::CheckDlgButton( hwndDlg, IDC_CHECK_SortTabList, m_Common.m_sTabBar.m_bSortTabList );			//@@@ 2006.03.23 fon
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispTabWndMultiWin, ! m_Common.m_sTabBar.m_bDispTabWndMultiWin ); //@@@ 2003.05.31 MIK
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_TABWND_CAPTION ), _countof( m_Common.m_sTabBar.m_szTabWndCaption ) - 1 );
	::DlgItem_SetText( hwndDlg, IDC_TABWND_CAPTION, m_Common.m_sTabBar.m_szTabWndCaption );

	//	Feb. 11, 2007 genta 新規作成
	::CheckDlgButton( hwndDlg, IDC_CHECK_RetainEmptyWindow, m_Common.m_sTabBar.m_bTab_RetainEmptyWin );
	::CheckDlgButton( hwndDlg, IDC_CHECK_CloseOneWin, m_Common.m_sTabBar.m_bTab_CloseOneWin );
	::CheckDlgButton( hwndDlg, IDC_CHECK_ChgWndByWheel, m_Common.m_sTabBar.m_bChgWndByWheel );	// 2007.04.03 ryoji
	::CheckDlgButton( hwndDlg, IDC_CHECK_OpenNewWin, m_Common.m_sTabBar.m_bNewWindow ); // 2009.06.17

	// タブ フォント	// 2013/4/24 Uchi
	m_hTabFont = SetFontLabel( hwndDlg, IDC_STATIC_TABFONT, m_Common.m_sTabBar.m_lf, m_Common.m_sTabBar.m_nPointSize);

	EnableTabPropInput(hwndDlg);
}

/* ダイアログデータの取得 */
int CPropTab::GetData( HWND hwndDlg )
{
	//	Feb. 11, 2007 genta「ウィンドウ」シートより移動
	m_Common.m_sTabBar.m_bDispTabWnd = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTabWnd );
	m_Common.m_sTabBar.m_bSameTabWidth = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SameTabWidth );		// 2006.01.28 ryoji
	m_Common.m_sTabBar.m_bDispTabIcon = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTabIcon );		// 2006.01.28 ryoji
	m_Common.m_sTabBar.m_bDispTabClose = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTabClose );		// 2012.04.14 syat
	m_Common.m_sTabBar.m_bSortTabList = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SortTabList );		// 2006.03.23 fon
	m_Common.m_sTabBar.m_bDispTabWndMultiWin =
		( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTabWndMultiWin ) == BST_CHECKED ) ? FALSE : TRUE;
	::DlgItem_GetText( hwndDlg, IDC_TABWND_CAPTION, m_Common.m_sTabBar.m_szTabWndCaption, _countof( m_Common.m_sTabBar.m_szTabWndCaption ) );

	//	Feb. 11, 2007 genta 新規作成
	m_Common.m_sTabBar.m_bTab_RetainEmptyWin = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_RetainEmptyWindow );
	m_Common.m_sTabBar.m_bTab_CloseOneWin = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CloseOneWin );
	m_Common.m_sTabBar.m_bChgWndByWheel = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ChgWndByWheel );	// 2007.04.03 ryoji
	m_Common.m_sTabBar.m_bNewWindow = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_OpenNewWin );  // 2009.06.17

	return TRUE;
}

/*! 「タブバー」シート上のアイテムの有効・無効を適切に設定する

	@date 2007.02.12 genta 新規作成
*/
void CPropTab::EnableTabPropInput(HWND hwndDlg)
{
	
	BOOL bTabWnd = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTabWnd );
	BOOL bMultiWin = FALSE;
	if( bTabWnd ){
		bMultiWin = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTabWndMultiWin );
	}
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DispTabWndMultiWin ), bTabWnd );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_RetainEmptyWindow  ), bMultiWin );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_CloseOneWin        ), bMultiWin );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_OpenNewWin         ), bMultiWin );	// 2009.06.17
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DispTabIcon        ), bTabWnd );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_SameTabWidth       ), bTabWnd );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DispTabClose       ), bTabWnd );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_TABFONT           ), bTabWnd );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_TABFONT           ), bTabWnd );	// 2013/4/24 Uchi
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_SortTabList        ), bTabWnd );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_TABWND_CAPTION           ), bTabWnd );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_ChgWndByWheel      ), bTabWnd );	// 2007.04.03 ryoji
}
