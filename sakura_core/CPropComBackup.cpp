//	$Id$
/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�o�b�N�A�b�v�v�y�[�W

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CPropCommon.h"

#include "etc_uty.h"

//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//10000
	IDC_BUTTON_BACKUP_FOLDER_REF,	HIDC_BUTTON_BACKUP_FOLDER_REF,	//�o�b�N�A�b�v�t�H���_�Q��
	IDC_CHECK_BACKUP,				HIDC_CHECK_BACKUP,				//�o�b�N�A�b�v�̍쐬
	IDC_CHECK_BACKUP_YEAR,			HIDC_CHECK_BACKUP_YEAR,			//�o�b�N�A�b�v�t�@�C�����i����N�j
	IDC_CHECK_BACKUP_MONTH,			HIDC_CHECK_BACKUP_MONTH,		//�o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_DAY,			HIDC_CHECK_BACKUP_DAY,			//�o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_HOUR,			HIDC_CHECK_BACKUP_HOUR,			//�o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_MIN,			HIDC_CHECK_BACKUP_MIN,			//�o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_SEC,			HIDC_CHECK_BACKUP_SEC,			//�o�b�N�A�b�v�t�@�C�����i�b�j
	IDC_CHECK_BACKUPDIALOG,			HIDC_CHECK_BACKUPDIALOG,		//�쐬�O�Ɋm�F
	IDC_CHECK_BACKUPFOLDER,			HIDC_CHECK_BACKUPFOLDER,		//�w��t�H���_�ɍ쐬
	IDC_CHECK_BACKUP_DUSTBOX,		HIDC_CHECK_BACKUP_DUSTBOX,		//�o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add MIK
	IDC_EDIT_BACKUPFOLDER,			HIDC_EDIT_BACKUPFOLDER,			//�ۑ��t�H���_��
	IDC_EDIT_BACKUP_3,				HIDC_EDIT_BACKUP_3,				//���㐔
	IDC_RADIO_BACKUP_TYPE1,			HIDC_RADIO_BACKUP_TYPE1,		//�o�b�N�A�b�v�̎�ށi�g���q�j
//	IDC_RADIO_BACKUP_TYPE2,			HIDC_RADIO_BACKUP_TYPE2NEWHID,		//�o�b�N�A�b�v�̎�ށi���t�E�����j // 2002.11.09 Moca HID��.._TYPE3�Ƌt������	// Jun.  5, 2004 genta �p�~
	IDC_RADIO_BACKUP_TYPE3,			HIDC_RADIO_BACKUP_TYPE3NEWHID,		//�o�b�N�A�b�v�̎�ށi�A�ԁj// 2002.11.09 Moca HID��.._TYPE2�Ƌt������
	IDC_RADIO_BACKUP_DATETYPE1,		HIDC_RADIO_BACKUP_DATETYPE1,	//�t����������̎�ށi�쐬�����j	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_RADIO_BACKUP_DATETYPE2,		HIDC_RADIO_BACKUP_DATETYPE2,	//�t����������̎�ށi�X�V�����j	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_SPIN_BACKUP_GENS,			HIDC_EDIT_BACKUP_3,				//�ۑ����鐢�㐔�̃X�s��
//	IDC_STATIC,						-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//10000
	IDC_BUTTON_BACKUP_FOLDER_REF,	10000,	//�o�b�N�A�b�v�t�H���_�Q��
	IDC_CHECK_BACKUP,				10010,	//�o�b�N�A�b�v�̍쐬
	IDC_CHECK_BACKUP_YEAR,			10011,	//�o�b�N�A�b�v�t�@�C�����i����N�j
	IDC_CHECK_BACKUP_MONTH,			10012,	//�o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_DAY,			10013,	//�o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_HOUR,			10014,	//�o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_MIN,			10015,	//�o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_SEC,			10016,	//�o�b�N�A�b�v�t�@�C�����i�b�j
	IDC_CHECK_BACKUPDIALOG,			10017,	//�쐬�O�Ɋm�F
	IDC_CHECK_BACKUPFOLDER,			10018,	//�w��t�H���_�ɍ쐬
	IDC_CHECK_BACKUP_DUSTBOX,		10019,	//�o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add MIK
	IDC_EDIT_BACKUPFOLDER,			10040,	//�ۑ��t�H���_��
	IDC_EDIT_BACKUP_3,				10041,	//���㐔
	IDC_RADIO_BACKUP_TYPE1,			10060,	//�o�b�N�A�b�v�̎�ށi�g���q�j
//	IDC_RADIO_BACKUP_TYPE2,			10062,	//�o�b�N�A�b�v�̎�ށi���t�E����	// Jun.  5, 2004 genta �p�~
	IDC_RADIO_BACKUP_TYPE3,			10061,	//�o�b�N�A�b�v�̎�ށi�A�ԁj
	IDC_RADIO_BACKUP_DATETYPE1,		10063,	//�t����������̎�ށi�쐬�����j	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_RADIO_BACKUP_DATETYPE2,		10064,	//�t����������̎�ށi�X�V�����j	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_SPIN_BACKUP_GENS,			-1,
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
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_BACKUP(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_PROP_BACKUP, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta


/* ���b�Z�[�W���� */
INT_PTR CPropCommon::DispatchEvent_PROP_BACKUP( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
	int			nVal;	//Sept.21, 2000 JEPRO �X�s���v�f���������̂ŕ���������
	char		szFolder[_MAX_PATH];
//	int			nDummy;
//	int			nCharChars;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p1 */
		SetData_PROP_BACKUP( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		//	Oct. 5, 2002 genta �o�b�N�A�b�v�t�H���_���̓��̓T�C�Y���w��
		//	Oct. 8, 2002 genta �Ō�ɕt�������\�̗̈���c�����߃o�b�t�@�T�C�Y-1�������͂����Ȃ�
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUPFOLDER ),  EM_LIMITTEXT, (WPARAM)( sizeof( m_Common.m_szBackUpFolder ) - 1 - 1 ), 0 );
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
//				OnHelp( hwndDlg, IDD_PROP_BACKUP );
//				return TRUE;
//			case PSN_KILLACTIVE:
//				/* �_�C�A���O�f�[�^�̎擾 p1 */
//				GetData_PROP_BACKUP( hwndDlg );
//				return TRUE;
//			}
//			break;
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
				OnHelp( hwndDlg, IDD_PROP_BACKUP );
				return TRUE;
			case PSN_KILLACTIVE:
				/* �_�C�A���O�f�[�^�̎擾 p1 */
				GetData_PROP_BACKUP( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_BACKUP;	//Oct. 25, 2000 JEPRO ZENPAN1��ZENPAN �ɕύX(�Q�Ƃ��Ă���̂�CPropCommon.cpp�݂̂�1�ӏ�)
				return TRUE;
			}
			break;

		case IDC_SPIN_BACKUP_GENS:
			/* �o�b�N�A�b�v�t�@�C���̐��㐔 */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_BACKUP_3, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 99 ){
				nVal = 99;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_BACKUP_3, nVal, FALSE );
			return TRUE;
		}
//****	To Here Sept. 21, 2000 JEPRO �_�C�A���O�v�f�ɃX�s��������̂ňȉ���WM_NOTIFY���R�����g�A�E�g�ɂ����ɏC����u����
		break;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_RADIO_BACKUP_TYPE1:
				//	Aug. 16, 2000 genta
				//	�o�b�N�A�b�v�����ǉ�
			case IDC_RADIO_BACKUP_TYPE3:
			case IDC_CHECK_BACKUP:
			case IDC_CHECK_BACKUPFOLDER:
				//	Aug. 21, 2000 genta
			case IDC_CHECK_AUTOSAVE:
			//	Jun.  5, 2004 genta IDC_RADIO_BACKUP_TYPE2��p�~���āC
			//	IDC_RADIO_BACKUP_DATETYPE1, IDC_RADIO_BACKUP_DATETYPE2�𓯗�Ɏ����Ă���
			case IDC_RADIO_BACKUP_DATETYPE1:
			case IDC_RADIO_BACKUP_DATETYPE2:
				EnableBackupInput(hwndDlg);
				return TRUE;
			case IDC_BUTTON_BACKUP_FOLDER_REF:	/* �t�H���_�Q�� */
//				strcpy( szFolder, m_Common.m_szBackUpFolder );
				/* �o�b�N�A�b�v���쐬����t�H���_ */
				::GetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, szFolder, sizeof( szFolder ));

				if( SelectDir( hwndDlg, "�o�b�N�A�b�v���쐬����t�H���_��I��ł�������", (const char *)szFolder, (char *)szFolder ) ){
					strcpy( m_Common.m_szBackUpFolder, szFolder );
					::SetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_szBackUpFolder );
				}
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */

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


/*! �_�C�A���O�f�[�^�̐ݒ�
	@date 2004.06.05 genta ���̊g���q���c���ݒ��ǉ��D
		�����w��Ń`�F�b�N�{�b�N�X���󗓂Ŏc��Ɛݒ肳��Ȃ���������邽�߁C
		IDC_RADIO_BACKUP_TYPE2
		��p�~���ă��C�A�E�g�ύX
*/
void CPropCommon::SetData_PROP_BACKUP( HWND hwndDlg )
{
//	BOOL	bRet;

//	BOOL				m_bGrepExitConfirm;	/* Grep���[�h�ŕۑ��m�F���邩 */


	/* �o�b�N�A�b�v�̍쐬 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP, m_Common.m_bBackUp );
	/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUPDIALOG, m_Common.m_bBackUpDialog );
	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUPFOLDER, m_Common.m_bBackUpFolder );

	/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
	//	Jun.  5, 2004 genta ���̊g���q���c���ݒ�(5,6)��ǉ��D
	switch( m_Common.GetBackupType() ){
	case 2:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1, 1 );	// �t��������t�̃^�C�v(������)
		break;
	case 3:
	case 6:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_TYPE3, 1 );
		break;
	case 4:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2, 1 );	// �t��������t�̃^�C�v(�O��̕ۑ�����)
		break;
	case 5:
	case 1:
	default:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_TYPE1, 1 );
		break;
	}
	
	//	Jun.  5, 2004 genta ���̊g���q���c���ݒ�(5,6)��ǉ��D
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_RETAINEXT,
		( m_Common.GetBackupType() == 5 || m_Common.GetBackupType() == 6 ) ? 1 : 0
	 );

	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_YEAR, m_Common.GetBackupOpt(BKUP_YEAR) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_MONTH, m_Common.GetBackupOpt(BKUP_MONTH) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_DAY, m_Common.GetBackupOpt(BKUP_DAY) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_HOUR, m_Common.GetBackupOpt(BKUP_HOUR) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_MIN, m_Common.GetBackupOpt(BKUP_MIN) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_SEC, m_Common.GetBackupOpt(BKUP_SEC) );

	/* �o�b�N�A�b�v���쐬����t�H���_ */
	::SetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_szBackUpFolder );

	/* �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�� */	//@@@ 2001.12.11 add MIK
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_DUSTBOX, m_Common.m_bBackUpDustBox );	//@@@ 2001.12.11 add MIK

	//	From Here Aug. 16, 2000 genta
	int nN = m_Common.GetBackupCount();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;

	::SetDlgItemInt( hwndDlg, IDC_EDIT_BACKUP_3, nN, FALSE );	//	Oct. 29, 2001 genta
	//	To Here Aug. 16, 2000 genta

	EnableBackupInput(hwndDlg);
	return;
}








/* �_�C�A���O�f�[�^�̎擾 */
int CPropCommon::GetData_PROP_BACKUP( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_BACKUP;

	/* �o�b�N�A�b�v�̍쐬 */
	m_Common.m_bBackUp = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP );
	/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
	m_Common.m_bBackUpDialog = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPDIALOG );
	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
	m_Common.m_bBackUpFolder = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPFOLDER );


	/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ) ){
		//	Jun.  5, 2005 genta �g���q���c���p�^�[����ǉ�
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_RETAINEXT )){
			m_Common.SetBackupType(5);
		}
		else {
			m_Common.SetBackupType(1);
		}
	}
//	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE2 ) ){
		// 2001/06/05 Start by asa-o: ���t�̃^�C�v
		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1 ) ){
			m_Common.SetBackupType(2);	// ������
		}
		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2 ) ){
			m_Common.SetBackupType(4);	// �O��̕ۑ�����
		}
		// 2001/06/05 End
//	}

	//	Aug. 16, 2000 genta
	//	3 = *.b??
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ) ){
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_RETAINEXT )){
			m_Common.SetBackupType(6);
		}
		else {
			m_Common.SetBackupType(3);
		}
	}

	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
	m_Common.SetBackupOpt(BKUP_YEAR, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_YEAR ) == BST_CHECKED);
	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
	m_Common.SetBackupOpt(BKUP_MONTH, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_MONTH ) == BST_CHECKED);
	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
	m_Common.SetBackupOpt(BKUP_DAY, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_DAY ) == BST_CHECKED);
	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
	m_Common.SetBackupOpt(BKUP_HOUR, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_HOUR ) == BST_CHECKED);
	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
	m_Common.SetBackupOpt(BKUP_MIN, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_MIN ) == BST_CHECKED);
	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
	m_Common.SetBackupOpt(BKUP_SEC, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_SEC ) == BST_CHECKED);

	/* �o�b�N�A�b�v���쐬����t�H���_ */
	//	Oct. 5, 2002 genta �T�C�Y��sizeof()�Ŏw��
	//	Oct. 8, 2002 genta ����\���ǉ������̂ŁC1�����]�T������K�v������D
	::GetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_szBackUpFolder, sizeof( m_Common.m_szBackUpFolder ) - 1);

	/* �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�� */	//@@@ 2001.12.11 add MIK
	m_Common.m_bBackUpDustBox = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_DUSTBOX );	//@@@ 2001.12.11 add MIK

//	int nDummy;
//	int nCharChars;
//	nDummy = strlen( m_Common.m_szBackUpFolder );
//	if( 0 < nDummy ){
//		/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
//		nCharChars = &m_Common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( m_Common.m_szBackUpFolder, nDummy, &m_Common.m_szBackUpFolder[nDummy] );
//		if( 1 == nCharChars && m_Common.m_szBackUpFolder[nDummy - 1] == '\\' ){
//		}else{
//			strcat( m_Common.m_szBackUpFolder, "\\" );
//		}
//	}

	//	From Here Aug. 16, 2000 genta
	//	���㐔�̎擾
//	char szNumBuf[6];
	int	 nN;
//	char *pDigit;
	nN = ::GetDlgItemInt( hwndDlg, IDC_EDIT_BACKUP_3, NULL, FALSE );	//	Oct. 29, 2001 genta

//	for( nN = 0, pDigit = szNumBuf; *pDigit != '\0'; pDigit++ ){
//		if( '0' <= *pDigit && *pDigit <= '9' ){
//			nN = nN * 10 + *pDigit - '0';
//		}
//		else
//			break;
//	}
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;
	m_Common.SetBackupCount( nN );
	//	To Here Aug. 16, 2000 genta

	return TRUE;
}

//	From Here Aug. 16, 2000 genta
/*!	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
	�K�؂ɐݒ肷��

	@date 2004.06.05 genta ���̊g���q���c���ݒ��ǉ��D
		�����w��Ń`�F�b�N�{�b�N�X���󗓂Ŏc��Ɛݒ肳��Ȃ���������邽�߁C
		IDC_RADIO_BACKUP_TYPE2
		��p�~���ă��C�A�E�g�ύX
*/
void CPropCommon::EnableBackupInput(HWND hwndDlg)
{
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2 ), TRUE );
		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1 ) ||
			::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2 ) ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_YEAR ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MONTH ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_DAY ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_HOUR ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MIN ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_SEC ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_YEAR ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MONTH ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_DAY ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_HOUR ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MIN ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_SEC ), FALSE );
		}
		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ) ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_3 ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUP_3 ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_BACKUP_GENS ), TRUE );	// added Oct. 6, JEPRO Enable�ɂȂ�悤�ɕύX
		}
		else {
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_3 ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUP_3 ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_BACKUP_GENS ), FALSE );	// added Oct. 6, JEPRO Disable�ɂȂ�悤�ɕύX
		}
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUPDIALOG ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUPFOLDER ), TRUE );

		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPFOLDER ) ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_4 ), TRUE );	// added Sept. 6, JEPRO �t�H���_�w�肵���Ƃ�����Enable�ɂȂ�悤�ɕύX
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUPFOLDER ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKUP_FOLDER_REF ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_4 ), FALSE );	// added Sept. 6, JEPRO �t�H���_�w�肵���Ƃ�����Enable�ɂȂ�悤�ɕύX
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUPFOLDER ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKUP_FOLDER_REF ), FALSE );
		}
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_DUSTBOX ), TRUE );	//@@@ 2001.12.11 add MIK

	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ) ||
		::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE3 )){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_RETAINEXT ), TRUE );
	}
	else {
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_RETAINEXT ), FALSE );
	}

	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_YEAR ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MONTH ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_DAY ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_HOUR ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_MIN ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_SEC ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_3 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUP_3 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUPDIALOG ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUPFOLDER ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_BACKUP_4 ), FALSE );	// added Sept. 6, JEPRO �t�H���_�w�肵���Ƃ�����Enable�ɂȂ�悤�ɕύX
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUPFOLDER ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKUP_FOLDER_REF ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_DUSTBOX ), FALSE );	//@@@ 2001.12.11 add MIK
		//	Jun.  5, 2005 genta �ǉ�
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BACKUP_RETAINEXT ), FALSE );
	}
}
//	To Here Aug. 16, 2000 genta


/*[EOF]*/
