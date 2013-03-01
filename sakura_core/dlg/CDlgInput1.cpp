/*!	@file
	@brief 1�s���̓_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/05/31 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgInput1.h"
#include "CEditApp.h"
#include "Funccode_enum.h"	// EFunctionCode
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

// ���� CDlgInput1.cpp	//@@@ 2002.01.07 add start MIK
static const DWORD p_helpids[] = {	//13000
	IDOK,					HIDOK_DLG1,
	IDCANCEL,				HIDCANCEL_DLG1,
	IDC_EDIT_INPUT1,		HIDC_DLG1_EDIT1,	//���̓t�B�[���h	IDC_EDIT1->IDC_EDIT_INPUT1	2008/7/3 Uchi
	IDC_STATIC_MSG,			HIDC_DLG1_EDIT1,	//���b�Z�[�W
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK


/* �_�C�A���O�v���V�[�W�� */
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
	/* �w���v�t�@�C���̃t���p�X��Ԃ� */
	m_pszHelpFile = CEditApp::getInstance()->GetHelpFilePath();	//@@@ 2002.01.07 add

	return;
}



CDlgInput1::~CDlgInput1()
{
	return;
}



/* ���[�h���X�_�C�A���O�̕\�� */
BOOL CDlgInput1::DoModal(
	HINSTANCE		hInstApp,
	HWND			hwndParent,
	const TCHAR*	pszTitle,
	const TCHAR*	pszMessage,
	int				nMaxTextLen,
	TCHAR*			pszText
)
{
	BOOL bRet;
	m_hInstance = hInstApp;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_pszTitle = pszTitle;		/* �_�C�A���O�^�C�g�� */
	m_pszMessage = pszMessage;		/* ���b�Z�[�W */
	m_nMaxTextLen = nMaxTextLen;	/* ���̓T�C�Y��� */
//	m_pszText = pszText;			/* �e�L�X�g */
	m_cmemText.SetString( pszText );
	bRet = (BOOL)::DialogBoxParam(
		m_hInstance,
		MAKEINTRESOURCE( IDD_INPUT1 ),
		m_hwndParent,
		CDlgInput1Proc,
		(LPARAM)this
	);
	_tcscpy( pszText, m_cmemText.GetStringPtr() );
	return bRet;
}

BOOL CDlgInput1::DoModal(
	HINSTANCE		hInstApp,
	HWND			hwndParent,
	const TCHAR*	pszTitle,
	const TCHAR*	pszMessage,
	int				nMaxTextLen,
	NOT_TCHAR*		pszText
)
{
	TCHAR buf[1024];
	buf[0] = _T('\0');
	BOOL ret=DoModal(hInstApp, hwndParent, pszTitle, pszMessage, nMaxTextLen, buf);
	if(ret){
		auto_strcpy(pszText,to_not_tchar(buf));
	}
	return ret;
}



/* �_�C�A���O�̃��b�Z�[�W���� */
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
		/* �_�C�A���O�f�[�^�̐ݒ� */
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		::SetWindowText( hwndDlg, m_pszTitle );	/* �_�C�A���O�^�C�g�� */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_INPUT1 ), m_nMaxTextLen );	/* ���̓T�C�Y��� */
		DlgItem_SetText( hwndDlg, IDC_EDIT_INPUT1, m_cmemText.GetStringPtr() );	/* �e�L�X�g */
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_STATIC_MSG ), m_pszMessage );	/* ���b�Z�[�W */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDOK:
				m_cmemText.AllocStringBuffer( ::GetWindowTextLength( ::GetDlgItem( hwndDlg, IDC_EDIT_INPUT1 ) ) );
				::GetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT_INPUT1 ), m_cmemText.GetStringPtr(), m_nMaxTextLen + 1 );	/* �e�L�X�g */
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
			MyWinHelp( (HWND)p->hItemHandle, m_pszHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;

	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_pszHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	//@@@ 2002.01.07 add end
	}
	return FALSE;
}



