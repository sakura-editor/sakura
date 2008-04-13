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

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/


#include "stdafx.h"
#include "prop/CPropCommon.h"
#include "CDlgOpenFile.h"
#include "macro/CSMacroMgr.h" // 2002/2/10 aroka
#include "KeyCode.h"	/// 2002/2/3 aroka from here
#include "debug.h" ///
#include <stdio.h>	/// 2002/2/3 aroka from here
#include "io/CTextStream.h"
#include "CDataProfile.h"
#include "charcode.h"
#include "util/shell.h"
#include "util/file.h"
using namespace std;

const wchar_t WSTR_KEYDATA_HEAD[] = L"SakuraKeyBind_Ver3";	//2007.10.05 kobake ファイル形式をini形式に変更

#define STR_SHIFT_PLUS        _T("Shift+")  //@@@ 2001.11.08 add MIK
#define STR_CTRL_PLUS         _T("Ctrl+")  //@@@ 2001.11.08 add MIK
#define STR_ALT_PLUS          _T("Alt+")  //@@@ 2001.11.08 add MIK

//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
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
//	IDC_STATIC,						-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//10700
	IDC_BUTTON_IMPORT,				10700,	//インポート
	IDC_BUTTON_EXPORT,				10701,	//エクスポート
	IDC_BUTTON_ASSIGN,				10702,	//キー割り当て
	IDC_BUTTON_RELEASE,				10703,	//キー解除
	IDC_CHECK_SHIFT,				10710,	//Shiftキー
	IDC_CHECK_CTRL,					10711,	//Ctrlキー
	IDC_CHECK_ALT,					10712,	//Altキー
	IDC_COMBO_FUNCKIND,				10730,	//機能の種別
	IDC_EDIT_KEYSFUNC,				10740,	//キーに割り当てられている機能
	IDC_LIST_FUNC,					10750,	//機能一覧
	IDC_LIST_KEY,					10751,	//キー一覧
	IDC_LIST_ASSIGNEDKEYS,			10752,	//機能に割り当てられているキー
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_KEYKIND,				-1,
	IDC_LABEL_FUNCtoKEY,			-1,
	IDC_LABEL_KEYtoFUNC,			-1,
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







/* p5 メッセージ処理 */
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
	EFunctionCode	nFuncCode;
	static WCHAR pszLabel[256];

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 p5 */
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
		::SendMessageAny( hwndKeyList, LB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO ここをコメントアウトすると先頭項目が選択されなくなる
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );	//Oct. 14, 2000 JEPRO ここはどっちでもいい？(わからん)
//	To Here Oct. 14, 2000
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );

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
//			MYTRACE_A( "p5 PSN_KILLACTIVE\n" );
			/* ダイアログデータの取得 p5 */
			GetData_p5( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = ID_PAGENUM_KEYBOARD;

			// 表示を更新する（マクロ設定画面でのマクロ名変更を反映）	// 2007.11.02 ryoji
			nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
			nIndex2 = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
			nIndex3 = ::SendMessageAny( hwndFuncList, LB_GETCURSEL, 0, 0 );
			if( nIndex != LB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
			}
			if( nIndex2 != CB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );
				if( nIndex3 != LB_ERR ){
					::SendMessageAny( hwndFuncList, LB_SETCURSEL, nIndex3, 0 );
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
				/* p5:キー割り当て設定をインポートする */
				p5_Import_KeySetting( hwndDlg );
				return TRUE;
			case IDC_BUTTON_EXPORT:	/* エクスポート */
				/* p5:キー割り当て設定をエクスポートする */
				p5_Export_KeySetting( hwndDlg );
				return TRUE;
			case IDC_BUTTON_ASSIGN:	/* 割付 */
				nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
				nIndex3 = ::SendMessageAny( hwndFuncList, LB_GETCURSEL, 0, 0 );
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
				m_pKeyNameArr[nIndex].m_nFuncCodeArr[i] = nFuncCode;
				::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
				::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndFuncList );
				return TRUE;
			case IDC_BUTTON_RELEASE:	/* 解除 */
				nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
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
				m_pKeyNameArr[nIndex].m_nFuncCodeArr[i] = nFuncCode;
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
				p5_ChangeKeyList( hwndDlg );

				return TRUE;
			}
		}else
		if( hwndKeyList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
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
				nFuncCode = m_pKeyNameArr[nIndex].m_nFuncCodeArr[i];
				// Oct. 2, 2001 genta
				// 2007.11.02 ryoji F_DISABLEなら未割付
				if( nFuncCode == F_DISABLE ){
					auto_strcpy( pszLabel, LTEXT("未割付") );
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
				nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
				nIndex3 = ::SendMessageAny( hwndFuncList, LB_GETCURSEL, 0, 0 );
				nFuncCode = m_cLookup.Pos2FuncCode( nIndex2, nIndex3 );	// Oct. 2, 2001 genta
				/* 機能に対応するキー名の取得(複数) */
				CNativeT**	ppcAssignedKeyList;
				nAssignedKeyNum = CKeyBind::GetKeyStrList(	/* 機能に対応するキー名の取得(複数) */
					CNormalProcess::Instance()->GetProcessInstance(), m_nKeyNameArrNum, (KEYDATA*)m_pKeyNameArr,
					&ppcAssignedKeyList, nFuncCode,
					FALSE	// 2007.02.22 ryoji デフォルト機能は取得しない
				);	
				/* 割り当てキーリストをクリアして値の設定 */
				::SendMessageAny( hwndAssignedkeyList, LB_RESETCONTENT, 0, 0 );
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
				nIndex2 = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
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

					nIndex = ::SendMessageAny( hwndAssignedkeyList, LB_GETCURSEL, 0, 0 );
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
						for(j = 0; j < m_nKeyNameArrNum; j++)
						{
							if( _tcscmp(m_pKeyNameArr[j].m_szKeyName, p) == 0 )
							{
								::SendMessageAny( hwndKeyList, LB_SETCURSEL, (WPARAM)j, (LPARAM)0);
								if( i & _SHIFT ) ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_CHECKED );  //チェック
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_UNCHECKED );  //チェックをはずす
								if( i & _CTRL )  ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_CHECKED );  //チェック
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_UNCHECKED );  //チェックをはずす
								if( i & _ALT )   ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_CHECKED );  //チェック
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_UNCHECKED );  //チェックをはずす
								::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );

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
		nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
		nIndex2 = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
		nIndex3 = ::SendMessageAny( hwndFuncList, LB_GETCURSEL, 0, 0 );
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
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.11.07 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.11.07 End

	}
	return FALSE;
}







/* ダイアログデータの設定 p5 */
void CPropCommon::SetData_p5( HWND hwndDlg )
{
	HWND		hwndCombo;
	HWND		hwndKeyList;
	int			i;

	/* 機能種別一覧に文字列をセット（コンボボックス）*/
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 2, 2001 genta

	/* 種別の先頭の項目を選択（コンボボックス）*/
	::SendMessageAny( hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO JEPRO 「--未定義--」を表示させないように大元 Funcode.cpp で変更してある

	/* キー一覧に文字列をセット（リストボックス）*/
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	for( i = 0; i < m_nKeyNameArrNum; ++i ){
			::List_AddString( hwndKeyList, m_pKeyNameArr[i].m_szKeyName );
	}
	return;
}



/* ダイアログデータの取得 p5 */
int CPropCommon::GetData_p5( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = ID_PAGENUM_KEYBOARD;
	return TRUE;
}
/*! p5: キーリストをチェックボックスの状態に合わせて更新する */
void CPropCommon::p5_ChangeKeyList( HWND hwndDlg){
	HWND	hwndKeyList;
	int 	nIndex;
	int 	nIndexTop;
	int 	i;
	wchar_t	szKeyState[64];
	
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
	nIndexTop = ::SendMessageAny( hwndKeyList, LB_GETTOPINDEX, 0, 0 );
	wcscpy( szKeyState, L"" );
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
	::SendMessageAny( hwndKeyList, LB_RESETCONTENT, 0, 0 );
	for( i = 0; i < m_nKeyNameArrNum; ++i ){
		TCHAR	pszLabel[256];
		auto_sprintf( pszLabel, _T("%ls%ls"), szKeyState, m_pKeyNameArr[i].m_szKeyName );
		::List_AddString( hwndKeyList, pszLabel );
	}
	::SendMessageAny( hwndKeyList, LB_SETCURSEL, nIndex, 0 );
	::SendMessageAny( hwndKeyList, LB_SETTOPINDEX, nIndexTop, 0 );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
}

/* p5:キー割り当て設定をインポートする */
void CPropCommon::p5_Import_KeySetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];

	KEYDATA			pKeyNameArr[100];				/* キー割り当て表 */
	TCHAR			szInitDir[_MAX_PATH + 1];

	_tcscpy( szPath, _T("") );
	_tcscpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* インポート用フォルダ */
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		CNormalProcess::Instance()->GetProcessInstance(),
		hwndDlg,
		_T("*.key"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	_tcscat( m_pShareData->m_szIMPORTFOLDER, _T("\\") );



	//オープン
	CDataProfile in;
	in.SetReadingMode();
	if(!in.ReadProfile(szPath)){
		ErrorMessage_A( hwndDlg, "ファイルを開けませんでした。\n\n%ts", szPath );
		return;
	}
	static const wchar_t* szSection=L"SakuraKeybind";

	//バージョン確認
	WCHAR szHeader[256];
	in.IOProfileData(szSection,L"Ver",MakeStringBufferW(szHeader));
	if(wcscmp(szHeader,WSTR_KEYDATA_HEAD)!=0)goto err;

	//Count取得 -> nKeyNameArrNum
	int	nKeyNameArrNum;			// キー割り当て表の有効データ数
	in.IOProfileData(szSection,L"Count",nKeyNameArrNum);
	if(nKeyNameArrNum<0 || nKeyNameArrNum>100)goto err; //範囲チェック

	//各要素取得
	for(int i=0;i<100;i++)
	{
		//値 -> szData
		wchar_t szKey[256];
		auto_sprintf(szKey,L"KeyBind[%03d]",i);
		wchar_t szData[1024];
		in.IOProfileData(szSection,szKey,MakeStringBufferW(szData));

		//解析開始
		wchar_t* p=szData;

		//keycode取得
		int keycode;
		{
			//1トークン -> buf
			wchar_t buf[64];
			wchar_t* q=wcschr(p,',');
			if(q==NULL)goto err;
			wcsncpy(buf,p,q-p);
			buf[q-p]=L'\0';
			//buf -> 16進数変換 -> keycode
			int n=swscanf(buf,L"%04x",&keycode);
			if(n!=1)goto err;
			//p進める
			p=q+1;
		}
		pKeyNameArr[i].m_nKeyCode = keycode;

		//後に続くトークン
		for(int j=0;j<8;j++)
		{
			wchar_t* q=auto_strchr(p,L',');
			if(!q)goto err;
			*q=L'\0';

			//機能名を数値に置き換える。(数値の機能名もあるかも)
			//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
			WCHAR	szFuncNameJapanese[256];
			EFunctionCode n = CSMacroMgr::GetFuncInfoByName(CNormalProcess::Instance()->GetProcessInstance(), p, szFuncNameJapanese);
			if( n == F_INVALID )
			{
				if( WCODE::is09(*p) )
				{
					n = (EFunctionCode)auto_atol(p);
				}
				else
				{
					n = F_DEFAULT;
				}
			}
			pKeyNameArr[i].m_nFuncCodeArr[j] = n;
			p = q + 1;
		}

		auto_strcpy(pKeyNameArr[i].m_szKeyName, to_tchar(p));
	}

	// データのコピー
	m_nKeyNameArrNum = nKeyNameArrNum;
	memcpy_raw( m_pKeyNameArr, pKeyNameArr, sizeof_raw( pKeyNameArr ) );

	// ダイアログデータの設定 p5
	//@@@ 2001.11.07 modify start MIK: 機能に割り当てられているキー一覧を更新する。
	HWND			hwndCtrl;
	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndCtrl );
	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndCtrl );
	//@@@ 2001.11.07 modify end MIK
	return;

err:
	ErrorMessage( hwndDlg, _T("キー設定ファイルの形式が違います。\n\n%ts"), szPath );
}


/* p5:キー割り当て設定をエクスポートする */
void CPropCommon::p5_Export_KeySetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];

	_tcscpy( szPath, _T("") );
	_tcscpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* インポート用フォルダ */
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		CNormalProcess::Instance()->GetProcessInstance(),
		hwndDlg,
		_T("*.key"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	_tcscat( m_pShareData->m_szIMPORTFOLDER, _T("\\") );

	//@@@ 2001.11.07 add start MIK: テキスト形式で保存
	{
		int	i, j;
		WCHAR	szFuncNameJapanese[256];
		
		CTextOutputStream out(szPath);
		if(!out){
			ErrorMessage( hwndDlg, _T("ファイルを開けませんでした。\n\n%ts"), szPath );
			return;
		}
		
		out.WriteF( L"[SakuraKeybind]\n" );
		out.WriteF( L"Ver=%ls\n", WSTR_KEYDATA_HEAD);
		out.WriteF( L"Count=%d\n", m_nKeyNameArrNum);
		
		for(i = 0; i < 100; i++)
		{
			out.WriteF( L"KeyBind[%03d]=%04x", i, m_pKeyNameArr[i].m_nKeyCode);

			for(j = 0; j < 8; j++)
			{
				//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
				WCHAR szFuncName[256];
				WCHAR	*p = CSMacroMgr::GetFuncInfoByID(
					CNormalProcess::Instance()->GetProcessInstance(),
					m_pKeyNameArr[i].m_nFuncCodeArr[j],
					szFuncName,
					szFuncNameJapanese
				);
				if( p ) {
					out.WriteF( L",%ls", p);
				}
				else {
					out.WriteF( L",%d", m_pKeyNameArr[i].m_nFuncCodeArr[j]);
				}
			}
			
			out.WriteF( L",%ls\n", m_pKeyNameArr[i].m_szKeyName);
		}
		
		out.Close();
	}
//@@@ 2001.11.07 add end MIK

	return;
}


/*[EOF]*/
