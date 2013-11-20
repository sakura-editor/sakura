/*!	@file
	@brief 共通設定ダイアログボックス、「ツールバー」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro, MIK
	Copyright (C) 2002, genta, MIK, YAZAKI, aroka
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2005, aroka
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CPropCommon.h"
#include "CMenuDrawer.h" // 2002/2/10 aroka
#include "CImageListMgr.h" // 2005/8/9 aroka
#include "Debug.h" // 2002/2/10 aroka
#include "sakura.hh"


//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//11000
	IDC_BUTTON_DELETE,				HIDC_BUTTON_DELETE_TOOLBAR,				//ツールバーから機能削除
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_INSERTSEPARATOR_TOOLBAR,	//セパレータ挿入
	IDC_BUTTON_INSERT,				HIDC_BUTTON_INSERT_TOOLBAR,				//ツールバーへ機能挿入
	IDC_BUTTON_ADD,					HIDC_BUTTON_ADD_TOOLBAR,				//ツールバーへ機能追加
	IDC_BUTTON_UP,					HIDC_BUTTON_UP_TOOLBAR,					//ツールバーの機能を上へ移動
	IDC_BUTTON_DOWN,				HIDC_BUTTON_DOWN_TOOLBAR,				//ツールバーの機能を下へ移動
	IDC_CHECK_TOOLBARISFLAT,		HIDC_CHECK_TOOLBARISFLAT,				//フラットなボタン
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND_TOOLBAR,			//機能の種別
	IDC_LIST_FUNC,					HIDC_LIST_FUNC_TOOLBAR,					//機能一覧
	IDC_LIST_RES,					HIDC_LIST_RES_TOOLBAR,					//ツールバー一覧
	IDC_BUTTON_INSERTWRAP,			HIDC_BUTTON_INSERTWRAP,					//ツールバー折返	// 2006.08.06 ryoji
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_TOOLBAR,				-1,
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropToolbar::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropToolbar::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

//	From Here Apr. 13, 2002 genta
/*!
	Owner Draw List Boxに指定の値を挿入する (Windows XPの問題回避用)
	
	Windows XP + manifestの時にLB_INSERTSTRINGが値0を受け付けないので
	とりあえず0以外の値を入れてから0に設定し直して回避する。
	1回目の挿入は0でなければ何でもいいはず。
	
	@param hWnd [in] リストボックスのウィンドウハンドル
	@param index [in] 挿入位置
	@param value [in] 挿入する値
	@return 挿入位置。エラーの時はLB_ERRまたはLB_ERRSPACE
	
	@date 2002.04.13 genta 
*/
int Listbox_INSERTDATA(
	HWND hWnd,              //!< handle to destination window 
	int index,          //!< item index
	int value
)
{
	int nIndex1 = ::SendMessage( hWnd, LB_INSERTSTRING, index, 1 );
	if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
		TopErrorMessage( NULL, _T("Toolbar Dialog: 要素の挿入に失敗しました。(%d:%d)"), index, nIndex1 );
		return nIndex1;
	}
	else if( ::SendMessage( hWnd, LB_SETITEMDATA, nIndex1, value ) == LB_ERR ){
		TopErrorMessage( NULL, _T("Toolbar Dialog: INS: 値の設定に失敗しました。:%d"), nIndex1 );
		return LB_ERR;
	}
	return nIndex1;
}

//	From Here Apr. 13, 2002 genta
/*!
	Owner Draw List Boxに指定の値を追加する (Windows XPの問題回避用)
	
	Windows XP + manifestの時にLB_ADDSTRINGが値0を受け付けないので
	とりあえず0以外の値を入れてから0に設定し直して回避する。
	1回目の挿入は0でなければ何でもいいはず。
	
	@param hWnd [in] リストボックスのウィンドウハンドル
	@param index [in] 挿入位置
	@param value [in] 挿入する値
	@return 挿入位置。エラーの時はLB_ERRまたはLB_ERRSPACE
	
	@date 2002.04.13 genta 
*/
int Listbox_ADDDATA(
	HWND hWnd,              //!< handle to destination window 
	int index,          	//!< item index
	int value
)
{
	int nIndex1 = ::SendMessage( hWnd, LB_ADDSTRING, index, 1 );
	if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
		TopErrorMessage( NULL, _T("Toolbar Dialog: 要素の追加に失敗しました。(%d:%d)"), index, nIndex1 );
		return nIndex1;
	}
	else if( ::SendMessage( hWnd, LB_SETITEMDATA, nIndex1, value ) == LB_ERR ){
		TopErrorMessage( NULL, _T("Toolbar Dialog: ADD: 値の設定に失敗しました。:%d"), nIndex1 );
		return LB_ERR;
	}
	return nIndex1;
}

/* Toolbar メッセージ処理 */
INT_PTR CPropToolbar::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	NMHDR*				pNMHDR;
	int					idCtrl;
	static HWND			hwndCombo;
	static HWND			hwndFuncList;
	static HWND			hwndResList;
	LPDRAWITEMSTRUCT	pDis;
	int					nIndex1;
	int					nIndex2;
//	int					nIndex3;
	int					nNum;
	int					i;
	int					j;
	static char			pszLabel[256];
	HDC					hdc;
	TEXTMETRIC			tm;
	static int			nListItemHeight;
	LRESULT				lResult;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 Toolbar */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* コントロールのハンドルを取得 */
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
		hwndFuncList = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
		hwndResList = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

		hdc = ::GetDC( hwndDlg );
		::GetTextMetrics( hdc, &tm );
		::ReleaseDC( hwndDlg, hdc );
		nListItemHeight = 18/*18*/; //Oct. 18, 2000 JEPRO 「ツールバー」タブでの機能アイテムの行間を少し狭くして表示行数を増やした(20→18 これ以上小さくしても効果はないようだ)
		if( nListItemHeight < tm.tmHeight ){
			nListItemHeight = tm.tmHeight;
		}
//		nListItemHeight+=2;

//	From Here Oct.14, 2000 JEPRO added	(Ref. CPropComCustmenu.cpp 内のWM_INITDIALOGを参考にした)
		/* キー選択時の処理 */
		::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );
//	To Here Oct. 14, 2000

		::SetTimer( hwndDlg, 1, 300, NULL );

		return TRUE;

	case WM_DRAWITEM:
		idCtrl = (UINT) wParam;	/* コントロールのID */
		pDis = (LPDRAWITEMSTRUCT) lParam;	/* 項目描画情報 */
		switch( idCtrl ){
		case IDC_LIST_RES:	/* ツールバーボタン結果リスト */
		case IDC_LIST_FUNC:	/* ボタン一覧リスト */
			DrawToolBarItemList( pDis );	/* ツールバーボタンリストのアイテム描画 */
			return TRUE;
		}
		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_TOOLBAR );
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE( _T("PROP_TOOLBAR PSN_KILLACTIVE\n") );
			/* ダイアログデータの取得 Toolbar */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = ID_PROPCOM_PAGENUM_TOOLBAR;
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD( wParam );	/* 通知コード */
		wID = LOWORD( wParam );			/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl = (HWND) lParam;		/* コントロールのハンドル */

		if( hwndResList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				return TRUE;
			}
		}else
		if( hwndCombo == hwndCtl ){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );

				::SendMessage( hwndFuncList, LB_RESETCONTENT, 0, 0 );

				/* 機能一覧に文字列をセット (リストボックス) */
				//	From Here Oct. 15, 2001 genta Lookupを使うように変更
				nNum = m_cLookup.GetItemCount( nIndex2 );
				for( i = 0; i < nNum; ++i ){
					nIndex1 = m_cLookup.Pos2FuncCode( nIndex2, i );
					int nbarNo = m_pcMenuDrawer->FindToolbarNoFromCommandId( nIndex1 );

					if( nbarNo >= 0 ){
						/* ツールバーボタンの情報をセット (リストボックス) */
						lResult = ::Listbox_ADDDATA( hwndFuncList, 0, (LPARAM)nbarNo );
						if( lResult == LB_ERR || lResult == LB_ERRSPACE ){
							break;
						}
						lResult = ::SendMessage( hwndFuncList, LB_SETITEMHEIGHT , lResult, (LPARAM)MAKELPARAM(nListItemHeight, 0) );
					}

				}
				return TRUE;
			}
		}else{
			switch( wNotifyCode ){
			/* ボタン／チェックボックスがクリックされた */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_INSERTSEPARATOR:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex1 ){
//						break;
						nIndex1 = 0;
					}
					//	From Here Apr. 13, 2002 genta
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1, 0 );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
					//	To Here Apr. 13, 2002 genta
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
					break;

// 2005/8/9 aroka 折返ボタンが押されたら、右のリストに「ツールバー折返」を追加する。
				case IDC_BUTTON_INSERTWRAP:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex1 ){
//						break;
						nIndex1 = 0;
					}
					//	From Here Apr. 13, 2002 genta
					//	2010.06.25 Moca 折り返しのツールバーのボタン番号定数名を変更。最後ではなく固定値にする
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1, CMenuDrawer::TOOLBAR_BUTTON_F_TOOLBARWRAP );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
					//	To Here Apr. 13, 2002 genta
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
					break;

				case IDC_BUTTON_DELETE:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex1 ){
						break;
					}
					i = ::SendMessage( hwndResList, LB_DELETESTRING, nIndex1, 0 );
					if( i == LB_ERR ){
						break;
					}
					if( nIndex1 >= i ){
						if( i == 0 ){
							i = ::SendMessage( hwndResList, LB_SETCURSEL, 0, 0 );
						}else{
							i = ::SendMessage( hwndResList, LB_SETCURSEL, i - 1, 0 );
						}
					}else{
						i = ::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
					}
					break;

				case IDC_BUTTON_INSERT:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex1 ){
//						break;
						nIndex1 = 0;
					}
					nIndex2 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex2 ){
						break;
					}
					i = ::SendMessage( hwndFuncList, LB_GETITEMDATA, nIndex2, 0 );
					//	From Here Apr. 13, 2002 genta
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
					//	To Here Apr. 13, 2002 genta
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1 + 1, 0 );
					break;


				case IDC_BUTTON_ADD:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCOUNT, 0, 0 );
					nIndex2 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex2 ){
						break;
					}
					i = ::SendMessage( hwndFuncList, LB_GETITEMDATA, nIndex2, 0 );
					//	From Here Apr. 13, 2002 genta
					//	ここでは i != 0 だとは思うけど、一応保険です。
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						TopErrorMessage( NULL, _T("Toolbar Dialog: 要素の追加に失敗しました。:%d"), nIndex1 );
						break;
					}
					//	To Here Apr. 13, 2002 genta
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
					break;

				case IDC_BUTTON_UP:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex1 || 0 >= nIndex1 ){
						break;
					}
					i = ::SendMessage( hwndResList, LB_GETITEMDATA, nIndex1, 0 );

					j = ::SendMessage( hwndResList, LB_DELETESTRING, nIndex1, 0 );
					if( j == LB_ERR ){
						break;
					}
					//	From Here Apr. 13, 2002 genta
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1 - 1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						TopErrorMessage( NULL, _T("Toolbar Dialog: 要素の追加に失敗しました。:%d"), nIndex1 );
						break;
					}
					//	To Here Apr. 13, 2002 genta
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
					break;

				case IDC_BUTTON_DOWN:
					i = ::SendMessage( hwndResList, LB_GETCOUNT, 0, 0 );
					nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex1 || nIndex1 + 1 >= i ){
						break;
					}
					i = ::SendMessage( hwndResList, LB_GETITEMDATA, nIndex1, 0 );

					j = ::SendMessage( hwndResList, LB_DELETESTRING, nIndex1, 0 );
					if( j == LB_ERR ){
						break;
					}
					//	From Here Apr. 13, 2002 genta
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1 + 1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						TopErrorMessage( NULL, _T("Toolbar Dialog: 要素の追加に失敗しました。:%d"), nIndex1 );
						break;
					}
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
					//	To Here Apr. 13, 2002 genta
					break;
				}

				break;
			}
		}
		break;

	case WM_TIMER:
		nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
		nIndex2 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
		i = ::SendMessage( hwndResList, LB_GETCOUNT, 0, 0 );
		if( LB_ERR == nIndex1 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), TRUE );
			if( nIndex1 <= 0 ){
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
			}else{
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), TRUE );
			}
			if( nIndex1 + 1 >= i ){
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
			}else{
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), TRUE );
			}
		}
		if( LB_ERR == nIndex1 || LB_ERR == nIndex2 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), TRUE );
		}
		if( LB_ERR == nIndex2 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), TRUE );
		}
		break;

	case WM_DESTROY:
		::KillTimer( hwndDlg, 1 );
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

	}
	return FALSE;
}




/* ダイアログデータの設定 Toolbar */
void CPropToolbar::SetData( HWND hwndDlg )
{
	HWND		hwndCombo;
	HWND		hwndResList;
	HDC			hdc;
	int			i;
	int			nListItemHeight;
	LRESULT		lResult;
	TEXTMETRIC	tm;

	/* 機能種別一覧に文字列をセット(コンボボックス) */
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 15, 2001 genta
	
	/* 種別の先頭の項目を選択(コンボボックス) */
	::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO JEPRO 「--未定義--」を表示させないように大元 Funcode.cpp で変更してある
	::PostMessage( hwndCombo, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );

	/* コントロールのハンドルを取得 */
	hwndResList = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

	hdc = ::GetDC( hwndDlg );
	::GetTextMetrics( hdc, &tm );
	::ReleaseDC( hwndDlg, hdc );

	nListItemHeight = 18; //Oct. 18, 2000 JEPRO 「ツールバー」タブでのツールバーアイテムの行間を少し狭くして表示行数を増やした(20→18 これ以上小さくしても効果はないようだ)
	if( nListItemHeight < tm.tmHeight ){
		nListItemHeight = tm.tmHeight;
	}
//	nListItemHeight+=2;

	/* ツールバーボタンの情報をセット(リストボックス)*/
	for( i = 0; i < m_Common.m_sToolBar.m_nToolBarButtonNum; ++i ){
		//	From Here Apr. 13, 2002 genta
		lResult = ::Listbox_ADDDATA( hwndResList, 0, (LPARAM)m_Common.m_sToolBar.m_nToolBarButtonIdxArr[i] );
		if( lResult == LB_ERR || lResult == LB_ERRSPACE ){
			break;
		}
		//	To Here Apr. 13, 2002 genta
		lResult = ::SendMessage( hwndResList, LB_SETITEMHEIGHT, lResult, (LPARAM)MAKELPARAM(nListItemHeight, 0) );
	}
	/* ツールバーの先頭の項目を選択(リストボックス)*/
	::SendMessage( hwndResList, LB_SETCURSEL, 0, 0 );	//Oct. 14, 2000 JEPRO ここをコメントアウトすると先頭項目が選択されなくなる

	/* フラットツールバーにする／しない  */
	::CheckDlgButton( hwndDlg, IDC_CHECK_TOOLBARISFLAT, m_Common.m_sToolBar.m_bToolBarIsFlat );
	return;
}



/* ダイアログデータの取得 Toolbar */
int CPropToolbar::GetData( HWND hwndDlg )
{
	HWND	hwndResList;
	int		i;
	int		j;
	int		k;

	hwndResList = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

	/* ツールバーボタンの数 */
	m_Common.m_sToolBar.m_nToolBarButtonNum = ::SendMessage( hwndResList, LB_GETCOUNT, 0, 0 );

	/* ツールバーボタンの情報を取得 */
	k = 0;
	for( i = 0; i < m_Common.m_sToolBar.m_nToolBarButtonNum; ++i ){
		j = ::SendMessage( hwndResList, LB_GETITEMDATA, i, 0 );
		if( LB_ERR != j ){
			m_Common.m_sToolBar.m_nToolBarButtonIdxArr[k] = j;
			k++;
		}
	}
	m_Common.m_sToolBar.m_nToolBarButtonNum = k;

	/* フラットツールバーにする／しない  */
	m_Common.m_sToolBar.m_bToolBarIsFlat = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TOOLBARISFLAT );

	return TRUE;
}

/* ツールバーボタンリストのアイテム描画
	@date 2003.08.27 Moca システムカラーのブラシはCreateSolidBrushをやめGetSysColorBrushに
	@date 2005.08.09 aroka CPropCommon.cpp から移動
	@date 2007.11.02 ryoji ボタンとセパレータとで処理を分ける
*/
void CPropToolbar::DrawToolBarItemList( DRAWITEMSTRUCT* pDis )
{
	TBBUTTON	tbb;
	HBRUSH		hBrush;
	RECT		rc;
	RECT		rc0;
	RECT		rc1;
	RECT		rc2;


//	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
	hBrush = ::GetSysColorBrush( COLOR_WINDOW );
	::FillRect( pDis->hDC, &pDis->rcItem, hBrush );
//	::DeleteObject( hBrush );

	rc  = pDis->rcItem;
	rc0 = pDis->rcItem;
	rc0.left += 18;//20 //Oct. 18, 2000 JEPRO 行先頭のアイコンとそれに続くキャプションとの間を少し詰めた(20→18)
	rc1 = rc0;
	rc2 = rc0;

	if( (int)pDis->itemID < 0 ){
	}else{

//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
//		tbb = m_cShareData.m_tbMyButton[pDis->itemData];
//		tbb = m_pcMenuDrawer->m_tbMyButton[pDis->itemData];
		tbb = m_pcMenuDrawer->getButton(pDis->itemData);

		// ボタンとセパレータとで処理を分ける	2007.11.02 ryoji
		char	szLabel[256];
		if( tbb.fsStyle & TBSTYLE_SEP ){
			// テキストだけ表示する
			if( tbb.idCommand == F_DISABLE ){
				strcpy( szLabel, "───────────" );	// nLength 未使用 2003/01/09 Moca
			}else if( tbb.idCommand == F_MENU_NOT_USED_FIRST ){
				if( ::LoadString( m_hInstance, tbb.idCommand, szLabel, _countof( szLabel ) ) <= 0 ){
					strcpy( szLabel, "――ツールバー折返――" );
				}
			}else{
				strcpy( szLabel, "────不　明────" );
			}
		//	From Here Oct. 15, 2001 genta
		}else{
			// アイコンとテキストを表示する
			m_pcIcons->Draw( tbb.iBitmap, pDis->hDC, rc.left + 2, rc.top + 2, ILD_NORMAL );
			m_cLookup.Funccode2Name( tbb.idCommand, szLabel, _countof( szLabel ) );
		}
		//	To Here Oct. 15, 2001 genta

		/* アイテムが選択されている */
		if( pDis->itemState & ODS_SELECTED ){
//			hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_HIGHLIGHT ) );
			hBrush = ::GetSysColorBrush( COLOR_HIGHLIGHT );
			::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );
		}else{
//			hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
			hBrush = ::GetSysColorBrush( COLOR_WINDOW );
			::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_WINDOWTEXT ) );
		}
		rc1.left++;
		rc1.top++;
		rc1.right--;
		rc1.bottom--;
		::FillRect( pDis->hDC, &rc1, hBrush );
//		::DeleteObject( hBrush );

		::SetBkMode( pDis->hDC, TRANSPARENT );
		::TextOut( pDis->hDC, rc1.left + 4, rc1.top + 2, szLabel, _tcslen( szLabel ) );

	}

	/* アイテムにフォーカスがある */
	if( pDis->itemState & ODS_FOCUS ){
		::DrawFocusRect( pDis->hDC, &rc2 );
	}
	return;
}


/*[EOF]*/
