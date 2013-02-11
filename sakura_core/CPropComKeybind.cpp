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
#include "CPropCommon.h"
#include "CDlgOpenFile.h"
#include "etc_uty.h"
#include "CSMacroMgr.h" // 2002/2/10 aroka
#include "mymessage.h"
#include "Debug.h" ///
#include <stdio.h>	/// 2002/2/3 aroka from here
#include "sakura.hh"

//	From Here Sept. 5, 2000 JEPRO 半角カタカナの全角化に伴い文字長を変更(21→34)
#define STR_KEYDATA_HEAD_LEN  34
//	To Here Sept. 5, 2000
#define STR_KEYDATA_HEAD      "テキストエディタ キー設定ファイル\x1a"

const char STR_KEYDATA_HEAD2[] = "// テキストエディタキー設定 Ver2";	//@@@ 2001.11.07 add MIK

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
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_KEYKIND,				-1,
	IDC_LABEL_FUNCtoKEY,			-1,
	IDC_LABEL_KEYtoFUNC,			-1,
	IDC_CHECK_ACCELTBL_EACHWIN,		HIDC_CHECK_ACCELTBL_EACHWIN,	// ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)
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
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_KEYBIND(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_p5, hwndDlg, uMsg, wParam, lParam );
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
INT_PTR CPropCommon::DispatchEvent_p5(
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
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
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
	int			nFuncCode;
	static TCHAR pszLabel[256];

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 Keybind */
		SetData_p5( hwndDlg );
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
		::SendMessage( hwndKeyList, LB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO ここをコメントアウトすると先頭項目が選択されなくなる
		::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );	//Oct. 14, 2000 JEPRO ここはどっちでもいい？(わからん)
//	To Here Oct. 14, 2000
		::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );

		::SetTimer( hwndDlg, 1, 300, NULL );	// 2007.11.02 ryoji

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
//			OnHelp( hwndDlg, IDD_PROP1P5 );		// Sept. 9, 2000 JEPRO 実際のID名に変更
			OnHelp( hwndDlg, IDD_PROP_KEYBIND );
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE_A( "Keybind PSN_KILLACTIVE\n" );
			/* ダイアログデータの取得 Keybind */
			GetData_p5( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = ID_PAGENUM_KEYBOARD;

			// 表示を更新する（マクロ設定画面でのマクロ名変更を反映）	// 2007.11.02 ryoji
			nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
			nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
			nIndex3 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
			if( nIndex != LB_ERR ){
				::SendMessage( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
			}
			if( nIndex2 != CB_ERR ){
				::SendMessage( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );
				if( nIndex3 != LB_ERR ){
					::SendMessage( hwndFuncList, LB_SETCURSEL, nIndex3, 0 );
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
				p5_Import_KeySetting( hwndDlg );
				return TRUE;
			case IDC_BUTTON_EXPORT:	/* エクスポート */
				/* Keybind:キー割り当て設定をエクスポートする */
				p5_Export_KeySetting( hwndDlg );
				return TRUE;
			case IDC_BUTTON_ASSIGN:	/* 割付 */
				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
				nIndex3 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
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
				::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
				::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndFuncList );
				return TRUE;
			case IDC_BUTTON_RELEASE:	/* 解除 */
				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
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
				::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
				::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndFuncList );
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
				p5_ChangeKeyList( hwndDlg );

				return TRUE;
			}
		}else
		if( hwndKeyList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
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
					strcpy( pszLabel, _T("未割付") );
				}else{
					m_cLookup.Funccode2Name( nFuncCode, pszLabel, 255 );
				}
				::SetWindowText( hwndEDIT_KEYSFUNC, pszLabel );
				return TRUE;
			}
		}else
		if( hwndFuncList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
				nIndex3 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
				nFuncCode = m_cLookup.Pos2FuncCode( nIndex2, nIndex3 );	// Oct. 2, 2001 genta
				/* 機能に対応するキー名の取得(複数) */
				CMemory**	ppcAssignedKeyList;
				nAssignedKeyNum = CKeyBind::GetKeyStrList(	/* 機能に対応するキー名の取得(複数) */
					m_hInstance, m_Common.m_sKeyBind.m_nKeyNameArrNum, (KEYDATA*)m_Common.m_sKeyBind.m_pKeyNameArr,
					&ppcAssignedKeyList, nFuncCode,
					FALSE	// 2007.02.22 ryoji デフォルト機能は取得しない
				);	
				/* 割り当てキーリストをクリアして値の設定 */
				::SendMessage( hwndAssignedkeyList, LB_RESETCONTENT, 0, 0 );
				if( 0 < nAssignedKeyNum){
					for( j = 0; j < nAssignedKeyNum; ++j ){
						/* デバッグモニタに出力 */
						const TCHAR* cpszString = ppcAssignedKeyList[j]->GetStringPtr();
						::SendMessage( hwndAssignedkeyList, LB_ADDSTRING, 0, (LPARAM)cpszString );
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
				nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
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

					nIndex = ::SendMessage( hwndAssignedkeyList, LB_GETCURSEL, 0, 0 );
					memset(buff, 0, _countof(buff));
					ret = ::SendMessage( hwndAssignedkeyList, LB_GETTEXT, nIndex, (LPARAM)buff);
					if( ret != LB_ERR )
					{
						i = 0;
						p = buff;
						//SHIFT
						if( memcmp(p, STR_SHIFT_PLUS, _tcslen(STR_SHIFT_PLUS)) == 0 ){
							p += _tcslen(STR_SHIFT_PLUS);
							i |= _SHIFT;
						}
						//CTRL
						if( memcmp(p, STR_CTRL_PLUS, _tcslen(STR_CTRL_PLUS)) == 0 ){
							p += _tcslen(STR_CTRL_PLUS);
							i |= _CTRL;
						}
						//ALT
						if( memcmp(p, STR_ALT_PLUS, _tcslen(STR_ALT_PLUS)) == 0 ){
							p += _tcslen(STR_ALT_PLUS);
							i |= _ALT;
						}
						for(j = 0; j < m_Common.m_sKeyBind.m_nKeyNameArrNum; j++)
						{
							if( _tcscmp(m_Common.m_sKeyBind.m_pKeyNameArr[j].m_szKeyName, p) == 0 )
							{
								::SendMessage( hwndKeyList, LB_SETCURSEL, (WPARAM)j, (LPARAM)0);
								if( i & _SHIFT ) ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_CHECKED );  //チェック
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_UNCHECKED );  //チェックをはずす
								if( i & _CTRL )  ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_CHECKED );  //チェック
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_UNCHECKED );  //チェックをはずす
								if( i & _ALT )   ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_CHECKED );  //チェック
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_UNCHECKED );  //チェックをはずす
								::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );

								// キー一覧の文字列も変更
								p5_ChangeKeyList( hwndDlg );
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
		nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
		nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
		nIndex3 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
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
			MyWinHelp( (HWND)p->hItemHandle, m_pszHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.11.07 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_pszHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.11.07 End

	}
	return FALSE;
}







/* ダイアログデータの設定 Keybind */
void CPropCommon::SetData_p5( HWND hwndDlg )
{
	HWND		hwndCombo;
	HWND		hwndKeyList;
	int			i;

	/* 機能種別一覧に文字列をセット（コンボボックス）*/
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 2, 2001 genta

	/* 種別の先頭の項目を選択（コンボボックス）*/
	::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO JEPRO 「--未定義--」を表示させないように大元 Funcode.cpp で変更してある

	/* キー一覧に文字列をセット（リストボックス）*/
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	for( i = 0; i < m_Common.m_sKeyBind.m_nKeyNameArrNum; ++i ){
			::SendMessage( hwndKeyList, LB_ADDSTRING, 0, (LPARAM)m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName );
	}

	// 2009.08.15 nasukoji	ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)
	::CheckDlgButton( hwndDlg, IDC_CHECK_ACCELTBL_EACHWIN, m_Common.m_sKeyBind.m_bCreateAccelTblEachWin );

	return;
}



/* ダイアログデータの取得 Keybind */
int CPropCommon::GetData_p5( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = ID_PAGENUM_KEYBOARD;
	// 2009.08.15 nasukoji	ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)
	m_Common.m_sKeyBind.m_bCreateAccelTblEachWin = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ACCELTBL_EACHWIN );

	return TRUE;
}

/*! Keybind: キーリストをチェックボックスの状態に合わせて更新する */
void CPropCommon::p5_ChangeKeyList( HWND hwndDlg){
	HWND	hwndKeyList;
	int 	nIndex;
	int 	nIndexTop;
	int 	i;
	TCHAR	szKeyState[64];
	
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
	nIndexTop = ::SendMessage( hwndKeyList, LB_GETTOPINDEX, 0, 0 );
	_tcscpy( szKeyState, _T("") );
	i = 0;
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
		i |= _SHIFT;
		_tcscat( szKeyState, _T("Shift+") );
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
		i |= _CTRL;
		_tcscat( szKeyState, _T("Ctrl+") );
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
		i |= _ALT;
		_tcscat( szKeyState, _T("Alt+") );
	}
	/* キー一覧に文字列をセット（リストボックス）*/
	::SendMessage( hwndKeyList, LB_RESETCONTENT, 0, 0 );
	for( i = 0; i < m_Common.m_sKeyBind.m_nKeyNameArrNum; ++i ){
		TCHAR	pszLabel[256];
		wsprintf( pszLabel, _T("%s%s"), szKeyState, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName );
		::SendMessage( hwndKeyList, LB_ADDSTRING, 0, (LPARAM)pszLabel );
	}
	::SendMessage( hwndKeyList, LB_SETCURSEL, nIndex, 0 );
	::SendMessage( hwndKeyList, LB_SETTOPINDEX, nIndexTop, 0 );
	::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
}

/* Keybind:キー割り当て設定をインポートする */
void CPropCommon::p5_Import_KeySetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char			szPath[_MAX_PATH + 1];
	HFILE			hFile;

	char			pHeader[STR_KEYDATA_HEAD_LEN + 1];
	short			nKeyNameArrNum;				/* キー割り当て表の有効データ数 */
	const int		KEYNAME_SIZE = _countof(m_Common.m_sKeyBind.m_pKeyNameArr);
	KEYDATA			pKeyNameArr[KEYNAME_SIZE];				/* キー割り当て表 */
	HWND			hwndCtrl;
	char			szInitDir[_MAX_PATH + 1];

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_sHistory.m_szIMPORTFOLDER );	/* インポート用フォルダ */
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		_T("*.key"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_sHistory.m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_sHistory.m_szIMPORTFOLDER, "\\" );

	hFile = _lopen( szPath, OF_READ );
	if( HFILE_ERROR == hFile ){
		ErrorMessage( hwndDlg,
			_T("ファイルを開けませんでした。\n\n%s"), szPath
		);
		return;
	}
	if( STR_KEYDATA_HEAD_LEN		!= _lread( hFile, pHeader, STR_KEYDATA_HEAD_LEN ) ||
		sizeof( nKeyNameArrNum )	!= _lread( hFile, &nKeyNameArrNum, sizeof( nKeyNameArrNum ) ) ||
		sizeof( pKeyNameArr )		!= _lread( hFile,  pKeyNameArr,    sizeof( pKeyNameArr ) ) ||
		0 != memcmp( pHeader, STR_KEYDATA_HEAD, STR_KEYDATA_HEAD_LEN )
	){
		_lclose( hFile );  //@@@ 2001.11.07 add MIK

//@@@ 2001.11.07 add start MIK
		{
			FILE	*fp;
			int	i, j, cnt, kc, n, an;
			char	buff[1024], name[1024], szFuncNameJapanese[256], s[1024], *p, *q;

			if( (fp = fopen( szPath, "r" )) == NULL )
			{
				ErrorMessage( hwndDlg,
					_T("ファイルを開けませんでした。\n\n%s"), szPath
				);
				return;
			}

			if( fgets(buff, sizeof(buff), fp) != NULL
			 && memcmp(buff, STR_KEYDATA_HEAD2, strlen(STR_KEYDATA_HEAD2)) == 0
			 && fgets(buff, sizeof(buff), fp) != NULL )
			{
				cnt = sscanf(buff, "Count=%d", &an);
				nKeyNameArrNum = an;
				if( cnt == 1 && an >= 0 && an <= KEYNAME_SIZE )
				{
					for(i = 0; i < an; i++)
					{
						name[0] = '\0';
						if( fgets(buff, sizeof(buff), fp) == NULL ) break;
						for(j = strlen(buff) - 1; j >= 0; j--){
							if( buff[j] == '\n' || buff[j] == '\r' ) buff[j] = '\0';
						}
						cnt = sscanf(buff, "KeyBind[%03d]=%04x,%s",
							&n,
							&kc,
							s,
							name);
						if( cnt != 3 ) break;
						if( i != n ) break;
						pKeyNameArr[i].m_nKeyCode = kc;

						p = s;
						for(j = 0; j < 8; j++)
						{
							q = strstr(p, ",");
							if( !q )
								break;
							*q = '\0';
							//機能名を数値に置き換える。(数値の機能名もあるかも)
							//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
//							n = CMacro::GetFuncInfoByName(m_hInstance, p, szFuncNameJapanese);
							n = CSMacroMgr::GetFuncInfoByName(m_hInstance, p, szFuncNameJapanese);
							if( n == -1 )
							{
								if( *p >= '0' && *p <= '9' )
								{
									n = atol(p);
								}
								else
								{
									n = 0;
								}
							}
							pKeyNameArr[i].m_nFuncCodeArr[j] = n;
							p = q + 1;
						}

						if( j != 8 ) break;

						strcpy(pKeyNameArr[i].m_szKeyName, p);
					}
					if( i == an )
					{
						fclose(fp);
						goto ToMaster;
					}
				}
			}
			fclose(fp);
		}
//@@@ 2001.11.07 add end MIK

		ErrorMessage( hwndDlg,
			_T("キー設定ファイルの形式が違います。\n\n%s"), szPath
		);
		return;
	}
	_lclose( hFile );

ToMaster:	//@@@ 2001.11.07 add MIK
	/* データのコピー */
	m_Common.m_sKeyBind.m_nKeyNameArrNum = nKeyNameArrNum;
	memcpy( m_Common.m_sKeyBind.m_pKeyNameArr, pKeyNameArr, sizeof( pKeyNameArr ) );

//	CShareData::SetKeyNames( m_pShareData );	/* キー名称のセット */


//	/* ダイアログデータの設定 p5 */
	// 2012.11.18 aroka キー一覧の更新は全アイテムを更新する。
	p5_ChangeKeyList( hwndDlg );
	//@@@ 2001.11.07 modify start MIK: 機能に割り当てられているキーを更新する。// 2012.11.18 aroka コメント修正
	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndCtrl );
	//@@@ 2001.11.07 modify end MIK
}


/* Keybind:キー割り当て設定をエクスポートする */
void CPropCommon::p5_Export_KeySetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char			szPath[_MAX_PATH + 1];
	char			szInitDir[_MAX_PATH + 1];
	const int KEYNAME_SIZE = _countof(m_Common.m_sKeyBind.m_pKeyNameArr);

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_sHistory.m_szIMPORTFOLDER );	/* インポート用フォルダ */
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		_T("*.key"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_sHistory.m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_sHistory.m_szIMPORTFOLDER, "\\" );

//@@@ 2001.11.07 add start MIK: テキスト形式で保存
	{
		FILE	*fp;
		int	i, j;
		char	szFuncName[256], szFuncNameJapanese[256], *p;
		
		if( (fp = fopen( szPath, "w" )) == NULL )
		{
			ErrorMessage( hwndDlg,
				_T("ファイルを開けませんでした。\n\n%s"), szPath
			);
			return;
		}
		
		fprintf(fp, "%s\n", STR_KEYDATA_HEAD2);
		fprintf(fp, "Count=%d\n", m_Common.m_sKeyBind.m_nKeyNameArrNum);
		
		for(i = 0; i < KEYNAME_SIZE; i++)
		{
			fprintf(fp, "KeyBind[%03d]=%04x", i, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_nKeyCode);

			for(j = 0; j < 8; j++)
			{
				//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
//				p = CMacro::GetFuncInfoByID(m_hInstance, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_nFuncCodeArr[j], szFuncName, szFuncNameJapanese);
				p = CSMacroMgr::GetFuncInfoByID(m_hInstance, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_nFuncCodeArr[j], szFuncName, szFuncNameJapanese);
				if( p ) {
					fprintf(fp, ",%s", p);
				}
				else {
					fprintf(fp, ",%d", m_Common.m_sKeyBind.m_pKeyNameArr[i].m_nFuncCodeArr[j]);
				}
			}
			
			fprintf(fp, ",%s\n", m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName);
		}
		
		fclose(fp);
	}
//@@@ 2001.11.07 add end MIK
}


/*[EOF]*/
