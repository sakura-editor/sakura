//	$Id$
/*!	@file
	共通設定ダイアログボックス、「カスタムメニュー」ページ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "CPropCommon.h"
#include "CDlgInput1.h"
#include "CDlgOpenFile.h"
#include "etc_uty.h"

//	From Here Sept. 5, 2000 JEPRO 半角カタカナの全角化に伴い文字長を変更(27→46)
#define STR_CUSTMENU_HEAD_LEN 	46
#define STR_CUSTMENU_HEAD		"テキストエディタ カスタムメニュー設定ファイル\x1a"
//	To Here Sept. 5, 2000


//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//10100
	IDC_BUTTON_DELETE,				10100,	//メニューから機能削除
	IDC_BUTTON_INSERTSEPARATOR,		10101,	//セパレータ挿入
	IDC_BUTTON_INSERT,				10102,	//メニューへ機能挿入
	IDC_BUTTON_ADD,					10103,	//メニューへ機能追加
	IDC_BUTTON_UP,					10104,	//メニューの機能を上へ移動
	IDC_BUTTON_DOWN,				10105,	//メニューの機能を下へ移動
	IDC_BUTTON_IMPORT,				10106,	//インポート
	IDC_BUTTON_EXPORT,				10107,	//エクスポート
	IDC_COMBO_FUNCKIND,				10130,	//機能の種別
	IDC_COMBO_MENU,					10131,	//メニューの種別
	IDC_LIST_FUNC,					10150,	//機能一覧
	IDC_LIST_RES,					10151,	//メニュー一覧
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUCHOICE,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_MENU,					-1,
	IDC_LABEL_MENUKEYCHANGE,		-1,
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
BOOL CALLBACK CPropCommon::DlgProc_PROP_CUSTMENU(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_p8, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* p8 メッセージ処理 */
BOOL CPropCommon::DispatchEvent_p8(
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
	static HWND	hwndCOMBO_FUNCKIND;
	static HWND	hwndLIST_FUNC;
	static HWND	hwndCOMBO_MENU;
	static HWND	hwndLIST_RES;
//	static HWND	hwndEDIT_KEY;

//	int			nLength;
//	int			nAssignedKeyNum;
//	const char*	cpszString;

//	int			nIndex;
//	int			nIndex2;
//	int			nIndex3;
	int			i;
//	int			j;
//	int			nNum;
//	int			nFuncCode;

	int			nIdx1;
	int			nIdx2;
	int			nNum2;
	int			nIdx3;
	int			nIdx4;
	char		szLabel[300];
	char		szLabel2[300];
	char		szKey[2];
	int			nFunc;
	char		cKey;
//	WORD		vkey;		// virtual-key code
//	WORD		nCaretPos;	// caret position
//	HWND		hwndLB;		// handle of list box
//	char*		pszWork;
	CDlgInput1	cDlgInput1;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 p8 */
		SetData_p8( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* コントロールのハンドルを取得 */
		hwndCOMBO_FUNCKIND = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
		hwndLIST_FUNC = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
		hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
		hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
//		hwndEDIT_KEY = ::GetDlgItem( hwndDlg, IDC_EDIT_KEY );

//		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
//		::SendMessage( hwndEDIT_KEY, EM_LIMITTEXT, (WPARAM)1, 0 );


//		::SendMessage( hwndLIST_FUNC, LB_ADDSTRING, 0, (LPARAM)"テストテスト1" );
//		::SendMessage( hwndLIST_FUNC, LB_ADDSTRING, 0, (LPARAM)"テストテスト2" );
//		::SendMessage( hwndLIST_FUNC, LB_ADDSTRING, 0, (LPARAM)"テストテスト3" );
//		::SendMessage( hwndLIST_FUNC, LB_ADDSTRING, 0, (LPARAM)"テストテスト4" );


		/* キー選択時の処理 */
//		::SendMessage( hwndKeyList, LB_SETCURSEL, (WPARAM)0, (LPARAM)0 );
		::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_FUNCKIND );
//		::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );


		::SetTimer( hwndDlg, 1, 300, NULL );

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_CUSTMENU );
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE( "p8 PSN_KILLACTIVE\n" );
			/* ダイアログデータの取得 p8 */
			GetData_p8( hwndDlg );
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID = LOWORD(wParam);			/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl = (HWND) lParam;		/* コントロールのハンドル */

		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_BUTTON_IMPORT:	/* インポート */
				/* p8:カスタムメニュー設定をインポートする */
				p8_Import_CustMenuSetting( hwndDlg );
				return TRUE;
			case IDC_BUTTON_EXPORT:	/* エクスポート */
				/* p8:カスタムメニュー設定をエクスポートする */
				p8_Export_CustMenuSetting( hwndDlg );
				return TRUE;
			}
		}

		if( hwndCOMBO_MENU == hwndCtl ){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
				if( CB_ERR == nIdx1 ){
					break;
				}
				/* メニュー項目一覧に文字列をセット（リストボックス）*/
				::SendMessage( hwndLIST_RES, LB_RESETCONTENT, 0, 0 );
				for( i = 0; i < m_Common.m_nCustMenuItemNumArr[nIdx1]; ++i ){
					if( 0 == m_Common.m_nCustMenuItemFuncArr[nIdx1][i] ){
//						strcpy( szLabel2, "セパレータ" );
//						strcpy( szLabel2, "--------------------------------" );
						strcpy( szLabel2, " ─────────────" );	//Oct. 18, 2000 JEPRO 「ツールバー」タブで使っているセパレータと同じ線種に統一した
					}else{
						::LoadString( m_hInstance, m_Common.m_nCustMenuItemFuncArr[nIdx1][i], szLabel, 256 );
						/* キー */
						if( '\0' == m_Common.m_nCustMenuItemKeyArr[nIdx1][i] ){
							strcpy( szLabel2, szLabel );
						}else{
							wsprintf( szLabel2, "%s(%c)", szLabel, m_Common.m_nCustMenuItemKeyArr[nIdx1][i] );
						}
					}
			//		/* キー */
			//		if( '\0' == m_Common.nCustMenuItemKeyArr[nIdx][i] ||
			//			' '  == m_Common.nCustMenuItemKeyArr[nIdx][i] ){
			//			strcpy( szKey, "" );
			//		}else{
			//			sprintf( szKey, "%c", m_Common.nCustMenuItemKeyArr[nIdx][i] );
			//		}
			//		::SetWindowText( hwndEDIT_KEY, szKey );
					::SendMessage( hwndLIST_RES, LB_ADDSTRING, 0, (LPARAM)szLabel2 );
				}
//	From Here Sept. 7, 2000 JEPRO わかりにくいので選択しないようにコメントアウトに変更
//	(実際にはここでは選択されていないようなのでコメントアウトしなくても同じ。選択されているのはSetData_p8()の方)
//				/* カスタムメニューの先頭の項目を選択（リストボックス）*/
//				::SendMessage( hwndLIST_RES, LB_SETCURSEL, (WPARAM)0, (LPARAM)0 );
//	To Here Sept. 7, 2000
			}
		}else
		if( hwndLIST_RES == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_DBLCLK:
				nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
				if( LB_ERR == nIdx1 ){
					break;
				}
				nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
				if( LB_ERR == nIdx2 ){
					break;
				}

				if( 0 == m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2] ){
					break;
				}

//			idListBox = (int) LOWORD(wParam);	// identifier of list box
//			hwndListBox = (HWND) lParam;		// handle of list box
				wsprintf( szKey, "%c", m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] );
				if( FALSE == cDlgInput1.DoModal( m_hInstance, hwndDlg, "メニューアイテムのアクセスキー設定", "キーを入力してください。", 1, szKey ) ){
					return TRUE;
				}
				::LoadString( m_hInstance, m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2], szLabel, 255 );
//				if( ( szKey[0] == '\0' ) || ( '0' <= szKey[0] && szKey[0] <= '9' ) || ( 'A' <= szKey[0] && szKey[0] <= 'Z' ) ){
					m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = szKey[0];
					wsprintf( szLabel2, "%s(%c)", szLabel, m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] );
//				}else{
//					m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
//					sprintf( szLabel2, "%s", szLabel );
//				}
				::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2, (LPARAM)szLabel2 );
				::SendMessage( hwndLIST_RES, LB_DELETESTRING, nIdx2 + 1, 0 );


////			sprintf( szLabel2, "%c  %s", m_Common.m_nCustMenuItemKeyArr[nIdx1][nCaretPos], szLabel );

//				::SendMessage( hwndLIST_RES, LBN_SELCHANGE,  MAKELONG( IDC_LIST_RES, 0 ), (LPARAM)hwndLIST_RES );

				break;
			case LBN_SELCHANGE:
				nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
				if( LB_ERR == nIdx1 ){
//					::SetWindowText( hwndEDIT_KEY, "" );
					break;
				}

				if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_nCustMenuItemNumArr[nIdx1] ){
//					::SetWindowText( hwndEDIT_KEY, "" );
					break;
				}

				nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
				if( LB_ERR == nIdx2 ){
//					::SetWindowText( hwndEDIT_KEY, "" );
					break;
				}

				/* キー */
				if( '\0' == m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] ||
					' '  == m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] ){
//					strcpy( szKey, "" );
				}else{
//					sprintf( szKey, "%c", m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] );
				}
//				::SetWindowText( hwndEDIT_KEY, szKey );
			}
		}else
		if( hwndCOMBO_FUNCKIND == hwndCtl ){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIdx1 = ::SendMessage( hwndCOMBO_MENU, LB_GETCURSEL, 0, 0 );
				nIdx2 = ::SendMessage( hwndLIST_RES, CB_GETCURSEL, 0, 0 );
				nIdx3 = ::SendMessage( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
				nIdx4 = ::SendMessage( hwndLIST_FUNC, CB_GETCURSEL, 0, 0 );


				/* 機能一覧に文字列をセット（リストボックス）*/
//	Oct. 14, 2000 jepro note: ここのforブロックで実際にリストを書いているようである
				::SendMessage( hwndLIST_FUNC, LB_RESETCONTENT, 0, 0 );
				for( i = 0; i < nsFuncCode::pnFuncListNumArr[nIdx3]; ++i ){
					if( 0 < ::LoadString( m_hInstance, (nsFuncCode::ppnFuncListArr[nIdx3])[i], szLabel, 255 ) ){
						::SendMessage( hwndLIST_FUNC, LB_ADDSTRING, 0, (LPARAM)szLabel );
					}else{
						::SendMessage( hwndLIST_FUNC, LB_ADDSTRING, 0, (LPARAM)"--未定義--" );
					}
				}

				i = 0;
//				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
//					i |= _SHIFT;
//				}
//				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
//					i |= _CTRL;
//				}
//				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
//					i |= _ALT;
//				}
//				nFuncCode = m_pKeyNameArr[nIndex].m_nFuncCodeArr[i];
//				for( i = 0; i < nFincListNumArrNum; ++i ){
//					if( -1 != ( nIndex3 = SearchIntArr( nFuncCode, (int*)(nsFuncCode::ppnFuncListArr[i]), nsFuncCode::pnFuncListNumArr[i] ) ) ){
//						if( nIndex2 == i ){
//							::SendMessage( hwndFuncList, LB_SETCURSEL, (WPARAM)nIndex3, (LPARAM)0 );
//						}
//					}
//				}
				return TRUE;
			}
		}else{
			switch( wNotifyCode ){
			/* ボタン／チェックボックスがクリックされた */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_INSERTSEPARATOR:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
//					nIdx2 = ::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2, (LPARAM)"セパレータ" );	//Oct. 8, 2000 jepro 「ツールバー」タブ内のセパレータボタンで入る文字列
//					nIdx2 = ::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2, (LPARAM)"--------------------------------" );
					nIdx2 = ::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2, (LPARAM)" ─────────────" );	//Oct. 18, 2000 JEPRO 「ツールバー」タブで使っているセパレータと同じ線種に統一した
					if( nIdx2 == LB_ERR || nIdx2 == LB_ERRSPACE ){
						break;
					}
					::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );


					for( i = m_Common.m_nCustMenuItemNumArr[nIdx1]; i > nIdx2; i--){
						m_Common.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_nCustMenuItemFuncArr[nIdx1][i - 1];
						m_Common.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_nCustMenuItemKeyArr[nIdx1][i - 1];
					}
					m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2] = 0;
					m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
					m_Common.m_nCustMenuItemNumArr[nIdx1]++;

//					::SetWindowText( hwndEDIT_KEY, "" );
					break;
				case IDC_BUTTON_DELETE:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}

					if( 0 == m_Common.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						break;
					}
					nNum2 = ::SendMessage( hwndLIST_RES, LB_DELETESTRING, nIdx2, 0 );
					if( nNum2 == LB_ERR ){
						break;
					}

					for( i = nIdx2; i < m_Common.m_nCustMenuItemNumArr[nIdx1]; ++i ){
						m_Common.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_nCustMenuItemFuncArr[nIdx1][i + 1];
						m_Common.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_nCustMenuItemKeyArr[nIdx1][i + 1];
					}
					m_Common.m_nCustMenuItemNumArr[nIdx1]--;

					if( nNum2 > 0 ){
						if( nNum2 <= nIdx2 ){
							nIdx2 = nNum2 - 1;
						}
						nIdx2 = ::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );

//						sprintf( szKey, "%c", m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] );
//						::SetWindowText( hwndEDIT_KEY, szKey );
					}else{
//						::SetWindowText( hwndEDIT_KEY, "" );
					}
					break;


				case IDC_BUTTON_INSERT:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
					nIdx3 = ::SendMessage( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx3 ){
						break;
					}
					nIdx4 = ::SendMessage( hwndLIST_FUNC, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx4 ){
						break;
					}
					::SendMessage( hwndLIST_FUNC, LB_GETTEXT, nIdx4, (LPARAM)szLabel );

					for( i = m_Common.m_nCustMenuItemNumArr[nIdx1]; i > nIdx2; i-- ){
						m_Common.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_nCustMenuItemFuncArr[nIdx1][i - 1];
						m_Common.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_nCustMenuItemKeyArr[nIdx1][i - 1];
					}
					m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2] = nsFuncCode::ppnFuncListArr[nIdx3][nIdx4];
					m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
					m_Common.m_nCustMenuItemNumArr[nIdx1]++;

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2, (LPARAM)szLabel );
					if( LB_ERR == nIdx2 || LB_ERRSPACE == nIdx2 ){
						break;
					}
					::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );

//					sprintf( szKey, "%c", m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] );
//					::SetWindowText( hwndEDIT_KEY, szKey );

					break;


				case IDC_BUTTON_ADD:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
					nNum2 = ::SendMessage( hwndLIST_RES, LB_GETCOUNT, 0, 0 );
					if( LB_ERR == nNum2 ){
						nIdx2 = 0;
					}
					nIdx3 = ::SendMessage( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx3 ){
						break;
					}
					nIdx4 = ::SendMessage( hwndLIST_FUNC, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx4 ){
						break;
					}
					if( 0 == nsFuncCode::ppnFuncListArr[nIdx3][nIdx4] ){
						break;
					}

					::SendMessage( hwndLIST_FUNC, LB_GETTEXT, nIdx4, (LPARAM)szLabel );

					m_Common.m_nCustMenuItemFuncArr[nIdx1][nNum2] = nsFuncCode::ppnFuncListArr[nIdx3][nIdx4];
					m_Common.m_nCustMenuItemKeyArr[nIdx1][nNum2] = '\0';
					m_Common.m_nCustMenuItemNumArr[nIdx1]++;

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_ADDSTRING, nIdx2, (LPARAM)szLabel );
					if( LB_ERR == nIdx2 || LB_ERRSPACE == nIdx2 ){
						break;
					}
					::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );

//					sprintf( szKey, "%c", m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2] );
//					::SetWindowText( hwndEDIT_KEY, szKey );

					break;

				case IDC_BUTTON_UP:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}
					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						break;
					}
					if( 0 == nIdx2 ){
						break;
					}
					nFunc = m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2 - 1];
					cKey = m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2 - 1];
					m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2 - 1] = m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2];
					m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2 - 1]  = m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2];
					m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
					m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2]  = cKey;

					::SendMessage( hwndLIST_RES, LB_GETTEXT, nIdx2, (LPARAM)szLabel );
					::SendMessage( hwndLIST_RES, LB_DELETESTRING, nIdx2, 0 );
					::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2 - 1, (LPARAM)szLabel );
					::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2 - 1, 0 );
					break;

				case IDC_BUTTON_DOWN:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}
					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						break;
					}
					nNum2 = ::SendMessage( hwndLIST_RES, LB_GETCOUNT, 0, 0 );
					if( LB_ERR == nNum2 ){
						break;
					}
					if( nNum2 - 1 <= nIdx2 ){
						break;
					}
					nFunc = m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2 + 1];
					cKey = m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2 + 1];
					m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2 + 1] = m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2];
					m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2 + 1]  = m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2];
					m_Common.m_nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
					m_Common.m_nCustMenuItemKeyArr[nIdx1][nIdx2]  = cKey;

					::SendMessage( hwndLIST_RES, LB_GETTEXT, nIdx2, (LPARAM)szLabel );
					::SendMessage( hwndLIST_RES, LB_DELETESTRING, nIdx2, 0 );
					::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2 + 1, (LPARAM)szLabel );
					::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2 + 1, 0 );
					break;
				}

				break;
			}
		}
		break;

	case WM_TIMER:
		nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
		nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
		nIdx3 = ::SendMessage( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
		nIdx4 = ::SendMessage( hwndLIST_FUNC, LB_GETCURSEL, 0, 0 );
		i = ::SendMessage( hwndLIST_RES, LB_GETCOUNT, 0, 0 );
		if( LB_ERR == nIdx2	){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), TRUE );
			if( nIdx2 <= 0 ){
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
			}else{
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), TRUE );
			}
			if( nIdx2 + 1 >= i ){
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
			}else{
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), TRUE );
			}
		}
		if( LB_ERR == nIdx2 || LB_ERR == nIdx4 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), TRUE );
		}
		if( LB_ERR == nIdx4 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), TRUE );
		}
		if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_nCustMenuItemNumArr[nIdx1] ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERTSEPARATOR ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}
		if( LB_ERR != nIdx3 && LB_ERR != nIdx4 && 0 == nsFuncCode::ppnFuncListArr[nIdx3][nIdx4] ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}
		break;
//	case WM_VKEYTOITEM:
//		vkey = LOWORD(wParam);		// virtual-key code
//		nCaretPos = HIWORD(wParam);	// caret position
//		hwndLB = (HWND)lParam;		// handle of list box
//		MYTRACE( "WM_VKEYTOITEM  vkey=%d(%xh) nCaretPos=%d\n", vkey, vkey, nCaretPos );
//		nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
//		if( ( '0' <= vkey && vkey <= '9' ) || ( 'A' <= vkey && vkey <= 'Z' ) ){
//			m_Common.m_nCustMenuItemKeyArr[nIdx1][nCaretPos] = (char)vkey;
//
////			::SendMessage( hwndLIST_RES, LB_GETTEXT, nCaretPos, (LPARAM)szLabel );
////			sprintf( szLabel2, "%c  %s", m_Common.m_nCustMenuItemKeyArr[nIdx1][nCaretPos], szLabel );
////			::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nCaretPos, (LPARAM)szLabel2 );
////			::SendMessage( hwndLIST_RES, LB_DELETESTRING, nCaretPos + 1, 0 );
//		}
//		return -1;

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
		break;
//@@@ 2001.02.04 End

	}
	return FALSE;
}




/* ダイアログデータの設定 p8 */
void CPropCommon::SetData_p8( HWND hwndDlg )
{
	HWND		hwndCOMBO_MENU;
	HWND		hwndCombo;
//	HWND		hwndKeyList;	//Oct. 14, 2000 JEPRO killed
	HWND		hwndLIST_RES;
//	HWND		hwndEDIT_KEY;
	int			i;
	int			nIdx;
	char		szLabel[300];
	char		szLabel2[300];
//	char		szKey[2];

	/* 機能種別一覧に文字列をセット（コンボボックス） */
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	for( i = 0; i < nsFuncCode::nFuncKindNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)nsFuncCode::ppszFuncKind[i] );
	}
	/* 種別の先頭の項目を選択（コンボボックス）*/
//	::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)1, (LPARAM)0 );
	::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように大元 Funcode.cpp で変更してある

	/* キー一覧に文字列をセット（リストボックス）*/
//	Oct. 14, 2000 JEPRO note: ここのforブロックでは実際にはリストを書いていないようなのでコメントアウトした
/*
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
/	for( i = 0; i < m_nKeyNameArrNum; ++i ){
		::SendMessage( hwndKeyList, LB_ADDSTRING, 0, (LPARAM)m_pKeyNameArr[i].m_szKeyName );
	}
*/

	/* メニュー一覧に文字列をセット（コンボボックス）*/
	hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		::SendMessage( hwndCOMBO_MENU, CB_ADDSTRING, 0, (LPARAM)m_Common.m_szCustMenuNameArr[i] );
	}
	/* メニュー一覧の先頭の項目を選択（コンボボックス）*/
	::SendMessage( hwndCOMBO_MENU, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );

	/* メニュー項目一覧に文字列をセット（リストボックス）*/
	hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
//	hwndEDIT_KEY = ::GetDlgItem( hwndDlg, IDC_EDIT_KEY );
	::SendMessage( hwndLIST_RES, LB_RESETCONTENT, 0, 0 );
	nIdx = 0;
	for( i = 0; i < m_Common.m_nCustMenuItemNumArr[nIdx]; ++i ){
		if( 0 == m_Common.m_nCustMenuItemFuncArr[nIdx][i] ){
//			strcpy( szLabel, "セパレータ" );	//Oct. 8, 2000 jepro iniファイルやデフォルト設定などの情報から入る文字列
//			strcpy( szLabel, "--------------------------------" );
			strcpy( szLabel, " ─────────────" );	//Oct. 18, 2000 JEPRO 「ツールバー」タブで使っているセパレータと同じ線種に統一した
		}else{
			::LoadString( m_hInstance, m_Common.m_nCustMenuItemFuncArr[nIdx][i], szLabel, 256 );
		}
		/* キー */
		if( '\0' == m_Common.m_nCustMenuItemKeyArr[nIdx][i] ){
			strcpy( szLabel2, szLabel );
		}else{
			wsprintf( szLabel2, "%s(%c)", szLabel, m_Common.m_nCustMenuItemKeyArr[nIdx][i] );
		}
//	Oct. 14, 2000 JEPRO {}の対応を検索しやすいように部分的にコメントアウトせず、ペアになるように冗長でも上から次の１行をコピー
//		if( '\0' == m_Common.m_nCustMenuItemKeyArr[nIdx][i] ){
//			strcpy( szKey, "" );
//		}else{
//			sprintf( szKey, "%c", m_Common.nCustMenuItemKeyArr[nIdx][i] );
//		}
//		::SetWindowText( hwndEDIT_KEY, szKey );
		::SendMessage( hwndLIST_RES, LB_ADDSTRING, 0, (LPARAM)szLabel2 );
	}
//	/* カスタムメニューの先頭の項目を選択（リストボックス）*/	//Oct. 8, 2000 JEPRO ここをコメントアウトすると先頭項目が選択されなくなる
	::SendMessage( hwndLIST_RES, LB_SETCURSEL, (WPARAM)0, (LPARAM)0 );
	return;
}



/* ダイアログデータの取得 p8 */
int CPropCommon::GetData_p8( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_CUSTMENU;

	return TRUE;
}





/* p8:カスタムメニュー設定をインポートする */
void CPropCommon::p8_Import_CustMenuSetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char*			pszMRU = NULL;;
	char*			pszOPENFOLDER = NULL;;
	char			szPath[_MAX_PATH + 1];
	HFILE			hFile;
//	char			szLine[1024];
//	int				i;

	char			pHeader[STR_CUSTMENU_HEAD_LEN + 1];
//	short			nKeyNameArrNum;				/* キー割り当て表の有効データ数 */
//	KEYDATA			pKeyNameArr[100];			/* キー割り当て表 */
	HWND			hwndCtrl;
	char			szInitDir[_MAX_PATH + 1];

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* インポート用フォルダ */
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.mnu",
		szInitDir,
		(const char **)&pszMRU,
		(const char **)&pszOPENFOLDER
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );

	hFile = _lopen( szPath, OF_READ );
	if( HFILE_ERROR == hFile ){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"ファイルを開けませんでした。\n\n%s", szPath
		);
		return;
	}
	if( STR_CUSTMENU_HEAD_LEN						!= _lread( hFile, (LPVOID)pHeader, STR_CUSTMENU_HEAD_LEN ) ||
		sizeof( m_Common.m_szCustMenuNameArr	)	!= _lread( hFile, (LPVOID)&m_Common.m_szCustMenuNameArr   , sizeof( m_Common.m_szCustMenuNameArr    ) ) ||
		sizeof( m_Common.m_nCustMenuItemNumArr	)	!= _lread( hFile, (LPVOID)&m_Common.m_nCustMenuItemNumArr , sizeof( m_Common.m_nCustMenuItemNumArr  ) ) ||
		sizeof( m_Common.m_nCustMenuItemFuncArr	)	!= _lread( hFile, (LPVOID)&m_Common.m_nCustMenuItemFuncArr, sizeof( m_Common.m_nCustMenuItemFuncArr ) ) ||
		sizeof( m_Common.m_nCustMenuItemKeyArr	)	!= _lread( hFile, (LPVOID)&m_Common.m_nCustMenuItemKeyArr , sizeof( m_Common.m_nCustMenuItemKeyArr  ) ) ||
		0 != memcmp( pHeader, STR_CUSTMENU_HEAD, STR_CUSTMENU_HEAD_LEN )
	){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"カスタムメニュー設定ファイルの形式が違います。\n\n%s", szPath
		);
		return;
	}
	_lclose( hFile );

	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_MENU, CBN_SELCHANGE ), (LPARAM)hwndCtrl );

	return;
}

/* p8:カスタムメニュー設定をエクスポートする */
void CPropCommon::p8_Export_CustMenuSetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char*			pszMRU = NULL;;
	char*			pszOPENFOLDER = NULL;;
	char			szPath[_MAX_PATH + 1];
	HFILE			hFile;
//	char			szLine[1024];
//	int				i;
	char			szInitDir[_MAX_PATH + 1];

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* インポート用フォルダ */
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.mnu",
		szInitDir,
		(const char **)&pszMRU,
		(const char **)&pszOPENFOLDER
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );

	hFile = _lcreat( szPath, 0 );
	if( HFILE_ERROR == hFile ){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"ファイルを開けませんでした。\n\n%s", szPath
		);
		return;
	}
//	/* カスタムメニュー情報 */
//	char				m_szCustMenuNameArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_NAME_LEN + 1];
//	int					m_nCustMenuItemNumArr [MAX_CUSTOM_MENU];
//	int					m_nCustMenuItemFuncArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
//	char				m_nCustMenuItemKeyArr [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];

	if( STR_CUSTMENU_HEAD_LEN						!= _lwrite( hFile, (LPCSTR)STR_CUSTMENU_HEAD, STR_CUSTMENU_HEAD_LEN ) ||
		sizeof( m_Common.m_szCustMenuNameArr	)	!= _lwrite( hFile, (LPCSTR)&m_Common.m_szCustMenuNameArr   , sizeof( m_Common.m_szCustMenuNameArr    ) ) ||
		sizeof( m_Common.m_nCustMenuItemNumArr	)	!= _lwrite( hFile, (LPCSTR)&m_Common.m_nCustMenuItemNumArr , sizeof( m_Common.m_nCustMenuItemNumArr  ) ) ||
		sizeof( m_Common.m_nCustMenuItemFuncArr	)	!= _lwrite( hFile, (LPCSTR)&m_Common.m_nCustMenuItemFuncArr, sizeof( m_Common.m_nCustMenuItemFuncArr ) ) ||
		sizeof( m_Common.m_nCustMenuItemKeyArr	)	!= _lwrite( hFile, (LPCSTR)&m_Common.m_nCustMenuItemKeyArr , sizeof( m_Common.m_nCustMenuItemKeyArr  ) )
	){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"ファイルの書き込みに失敗しました。\n\n%s", szPath
		);
		return;
	}
	_lclose( hFile );

	return;

}


/*[EOF]*/
