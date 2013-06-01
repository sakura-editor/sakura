/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�X�e�[�^�X�o�[�v�y�[�W

	@author Uchi
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, jepro, genta
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta
	Copyright (C) 2007, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {
	IDC_CHECK_DISP_UNICODE_IN_SJIS,		HIDC_CHECK_DISP_UNICODE_IN_SJIS,		// SJIS�ŕ����R�[�h�l��Unicode�ŕ\������
	IDC_CHECK_DISP_UNICODE_IN_JIS,		HIDC_CHECK_DISP_UNICODE_IN_JIS,			// JIS�ŕ����R�[�h�l��Unicode�ŕ\������
	IDC_CHECK_DISP_UNICODE_IN_EUC,		HIDC_CHECK_DISP_UNICODE_IN_EUC,			// EUC�ŕ����R�[�h�l��Unicode�ŕ\������
	IDC_CHECK_DISP_UTF8_CODEPOINT,		HIDC_CHECK_DISP_UTF8_CODEPOINT,			// UTF-8���R�[�h�|�C���g�ŕ\������
	IDC_CHECK_DISP_SP_CODEPOINT,		HIDC_CHECK_DISP_SP_CODEPOINT,			// �T���Q�[�g�y�A���R�[�h�|�C���g�ŕ\������
	IDC_CHECK_DISP_SELCOUNT_BY_BYTE,	HIDC_CHECK_DISP_SELCOUNT_BY_BYTE,		// �I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
	0, 0
};

/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropStatusbar::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropStatusbar::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}

/* ���b�Z�[�W���� */
INT_PTR CPropStatusbar::DispatchEvent(
    HWND		hwndDlg,	// handle to dialog box
    UINT		uMsg,		// message
    WPARAM		wParam,		// first message parameter
    LPARAM		lParam 		// second message parameter
)
{
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		return TRUE;
	case WM_COMMAND:
		break;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_STATUSBAR );
			return TRUE;
		case PSN_KILLACTIVE:
			DEBUG_TRACE( _T("statusbar PSN_KILLACTIVE\n") );

			/* �_�C�A���O�f�[�^�̎擾 */
			GetData( hwndDlg );
			return TRUE;

		case PSN_SETACTIVE: //@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			m_nPageNum = ID_PAGENUM_STATUSBAR;
			return TRUE;
		}
		break;	/* WM_NOTIFY */

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CPropStatusbar::SetData( HWND hwndDlg )
{
	// �������R�[�h�̎w��
	// SJIS�ŕ����R�[�h�l��Unicode�ŏo�͂���
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS, m_Common.m_sStatusbar.m_bDispUniInSjis );
	// JIS�ŕ����R�[�h�l��Unicode�ŏo�͂���
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS,  m_Common.m_sStatusbar.m_bDispUniInJis );
	// EUC�ŕ����R�[�h�l��Unicode�ŏo�͂���
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC,  m_Common.m_sStatusbar.m_bDispUniInEuc );
	// UTF-8�ŕ\�����o�C�g�R�[�h�ōs��
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UTF8_CODEPOINT,  m_Common.m_sStatusbar.m_bDispUtf8Codepoint );
	// �T���Q�[�g�y�A���R�[�h�|�C���g�ŕ\��
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT,    m_Common.m_sStatusbar.m_bDispSPCodepoint );
	// �I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_SELCOUNT_BY_BYTE,m_Common.m_sStatusbar.m_bDispSelCountByByte );
	return;
}


/* �_�C�A���O�f�[�^�̎擾 */
int CPropStatusbar::GetData( HWND hwndDlg )
{
	// �������R�[�h�̎w��
	// SJIS�ŕ����R�[�h�l��Unicode�ŏo�͂���
	m_Common.m_sStatusbar.m_bDispUniInSjis		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS );
	// JIS�ŕ����R�[�h�l��Unicode�ŏo�͂���
	m_Common.m_sStatusbar.m_bDispUniInJis		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS );
	// EUC�ŕ����R�[�h�l��Unicode�ŏo�͂���
	m_Common.m_sStatusbar.m_bDispUniInEuc		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC );
	// UTF-8�ŕ\�����o�C�g�R�[�h�ōs��
	m_Common.m_sStatusbar.m_bDispUtf8Codepoint	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UTF8_CODEPOINT );
	// �T���Q�[�g�y�A���R�[�h�|�C���g�ŕ\��
	m_Common.m_sStatusbar.m_bDispSPCodepoint	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT );
	// �I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
	m_Common.m_sStatusbar.m_bDispSelCountByByte	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_SELCOUNT_BY_BYTE );

	return TRUE;
}
