//	$Id$
/*!	@file

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgDebug.h"

/* ダイアログプロシージャ */
BOOL CALLBACK CDlgDebug_Proc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	CDlgDebug* pCDlgDebug;
	switch( uMsg ){
	case WM_INITDIALOG:
		pCDlgDebug = ( CDlgDebug* )lParam;
		if( NULL != pCDlgDebug ){
			return pCDlgDebug->DispatchEvent( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	default:
		pCDlgDebug = ( CDlgDebug* )::GetWindowLong( hwndDlg, DWL_USER );
		if( NULL != pCDlgDebug ){
			return pCDlgDebug->DispatchEvent( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}



CDlgDebug::CDlgDebug()
{
	m_hInstance = NULL;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */
	m_hWnd = NULL;			/* このダイアログのハンドル */

	return;
}



CDlgDebug::~CDlgDebug()
{
//	int i;
//	for( i = 0; i < MAX_TEXTARR; ++i ){
//		delete m_ppTextArr[i];
//		m_ppTextArr[i] = NULL;
//	}
	return;
}




/* モーダルダイアログの表示 */
int CDlgDebug::DoModal( HINSTANCE hInstance, HWND hwndParent, CMemory& cmemDebugInfo )
{
	m_hInstance = hInstance;	/* アプリケーションインスタンスのハンドル */
	m_hwndParent = hwndParent;	/* オーナーウィンドウのハンドル */
	m_cmemDebugInfo = cmemDebugInfo;

	return ::DialogBoxParam(
		m_hInstance,
		MAKEINTRESOURCE( IDD_DEBUG ),
		m_hwndParent,
		(DLGPROC)CDlgDebug_Proc,
		(LPARAM)this
	);
}



/* ダイアログのメッセージ処理 */
BOOL CDlgDebug::DispatchEvent(
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
//	LONG	lStyle;
	switch( uMsg ){
	case WM_INITDIALOG:
		m_hWnd = hwndDlg;
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		::SetDlgItemText( hwndDlg, IDC_EDIT_DEBUG, m_cmemDebugInfo.GetPtr( NULL ) );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD( wParam );	/* 通知コード */
		wID			= LOWORD( wParam );	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */

		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDOK:			/* 下検索 */
			case IDCANCEL:
				::EndDialog( hwndDlg, 0 );
				return TRUE;
			}
		}
	}
	return FALSE;
}


/*[EOF]*/
