//	$Id$
/*!	@file
	
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
#include "sakura_rc.h"
#include "CDlgDebug.h"

/* �_�C�A���O�v���V�[�W�� */
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
	m_hInstance = NULL;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = NULL;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_hWnd = NULL;			/* ���̃_�C�A���O�̃n���h�� */

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




/* ���[�_���_�C�A���O�̕\�� */
int CDlgDebug::DoModal( HINSTANCE hInstance, HWND hwndParent, CMemory& cmemDebugInfo )
{
	m_hInstance = hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_cmemDebugInfo = cmemDebugInfo;

	return ::DialogBoxParam(
		m_hInstance,
		MAKEINTRESOURCE( IDD_DEBUG ),
		m_hwndParent,
		(DLGPROC)CDlgDebug_Proc,
		(LPARAM)this
	);
}



/* �_�C�A���O�̃��b�Z�[�W���� */
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
//	int		wJreVersion;
//	LONG	lStyle;
	switch( uMsg ){
	case WM_INITDIALOG:
		m_hWnd = hwndDlg;
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		::SetDlgItemText( hwndDlg, IDC_EDIT_DEBUG, m_cmemDebugInfo.GetPtr( NULL ) );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD( wParam );	/* �ʒm�R�[�h */
		wID			= LOWORD( wParam );	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */

		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDOK:			/* ������ */
			case IDCANCEL:
				::EndDialog( hwndDlg, 0 );
				return TRUE;
			}
		}
	}
	return FALSE;
}


/*[EOF]*/
