//	$Id$
/*!	@file
	@brief �O���R�}���h���s�_�C�A���O

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CDlgExec.h"
#include "funccode.h"	//Stonee, 2001/03/12  �R�����g�A�E�g����Ă��̂�L���ɂ���
#include "sakura_rc.h"
#include "etc_uty.h"	//Stonee, 2001/03/12
#include <windows.h>		//Mar. 28, 2001 JEPRO (�ꉞ���ꂽ���s�v�H)
#include <stdio.h>			//Mar. 28, 2001 JEPRO (�ꉞ���ꂽ���s�v�H)
#include <commctrl.h>		//Mar. 28, 2001 JEPRO
#include "CDlgOpenFile.h"	//Mar. 28, 2001 JEPRO
#include "debug.h"// 2002/2/10 aroka �w�b�_����

//�O���R�}���h CDlgExec.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12100
	IDC_BUTTON_REFERENCE,			HIDC_EXEC_BUTTON_REFERENCE,		//�Q��
	IDOK,							HIDOK_EXEC,						//���s
	IDCANCEL,						HIDCANCEL_EXEC,					//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_EXEC_BUTTON_HELP,			//�w���v
	IDC_CHECK_GETSTDOUT,			HIDC_EXEC_CHECK_GETSTDOUT,		//�W���o�͂𓾂�
	IDC_COMBO_m_szCommand,			HIDC_EXEC_COMBO_m_szCommand,	//�R�}���h
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgExec::CDlgExec()
{
	m_szCommand[0] = '\0';	/* �R�}���h���C�� */
//	m_bGetStdout = /*FALSE*/TRUE;	// �W���o�͂𓾂�	//Mar. 21, 2001 JEPRO [���Ȃ�]���f�t�H���g�ɕύX	//Jul. 03, 2001 JEPRO [����]���f�t�H���g�ƂȂ�悤�ɖ߂���

	return;
}




/* ���[�_���_�C�A���O�̕\�� */
int CDlgExec::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	m_szCommand[0] = '\0';	/* �R�}���h���C�� */
	return CDialog::DoModal( hInstance, hwndParent, IDD_EXEC, lParam );
}




/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgExec::SetData( void )
{
//	MYTRACE( "CDlgExec::SetData()" );
	int		i;
	HWND	hwndCombo;

	/*****************************
	*           ����             *
	*****************************/
	/* ���[�U�[���R���{ �{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_m_szCommand ), CB_LIMITTEXT, (WPARAM)sizeof( m_szCommand ) - 1, 0 );
	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_m_szCommand ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );


	// �W���o�͂𓾂�
//	From Here Sept. 12, 2000 jeprotest
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
	::CheckDlgButton( m_hWnd, IDC_CHECK_GETSTDOUT, m_pShareData->m_bGetStdout/*m_bGetStdout*/ ? BST_CHECKED : BST_UNCHECKED );
//	::CheckDlgButton( m_hWnd, IDC_CHECK_GETSTDOUT, TRUE );
//	To Here Sept. 12, 2000 	���܂������Ȃ��̂Ō��ɖ߂��Ă���


	/*****************************
	*         �f�[�^�ݒ�         *
	*****************************/
//	HWND hwndCombo;
	strcpy( m_szCommand, m_pShareData->m_szCmdArr[0] );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_m_szCommand );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szCommand );
	for( i = 0; i < m_pShareData->m_nCmdArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szCmdArr[i] );
	}
	::SendMessage( hwndCombo, CB_SETCURSEL, 0, 0 );
	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
int CDlgExec::GetData( void )
{
	::GetDlgItemText( m_hWnd, IDC_COMBO_m_szCommand, m_szCommand, sizeof( m_szCommand ));

	// �W���o�͂𓾂�
//	From Here Sept. 12, 2000 jeprotest
	if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
//	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
//	To Here Sept. 12, 2000 ���܂������Ȃ��̂Ō��ɖ߂��Ă���
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
		 m_pShareData->m_bGetStdout = TRUE;
	}else{
		 m_pShareData->m_bGetStdout = FALSE;
	}
	return 1;
}



BOOL CDlgExec::OnBnClicked( int wID )
{
//	int	nRet;
//	CEditView*	pcEditView = (CEditView*)m_lParam;
	switch( wID ){
	//	From Here Sept. 12, 2000 jeprotest
	case IDC_CHECK_GETSTDOUT:
//@@@ 2002.01.08 YAZAKI GetData()�Ŏ擾���邽��
#if 0
//		if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
			 m_pShareData->m_bGetStdout = TRUE;
		}else{
			 m_pShareData->m_bGetStdout = FALSE;
		}
#endif
		break;
	//	To Here Sept. 12, 2000 ���܂������Ȃ��̂Ō��ɖ߂��Ă���
	case IDC_BUTTON_HELP:
		/* �u�����v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_EXECCOMMAND_DIALOG) );
		break;

	//From Here Mar. 28, 2001 JEPRO
	case IDC_BUTTON_REFERENCE:	/* �t�@�C�����́u�Q��...�v�{�^�� */
		{
			CDlgOpenFile	cDlgOpenFile;
			char*			pszMRU = NULL;;
			char*			pszOPENFOLDER = NULL;;
			char			szPath[_MAX_PATH + 1];
			strcpy( szPath, m_szCommand );
			/* �t�@�C���I�[�v���_�C�A���O�̏����� */
			cDlgOpenFile.Create(
				m_hInstance,
				m_hWnd,
				"*.com;*.exe;*.bat",
				m_szCommand,
				(const char **)&pszMRU,
				(const char **)&pszOPENFOLDER
			);
			if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
				strcpy( m_szCommand, szPath );
				::SetDlgItemText( m_hWnd, IDC_COMBO_m_szCommand, m_szCommand );
			}
		}
		return TRUE;
	//To Here Mar. 28, 2001

	case IDOK:			/* ������ */
		/* �_�C�A���O�f�[�^�̎擾 */
		GetData();
		CloseDialog( 1 );
		return TRUE;
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgExec::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
