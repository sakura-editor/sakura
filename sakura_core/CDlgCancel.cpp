//	$Id$
/************************************************************************
	CDlgCancel.cpp
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/
//#include <stdio.h>
#include "CDlgCancel.h"
#include "debug.h"

CDlgCancel::CDlgCancel()
{
	m_bCANCEL = FALSE;	/* IDCANCEL�{�^���������ꂽ */
	return;
}
/* ���[�_���_�C�A���O�̕\�� */
int CDlgCancel::DoModal( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete )
{
	m_bCANCEL = FALSE;	/* IDCANCEL�{�^���������ꂽ */
	return CDialog::DoModal( hInstance, hwndParent, nDlgTemplete, NULL );
}
/* ���[�h���X�_�C�A���O�̕\�� */
HWND CDlgCancel::DoModeless( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete )
{
	m_bCANCEL = FALSE;	/* IDCANCEL�{�^���������ꂽ */
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

	
	/* ���N���X�����o */
//	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}

BOOL CDlgCancel::OnBnClicked( int wID )
{
	switch( wID ){
	case IDCANCEL:
		m_bCANCEL = TRUE;	/* IDCANCEL�{�^���������ꂽ */
//		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}

/*[EOF]*/
