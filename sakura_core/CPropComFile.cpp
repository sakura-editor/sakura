//	$Id$
//	Copyright (C) 1998-2000, Norio Nakatani

#include "CPropCommon.h"


/* p2 ���b�Z�[�W���� */
BOOL CPropCommon::DispatchEvent_p2(
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
	int			nVal;	//Sept.21, 2000 JEPRO �X�s���v�f���������̂ŕ���������
//	char		szFolder[_MAX_PATH];

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p2 */
		SetData_p2( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

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
//				OnHelp( hwndDlg, IDD_PROP_FILE );
//				return TRUE;
//			case PSN_KILLACTIVE:
////				MYTRACE( "p2 PSN_KILLACTIVE\n" );
//				/* �_�C�A���O�f�[�^�̎擾 p2 */
//				GetData_p2( hwndDlg );
//				return TRUE;
//			}
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
				OnHelp( hwndDlg, IDD_PROP_FILE );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( "p2 PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 p2 */
				GetData_p2( hwndDlg );
				return TRUE;
			}
		break;
		case IDC_SPIN_nDropFileNumMax:
			/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nDropFileNumMax, NULL, FALSE );
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
			::SetDlgItemInt( hwndDlg, IDC_EDIT_nDropFileNumMax, nVal, FALSE );
			return TRUE;
		}
//****	To Here Sept. 21, 2000 JEPRO �_�C�A���O�v�f�ɃX�s��������̂�WM_NOTIFY���R�����g�A�E�g�ɂ����̉��ɏC����u����
		
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID         = LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl     = (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_EXCVLUSIVE_NO:
			case IDC_CHECK_EXCVLUSIVE_WRITE:
			case IDC_CHECK_EXCVLUSIVE_READWRITE:
			case IDC_CHECK_bDropFileAndClose:/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
			case IDC_CHECK_AUTOSAVE:
				EnableFilePropInput(hwndDlg);
				break;
			}
			break;
		}
		break;

	}
	return FALSE;
}





/* �_�C�A���O�f�[�^�̐ݒ� p2 */
void CPropCommon::SetData_p2( HWND hwndDlg )
{
	/*--- p2 ---*/
	/* �t�@�C���̔r�����䃂�[�h */
	switch( m_Common.m_nFileShareMode ){
	case OF_SHARE_DENY_WRITE:	/* �������݋֎~ */
		::CheckDlgButton( hwndDlg, IDC_CHECK_EXCVLUSIVE_WRITE, BST_CHECKED );
		break;
	case OF_SHARE_EXCLUSIVE:	/* �ǂݏ����֎~ */
		::CheckDlgButton( hwndDlg, IDC_CHECK_EXCVLUSIVE_READWRITE, BST_CHECKED );
		break;
	case 0:	/* �r���Ȃ� */
	default:	/* �r���Ȃ� */
		::CheckDlgButton( hwndDlg, IDC_CHECK_EXCVLUSIVE_NO, BST_CHECKED );
		break;
	}
	/* �X�V�̊Ď� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bCheckFileTimeStamp, m_Common.m_bCheckFileTimeStamp );

	/* ���ύX�ł��㏑�����邩 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE, m_Common.m_bEnableUnmodifiedOverwrite );

	/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bDropFileAndClose, m_Common.m_bDropFileAndClose );
	/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_nDropFileNumMax, m_Common.m_nDropFileNumMax, FALSE );

	//	From Here Aug. 21, 2000 genta
	//	�����ۑ��̗L���E����
	::CheckDlgButton( hwndDlg, IDC_CHECK_AUTOSAVE, m_Common.IsAutoBackupEnabled() );

	char buf[6];
	int nN;

	nN = m_Common.GetAutoBackupInterval();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 35791 ? 35791 : nN;

	wsprintf( buf, "%d", nN);
	::SetDlgItemText( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, buf );
	//	To Here Aug. 21, 2000 genta

	//	Oct. 27, 2000 genta	�J�[�\���ʒu�����t���O
	::CheckDlgButton( hwndDlg, IDC_CHECK_RestoreCurPosition, m_Common.GetRestoreCurPosition() );
	//	Nov. 12, 2000 genta	MIME Decode�t���O
	::CheckDlgButton( hwndDlg, IDC_CHECK_AutoMIMEDecode, m_Common.GetAutoMIMEdecode() );

	EnableFilePropInput(hwndDlg);
	return;
}





/* �_�C�A���O�f�[�^�̎擾 p2 */
int CPropCommon::GetData_p2( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_FILE;

	/* �t�@�C���̔r�����䃂�[�h */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_EXCVLUSIVE_NO ) ){	/* �r���Ȃ� */
		m_Common.m_nFileShareMode = 0;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_EXCVLUSIVE_WRITE ) ){	/* �������݋֎~ */
		m_Common.m_nFileShareMode = OF_SHARE_DENY_WRITE	;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_EXCVLUSIVE_READWRITE ) ){	/* �ǂݏ����֎~ */
		m_Common.m_nFileShareMode = OF_SHARE_EXCLUSIVE;
	}else{
		/* �r���Ȃ� */
		m_Common.m_nFileShareMode = 0;
	}
	/* �X�V�̊Ď� */
	m_Common.m_bCheckFileTimeStamp = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bCheckFileTimeStamp );

	/* ���ύX�ł��㏑�����邩 */
	m_Common.m_bEnableUnmodifiedOverwrite = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE );

	/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
	m_Common.m_bDropFileAndClose = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bDropFileAndClose );
	/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
	m_Common.m_nDropFileNumMax = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nDropFileNumMax, NULL, FALSE );
	if( 1 > m_Common.m_nDropFileNumMax ){
		m_Common.m_nDropFileNumMax = 1;
	}
	if( 99 < m_Common.m_nDropFileNumMax ){	//Sept. 21, 2000, JEPRO 16���傫���Ƃ���99�Ɛ�������Ă����̂��C��(16��99�ƕύX)
		m_Common.m_nDropFileNumMax = 99;
	}

	//	From Here Aug. 16, 2000 genta
	//	�����ۑ����s�����ǂ���
	m_Common.EnableAutoBackup( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_AUTOSAVE ) == TRUE );

	//	�����ۑ��Ԋu�̎擾
	char szNumBuf[6];
	int	 nN;
	char *pDigit;

	::GetDlgItemText( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, szNumBuf, 5 );

	for( nN = 0, pDigit = szNumBuf; *pDigit != '\0'; pDigit++ ){
		if( '0' <= *pDigit && *pDigit <= '9' ){
			nN = nN * 10 + *pDigit - '0';
		}
		else
			break;
	}
	nN = nN < 1  ?  1 : nN;
	nN = nN > 35791 ? 35791 : nN;
	m_Common.SetAutoBackupInterval( nN );

	//	To Here Aug. 16, 2000 genta

	//	Oct. 27, 2000 genta	�J�[�\���ʒu�����t���O
	m_Common.SetRestoreCurPosition( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_RestoreCurPosition ) == TRUE );
	//	Nov. 12, 2000 genta	MIME Decode�t���O
	m_Common.SetAutoMIMEdecode( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_AutoMIMEDecode ) == TRUE );

	return TRUE;
}

//	From Here Aug. 21, 2000 genta
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void CPropCommon::EnableFilePropInput(HWND hwndDlg)
{

	//	Drop���̓���
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bDropFileAndClose ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE3 ), FALSE );	// added Sept. 6, JEPRO �����ۑ��ɂ����Ƃ�����Enable�ɂȂ�悤�ɕύX
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE4 ), FALSE );	// added Sept. 6, JEPRO	����
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_nDropFileNumMax ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_nDropFileNumMax ), FALSE );// added Oct. 6, JEPRO �t�@�C���I�[�v�����u���ĊJ���v�ɂ����Ƃ���Disable�ɂȂ�悤�ɕύX
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE3 ), TRUE );	// added Sept. 6, JEPRO	����
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE4 ), TRUE );	// added Sept. 6, JEPRO	����
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_nDropFileNumMax ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_nDropFileNumMax ), TRUE );// added Oct. 6, JEPRO �t�@�C���I�[�v�����u�����t�@�C���h���b�v�v�ɂ����Ƃ�����Enable�ɂȂ�悤�ɕύX
	}

	//	�r�����邩�ǂ���
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_EXCVLUSIVE_NO ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_bCheckFileTimeStamp ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_bCheckFileTimeStamp ), FALSE );
	}

	//	�����ۑ�
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_AUTOSAVE ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE2 ), TRUE );	// added Sept. 6, JEPRO �����ۑ��ɂ����Ƃ�����Enable�ɂȂ�悤�ɕύX
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE2 ), FALSE );	// added Sept. 6, JEPRO	����
	}
}
//	To Here Aug. 21, 2000 genta
