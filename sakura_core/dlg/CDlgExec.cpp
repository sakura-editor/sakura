/*!	@file
	@brief �O���R�}���h���s�_�C�A���O

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, aroka, YAZAKI, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, maru
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "dlg/CDlgExec.h"
#include "dlg/CDlgOpenFile.h"	//Mar. 28, 2001 JEPRO
#include "func/Funccode.h"	//Stonee, 2001/03/12  �R�����g�A�E�g����Ă��̂�L���ɂ���
#include "util/shell.h"
#include "util/window.h"
#include "_main/CAppMode.h"
#include "doc/CEditDoc.h"
#include "sakura_rc.h"
#include "sakura.hh"

//�O���R�}���h CDlgExec.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12100
	IDC_BUTTON_REFERENCE,			HIDC_EXEC_BUTTON_REFERENCE,		//�Q��
	IDOK,							HIDOK_EXEC,						//���s
	IDCANCEL,						HIDCANCEL_EXEC,					//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_EXEC_BUTTON_HELP,			//�w���v
	IDC_CHECK_GETSTDOUT,			HIDC_EXEC_CHECK_GETSTDOUT,		//�W���o�͂𓾂�
	IDC_COMBO_CODE_GET,				HIDC_COMBO_CODE_GET,			//�W���o�͕����R�[�h
	IDC_COMBO_m_szCommand,			HIDC_EXEC_COMBO_m_szCommand,	//�R�}���h
	IDC_RADIO_OUTPUT,				HIDC_RADIO_OUTPUT,				//�W���o�̓��_�C���N�g��F�A�E�g�v�b�g�E�B���h�E
	IDC_RADIO_EDITWINDOW,			HIDC_RADIO_EDITWINDOW,			//�W���o�̓��_�C���N�g��F�ҏW���̃E�B���h�E
	IDC_CHECK_SENDSTDIN,			HIDC_CHECK_SENDSTDIN,			//�W�����͂ɑ���
	IDC_COMBO_CODE_SEND,			HIDC_COMBO_CODE_SEND,			//�W���o�͕����R�[�h
	IDC_CHECK_CUR_DIR,				HIDC_CHECK_CUR_DIR,				//�J�����g�f�B���N�g��
	IDC_COMBO_CUR_DIR,				HIDC_COMBO_CUR_DIR,				//�J�����g�f�B���N�g���w��
	IDC_BUTTON_REFERENCE2,			HIDC_COMBO_CUR_DIR,				//�J�����g�f�B���N�g���w��(�Q��)
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgExec::CDlgExec()
{
	m_szCommand[0] = _T('\0');	/* �R�}���h���C�� */
	return;
}

static const int codeTable1[] = { 0x00, 0x08, 0x80 };
static const int codeTable2[] = { 0x00, 0x10, 0x100 };



/* ���[�_���_�C�A���O�̕\�� */
int CDlgExec::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	m_szCommand[0] = _T('\0');	/* �R�}���h���C�� */
	m_bEditable = CEditDoc::GetInstance(0)->IsEditable();
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_EXEC, lParam );
}


BOOL CDlgExec::OnInitDialog( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwnd );
	
	ECodeType codes[] = { CODE_SJIS, CODE_UNICODE, CODE_UTF8 };
	HWND hwndCombo;
	int i;
	hwndCombo = GetItemHwnd( IDC_COMBO_CODE_GET );
	for( i = 0; i < _countof(codes); ++i ){
		Combo_AddString( hwndCombo, CCodeTypeName(codes[i]).Normal() );
	}
	hwndCombo = GetItemHwnd( IDC_COMBO_CODE_SEND );
	for( i = 0; i < _countof(codes); ++i ){
		Combo_AddString( hwndCombo, CCodeTypeName(codes[i]).Normal() );
	}
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgExec::SetData( void )
{
//	MYTRACE( _T("CDlgExec::SetData()") );
	int		i;
	HWND	hwndCombo;

	/*****************************
	*           ����             *
	*****************************/
	/* ���[�U�[���R���{ �{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	Combo_LimitText( ::GetDlgItem( GetHwnd(), IDC_COMBO_m_szCommand ), _countof( m_szCommand ) - 1 );
	Combo_LimitText( ::GetDlgItem( GetHwnd(), IDC_COMBO_CUR_DIR ), _countof2( m_szCurDir ) - 1 );
	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_m_szCommand ), TRUE );

	{	//	From Here 2007.01.02 maru �������g���̂���
		//	�}�N������̌Ăяo���ł�ShareData�ɕۑ������Ȃ��悤�ɁCShareData�Ƃ̎󂯓n����ExecCmd�̊O��
		int nExecFlgOpt;
		nExecFlgOpt = m_pShareData->m_nExecFlgOpt;
		
		// �r���[���[�h��㏑���֎~�̂Ƃ��͕ҏW���E�B���h�E�ւ͏o�͂��Ȃ�	// 2009.02.21 ryoji
		if( !m_bEditable ){
			nExecFlgOpt &= ~0x02;
		}

		::CheckDlgButton( GetHwnd(), IDC_CHECK_GETSTDOUT, nExecFlgOpt & 0x01 ? BST_CHECKED : BST_UNCHECKED );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUT, nExecFlgOpt & 0x02 ? BST_UNCHECKED : BST_CHECKED );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_EDITWINDOW, nExecFlgOpt & 0x02 ? BST_CHECKED : BST_UNCHECKED );
		::CheckDlgButton( GetHwnd(), IDC_CHECK_SENDSTDIN, nExecFlgOpt & 0x04 ? BST_CHECKED : BST_UNCHECKED );
		::CheckDlgButton( GetHwnd(), IDC_CHECK_CUR_DIR, nExecFlgOpt & 0x200 ? BST_CHECKED : BST_UNCHECKED );

		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_OUTPUT ), nExecFlgOpt & 0x01 ? TRUE : FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_EDITWINDOW ), ((nExecFlgOpt & 0x01) && m_bEditable)? TRUE : FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_CODE_GET ), nExecFlgOpt & 0x01 ? TRUE : FALSE );		// �W���o��Off���AUnicode���g�p�����Desable����	2008/6/20 Uchi
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_CODE_SEND ), nExecFlgOpt & 0x04 ? TRUE : FALSE );		// �W������Off���AUnicode���g�p�����Desable����	2008/6/20 Uchi
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_CUR_DIR ), nExecFlgOpt & 0x200 ? TRUE : FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_REFERENCE2 ), nExecFlgOpt & 0x200 ? TRUE : FALSE );
	}	//	To Here 2007.01.02 maru �������g���̂���

	/*****************************
	*         �f�[�^�ݒ�         *
	*****************************/
	_tcscpy( m_szCommand, m_pShareData->m_sHistory.m_aCommands[0] );
	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_m_szCommand );
	Combo_ResetContent( hwndCombo );
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT, m_szCommand );
	int nSize = m_pShareData->m_sHistory.m_aCommands.size();
	for( i = 0; i < nSize; ++i ){
		Combo_AddString( hwndCombo, m_pShareData->m_sHistory.m_aCommands[i] );
	}
	Combo_SetCurSel( hwndCombo, 0 );

	_tcscpy( m_szCurDir, m_pShareData->m_sHistory.m_aCurDirs[0] );
	hwndCombo = GetItemHwnd( IDC_COMBO_CUR_DIR );
	Combo_ResetContent( hwndCombo );
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT, m_szCurDir );
	for( i = 0; i < m_pShareData->m_sHistory.m_aCurDirs.size(); ++i ){
		Combo_AddString( hwndCombo, m_pShareData->m_sHistory.m_aCurDirs[i] );
	}
	Combo_SetCurSel( hwndCombo, 0 );
	
	int nOpt;
	hwndCombo = GetItemHwnd( IDC_COMBO_CODE_GET );
	nOpt = m_pShareData->m_nExecFlgOpt & 0x88;
	for( i = 0; _countof(codeTable1); i++ ){
		if( codeTable1[i] == nOpt ){
			Combo_SetCurSel( hwndCombo, i );
			break;
		}
	}
	hwndCombo = GetItemHwnd( IDC_COMBO_CODE_SEND );
	nOpt = m_pShareData->m_nExecFlgOpt & 0x110;
	for( i = 0; _countof(codeTable2); i++ ){
		if( codeTable2[i] == nOpt ){
			Combo_SetCurSel( hwndCombo, i );
			break;
		}
	}
	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
int CDlgExec::GetData( void )
{
	DlgItem_GetText( GetHwnd(), IDC_COMBO_m_szCommand, m_szCommand, _countof( m_szCommand ));
	if( IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_CUR_DIR ) ){
		DlgItem_GetText( GetHwnd(), IDC_COMBO_CUR_DIR, &m_szCurDir[0], _countof2( m_szCurDir ));
	}else{
		m_szCurDir[0] = _T('\0');
	}
	{	//	From Here 2007.01.02 maru �������g���̂���
		//	�}�N������̌Ăяo���ł�ShareData�ɕۑ������Ȃ��悤�ɁCShareData�Ƃ̎󂯓n����ExecCmd�̊O��
		int nFlgOpt = 0;
		nFlgOpt |= ( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_GETSTDOUT ) ) ? 0x01 : 0;	// �W���o�͂𓾂�
		nFlgOpt |= ( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_EDITWINDOW ) ) ? 0x02 : 0;	// �W���o�͂�ҏW���̃E�C���h�E��
		nFlgOpt |= ( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_SENDSTDIN ) ) ? 0x04 : 0;	// �ҏW���t�@�C����W�����͂�
		nFlgOpt |= ( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_CUR_DIR ) ) ? 0x200 : 0;	// �J�����g�f�B���N�g���w��
		int sel;
		sel = Combo_GetCurSel( GetItemHwnd( IDC_COMBO_CODE_GET ) );
		nFlgOpt |= codeTable1[sel];
		sel = Combo_GetCurSel( GetItemHwnd( IDC_COMBO_CODE_SEND ) );
		nFlgOpt |= codeTable2[sel];
		m_pShareData->m_nExecFlgOpt = nFlgOpt;
	}	//	To Here 2007.01.02 maru �������g���̂���
	return 1;
}



BOOL CDlgExec::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_CHECK_GETSTDOUT:
		{	//	From Here 2007.01.02 maru �������g���̂���
			BOOL bEnabled;
			bEnabled = (BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_GETSTDOUT)) ? TRUE : FALSE;
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_OUTPUT ), bEnabled );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_EDITWINDOW ), (bEnabled && m_bEditable) ? TRUE : FALSE );	// �r���[���[�h��㏑���֎~�̏����ǉ�	// 2009.02.21 ryoji
		}	//	To Here 2007.01.02 maru �������g���̂���

		// �W���o��Off���AUnicode���g�p�����Desable����	2008/6/20 Uchi
		::EnableWindow(
			GetItemHwnd( IDC_COMBO_CODE_GET ), 
			BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_GETSTDOUT )
		);
		break;
	case IDC_CHECK_SENDSTDIN:	// �W������Off���AUnicode���g�p�����Desable����	2008/6/20 Uchi
		::EnableWindow( GetItemHwnd( IDC_COMBO_CODE_SEND ), 
			BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_SENDSTDIN ) );
		break;
	case IDC_CHECK_CUR_DIR:
		::EnableWindow( GetItemHwnd( IDC_COMBO_CUR_DIR ),
			BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_CUR_DIR ) );
		::EnableWindow( GetItemHwnd( IDC_BUTTON_REFERENCE2 ),
			BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_CUR_DIR ) );
		break;

	case IDC_BUTTON_HELP:
		/* �u�����v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_EXECMD_DIALOG) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		break;

	//From Here Mar. 28, 2001 JEPRO
	case IDC_BUTTON_REFERENCE:	/* �t�@�C�����́u�Q��...�v�{�^�� */
		{
			CDlgOpenFile	cDlgOpenFile;
			TCHAR			szPath[_MAX_PATH + 1];
			int				size = _countof(szPath) - 1;
			_tcsncpy( szPath, m_szCommand, size);
			szPath[size] = _T('\0');
			/* �t�@�C���I�[�v���_�C�A���O�̏����� */
			cDlgOpenFile.Create(
				m_hInstance,
				GetHwnd(),
				_T("*.com;*.exe;*.bat;*.cmd"),
				m_szCommand
			);
			if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
				_tcscpy( m_szCommand, szPath );
				::DlgItem_SetText( GetHwnd(), IDC_COMBO_m_szCommand, m_szCommand );
			}
		}
		return TRUE;
	//To Here Mar. 28, 2001

	case IDC_BUTTON_REFERENCE2:
		{
			if( SelectDir( GetHwnd(), _T("�J�����g�f�B���N�g����I��ł�������"), &m_szCurDir[0], &m_szCurDir[0] ) ){
				::DlgItem_SetText( GetHwnd(), IDC_COMBO_CUR_DIR, &m_szCurDir[0] );
				
			}
		}
		return TRUE;

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


