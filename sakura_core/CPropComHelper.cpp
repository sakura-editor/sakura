//	$Id$
/************************************************************************

	CPropComHelp.cpp

	���ʐݒ�F�x��
	Copyright (C) 1998-2000, Norio Nakatani


************************************************************************/
#include "sakura_rc.h"
#include "CPropCommon.h"
#include "debug.h"
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "etc_uty.h"
#include "CDlgInput1.h"
#include "global.h"



/* p10 ���b�Z�[�W���� */
BOOL CPropCommon::DispatchEvent_p10(
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

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p10 */
		SetData_p10( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		/* ���͕⊮�@�P��t�@�C�� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_HOKANFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );
		/* �L�[���[�h�w���v�@�����t�@�C�� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		/* �O���w���v�P */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHELP1 ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );
		/* �O��HTML�w���v */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHTMLHELP ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );


		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID         = LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl     = (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			/* �_�C�A���O�f�[�^�̎擾 p10 */
			GetData_p10( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_HOKANFILE_REF:	/* ���͕⊮�@�P��t�@�C���́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					char*			pszMRU = NULL;;
					char*			pszOPENFOLDER = NULL;;
					char			szPath[_MAX_PATH + 1];
					strcpy( szPath, m_Common.m_szHokanFile );
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create( 
						m_hInstance, 
						hwndDlg, 
						"*.*", 
						m_Common.m_szHokanFile, 
						(const char **)&pszMRU, 
						(const char **)&pszOPENFOLDER
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Common.m_szHokanFile, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Common.m_szHokanFile );
					}
				}
				return TRUE;

			//	From Here Sept. 12, 2000 JEPRO	
			case IDC_CHECK_USEKEYWORDHELP:	/* �L�[���[�h�w���v�@�\���g�������������t�@�C���w��ƎQ�ƃ{�^����Enable�ɂ��� */
				::CheckDlgButton( hwndDlg, IDC_CHECK_USEKEYWORDHELP, m_Common.m_bUseKeyWordHelp );
				if( BST_CHECKED == m_Common.m_bUseKeyWordHelp ){
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), TRUE );
				}else{
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), FALSE );
				}
				return TRUE;
			//	To Here Sept. 12, 2000
		
			case IDC_BUTTON_KEYWORDHELPFILE_REF:	/* �L�[���[�h�w���v�@�����t�@�C���́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					char*			pszMRU = NULL;;
					char*			pszOPENFOLDER = NULL;;
					char			szPath[_MAX_PATH + 1];
					strcpy( szPath, m_Common.m_szKeyWordHelpFile );
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create( 
						m_hInstance, 
						hwndDlg, 
						"*.*", 
						m_Common.m_szKeyWordHelpFile, 
						(const char **)&pszMRU, 
						(const char **)&pszOPENFOLDER
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Common.m_szKeyWordHelpFile, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_KEYWORDHELPFILE, m_Common.m_szKeyWordHelpFile );
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENHELP1:	/* �O���w���v�P�́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					char*			pszMRU = NULL;;
					char*			pszOPENFOLDER = NULL;;
					char			szPath[_MAX_PATH + 1];
					strcpy( szPath, m_Common.m_szExtHelp1 );
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create( 
						m_hInstance, 
						hwndDlg, 
						"*.hlp", 
						m_Common.m_szExtHelp1, 
						(const char **)&pszMRU, 
						(const char **)&pszOPENFOLDER
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Common.m_szExtHelp1, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1 );
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENEXTHTMLHELP:	/* �O��HTML�w���v�́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					char*			pszMRU = NULL;;
					char*			pszOPENFOLDER = NULL;;
					char			szPath[_MAX_PATH + 1];
					strcpy( szPath, m_Common.m_szExtHtmlHelp );
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create( 
						m_hInstance, 
						hwndDlg, 
						"*.chm;*.col", 
						m_Common.m_szExtHtmlHelp, 
						(const char **)&pszMRU, 
						(const char **)&pszOPENFOLDER
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Common.m_szExtHtmlHelp, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp );
					}
				}
				return TRUE;
			}
		}
		break;
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
//				MYTRACE( "p10 PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 p10 */
				GetData_p10( hwndDlg );
				return TRUE;
			}
			break;
//		}

//		MYTRACE( "pNMHDR->hwndFrom=%xh\n", pNMHDR->hwndFrom );
//		MYTRACE( "pNMHDR->idFrom  =%xh\n", pNMHDR->idFrom   );
//		MYTRACE( "pNMHDR->code    =%xh\n", pNMHDR->code     );
//		MYTRACE( "pMNUD->iPos    =%d\n", pMNUD->iPos      );
//		MYTRACE( "pMNUD->iDelta  =%d\n", pMNUD->iDelta    );
		break;
	}
	return FALSE;
}

/* �_�C�A���O�f�[�^�̐ݒ� p10 */
void CPropCommon::SetData_p10( HWND hwndDlg )
{
	/*  ���͕⊮�@�\���g�p���� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_USEHOKAN, m_Common.m_bUseHokan );

	/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANLOHICASE, m_Common.m_bHokanLoHiCase );

	/* ���͕⊮ �P��t�@�C�� */
	::SetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Common.m_szHokanFile );

	/* �L�[���[�h�w���v���g�p����  */
	::CheckDlgButton( hwndDlg, IDC_CHECK_USEKEYWORDHELP, m_Common.m_bUseKeyWordHelp );
//	From Here Sept. 12, 2000 JEPRO �L�[���[�h�w���v�@�\���g�������������t�@�C���w��ƎQ�ƃ{�^����Enable�ɂ���
	if( BST_CHECKED == m_Common.m_bUseKeyWordHelp ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), FALSE );
	}
//	To Here Sept. 12, 2000

	/* �L�[���[�h�w���v�@�����t�@�C�� */
	::SetDlgItemText( hwndDlg, IDC_EDIT_KEYWORDHELPFILE, m_Common.m_szKeyWordHelpFile );

	/* �O���w���v�P */
	::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1 );

	/* �O��HTML�w���v */
	::SetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp );

	/* HtmlHelp�r���[�A�͂ЂƂ� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HTMLHELPISSINGLE, m_Common.m_bHtmlHelpIsSingle );

	/* �⊮��⌈��L�[ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN, m_Common.m_bHokanKey_RETURN );//VK_RETURN �⊮����L�[���L��/����
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_TAB, m_Common.m_bHokanKey_TAB );//VK_TAB    �⊮����L�[���L��/����
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT, m_Common.m_bHokanKey_RIGHT );//VK_RIGHT  �⊮����L�[���L��/����
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_SPACE, m_Common.m_bHokanKey_SPACE );//VK_SPACE  �⊮����L�[���L��/����

	return;
}


/* �_�C�A���O�f�[�^�̎擾 p10 */
int CPropCommon::GetData_p10( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_HELPER;

	/*  ���͕⊮�@�\���g�p���� */
	m_Common.m_bUseHokan = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USEHOKAN );

	/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
	m_Common.m_bHokanLoHiCase = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANLOHICASE );

	/* ���͕⊮ �P��t�@�C�� */
	::GetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Common.m_szHokanFile, MAX_PATH - 1 );

	
	/* �L�[���[�h�w���v���g�p����  */
	m_Common.m_bUseKeyWordHelp = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USEKEYWORDHELP );
	
	/* �L�[���[�h�w���v�@�����t�@�C�� */
	::GetDlgItemText( hwndDlg, IDC_EDIT_KEYWORDHELPFILE, m_Common.m_szKeyWordHelpFile, MAX_PATH - 1 );

	/* �O���w���v�P */
	::GetDlgItemText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_szExtHelp1, MAX_PATH - 1 );

	/* �O��HTML�w���v */
	::GetDlgItemText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_szExtHtmlHelp, MAX_PATH - 1 );

	/* HtmlHelp�r���[�A�͂ЂƂ� */
	m_Common.m_bHtmlHelpIsSingle = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HTMLHELPISSINGLE );

	
	/* �⊮��⌈��L�[ */
	m_Common.m_bHokanKey_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN );//VK_RETURN �⊮����L�[���L��/����
	m_Common.m_bHokanKey_TAB = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_TAB );//VK_TAB    �⊮����L�[���L��/����
	m_Common.m_bHokanKey_RIGHT = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT );//VK_RIGHT  �⊮����L�[���L��/����
	m_Common.m_bHokanKey_SPACE = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_SPACE );//VK_SPACE  �⊮����L�[���L��/����

	return TRUE;
}
