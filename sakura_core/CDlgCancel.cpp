//	$Id$
/*!	@file
	�L�����Z���{�^���_�C�A���O�{�b�N�X

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
