//	$Id$
/*!	@file
	共通設定ダイアログボックス、「マクロ」ページ
	@author Norio Nakatani
	@version $Revision$
	@date Jun. 2, 2001 genta
	
*/
//	Copyright (C) 2001, genta


#include "CPropCommon.h"

//! Popup Help用ID
//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//10500
	0, 0
};
//@@@ 2001.02.04 End

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
BOOL CALLBACK CPropCommon::DlgProc_PROP_MACRO(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc(DispatchEvent_PROP_Macro, hwndDlg, uMsg, wParam, lParam );
}

/*! Macroページのメッセージ処理
	@param hwndDlg ダイアログボックスのWindow Handlw
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
BOOL CPropCommon::DispatchEvent_PROP_Macro( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 p1 */
		SetData_PROP_Macro( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		return TRUE;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_MACRO );
				return TRUE;
			case PSN_KILLACTIVE:
				/* ダイアログデータの取得 p1 */
				GetData_PROP_Macro( hwndDlg );
				return TRUE;
			}
			break;
//		}
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
		//	HELPINFO *p = (HELPINFO *)lParam;
		//	::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
		break;
//@@@ 2001.02.04 End

	}
	return FALSE;
}


/*!
	ダイアログ上のコントロールにデータを設定する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/
void CPropCommon::SetData_PROP_Macro( HWND hwndDlg )
{
	return;
}

/*!
	ダイアログ上のコントロールからデータを取得してメモリに格納する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/

int CPropCommon::GetData_PROP_Macro( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_MACRO;

	return TRUE;
}

/*[EOF]*/
