/*!	@file

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, KEITA

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "sakura_rc.h"
#include "dlg/CDlgDebug.h"

/* �_�C�A���O�v���V�[�W�� */
INT_PTR CALLBACK CDlgDebug_Proc(
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
		// Modified by KEITA for WIN64 2003.9.6
		pCDlgDebug = ( CDlgDebug* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
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
int CDlgDebug::DoModal( HINSTANCE hInstance, HWND hwndParent, const CNativeA& cmemDebugInfo )
{
	m_hInstance = hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_cmemDebugInfo = cmemDebugInfo;

	return (int)::DialogBoxParam(
		m_hInstance,
		MAKEINTRESOURCE( IDD_DEBUG ),
		m_hwndParent,
		(DLGPROC)CDlgDebug_Proc,
		(LPARAM)this
	);
}



/* �_�C�A���O�̃��b�Z�[�W���� */
INT_PTR CDlgDebug::DispatchEvent(
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
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		::SetDlgItemTextA( hwndDlg, IDC_EDIT_DEBUG, m_cmemDebugInfo.GetStringPtr() );

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
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	}
	return FALSE;
}



