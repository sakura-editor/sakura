//	$Id$
//	Copyright (C) 1998-2000, Norio Nakatani

#include "CPropCommon.h"





/* ���b�Z�[�W���� */
BOOL CPropCommon::DispatchEvent_PROP_EDIT(
    HWND	hwndDlg,	// handle to dialog box
    UINT	uMsg,	// message
    WPARAM	wParam,	// first message parameter
    LPARAM	lParam 	// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
//    LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p1 */
		SetData_PROP_EDIT( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID         = LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl     = (HWND) lParam;	/* �R���g���[���̃n���h�� */
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
			}
			break;
//		}
		break;
	}
	return FALSE;
}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CPropCommon::SetData_PROP_EDIT( HWND hwndDlg )
{
//	BOOL	bRet;

	/* Drag && Drop�ҏW */
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
	m_nPageNum = ID_PAGENUM_EDIT;

	/* Drag && Drop�ҏW */
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
