/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�}�N���n)�֐��Q

	2012/12/20	CViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, MIK, Stonee, Misaka, asa-o, novice, hor, YAZAKI
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2003, Moca
	Copyright (C) 2005, novice
	Copyright (C) 2006, maru
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, syat

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
#include "macro/CSMacroMgr.h"
#include "dlg/CDlgExec.h"
#include "window/CEditWnd.h"/// 2002/2/3 aroka �ǉ�
#include "CEditApp.h"
#include "util/module.h"
#include "env/CShareData.h"
#include "env/CSakuraEnvironment.h"


/* �L�[�}�N���̋L�^�J�n�^�I�� */
void CViewCommander::Command_RECKEYMACRO( void )
{
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro ){									/* �L�[�{�[�h�}�N���̋L�^�� */
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;							/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
		//@@@ 2002.1.24 YAZAKI �L�[�}�N�����}�N���p�t�H���_�ɁuRecKey.mac�v�Ƃ������ŕۑ�
		TCHAR szInitDir[MAX_PATH];
		int nRet;
		// 2003.06.23 Moca �L�^�p�L�[�}�N���̃t���p�X��CShareData�o�R�Ŏ擾
		nRet = CShareData::getInstance()->GetMacroFilename( -1, szInitDir, MAX_PATH ); 
		if( nRet <= 0 ){
			ErrorMessage( m_pCommanderView->GetHwnd(), _T("�}�N���t�@�C�����쐬�ł��܂���ł����B\n�t�@�C�����̎擾�G���[ nRet=%d"), nRet );
			return;
		}else{
			_tcscpy( GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName, szInitDir );
		}
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		int nSaveResult=CEditApp::getInstance()->m_pcSMacroMgr->Save(
			STAND_KEYMACRO,
			G_AppInstance(),
			GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName
		);
		if ( !nSaveResult ){
			ErrorMessage(	m_pCommanderView->GetHwnd(), _T("�}�N���t�@�C�����쐬�ł��܂���ł����B\n\n%ts"), GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName );
		}
	}else{
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = TRUE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = GetMainWindow();	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
		/* �L�[�}�N���̃o�b�t�@���N���A���� */
		//@@@ 2002.1.24 m_CKeyMacroMgr��CEditDoc�ֈړ�
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		CEditApp::getInstance()->m_pcSMacroMgr->Clear(STAND_KEYMACRO);
//		GetDocument()->m_CKeyMacroMgr.ClearAll();
//		GetDllShareData().m_CKeyMacroMgr.Clear();
	}
	/* �e�E�B���h�E�̃^�C�g�����X�V */
	this->GetEditWindow()->UpdateCaption();

	/* �L�����b�g�̍s���ʒu��\������ */
	GetCaret().ShowCaretPosInfo();
}



/* �L�[�}�N���̕ۑ� */
void CViewCommander::Command_SAVEKEYMACRO( void )
{
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

	//	Jun. 16, 2002 genta
	if( !CEditApp::getInstance()->m_pcSMacroMgr->IsSaveOk() ){
		//	�ۑ��s��
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("�ۑ��\�ȃ}�N��������܂���D�L�[�{�[�h�}�N���ȊO�͕ۑ��ł��܂���D") );
	}

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	_tcscpy( szPath, _T("") );
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	if( _IS_REL_PATH( GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER ) ){
		GetInidirOrExedir( szInitDir, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER );
	}else{
		_tcscpy( szInitDir, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER );	/* �}�N���p�t�H���_ */
	}
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		_T("*.mac"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
//	::SplitPath_FolderAndFile( szPath, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER, NULL );
//	wcscat( GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER, L"\\" );

	/* �L�[�{�[�h�}�N���̕ۑ� */
	//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
	//@@@ 2002.1.24 YAZAKI
	if ( !CEditApp::getInstance()->m_pcSMacroMgr->Save( STAND_KEYMACRO, G_AppInstance(), szPath ) ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("�}�N���t�@�C�����쐬�ł��܂���ł����B\n\n%ts"), szPath );
	}
	return;
}



/*! �L�[�}�N���̓ǂݍ���
	@date 2005.02.20 novice �f�t�H���g�̊g���q�ύX
 */
void CViewCommander::Command_LOADKEYMACRO( void )
{
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	const TCHAR*		pszFolder;
	_tcscpy( szPath, _T("") );
	pszFolder = GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER;
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	if( _IS_REL_PATH( pszFolder ) ){
		GetInidirOrExedir( szInitDir, pszFolder );
	}else{
		_tcscpy( szInitDir, pszFolder );	/* �}�N���p�t�H���_ */
	}
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
// 2005/02/20 novice �f�t�H���g�̊g���q�ύX
// 2005/07/13 novice ���l�ȃ}�N�����T�|�[�g���Ă���̂Ńf�t�H���g�͑S�ĕ\���ɂ���
		_T("*.*"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}

	/* �L�[�{�[�h�}�N���̓ǂݍ��� */
	//@@@ 2002.1.24 YAZAKI �ǂݍ��݂Ƃ������A�t�@�C�������R�s�[���邾���B���s���O�ɓǂݍ���
	_tcscpy(GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName, szPath);
//	GetDllShareData().m_CKeyMacroMgr.LoadKeyMacro( G_AppInstance(), m_pCommanderView->GetHwnd(), szPath );
	return;
}



/* �L�[�}�N���̎��s */
void CViewCommander::Command_EXECKEYMACRO( void )
{
	//@@@ 2002.1.24 YAZAKI �L�^���͏I�����Ă�����s
	if (GetDllShareData().m_sFlags.m_bRecordingKeyMacro){
		Command_RECKEYMACRO();
	}
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

	/* �L�[�{�[�h�}�N���̎��s */
	//@@@ 2002.1.24 YAZAKI
	if ( GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName[0] ){
		//	�t�@�C�����ۑ�����Ă�����
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		BOOL bLoadResult = CEditApp::getInstance()->m_pcSMacroMgr->Load(
			STAND_KEYMACRO,
			G_AppInstance(),
			GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName,
			NULL
		);
		if ( !bLoadResult ){
			ErrorMessage( m_pCommanderView->GetHwnd(), _T("�t�@�C�����J���܂���ł����B\n\n%ts"), GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName );
		}
		else {
			//	2007.07.20 genta : flags�I�v�V�����ǉ�
			CEditApp::getInstance()->m_pcSMacroMgr->Exec( STAND_KEYMACRO, G_AppInstance(), m_pCommanderView, 0 );
		}
	}

	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	m_pCommanderView->RedrawAll();

	return;
}



/*! ���O���w�肵�ă}�N�����s
	@param pszPath	�}�N���̃t�@�C���p�X�A�܂��̓}�N���̃R�[�h�B
	@param pszType	��ʁBNULL�̏ꍇ�t�@�C���w��A����ȊO�̏ꍇ�͌���̊g���q���w��

	@date 2008.10.23 syat �V�K�쐬
	@date 2008.12.21 syat �����u��ʁv��ǉ�
 */
void CViewCommander::Command_EXECEXTMACRO( const WCHAR* pszPathW, const WCHAR* pszTypeW )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];	//�t�@�C���I���_�C�A���O�̏����t�H���_
	const TCHAR*	pszFolder;					//�}�N���t�H���_
	const TCHAR*	pszPath = NULL;				//��1������TCHAR*�ɕϊ�����������
	const TCHAR*	pszType = NULL;				//��2������TCHAR*�ɕϊ�����������
	HWND			hwndRecordingKeyMacro = NULL;

	if ( pszPathW != NULL ) {
		//to_tchar()�Ŏ擾�����������delete���Ȃ����ƁB
		pszPath = to_tchar( pszPathW );
		pszType = to_tchar( pszTypeW );

	} else {
		// �t�@�C�����w�肳��Ă��Ȃ��ꍇ�A�_�C�A���O��\������
		_tcscpy( szPath, _T("") );
		pszFolder = GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER;

		if( _IS_REL_PATH( pszFolder ) ){
			GetInidirOrExedir( szInitDir, pszFolder );
		}else{
			_tcscpy( szInitDir, pszFolder );	/* �}�N���p�t�H���_ */
		}
		/* �t�@�C���I�[�v���_�C�A���O�̏����� */
		cDlgOpenFile.Create(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			_T("*.*"),
			szInitDir
		);
		if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
			return;
		}
		pszPath = szPath;
		pszType = NULL;
	}

	//�L�[�}�N���L�^���̏ꍇ�A�ǉ�����
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro &&									/* �L�[�{�[�h�}�N���̋L�^�� */
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow()	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
	){
		CEditApp::getInstance()->m_pcSMacroMgr->Append( STAND_KEYMACRO, F_EXECEXTMACRO, (LPARAM)pszPath, m_pCommanderView );

		//�L�[�}�N���̋L�^���ꎞ��~����
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
		hwndRecordingKeyMacro = GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
	}

	//�Â��ꎞ�}�N���̑ޔ�
	CMacroManagerBase* oldMacro = CEditApp::getInstance()->m_pcSMacroMgr->SetTempMacro( NULL );

	BOOL bLoadResult = CEditApp::getInstance()->m_pcSMacroMgr->Load(
		TEMP_KEYMACRO,
		G_AppInstance(),
		pszPath,
		pszType
	);
	if ( !bLoadResult ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("�}�N���̓ǂݍ��݂Ɏ��s���܂����B\n\n%ts"), pszPath );
	}
	else {
		CEditApp::getInstance()->m_pcSMacroMgr->Exec( TEMP_KEYMACRO, G_AppInstance(), m_pCommanderView, FA_NONRECORD | FA_FROMMACRO );
	}

	// �I�������J��
	CEditApp::getInstance()->m_pcSMacroMgr->Clear( TEMP_KEYMACRO );
	if ( oldMacro != NULL ) {
		CEditApp::getInstance()->m_pcSMacroMgr->SetTempMacro( oldMacro );
	}

	// �L�[�}�N���L�^���������ꍇ�͍ĊJ����
	if ( hwndRecordingKeyMacro != NULL ) {
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = TRUE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = hwndRecordingKeyMacro;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
	}

	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	m_pCommanderView->RedrawAll();

	return;
}



/*! �O���R�}���h���s�_�C�A���O�\��
	@date 2002.02.02 YAZAKI.
*/
void CViewCommander::Command_EXECCOMMAND_DIALOG( void )
{
	CDlgExec cDlgExec;

	/* ���[�h���X�_�C�A���O�̕\�� */
	if( !cDlgExec.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0 ) ){
		return;
	}

	m_pCommanderView->AddToCmdArr( cDlgExec.m_szCommand );
	const WCHAR* cmd_string = to_wchar(cDlgExec.m_szCommand);

	//HandleCommand( F_EXECMD, true, (LPARAM)cmd_string, 0, 0, 0);	//	�O���R�}���h���s�R�}���h�̔��s
	HandleCommand( F_EXECMD, true, (LPARAM)cmd_string, (LPARAM)(GetDllShareData().m_nExecFlgOpt), 0, 0);	//	�O���R�}���h���s�R�}���h�̔��s
}



//�O���R�}���h���s
//	Sept. 20, 2000 JEPRO  ����CMMAND��COMMAND�ɕύX
//	Oct. 9, 2001   genta  �}�N���Ή��̂��߈����ǉ�
//  2002.2.2       YAZAKI �_�C�A���O�Ăяo�����ƃR�}���h���s���𕪗�
//void CEditView::Command_EXECCOMMAND( const char *cmd_string )
void CViewCommander::Command_EXECCOMMAND( LPCWSTR cmd_string, const int nFlgOpt)	//	2006.12.03 maru �����̊g��
{
	//	From Here Aug. 21, 2001 genta
	//	�p�����[�^�u�� (���b��)
	const int bufmax = 1024;
	wchar_t buf[bufmax + 1];
	CSakuraEnvironment::ExpandParameter(cmd_string, buf, bufmax);

	// �q�v���Z�X�̕W���o�͂����_�C���N�g����
	m_pCommanderView->ExecCmd( to_tchar(buf), nFlgOpt );
	//	To Here Aug. 21, 2001 genta
	return;
}
