//	$Id$
//	Copyright (C) 1998-2000, Norio Nakatani
/*! @file
	���ʐݒ�|�u�t�@�C���v�v���p�e�B�V�[�g

	@author Norio Nakatani
*/

#include "CPropCommon.h"


//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//01310
	IDC_CHECK_EXCVLUSIVE_NO,				10310,	//�t�@�C���̔r������i�r�����䂵�Ȃ��j
	IDC_CHECK_bCheckFileTimeStamp,			10311,	//�X�V�̊Ď�
	IDC_CHECK_EXCVLUSIVE_WRITE,				10312,	//�t�@�C���̔r������i�㏑���֎~�j
	IDC_CHECK_EXCVLUSIVE_READWRITE,			10313,	//�t�@�C�����r������i�ǂݏ����֎~�j
	IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE,	10314,	//���ύX�ł��㏑��
	IDC_CHECK_AUTOSAVE,						10315,	//�����I�ɕۑ�
	IDC_CHECK_bDropFileAndClose,			10316,	//���ĊJ��
	IDC_CHECK_RestoreCurPosition,			10317,	//�J�[�\���ʒu�̕���
	IDC_CHECK_AutoMIMEDecode,				10318,	//MIME�f�R�[�h
	IDC_EDIT_AUTOBACKUP_INTERVAL,			10340,	//�����ۑ��Ԋu
	IDC_EDIT_nDropFileNumMax,				10341,	//�t�@�C���h���b�v�ő吔
	IDC_SPIN_AUTOBACKUP_INTERVAL,			-1,
	IDC_SPIN_nDropFileNumMax,				-1,
//	IDC_STATIC,								-1,
	0, 0
};
//@@@ 2001.02.04 End


/*! �t�@�C���y�[�W ���b�Z�[�W���� */
BOOL CPropCommon::DispatchEvent_p2(
    HWND	hwndDlg,	//!< handle to dialog box
    UINT	uMsg,	//!< message
    WPARAM	wParam,	//!< first message parameter
    LPARAM	lParam 	//!< second message parameter
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
//@@@ 2001.03.21 Start by MIK
			/*NOTREACHED*/
			break;
		case IDC_SPIN_AUTOBACKUP_INTERVAL:
			/* �o�b�N�A�b�v�Ԋu */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 35791 ){
				nVal = 35791;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, nVal, FALSE );
			return TRUE;
			/*NOTREACHED*/
			break;
//@@@ 2001.03.21 End by MIK
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





/*! �t�@�C���y�[�W: �_�C�A���O�f�[�^�̐ݒ�
	���L����������f�[�^��ǂݏo���Ċe�R���g���[���ɒl��ݒ肷��B

	@par �o�b�N�A�b�v���㐔���Ó��Ȓl���ǂ����̃`�F�b�N���s���B�s�K�؂Ȓl�̎���
	�ł��߂��K�؂Ȓl��ݒ肷��B

	@param hwndDlg �v���p�e�B�y�[�W��Window Handle
*/
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

/*! �t�@�C���y�[�W �_�C�A���O�f�[�^�̎擾
	�_�C�A���O�{�b�N�X�ɐݒ肳�ꂽ�f�[�^�����L�������ɔ��f������
	
	@par �o�b�N�A�b�v���㐔���Ó��Ȓl���ǂ����̃`�F�b�N���s���B�s�K�؂Ȓl�̎���
	�ł��߂��K�؂Ȓl��ݒ肷��B

	@param hwndDlg �v���p�e�B�y�[�W��Window Handle
	@return ���TRUE
*/
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
	char szNumBuf[/*6*/ 7];	//@@@ 2001.03.21 by MIK
	int	 nN;
	char *pDigit;

	::GetDlgItemText( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, szNumBuf, /*5*/ 6 );	//@@@ 2001.03.21 by MIK

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
/*!	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
	�K�؂ɐݒ肷��
	
	@param hwndDlg �v���p�e�B�V�[�g��Window Handle
*/
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
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE2 ), TRUE );	//Sept. 6, 2000 JEPRO �����ۑ��ɂ����Ƃ�����Enable�ɂȂ�悤�ɕύX
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_AUTOBACKUP_INTERVAL ), TRUE );	//@@@ 2001.03.21 by MIK
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_AUTOSAVE2 ), FALSE );	//Sept. 6, 2000 JEPRO ����
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_AUTOBACKUP_INTERVAL ), FALSE );	//@@@ 2001.03.21 by MIK
	}
}
//	To Here Aug. 21, 2000 genta
