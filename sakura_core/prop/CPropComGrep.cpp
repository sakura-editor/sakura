/*!	@file
	@brief 共通設定ダイアログボックス、「検索」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, genta
	Copyright (C) 2002, YAZAKI, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "extmodule/CBregexp.h"	// 2007.08/12 genta バージョン取得
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10500
	IDC_EDIT_REGEXPLIB,				HIDC_EDIT_REGEXPLIB,	//正規表現ライブラリ選択	// 2007.09.02 genta
	IDC_LABEL_REGEXP,				HIDC_EDIT_REGEXPLIB,
	IDC_LABEL_REGEXP_VER,			HIDC_LABEL_REGEXPVER,	//正規表現ライブラリバージョン	// 2007.09.02 genta
	IDC_CHECK_bCaretTextForSearch,	HIDC_CHECK_bCaretTextForSearch,	//カーソル位置の文字列をデフォルトの検索文字列にする	// 2006.08.23 ryoji
	IDC_CHECK_INHERIT_KEY_OTHER_VIEW, HIDC_CHECK_INHERIT_KEY_OTHER_VIEW,	// 次・前検索で他のビューの検索条件を引き継ぐ	// 2011.12.18 Moca
	IDC_CHECK_bGrepExitConfirm,		HIDC_CHECK_bGrepExitConfirm,	//GREPの保存確認
	IDC_CHECK_GTJW_RETURN,			HIDC_CHECK_GTJW_RETURN,			//タグジャンプ（エンターキー）
	IDC_CHECK_GTJW_LDBLCLK,			HIDC_CHECK_GTJW_LDBLCLK,		//タグジャンプ（ダブルクリック）
	IDC_CHECK_GREPREALTIME,			HIDC_CHECK_GREPREALTIME,		//リアルタイムで表示する	// 2006.08.08 ryoji
	IDC_COMBO_TAGJUMP,				HIDC_COMBO_TAGJUMP,				//タグファイルの検索
	IDC_COMBO_KEYWORD_TAGJUMP,		HIDC_COMBO_KEYWORD_TAGJUMP,		//タグファイルの検索
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
INT_PTR CALLBACK CPropGrep::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropGrep::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* メッセージ処理 */
INT_PTR CPropGrep::DispatchEvent( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	WORD		wNotifyCode;
//	WORD		wID;
//	HWND		hwndCtl;
	NMHDR*		pNMHDR;
//	int			nVal;
//    LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 Grep */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */

		return TRUE;
	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
//		switch( idCtrl ){
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_GREP );
				return TRUE;
			case PSN_KILLACTIVE:
				/* ダイアログデータの取得 Grep */
				GetData( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPCOM_PAGENUM_GREP;
				return TRUE;
			}
//			break;	/* default */
//		}
		break;	/* WM_NOTIFY */
	case WM_COMMAND:
		//	2007.08.12 genta 正規表現DLLの変更に応じてVersionを再取得する
		if( wParam == MAKEWPARAM( IDC_EDIT_REGEXPLIB, EN_KILLFOCUS )){
			SetRegexpVersion( hwndDlg );
		}
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

struct tagTagJumpMode{
	DWORD	m_nMethod;
	DWORD	m_nNameID;
};

/* ダイアログデータの設定 */
void CPropGrep::SetData( HWND hwndDlg )
{
	/* 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bCaretTextForSearch, m_Common.m_sSearch.m_bCaretTextForSearch );

	CheckDlgButtonBool( hwndDlg, IDC_CHECK_INHERIT_KEY_OTHER_VIEW, m_Common.m_sSearch.m_bInheritKeyOtherView );

	/* Grepモードで保存確認するか */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bGrepExitConfirm, m_Common.m_sSearch.m_bGrepExitConfirm );

	/* Grep結果のリアルタイム表示 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_GREPREALTIME, m_Common.m_sSearch.m_bGrepRealTimeView );	// 2006.08.08 ryoji ID修正


	/* Grepモード: エンターキーでタグジャンプ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_RETURN, m_Common.m_sSearch.m_bGTJW_RETURN );

	/* Grepモード: ダブルクリックでタグジャンプ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_LDBLCLK, m_Common.m_sSearch.m_bGTJW_LDBLCLK );

	//	2007.08.12 genta 正規表現DLL
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_REGEXPLIB ), _countof(m_Common.m_sSearch.m_szRegexpLib ) - 1 );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEXPLIB, m_Common.m_sSearch.m_szRegexpLib);
	SetRegexpVersion( hwndDlg );

	struct tagTagJumpMode TagJumpMode1Arr[] ={
		{ 0, STR_TAGJUMP_0 },
		{ 1, STR_TAGJUMP_1 },
		//{ 2, STR_TAGJUMP_2 },
		{ 3, STR_TAGJUMP_3 }
	};
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAGJUMP);
	Combo_ResetContent(hwndCombo);
	int nSelPos = 0;
	for(int i = 0; i < _countof(TagJumpMode1Arr); ++i){
		Combo_InsertString(hwndCombo, i, LS(TagJumpMode1Arr[i].m_nNameID));
		Combo_SetItemData(hwndCombo, i, TagJumpMode1Arr[i].m_nMethod);
		if(TagJumpMode1Arr[i].m_nMethod == m_Common.m_sSearch.m_nTagJumpMode ){
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	struct tagTagJumpMode TagJumpMode2Arr[] ={
		{ 0, STR_TAGJUMP_0 },
		{ 1, STR_TAGJUMP_1 },
		{ 2, STR_TAGJUMP_2 },
		{ 3, STR_TAGJUMP_3 }
	};
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_KEYWORD_TAGJUMP);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for(int i = 0; i < _countof(TagJumpMode2Arr); ++i){
		Combo_InsertString(hwndCombo, i, LS(TagJumpMode2Arr[i].m_nNameID));
		Combo_SetItemData(hwndCombo, i, TagJumpMode2Arr[i].m_nMethod);
		if(TagJumpMode2Arr[i].m_nMethod == m_Common.m_sSearch.m_nTagJumpModeKeyword ){
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	return;
}




/* ダイアログデータの取得 */
int CPropGrep::GetData( HWND hwndDlg )
{
	/* 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする */
	m_Common.m_sSearch.m_bCaretTextForSearch = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bCaretTextForSearch );

	m_Common.m_sSearch.m_bInheritKeyOtherView = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_INHERIT_KEY_OTHER_VIEW );

	/* Grepモードで保存確認するか */
	m_Common.m_sSearch.m_bGrepExitConfirm = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bGrepExitConfirm );

	/* Grep結果のリアルタイム表示 */
	m_Common.m_sSearch.m_bGrepRealTimeView = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GREPREALTIME );	// 2006.08.08 ryoji ID修正

	/* Grepモード: エンターキーでタグジャンプ */
	m_Common.m_sSearch.m_bGTJW_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_RETURN );

	/* Grepモード: ダブルクリックでタグジャンプ */
	m_Common.m_sSearch.m_bGTJW_LDBLCLK = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_LDBLCLK );

	//	2007.08.12 genta 正規表現DLL
	::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEXPLIB, m_Common.m_sSearch.m_szRegexpLib, _countof( m_Common.m_sSearch.m_szRegexpLib ));

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAGJUMP);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	m_Common.m_sSearch.m_nTagJumpMode = Combo_GetItemData(hwndCombo, nSelPos);
	
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_KEYWORD_TAGJUMP);
	nSelPos = Combo_GetCurSel(hwndCombo);
	m_Common.m_sSearch.m_nTagJumpModeKeyword = Combo_GetItemData(hwndCombo, nSelPos);

	return TRUE;
}

void CPropGrep::SetRegexpVersion( HWND hwndDlg )
{
	TCHAR regexp_dll[_MAX_PATH];
	
	::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEXPLIB, regexp_dll, _countof( regexp_dll ));
	CBregexp breg;
	if( DLL_SUCCESS != breg.InitDll( regexp_dll ) ){
		::DlgItem_SetText( hwndDlg, IDC_LABEL_REGEXP_VER, LS(STR_PROPCOMGREP_DLL) );
		return;
	}
	::DlgItem_SetText( hwndDlg, IDC_LABEL_REGEXP_VER, breg.GetVersionT() );
}



