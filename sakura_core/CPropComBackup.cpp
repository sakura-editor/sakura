//	$Id$
//	Copyright (C) 1998-2000, Norio Nakatani
#include "CPropCommon.h"

#include "etc_uty.h"

//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//10000
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
	IDC_EDIT_BACKUPFOLDER,			10040,	//�ۑ��t�H���_��
	IDC_EDIT_BACKUP_3,				10041,	//���㐔
	IDC_RADIO_BACKUP_TYPE1,			10060,	//�o�b�N�A�b�v�̎�ށi�g���q�j
	IDC_RADIO_BACKUP_TYPE2,			10061,	//�o�b�N�A�b�v�̎�ށi�A�ԁj
	IDC_RADIO_BACKUP_TYPE3,			10062,	//�o�b�N�A�b�v�̎�ށi���t�E�����j
	IDC_SPIN_BACKUP_GENS,			-1,
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End



/* ���b�Z�[�W���� */
BOOL CPropCommon::DispatchEvent_PROP_BACKUP( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
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
		

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID         = LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl     = (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_RADIO_BACKUP_TYPE1:
			case IDC_RADIO_BACKUP_TYPE2:
				//	Aug. 16, 2000 genta
				//	�o�b�N�A�b�v�����ǉ�
			case IDC_RADIO_BACKUP_TYPE3:
			case IDC_CHECK_BACKUP:
			case IDC_CHECK_BACKUPFOLDER:
				//	Aug. 21, 2000 genta
			case IDC_CHECK_AUTOSAVE:
				EnableBackupInput(hwndDlg);
				return TRUE;
			case IDC_BUTTON_BACKUP_FOLDER_REF:	/* �t�H���_�Q�� */
//				strcpy( szFolder, m_Common.m_szBackUpFolder );
				/* �o�b�N�A�b�v���쐬����t�H���_ */
				::GetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, szFolder, MAX_PATH - 1 );

				if( SelectDir( hwndDlg, "�o�b�N�A�b�v���쐬����t�H���_��I��ł�������", (const char *)szFolder, (char *)szFolder ) ){
					strcpy( m_Common.m_szBackUpFolder, szFolder );
					::SetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_szBackUpFolder );
				}
				return TRUE;
			}
		}
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
	switch( m_Common.GetBackupType()){
	case 2:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_TYPE2, 1 );
		break;
	case 3:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_TYPE3, 1 );
		break;
	case 1:
	default:
		::CheckDlgButton( hwndDlg, IDC_RADIO_BACKUP_TYPE1, 1 );
		break;
	}
	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_YEAR, m_Common.GetBackupOpt(BKUP_YEAR)  );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_MONTH, m_Common.GetBackupOpt(BKUP_MONTH) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_DAY, m_Common.GetBackupOpt(BKUP_DAY));
	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_HOUR, m_Common.GetBackupOpt(BKUP_HOUR) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_MIN, m_Common.GetBackupOpt(BKUP_MIN) );
	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
	::CheckDlgButton( hwndDlg, IDC_CHECK_BACKUP_SEC, m_Common.GetBackupOpt(BKUP_SEC) );

	/* �o�b�N�A�b�v���쐬����t�H���_ */
	::SetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_szBackUpFolder );

	//	From Here Aug. 16, 2000 genta
	int nN = m_Common.GetBackupCount();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;

	char buf[6];
	wsprintf( buf, "%d", nN);
	::SetDlgItemText( hwndDlg, IDC_EDIT_BACKUP_3, buf );
	//	To Here Aug. 16, 2000 genta

	EnableBackupInput(hwndDlg);
	return;
}








/* �_�C�A���O�f�[�^�̎擾 */
int CPropCommon::GetData_PROP_BACKUP( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_BACKUP;

	/* �o�b�N�A�b�v�̍쐬 */
	m_Common.m_bBackUp = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP );
	/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
	m_Common.m_bBackUpDialog = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPDIALOG );
	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
	m_Common.m_bBackUpFolder = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUPFOLDER );


	/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ) ){
		m_Common.SetBackupType(1);
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE2 ) ){
		m_Common.SetBackupType(2);
	}

	//	Aug. 16, 2000 genta
	//	3 = *.b??
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ) ){
		m_Common.SetBackupType(3);
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
	::GetDlgItemText( hwndDlg, IDC_EDIT_BACKUPFOLDER, m_Common.m_szBackUpFolder, MAX_PATH - 1 );

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
	char szNumBuf[6];
	int	 nN;
	char *pDigit;
	::GetDlgItemText( hwndDlg, IDC_EDIT_BACKUP_3, szNumBuf, 4 );

	for( nN = 0, pDigit = szNumBuf; *pDigit != '\0'; pDigit++ ){
		if( '0' <= *pDigit && *pDigit <= '9' ){
			nN = nN * 10 + *pDigit - '0';
		}
		else
			break;
	}
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;
	m_Common.SetBackupCount( nN );
	//	To Here Aug. 16, 2000 genta

	return TRUE;
}

//	From Here Aug. 16, 2000 genta
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void CPropCommon::EnableBackupInput(HWND hwndDlg)
{
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BACKUP ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE2 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE3 ), TRUE );
		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_BACKUP_TYPE2 ) ){
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

	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE1 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_BACKUP_TYPE2 ), FALSE );
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
	}
}
//	To Here Aug. 16, 2000 genta
