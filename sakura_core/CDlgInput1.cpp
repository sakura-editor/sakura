//	$Id$
/*!	@file
	1行入力ダイアログボックス
	
	@author Norio Nakatani
	@date	1998/05/31 作成
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
//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgInput1.h"
#include "debug.h"



/* ダイアログプロシージャ */
BOOL CALLBACK CDlgInput1Proc(
    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
)
{
	CDlgInput1* pCDlgInput1;
	switch( uMsg ){
	case WM_INITDIALOG:
		pCDlgInput1 = ( CDlgInput1* )lParam;
		if( NULL != pCDlgInput1 ){
			return pCDlgInput1->DispatchEvent( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	default:
		pCDlgInput1 = ( CDlgInput1* )::GetWindowLong( hwndDlg, DWL_USER );
		if( NULL != pCDlgInput1 ){
			return pCDlgInput1->DispatchEvent( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}



CDlgInput1::CDlgInput1()
{
	return;
}



CDlgInput1::~CDlgInput1()
{
	return;
}



/* モードレスダイアログの表示 */
BOOL CDlgInput1::DoModal( HINSTANCE hInstApp, HWND hwndParent, const char* pszTitle, const char* pszMessage, int nMaxTextLen, char* pszText )
{
	BOOL bRet;
	m_hInstance = hInstApp;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = hwndParent;	/* オーナーウィンドウのハンドル */
	m_pszTitle = pszTitle;		/* ダイアログタイトル */
	m_pszMessage = pszMessage;		/* メッセージ */
	m_nMaxTextLen = nMaxTextLen;	/* 入力サイズ上限 */
//	m_pszText = pszText;			/* テキスト */
	m_cmemText.SetDataSz( pszText );
	bRet = ::DialogBoxParam(
		m_hInstance,
		MAKEINTRESOURCE( IDD_INPUT1 ),
		m_hwndParent,
		(DLGPROC)CDlgInput1Proc,
		(LPARAM)this
	);
	strcpy( pszText, m_cmemText.GetPtr( NULL ) );
	return bRet;
}



/* ダイアログのメッセージ処理 */
BOOL CDlgInput1::DispatchEvent(
    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
)
{
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl;
//	int		nRet;
	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 */
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		::SetWindowText( hwndDlg, m_pszTitle );	/* ダイアログタイトル */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT1 ), EM_LIMITTEXT, m_nMaxTextLen, 0 );	/* 入力サイズ上限 */
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT1 ), m_cmemText.GetPtr( NULL ) );	/* テキスト */
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_STATIC_MSG ), m_pszMessage );	/* メッセージ */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID         = LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl     = (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDOK:
				m_cmemText.AllocBuffer( ::GetWindowTextLength( ::GetDlgItem( hwndDlg, IDC_EDIT1 ) ) );
				::GetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT1 ), m_cmemText.GetPtr( NULL ), m_nMaxTextLen + 1 );	/* テキスト */
				::EndDialog( hwndDlg, TRUE );
				return TRUE;
			case IDCANCEL:
				::EndDialog( hwndDlg, FALSE );
				return TRUE;
			}
		}
	}
	return FALSE;
}



/*[EOF]*/
