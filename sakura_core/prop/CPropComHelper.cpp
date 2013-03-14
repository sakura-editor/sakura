/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�x���v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, genta, MIK, jepro, asa-o
	Copyright (C) 2002, YAZAKI, MIK, genta
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2012, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "dlg/CDlgOpenFile.h"
#include "util/shell.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"


//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10600
	IDC_BUTTON_OPENHELP1,			HIDC_BUTTON_OPENHELP1,			//�O���w���v�t�@�C���Q��
	IDC_BUTTON_OPENEXTHTMLHELP,		HIDC_BUTTON_OPENEXTHTMLHELP,	//�O��HTML�t�@�C���Q��
//	IDC_CHECK_USEHOKAN,				HIDC_CHECK_USEHOKAN,			//�������͕⊮
	IDC_CHECK_m_bHokanKey_RETURN,	HIDC_CHECK_m_bHokanKey_RETURN,	//��⌈��L�[�iEnter�j
	IDC_CHECK_m_bHokanKey_TAB,		HIDC_CHECK_m_bHokanKey_TAB,		//��⌈��L�[�iTab�j
	IDC_CHECK_m_bHokanKey_RIGHT,	HIDC_CHECK_m_bHokanKey_RIGHT,	//��⌈��L�[�i���j
//	IDC_CHECK_m_bHokanKey_SPACE,	HIDC_CHECK_m_bHokanKey_SPACE,	//��⌈��L�[�iSpace�j
	IDC_CHECK_HTMLHELPISSINGLE,		HIDC_CHECK_HTMLHELPISSINGLE,	//�r���[�A�̕����N��
	IDC_EDIT_EXTHELP1,				HIDC_EDIT_EXTHELP1,				//�O���w���v�t�@�C����
	IDC_EDIT_EXTHTMLHELP,			HIDC_EDIT_EXTHTMLHELP,			//�O��HTML�w���v�t�@�C����
	//	2007.02.04 genta �J�[�\���ʒu�̒P��̎��������͋��ʐݒ肩��O����
	//IDC_CHECK_CLICKKEYSEARCH,		HIDC_CHECK_CLICKKEYSEARCH,		//�L�����b�g�ʒu�̒P�����������	// 2006.03.24 fon
	IDC_BUTTON_KEYWORDHELPFONT,		HIDC_BUTTON_KEYWORDHELPFONT,	//�L�[���[�h�w���v�̃t�H���g
	IDC_EDIT_MIGEMO_DLL,			HIDC_EDIT_MIGEMO_DLL,			//Migemo DLL�t�@�C����	// 2006.08.06 ryoji
	IDC_BUTTON_OPENMDLL,			HIDC_BUTTON_OPENMDLL,			//Migemo DLL�t�@�C���Q��	// 2006.08.06 ryoji
	IDC_EDIT_MIGEMO_DICT,			HIDC_EDIT_MIGEMO_DICT,			//Migemo �����t�@�C����	// 2006.08.06 ryoji
	IDC_BUTTON_OPENMDICT,			HIDC_BUTTON_OPENMDICT,			//Migemo �����t�@�C���Q��	// 2006.08.06 ryoji
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
INT_PTR CALLBACK CPropHelper::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropHelper::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* Helper ���b�Z�[�W���� */
INT_PTR CPropHelper::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� Helper */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		/* �O���w���v�P */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHELP1 ), _MAX_PATH - 1 );
		/* �O��HTML�w���v */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHTMLHELP ), _MAX_PATH - 1 );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			/* �_�C�A���O�f�[�^�̎擾 Helper */
			GetData( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_OPENHELP1:	/* �O���w���v�P�́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH];
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
					// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					if( _IS_REL_PATH( m_Common.m_sHelper.m_szExtHelp ) ){
						GetInidirOrExedir( szPath, m_Common.m_sHelper.m_szExtHelp, TRUE );
					}else{
						_tcscpy( szPath, m_Common.m_sHelper.m_szExtHelp );
					}
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create(
						G_AppInstance(),
						hwndDlg,
						_T("*.hlp;*.chm;*.col"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Common.m_sHelper.m_szExtHelp, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_sHelper.m_szExtHelp );
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENEXTHTMLHELP:	/* �O��HTML�w���v�́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH];
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
					// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					if( _IS_REL_PATH( m_Common.m_sHelper.m_szExtHtmlHelp ) ){
						GetInidirOrExedir( szPath, m_Common.m_sHelper.m_szExtHtmlHelp, TRUE );
					}else{
						_tcscpy( szPath, m_Common.m_sHelper.m_szExtHtmlHelp );
					}
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create(
						G_AppInstance(),
						hwndDlg,
						_T("*.chm;*.col"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Common.m_sHelper.m_szExtHtmlHelp, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_sHelper.m_szExtHtmlHelp );
					}
				}
				return TRUE;
			// ai 02/05/21 Add S
			case IDC_BUTTON_KEYWORDHELPFONT:	/* �L�[���[�h�w���v�́u�t�H���g�v�{�^�� */
				{
					LOGFONT   lf = m_Common.m_sHelper.m_lf_kh;
					INT fontSize = m_Common.m_sHelper.m_ps_kh;

					if( MySelectFont( &lf, &fontSize, hwndDlg, false) ){
						m_Common.m_sHelper.m_lf_kh = lf;
						m_Common.m_sHelper.m_ps_kh = fontSize;	// 2009.10.01 ryoji
					}
				}
				return TRUE;
			// ai 02/05/21 Add E
			case IDC_BUTTON_OPENMDLL:	/* MIGEMODLL�ꏊ�w��u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH];
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
					// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					if( _IS_REL_PATH( m_Common.m_sHelper.m_szMigemoDll ) ){
						GetInidirOrExedir( szPath, m_Common.m_sHelper.m_szMigemoDll, TRUE );
					}else{
						_tcscpy( szPath, m_Common.m_sHelper.m_szMigemoDll );
					}
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create(
						G_AppInstance(),
						hwndDlg,
						_T("*.dll"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Common.m_sHelper.m_szMigemoDll, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DLL, m_Common.m_sHelper.m_szMigemoDll );
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENMDICT:	/* MigemoDict�ꏊ�w��u�Q��...�v�{�^�� */
				{
					TCHAR	szPath[_MAX_PATH];
					/* �����t�H���_ */
					// 2007.05.27 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					if( _IS_REL_PATH( m_Common.m_sHelper.m_szMigemoDict ) ){
						GetInidirOrExedir( szPath, m_Common.m_sHelper.m_szMigemoDict, TRUE );
					}else{
						_tcscpy( szPath, m_Common.m_sHelper.m_szMigemoDict );
					}
					if( SelectDir( hwndDlg, _T("��������t�H���_��I��ł�������"), szPath, szPath ) ){
						_tcscpy( m_Common.m_sHelper.m_szMigemoDict, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict );
					}
				}
				return TRUE;
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
				OnHelp( hwndDlg, IDD_PROP_HELPER );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE_A( "Helper PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 Helper */
				GetData( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_HELPER;
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
			MyWinHelp( (HWND)p->hItemHandle, m_pszHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_pszHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}

/* �_�C�A���O�f�[�^�̐ݒ� Helper */
void CPropHelper::SetData( HWND hwndDlg )
{
	/* �O���w���v�P */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_sHelper.m_szExtHelp );

	/* �O��HTML�w���v */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_sHelper.m_szExtHtmlHelp );

	/* HtmlHelp�r���[�A�͂ЂƂ� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HTMLHELPISSINGLE, m_Common.m_sHelper.m_bHtmlHelpIsSingle ? BST_CHECKED : BST_UNCHECKED );

	/* �⊮��⌈��L�[ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN, m_Common.m_sHelper.m_bHokanKey_RETURN );	//VK_RETURN �⊮����L�[���L��/����
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_TAB, m_Common.m_sHelper.m_bHokanKey_TAB );		//VK_TAB    �⊮����L�[���L��/����
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT, m_Common.m_sHelper.m_bHokanKey_RIGHT );	//VK_RIGHT  �⊮����L�[���L��/����

	//migemo dict 
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DLL, m_Common.m_sHelper.m_szMigemoDll);
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict);

	return;
}


/* �_�C�A���O�f�[�^�̎擾 Helper */
int CPropHelper::GetData( HWND hwndDlg )
{
	// Oct. 5, 2002 genta �T�C�Y�������@�ύX
	/* �O���w���v�P */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_sHelper.m_szExtHelp, _countof( m_Common.m_sHelper.m_szExtHelp ));

	/* �O��HTML�w���v */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_sHelper.m_szExtHtmlHelp, _countof( m_Common.m_sHelper.m_szExtHtmlHelp ));

	/* HtmlHelp�r���[�A�͂ЂƂ� */
	m_Common.m_sHelper.m_bHtmlHelpIsSingle = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HTMLHELPISSINGLE ) != 0;


	/* �⊮��⌈��L�[ */
	m_Common.m_sHelper.m_bHokanKey_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN );//VK_RETURN �⊮����L�[���L��/����
	m_Common.m_sHelper.m_bHokanKey_TAB = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_TAB );		//VK_TAB    �⊮����L�[���L��/����
	m_Common.m_sHelper.m_bHokanKey_RIGHT = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT );	//VK_RIGHT  �⊮����L�[���L��/����

	::DlgItem_GetText( hwndDlg, IDC_EDIT_MIGEMO_DLL, m_Common.m_sHelper.m_szMigemoDll, _countof( m_Common.m_sHelper.m_szMigemoDll ));
	::DlgItem_GetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict, _countof( m_Common.m_sHelper.m_szMigemoDict ));


	return TRUE;
}



