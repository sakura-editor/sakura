/*!	@file
	@brief 履歴の管理ダイアログボックス

	@author MIK
	@date 2003.4.8
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji

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
#include <stdio.h>
#include <assert.h>
#include "global.h"
#include "Funccode.h"
#include "mymessage.h"
#include "CDlgFavorite.h"
#include "etc_uty.h"
#include "Debug.h"
#include "my_icmp.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDC_TAB_FAVORITE,				HIDC_TAB_FAVORITE,				//タブ
	IDC_LIST_FAVORITE_FILE,			HIDC_LIST_FAVORITE_FILE,		//ファイル
	IDC_LIST_FAVORITE_FOLDER,		HIDC_LIST_FAVORITE_FOLDER,		//フォルダ
	IDC_LIST_FAVORITE_SEARCH,		HIDC_LIST_FAVORITE_SEARCH,		//検索
	IDC_LIST_FAVORITE_REPLACE,		HIDC_LIST_FAVORITE_REPLACE,		//置換
	IDC_LIST_FAVORITE_GREP_FILE,	HIDC_LIST_FAVORITE_GREPFILE,	//GREPファイル
	IDC_LIST_FAVORITE_GREP_FOLDER,	HIDC_LIST_FAVORITE_GREPFOLDER,	//GREPフォルダ
	IDC_LIST_FAVORITE_CMD,			HIDC_LIST_FAVORITE_CMD,			//コマンド
	IDC_BUTTON_CLEAR,				HIDC_BUTTON_FAVORITE_CLEAR,		//履歴のクリア	// 2006.10.10 ryoji
	IDOK,							HIDC_FAVORITE_IDOK,				//OK
	IDCANCEL,						HIDC_FAVORITE_IDCANCEL,			//キャンセル
	IDC_BUTTON_HELP,				HIDC_BUTTON_FAVORITE_HELP,		//ヘルプ
//	IDC_STATIC,						-1,
	0, 0
};



//SDKにしか定義されていない。
#ifndef	ListView_SetCheckState
//#if (_WIN32_IE >= 0x0300)
#define ListView_SetCheckState(hwndLV, i, fCheck) \
  ListView_SetItemState(hwndLV, i, INDEXTOSTATEIMAGEMASK((fCheck)?2:1), LVIS_STATEIMAGEMASK)
//#endif
#endif



CDlgFavorite::CDlgFavorite()
{
	int	i;

	m_nCurrentTab = 0;
	_tcscpy( m_szMsg, _T("") );

	{
		memset( m_aFavoriteInfo, 0, sizeof( m_aFavoriteInfo ) );

		i = 0;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentFile;
		m_aFavoriteInfo[i].m_pszCaption = _T("ファイル");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_FILE;
		m_aFavoriteInfo[i].m_bHaveFavorite = true;
		m_aFavoriteInfo[i].m_bHaveView  = true;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentFolder;
		m_aFavoriteInfo[i].m_pszCaption = _T("フォルダ");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_FOLDER;
		m_aFavoriteInfo[i].m_bHaveFavorite = true;
		m_aFavoriteInfo[i].m_bHaveView  = true;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentSearch;
		m_aFavoriteInfo[i].m_pszCaption = _T("検索");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_SEARCH;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentReplace;
		m_aFavoriteInfo[i].m_pszCaption = _T("置換");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_REPLACE;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentGrepFile;
		m_aFavoriteInfo[i].m_pszCaption = _T("GREPファイル");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_GREP_FILE;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentGrepFolder;
		m_aFavoriteInfo[i].m_pszCaption = _T("GREPフォルダ");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_GREP_FOLDER;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentCmd;
		m_aFavoriteInfo[i].m_pszCaption = _T("コマンド");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_CMD;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = NULL;
		m_aFavoriteInfo[i].m_pszCaption = NULL;
		m_aFavoriteInfo[i].m_nId        = -1;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		/* これ以上増やすときはテーブルサイズも書き換えてね */
		assert( i < _countof(m_aFavoriteInfo) );
	}
}

CDlgFavorite::~CDlgFavorite()
{
	CRecent	*pRecent;
	int		nTab;

	for( nTab = 0; pRecent = m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		pRecent->Terminate();
	}
}

/* モーダルダイアログの表示 */
int CDlgFavorite::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_FAVORITE, lParam );
}

/* ダイアログデータの設定 */
void CDlgFavorite::SetData( void )
{
	int		nTab;

	for( nTab = 0; NULL != m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		SetDataOne( nTab, 0 );
	}

	::SetDlgItemText( m_hWnd, IDC_STATIC_FAVORITE_MSG, _T("") );

	return;
}

/* ダイアログデータの1つのタブの設定・更新
	@param nIndex       タブのIndex
	@param nLvItemIndex 選択・表示したいListViewのIndex。-1で選択しない
*/
void CDlgFavorite::SetDataOne( int nIndex, int nLvItemIndex )
{
	HWND	hwndList;
	int		i;
	LV_ITEM	lvi;
	int		nNewFocus = -1;

	CRecent*	pRecent = m_aFavoriteInfo[nIndex].m_pRecent;

	/* リスト */
	hwndList = ::GetDlgItem( m_hWnd, m_aFavoriteInfo[nIndex].m_nId );
	ListView_DeleteAllItems( hwndList );  /* リストを空にする */

	int   nViewCount = pRecent->GetViewCount();
	m_aFavoriteInfo[nIndex].m_nViewCount = nViewCount;

	TCHAR	tmp[1024];
	for( i = 0; i < pRecent->GetItemCount(); i++ )
	{
		wsprintf( tmp, "%s", (i < nViewCount) ? " " : "(非表示)" );
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = tmp;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = 0;
		ListView_InsertItem( hwndList, &lvi );

		const TCHAR	*p;
		p = pRecent->GetDataOfItem( i );
		wsprintf( tmp, "%s", p ? p : "" );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = tmp;
		ListView_SetItem( hwndList, &lvi );

		if( m_aFavoriteInfo[nIndex].m_bHaveFavorite )
		{
			ListView_SetCheckState( hwndList, i, (BOOL)pRecent->IsFavorite( i ) );
		}
		ListView_SetItemState( hwndList, i, 0, LVIS_SELECTED | LVIS_FOCUSED );

		if( i == nLvItemIndex ) nNewFocus = i;
	}

	//アイテムがあってどれも非選択なら最初を選択する。
	if( i > 0 && -1 != nLvItemIndex && nNewFocus == -1 )
	{
		nNewFocus = 0;
	}

	if( -1 != nNewFocus )
	{
		ListView_SetItemState( hwndList, nNewFocus, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		ListView_EnsureVisible( hwndList, nNewFocus, FALSE );
	}

	return;
}

/*! ダイアログデータを取得し、共有データのお気に入りを更新
	
	@retval TRUE 正常(今のところFALSEは返さない)
*/
int CDlgFavorite::GetData( void )
{
	HWND	hwndList;
	int		nCount;
	int		i;
	BOOL	bret;
	CRecent	*pRecent;
	int		nTab;

	for( nTab = 0; NULL != (pRecent = m_aFavoriteInfo[nTab].m_pRecent); nTab++ )
	{
		if( m_aFavoriteInfo[nTab].m_bHaveFavorite )
		{
			hwndList = GetDlgItem( m_hWnd, m_aFavoriteInfo[nTab].m_nId );

			nCount = ListView_GetItemCount( hwndList );

			//お気に入りフラグを更新する。
			for( i = 0; i < nCount; i++ )
			{
				bret = ListView_GetCheckState( hwndList, i );
				(void)pRecent->SetFavorite( i, bret ? true : false );
			}

			//リストを更新する。
			pRecent->UpdateView();
		}
	}

	return TRUE;
}

BOOL CDlgFavorite::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	HWND		hwndList;
	HWND		hwndBaseList;
	HWND		hwndTab;
	TCITEM		tcitem;
	LV_COLUMN	col;
	RECT		rc;
	CRecent		*pRecent;
	int			nTab;
	POINT		po;
	long		lngStyle;

	m_hWnd = hwndDlg;

	hwndTab = ::GetDlgItem( hwndDlg, IDC_TAB_FAVORITE );
	TabCtrl_DeleteAllItems( hwndTab );

	//リストビューの表示位置を取得する。
	m_nCurrentTab = 0;
	hwndBaseList = ::GetDlgItem( hwndDlg, m_aFavoriteInfo[0].m_nId );
	{
		rc.left = rc.top = rc.right = rc.bottom = 0;
		::GetWindowRect( hwndBaseList, &rc );
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient( hwndDlg, &po );
		rc.left = po.x;
		rc.top  = po.y;
		po.x = rc.right;
		po.y = rc.bottom;
		::ScreenToClient( hwndDlg, &po );
		rc.right  = po.x;
		rc.bottom = po.y;
	}

	TCHAR* pszFAVORITE_TEXT = const_cast<TCHAR*>(_T("お気に入り"));

	for( nTab = 0; NULL != (pRecent = m_aFavoriteInfo[nTab].m_pRecent); nTab++ )
	{
		hwndList = GetDlgItem( hwndDlg, m_aFavoriteInfo[nTab].m_nId );
		::MoveWindow( hwndList, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE );
		::ShowWindow( hwndList, SW_HIDE );

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 16 / 100;
		col.pszText  = pszFAVORITE_TEXT;
		col.iSubItem = 0;
		ListView_InsertColumn( hwndList, 0, &col );

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 79 / 100;
		col.pszText  = const_cast<TCHAR*>(m_aFavoriteInfo[nTab].m_pszCaption);
		col.iSubItem = 1;
		ListView_InsertColumn( hwndList, 1, &col );

		/* 行選択 */
		lngStyle = ListView_GetExtendedListViewStyle( hwndList );
		lngStyle |= LVS_EX_FULLROWSELECT;
		if( m_aFavoriteInfo[nTab].m_bHaveFavorite ) lngStyle |= LVS_EX_CHECKBOXES;
		ListView_SetExtendedListViewStyle( hwndList, lngStyle );

		/* タブ項目追加 */
		tcitem.mask = TCIF_TEXT;
		tcitem.pszText = const_cast<TCHAR*>(m_aFavoriteInfo[nTab].m_pszCaption);
		TabCtrl_InsertItem( hwndTab, nTab, &tcitem );
	}

	hwndList = ::GetDlgItem( hwndDlg, m_aFavoriteInfo[m_nCurrentTab].m_nId );
	::ShowWindow( hwndList, SW_SHOW );
	TabCtrl_SetCurSel( hwndTab, m_nCurrentTab );
	//ChangeSlider( m_nCurrentTab );

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( m_hWnd, wParam, lParam );
}

BOOL CDlgFavorite::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* ヘルプ */
		MyWinHelp( m_hWnd, HELP_CONTEXT, ::FuncID_To_HelpContextID( F_FAVORITE ) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	case IDOK:
		/* ダイアログデータの取得 */
		::EndDialog( m_hWnd, (BOOL)GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
		return TRUE;

	case IDC_BUTTON_CLEAR:	//履歴のクリア
		{
			::SetDlgItemText( m_hWnd, IDC_STATIC_FAVORITE_MSG, _T("") );
			HWND	hwndTab;
			int		nIndex;
			hwndTab = ::GetDlgItem( m_hWnd, IDC_TAB_FAVORITE );
			nIndex = TabCtrl_GetCurSel( hwndTab );
			if( -1 != nIndex )
			{
				int nRet;
				if( m_aFavoriteInfo[nIndex].m_bHaveFavorite ){
					nRet = ConfirmMessage( m_hWnd, 
						_T("最近使った%sの履歴を削除します。\n\nよろしいですか？\n\n")
						_T("「はい」\tすべて削除します。\n")
						_T("「いいえ」\tお気に入り以外を削除します。\n"),
						m_aFavoriteInfo[nIndex].m_pszCaption );
				}
				else
				{
					nRet = ConfirmMessage( m_hWnd, 
						_T("最近使った%sの履歴を削除します。\n\nよろしいですか？\n"),
						m_aFavoriteInfo[nIndex].m_pszCaption );
				}
				
				CRecent	*pRecent;
				pRecent = m_aFavoriteInfo[nIndex].m_pRecent;

				switch( nRet )
				{
				case IDYES:
				case IDOK:
					if( pRecent ) pRecent->DeleteAllItem();
					InfoMessage( m_hWnd,
						_T("最近使った%sの履歴を削除しました。"),
						m_aFavoriteInfo[nIndex].m_pszCaption );
					break;
					
				case IDNO:
					if( pRecent )
					{
						HWND hwndList = ::GetDlgItem( m_hWnd, m_aFavoriteInfo[nIndex].m_nId );
						for( int i = pRecent->GetItemCount() - 1; i >= 0; i-- )
						{
							if( ! ListView_GetCheckState( hwndList, i ) ){
								pRecent->DeleteItem( i );
							}
							else {
								pRecent->SetFavorite( i );
							}
						}
					}
					InfoMessage( m_hWnd,
						_T("最近使った%sの履歴(お気に入り以外)を削除しました。"),
						m_aFavoriteInfo[nIndex].m_pszCaption );
					break;
					
				case IDCANCEL:
				default:
					break;
				}
			}
		}
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgFavorite::OnNotify( WPARAM wParam, LPARAM lParam )
{
	LPNMHDR	lpnmhdr;
	HWND	hwndTab;
	HWND	hwndList;
	int		nIndex;

	hwndTab = ::GetDlgItem( m_hWnd, IDC_TAB_FAVORITE );
	lpnmhdr = (LPNMHDR) lParam;
	if( lpnmhdr->hwndFrom == hwndTab )
	{
		switch( lpnmhdr->code )
		{
		case TCN_SELCHANGE:
			::SetDlgItemText( m_hWnd, IDC_STATIC_FAVORITE_MSG, _T("") );
			nIndex = TabCtrl_GetCurSel( hwndTab );
			if( -1 != nIndex )
			{
				//現在表示中のリストを隠す。
				hwndList = GetDlgItem( m_hWnd, m_aFavoriteInfo[m_nCurrentTab].m_nId );
				::ShowWindow( hwndList, SW_HIDE );

				//新しく表示する。
				hwndList = GetDlgItem( m_hWnd, m_aFavoriteInfo[nIndex].m_nId );
				::ShowWindow( hwndList, SW_SHOW );

				::SetFocus( hwndList );

				m_nCurrentTab = nIndex;

				//ChangeSlider( nIndex );
			}
			return TRUE;
			//break;
		}
	}

	/* 基底クラスメンバ */
	return CDialog::OnNotify( wParam, lParam );
}

BOOL CDlgFavorite::OnActivate( WPARAM wParam, LPARAM lParam )
{
	switch( LOWORD( wParam ) )
	{
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		RefreshList();
		::SetDlgItemText( m_hWnd, IDC_STATIC_FAVORITE_MSG, m_szMsg );
		return TRUE;
		//break;

	case WA_INACTIVE:
	default:
		break;
	}

	/* 基底クラスメンバ */
	return CDialog::OnActivate( wParam, lParam );
}

LPVOID CDlgFavorite::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

/*
	リストを更新する。
*/
bool CDlgFavorite::RefreshList( void )
{
	int		nTab;
	bool	bret;
	bool	ret_val = false;
	TCHAR	msg[1024];

	_tcscpy( msg, _T("") );
	_tcscpy( m_szMsg, _T("") );

	//全リストの現在選択中のアイテムを取得する。
	for( nTab = 0; NULL != m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		bret = RefreshListOne( nTab );
		if( bret == true )
		{
			ret_val = true;

			if( msg[0] != _T('\0') ) _tcscat( msg, _T("、") );
			_tcscat( msg, m_aFavoriteInfo[nTab].m_pszCaption );
		}
	}

	if( ret_val )
	{
		wsprintf( m_szMsg, 
			"履歴(%s)が更新されたため編集中情報を破棄し再表示しました。",
			msg );
	}

	return ret_val;
}

/*
	履歴種別リストのうち1個のリストビューを更新する。
*/
bool CDlgFavorite::RefreshListOne( int nIndex )
{
	HWND	hwndList;
	int		nCount;
	int		nCurrentIndex;
	int		nItemCount;
	int		i;
	BOOL	bret;
	LVITEM	lvitem;

	CRecent*	pRecent       = m_aFavoriteInfo[nIndex].m_pRecent;
	nItemCount    = pRecent->GetItemCount();
	hwndList      = GetDlgItem( m_hWnd, m_aFavoriteInfo[nIndex].m_nId );
	nCount        = ListView_GetItemCount( hwndList );
	nCurrentIndex = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
	if( -1 == nCurrentIndex ) nCurrentIndex = ListView_GetNextItem( hwndList, -1, LVNI_FOCUSED );

	if( nItemCount != nCount ) goto changed;	//個数が変わったので再構築

	//お気に入り数が変わったので再構築
	if( m_aFavoriteInfo[nIndex].m_nViewCount != pRecent->GetViewCount() ) goto changed;

	for( i = 0; i < nCount; i++ )
	{
		TCHAR	szText[1024];
		memset( szText, 0, _countof( szText ) );
		memset( &lvitem, 0, sizeof( lvitem ) );
		lvitem.mask       = LVIF_TEXT;
		lvitem.pszText    = szText;
		lvitem.cchTextMax = _countof( szText );
		lvitem.iItem      = i;
		lvitem.iSubItem   = 1;
		bret = ListView_GetItem( hwndList, &lvitem );
		if( !bret ) goto changed;	//エラーなので再構築

		//アイテム内容が変わったので再構築
		if( i != pRecent->FindItem( szText ) ) goto changed;
	}

	return false;

changed:
	SetDataOne( nIndex, nCurrentIndex );
	

	return true;
}

/*[EOF]*/
