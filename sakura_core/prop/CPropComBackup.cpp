/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�o�b�N�A�b�v�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, MIK, asa-o, genta, jepro
	Copyright (C) 2002, MIK, YAZAKI, genta, Moca
	Copyright (C) 2003, KEITA
	Copyright (C) 2004, genta
	Copyright (C) 2005, genta, aroka
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
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
	IDC_CHECK_BACKUP_FOLDER_RM,		HIDC_CHECK_BACKUP_FOLDER_RM,	//�w��t�H���_�ɍ쐬(�����[�o�u�����f�B�A�̂�)
	IDC_CHECK_BACKUP_DUSTBOX,		HIDC_CHECK_BACKUP_DUSTBOX,		//�o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add MIK
	IDC_EDIT_BACKUPFOLDER,			HIDC_EDIT_BACKUPFOLDER,			//�ۑ��t�H���_��
	IDC_EDIT_BACKUP_3,				HIDC_EDIT_BACKUP_3,				//���㐔
	IDC_RADIO_BACKUP_TYPE1,			HIDC_RADIO_BACKUP_TYPE1,		//�o�b�N�A�b�v�̎�ށi�g���q�j
//	IDC_RADIO_BACKUP_TYPE2,			HIDC_RADIO_BACKUP_TYPE2NEWHID,		//�o�b�N�A�b�v�̎�ށi���t�E�����j // 2002.11.09 Moca HID��.._TYPE3�Ƌt������	// Jun.  5, 2004 genta �p�~
	IDC_RADIO_BACKUP_TYPE3,			HIDC_RADIO_BACKUP_TYPE3NEWHID,		//�o�b�N�A�b�v�̎�ށi�A�ԁj// 2002.11.09 Moca HID��.._TYPE2�Ƌt������
	IDC_RADIO_BACKUP_DATETYPE1,		HIDC_RADIO_BACKUP_DATETYPE1,	//�t����������̎�ށi�쐬�����j	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_RADIO_BACKUP_DATETYPE2,		HIDC_RADIO_BACKUP_DATETYPE2,	//�t����������̎�ށi�X�V�����j	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_SPIN_BACKUP_GENS,			HIDC_EDIT_BACKUP_3,				//�ۑ����鐢�㐔�̃X�s��
	IDC_CHECK_BACKUP_RETAINEXT,		HIDC_CHECK_BACKUP_RETAINEXT,	//���̊g���q��ۑ�	// 2006.08.06 ryoji
	IDC_CHECK_BACKUP_ADVANCED,		HIDC_CHECK_BACKUP_ADVANCED,		//�ڍאݒ�	// 2006.08.06 ryoji
	IDC_EDIT_BACKUPFILE,			HIDC_EDIT_BACKUPFILE,			//�ڍאݒ�̃G�f�B�b�g�{�b�N�X	// 2006.08.06 ryoji
	IDC_RADIO_BACKUP_DATETYPE1A,	HIDC_RADIO_BACKUP_DATETYPE1A,	//�t����������̎�ށi�쐬�����j���ڍאݒ�ON�p	// 2009.02.20 ryoji
	IDC_RADIO_BACKUP_DATETYPE2A,	HIDC_RADIO_BACKUP_DATETYPE2A,	//�t����������̎�ށi�X�V�����j���ڍאݒ�ON�p	// 2009.02.20 ryoji
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
INT_PTR CALLBACK CPropBackup::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta


/* ���b�Z�[�W���� */
INT_PTR CPropBackup::DispatchEvent( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
	int			nVal;	//Sept.21, 2000 JEPRO �X�s���v�f���������̂ŕ���������
//	int			nDummy;
//	int			nCharChars;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� Backup */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		//	Oct. 5, 2002 genta �o�b�N�A�b�v�t�H���_���̓��̓T�C�Y���w��
		//	Oct. 8, 2002 genta �Ō�ɕt�������\�̗̈���c�����߃o�b�t�@�T�C�Y-1�������͂����Ȃ�
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUPFOLDER ), _countof2(m_Common.m_sBackup.m_szBackUpFolder) - 1 - 1 );
		// 20051107 aroka
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BACKUPFILE ), _countof2(m_Common.m_sBackup.m_szBackUpPathAdvanced) - 1 - 1 );
		return TRUE;

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
				/* �_�C�A���O�f�[�^�̎擾 Backup */
				GetData( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_BACKUP;
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
			// 20051107 aroka
			case IDC_CHECK_BACKUP_ADVANCED:
				GetData( hwndDlg );
				UpdateBackupFile( hwndDlg );
				EnableBackupInput(hwndDlg);
				return TRUE;
			case IDC_BUTTON_BACKUP_FOLDER_REF:	/* �t�H���_�Q�� */
				{
					/* �o�b�N�A�b�v���쐬����t�H���_ */
					TCHAR		szFolder[_MAX_PATH];
					::DlgItem_GetText( hwndDlg, IDC_EDIT_BACKUPFOLDER, szFolder, _countof( szFolder ));

					if( SelectDir( hwndDlg, _T("�o�b�N�A�b�v���쐬����t�H���_��I��ł�������"), szFolder, szFolder ) ){
						_tcscpy( m_Common.m_sBackup.m_szBackUpFolder, szFolder );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_sBackup.m_szBackUpFolder );
					}
					UpdateBackupFile( hwndDlg );
				}
				return TRUE;
			default: // 20051107 aroka Default�� �ǉ�
				GetData( hwndDlg );
				UpdateBackupFile( hwndDlg );
			}
			break;	/* BN_CLICKED */
		case EN_CHANGE: // 20051107 aroka �t�H���_���ύX���ꂽ�烊�A���^�C���ɃG�f�B�b�g�{�b�N�X�����X�V
			switch( wID ){
			case IDC_EDIT_BACKUPFOLDER:
				// 2009.02.21 ryoji ����\���ǉ������̂ŁC1�����]�T���݂�K�v������D
				::DlgItem_GetText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_sBackup.m_szBackUpFolder, _countof2(m_Common.m_sBackup.m_szBackUpFolder) - 1 );
				UpdateBackupFile( hwndDlg );
				break;
			}
			break;	/* EN_CHANGE */
		}
		break;	/* WM_COMMAND */

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
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


/*! �_�C�A���O�f�[�^�̐ݒ�
	@date 2004.06.05 genta ���̊g���q���c���ݒ��ǉ��D
		�����w��Ń`�F�b�N�{�b�N�X���󗓂Ŏc��Ɛݒ肳��Ȃ���������邽�߁C
		IDC_RADIO_BACKUP_TYPE2
		��p�~���ă��C�A�E�g�ύX
*/
void CPropBackup::SetData( HWND hwndDlg )
{
//	BOOL	bRet;

//	BOOL				m_bGrepExitConfirm;	/* Grep���[�h�ŕۑ��m�F���邩 */


	/* �o�b�N�A�b�v�̍쐬 */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP, m_Common.m_sBackup.m_bBackUp );
	/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUPDIALOG, m_Common.m_sBackup.m_bBackUpDialog );
//	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */ //	20051107 aroka �u�o�b�N�A�b�v�̍쐬�v�ɘA��������
//	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUPFOLDER, .m_sBackup.m_bBackUpFolder );

	/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
	//	Jun.  5, 2004 genta ���̊g���q���c���ݒ�(5,6)��ǉ��D
	switch( m_Common.m_sBackup.GetBackupType() ){
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
		( m_Common.m_sBackup.GetBackupType() == 5 || m_Common.m_sBackup.GetBackupType() == 6 ) ? 1 : 0
	 );

	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_YEAR, m_Common.m_sBackup.GetBackupOpt(BKUP_YEAR) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_MONTH, m_Common.m_sBackup.GetBackupOpt(BKUP_MONTH) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_DAY, m_Common.m_sBackup.GetBackupOpt(BKUP_DAY) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_HOUR, m_Common.m_sBackup.GetBackupOpt(BKUP_HOUR) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_MIN, m_Common.m_sBackup.GetBackupOpt(BKUP_MIN) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_SEC, m_Common.m_sBackup.GetBackupOpt(BKUP_SEC) );

	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */ // 20051107 aroka �ړ��F�A���Ώۂɂ���B
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUPFOLDER, m_Common.m_sBackup.m_bBackUpFolder );
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BACKUP_FOLDER_RM, m_Common.m_sBackup.m_bBackUpFolderRM );	// 2010/5/27 Uchi

	/* �o�b�N�A�b�v���쐬����t�H���_ */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_sBackup.m_szBackUpFolder );

	/* �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�� */	//@@@ 2001.12.11 add MIK
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_DUSTBOX, m_Common.m_sBackup.m_bBackUpDustBox?BST_CHECKED:BST_UNCHECKED );	//@@@ 2001.12.11 add MIK

	/* �o�b�N�A�b�v��t�H���_���ڍאݒ肷�� */ // 20051107 aroka
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_ADVANCED, m_Common.m_sBackup.m_bBackUpPathAdvanced?BST_CHECKED:BST_UNCHECKED );

	/* �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ� */ // 20051107 aroka
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BACKUPFILE, m_Common.m_sBackup.m_szBackUpPathAdvanced );

	/* �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ� */ // 20051128 aroka
	switch( m_Common.m_sBackup.GetBackupTypeAdv() ){
	case 2:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A, 1 );	// �t��������t�̃^�C�v(������)
		break;
	case 4:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2A, 1 );	// �t��������t�̃^�C�v(�O��̕ۑ�����)
		break;
	default:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A, 1 );
		break;
	}

	//	From Here Aug. 16, 2000 genta
	int nN = m_Common.m_sBackup.GetBackupCount();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;

	::SetDlgItemInt( hwndDlg, IDC_EDIT_BACKUP_3, nN, FALSE );	//	Oct. 29, 2001 genta
	//	To Here Aug. 16, 2000 genta

	UpdateBackupFile( hwndDlg );

	EnableBackupInput(hwndDlg);
	return;
}








/* �_�C�A���O�f�[�^�̎擾 */
int CPropBackup::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_BACKUP;

	/* �o�b�N�A�b�v�̍쐬 */
	m_Common.m_sBackup.m_bBackUp = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_BACKUP );
	/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
	m_Common.m_sBackup.m_bBackUpDialog = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_BACKUPDIALOG );
//	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */ // 20051107 aroka �u�o�b�N�A�b�v�̍쐬�v�ɘA��������
//	m_Common.m_sBackup.m_bBackUpFolder = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPFOLDER );


	/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ) ){
		//	Jun.  5, 2005 genta �g���q���c���p�^�[����ǉ�
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_RETAINEXT )){
			m_Common.m_sBackup.SetBackupType(5);
		}
		else {
			m_Common.m_sBackup.SetBackupType(1);
		}
	}
//	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE2 ) ){
		// 2001/06/05 Start by asa-o: ���t�̃^�C�v
		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1 ) ){
			m_Common.m_sBackup.SetBackupType(2);	// ������
		}
		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2 ) ){
			m_Common.m_sBackup.SetBackupType(4);	// �O��̕ۑ�����
		}
		// 2001/06/05 End
//	}

	//	Aug. 16, 2000 genta
	//	3 = *.b??
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ) ){
		if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_RETAINEXT )){
			m_Common.m_sBackup.SetBackupType(6);
		}
		else {
			m_Common.m_sBackup.SetBackupType(3);
		}
	}

	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
	m_Common.m_sBackup.SetBackupOpt(BKUP_YEAR, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_YEAR ) == BST_CHECKED);
	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
	m_Common.m_sBackup.SetBackupOpt(BKUP_MONTH, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_MONTH ) == BST_CHECKED);
	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
	m_Common.m_sBackup.SetBackupOpt(BKUP_DAY, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_DAY ) == BST_CHECKED);
	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
	m_Common.m_sBackup.SetBackupOpt(BKUP_HOUR, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_HOUR ) == BST_CHECKED);
	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
	m_Common.m_sBackup.SetBackupOpt(BKUP_MIN, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_MIN ) == BST_CHECKED);
	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
	m_Common.m_sBackup.SetBackupOpt(BKUP_SEC, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_SEC ) == BST_CHECKED);

	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */ // 20051107 aroka �ړ�
	m_Common.m_sBackup.m_bBackUpFolder = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_BACKUPFOLDER );
	m_Common.m_sBackup.m_bBackUpFolderRM = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_BACKUP_FOLDER_RM );	// 2010/5/27 Uchi

	/* �o�b�N�A�b�v���쐬����t�H���_ */
	//	Oct. 5, 2002 genta �T�C�Y��sizeof()�Ŏw��
	//	Oct. 8, 2002 genta ����\���ǉ������̂ŁC1�����]�T������K�v������D
	::DlgItem_GetText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_sBackup.m_szBackUpFolder, _countof2(m_Common.m_sBackup.m_szBackUpFolder) - 1);

	/* �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�� */	//@@@ 2001.12.11 add MIK
	m_Common.m_sBackup.m_bBackUpDustBox = (BST_CHECKED==::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_DUSTBOX ));	//@@@ 2001.12.11 add MIK

	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬����ڍאݒ� */ // 20051107 aroka
	m_Common.m_sBackup.m_bBackUpPathAdvanced = (BST_CHECKED==::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_ADVANCED ));
	/* �o�b�N�A�b�v���쐬����t�H���_ */ // 20051107 aroka
	::DlgItem_GetText( hwndDlg, IDC_EDIT_BACKUPFILE, m_Common.m_sBackup.m_szBackUpPathAdvanced, _countof2( m_Common.m_sBackup.m_szBackUpPathAdvanced ) - 1);

	// 20051128 aroka �ڍאݒ�̓��t�̃^�C�v
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A ) ){
		m_Common.m_sBackup.SetBackupTypeAdv(2);	// ������
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2A ) ){
		m_Common.m_sBackup.SetBackupTypeAdv(4);	// �O��̕ۑ�����
	}

	//	From Here Aug. 16, 2000 genta
	//	���㐔�̎擾
	int	 nN;
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
	m_Common.m_sBackup.SetBackupCount( nN );
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
	@date 2005.11.07 aroka ���C�A�E�g�ɍ��킹�ď��������ւ��A�C���f���g�𐮗�
	@date 2005.11.21 aroka �ڍאݒ胂�[�h�̐����ǉ�
	@date 2009.02.20 ryoji IDC_LABEL_BACKUP_HELP�ɂ��ʃR���g���[���B����p�~�Aif�������ShowEnable�t���O����ɒu�������Ċȑf�����ĉ��L���C���B
	                       �EVista Aero���Əڍאݒ�ON�ɂ��Ă��ڍאݒ�OFF���ڂ���ʂ�������Ȃ�
	                       �E�ڍאݒ�OFF���ڂ���\���ł͂Ȃ������̂ŉB��Ă��Ă�Tooltip�w���v���\�������
	                       �E�ڍאݒ�ON�Ȃ̂Ƀo�b�N�A�b�v�쐬OFF���Əڍאݒ�OFF���ڂ̂ق����\�������
*/
static inline void ShowEnable(HWND hWnd, BOOL bShow, BOOL bEnable)
{
	::ShowWindow( hWnd, bShow? SW_SHOW: SW_HIDE );
	::EnableWindow( hWnd, bEnable && bShow );		// bShow=false,bEnable=true�̏ꍇ�V���[�g�J�b�g�L�[���ςȓ���������̂ŏC��	2010/5/27 Uchi
}

void CPropBackup::EnableBackupInput(HWND hwndDlg)
{
	#define SHOWENABLE(id, show, enable) ShowEnable( ::GetDlgItem( hwndDlg, id ), show, enable )

	BOOL bBackup = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP );
	BOOL bAdvanced = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP_ADVANCED );
	BOOL bType1 = ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE1 );
	//BOOL bType2 = ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE2 );
	BOOL bType3 = ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE3 );
	BOOL bDate1 = ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE1 );
	BOOL bDate2 = ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_DATETYPE2 );
	BOOL bFolder = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPFOLDER );

	SHOWENABLE( IDC_CHECK_BACKUP_ADVANCED,	TRUE, bBackup );	// 20050628 aroka
	SHOWENABLE( IDC_RADIO_BACKUP_TYPE1,		!bAdvanced, bBackup );
	SHOWENABLE( IDC_RADIO_BACKUP_TYPE3,		!bAdvanced, bBackup );
	SHOWENABLE( IDC_RADIO_BACKUP_DATETYPE1,	!bAdvanced, bBackup );
	SHOWENABLE( IDC_RADIO_BACKUP_DATETYPE2,	!bAdvanced, bBackup );
	SHOWENABLE( IDC_LABEL_BACKUP_3,			!bAdvanced, bBackup && bType3);
	SHOWENABLE( IDC_EDIT_BACKUP_3,			!bAdvanced, bBackup && bType3);
	SHOWENABLE( IDC_SPIN_BACKUP_GENS,		!bAdvanced, bBackup && bType3 );	//	20051107 aroka �ǉ�
	SHOWENABLE( IDC_CHECK_BACKUP_RETAINEXT,	!bAdvanced, bBackup && (bType1 || bType3) );	//	Jun.  5, 2005 genta �ǉ�
	SHOWENABLE( IDC_CHECK_BACKUP_YEAR,		!bAdvanced, bBackup && (bDate1 || bDate2) );
	SHOWENABLE( IDC_CHECK_BACKUP_MONTH,		!bAdvanced, bBackup && (bDate1 || bDate2) );
	SHOWENABLE( IDC_CHECK_BACKUP_DAY,		!bAdvanced, bBackup && (bDate1 || bDate2) );
	SHOWENABLE( IDC_CHECK_BACKUP_HOUR,		!bAdvanced, bBackup && (bDate1 || bDate2) );
	SHOWENABLE( IDC_CHECK_BACKUP_MIN,		!bAdvanced, bBackup && (bDate1 || bDate2) );
	SHOWENABLE( IDC_CHECK_BACKUP_SEC,		!bAdvanced, bBackup && (bDate1 || bDate2) );

	// �ڍאݒ�
	SHOWENABLE( IDC_EDIT_BACKUPFILE,		TRUE, bBackup && bAdvanced );
//	SHOWENABLE( IDC_LABEL_BACKUP_HELP,		bAdvanced, bBackup );	// �s���̂܂ܕ��u�i���R���g���[���B���̕����͔p�~�j 2009.02.20 ryoji
	SHOWENABLE( IDC_LABEL_BACKUP_HELP2,		bAdvanced, bBackup );
	SHOWENABLE( IDC_RADIO_BACKUP_DATETYPE1A,	bAdvanced, bBackup );
	SHOWENABLE( IDC_RADIO_BACKUP_DATETYPE2A,	bAdvanced, bBackup );

	SHOWENABLE( IDC_CHECK_BACKUPFOLDER,		TRUE, bBackup );
	SHOWENABLE( IDC_LABEL_BACKUP_4,			TRUE, bBackup && bFolder );	// added Sept. 6, JEPRO �t�H���_�w�肵���Ƃ�����Enable�ɂȂ�悤�ɕύX
	SHOWENABLE( IDC_CHECK_BACKUP_FOLDER_RM,	TRUE, bBackup && bFolder );	// 2010/5/27 Uchi
	SHOWENABLE( IDC_EDIT_BACKUPFOLDER,		TRUE, bBackup && bFolder );
	SHOWENABLE( IDC_BUTTON_BACKUP_FOLDER_REF,	TRUE, bBackup && bFolder );
	SHOWENABLE( IDC_CHECK_BACKUP_DUSTBOX,	TRUE, bBackup );	//@@@ 2001.12.11 add MIK

	// �쐬�O�Ɋm�F
	SHOWENABLE( IDC_CHECK_BACKUPDIALOG,		TRUE, bBackup );

	#undef SHOWENABLE
}
//	To Here Aug. 16, 2000 genta


/*!	�o�b�N�A�b�v�t�@�C���̏ڍאݒ�G�f�B�b�g�{�b�N�X��K�؂ɍX�V����

	@date 2005.11.07 aroka �V�K�ǉ�

	@note �ڍאݒ�؂�ւ����̃f�t�H���g���I�v�V�����ɍ��킹�邽�߁A
		m_szBackUpPathAdvanced ���X�V����
*/
void CPropBackup::UpdateBackupFile(HWND hwndDlg)	//	�o�b�N�A�b�v�t�@�C���̏ڍאݒ�
{
	wchar_t temp[MAX_PATH];
	/* �o�b�N�A�b�v���쐬����t�@�C�� */ // 20051107 aroka
	if( !m_Common.m_sBackup.m_bBackUp ){
		wcscpy( temp, LTEXT("") );
	}
	else{
		if( m_Common.m_sBackup.m_bBackUpFolder ){
			wcscpy( temp, LTEXT("") );
		}
		else if( m_Common.m_sBackup.m_bBackUpDustBox  ){
			auto_sprintf( temp, LTEXT("%ls\\"), LTEXT("(�S�~��)") );
		}
		else{
			auto_sprintf( temp, LTEXT(".\\") );
		}

		switch( m_Common.m_sBackup.GetBackupType() ){
		case 1: // .bak
			wcscat( temp, LTEXT("$0.bak") );
			break;
		case 5: // .*.bak
			wcscat( temp, LTEXT("$0.*.bak") );
			break;
		case 3: // .b??
			wcscat( temp, LTEXT("$0.b??") );
			break;
		case 6: // .*.b??
			wcscat( temp, LTEXT("$0.*.b??") );
			break;
		case 2:	//	���t�C����
		case 4:	//	���t�C����
			wcscat( temp, LTEXT("$0_") );

			if( m_Common.m_sBackup.GetBackupOpt(BKUP_YEAR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
				wcscat( temp, LTEXT("%Y") );
			}
			if( m_Common.m_sBackup.GetBackupOpt(BKUP_MONTH) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
				wcscat( temp, LTEXT("%m") );
			}
			if( m_Common.m_sBackup.GetBackupOpt(BKUP_DAY) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
				wcscat( temp, LTEXT("%d") );
			}
			if( m_Common.m_sBackup.GetBackupOpt(BKUP_HOUR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
				wcscat( temp, LTEXT("%H") );
			}
			if( m_Common.m_sBackup.GetBackupOpt(BKUP_MIN) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
				wcscat( temp, LTEXT("%M") );
			}
			if( m_Common.m_sBackup.GetBackupOpt(BKUP_SEC) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
				wcscat( temp, LTEXT("%S") );
			}

			wcscat( temp, LTEXT(".*") );
			break;
		default:
			break;
		}
	}
	if( !m_Common.m_sBackup.m_bBackUpPathAdvanced ){	// �ڍאݒ胂�[�h�łȂ��Ƃ����������X�V����
		auto_sprintf( m_Common.m_sBackup.m_szBackUpPathAdvanced, _T("%ls"), temp );
		::DlgItem_SetText( hwndDlg, IDC_EDIT_BACKUPFILE, m_Common.m_sBackup.m_szBackUpPathAdvanced );
	}
	return;
}

