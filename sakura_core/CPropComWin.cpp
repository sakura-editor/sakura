//	$Id$
/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�E�B���h�E�v�y�[�W

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, genta, MIK, asa-o

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "CPropCommon.h"

//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//11200
	IDC_CHECK_DispFUNCKEYWND,		11210,	//�t�@���N�V�����L�[�\��
	IDC_CHECK_DispSTATUSBAR,		11211,	//�X�e�[�^�X�o�[�\��
	IDC_CHECK_DispTOOLBAR,			11212,	//�c�[���o�[�\��
	IDC_CHECK_bScrollBarHorz,		11213,	//�����X�N���[���o�[
	IDC_CHECK_bMenuIcon,			11214,	//�A�C�R���t�����j���[
	IDC_CHECK_WINSIZE,				11215,	//�E�C���h�E�T�C�Y�p��
	IDC_CHECK_SplitterWndVScroll,	11216,	//�����X�N���[���̓���	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_CHECK_SplitterWndHScroll,	11217,	//�����X�N���[���̓���	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_EDIT_nRulerBottomSpace,		11240,	//���[���[
	IDC_EDIT_nRulerHeight,			11241,	//���[���[
	IDC_RADIO_FUNCKEYWND_PLACE1,	11260,	//�t�@���N�V�����L�[�\���ʒu
	IDC_RADIO_FUNCKEYWND_PLACE2,	11261,	//�t�@���N�V�����L�[�\���ʒu
	IDC_SPIN_nRulerBottomSpace,		-1,
	IDC_SPIN_nRulerHeight,			-1,
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
BOOL CALLBACK CPropCommon::DlgProc_PROP_WIN(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_PROP_WIN, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta


/* ���b�Z�[�W���� */
BOOL CPropCommon::DispatchEvent_PROP_WIN(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,	// message
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
//	WORD		wNotifyCode;
//	WORD		wID;
//	HWND		hwndCtl;

// From Here Sept. 9, 2000 JEPRO
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
// To Here Sept. 9, 2000

	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
	int			nVal;	//Sept.21, 2000 JEPRO �X�s���v�f���������̂ŕ���������
//	LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p1 */
		SetData_PROP_WIN( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		/* ���[���[���� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_nRulerHeight ), EM_LIMITTEXT, (WPARAM)2, 0 );
		/* ���[���[�ƃe�L�X�g�̌��� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_nRulerBottomSpace ), EM_LIMITTEXT, (WPARAM)2, 0 );

		return TRUE;
//****	From Here Sept. 21, 2000 JEPRO �_�C�A���O�v�f�ɃX�s��������̂ňȉ���WM_NOTIFY���R�����g�A�E�g�ɂ����ɏC����u����
//	case WM_NOTIFY:
//		idCtrl = (int)wParam;
//		pNMHDR = (NMHDR*)lParam;
//		pMNUD  = (NM_UPDOWN*)lParam;
////		switch( idCtrl ){
////		default:
//			switch( pNMHDR->code ){
//			case PSN_HELP:
//				OnHelp( hwndDlg, IDD_PROP_WIN );
//				return TRUE;
//			case PSN_KILLACTIVE:
////				MYTRACE( "p1 PSN_KILLACTIVE\n" );
//				/* �_�C�A���O�f�[�^�̎擾 p1 */
//				GetData_PROP_WIN( hwndDlg );
//				return TRUE;
//			}
////			break;	//	Sept. 9, 2000 JEPRO ���̍s�͉���break�ƃ_�u���Ă��ď璷�Ȃ̂ō폜���Ă悢�͂�
////		}
//		break;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_WIN );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( "p1 PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 p1 */
				GetData_PROP_WIN( hwndDlg );
				return TRUE;
			}
			break;
		case IDC_SPIN_nRulerHeight:
			/* ���[���|�̍��� */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 13 ){
				nVal = 13;
			}
			if( nVal > 32 ){
				nVal = 32;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_nRulerBottomSpace:
			/* ���[���[�ƃe�L�X�g�̌��� */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > 32 ){
				nVal = 32;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, nVal, FALSE );
			return TRUE;
		}
//****	To Here Sept. 21, 2000
//	From Here Sept. 9, 2000 JEPRO
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			//	�t�@���N�V�����L�[��\�����鎞�������̈ʒu�w���Enable�ɐݒ�
			case IDC_CHECK_DispFUNCKEYWND:
				EnableWinPropInput( hwndDlg );
				break;
			}
			break;
		}
		break;
//	To Here Sept. 9, 2000

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
void CPropCommon::SetData_PROP_WIN( HWND hwndDlg )
{
//	BOOL	bRet;

	/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispTOOLBAR, m_Common.m_bDispTOOLBAR );

	/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispFUNCKEYWND, m_Common.m_bDispFUNCKEYWND );

	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
	if( 0 == m_Common.m_nFUNCKEYWND_Place ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, TRUE );
	}

	/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispSTATUSBAR, m_Common.m_bDispSTATUSBAR );

	/* �E�B���h�E�T�C�Y�p�� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_WINSIZE, m_Common.m_bSaveWindowSize );

	/* ���[���[���� */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, m_Common.m_nRulerHeight, FALSE );
	/* ���[���[�ƃe�L�X�g�̌��� */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, m_Common.m_nRulerBottomSpace, FALSE );

	/* ���[���[�̃^�C�v */
	if( 0 == m_Common.m_nRulerType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_0, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_1, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_1, TRUE );
	}

	/* �����X�N���[���o�[ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bScrollBarHorz, m_Common.m_bScrollBarHorz );

	/* �A�C�R���t�����j���[ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bMenuIcon, m_Common.m_bMenuIcon );

	//	2001/06/20 Start by asa-o:	�X�N���[���̓���
	::CheckDlgButton( hwndDlg, IDC_CHECK_SplitterWndVScroll, m_Common.m_bSplitterWndVScroll );
	::CheckDlgButton( hwndDlg, IDC_CHECK_SplitterWndHScroll, m_Common.m_bSplitterWndHScroll );
	//	2001/06/20 End

	//	Fronm Here Sept. 9, 2000 JEPRO
	//	�t�@���N�V�����L�[��\�����鎞�������̈ʒu�w���Enable�ɐݒ�
	EnableWinPropInput( hwndDlg );
	//	To Here Sept. 9, 2000

	return;
}





/* �_�C�A���O�f�[�^�̎擾 */
int CPropCommon::GetData_PROP_WIN( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_WIN;

	/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
	m_Common.m_bDispTOOLBAR = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTOOLBAR );

	/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
	m_Common.m_bDispFUNCKEYWND = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispFUNCKEYWND );

	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1 ) ){
		m_Common.m_nFUNCKEYWND_Place = 0;
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2) ){
		m_Common.m_nFUNCKEYWND_Place = 1;
	}

	/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */
	m_Common.m_bDispSTATUSBAR = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispSTATUSBAR );

	/* �E�B���h�E�T�C�Y�p�� */
	m_Common.m_bSaveWindowSize = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_WINSIZE );

	/* ���[���[�̃^�C�v */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_nRulerType_0 ) ){
		m_Common.m_nRulerType = 0;
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_nRulerType_1 ) ){
		m_Common.m_nRulerType = 1;
	}

	/* ���[���[���� */
	m_Common.m_nRulerHeight = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, NULL, FALSE );
	if( m_Common.m_nRulerHeight < 13 ){
		m_Common.m_nRulerHeight = 13;
	}
	if( m_Common.m_nRulerHeight > 32 ){
		m_Common.m_nRulerHeight = 32;
	}
	/* ���[���[�ƃe�L�X�g�̌��� */
	m_Common.m_nRulerBottomSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, NULL, FALSE );
	if( m_Common.m_nRulerBottomSpace < 0 ){
		m_Common.m_nRulerBottomSpace = 0;
	}
	if( m_Common.m_nRulerBottomSpace > 32 ){
		m_Common.m_nRulerBottomSpace = 32;
	}

	/* �����X�N���[���o�[ */
	m_Common.m_bScrollBarHorz = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bScrollBarHorz );

	/* �A�C�R���t�����j���[ */
	m_Common.m_bMenuIcon = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bMenuIcon );

	//	2001/06/20 Start by asa-o:	�X�N���[���̓���
	m_Common.m_bSplitterWndVScroll = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SplitterWndVScroll );
	m_Common.m_bSplitterWndHScroll = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SplitterWndHScroll );
	//	2001/06/20 End




	return TRUE;
}





//	From Here Sept. 9, 2000 JEPRO
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void CPropCommon::EnableWinPropInput( HWND hwndDlg )
{
	//	�t�@�N�V�����L�[��\�����邩�ǂ���
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispFUNCKEYWND ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_GROUP_FUNCKEYWND_POSITION ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2 ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_GROUP_FUNCKEYWND_POSITION ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2 ), FALSE );
	}
}
//	To Here Sept. 9, 2000


/*[EOF]*/
