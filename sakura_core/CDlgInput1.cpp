//	$Id$
/*!	@file
	1�s���̓_�C�A���O�{�b�N�X
	
	@author Norio Nakatani
	@date	1998/05/31 �쐬
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



/* �_�C�A���O�v���V�[�W�� */
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



/* ���[�h���X�_�C�A���O�̕\�� */
BOOL CDlgInput1::DoModal( HINSTANCE hInstApp, HWND hwndParent, const char* pszTitle, const char* pszMessage, int nMaxTextLen, char* pszText )
{
	BOOL bRet;
	m_hInstance = hInstApp;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_pszTitle = pszTitle;		/* �_�C�A���O�^�C�g�� */
	m_pszMessage = pszMessage;		/* ���b�Z�[�W */
	m_nMaxTextLen = nMaxTextLen;	/* ���̓T�C�Y��� */
//	m_pszText = pszText;			/* �e�L�X�g */
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



/* �_�C�A���O�̃��b�Z�[�W���� */
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
		/* �_�C�A���O�f�[�^�̐ݒ� */
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		::SetWindowText( hwndDlg, m_pszTitle );	/* �_�C�A���O�^�C�g�� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT1 ), EM_LIMITTEXT, m_nMaxTextLen, 0 );	/* ���̓T�C�Y��� */
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT1 ), m_cmemText.GetPtr( NULL ) );	/* �e�L�X�g */
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_STATIC_MSG ), m_pszMessage );	/* ���b�Z�[�W */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID         = LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl     = (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDOK:
				m_cmemText.AllocBuffer( ::GetWindowTextLength( ::GetDlgItem( hwndDlg, IDC_EDIT1 ) ) );
				::GetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT1 ), m_cmemText.GetPtr( NULL ), m_nMaxTextLen + 1 );	/* �e�L�X�g */
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
