/*!	@file
	共通設定ダイアログボックス、「ファイル名表示」ページ

	@author Moca
	@date 2002.12.09 Moca CPropTypesRegex.cppを参考にして作成
*/
/*
	Copyright (C) 2001, MIK
	Copyright (C) 2002, Moca, YAZAKI
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2004, D.S.Koba
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
#include "sakura_rc.h"
#include "CPropCommon.h"
#include "CShareData.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {	//13400
	IDC_LIST_FNAME,			HIDC_LIST_FNAME, 		// ファイル名置換リスト
	IDC_EDIT_FNAME_FROM,	HIDC_EDIT_FNAME_FROM,	// 置換前
	IDC_EDIT_FNAME_TO,		HIDC_EDIT_FNAME_TO,		// 置換後
	IDC_BUTTON_FNAME_INS,	HIDC_BUTTON_FNAME_INS,	// 挿入
	IDC_BUTTON_FNAME_ADD,	HIDC_BUTTON_FNAME_ADD,	// 追加
	IDC_BUTTON_FNAME_UPD,	HIDC_BUTTON_FNAME_UPD,	// 更新
	IDC_BUTTON_FNAME_DEL,	HIDC_BUTTON_FNAME_DEL,	// 削除
	IDC_BUTTON_FNAME_TOP,	HIDC_BUTTON_FNAME_TOP,	// 先頭
	IDC_BUTTON_FNAME_UP,	HIDC_BUTTON_FNAME_UP,	// 上へ
	IDC_BUTTON_FNAME_DOWN,	HIDC_BUTTON_FNAME_DOWN,	// 下へ
	IDC_BUTTON_FNAME_LAST,	HIDC_BUTTON_FNAME_LAST,	// 最終
//	IDC_CHECK_FNAME,		HIDC_CHECK_FNAME,	// ファイル名を簡易表示する
	0, 0 // 
};



INT_PTR CALLBACK CPropCommon::DlgProc_PROP_FILENAME(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_PROP_FILENAME, hwndDlg, uMsg, wParam, lParam );
}

INT_PTR CPropCommon::DispatchEvent_PROP_FILENAME( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	HWND	hListView;
	int		nIndex;
	TCHAR	szFrom[_MAX_PATH];
	TCHAR	szTo[_MAX_PATH];

	switch( uMsg ){

	case WM_INITDIALOG:
		{
			RECT		rc;
			LV_COLUMN	col;
			hListView = GetDlgItem( hwndDlg, IDC_LIST_FNAME );

			// Modified by KEITA for WIN64 2003.9.6
			::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );
			::GetWindowRect( hListView, &rc );
			col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			col.fmt      = LVCFMT_LEFT;
			col.cx       = ( rc.right - rc.left ) * 60 / 100;
			col.pszText  = _T("置換前");
			col.iSubItem = 0;
			ListView_InsertColumn( hListView, 0, &col );
			col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			col.fmt      = LVCFMT_LEFT;
			col.cx       = ( rc.right - rc.left ) * 35 / 100;
			col.pszText  = _T("置換後");
			col.iSubItem = 1;
			ListView_InsertColumn( hListView, 1, &col );

			// Apr. 28, 2003 Moca 初期化漏れ修正
			// ダイアログを開いたときにリストが選択されていてもフィールドが空の場合があった
			m_nLastPos_FILENAME = -1;

			// ダイアログデータの設定
			SetData_PROP_FILENAME( hwndDlg );

			// エディット コントロールに入力できるテキストの長さを制限する
			::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_FNAME_FROM ),  EM_LIMITTEXT, (WPARAM)( _MAX_PATH - 1 ), 0 );
			::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_FNAME_TO ),    EM_LIMITTEXT, (WPARAM)( _MAX_PATH - 1 ), 0 );
		}
		return TRUE;

	case WM_NOTIFY:
		{
			NMHDR*		pNMHDR = (NMHDR*)lParam;
			NM_UPDOWN*	pMNUD  = (NM_UPDOWN*)lParam;
			int			idCtrl = (int)wParam;

			switch( idCtrl ){
			case IDC_LIST_FNAME:
				switch( pNMHDR->code ){
				case LVN_ITEMCHANGED:
					hListView = GetDlgItem( hwndDlg, IDC_LIST_FNAME );
					nIndex = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
					// 未選択
					if( -1 == nIndex ){
						::SetDlgItemText( hwndDlg, IDC_EDIT_FNAME_FROM, _T("") );
						::SetDlgItemText( hwndDlg, IDC_EDIT_FNAME_TO, _T("") );
					}else if( nIndex != m_nLastPos_FILENAME ){
						GetListViewItem_FILENAME( hListView, nIndex, szFrom, szTo );
						::SetDlgItemText( hwndDlg, IDC_EDIT_FNAME_FROM, szFrom );
						::SetDlgItemText( hwndDlg, IDC_EDIT_FNAME_TO, szTo );
					}else{
						// nIndex == m_nLastPos_FILENAMEのとき
						// リスト→エディットボックスにデータをコピーすると[更新]がうまくうまく動作しない
					}
					m_nLastPos_FILENAME = nIndex;
					break;
				}
				break;
			default:
				switch( pNMHDR->code ){
				case PSN_HELP:
					OnHelp( hwndDlg, IDD_PROP_FNAME );
					return TRUE;
				case PSN_KILLACTIVE:
					// ダイアログデータの取得
					GetData_PROP_FILENAME( hwndDlg );
					return TRUE;
	//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
				case PSN_SETACTIVE:
					m_nPageNum = ID_PAGENUM_FILENAME;
					return TRUE;
				}
				break;
			}
		}
		break;

	case WM_COMMAND:
		{
			WORD	wNotifyCode = HIWORD(wParam);	// 通知コード
			WORD	wID = LOWORD(wParam);			// 項目ID､ コントロールID､ またはアクセラレータID
			HWND	hwndCtl = (HWND) lParam;		// コントロールのハンドル
			int		nCount;

			switch( wNotifyCode ){
			// ボタンがクリックされた
			case BN_CLICKED:
				hListView = GetDlgItem( hwndDlg, IDC_LIST_FNAME );
				nIndex = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
				switch( wID ){
				case IDC_BUTTON_FNAME_INS:	// 挿入
					// 選択中のキーを探す
					nCount = ListView_GetItemCount( hListView );
					if( -1 == nIndex ){
						// 選択中でなければ最後に追加
						nIndex = nCount;
					}
					::GetDlgItemText( hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH );
					::GetDlgItemText( hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH );

					if( -1 != SetListViewItem_FILENAME( hListView, nIndex, szFrom, szTo, true ) ){
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_ADD:	// 追加
					nCount = ListView_GetItemCount( hListView );

					::GetDlgItemText( hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH );
					::GetDlgItemText( hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH );
					
					if( -1 != SetListViewItem_FILENAME( hListView, nCount, szFrom, szTo, true ) ){
						return TRUE;
					}
					break;

				case IDC_BUTTON_FNAME_UPD:	// 更新
					if( -1 != nIndex ){
						::GetDlgItemText( hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH );
						::GetDlgItemText( hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH );
						if( -1 != SetListViewItem_FILENAME( hListView, nIndex, szFrom, szTo, false ) ){
							return TRUE;
						}
					}else{
						// 未選択でリストにひとつも項目がない場合は追加しておく
						if( 0 == ListView_GetItemCount( hListView ) ){
							if( -1 != SetListViewItem_FILENAME( hListView, 0, szFrom, szTo, true ) ){
								return TRUE;
							}
						}
					}
					break;
				case IDC_BUTTON_FNAME_DEL:	// 削除
					if( -1 != nIndex ){
						ListView_DeleteItem( hListView, nIndex );	//古いキーを削除
						ListView_SetItemState( hListView, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_TOP:	// 先頭
					if( -1 != MoveListViewItem_FILENAME( hListView, nIndex, 0 ) ){
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_UP: 	// 上へ
					if( -1 != MoveListViewItem_FILENAME( hListView, nIndex, nIndex - 1 ) ){
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_DOWN:	// 下へ
					if( -1 != MoveListViewItem_FILENAME( hListView, nIndex, nIndex + 1 ) ){
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_LAST:	// 最終
					nCount = ListView_GetItemCount( hListView );
					if( -1 != MoveListViewItem_FILENAME( hListView, nIndex, nCount - 1 ) ){
						return TRUE;
					}
					break;
				// default:
				}
				break;
			// default:
			}
		}

		break;	/* WM_COMMAND */
//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
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



/*!
	ダイアログ上のコントロールにデータを設定する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/
void CPropCommon::SetData_PROP_FILENAME( HWND hwndDlg )
{
	int nIndex;
	int i;
	LVITEM Item;

	// ファイル名を簡易表示する
//	if( FALSE != m_bUseTransformFileName ){
//		CheckDlgButton( hwndDlg, IDC_CHECK_FNAME, BST_CHECKED );
//	}else{
//		CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_UNCHECKED );
//	}
//	EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_FNAME ), FALSE );

	// ファイル名置換リスト
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_LIST_FNAME );
	ListView_DeleteAllItems( hListView ); // リストを空にする

	// リストにデータをセット
	for( i = 0, nIndex = 0; i < m_nTransformFileNameArrNum; i++ ){
		if( '\0' == m_szTransformFileNameFrom[i][0] ) continue;

		::ZeroMemory( &Item, sizeof( Item ));
		Item.mask     = LVIF_TEXT;
		Item.iItem    = nIndex;
		Item.iSubItem = 0;
		Item.pszText  = m_szTransformFileNameFrom[i];
		ListView_InsertItem( hListView, &Item );

		::ZeroMemory( &Item, sizeof( Item ));
		Item.mask     = LVIF_TEXT;
		Item.iItem    = nIndex;
		Item.iSubItem = 1;
		Item.pszText  = m_szTransformFileNameTo[i];
		ListView_SetItem( hListView, &Item );

		nIndex++;
	}

	// 一番上を選択しておく
	if( 0 != nIndex ){
		ListView_SetItemState( hListView, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}
	//	リストビューの行選択を可能にする．
	DWORD dwStyle;
	dwStyle = ListView_GetExtendedListViewStyle( hListView );
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( hListView, dwStyle );

	return;
}

/*!
	ダイアログ上のコントロールからデータを取得してメモリに格納する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/

int CPropCommon::GetData_PROP_FILENAME( HWND hwndDlg )
{

	int nIndex;
	int nCount;

	// ファイル名を簡易表示する
//	if( IsDlgButtonChecked( hwndDlg, IDC_CHECK_FNAME ) ){
//		m_bUseTransformFileName = TRUE;
//	}else{
//		m_bUseRegexKeyword = FALSE;
//	}

	// ファイル名置換リスト
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_LIST_FNAME );
	m_nTransformFileNameArrNum = ListView_GetItemCount( hListView );

	for( nIndex = 0, nCount = 0; nIndex < MAX_TRANSFORM_FILENAME ; ++nIndex ){
		if( nIndex < m_nTransformFileNameArrNum ){
			ListView_GetItemText( hListView, nIndex, 0, m_szTransformFileNameFrom[nCount], _MAX_PATH );

			// 置換前文字列がNULLだったら捨てる
			if( '\0' == m_szTransformFileNameFrom[nCount][0] ){
				m_szTransformFileNameTo[nIndex][0] = '\0';
			}else{
				ListView_GetItemText( hListView, nIndex, 1, m_szTransformFileNameTo[nCount], _MAX_PATH );
				nCount++;
			}
		}else{
			m_szTransformFileNameFrom[nIndex][0] = '\0';
			m_szTransformFileNameTo[nIndex][0] = '\0';
		}
	}

	return TRUE;
}


int CPropCommon::SetListViewItem_FILENAME( HWND hListView, int nIndex, LPTSTR szFrom, LPTSTR szTo, bool bInsMode )
{
	LV_ITEM	Item;
	int nCount;

	if( '\0' == szFrom[0] || -1 == nIndex ) return -1;

	nCount = ListView_GetItemCount( hListView );

	// これ以上追加できない
	if( bInsMode && MAX_TRANSFORM_FILENAME <= nCount ){
		::MessageBox( GetParent( hListView ), _T("これ以上登録できません。"), GSTR_APPNAME, MB_OK | MB_ICONSTOP );
		return -1;
	}

	::ZeroMemory( &Item, sizeof( Item ));
	Item.mask     = LVIF_TEXT;
	Item.iItem    = nIndex;
	Item.iSubItem = 0;
	Item.pszText  = szFrom;
	if( bInsMode ){
		ListView_InsertItem( hListView, &Item );
	}else{
		ListView_SetItem( hListView, &Item );
	}

	::ZeroMemory( &Item, sizeof( Item ));
	Item.mask     = LVIF_TEXT;
	Item.iItem    = nIndex;
	Item.iSubItem = 1;
	Item.pszText  = szTo;
	ListView_SetItem( hListView, &Item );

	ListView_SetItemState( hListView, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	return nIndex;
}


void CPropCommon::GetListViewItem_FILENAME( HWND hListView, int nIndex, LPTSTR szFrom, LPTSTR szTo )
{
	ListView_GetItemText( hListView, nIndex, 0, szFrom, _MAX_PATH );
	ListView_GetItemText( hListView, nIndex, 1, szTo, _MAX_PATH );
}


int CPropCommon::MoveListViewItem_FILENAME( HWND hListView, int nIndex, int nIndex2 )
{
	TCHAR szFrom[_MAX_PATH];
	TCHAR szTo[_MAX_PATH];
	int nCount = ListView_GetItemCount( hListView );

	//	2004.03.24 dskoba
	if( nIndex > nCount - 1 ){
		nIndex = nCount - 1;
	}
	if( nIndex2 > nCount - 1 ){
		nIndex2 = nCount - 1;
	}
	if( nIndex < 0 ){
		nIndex = 0;
	}
	if( nIndex2 < 0 ){
		nIndex2 = 0;
	}
	
	if( nIndex == nIndex2 ){
		return -1;
	}

	GetListViewItem_FILENAME( hListView, nIndex, szFrom, szTo );
	ListView_DeleteItem( hListView, nIndex );	//古いキーを削除
	SetListViewItem_FILENAME( hListView, nIndex2, szFrom, szTo, true );
	return nIndex2;
}
