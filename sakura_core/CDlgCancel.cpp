/*!	@file
	@brief キャンセルボタンダイアログボックス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CDlgCancel.h"
#include "debug.h"

CDlgCancel::CDlgCancel()
{
	m_bCANCEL = FALSE;	/* IDCANCELボタンが押された */
	return;
}
/* モーダルダイアログの表示 */
int CDlgCancel::DoModal( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete )
{
	m_bCANCEL = FALSE;	/* IDCANCELボタンが押された */
	return (int)CDialog::DoModal( hInstance, hwndParent, nDlgTemplete, NULL );
}
/* モードレスダイアログの表示 */
HWND CDlgCancel::DoModeless( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete )
{
	m_bCANCEL = FALSE;	/* IDCANCELボタンが押された */
	return CDialog::DoModeless( hInstance, hwndParent, nDlgTemplete, NULL, SW_SHOW );
}


BOOL CDlgCancel::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;
	HICON	hIcon;
	hIcon = ::LoadIcon( NULL, IDI_ASTERISK );
//	hIcon = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_GREP ) );
	::SendMessage( m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)NULL );
	::SendMessage( m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon );
	::SendMessage( m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)NULL );
	::SendMessage( m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon );


	/* 基底クラスメンバ */
//	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}

BOOL CDlgCancel::OnBnClicked( int wID )
{
	switch( wID ){
	case IDCANCEL:
		m_bCANCEL = TRUE;	/* IDCANCELボタンが押された */
//		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}

//@@@ 2002.01.18 add start
const DWORD p_helpids[] = {
	0, 0
};

LPVOID CDlgCancel::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
