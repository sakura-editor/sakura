//	$Id$
//	Copyright (C) 1998-2000, Norio Nakatani

#include "CPropCommon.h"





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
	}
	return FALSE;
}


/* ダイアログデータの設定 */
void CPropCommon::SetData_PROP_URL( HWND hwndDlg )
{
//	BOOL	bRet;

//	BOOL				m_bSelectClickedURL;	/* URLがクリックされたら選択するか */


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


