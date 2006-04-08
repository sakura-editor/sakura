//	$Id$
/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�uGrep�v�y�[�W

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, genta
	Copyright (C) 2002, YAZAKI, MIK
	Copyright (C) 2003, KEITA

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CPropCommon.h"

//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//10500
	IDC_CHECK_bGrepExitConfirm,		HIDC_CHECK_bGrepExitConfirm,	//GREP�̕ۑ��m�F
	IDC_CHECK_GTJW_RETURN,			HIDC_CHECK_GTJW_RETURN,			//�^�O�W�����v�i�G���^�[�L�[�j
	IDC_CHECK_GTJW_LDBLCLK,			HIDC_CHECK_GTJW_LDBLCLK,		//�^�O�W�����v�i�_�u���N���b�N�j
//	IDC_STATIC,						-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//10500
	IDC_CHECK_bGrepExitConfirm,		10510,	//GREP�̕ۑ��m�F
	IDC_CHECK_GTJW_RETURN,			10511,	//�^�O�W�����v�i�G���^�[�L�[�j
	IDC_CHECK_GTJW_LDBLCLK,			10512,	//�^�O�W�����v�i�_�u���N���b�N�j
//	IDC_STATIC,						-1,
	0, 0
};
#endif
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_GREP(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_PROP_GREP, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* ���b�Z�[�W���� */
INT_PTR CPropCommon::DispatchEvent_PROP_GREP( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	WORD		wNotifyCode;
//	WORD		wID;
//	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
//    LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p1 */
		SetData_PROP_GREP( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */

		return TRUE;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_GREP );
				return TRUE;
			case PSN_KILLACTIVE:
				/* �_�C�A���O�f�[�^�̎擾 p1 */
				GetData_PROP_GREP( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_GREP;
				return TRUE;
			}
//			break;	/* default */
//		}
		break;	/* WM_NOTIFY */

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CPropCommon::SetData_PROP_GREP( HWND hwndDlg )
{
//	BOOL	bRet;

//	BOOL	m_bGrepExitConfirm;	/* Grep���[�h�ŕۑ��m�F���邩 */


	/* Grep���[�h�ŕۑ��m�F���邩 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bGrepExitConfirm, m_Common.m_bGrepExitConfirm );

	/* Grep���ʂ̃��A���^�C���\�� */
	::CheckDlgButton( hwndDlg, IDC_CHECH_GREPREALTIME, m_Common.m_bGrepRealTimeView );


	/* Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v */
	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_RETURN, m_Common.m_bGTJW_RETURN );

	/* Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v */
	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_LDBLCLK, m_Common.m_bGTJW_LDBLCLK );

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
int CPropCommon::GetData_PROP_GREP( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_GREP;

	/* Grep���[�h�ŕۑ��m�F���邩 */
	m_Common.m_bGrepExitConfirm = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bGrepExitConfirm );

	/* Grep���ʂ̃��A���^�C���\�� */
	m_Common.m_bGrepRealTimeView = ::IsDlgButtonChecked( hwndDlg, IDC_CHECH_GREPREALTIME );

	/* Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v */
	m_Common.m_bGTJW_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_RETURN );

	/* Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v */
	m_Common.m_bGTJW_LDBLCLK = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_LDBLCLK );

	return TRUE;
}


/*[EOF]*/
