/*! @file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�����v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK, jepro
	Copyright (C) 2002, YAZAKI, MIK, aroka, hor
	Copyright (C) 2004, genta, ryoji
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "CPropertyManager.h"
#include "debug/Debug.h" // 2002/2/10 aroka
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {	//01310
	IDC_COMBO_FILESHAREMODE,				HIDC_COMBO_FILESHAREMODE,				//�r������
	IDC_CHECK_bCheckFileTimeStamp,			HIDC_CHECK_bCheckFileTimeStamp,			//�X�V�̊Ď�
	IDC_CHECK_bUneditableIfUnwritable,		HIDC_CHECK_bUneditableIfUnwritable,		//�㏑���֎~���o���͕ҏW�֎~�ɂ���
	IDC_CHECK_AUTOSAVE,						HIDC_CHECK_AUTOSAVE,					//�����I�ɕۑ�
	IDC_CHECK_bDropFileAndClose,			HIDC_CHECK_bDropFileAndClose,			//���ĊJ��
	IDC_CHECK_RestoreCurPosition,			HIDC_CHECK_RestoreCurPosition,			//�J�[�\���ʒu�̕���
	IDC_CHECK_AutoMIMEDecode,				HIDC_CHECK_AutoMIMEDecode,				//MIME�f�R�[�h
	IDC_EDIT_AUTOBACKUP_INTERVAL,			HIDC_EDIT_AUTOBACKUP_INTERVAL,			//�����ۑ��Ԋu
	IDC_EDIT_nDropFileNumMax,				HIDC_EDIT_nDropFileNumMax,				//�t�@�C���h���b�v�ő吔
	IDC_SPIN_AUTOBACKUP_INTERVAL,			HIDC_EDIT_AUTOBACKUP_INTERVAL,
	IDC_SPIN_nDropFileNumMax,				HIDC_EDIT_nDropFileNumMax,
	IDC_CHECK_RestoreBookmarks,				HIDC_CHECK_RestoreBookmarks,			// 2002.01.16 hor �u�b�N�}�[�N�̕���
	IDC_CHECK_QueryIfCodeChange,			HIDC_CHECK_QueryIfCodeChange,			//�O��ƈقȂ镶���R�[�h�̂Ƃ��₢���킹���s��	// 2006.08.06 ryoji
	IDC_CHECK_AlertIfFileNotExist,			HIDC_CHECK_AlertIfFileNotExist,			//�J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������	// 2006.08.06 ryoji
	IDC_CHECK_ALERT_IF_LARGEFILE,			HIDC_CHECK_ALERT_IF_LARGEFILE,			//�J�����Ƃ����t�@�C�����傫���ꍇ�Ɍx������
	IDC_CHECK_NoFilterSaveNew,				HIDC_CHECK_NoFilterSaveNew,				// �V�K����ۑ����͑S�t�@�C���\��	// 2006.11.16 ryoji
	IDC_CHECK_NoFilterSaveFile,				HIDC_CHECK_NoFilterSaveFile,			// �V�K�ȊO����ۑ����͑S�t�@�C���\��	// 2006.11.16 ryoji
//	IDC_STATIC,								-1,
	0, 0
};

TYPE_NAME<EShareMode> ShareModeArr[] = {
	{ SHAREMODE_NOT_EXCLUSIVE,	_T("���Ȃ�") },
	{ SHAREMODE_DENY_WRITE,		_T("�㏑�����֎~����") },
	{ SHAREMODE_DENY_READWRITE,	_T("�ǂݏ������֎~����") },
};

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropFile::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/*! �t�@�C���y�[�W ���b�Z�[�W���� */
INT_PTR CPropFile::DispatchEvent(
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
		/* �_�C�A���O�f�[�^�̐ݒ� File */
		SetData( hwndDlg );
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

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
////				MYTRACE_A( "p2 PSN_KILLACTIVE\n" );
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
//				MYTRACE_A( "File PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 File */
				GetData( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_FILE;
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
//			break;
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
		case IDC_SPIN_ALERT_FILESIZE:
			/* �t�@�C���̌x���T�C�Y */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_ALERT_FILESIZE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else 
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 2048 ){
				nVal = 2048;  // �ő� 2GB �܂�
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_ALERT_FILESIZE, nVal, FALSE );
			return TRUE;
			/*NOTREACHED*/
//			break;
//@@@ 2001.03.21 End by MIK
		}
//****	To Here Sept. 21, 2000 JEPRO �_�C�A���O�v�f�ɃX�s��������̂�WM_NOTIFY���R�����g�A�E�g�ɂ����̉��ɏC����u����
		break;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */

		if( wID == IDC_COMBO_FILESHAREMODE && wNotifyCode == CBN_SELCHANGE ){	// �R���{�{�b�N�X�̑I��ύX
			EnableFilePropInput(hwndDlg);
			break;
		}

		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_bDropFileAndClose:/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
			case IDC_CHECK_AUTOSAVE:
			case IDC_CHECK_ALERT_IF_LARGEFILE:
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
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
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





/*! �t�@�C���y�[�W: �_�C�A���O�f�[�^�̐ݒ�
	���L����������f�[�^��ǂݏo���Ċe�R���g���[���ɒl��ݒ肷��B

	@par �o�b�N�A�b�v���㐔���Ó��Ȓl���ǂ����̃`�F�b�N���s���B�s�K�؂Ȓl�̎���
	�ł��߂��K�؂Ȓl��ݒ肷��B

	@param hwndDlg �v���p�e�B�y�[�W��Window Handle
*/
void CPropFile::SetData( HWND hwndDlg )
{
	/*--- File ---*/
	/* �t�@�C���̔r�����䃂�[�h */
	HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FILESHAREMODE );
	Combo_ResetContent( hwndCombo );
	int		nSelPos = 0;
	for( int i = 0; i < _countof( ShareModeArr ); ++i ){
		Combo_InsertString( hwndCombo, i, ShareModeArr[i].pszName );
		if( ShareModeArr[i].nMethod == m_Common.m_sFile.m_nFileShareMode ){
			nSelPos = i;
		}
	}
	Combo_SetCurSel( hwndCombo, nSelPos );

	/* �X�V�̊Ď� */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_bCheckFileTimeStamp, m_Common.m_sFile.m_bCheckFileTimeStamp );

	/* �㏑���֎~���o���͕ҏW�֎~�ɂ��� */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_bUneditableIfUnwritable, m_Common.m_sFile.m_bUneditableIfUnwritable );

	/* ���ύX�ł��㏑�����邩 */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE, m_Common.m_sFile.m_bEnableUnmodifiedOverwrite );

	/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_bDropFileAndClose, m_Common.m_sFile.m_bDropFileAndClose );
	/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_nDropFileNumMax, m_Common.m_sFile.m_nDropFileNumMax, FALSE );

	//	From Here Aug. 21, 2000 genta
	//	�����ۑ��̗L���E����
	::CheckDlgButton( hwndDlg, IDC_CHECK_AUTOSAVE, m_Common.m_sBackup.IsAutoBackupEnabled() );

	TCHAR buf[6];
	int nN;

	nN = m_Common.m_sBackup.GetAutoBackupInterval();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 35791 ? 35791 : nN;

	auto_sprintf( buf, _T("%d"), nN);
	::DlgItem_SetText( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, buf );
	//	To Here Aug. 21, 2000 genta

	//	Oct. 27, 2000 genta	�J�[�\���ʒu�����t���O
	::CheckDlgButton( hwndDlg, IDC_CHECK_RestoreCurPosition, m_Common.m_sFile.GetRestoreCurPosition() );
	// 2002.01.16 hor �u�b�N�}�[�N�����t���O
	::CheckDlgButton( hwndDlg, IDC_CHECK_RestoreBookmarks, m_Common.m_sFile.GetRestoreBookmarks() );
	//	Nov. 12, 2000 genta	MIME Decode�t���O
	::CheckDlgButton( hwndDlg, IDC_CHECK_AutoMIMEDecode, m_Common.m_sFile.GetAutoMIMEdecode() );
	//	Oct. 03, 2004 genta �O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s�����ǂ����̃t���O
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_QueryIfCodeChange, m_Common.m_sFile.GetQueryIfCodeChange() );
	//	Oct. 09, 2004 genta �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x�����邩�ǂ����̃t���O
	::CheckDlgButton( hwndDlg, IDC_CHECK_AlertIfFileNotExist, m_Common.m_sFile.GetAlertIfFileNotExist() );
	//	�t�@�C���T�C�Y���傫���ꍇ�Ɍx�����o��
	::CheckDlgButton( hwndDlg, IDC_CHECK_ALERT_IF_LARGEFILE, m_Common.m_sFile.m_bAlertIfLargeFile );
	::SetDlgItemInt( hwndDlg, IDC_EDIT_ALERT_FILESIZE, m_Common.m_sFile.m_nAlertFileSize, FALSE );

	// �t�@�C���ۑ��_�C�A���O�̃t�B���^�ݒ�	// 2006.11.16 ryoji
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_NoFilterSaveNew, m_Common.m_sFile.m_bNoFilterSaveNew );	// �V�K����ۑ����͑S�t�@�C���\��
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_NoFilterSaveFile, m_Common.m_sFile.m_bNoFilterSaveFile );	// �V�K�ȊO����ۑ����͑S�t�@�C���\��

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
int CPropFile::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_FILE;

	/* �t�@�C���̔r�����䃂�[�h */
	HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FILESHAREMODE );
	int		nSelPos = Combo_GetCurSel( hwndCombo );
	m_Common.m_sFile.m_nFileShareMode = ShareModeArr[nSelPos].nMethod;

	/* �X�V�̊Ď� */
	m_Common.m_sFile.m_bCheckFileTimeStamp = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_bCheckFileTimeStamp );

	/* �㏑���֎~���o���͕ҏW�֎~�ɂ��� */
	m_Common.m_sFile.m_bUneditableIfUnwritable = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_bUneditableIfUnwritable );

	/* ���ύX�ł��㏑�����邩 */
	m_Common.m_sFile.m_bEnableUnmodifiedOverwrite = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE );

	/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
	m_Common.m_sFile.m_bDropFileAndClose = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_bDropFileAndClose );
	/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
	m_Common.m_sFile.m_nDropFileNumMax = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nDropFileNumMax, NULL, FALSE );
	if( 1 > m_Common.m_sFile.m_nDropFileNumMax ){
		m_Common.m_sFile.m_nDropFileNumMax = 1;
	}
	if( 99 < m_Common.m_sFile.m_nDropFileNumMax ){	//Sept. 21, 2000, JEPRO 16���傫���Ƃ���99�Ɛ�������Ă����̂��C��(16��99�ƕύX)
		m_Common.m_sFile.m_nDropFileNumMax = 99;
	}

	//	From Here Aug. 16, 2000 genta
	//	�����ۑ����s�����ǂ���
	m_Common.m_sBackup.EnableAutoBackup( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_AUTOSAVE ) == TRUE );

	//	�����ۑ��Ԋu�̎擾
	TCHAR szNumBuf[/*6*/ 7];	//@@@ 2001.03.21 by MIK
	int	 nN;
	TCHAR *pDigit;

	::DlgItem_GetText( hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, szNumBuf, /*5*/ 6 );	//@@@ 2001.03.21 by MIK

	for( nN = 0, pDigit = szNumBuf; *pDigit != _T('\0'); pDigit++ ){
		if( _T('0') <= *pDigit && *pDigit <= _T('9') ){
			nN = nN * 10 + *pDigit - _T('0');
		}
		else
			break;
	}
	nN = nN < 1  ?  1 : nN;
	nN = nN > 35791 ? 35791 : nN;
	m_Common.m_sBackup.SetAutoBackupInterval( nN );

	//	To Here Aug. 16, 2000 genta

	//	Oct. 27, 2000 genta	�J�[�\���ʒu�����t���O
	m_Common.m_sFile.SetRestoreCurPosition( ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_RestoreCurPosition ) );
	// 2002.01.16 hor �u�b�N�}�[�N�����t���O
	m_Common.m_sFile.SetRestoreBookmarks( ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_RestoreBookmarks ) );
	//	Nov. 12, 2000 genta	MIME Decode�t���O
	m_Common.m_sFile.SetAutoMIMEdecode( ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_AutoMIMEDecode ) );
	//	Oct. 03, 2004 genta �O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s�����ǂ����̃t���O
	m_Common.m_sFile.SetQueryIfCodeChange( ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_QueryIfCodeChange ) );
	//	Oct. 03, 2004 genta �O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s�����ǂ����̃t���O
	m_Common.m_sFile.SetAlertIfFileNotExist( ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_AlertIfFileNotExist ) );
	// �J�����Ƃ����t�@�C�����傫���ꍇ�Ɍx������
	m_Common.m_sFile.m_bAlertIfLargeFile = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_ALERT_IF_LARGEFILE );
	m_Common.m_sFile.m_nAlertFileSize = ::GetDlgItemInt( hwndDlg, IDC_EDIT_ALERT_FILESIZE, NULL, FALSE );
	if( m_Common.m_sFile.m_nAlertFileSize < 1 ){
		m_Common.m_sFile.m_nAlertFileSize = 1;
	}
	if( m_Common.m_sFile.m_nAlertFileSize > 2048 ){
		m_Common.m_sFile.m_nAlertFileSize = 2048;
	}

	// �t�@�C���ۑ��_�C�A���O�̃t�B���^�ݒ�	// 2006.11.16 ryoji
	m_Common.m_sFile.m_bNoFilterSaveNew = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_NoFilterSaveNew );	// �V�K����ۑ����͑S�t�@�C���\��
	m_Common.m_sFile.m_bNoFilterSaveFile = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_NoFilterSaveFile );	// �V�K�ȊO����ۑ����͑S�t�@�C���\��

	return TRUE;
}

//	From Here Aug. 21, 2000 genta
/*!	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
	�K�؂ɐݒ肷��

	@param hwndDlg �v���p�e�B�V�[�g��Window Handle
*/
void CPropFile::EnableFilePropInput(HWND hwndDlg)
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
	int nSelPos = Combo_GetCurSel( ::GetDlgItem( hwndDlg, IDC_COMBO_FILESHAREMODE ) );
	if( ShareModeArr[nSelPos].nMethod == SHAREMODE_NOT_EXCLUSIVE ){
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

	// �u�J�����Ƃ����t�@�C�����傫���ꍇ�Ɍx�����o���v
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALERT_IF_LARGEFILE ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_ALERT_FILESIZE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_ALERT_FILESIZE ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_ALERT_FILESIZE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_ALERT_FILESIZE ), FALSE );
	}
}
//	To Here Aug. 21, 2000 genta



