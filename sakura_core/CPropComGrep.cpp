//	$Id$
/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�uGrep�v�y�[�W

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

#include "CPropCommon.h"

//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//10500
	IDC_CHECK_bGrepExitConfirm,		10510,	//GREP�̕ۑ��m�F
	IDC_CHECK_GTJW_RETURN,			10511,	//�^�O�W�����v�i�G���^�[�L�[�j
	IDC_CHECK_GTJW_LDBLCLK,			10512,	//�^�O�W�����v�i�_�u���N���b�N�j
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
BOOL CALLBACK CPropCommon::DlgProc_PROP_GREP(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_PROP_GREP, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* ���b�Z�[�W���� */
BOOL CPropCommon::DispatchEvent_PROP_GREP( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

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
			}
			break;
//		}
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
		break;
//@@@ 2001.02.04 End

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

	/* Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v */
	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_RETURN, m_Common.m_bGTJW_RETURN );

	/* Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v */
	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_LDBLCLK, m_Common.m_bGTJW_LDBLCLK );

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
int CPropCommon::GetData_PROP_GREP( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_GREP;

	/* Grep���[�h�ŕۑ��m�F���邩 */
	m_Common.m_bGrepExitConfirm = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bGrepExitConfirm );

	/* Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v */
	m_Common.m_bGTJW_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_RETURN );

	/* Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v */
	m_Common.m_bGTJW_LDBLCLK = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_LDBLCLK );

	return TRUE;
}


/*[EOF]*/
