/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�ҏW�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, jepro, genta
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10210
	IDC_CHECK_ADDCRLFWHENCOPY,			HIDC_CHECK_ADDCRLFWHENCOPY,				//�܂�Ԃ��s�ɉ��s��t���ăR�s�[
	IDC_CHECK_COPYnDISABLESELECTEDAREA,	HIDC_CHECK_COPYnDISABLESELECTEDAREA,	//�R�s�[������I������
	IDC_CHECK_bEnableNoSelectCopy,		HIDC_CHECK_bEnableNoSelectCopy,			//�I���Ȃ��ŃR�s�[���\�ɂ���	// 2007.11.18 ryoji
	IDC_CHECK_bEnableLineModePaste,		HIDC_CHECK_bEnableLineModePaste,		//���C�����[�h�\��t�����\�ɂ���	// 2007.10.08 ryoji
	IDC_CHECK_DRAGDROP,					HIDC_CHECK_DRAGDROP,					//Drag&Drop�ҏW����
	IDC_CHECK_DROPSOURCE,				HIDC_CHECK_DROPSOURCE,					//�h���b�v���ɂ���
	IDC_CHECK_bNotOverWriteCRLF,		HIDC_CHECK_bNotOverWriteCRLF,			//�㏑�����[�h
	//	2007.02.11 genta �N���b�J�u��URL�����̃y�[�W�Ɉړ�
	IDC_CHECK_bSelectClickedURL,	HIDC_CHECK_bSelectClickedURL,	//�N���b�J�u��URL
	IDC_CHECK_CONVERTEOLPASTE,			HIDC_CHECK_CONVERTEOLPASTE,			//���s�R�[�h��ϊ����ē\��t����
//	IDC_STATIC,							-1,
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
INT_PTR CALLBACK CPropEdit::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropEdit::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* ���b�Z�[�W���� */
INT_PTR CPropEdit::DispatchEvent(
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
		/* �_�C�A���O�f�[�^�̐ݒ� Edit */
		SetData( hwndDlg );
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
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), TRUE );
				}
				else{
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), FALSE );
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
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_EDIT );
			return TRUE;
		case PSN_KILLACTIVE:
			DBPRINT_A( "Edit PSN_KILLACTIVE\n" );

			/* �_�C�A���O�f�[�^�̎擾 Edit */
			GetData( hwndDlg );
			return TRUE;

		case PSN_SETACTIVE: //@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			m_nPageNum = ID_PAGENUM_EDIT;
			return TRUE;
		}
		break;	/* WM_NOTIFY */

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_pszHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_pszHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CPropEdit::SetData( HWND hwndDlg )
{
	/* �h���b�O & �h���b�v�ҏW */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DRAGDROP, m_Common.m_sEdit.m_bUseOLE_DragDrop );
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), TRUE );
	}
	else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), FALSE );
	}

	/* DropSource */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, m_Common.m_sEdit.m_bUseOLE_DropSource );

	/* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY, m_Common.m_sEdit.m_bAddCRLFWhenCopy );

	/* �R�s�[������I������ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA, m_Common.m_sEdit.m_bCopyAndDisablSelection );

	/* �I���Ȃ��ŃR�s�[���\�ɂ��� */	// 2007.11.18 ryoji
	::CheckDlgButton( hwndDlg, IDC_CHECK_bEnableNoSelectCopy, m_Common.m_sEdit.m_bEnableNoSelectCopy );

	/* ���C�����[�h�\��t�����\�ɂ��� */	// 2007.10.08 ryoji
	::CheckDlgButton( hwndDlg, IDC_CHECK_bEnableLineModePaste, m_Common.m_sEdit.m_bEnableLineModePaste );

	/* ���s�͏㏑�����Ȃ� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bNotOverWriteCRLF, m_Common.m_sEdit.m_bNotOverWriteCRLF );

	//	URL���N���b�N���ꂽ��I�����邩 */	// 2007.02.11 genta ���̃y�[�W�ֈړ�
	::CheckDlgButton( hwndDlg, IDC_CHECK_bSelectClickedURL, m_Common.m_sEdit.m_bSelectClickedURL );

	/*	���s�R�[�h��ϊ����ē\��t���� */	// 2009.02.28 salarm
	::CheckDlgButton( hwndDlg, IDC_CHECK_CONVERTEOLPASTE, m_Common.m_sEdit.m_bConvertEOLPaste );
	return;
}





/* �_�C�A���O�f�[�^�̎擾 */
int CPropEdit::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_EDIT;

	/* �h���b�O & �h���b�v�ҏW */
	m_Common.m_sEdit.m_bUseOLE_DragDrop = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP );
	/* DropSource */
	m_Common.m_sEdit.m_bUseOLE_DropSource = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DROPSOURCE );

	/* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
	m_Common.m_sEdit.m_bAddCRLFWhenCopy = (0 != ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY ));

	/* �R�s�[������I������ */
	m_Common.m_sEdit.m_bCopyAndDisablSelection = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA );

	/* �I���Ȃ��ŃR�s�[���\�ɂ��� */	// 2007.11.18 ryoji
	m_Common.m_sEdit.m_bEnableNoSelectCopy = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bEnableNoSelectCopy );

	/* ���C�����[�h�\��t�����\�ɂ��� */	// 2007.10.08 ryoji
	m_Common.m_sEdit.m_bEnableLineModePaste = (0 != ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bEnableLineModePaste ));

	/* ���s�͏㏑�����Ȃ� */
	m_Common.m_sEdit.m_bNotOverWriteCRLF = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bNotOverWriteCRLF );

	/* URL���N���b�N���ꂽ��I�����邩 */	// 2007.02.11 genta ���̃y�[�W�ֈړ�
	m_Common.m_sEdit.m_bSelectClickedURL = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bSelectClickedURL );

	//	���s�R�[�h��ϊ����ē\��t���� */	// 2009.02.28 salarm
	m_Common.m_sEdit.m_bConvertEOLPaste = (0 != ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CONVERTEOLPASTE ));
	return TRUE;
}



