//	$Id$
/************************************************************************
	CPropComUrl.cpp
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/

#include "CPropCommon.h"

//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//11100
	IDC_CHECK_bSelectClickedURL,	11110,	//クリッカブルURL
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End




/* メッセージ処理 */
BOOL CPropCommon::DispatchEvent_PROP_URL( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	WORD		wNotifyCode;
//	WORD		wID;
//	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
//    LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 p1 */
		SetData_PROP_URL( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */

		return TRUE;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_URL );
				return TRUE;
			case PSN_KILLACTIVE:
				/* ダイアログデータの取得 p1 */
				GetData_PROP_URL( hwndDlg );
				return TRUE;
			}
			break;
//		}
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


/* ダイアログデータの設定 */
void CPropCommon::SetData_PROP_URL( HWND hwndDlg )
{
//	BOOL	bRet;

//	BOOL	m_bSelectClickedURL;	/* URLがクリックされたら選択するか */


	/* URLがクリックされたら選択するか */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bSelectClickedURL, m_Common.m_bSelectClickedURL );

	return;
}





/* ダイアログデータの取得 */
int CPropCommon::GetData_PROP_URL( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_URL;



	/* URLがクリックされたら選択するか */
	m_Common.m_bSelectClickedURL = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bSelectClickedURL );


	return TRUE;
}


/*[EOF]*/
