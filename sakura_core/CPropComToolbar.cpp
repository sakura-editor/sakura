//	$Id$
/*!	@file
	@brief 共通設定ダイアログボックス、「バックアップ」ページ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "CPropCommon.h"


//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
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
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_TOOLBAR,				-1,
//	IDC_STATIC,						-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//11000
	IDC_BUTTON_DELETE,				11000,	//ツールバーから機能削除
	IDC_BUTTON_INSERTSEPARATOR,		11001,	//セパレータ挿入
	IDC_BUTTON_INSERT,				11002,	//ツールバーへ機能挿入
	IDC_BUTTON_ADD,					11003,	//ツールバーへ機能追加
	IDC_BUTTON_UP,					11004,	//ツールバーの機能を上へ移動
	IDC_BUTTON_DOWN,				11005,	//ツールバーの機能を下へ移動
	IDC_CHECK_TOOLBARISFLAT,		11010,	//フラットなボタン
	IDC_COMBO_FUNCKIND,				11030,	//機能の種別
	IDC_LIST_FUNC,					11040,	//機能一覧
	IDC_LIST_RES,					11041,	//ツールバー一覧
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_TOOLBAR,				-1,
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
BOOL CALLBACK CPropCommon::DlgProc_PROP_TOOLBAR(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_p6, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* p6 メッセージ処理 */
BOOL CPropCommon::DispatchEvent_p6(
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
	NM_UPDOWN*			pMNUD;
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
		/* ダイアログデータの設定 p6 */
		SetData_p6( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

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
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_TOOLBAR );
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE( "p6 PSN_KILLACTIVE\n" );
			/* ダイアログデータの取得 p6 */
			GetData_p6( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = ID_PAGENUM_TOOLBAR;
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
//				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );

				::SendMessage( hwndFuncList, LB_RESETCONTENT, 0, 0 );
//				nNum = ::SendMessage( hwndFuncList, LB_GETCOUNT, 0, 0 );
//				for( i = 0; i < nNum; ++i ){
//					::SendMessage( hwndFuncList, LB_DELETESTRING, 0, 0 );
//				}
				/* 機能一覧に文字列をセット (リストボックス) */
				//	From Here Oct. 15, 2001 genta Lookupを使うように変更
				nNum = m_cLookup.GetItemCount( nIndex2 );
				for( i = 0; i < nNum; ++i ){
					nIndex1 = m_cLookup.Pos2FuncCode( nIndex2, i );
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
//					for( j = 0; j < m_cShareData.m_nMyButtonNum; ++j ){
//						if( m_cShareData.m_tbMyButton[j].idCommand == nIndex1 ){
					for( j = 0; j < m_pcMenuDrawer->m_nMyButtonNum; ++j ){
						if( m_pcMenuDrawer->m_tbMyButton[j].idCommand == nIndex1 ){	//	jは、nIndex1で指定された機能コードを持つ
				//	To Here Oct. 15, 2001 genta Lookupを使うように変更
							break;
						}
					}
//jepro note: 次行不要???
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
//					if( j < m_cShareData.m_nMyButtonNum ){
					if( j < m_pcMenuDrawer->m_nMyButtonNum ){

//						/* ツールバーボタンの情報をセット (リストボックス) */
//						for( i = 0; i < m_Common.m_nToolBarButtonNum; ++i ){
							lResult = ::SendMessage( hwndFuncList, LB_ADDSTRING, 0, (LPARAM)j );
							lResult = ::SendMessage( hwndFuncList, LB_SETITEMHEIGHT , lResult, (LPARAM)MAKELPARAM(nListItemHeight, 0) );
//						}
					}

//jeprotest コメントアウトされていた以下の５行のコメントアウトを解除するとエラーが出る
//					if( 0 < ::LoadString( m_hInstance, (nsFuncCode::ppnFuncListArr[nIndex2])[i], pszLabel, sizeof(pszLabel) ) ){
//						::SendMessage( hwndFuncList, LB_ADDSTRING, 0, (LPARAM)pszLabel );
//					}else{
//						::SendMessage( hwndFuncList, LB_ADDSTRING, 0, (LPARAM)"--未定義--" );
//					}
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
					nIndex1 = ::SendMessage( hwndResList, LB_INSERTSTRING, nIndex1, 0 );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
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
					nIndex1 = ::SendMessage( hwndResList, LB_INSERTSTRING, nIndex1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1 + 1, 0 );
					break;


				case IDC_BUTTON_ADD:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCOUNT, 0, 0 );
					nIndex2 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex2 ){
						break;
					}
					i = ::SendMessage( hwndFuncList, LB_GETITEMDATA, nIndex2, 0 );
					nIndex1 = ::SendMessage( hwndResList, LB_INSERTSTRING, nIndex1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
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
					nIndex1 = ::SendMessage( hwndResList, LB_INSERTSTRING, nIndex1 - 1, i );
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
					nIndex1 = ::SendMessage( hwndResList, LB_INSERTSTRING, nIndex1 + 1, i );
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
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
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)p_helpids );
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}




/* ダイアログデータの設定 p6 */
void CPropCommon::SetData_p6( HWND hwndDlg )
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
//	for( i = 0; i < nsFuncCode::nFuncKindNum; ++i ){
//		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)nsFuncCode::ppszFuncKind[i] );
//	}
	
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
	for( i = 0; i < m_Common.m_nToolBarButtonNum; ++i ){
		lResult = ::SendMessage( hwndResList, LB_ADDSTRING, 0, (LPARAM)m_Common.m_nToolBarButtonIdxArr[i] );
		lResult = ::SendMessage( hwndResList, LB_SETITEMHEIGHT , lResult, (LPARAM)MAKELPARAM(nListItemHeight, 0) );
	}
//	/* ツールバーの先頭の項目を選択(リストボックス)*/
	::SendMessage( hwndResList, LB_SETCURSEL, 0, 0 );	//Oct. 14, 2000 JEPRO ここをコメントアウトすると先頭項目が選択されなくなる

	/* フラットツールバーにする／しない  */
	::CheckDlgButton( hwndDlg, IDC_CHECK_TOOLBARISFLAT, m_Common.m_bToolBarIsFlat );
	return;
}



/* ダイアログデータの取得 p6 */
int CPropCommon::GetData_p6( HWND hwndDlg )
{
	HWND	hwndResList;
	int		i;
	int		j;
	int		k;

//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = ID_PAGENUM_TOOLBAR;


	hwndResList = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

	/* ツールバーボタンの数 */
	m_Common.m_nToolBarButtonNum = ::SendMessage( hwndResList, LB_GETCOUNT, 0, 0 );

	/* ツールバーボタンの情報を取得 */
	k = 0;
	for( i = 0; i < m_Common.m_nToolBarButtonNum; ++i ){
		j = ::SendMessage( hwndResList, LB_GETITEMDATA, i, 0 );
		if( LB_ERR != j ){
			m_Common.m_nToolBarButtonIdxArr[k] = j;
			k++;
		}
	}
	m_Common.m_nToolBarButtonNum = k;

	/* フラットツールバーにする／しない  */
	m_Common.m_bToolBarIsFlat = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TOOLBARISFLAT );

	return TRUE;
}


/*[EOF]*/
