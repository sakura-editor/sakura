//	$Id$
/*!	@file
	@brief 1行入力ダイアログボックス

	@author Norio Nakatani
	@date	1998/05/31 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "sakura_rc.h"
#include "CDlgInput1.h"
#include "debug.h"

// 入力 CDlgInput1.cpp	//@@@ 2002.01.07 add start MIK
#include "etc_uty.h"
#include "sakura.hh"
static const DWORD p_helpids[] = {	//13000
	IDOK,					HIDOK_DLG1,
	IDCANCEL,				HIDCANCEL_DLG1,
	IDC_EDIT1,				HIDC_DLG1_EDIT1,	//入力フィールド
	IDC_STATIC_MSG,			HIDC_DLG1_EDIT1,	//メッセージ
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK


/* ダイアログプロシージャ */
INT_PTR CALLBACK CDlgInput1Proc(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
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
		// Modified by KEITA for WIN64 2003.9.6
		pCDlgInput1 = ( CDlgInput1* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCDlgInput1 ){
			return pCDlgInput1->DispatchEvent( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}



CDlgInput1::CDlgInput1()
{
	/* ヘルプファイルのフルパスを返す */
	::GetHelpFilePath( m_szHelpFile );	//@@@ 2002.01.07 add

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
	bRet = (BOOL)::DialogBoxParam(
		m_hInstance,
		MAKEINTRESOURCE( IDD_INPUT1 ),
		m_hwndParent,
		CDlgInput1Proc,
		(LPARAM)this
	);
	strcpy( pszText, m_cmemText.GetPtr() );
	return bRet;
}



/* ダイアログのメッセージ処理 */
INT_PTR CDlgInput1::DispatchEvent(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
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
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		::SetWindowText( hwndDlg, m_pszTitle );	/* ダイアログタイトル */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT1 ), EM_LIMITTEXT, m_nMaxTextLen, 0 );	/* 入力サイズ上限 */
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT1 ), m_cmemText.GetPtr() );	/* テキスト */
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_STATIC_MSG ), m_pszMessage );	/* メッセージ */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDOK:
				m_cmemText.AllocBuffer( ::GetWindowTextLength( ::GetDlgItem( hwndDlg, IDC_EDIT1 ) ) );
				::GetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT1 ), m_cmemText.GetPtr(), m_nMaxTextLen + 1 );	/* テキスト */
				::EndDialog( hwndDlg, TRUE );
				return TRUE;
			case IDCANCEL:
				::EndDialog( hwndDlg, FALSE );
				return TRUE;
			}
			break;	//@@@ 2002.01.07 add
		}
		break;	//@@@ 2002.01.07 add
	//@@@ 2002.01.07 add start
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );
		}
		return TRUE;

	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );
		return TRUE;
	//@@@ 2002.01.07 add end
	}
	return FALSE;
}


/*[EOF]*/
