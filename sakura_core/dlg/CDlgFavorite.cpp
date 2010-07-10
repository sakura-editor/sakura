/*!	@file
	@brief 履歴の管理ダイアログボックス

	@author MIK
	@date 2003.4.8
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2010, Moca

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



#include "stdafx.h"
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "CDlgFavorite.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/std_macro.h"
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
	IDC_BUTTON_CLEAR,				HIDC_BUTTON_FAVORITE_CLEAR,		//すべて	// 2006.10.10 ryoji
	IDC_BUTTON_DELETE_NOFAVORATE,   HIDC_BUTTON_FAVORITE_DELETE_NOFAVORATE,  //お気に入り以外
	IDC_BUTTON_DELETE_NOTFOUND,		HIDC_BUTTON_FAVORITE_DELETE_NOTFOUND		,  //存在しない項目
	IDC_BUTTON_DELETE_SELECTED,     HIDC_BUTTON_FAVORITE_DELETE_SELECTED,    //選択項目
	IDOK,							HIDC_FAVORITE_IDOK,				//閉じる
//	IDCANCEL,						HIDC_FAVORITE_IDCANCEL,			//キャンセル
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

static int FormatFavoriteColm( TCHAR*, int, int , bool );
static int ListView_GetLParamInt( HWND, int );
static int CALLBACK CompareListViewFunc( LPARAM, LPARAM, LPARAM );

struct CompareListViewLParam
{
	int         nSortColumn;
	bool        bAbsOrder;
	HWND        hwndListView;
	const CRecent* pRecent;
};

/*
	CRecentの各実装クラスは DLLSHAREDATA へ直接アクセスしている。
	履歴はほかのウィンドウが書き換える可能性があるため、
	ダイアログがアクティブになった際に変更を確認し再取得するようになっている。
	編集中は変更を確認していないので、裏でDLLSHAREDATAを変更されるとListViewと
	DLLSHAREDATAが一致しない可能性もある。
*/


CDlgFavorite::CDlgFavorite()
{
	int	i;

	m_nCurrentTab = 0;
	_tcscpy( m_szMsg, _T("") );

	{
		memset_raw( m_aFavoriteInfo, 0, sizeof( m_aFavoriteInfo ) );

		i = 0;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentFile;
		m_aFavoriteInfo[i].m_pszCaption = _T("ファイル");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_FILE;
		m_aFavoriteInfo[i].m_bHaveFavorite = true;
		m_aFavoriteInfo[i].m_bFilePath  = true;
		m_aFavoriteInfo[i].m_bHaveView  = true;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentFolder;
		m_aFavoriteInfo[i].m_pszCaption = _T("フォルダ");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_FOLDER;
		m_aFavoriteInfo[i].m_bHaveFavorite = true;
		m_aFavoriteInfo[i].m_bFilePath  = true;
		m_aFavoriteInfo[i].m_bHaveView  = true;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentSearch;
		m_aFavoriteInfo[i].m_pszCaption = _T("検索");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_SEARCH;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentReplace;
		m_aFavoriteInfo[i].m_pszCaption = _T("置換");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_REPLACE;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentGrepFile;
		m_aFavoriteInfo[i].m_pszCaption = _T("GREPファイル");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_GREP_FILE;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentGrepFolder;
		m_aFavoriteInfo[i].m_pszCaption = _T("GREPフォルダ");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_GREP_FOLDER;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = true;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentCmd;
		m_aFavoriteInfo[i].m_pszCaption = _T("コマンド");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_CMD;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = NULL;
		m_aFavoriteInfo[i].m_pszCaption = NULL;
		m_aFavoriteInfo[i].m_nId        = -1;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		/* これ以上増やすときはテーブルサイズも書き換えてね */
		assert( i < _countof(m_aFavoriteInfo) );
	}
	for( i = 0; i < FAVORITE_INFO_MAX; i++ ){
		m_aListViewInfo[i].hListView   = 0;
		m_aListViewInfo[i].nSortColumn = -1;
		m_aListViewInfo[i].bSortAscending = false;
	}
}

CDlgFavorite::~CDlgFavorite()
{
	for( int nTab = 0; m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		m_aFavoriteInfo[nTab].m_pRecent->Terminate();
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

	::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );

	return;
}

/* ダイアログデータの1つのタブの設定・更新
	@param nIndex       タブのIndex
	@param nLvItemIndex 選択・表示したいListViewのIndex。-1で選択しない
*/
void CDlgFavorite::SetDataOne( int nIndex, int nLvItemIndex )
{
	HWND	hwndList;
	LV_ITEM	lvi;
	int		nNewFocus = -1;

	const CRecent*  pRecent = m_aFavoriteInfo[nIndex].m_pRecent;

	/* リスト */
	hwndList = ::GetDlgItem( GetHwnd(), m_aFavoriteInfo[nIndex].m_nId );
	ListView_DeleteAllItems( hwndList );  /* リストを空にする */

	const int   nViewCount = pRecent->GetViewCount();
	const int   nItemCount = pRecent->GetItemCount();
	m_aFavoriteInfo[nIndex].m_nViewCount = nViewCount;

	TCHAR	tmp[1024];
	for( int i = 0; i < nItemCount; i++ )
	{
		FormatFavoriteColm( tmp, _countof(tmp), i, i < nViewCount );
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = tmp;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = i;
		ListView_InsertItem( hwndList, &lvi );

		const TCHAR	*p;
		p = pRecent->GetItemText( i );
		auto_snprintf_s( tmp, _countof(tmp), _T("%ts"), p ? p : _T("") );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = tmp;
		ListView_SetItem( hwndList, &lvi );

		if( m_aFavoriteInfo[nIndex].m_bHaveFavorite )
		{
			ListView_SetCheckState( hwndList, i, (BOOL)pRecent->IsFavorite( i ) );
		}

	}

	if( -1 != m_aListViewInfo[nIndex].nSortColumn ){
		//ソートを維持
		ListViewSort( m_aListViewInfo[nIndex], pRecent, m_aListViewInfo[nIndex].nSortColumn, false );
	}

	if( -1 != nLvItemIndex && nLvItemIndex < nItemCount )
	{
		nNewFocus = nLvItemIndex;
	}

	//アイテムがあってどれも非選択なら、要求に近いアイテム(先頭か末尾)を選択
	if( nItemCount > 0 && -1 != nLvItemIndex && nNewFocus == -1 )
	{
		nNewFocus = (0 < nLvItemIndex ? nItemCount - 1: 0);
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
	int		nTab;

	for( nTab = 0; m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		if( m_aFavoriteInfo[nTab].m_bHaveFavorite )
		{
			GetFavorite( nTab );

			//リストを更新する。
			CRecent* pRecent = m_aFavoriteInfo[nTab].m_pRecent;
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
	int			nTab;
	POINT		po;
	long		lngStyle;

	_SetHwnd( hwndDlg );

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

	// リストビューのItem/SubItem幅を計算
	TCHAR* pszFAVORITE_TEXT = _T("お気に入り");
	const int nListViewWidthClient = rc.right - rc.left
		 - CTextWidthCalc::WIDTH_MARGIN_SCROLLBER - ::GetSystemMetrics(SM_CXVSCROLL);
	// 初期値は従来方式の%指定
	int nItemCx = nListViewWidthClient * 16 / 100;
	int nSubItem1Cx = nListViewWidthClient * 79 / 100;
	
	{
		// 適用されているフォントから算出
		CTextWidthCalc calc( hwndBaseList );
		calc.SetTextWidthIfMax( pszFAVORITE_TEXT, CTextWidthCalc::WIDTH_LV_HEADER );
		TCHAR szBuf[200];
		for(int i = 0; i < 40; i++ ){
			// 「M (非表示)」等の幅を求める
			FormatFavoriteColm( szBuf, _countof(szBuf), i, false);
			calc.SetTextWidthIfMax( szBuf, CTextWidthCalc::WIDTH_LV_ITEM_CHECKBOX );
		}
		
		if( 0 < calc.GetCx() ){
			nItemCx = calc.GetCx();
			nSubItem1Cx = nListViewWidthClient - nItemCx;
		}
	}

	for( nTab = 0; m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		CRecent* pRecent = m_aFavoriteInfo[nTab].m_pRecent;

		hwndList = GetDlgItem( hwndDlg, m_aFavoriteInfo[nTab].m_nId );
		m_aListViewInfo[nTab].hListView = hwndList;
		
		::MoveWindow( hwndList, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE );
		::ShowWindow( hwndList, SW_HIDE );

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = nItemCx;
		col.pszText  = pszFAVORITE_TEXT;
		col.iSubItem = 0;
		ListView_InsertColumn( hwndList, 0, &col );

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = nSubItem1Cx;
		col.pszText  = m_aFavoriteInfo[nTab].m_pszCaption;
		col.iSubItem = 1;
		ListView_InsertColumn( hwndList, 1, &col );

		/* 行選択 */
		lngStyle = ::SendMessageAny( hwndList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
		lngStyle |= LVS_EX_FULLROWSELECT;
		if( m_aFavoriteInfo[nTab].m_bHaveFavorite ) lngStyle |= LVS_EX_CHECKBOXES;
		::SendMessageAny( hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lngStyle );

		/* タブ項目追加 */
		tcitem.mask = TCIF_TEXT;
		tcitem.pszText = m_aFavoriteInfo[nTab].m_pszCaption;
		TabCtrl_InsertItem( hwndTab, nTab, &tcitem );
	}

	hwndList = ::GetDlgItem( hwndDlg, m_aFavoriteInfo[m_nCurrentTab].m_nId );
	::ShowWindow( hwndList, SW_SHOW );
	TabCtrl_SetCurSel( hwndTab, m_nCurrentTab );
	//ChangeSlider( m_nCurrentTab );

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgFavorite::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* ヘルプ */
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID( F_FAVORITE ) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	case IDOK:
		/* ダイアログデータの取得 */
		::EndDialog( GetHwnd(), (BOOL)GetData() );
		return TRUE;

	case IDCANCEL:
		// 2010.03.20 キャンセルを廃止。OKと同じにする。
		// [X]ボタンを押すと通過する
		::EndDialog( GetHwnd(), (BOOL)GetData() );
		return TRUE;

	// 2010.03.20 Moca 「お気に入り以外かすべて削除」選択メッセージボックスを廃止し
	//     それぞれのボタンに変更
	//すべて削除
	case IDC_BUTTON_CLEAR:
		{
			::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
			CRecent	*pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
			if( pRecent ){
				const int nRet = ConfirmMessage( GetHwnd(), 
					_T("最近使った%tsの履歴を削除します。\nよろしいですか？\n"),
					m_aFavoriteInfo[m_nCurrentTab].m_pszCaption );
				if( IDYES == nRet ){
					pRecent->DeleteAllItem();
					RefreshListOne( m_nCurrentTab );
				}
			}
		}
		return TRUE;
	//お気に入り以外削除
	case IDC_BUTTON_DELETE_NOFAVORATE:
		{
			::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
			if( m_aFavoriteInfo[m_nCurrentTab].m_bHaveFavorite ){
				int const nRet = ConfirmMessage( GetHwnd(), 
					_T("最近使った%tsの履歴のお気に入り以外を削除します。\nよろしいですか？"),
					m_aFavoriteInfo[m_nCurrentTab].m_pszCaption );
				CRecent * const pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
				if( IDYES == nRet && pRecent ){
					GetFavorite( m_nCurrentTab );
					pRecent->DeleteItemsNoFavorite();
					pRecent->UpdateView();
					RefreshListOne( m_nCurrentTab );
				}
			}
		}
		return TRUE;
	// 存在しない項目 を削除
	case IDC_BUTTON_DELETE_NOTFOUND:
		{
			::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
			if( m_aFavoriteInfo[m_nCurrentTab].m_bFilePath ){
				const int nRet = ConfirmMessage( GetHwnd(), 
					_T("最近使った%tsの存在しないパスを削除します。\n\nよろしいですか？"),
					m_aFavoriteInfo[m_nCurrentTab].m_pszCaption );
				CRecent * const pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
				if( IDYES == nRet && pRecent ){
					GetFavorite( m_nCurrentTab );

					// 存在しないパスの削除
					for( int i = pRecent->GetItemCount() - 1; i >= 0; i-- ){
						TCHAR szPath[_MAX_PATH];
						auto_strcpy( szPath, pRecent->GetItemText(i) );
						CutLastYenFromDirectoryPath(szPath);
						if( false == IsFileExists(szPath, false ) ){
							pRecent->DeleteItem(i);
						}
					}
					pRecent->UpdateView();
					RefreshListOne( m_nCurrentTab );
				}
			}
		}
		return TRUE;
	//選択項目の削除
	case IDC_BUTTON_DELETE_SELECTED:
		{
			DeleteSelected();
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

	hwndTab = ::GetDlgItem( GetHwnd(), IDC_TAB_FAVORITE );
	lpnmhdr = (LPNMHDR) lParam;
	if( lpnmhdr->hwndFrom == hwndTab )
	{
		switch( lpnmhdr->code )
		{
		case TCN_SELCHANGE:
			::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
			nIndex = TabCtrl_GetCurSel( hwndTab );
			if( -1 != nIndex )
			{
				//新しく表示する。
				hwndList = GetDlgItem( GetHwnd(), m_aFavoriteInfo[nIndex].m_nId );
				::ShowWindow( hwndList, SW_SHOW );

				//現在表示中のリストを隠す。
				hwndList = GetDlgItem( GetHwnd(), m_aFavoriteInfo[m_nCurrentTab].m_nId );
				::ShowWindow( hwndList, SW_HIDE );

				// お気に入り以外削除の有効・無効化
				DlgItem_Enable( GetHwnd(), IDC_BUTTON_DELETE_NOFAVORATE,
					m_aFavoriteInfo[nIndex].m_bHaveFavorite );

				DlgItem_Enable( GetHwnd(), IDC_BUTTON_DELETE_NOTFOUND,
					m_aFavoriteInfo[nIndex].m_bFilePath );

				::SetFocus( hwndList );

				m_nCurrentTab = nIndex;

				//ChangeSlider( nIndex );
			}
			return TRUE;
			//break;
		}
	}else{
		hwndList = m_aListViewInfo[m_nCurrentTab].hListView;
		if( hwndList == lpnmhdr->hwndFrom )
		{
			NM_LISTVIEW* pnlv = (NM_LISTVIEW*)lParam;
			switch( lpnmhdr->code )
			{
			// ListViewヘッダクリック:ソートする
			case LVN_COLUMNCLICK:
				ListViewSort(
					m_aListViewInfo[m_nCurrentTab],
					m_aFavoriteInfo[m_nCurrentTab].m_pRecent,
					pnlv->iSubItem, true );
				return TRUE;
			
			// ListViewでDeleteキーが押された:削除
			case LVN_KEYDOWN:
				switch( ((NMLVKEYDOWN*)lParam)->wVKey )
				{
				case VK_DELETE:
					DeleteSelected();
					return TRUE;
				}
			}
		}
	}

	/* 基底クラスメンバ */
	return CDialog::OnNotify( wParam, lParam );
}

BOOL CDlgFavorite::OnActivate( WPARAM wParam, LPARAM lParam )
{
	bool	bret;

	switch( LOWORD( wParam ) )
	{
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		bret = RefreshList();
		::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, m_szMsg );
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
		
			if( _tcslen( msg ) > 0 ) _tcscat( msg, _T("、") );
			_tcscat( msg, m_aFavoriteInfo[nTab].m_pszCaption );
		}
	}

	if( ret_val )
	{
		auto_snprintf_s( m_szMsg, _countof(m_szMsg),
			_T("履歴(%ts)が更新されたため編集中情報を破棄し再表示しました。"),
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

	CRecent*	pRecent = m_aFavoriteInfo[nIndex].m_pRecent;
	nItemCount    = pRecent->GetItemCount();
	hwndList      = GetDlgItem( GetHwnd(), m_aFavoriteInfo[nIndex].m_nId );
	nCount        = ListView_GetItemCount( hwndList );
	nCurrentIndex = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
	if( -1 == nCurrentIndex ) nCurrentIndex = ListView_GetNextItem( hwndList, -1, LVNI_FOCUSED );

	if( nItemCount != nCount ) goto changed;	//個数が変わったので再構築

	//お気に入り数が変わったので再構築
	if( m_aFavoriteInfo[nIndex].m_nViewCount != pRecent->GetViewCount() ) goto changed;

	for( i = 0; i < nCount; i++ )
	{
		TCHAR	szText[1024];
		auto_memset( szText, 0, _countof( szText ) );
		memset_raw( &lvitem, 0, sizeof( lvitem ) );
		lvitem.mask       = LVIF_TEXT | LVIF_PARAM;
		lvitem.pszText    = szText;
		lvitem.cchTextMax = _countof( szText );
		lvitem.iItem      = i;
		lvitem.iSubItem   = 1;
		bret = ListView_GetItem( hwndList, &lvitem );
		if( !bret ) goto changed;	//エラーなので再構築

		//アイテム内容が変わったので再構築
		if( lvitem.lParam != pRecent->FindItemByText( szText ) ) goto changed;
	}

	return false;

changed:
	SetDataOne( nIndex, nCurrentIndex );
	

	return true;
}

// お気に入りのフラグだけ適用
void CDlgFavorite::GetFavorite( int nIndex )
{
	CRecent * const pRecent  = m_aFavoriteInfo[nIndex].m_pRecent;
	const HWND      hwndList = m_aListViewInfo[nIndex].hListView;
	if( m_aFavoriteInfo[nIndex].m_bHaveFavorite ){
		const int nCount = ListView_GetItemCount( hwndList );
		for( int i = 0; i < nCount; i++ ){
			const int  recIndex = ListView_GetLParamInt( hwndList, i );
			const BOOL bret = ListView_GetCheckState( hwndList, i );
			pRecent->SetFavorite( recIndex, bret ? true : false );
		}
	}
}


/*
	選択中の項目を削除
	リストの更新もする
*/
int CDlgFavorite::DeleteSelected()
{
	::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
	const int nLIST_SUBITEM_TEXT = 1;
	int     nDelItemCount = 0;
	CRecent *pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
	if( pRecent ){
		HWND hwndList = m_aListViewInfo[m_nCurrentTab].hListView;
		int nSelectedCount = ListView_GetSelectedCount(hwndList);
		if( 0 < nSelectedCount ){
			GetFavorite( m_nCurrentTab );

			int nLastSelectedItem = -1;
			std::vector<int> selRecIndexs;
			{
				int nLvItem = -1;
				while( (nLvItem = ListView_GetNextItem(hwndList, nLvItem, LVNI_SELECTED)) != -1 )
				{
					int nRecIndex = ListView_GetLParamInt(hwndList, nLvItem);
					if( 0 <= nRecIndex ){
						selRecIndexs.push_back( nRecIndex );
						nLastSelectedItem = nLvItem;
					}
				}
			}
			std::sort(selRecIndexs.rbegin(),selRecIndexs.rend());
			// 大きいほうから削除しないと、CRecent側のindexがずれる
			for( size_t n = 0; n < selRecIndexs.size(); n++ )
			{
				pRecent->DeleteItem(selRecIndexs[n]);
				++nDelItemCount;
			}
			pRecent->UpdateView();
			if( 0 < nDelItemCount ){
				int nItem = nLastSelectedItem;
				if( -1 != nItem ){
					nItem += 1; // 削除したアイテムの次のアイテム
					nItem -= nDelItemCount; // 新しい位置は、削除した分だけずれる
					if( pRecent->GetItemCount() <= nItem ){
						// 旧データの最後の要素が削除されているときは、
						// 新データの最後を選択
						nItem = pRecent->GetItemCount() -1;
					}
				}
				int nLvTopIndex = ListView_GetTopIndex(hwndList);
				SetDataOne(m_nCurrentTab, nItem);
				if( 1 == nDelItemCount ){
					// 1つ削除のときは、Yスクロール位置を保持
					// 2つ以上は複雑なのでSetDataOneにおまかせする
					nLvTopIndex = t_max(0, t_min(pRecent->GetItemCount() - 1, nLvTopIndex));
					int nNowLvTopIndex = ListView_GetTopIndex(hwndList);
					if( nNowLvTopIndex != nLvTopIndex ){
						CMyRect rect;
						if( ListView_GetItemRect(hwndList, nNowLvTopIndex, &rect, LVIR_BOUNDS) ){
							// ListView_ScrollのY座標はpixel単位でスクロール変化分を指定
							ListView_Scroll(hwndList, 0,
								(nLvTopIndex - nNowLvTopIndex) * (rect.bottom - rect.top) );
						}
					}
				}
			}
		}
	}
	return nDelItemCount;
}

int FormatFavoriteColm(TCHAR* buf, int size, int index, bool view)
{
	// 2010.03.21 Moca Textに連番を設定することによってアクセスキーにする
	// 0 - 9 A - Z
	const int mod = index % 36;
	const TCHAR c = ((mod) <= 9)?(_T('0') + mod):(_T('A') + mod - 10);
	return auto_snprintf_s( buf, size, _T("%tc%ts"), c, (view ? _T("   ") : _T(" (非表示)")) );
}


/*!
	ListViewのItem(index)からLParamをint型として取得
*/
static int ListView_GetLParamInt( HWND hwndList, int lvIndex )
{
	LV_ITEM	lvitem;
	memset_raw( &lvitem, 0, sizeof(lvitem) );
	lvitem.mask = LVIF_PARAM;
	lvitem.iItem = lvIndex;
	lvitem.iSubItem = 0;
	if( ListView_GetItem( hwndList, &lvitem ) ){
		return (int)lvitem.lParam;
	}
	return -1;
}

/*!
	
	@param info [in,out] リストビューのソート状態情報
	@param pRecent       ソートアイテム
	@param colm          ソートしたい列番号
	@param bReverse      ソート済みの場合に降順に切り替える
*/
// static
void CDlgFavorite::ListViewSort(ListViewSortInfo& info, const CRecent* pRecent, int colm, bool bReverse )
{
	CompareListViewLParam lparamInfo;
	// ソート順の決定
	if( info.nSortColumn != colm ){
		info.bSortAscending = true;
	}else{
		// ソート逆順(降順)
		info.bSortAscending = (bReverse ? (!info.bSortAscending): true);
	}
	
	// ヘッダ書き換え
	TCHAR szHeader[200];
	LV_COLUMN	col;
	if( -1 != info.nSortColumn ){
		// 元のソートの「 ▼」を取り除く
		col.mask = LVCF_TEXT;
		col.pszText = szHeader;
		col.cchTextMax = _countof(szHeader);
		col.iSubItem = 0;
		ListView_GetColumn( info.hListView, info.nSortColumn, &col );
		int nLen = (int)_tcslen(szHeader) - _tcslen(_T("▼"));
		if( 0 <= nLen ){
			szHeader[nLen] = _T('\0');
		}
		col.mask = LVCF_TEXT;
		col.pszText = szHeader;
		col.iSubItem = 0;
		ListView_SetColumn( info.hListView, info.nSortColumn, &col );
	}
	// 「▼」を付加
	col.mask = LVCF_TEXT;
	col.pszText = szHeader;
	col.cchTextMax = _countof(szHeader) - 4;
	col.iSubItem = 0;
	ListView_GetColumn( info.hListView, colm, &col );
	_tcscat(szHeader, info.bSortAscending ? _T("▼") : _T("▲"));
	col.mask = LVCF_TEXT;
	col.pszText = szHeader;
	col.iSubItem = 0;
	ListView_SetColumn( info.hListView, colm, &col );

	info.nSortColumn = colm;

	lparamInfo.nSortColumn = colm;
	lparamInfo.hwndListView = info.hListView;
	lparamInfo.pRecent = pRecent;
	lparamInfo.bAbsOrder = info.bSortAscending;

	ListView_SortItems( info.hListView, CompareListViewFunc, (LPARAM)&lparamInfo );
}


static int CALLBACK CompareListViewFunc( LPARAM lParamItem1, LPARAM lParamItem2, LPARAM lParamSort )
{
	CompareListViewLParam* pCompInfo = (CompareListViewLParam*)lParamSort;
	int nRet = 0;
	if(0 == pCompInfo->nSortColumn){
		nRet = lParamItem1 - lParamItem2;
	}else{
		const CRecent* p = pCompInfo->pRecent;
		nRet = auto_stricmp(p->GetItemText((int)lParamItem1), p->GetItemText((int)lParamItem2));
	}
	return pCompInfo->bAbsOrder ? nRet : -nRet;
}
