//	$Id$
/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�ҏW�v�y�[�W

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, jepro

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "CPropCommon.h"
#include "debug.h" // 2002/2/10 aroka


//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//10210
	IDC_CHECK_ADDCRLFWHENCOPY,			HIDC_CHECK_ADDCRLFWHENCOPY,				//�܂�Ԃ��s�ɉ��s��t���ăR�s�[
	IDC_CHECK_COPYnDISABLESELECTEDAREA,	HIDC_CHECK_COPYnDISABLESELECTEDAREA,	//�R�s�[������I������
	IDC_CHECK_DRAGDROP,					HIDC_CHECK_DRAGDROP,					//Drag&Drop�ҏW����
	IDC_CHECK_DROPSOURCE,				HIDC_CHECK_DROPSOURCE,					//�h���b�v���ɂ���
	IDC_CHECK_bNotOverWriteCRLF,		HIDC_CHECK_bNotOverWriteCRLF,			//�㏑�����[�h
//	IDC_STATIC,							-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//10210
	IDC_CHECK_ADDCRLFWHENCOPY,			10210,	//�܂�Ԃ��s�ɉ��s��t���ăR�s�[
	IDC_CHECK_COPYnDISABLESELECTEDAREA,	10211,	//�R�s�[������I������
	IDC_CHECK_DRAGDROP,					10212,	//Drag&Drop�ҏW����
	IDC_CHECK_DROPSOURCE,				10213,	//�h���b�v���ɂ���
	IDC_CHECK_bNotOverWriteCRLF,		10214,	//�㏑�����[�h
//	IDC_STATIC,							-1,
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
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_EDIT(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_PROP_EDIT, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* ���b�Z�[�W���� */
INT_PTR CPropCommon::DispatchEvent_PROP_EDIT(
    HWND		hwndDlg,	// handle to dialog box
    UINT		uMsg,		// message
    WPARAM		wParam,		// first message parameter
    LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
//	LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p1 */
		SetData_PROP_EDIT( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_DRAGDROP:	/* �^�X�N�g���C���g�� */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP ) ){
//	From Here Sept. 9, 2000 JEPRO
//	�O�̃`�F�b�N��Ԃ��c��悤�Ɏ��̍s���R�����g�A�E�g�ɕύX
//					::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, TRUE );	/* DropSource */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), TRUE );
//	To Here Sept. 9, 2000
				}else{
//	From Here Sept. 9, 2000 JEPRO
//	�O�̃`�F�b�N��Ԃ��c��悤�Ɏ��̍s���R�����g�A�E�g�ɕύX
//					::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, FALSE );	/* DropSource */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), FALSE );
//	To Here Sept. 9, 2000
				}
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_EDIT );
				return TRUE;
			case PSN_KILLACTIVE:
#ifdef _DEBUG
				MYTRACE( "p1 PSN_KILLACTIVE\n" );
#endif
				/* �_�C�A���O�f�[�^�̎擾 p1 */
				GetData_PROP_EDIT( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_EDIT;	//Oct. 25, 2000 JEPRO ZENPAN1��ZENPAN �ɕύX(�Q�Ƃ��Ă���̂�CPropCommon.cpp�݂̂�1�ӏ�)
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
void CPropCommon::SetData_PROP_EDIT( HWND hwndDlg )
{
//	BOOL	bRet;

	/* �h���b�O & �h���b�v�ҏW */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DRAGDROP, m_Common.m_bUseOLE_DragDrop );
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP ) ){
//	From Here Sept. 9, 2000 JEPRO
//	�O�̃`�F�b�N��Ԃ��c��悤�Ɏ��̍s���R�����g�A�E�g�ɕύX
//		::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, TRUE );	/* DropSource */
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), TRUE );
//	To Here Sept. 9, 2000
	}else{
//	From Here Sept. 9, 2000 JEPRO
//	�O�̃`�F�b�N��Ԃ��c��悤�Ɏ��̍s���R�����g�A�E�g�ɕύX
//		::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, FALSE );	/* DropSource */
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), FALSE );
//	To Here Sept. 9, 2000
	}

	/* DropSource */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, m_Common.m_bUseOLE_DropSource );

	/* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY, m_Common.m_bAddCRLFWhenCopy );

	/* �R�s�[������I������ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA, m_Common.m_bCopyAndDisablSelection );

	/* ���s�͏㏑�����Ȃ� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bNotOverWriteCRLF, m_Common.m_bNotOverWriteCRLF );
	return;
}





/* �_�C�A���O�f�[�^�̎擾 */
int CPropCommon::GetData_PROP_EDIT( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_EDIT;

	/* �h���b�O & �h���b�v�ҏW */
	m_Common.m_bUseOLE_DragDrop = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP );
	/* DropSource */
	m_Common.m_bUseOLE_DropSource = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DROPSOURCE );

	/* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
	m_Common.m_bAddCRLFWhenCopy = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY );

	/* �R�s�[������I������ */
	m_Common.m_bCopyAndDisablSelection = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA );

	/* ���s�͏㏑�����Ȃ� */
	m_Common.m_bNotOverWriteCRLF = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bNotOverWriteCRLF );
	return TRUE;
}


/*[EOF]*/
