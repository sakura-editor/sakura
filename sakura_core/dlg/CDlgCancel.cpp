/*!	@file
	@brief �L�����Z���{�^���_�C�A���O�{�b�N�X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "dlg/CDlgCancel.h"
#include "debug/Debug.h"

CDlgCancel::CDlgCancel()
{
	m_bCANCEL = FALSE;	/* IDCANCEL�{�^���������ꂽ */
	m_bAutoCleanup = false;
	return;
}

/** �W���ȊO�̃��b�Z�[�W��ߑ�����
	@date 2008.05.28 ryoji �V�K�쐬
*/
INT_PTR CDlgCancel::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_CLOSE:
		if( m_bAutoCleanup ){
			::DestroyWindow( GetHwnd() );
			return TRUE;
		}
		break;
	case WM_NCDESTROY:
		if( m_bAutoCleanup ){
			delete this;
			return TRUE;
		}
		break;
	}
	return result;
}

/** �����j����x�����s����
	@date 2008.05.28 ryoji �V�K�쐬
*/
void CDlgCancel::DeleteAsync( void )
{
	m_bAutoCleanup = true;
	::PostMessageAny( GetHwnd(), WM_CLOSE, 0, 0 );
}

/* ���[�_���_�C�A���O�̕\�� */
int CDlgCancel::DoModal( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete )
{
	m_bCANCEL = FALSE;	/* IDCANCEL�{�^���������ꂽ */
	return (int)CDialog::DoModal( hInstance, hwndParent, nDlgTemplete, NULL );
}
/* ���[�h���X�_�C�A���O�̕\�� */
HWND CDlgCancel::DoModeless( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete )
{
	m_bCANCEL = FALSE;	/* IDCANCEL�{�^���������ꂽ */
	return CDialog::DoModeless( hInstance, hwndParent, nDlgTemplete, NULL, SW_SHOW );
}


BOOL CDlgCancel::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );
	HICON	hIcon;
	hIcon = ::LoadIcon( NULL, IDI_ASTERISK );
//	hIcon = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_GREP ) );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)NULL );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)NULL );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)hIcon );


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

//@@@ 2002.01.18 add start
const DWORD p_helpids[] = {
	0, 0
};

LPVOID CDlgCancel::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


