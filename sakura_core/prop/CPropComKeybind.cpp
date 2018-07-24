/*!	@file
	@brief 共通設定ダイアログボックス、「キーバインド」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK, jepro
	Copyright (C) 2002, MIK, YAZAKI, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/


#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 20210/4/23 Uchi
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

#define STR_SHIFT_PLUS        _T("Shift+")  //@@@ 2001.11.08 add MIK
#define STR_CTRL_PLUS         _T("Ctrl+")  //@@@ 2001.11.08 add MIK
#define STR_ALT_PLUS          _T("Alt+")  //@@@ 2001.11.08 add MIK

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10700
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_KEYBIND,		//インポート
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_KEYBIND,		//エクスポート
	IDC_BUTTON_ASSIGN,				HIDC_BUTTON_ASSIGN,				//キー割り当て
	IDC_BUTTON_RELEASE,				HIDC_BUTTON_RELEASE,			//キー解除
	IDC_CHECK_SHIFT,				HIDC_CHECK_SHIFT,				//Shiftキー
	IDC_CHECK_CTRL,					HIDC_CHECK_CTRL,				//Ctrlキー
	IDC_CHECK_ALT,					HIDC_CHECK_ALT,					//Altキー
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND_KEYBIND,	//機能の種別
	IDC_EDIT_KEYSFUNC,				HIDC_EDIT_KEYSFUNC,				//キーに割り当てられている機能
	IDC_LIST_FUNC,					HIDC_LIST_FUNC_KEYBIND,			//機能一覧
	IDC_LIST_KEY,					HIDC_LIST_KEY,					//キー一覧
	IDC_LIST_ASSIGNEDKEYS,			HIDC_LIST_ASSIGNEDKEYS,			//機能に割り当てられているキー
	IDC_LABEL_MENUFUNCKIND,			(DWORD)-1,
	IDC_LABEL_MENUFUNC,				(DWORD)-1,
	IDC_LABEL_KEYKIND,				(DWORD)-1,
	IDC_LABEL_FUNCtoKEY,			(DWORD)-1,
	IDC_LABEL_KEYtoFUNC,			(DWORD)-1,
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
INT_PTR CALLBACK CPropKeybind::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropKeybind::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* From Here Oct. 13, 2000 Studio CでMr.コーヒー氏に教わったやり方ですがうまくいってません */
// ウィンドウプロシージャの中で・・・
LRESULT CALLBACK CPropComKeybindWndProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	// WM_CTLCOLORSTATIC メッセージに対して
	case WM_CTLCOLORSTATIC:
	// 白色のブラシハンドルを返す
		return (LRESULT)GetStockObject(WHITE_BRUSH);
//	default:
//		break;
	}
	return 0;
}
/* To Here Oct. 13, 2000 */







/* Keybind メッセージ処理 */
INT_PTR CPropKeybind::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,	// message
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	static HWND	hwndCombo;
	static HWND	hwndFuncList;
	static HWND	hwndKeyList;
	static HWND	hwndCheckShift;
	static HWND	hwndCheckCtrl;
	static HWND	hwndCheckAlt;
	static HWND	hwndAssignedkeyList;
//	static HWND hwndLIST_KEYSFUNC;
	static HWND hwndEDIT_KEYSFUNC;
//	int			nLength;
	int			nAssignedKeyNum;

	int			nIndex;
	int			nIndex2;
	int			nIndex3;
	int			i;
	int			j;
	EFunctionCode	nFuncCode;
	static WCHAR pszLabel[256];

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 Keybind */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* コントロールのハンドルを取得 */
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
		hwndFuncList = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
		hwndAssignedkeyList = ::GetDlgItem( hwndDlg, IDC_LIST_ASSIGNEDKEYS );
		hwndCheckShift = ::GetDlgItem( hwndDlg, IDC_CHECK_SHIFT );
		hwndCheckCtrl = ::GetDlgItem( hwndDlg, IDC_CHECK_CTRL );
		hwndCheckAlt = ::GetDlgItem( hwndDlg, IDC_CHECK_ALT );
		hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
//		hwndLIST_KEYSFUNC = ::GetDlgItem( hwndDlg, IDC_LIST_KEYSFUNC );
		hwndEDIT_KEYSFUNC = ::GetDlgItem( hwndDlg, IDC_EDIT_KEYSFUNC );

		/* キー選択時の処理 */
//	From Here Oct. 14, 2000 JEPRO わかりにくいので選択しないように変更	//Oct. 17, 2000 JEPRO 復活！
//	/* キーリストの先頭の項目を選択（リストボックス）*/
		List_SetCurSel( hwndKeyList, 0 );	//Oct. 14, 2000 JEPRO ここをコメントアウトすると先頭項目が選択されなくなる
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );	//Oct. 14, 2000 JEPRO ここはどっちでもいい？(わからん)
//	To Here Oct. 14, 2000
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );

		::SetTimer( hwndDlg, 1, 300, NULL );	// 2007.11.02 ryoji

		return TRUE;

	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
//			OnHelp( hwndDlg, IDD_PROP1P5 );		// Sept. 9, 2000 JEPRO 実際のID名に変更
			OnHelp( hwndDlg, IDD_PROP_KEYBIND );
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE( _T("Keybind PSN_KILLACTIVE\n") );
			/* ダイアログデータの取得 Keybind */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = ID_PROPCOM_PAGENUM_KEYBOARD;

			// 表示を更新する（マクロ設定画面でのマクロ名変更を反映）	// 2007.11.02 ryoji
			nIndex = List_GetCurSel( hwndKeyList );
			nIndex2 = Combo_GetCurSel( hwndCombo );
			nIndex3 = List_GetCurSel( hwndFuncList );
			if( nIndex != LB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
			}
			if( nIndex2 != CB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );
				if( nIndex3 != LB_ERR ){
					List_SetCurSel( hwndFuncList, nIndex3 );
				}
			}
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID = LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl = (HWND) lParam;	/* コントロールのハンドル */

		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_BUTTON_IMPORT:	/* インポート */
				/* Keybind:キー割り当て設定をインポートする */
				Import( hwndDlg );
				return TRUE;
			case IDC_BUTTON_EXPORT:	/* エクスポート */
				/* Keybind:キー割り当て設定をエクスポートする */
				Export( hwndDlg );
				return TRUE;
			case IDC_BUTTON_ASSIGN:	/* 割付 */
				nIndex = List_GetCurSel( hwndKeyList );
				nIndex2 = Combo_GetCurSel( hwndCombo );
				nIndex3 = List_GetCurSel( hwndFuncList );
				if( nIndex == LB_ERR || nIndex2 == CB_ERR || nIndex3 == LB_ERR ){
					return TRUE;
				}
				nFuncCode = m_cLookup.Pos2FuncCode( nIndex2, nIndex3 );	// Oct. 2, 2001 genta
				i = 0;
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
					i |= _SHIFT;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
					i |= _CTRL;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
					i |= _ALT;
				}
				m_Common.m_sKeyBind.m_pKeyNameArr[nIndex].m_nFuncCodeArr[i] = nFuncCode;
				::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
				::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndFuncList );
				return TRUE;
			case IDC_BUTTON_RELEASE:	/* 解除 */
				nIndex = List_GetCurSel( hwndKeyList );
				if( nIndex == LB_ERR ){
					return TRUE;
				}
				nFuncCode = F_DEFAULT;
				i = 0;
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
					i |= _SHIFT;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
					i |= _CTRL;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
					i |= _ALT;
				}
				m_Common.m_sKeyBind.m_pKeyNameArr[nIndex].m_nFuncCodeArr[i] = nFuncCode;
				::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
				::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndFuncList );
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		if( hwndCheckShift == hwndCtl
		 || hwndCheckCtrl == hwndCtl
		 || hwndCheckAlt == hwndCtl
		){
			switch( wNotifyCode ){
			case BN_CLICKED:
				ChangeKeyList( hwndDlg );

				return TRUE;
			}
		}else
		if( hwndKeyList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = List_GetCurSel( hwndKeyList );
				i = 0;
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
					i |= _SHIFT;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
					i |= _CTRL;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
					i |= _ALT;
				}
				nFuncCode = m_Common.m_sKeyBind.m_pKeyNameArr[nIndex].m_nFuncCodeArr[i];
				// Oct. 2, 2001 genta
				// 2007.11.02 ryoji F_DISABLEなら未割付
				if( nFuncCode == F_DISABLE ){
					auto_strncpy( pszLabel, LSW(STR_PROPCOMKEYBIND_UNASSIGN), _countof(pszLabel) - 1 );
					pszLabel[_countof(pszLabel) - 1] = L'\0';
				}else{
					m_cLookup.Funccode2Name( nFuncCode, pszLabel, 255 );
				}
				Wnd_SetText( hwndEDIT_KEYSFUNC, pszLabel );
				return TRUE;
			}
		}else
		if( hwndFuncList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = List_GetCurSel( hwndKeyList );
				nIndex2 = Combo_GetCurSel( hwndCombo );
				nIndex3 = List_GetCurSel( hwndFuncList );
				nFuncCode = m_cLookup.Pos2FuncCode( nIndex2, nIndex3 );	// Oct. 2, 2001 genta
				/* 機能に対応するキー名の取得(複数) */
				CNativeT**	ppcAssignedKeyList;
				nAssignedKeyNum = CKeyBind::GetKeyStrList(	/* 機能に対応するキー名の取得(複数) */
					G_AppInstance(), m_Common.m_sKeyBind.m_nKeyNameArrNum, (KEYDATA*)m_Common.m_sKeyBind.m_pKeyNameArr,
					&ppcAssignedKeyList, nFuncCode,
					FALSE	// 2007.02.22 ryoji デフォルト機能は取得しない
				);	
				/* 割り当てキーリストをクリアして値の設定 */
				List_ResetContent( hwndAssignedkeyList );
				if( 0 < nAssignedKeyNum){
					for( j = 0; j < nAssignedKeyNum; ++j ){
						/* デバッグモニタに出力 */
						const TCHAR* cpszString = ppcAssignedKeyList[j]->GetStringPtr();
						::List_AddString( hwndAssignedkeyList, cpszString );
						delete ppcAssignedKeyList[j];
					}
					delete [] ppcAssignedKeyList;
				}
				return TRUE;
			}
		}else
		if( hwndCombo == hwndCtl){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIndex2 = Combo_GetCurSel( hwndCombo );
				/* 機能一覧に文字列をセット（リストボックス）*/
				m_cLookup.SetListItem( hwndFuncList, nIndex2 );	//	Oct. 2, 2001 genta
				return TRUE;
			}

//@@@ 2001.11.08 add start MIK
		}else
		if( hwndAssignedkeyList == hwndCtl){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
			//case LBN_DBLCLK:
				{
					TCHAR	buff[1024], *p;
					int	ret;

					nIndex = List_GetCurSel( hwndAssignedkeyList );
					auto_memset(buff, 0, _countof(buff));
					ret = List_GetText( hwndAssignedkeyList, nIndex, buff);
					if( ret != LB_ERR )
					{
						i = 0;
						p = buff;
						//SHIFT
						if( auto_memcmp(p, STR_SHIFT_PLUS, _tcslen(STR_SHIFT_PLUS)) == 0 ){
							p += _tcslen(STR_SHIFT_PLUS);
							i |= _SHIFT;
						}
						//CTRL
						if( auto_memcmp(p, STR_CTRL_PLUS, _tcslen(STR_CTRL_PLUS)) == 0 ){
							p += _tcslen(STR_CTRL_PLUS);
							i |= _CTRL;
						}
						//ALT
						if( auto_memcmp(p, STR_ALT_PLUS, _tcslen(STR_ALT_PLUS)) == 0 ){
							p += _tcslen(STR_ALT_PLUS);
							i |= _ALT;
						}
						for(j = 0; j < m_Common.m_sKeyBind.m_nKeyNameArrNum; j++)
						{
							if( _tcscmp(m_Common.m_sKeyBind.m_pKeyNameArr[j].m_szKeyName, p) == 0 )
							{
								List_SetCurSel( hwndKeyList, j);
								if( i & _SHIFT ) ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_CHECKED );  //チェック
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_UNCHECKED );  //チェックをはずす
								if( i & _CTRL )  ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_CHECKED );  //チェック
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_UNCHECKED );  //チェックをはずす
								if( i & _ALT )   ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_CHECKED );  //チェック
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_UNCHECKED );  //チェックをはずす
								::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );

								// キー一覧の文字列も変更
								ChangeKeyList( hwndDlg );
								break;
							}
						}
					}
					return TRUE;
				}
			}
//@@@ 2001.11.08 add end MIK

		}
		break;

	case WM_TIMER:
		// ボタンの有効／無効を切り替える	// 2007.11.02 ryoji
		nIndex = List_GetCurSel( hwndKeyList );
		nIndex2 = Combo_GetCurSel( hwndCombo );
		nIndex3 = List_GetCurSel( hwndFuncList );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ASSIGN ), !( LB_ERR == nIndex || nIndex2 == CB_ERR || nIndex3 == LB_ERR ) );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RELEASE ), !( LB_ERR == nIndex ) );
		break;

	case WM_DESTROY:
		::KillTimer( hwndDlg, 1 );	// 2007.11.02 ryoji
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

//@@@ 2001.11.07 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.11.07 End

	}
	return FALSE;
}







/* ダイアログデータの設定 Keybind */
void CPropKeybind::SetData( HWND hwndDlg )
{
	HWND		hwndCombo;
	HWND		hwndKeyList;
	int			i;

	/* 機能種別一覧に文字列をセット（コンボボックス）*/
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 2, 2001 genta

	/* 種別の先頭の項目を選択（コンボボックス）*/
	Combo_SetCurSel( hwndCombo, 0 );	//Oct. 14, 2000 JEPRO JEPRO 「--未定義--」を表示させないように大元 Funcode.cpp で変更してある

	/* キー一覧に文字列をセット（リストボックス）*/
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	for( i = 0; i < m_Common.m_sKeyBind.m_nKeyNameArrNum; ++i ){
		::List_AddString( hwndKeyList, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName );
	}

	return;
}



/* ダイアログデータの取得 Keybind */
int CPropKeybind::GetData( HWND hwndDlg )
{
	return TRUE;
}
	
/*! Keybind: キーリストをチェックボックスの状態に合わせて更新する */
void CPropKeybind::ChangeKeyList( HWND hwndDlg){
	HWND	hwndKeyList;
	int 	nIndex;
	int 	nIndexTop;
	int 	i;
	wchar_t	szKeyState[64];
	
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	nIndex = List_GetCurSel( hwndKeyList );
	nIndexTop = List_GetTopIndex( hwndKeyList );
	szKeyState[0] = L'\0';
	i = 0;
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
		i |= _SHIFT;
		wcscat( szKeyState, L"Shift+" );
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
		i |= _CTRL;
		wcscat( szKeyState, L"Ctrl+" );
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
		i |= _ALT;
		wcscat( szKeyState, L"Alt+" );
	}
	/* キー一覧に文字列をセット（リストボックス）*/
	List_ResetContent( hwndKeyList );
	for( i = 0; i < m_Common.m_sKeyBind.m_nKeyNameArrNum; ++i ){
		TCHAR	pszLabel[256];
		auto_sprintf( pszLabel, _T("%ls%ts"), szKeyState, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName );
		::List_AddString( hwndKeyList, pszLabel );
	}
	List_SetCurSel( hwndKeyList, nIndex );
	List_SetTopIndex( hwndKeyList, nIndexTop );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
}

/* Keybind:キー割り当て設定をインポートする */
void CPropKeybind::Import( HWND hwndDlg )
{
	CImpExpKeybind	cImpExpKeybind( m_Common );

	// インポート
	if (!cImpExpKeybind.ImportUI( G_AppInstance(), hwndDlg )) {
		// インポートをしていない
		return;
	}

	// ダイアログデータの設定 Keybind
	// 2012.11.18 aroka キー一覧の更新は全アイテムを更新する。
	ChangeKeyList( hwndDlg );
	//@@@ 2001.11.07 modify start MIK: 機能に割り当てられているキーを更新する。// 2012.11.18 aroka コメント修正
	HWND			hwndCtrl;
	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndCtrl );
	//@@@ 2001.11.07 modify end MIK
}


/* Keybind:キー割り当て設定をエクスポートする */
void CPropKeybind::Export( HWND hwndDlg )
{
	CImpExpKeybind	cImpExpKeybind( m_Common );

	// エクスポート
	if (!cImpExpKeybind.ExportUI( G_AppInstance(), hwndDlg )) {
		// エクスポートをしていない
		return;
	}
}
