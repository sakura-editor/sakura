/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�����v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, genta
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "util/shell.h"
#include "env/DLLSHAREDATA.h" // CFormatManager.h���O�ɕK�v
#include "env/CFormatManager.h"
#include "sakura_rc.h"
#include "sakura.hh"


//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10400
	IDC_EDIT_DFORM,						HIDC_EDIT_DFORM,		//���t����
	IDC_EDIT_TFORM,						HIDC_EDIT_TFORM,		//��������
	IDC_EDIT_DFORM_EX,					HIDC_EDIT_DFORM_EX,		//���t�����i�\����j
	IDC_EDIT_TFORM_EX,					HIDC_EDIT_TFORM_EX,		//���������i�\����j
	IDC_EDIT_MIDASHIKIGOU,				HIDC_EDIT_MIDASHIKIGOU,	//���o���L��
	IDC_EDIT_INYOUKIGOU,				HIDC_EDIT_INYOUKIGOU,	//���p��
	IDC_RADIO_DFORM_0,					HIDC_RADIO_DFORM_0,		//���t�����i�W���j
	IDC_RADIO_DFORM_1,					HIDC_RADIO_DFORM_1,		//���t�����i�J�X�^���j
	IDC_RADIO_TFORM_0,					HIDC_RADIO_TFORM_0,		//���������i�W���j
	IDC_RADIO_TFORM_1,					HIDC_RADIO_TFORM_1,		//���������i�J�X�^���j
//	IDC_STATIC,							-1,
	0, 0
};
//@@@ 2001.02.04 End

//@@@ 2002.01.12 add start
static const char *p_date_form[] = {
	"yyyy'�N'M'��'d'��'",
	"yyyy'�N'M'��'d'��('dddd')'",
	"yyyy'�N'MM'��'dd'��'",
	"yyyy'�N'M'��'d'��' dddd",
	"yyyy'�N'MM'��'dd'��' dddd",
	"yyyy/MM/dd",
	"yy/MM/dd",
	"yy/M/d",
	"yyyy/M/d",
	"yy/MM/dd' ('ddd')'",
	"yy/M/d' ('ddd')'",
	"yyyy/MM/dd' ('ddd')'",
	"yyyy/M/d' ('ddd')'",
	"yyyy/M/d' ('ddd')'",
	NULL
};

static const char *p_time_form[] = {
	"hh:mm:ss",
	"tthh'��'mm'��'ss'�b'",
	"H:mm:ss",
	"HH:mm:ss",
	"tt h:mm:ss",
	"tt hh:mm:ss",
	NULL
};
//@@@ 2002.01.12 add end

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropFormat::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

void CPropFormat::ChangeDateExample( HWND hwndDlg )
{
	/* �_�C�A���O�f�[�^�̎擾 Format */
	GetData( hwndDlg );

	/* ���t���t�H�[�}�b�g */
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetDateFormat( systime, szText, _countof( szText ) - 1, m_Common.m_sFormat.m_nDateFormatType, m_Common.m_sFormat.m_szDateFormat );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_DFORM_EX, szText );
	return;
}
void CPropFormat::ChangeTimeExample( HWND hwndDlg )
{
	/* �_�C�A���O�f�[�^�̎擾 Format */
	GetData( hwndDlg );

	/* �������t�H�[�}�b�g */
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetTimeFormat( systime, szText, _countof( szText ) - 1, m_Common.m_sFormat.m_nTimeFormatType, m_Common.m_sFormat.m_szTimeFormat );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TFORM_EX, szText );
	return;
}


/* Format ���b�Z�[�W���� */
INT_PTR CPropFormat::DispatchEvent(
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
		/* �_�C�A���O�f�[�^�̐ݒ� Format */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		ChangeDateExample( hwndDlg );
		ChangeTimeExample( hwndDlg );

		/* ���o���L�� */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_MIDASHIKIGOU ), _countof(m_Common.m_sFormat.m_szMidashiKigou) - 1 );

		/* ���p�� */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_INYOUKIGOU ), _countof(m_Common.m_sFormat.m_szInyouKigou) - 1 );

		/* ���t���� */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_DFORM ), _countof(m_Common.m_sFormat.m_szDateFormat) - 1 );

		/* �������� */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_TFORM ), _countof(m_Common.m_sFormat.m_szTimeFormat) - 1 );



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




			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
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
//				MYTRACE_A( "Format PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 Format */
				GetData( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_FORMAT;
				return TRUE;
			}
//			break;	/* default */
//		}

//		MYTRACE_A( "pNMHDR->hwndFrom=%xh\n", pNMHDR->hwndFrom );
//		MYTRACE_A( "pNMHDR->idFrom  =%xh\n", pNMHDR->idFrom );
//		MYTRACE_A( "pNMHDR->code    =%xh\n", pNMHDR->code );
//		MYTRACE_A( "pMNUD->iPos    =%d\n", pMNUD->iPos );
//		MYTRACE_A( "pMNUD->iDelta  =%d\n", pMNUD->iDelta );
		break;	/* WM_NOTIFY */

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}





/* �_�C�A���O�f�[�^�̐ݒ� Format */
void CPropFormat::SetData( HWND hwndDlg )
{

	/* ���o���L�� */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MIDASHIKIGOU, m_Common.m_sFormat.m_szMidashiKigou );

	/* ���p�� */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_INYOUKIGOU, m_Common.m_sFormat.m_szInyouKigou );


	//���t�����̃^�C�v
	if( 0 == m_Common.m_sFormat.m_nDateFormatType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_DFORM_0, BST_CHECKED );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_DFORM_1, BST_CHECKED );
	}
	//���t����
	::DlgItem_SetText( hwndDlg, IDC_EDIT_DFORM, m_Common.m_sFormat.m_szDateFormat );

	//���������̃^�C�v
	if( 0 == m_Common.m_sFormat.m_nTimeFormatType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_TFORM_0, BST_CHECKED );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_TFORM_1, BST_CHECKED );
	}
	//��������
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TFORM, m_Common.m_sFormat.m_szTimeFormat );

	//	From Here Sept. 10, 2000 JEPRO
	//	���t/�������� 0=�W�� 1=�J�X�^��
	//	���t/�����������J�X�^���ɂ���Ƃ����������w�蕶�����͂�Enable�ɐݒ�
	EnableFormatPropInput( hwndDlg );
	//	To Here Sept. 10, 2000

	return;
}




/* �_�C�A���O�f�[�^�̎擾 Format */
int CPropFormat::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_FORMAT;

	/* ���o���L�� */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_MIDASHIKIGOU, m_Common.m_sFormat.m_szMidashiKigou, _countof(m_Common.m_sFormat.m_szMidashiKigou) );

//	/* �O���w���v�P */
//	::DlgItem_GetText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_sFormat.m_szExtHelp1, MAX_PATH - 1 );
//
//	/* �O��HTML�w���v */
//	::DlgItem_GetText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_sFormat.m_szExtHtmlHelp, MAX_PATH - 1 );

	/* ���p�� */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_INYOUKIGOU, m_Common.m_sFormat.m_szInyouKigou, _countof(m_Common.m_sFormat.m_szInyouKigou) );


	//���t�����̃^�C�v
	if( BST_CHECKED == ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_DFORM_0 ) ){
		m_Common.m_sFormat.m_nDateFormatType = 0;
	}else{
		m_Common.m_sFormat.m_nDateFormatType = 1;
	}
	//���t����
	::DlgItem_GetText( hwndDlg, IDC_EDIT_DFORM, m_Common.m_sFormat.m_szDateFormat, _countof( m_Common.m_sFormat.m_szDateFormat ));

	//���������̃^�C�v
	if( BST_CHECKED == ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_TFORM_0 ) ){
		m_Common.m_sFormat.m_nTimeFormatType = 0;
	}else{
		m_Common.m_sFormat.m_nTimeFormatType = 1;
	}

	//��������
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TFORM, m_Common.m_sFormat.m_szTimeFormat, _countof( m_Common.m_sFormat.m_szTimeFormat ));












	return TRUE;
}





//	From Here Sept. 10, 2000 JEPRO
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void CPropFormat::EnableFormatPropInput( HWND hwndDlg )
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



