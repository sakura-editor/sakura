//	$Id$
/*!	@file
	共通設定ダイアログボックス、「編集」ページ

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


//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//10210
	IDC_CHECK_ADDCRLFWHENCOPY,			10210,	//折り返し行に改行を付けてコピー
	IDC_CHECK_COPYnDISABLESELECTEDAREA,	10211,	//コピーしたら選択解除
	IDC_CHECK_DRAGDROP,					10212,	//Drag&Drop編集する
	IDC_CHECK_DROPSOURCE,				10213,	//ドロップ元にする
	IDC_CHECK_bNotOverWriteCRLF,		10214,	//上書きモード
//	IDC_STATIC,							-1,
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
BOOL CALLBACK CPropCommon::DlgProc_PROP_EDIT(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_PROP_EDIT, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* メッセージ処理 */
BOOL CPropCommon::DispatchEvent_PROP_EDIT(
    HWND		hwndDlg,	// handle to dialog box
    UINT		uMsg,		// message
    WPARAM		wParam,		// first message parameter
    LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
//	LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 p1 */
		SetData_PROP_EDIT( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_DRAGDROP:	/* タスクトレイを使う */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP ) ){
//	From Here Sept. 9, 2000 JEPRO
//	前のチェック状態が残るように次の行をコメントアウトに変更
//					::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, TRUE );	/* DropSource */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), TRUE );
//	To Here Sept. 9, 2000
				}else{
//	From Here Sept. 9, 2000 JEPRO
//	前のチェック状態が残るように次の行をコメントアウトに変更
//					::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, FALSE );	/* DropSource */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), FALSE );
//	To Here Sept. 9, 2000
				}
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_EDIT );
				return TRUE;
			case PSN_KILLACTIVE:
#ifdef _DEBUG
				MYTRACE( "p1 PSN_KILLACTIVE\n" );
#endif
				/* ダイアログデータの取得 p1 */
				GetData_PROP_EDIT( hwndDlg );
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
void CPropCommon::SetData_PROP_EDIT( HWND hwndDlg )
{
//	BOOL	bRet;

	/* ドラッグ & ドロップ編集 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DRAGDROP, m_Common.m_bUseOLE_DragDrop );
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP ) ){
//	From Here Sept. 9, 2000 JEPRO
//	前のチェック状態が残るように次の行をコメントアウトに変更
//		::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, TRUE );	/* DropSource */
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), TRUE );
//	To Here Sept. 9, 2000
	}else{
//	From Here Sept. 9, 2000 JEPRO
//	前のチェック状態が残るように次の行をコメントアウトに変更
//		::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, FALSE );	/* DropSource */
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), FALSE );
//	To Here Sept. 9, 2000
	}

	/* DropSource */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, m_Common.m_bUseOLE_DropSource );

	/* 折り返し行に改行を付けてコピー */
	::CheckDlgButton( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY, m_Common.m_bAddCRLFWhenCopy );

	/* コピーしたら選択解除 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA, m_Common.m_bCopyAndDisablSelection );

	/* 改行は上書きしない */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bNotOverWriteCRLF, m_Common.m_bNotOverWriteCRLF );
	return;
}





/* ダイアログデータの取得 */
int CPropCommon::GetData_PROP_EDIT( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_EDIT;

	/* ドラッグ & ドロップ編集 */
	m_Common.m_bUseOLE_DragDrop = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP );
	/* DropSource */
	m_Common.m_bUseOLE_DropSource = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DROPSOURCE );

	/* 折り返し行に改行を付けてコピー */
	m_Common.m_bAddCRLFWhenCopy = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY );

	/* コピーしたら選択解除 */
	m_Common.m_bCopyAndDisablSelection = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA );

	/* 改行は上書きしない */
	m_Common.m_bNotOverWriteCRLF = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bNotOverWriteCRLF );
	return TRUE;
}


/*[EOF]*/
