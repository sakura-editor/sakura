//	$Id$
/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�����v�y�[�W

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CPropCommon.h"
#include "etc_uty.h"


//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//10400
	IDC_EDIT_DFORM,						10440,	//���t����
	IDC_EDIT_TFORM,						10441,	//��������
	IDC_EDIT_DFORM_EX,					10442,	//���t�����i�\����j
	IDC_EDIT_TFORM_EX,					10443,	//���������i�\����j
	IDC_EDIT_MIDASHIKIGOU,				10444,	//���o���L��
	IDC_EDIT_INYOUKIGOU,				10445,	//���p��
	IDC_RADIO_DFORM_0,					10460,	//���t�����i�W���j
	IDC_RADIO_DFORM_1,					10461,	//���t�����i�J�X�^���j
	IDC_RADIO_TFORM_0,					10462,	//���������i�W���j
	IDC_RADIO_TFORM_1,					10463,	//���������i�J�X�^���j
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
BOOL CALLBACK CPropCommon::DlgProc_PROP_FORMAT(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_p9, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

void CPropCommon::ChangeDateExample( HWND hwndDlg )
{
	/* �_�C�A���O�f�[�^�̎擾 p9 */
	GetData_p9( hwndDlg );

	/* ���t���t�H�[�}�b�g */
	char szText[1024];
	::MyGetDateFormat( szText, sizeof( szText ) - 1, m_Common.m_nDateFormatType, m_Common.m_szDateFormat );
	::SetDlgItemText( hwndDlg, IDC_EDIT_DFORM_EX, szText );
	return;
}
void CPropCommon::ChangeTimeExample( HWND hwndDlg )
{
	/* �_�C�A���O�f�[�^�̎擾 p9 */
	GetData_p9( hwndDlg );

	/* �������t�H�[�}�b�g */
	char szText[1024];
	::MyGetTimeFormat( szText, sizeof( szText ) - 1, m_Common.m_nTimeFormatType, m_Common.m_szTimeFormat );
	::SetDlgItemText( hwndDlg, IDC_EDIT_TFORM_EX, szText );
	return;
}


/* p9 ���b�Z�[�W���� */
BOOL CPropCommon::DispatchEvent_p9(
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

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p9 */
		SetData_p9( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		ChangeDateExample( hwndDlg );
		ChangeTimeExample( hwndDlg );


		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
//		/* �O���w���v�P */
//		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHELP1 ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );
//		/* �O��HTML�w���v */
//		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHTMLHELP ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );


		/* ���o���L�� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_MIDASHIKIGOU ), EM_LIMITTEXT, (WPARAM)(sizeof(m_Common.m_szMidashiKigou) - 1 ), 0 );

		/* ���p�� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_INYOUKIGOU ), EM_LIMITTEXT, (WPARAM)(sizeof(m_Common.m_szInyouKigou) - 1 ), 0 );

		/* ���t���� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_DFORM ), EM_LIMITTEXT, (WPARAM)(sizeof(m_Common.m_szDateFormat) - 1 ), 0 );

		/* �������� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_TFORM ), EM_LIMITTEXT, (WPARAM)(sizeof(m_Common.m_szTimeFormat) - 1 ), 0 );



		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		case EN_CHANGE:
			if( IDC_EDIT_DFORM == wID ){
				ChangeDateExample( hwndDlg );
				return 0;
			}
			if( IDC_EDIT_TFORM == wID  ){
				ChangeTimeExample( hwndDlg );
				return 0;
			}
			break;

		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_RADIO_DFORM_0:
			case IDC_RADIO_DFORM_1:
				ChangeDateExample( hwndDlg );
			//	From Here Sept. 10, 2000 JEPRO
			//	���t���� 0=�W�� 1=�J�X�^��
			//	���t�������J�X�^���ɂ���Ƃ����������w�蕶�����͂�Enable�ɐݒ�
				EnableFormatPropInput( hwndDlg );
			//	To Here Sept. 10, 2000
				return 0;
			case IDC_RADIO_TFORM_0:
			case IDC_RADIO_TFORM_1:
				ChangeTimeExample( hwndDlg );
			//	From Here Sept. 10, 2000 JEPRO
			//	�������� 0=�W�� 1=�J�X�^��
			//	�����������J�X�^���ɂ���Ƃ����������w�蕶�����͂�Enable�ɐݒ�
				EnableFormatPropInput( hwndDlg );
			//	To Here Sept. 10, 2000
				return 0;





//			case IDC_BUTTON_OPENHELP1:	/* �O���w���v�P�́u�Q��...�v�{�^�� */
//				{
//					CDlgOpenFile	cDlgOpenFile;
//					char*			pszMRU = NULL;;
//					char*			pszOPENFOLDER = NULL;;
//					char			szPath[_MAX_PATH + 1];
//					strcpy( szPath, m_Common.m_szExtHelp1 );
//					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
//					cDlgOpenFile.Create(
//						m_hInstance,
//						hwndDlg,
//						"*.hlp",
//						m_Common.m_szExtHelp1,
//						(const char **)&pszMRU,
//						(const char **)&pszOPENFOLDER
//					);
//					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
//						strcpy( m_Common.m_szExtHelp1, szPath );
//						::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1 );
//					}
//				}
//				return TRUE;
//			case IDC_BUTTON_OPENEXTHTMLHELP:	/* �O��HTML�w���v�́u�Q��...�v�{�^�� */
//				{
//					CDlgOpenFile	cDlgOpenFile;
//					char*			pszMRU = NULL;;
//					char*			pszOPENFOLDER = NULL;;
//					char			szPath[_MAX_PATH + 1];
//					strcpy( szPath, m_Common.m_szExtHtmlHelp );
//					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
//					cDlgOpenFile.Create(
//						m_hInstance,
//						hwndDlg,
//						"*.chm;*.col",
//						m_Common.m_szExtHtmlHelp,
//						(const char **)&pszMRU,
//						(const char **)&pszOPENFOLDER
//					);
//					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
//						strcpy( m_Common.m_szExtHtmlHelp, szPath );
//						::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp );
//					}
//				}
//				return TRUE;
//
			}
		}
		break;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		case ???????:
//			return 0L;
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_FORMAT );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( "p9 PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 p9 */
				GetData_p9( hwndDlg );
				return TRUE;
			}
			break;
//		}

//		MYTRACE( "pNMHDR->hwndFrom=%xh\n", pNMHDR->hwndFrom );
//		MYTRACE( "pNMHDR->idFrom  =%xh\n", pNMHDR->idFrom );
//		MYTRACE( "pNMHDR->code    =%xh\n", pNMHDR->code );
//		MYTRACE( "pMNUD->iPos    =%d\n", pMNUD->iPos );
//		MYTRACE( "pMNUD->iDelta  =%d\n", pMNUD->iDelta );
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





/* �_�C�A���O�f�[�^�̐ݒ� p9 */
void CPropCommon::SetData_p9( HWND hwndDlg )
{

	/* ���o���L�� */
	::SetDlgItemText( hwndDlg, IDC_EDIT_MIDASHIKIGOU, m_Common.m_szMidashiKigou );

//	/* �O���w���v�P */
//	::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1 );
//
//	/* �O��HTML�w���v */
//	::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp );

	/* ���p�� */
	::SetDlgItemText( hwndDlg, IDC_EDIT_INYOUKIGOU, m_Common.m_szInyouKigou );


	//���t�����̃^�C�v
	if( 0 == m_Common.m_nDateFormatType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_DFORM_0, BST_CHECKED );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_DFORM_1, BST_CHECKED );
	}
	//���t����
	::SetDlgItemText( hwndDlg, IDC_EDIT_DFORM, m_Common.m_szDateFormat );

	//���������̃^�C�v
	if( 0 == m_Common.m_nTimeFormatType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_TFORM_0, BST_CHECKED );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_TFORM_1, BST_CHECKED );
	}
	//��������
	::SetDlgItemText( hwndDlg, IDC_EDIT_TFORM, m_Common.m_szTimeFormat );

	//	From Here Sept. 10, 2000 JEPRO
	//	���t/�������� 0=�W�� 1=�J�X�^��
	//	���t/�����������J�X�^���ɂ���Ƃ����������w�蕶�����͂�Enable�ɐݒ�
	EnableFormatPropInput( hwndDlg );
	//	To Here Sept. 10, 2000

	return;
}




/* �_�C�A���O�f�[�^�̎擾 p9 */
int CPropCommon::GetData_p9( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_FORMAT;

	/* ���o���L�� */
	::GetDlgItemText( hwndDlg, IDC_EDIT_MIDASHIKIGOU, m_Common.m_szMidashiKigou, sizeof(m_Common.m_szMidashiKigou) );

//	/* �O���w���v�P */
//	::GetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1, MAX_PATH - 1 );
//
//	/* �O��HTML�w���v */
//	::GetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp, MAX_PATH - 1 );

	/* ���p�� */
	::GetDlgItemText( hwndDlg, IDC_EDIT_INYOUKIGOU, m_Common.m_szInyouKigou, sizeof(m_Common.m_szInyouKigou) );


	//���t�����̃^�C�v
	if( BST_CHECKED == ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_DFORM_0 ) ){
		m_Common.m_nDateFormatType = 0;
	}else{
		m_Common.m_nDateFormatType = 1;
	}
	//���t����
	::GetDlgItemText( hwndDlg, IDC_EDIT_DFORM, m_Common.m_szDateFormat, sizeof( m_Common.m_szDateFormat ) - 1 );

	//���������̃^�C�v
	if( BST_CHECKED == ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_TFORM_0 ) ){
		m_Common.m_nTimeFormatType = 0;
	}else{
		m_Common.m_nTimeFormatType = 1;
	}

	//��������
	::GetDlgItemText( hwndDlg, IDC_EDIT_TFORM, m_Common.m_szTimeFormat, sizeof( m_Common.m_szTimeFormat ) - 1 );












	return TRUE;
}





//	From Here Sept. 10, 2000 JEPRO
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void CPropCommon::EnableFormatPropInput( HWND hwndDlg )
{
	//	���t�������J�X�^���ɂ��邩�ǂ���
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_DFORM_1 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_DFORM ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_DFORM ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_DFORM ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_DFORM ), FALSE );
	}

	//	�����������J�X�^���ɂ��邩�ǂ���
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_TFORM_1 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_TFORM ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TFORM ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_TFORM ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TFORM ), FALSE );
	}
}
//	To Here Sept. 10, 2000


/*[EOF]*/
