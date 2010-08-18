/*!	@file
	共通設定ダイアログボックス、「カスタムメニュー」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK, jepro
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "dlg/CDlgInput1.h"
#include "env/CShareData.h"
#include "util/shell.h"
#include "typeprop/CImpExpManager.h"	// 20210/4/23 Uchi
#include "sakura_rc.h"
#include "sakura.hh"

using namespace std;

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10100
	IDC_BUTTON_DELETE,				HIDC_BUTTON_DELETE,				//メニューから機能削除
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_INSERTSEPARATOR,	//セパレータ挿入
	IDC_BUTTON_INSERT,				HIDC_BUTTON_INSERT,				//メニューへ機能挿入
	IDC_BUTTON_ADD,					HIDC_BUTTON_ADD,				//メニューへ機能追加
	IDC_BUTTON_UP,					HIDC_BUTTON_UP,					//メニューの機能を上へ移動
	IDC_BUTTON_DOWN,				HIDC_BUTTON_DOWN,				//メニューの機能を下へ移動
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT,				//インポート
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT,				//エクスポート
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND,			//機能の種別
	IDC_COMBO_MENU,					HIDC_COMBO_MENU,				//メニューの種別
	IDC_EDIT_MENUNAME,				HIDC_EDIT_MENUNAME,				//メニュー名		// 2009.02.20 ryoji
	IDC_BUTTON_MENUNAME,			HIDC_BUTTON_MENUNAME,			//メニュー名設定	// 2009.02.20 ryoji
	IDC_LIST_FUNC,					HIDC_LIST_FUNC,					//機能一覧
	IDC_LIST_RES,					HIDC_LIST_RES,					//メニュー一覧
//	IDC_LABEL_MENUFUNCKIND,			-1,
//	IDC_LABEL_MENUCHOICE,			-1,
//	IDC_LABEL_MENUFUNC,				-1,
//	IDC_LABEL_MENU,					-1,
//	IDC_LABEL_MENUKEYCHANGE,		-1,
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
INT_PTR CALLBACK CPropCustmenu::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* Custom menu メッセージ処理 */
INT_PTR CPropCustmenu::DispatchEvent(
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

	int			i;

	int			nIdx1;
	int			nIdx2;
	int			nNum2;
	int			nIdx3;
	int			nIdx4;
	WCHAR		szLabel[300];
	WCHAR		szLabel2[300];

	CDlgInput1	cDlgInput1;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 Custom menu */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* コントロールのハンドルを取得 */
		hwndCOMBO_FUNCKIND = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
		hwndLIST_FUNC = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
		hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
		hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

		/* キー選択時の処理 */
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_FUNCKIND );

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
//			MYTRACE_A( "Custom menu PSN_KILLACTIVE\n" );
			/* ダイアログデータの取得 Custom menu */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = ID_PAGENUM_CUSTMENU;

			// 表示を更新する（マクロ設定画面でのマクロ名変更を反映）	// 2007.11.02 ryoji
			nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
			nIdx2 = List_GetCurSel( hwndLIST_RES );
			nIdx3 = Combo_GetCurSel( hwndCOMBO_FUNCKIND );
			nIdx4 = List_GetCurSel( hwndLIST_FUNC );
			if( nIdx1 != CB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_MENU, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_MENU );
				if( nIdx2 != LB_ERR ){
					List_SetCurSel( hwndLIST_RES, nIdx2 );
				}
			}
			if( nIdx3 != CB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_FUNCKIND );
				if( nIdx4 != LB_ERR ){
					List_SetCurSel( hwndLIST_FUNC, nIdx4 );
				}
			}
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
				/* カスタムメニュー設定をインポートする */
				Import( hwndDlg );
				return TRUE;
			case IDC_BUTTON_EXPORT:	/* エクスポート */
				/* カスタムメニュー設定をエクスポートする */
				Export( hwndDlg );
				return TRUE;
			case IDC_BUTTON_MENUNAME:
				//	メニュー文字列の設定
				nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
				if( CB_ERR == nIdx1 ){
					break;
				}
				::DlgItem_GetText( hwndDlg, IDC_EDIT_MENUNAME,
					m_Common.m_sCustomMenu.m_szCustMenuNameArr[nIdx1], MAX_CUSTOM_MENU_NAME_LEN );
				//	Combo Boxも変更 削除＆再登録
				Combo_DeleteString( hwndCOMBO_MENU, nIdx1 );
				Combo_InsertString( hwndCOMBO_MENU, nIdx1,
					m_Common.m_sCustomMenu.m_szCustMenuNameArr[nIdx1] );
				// 削除すると選択が解除されるので，元に戻す
				Combo_SetCurSel( hwndCOMBO_MENU, nIdx1 );
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}

		if( hwndCOMBO_MENU == hwndCtl ){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
				if( CB_ERR == nIdx1 ){
					break;
				}
				/* メニュー項目一覧に文字列をセット（リストボックス）*/
				List_ResetContent( hwndLIST_RES );
				for( i = 0; i < m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; ++i ){
					if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] ){
						auto_strcpy( szLabel2, LTEXT(" ─────────────") );	//Oct. 18, 2000 JEPRO 「ツールバー」タブで使っているセパレータと同じ線種に統一した
					}else{
						//	Oct. 3, 2001 genta
						m_cLookup.Funccode2Name( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i], szLabel, 256 );
						/* キー */
						if( '\0' == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] ){
							auto_strcpy( szLabel2, szLabel );
						}else{
							auto_sprintf( szLabel2, LTEXT("%ls(%hc)"), szLabel, m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] );
						}
					}
					::List_AddString( hwndLIST_RES, szLabel2 );
				}
				//	Oct. 15, 2001 genta メニュー名を設定
				::DlgItem_SetText( hwndDlg, IDC_EDIT_MENUNAME, m_Common.m_sCustomMenu.m_szCustMenuNameArr[nIdx1] );
				
				break;	/* CBN_SELCHANGE */
			}
		}else
		if( hwndLIST_RES == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_DBLCLK:
				nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
				if( CB_ERR == nIdx1 ){
					break;
				}
				nIdx2 = List_GetCurSel( hwndLIST_RES );
				if( LB_ERR == nIdx2 ){
					break;
				}

				if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] ){
					break;
				}

//			idListBox = (int) LOWORD(wParam);	// identifier of list box
//			hwndListBox = (HWND) lParam;		// handle of list box
				TCHAR		szKey[2];
				auto_sprintf( szKey, _T("%hc"), m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] );
				{
					BOOL bDlgInputResult = cDlgInput1.DoModal(
						G_AppInstance(),
						hwndDlg,
						_T("メニューアイテムのアクセスキー設定"),
						_T("キーを入力してください。"),
						1,
						szKey
					);
					if( !bDlgInputResult ){
						return TRUE;
					}
				}
				//	Oct. 3, 2001 genta
				m_cLookup.Funccode2Name( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2], szLabel, 255 );
				//::LoadString( G_AppInstance(), m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2], szLabel, 255 );

				{
					KEYCODE keycode[3]={0}; _tctomb(szKey, keycode);
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = keycode[0];
				}
//@@@ 2002.01.08 YAZAKI カスタムメニューでアクセスキーを消した時、左カッコ ( がメニュー項目に一回残るバグ修正
				if (m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]){
					auto_sprintf( szLabel2, LTEXT("%ts(%hc)"),
						szLabel,
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]
					);
				}
				else {
					auto_sprintf( szLabel2, LTEXT("%ls"), szLabel );
				}

				List_InsertString( hwndLIST_RES, nIdx2, szLabel2 );
				List_DeleteString( hwndLIST_RES, nIdx2 + 1 );

				break;
			case LBN_SELCHANGE:
				nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
				if( CB_ERR == nIdx1 ){
					break;
				}

				if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
					break;
				}

				nIdx2 = List_GetCurSel( hwndLIST_RES );
				if( LB_ERR == nIdx2 ){
					break;
				}

				/* キー */
				if( '\0' == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] ||
					' '  == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] ){
				}else{
				}
				break;	/* LBN_SELCHANGE */
			}
		}
		else if( hwndCOMBO_FUNCKIND == hwndCtl ){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIdx3 = Combo_GetCurSel( hwndCOMBO_FUNCKIND );

				/* 機能一覧に文字列をセット（リストボックス）*/
//	Oct. 14, 2000 jepro note: ここのforブロックで実際にリストを書いているようである
				// Oct. 3, 2001 genta
				// 専用ルーチンに置き換え
				m_cLookup.SetListItem( hwndLIST_FUNC, nIdx3 );
				return TRUE;
			}
		}else{
			switch( wNotifyCode ){
			/* ボタン／チェックボックスがクリックされた */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_INSERTSEPARATOR:
					nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
					if( CB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = List_GetCurSel( hwndLIST_RES );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
					nIdx2 = List_InsertString( hwndLIST_RES, nIdx2, L" ─────────────" );	//Oct. 18, 2000 JEPRO 「ツールバー」タブで使っているセパレータと同じ線種に統一した
					if( nIdx2 == LB_ERR || nIdx2 == LB_ERRSPACE ){
						break;
					}
					List_SetCurSel( hwndLIST_RES, nIdx2 );


					for( i = m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; i > nIdx2; i--){
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i - 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i - 1];
					}
					m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] = F_0;
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

//					::SetWindowText( hwndEDIT_KEY, L"" );
					break;
				case IDC_BUTTON_DELETE:
					nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
					if( CB_ERR == nIdx1 ){
						break;
					}

					if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = List_GetCurSel( hwndLIST_RES );
					if( LB_ERR == nIdx2 ){
						break;
					}
					nNum2 = List_DeleteString( hwndLIST_RES, nIdx2 );
					if( nNum2 == LB_ERR ){
						break;
					}

					for( i = nIdx2; i < m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; ++i ){
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i + 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i + 1];
					}
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]--;

					if( nNum2 > 0 ){
						if( nNum2 <= nIdx2 ){
							nIdx2 = nNum2 - 1;
						}
						nIdx2 = List_SetCurSel( hwndLIST_RES, nIdx2 );

					}else{
					}
					break;


				case IDC_BUTTON_INSERT:
					nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
					if( CB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = List_GetCurSel( hwndLIST_RES );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
					nIdx3 = Combo_GetCurSel( hwndCOMBO_FUNCKIND );
					if( CB_ERR == nIdx3 ){
						break;
					}
					nIdx4 = List_GetCurSel( hwndLIST_FUNC );
					if( LB_ERR == nIdx4 ){
						break;
					}
					List_GetText( hwndLIST_FUNC, nIdx4, szLabel );

					for( i = m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; i > nIdx2; i-- ){
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i - 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i - 1];
					}
					//	Oct. 3, 2001 genta
					m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] = m_cLookup.Pos2FuncCode( nIdx3, nIdx4 );
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

					nIdx2 = List_InsertString( hwndLIST_RES, nIdx2, szLabel );
					if( LB_ERR == nIdx2 || LB_ERRSPACE == nIdx2 ){
						break;
					}
					List_SetCurSel( hwndLIST_RES, nIdx2 );

					break;


				case IDC_BUTTON_ADD:
					nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
					if( CB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = List_GetCurSel( hwndLIST_RES );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
					nNum2 = List_GetCount( hwndLIST_RES );
					if( LB_ERR == nNum2 ){
						nIdx2 = 0;
					}
					nIdx3 = Combo_GetCurSel( hwndCOMBO_FUNCKIND );
					if( CB_ERR == nIdx3 ){
						break;
					}
					nIdx4 = List_GetCurSel( hwndLIST_FUNC );
					if( LB_ERR == nIdx4 ){
						break;
					}
					//	Oct. 3, 2001 genta
					if( m_cLookup.Pos2FuncCode( nIdx3, nIdx4 ) == 0 )
						break;

					List_GetText( hwndLIST_FUNC, nIdx4, szLabel );

					//	Oct. 3, 2001 genta
					m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nNum2] = m_cLookup.Pos2FuncCode( nIdx3, nIdx4 );
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nNum2] = '\0';
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

					nIdx2 = List_AddString( hwndLIST_RES, szLabel );
					if( LB_ERR == nIdx2 || LB_ERRSPACE == nIdx2 ){
						break;
					}
					List_SetCurSel( hwndLIST_RES, nIdx2 );

					break;

				case IDC_BUTTON_UP:
					nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
					if( CB_ERR == nIdx1 ){
						break;
					}
					nIdx2 = List_GetCurSel( hwndLIST_RES );
					if( LB_ERR == nIdx2 ){
						break;
					}
					if( 0 == nIdx2 ){
						break;
					}

					{
						EFunctionCode	nFunc = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 - 1];
						KEYCODE key = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 - 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 - 1] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 - 1]  = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]  = key;
					}

					List_GetText( hwndLIST_RES, nIdx2, szLabel );
					List_DeleteString( hwndLIST_RES, nIdx2 );
					List_InsertString( hwndLIST_RES, nIdx2 - 1, szLabel );
					List_SetCurSel( hwndLIST_RES, nIdx2 - 1 );
					break;

				case IDC_BUTTON_DOWN:
					nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
					if( CB_ERR == nIdx1 ){
						break;
					}
					nIdx2 = List_GetCurSel( hwndLIST_RES );
					if( LB_ERR == nIdx2 ){
						break;
					}
					nNum2 = List_GetCount( hwndLIST_RES );
					if( LB_ERR == nNum2 ){
						break;
					}
					if( nNum2 - 1 <= nIdx2 ){
						break;
					}

					{
						EFunctionCode	nFunc = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 + 1];
						KEYCODE key = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 + 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 + 1] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 + 1]  = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]  = key;
					}
					List_GetText( hwndLIST_RES, nIdx2, szLabel );
					List_DeleteString( hwndLIST_RES, nIdx2 );
					List_InsertString( hwndLIST_RES, nIdx2 + 1, szLabel );
					List_SetCurSel( hwndLIST_RES, nIdx2 + 1 );
					break;
				}

				break;
			}
		}
		break;

	case WM_TIMER:
		nIdx1 = Combo_GetCurSel( hwndCOMBO_MENU );
		nIdx2 = List_GetCurSel( hwndLIST_RES );
		nIdx3 = Combo_GetCurSel( hwndCOMBO_FUNCKIND );
		nIdx4 = List_GetCurSel( hwndLIST_FUNC );
		i = List_GetCount( hwndLIST_RES );
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
		if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERTSEPARATOR ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}
		if( CB_ERR != nIdx3 && LB_ERR != nIdx4 &&
		 	m_cLookup.Pos2FuncCode( nIdx3, nIdx4 ) == 0
		){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}
		break;
	case WM_DESTROY:
		::KillTimer( hwndDlg, 1 );
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




/* ダイアログデータの設定 Custom menu */
void CPropCustmenu::SetData( HWND hwndDlg )
{
	HWND		hwndCOMBO_MENU;
	HWND		hwndCombo;
	HWND		hwndLIST_RES;
	int			i;
	int			nIdx;
	WCHAR		szLabel[300];
	WCHAR		szLabel2[300];

	/* 機能種別一覧に文字列をセット（コンボボックス） */
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 3, 2001 genta

	/* 種別の先頭の項目を選択（コンボボックス）*/
	Combo_SetCurSel( hwndCombo, 0 );	//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように大元 Funcode.cpp で変更してある

	/* メニュー一覧に文字列をセット（コンボボックス）*/
	hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		Combo_AddString( hwndCOMBO_MENU, m_Common.m_sCustomMenu.m_szCustMenuNameArr[i] );
	}
	/* メニュー一覧の先頭の項目を選択（コンボボックス）*/
	Combo_SetCurSel( hwndCOMBO_MENU, 0 );

	/* メニュー項目一覧に文字列をセット（リストボックス）*/
	hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
//	hwndEDIT_KEY = ::GetDlgItem( hwndDlg, IDC_EDIT_KEY );
	List_ResetContent( hwndLIST_RES );
	nIdx = 0;
	for( i = 0; i < m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx]; ++i ){
		if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx][i] ){
			auto_strcpy( szLabel, LTEXT(" ─────────────") );	//Oct. 18, 2000 JEPRO 「ツールバー」タブで使っているセパレータと同じ線種に統一した
		}else{
			//	Oct. 3, 2001 genta
			m_cLookup.Funccode2Name( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx][i], szLabel, 256 );
			//::LoadString( G_AppInstance(), m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx][i], szLabel, 256 );
		}
		/* キー */
		if( '\0' == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx][i] ){
			auto_strcpy( szLabel2, szLabel );
		}else{
			auto_sprintf( szLabel2, LTEXT("%ts(%hc)"),
				szLabel,
				m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx][i]
			);
		}
		::List_AddString( hwndLIST_RES, szLabel2 );
	}
	
	//	Oct. 15, 2001 genta メニュー名を設定
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MENUNAME, m_Common.m_sCustomMenu.m_szCustMenuNameArr[0] );
	
//	/* カスタムメニューの先頭の項目を選択（リストボックス）*/	//Oct. 8, 2000 JEPRO ここをコメントアウトすると先頭項目が選択されなくなる
	List_SetCurSel( hwndLIST_RES, 0 );
	return;
}



/* ダイアログデータの取得 Custom menu */
int CPropCustmenu::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = ID_PAGENUM_CUSTMENU;

	return TRUE;
}





/* カスタムメニュー設定をインポートする */
void CPropCustmenu::Import( HWND hwndDlg )
{
	CImpExpCustMenu	cImpExpCustMenu( m_Common );

	// インポート
	if (!cImpExpCustMenu.ImportUI( G_AppInstance(), hwndDlg )) {
		// インポートをしていない
		return;
	}
}

/* カスタムメニュー設定をエクスポートする */
void CPropCustmenu::Export( HWND hwndDlg )
{
	CImpExpCustMenu	cImpExpCustMenu( m_Common );

	// エクスポート
	if (!cImpExpCustMenu.ExportUI( G_AppInstance(), hwndDlg )) {
		// エクスポートをしていない
		return;
	}
}
