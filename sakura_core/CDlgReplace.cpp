//	$Id$
/*!	@file
	�u���_�C�A���O
	
	@author Norio Nakatani
	@date 2001/06/23 N.Nakatani�@�P��P�ʂŌ�������@�\������
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgReplace.h"
#include "debug.h"
#include "CEditView.h"
#include "etc_uty.h"
#include "global.h"
#include "CWaitCursor.h"
#include "funccode.h"		// Stonee, 2001/03/12

CDlgReplace::CDlgReplace()
{
	m_bLoHiCase = FALSE;		/* �p�啶���Ɖp����������ʂ��� */
	m_bWordOnly = FALSE;		/* ��v����P��̂݌������� */
	m_bRegularExp = FALSE;		/* ���K�\�� */
	m_bSelectedArea = FALSE;	/* �I��͈͓��u�� */
	m_szText[0] = '\0';			/* ���������� */
	m_szText2[0] = '\0';		/* �u���㕶���� */
	return;
}

/* ���[�h���X�_�C�A���O�̕\�� */
HWND CDlgReplace::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam, BOOL bSelected )
{
	m_bRegularExp = m_pShareData->m_Common.m_bRegularExp;			/* 1==���K�\�� */
	m_bLoHiCase = m_pShareData->m_Common.m_bLoHiCase;				/* 1==�p�啶���������̋�� */
	m_bWordOnly = m_pShareData->m_Common.m_bWordOnly;				/* 1==�P��̂݌��� */
	m_bSelectedArea = m_pShareData->m_Common.m_bSelectedArea;		/* �I��͈͓��u�� */
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_bNOTIFYNOTFOUND;	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	m_bSelected = bSelected;
	return CDialog::DoModeless( hInstance, hwndParent, IDD_REPLACE, lParam, SW_SHOW );
}

/* ���[�h���X���F�u���E�����ΏۂƂȂ�r���[�̕ύX */
void CDlgReplace::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}




/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgReplace::SetData( void )
{
	int		i;
	HWND	hwndCombo;
//	m_hWnd = hwndDlg;	/* ���̃_�C�A���O�̃n���h�� */

	/* ���������� */
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT );
	for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[i] );
	}

	/* �u���㕶���� */
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT2, m_szText2 );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 );
	for( i = 0; i < m_pShareData->m_nREPLACEKEYArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szREPLACEKEYArr[i] );
	}

	/* �p�啶���Ɖp����������ʂ��� */
	::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, m_bLoHiCase );
	
	// 2001/06/23 N.Nakatani 
	/* �P��P�ʂŒT�� */
	::CheckDlgButton( m_hWnd, IDC_CHK_WORD, m_bWordOnly );

	/* �I��͈͓��u�� */
	if( m_pShareData->m_Common.m_bSelectedArea ){
		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, TRUE );
	}else{
		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, TRUE );
	}

	// From Here Jun. 29, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if( CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, false )
		&& m_bRegularExp){
		/* �p�啶���Ɖp����������ʂ��� */
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 1 );
		::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

		// 2001/06/23 N.Nakatani 
		/* �P��P�ʂŒT�� */
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );
	}
	else {
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
	}
	// To Here Jun. 29, 2001 genta

	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND );
	
	
	/* �u�� �_�C�A���O�������I�ɕ��� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgReplace, m_pShareData->m_Common.m_bAutoCloseDlgReplace );
	
	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
/* 0==����������  0���傫��==����   0��菬����==���̓G���[ */
int CDlgReplace::GetData( void )
{
	int			i;
	int			j;
	CMemory*	pcmWork;

	/* �p�啶���Ɖp����������ʂ��� */
	m_bLoHiCase = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_LOHICASE );

	// 2001/06/23 N.Nakatani 
	/* �P��P�ʂŒT�� */
	m_bWordOnly = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_WORD );

	/* ���K�\�� */
	m_bRegularExp = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP );
	/* �I��͈͓��u�� */
	m_bSelectedArea = ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_SELECTEDAREA );
	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND );

	m_pShareData->m_Common.m_bRegularExp = m_bRegularExp;			/* 1==���K�\�� */
	m_pShareData->m_Common.m_bLoHiCase = m_bLoHiCase;				/* 1==�p�啶���������̋�� */
	m_pShareData->m_Common.m_bWordOnly = m_bWordOnly;				/* 1==�P��̂݌��� */
	m_pShareData->m_Common.m_bSelectedArea = m_bSelectedArea;		/* �I��͈͓��u�� */
	m_pShareData->m_Common.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */

	/* ���������� */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText, _MAX_PATH - 1 );
	/* �u���㕶���� */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT2, m_szText2, _MAX_PATH - 1 );

	/* �u�� �_�C�A���O�������I�ɕ��� */
	m_pShareData->m_Common.m_bAutoCloseDlgReplace = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgReplace );

	if( 0 < lstrlen( m_szText ) ){
		/* ���K�\���H */
		// From Here Jun. 26, 2001 genta
		//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
		if( m_bRegularExp && !CheckRegexpSyntax( m_szText, m_hWnd, true )){
			return -1;
		}
		// To Here Jun. 26, 2001 genta ���K�\�����C�u���������ւ�

		/* ���������� */
		pcmWork = new CMemory( m_szText, lstrlen( m_szText ) );
		for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
			if( 0 == strcmp( m_szText, m_pShareData->m_szSEARCHKEYArr[i] ) ){
				break;
			}
		}
		if( i < m_pShareData->m_nSEARCHKEYArrNum ){
			for( j = i; j > 0; j-- ){
				strcpy( m_pShareData->m_szSEARCHKEYArr[j], m_pShareData->m_szSEARCHKEYArr[j - 1] );
			}
		}else{
			for( j = MAX_SEARCHKEY - 1; j > 0; j-- ){
				strcpy( m_pShareData->m_szSEARCHKEYArr[j], m_pShareData->m_szSEARCHKEYArr[j - 1] );
			}
			++m_pShareData->m_nSEARCHKEYArrNum;
			if( m_pShareData->m_nSEARCHKEYArrNum > MAX_SEARCHKEY ){
				m_pShareData->m_nSEARCHKEYArrNum = MAX_SEARCHKEY;
			}
		}
		strcpy( m_pShareData->m_szSEARCHKEYArr[0], pcmWork->GetPtr( NULL ) );
		delete pcmWork;

		/* �u���㕶���� */
		pcmWork = new CMemory( m_szText2, lstrlen( m_szText2 ) );
		for( i = 0; i < m_pShareData->m_nREPLACEKEYArrNum; ++i ){
			if( 0 == strcmp( m_szText2, m_pShareData->m_szREPLACEKEYArr[i] ) ){
				break;
			}
		}
		if( i < m_pShareData->m_nREPLACEKEYArrNum ){
			for( j = i; j > 0; j-- ){
				strcpy( m_pShareData->m_szREPLACEKEYArr[j], m_pShareData->m_szREPLACEKEYArr[j - 1] );
			}
		}else{
			for( j = MAX_REPLACEKEY - 1; j > 0; j-- ){
				strcpy( m_pShareData->m_szREPLACEKEYArr[j], m_pShareData->m_szREPLACEKEYArr[j - 1] );
			}
			++m_pShareData->m_nREPLACEKEYArrNum;
			if( m_pShareData->m_nREPLACEKEYArrNum > MAX_REPLACEKEY ){
				m_pShareData->m_nREPLACEKEYArrNum = MAX_REPLACEKEY;
			}
		}
		strcpy( m_pShareData->m_szREPLACEKEYArr[0], pcmWork->GetPtr( NULL ) );
		delete pcmWork;


		return 1;
	}else{
		return 0;
	}
}




BOOL CDlgReplace::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;
	//	Jun. 26, 2001 genta
	//	���̈ʒu�Ő��K�\���̏�����������K�v�͂Ȃ�
	//	���Ƃ̈�ѐ���ۂ��ߍ폜

	/* ���[�U�[���R���{ �{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );

	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );


	/* �e�L�X�g�I�𒆂� */
	if( m_bSelected ){
//		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHPREV ), FALSE );
//		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHNEXT ), FALSE );
//		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_REPALCE ), FALSE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, TRUE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_SELECTEDAREA ), FALSE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, FALSE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, TRUE );
	}
	/* ���N���X�����o */
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );

}




BOOL CDlgReplace::OnBnClicked( int wID )
{
	CEditView*	pcEditView = (CEditView*)m_lParam;
	int			nNewPos;
	int			nReplaceNum;
	char		szLabel[64];
	int			nAllLineNum;
	HWND		hwndProgress;
	HWND		hwndStatic;
//	MSG			msg;
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�u���v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_REPLACE) );
		return TRUE;
//	case IDC_CHK_LOHICASE:	/* �啶���Ə���������ʂ��� */
//		MYTRACE( "IDC_CHK_LOHICASE\n" );
//		return TRUE;
//	case IDC_CHK_WORDONLY:	/* ��v����P��̂݌��� */
//		MYTRACE( "IDC_CHK_WORDONLY\n" );
//		break;
	case IDC_CHK_REGULAREXP:	/* ���K�\�� */
//		MYTRACE( "IDC_CHK_REGULAREXP ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) = %d\n", ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) ){
			// From Here Jun. 26, 2001 genta
			//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
			if( !CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, true )){
				::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
			}else{
			// To Here Jun. 26, 2001 genta

				/* �p�啶���Ɖp����������ʂ��� */
				::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

				// 2001/06/23 N.Nakatani 
				/* �P��P�ʂŒT�� */
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );

			}
		}else{
			/* �p�啶���Ɖp����������ʂ��� */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
			::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 0 );

			// 2001/06/23 N.Nakatani 
			/* �P��P�ʂŒT�� */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), TRUE );

		}
		return TRUE;;
//	case IDOK:			/* ������ */
//		/* �_�C�A���O�f�[�^�̎擾 */
//		nRet = GetData();
//		if( 0 < nRet ){
//			::EndDialog( hwndDlg, 2 );
//		}else
//		if( 0 == nRet ){
//			::EndDialog( hwndDlg, 0 );
//		}
//		return TRUE;


	case IDC_BUTTON_SEARCHPREV:
		if( 0 < GetData() ){
			/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
			/* �O������ */
			pcEditView->HandleCommand( F_SEARCH_PREV, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
			/* �ĕ`�� */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"��������w�肵�Ă��������B"
			);
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:
		if( 0 < GetData() ){
			/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
			/* �������� */
			pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
			/* �ĕ`�� */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"��������w�肵�Ă��������B"
			);
		}
		return TRUE;

	case IDC_BUTTON_REPALCE:	/* �u�� */
		if( 0 < GetData() ){
			/* �J�[�\�����ړ� */
			pcEditView->HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );

			/* �e�L�X�g�I������ */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			pcEditView->DisableSelectArea( TRUE );

			/* �������� */
			pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );

			/* �e�L�X�g���I������Ă��邩 */
			if( pcEditView->IsTextSelected() ){
				/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
				/* �e�L�X�g��\��t�� */
				pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)m_szText2, FALSE, 0, 0 );
				/* �������� */
				pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, (LPARAM)"�Ō�܂Œu�����܂����B", 0, 0 );
			}
			/* �ĕ`�� */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"��������w�肵�Ă��������B"
			);
		}
		return TRUE;
	case IDC_BUTTON_REPALCEALL:
		if( 0 < GetData() ){


			/* �\������ON/OFF */
			BOOL bDisplayUpdate = FALSE;


			pcEditView->m_bDrawSWITCH = bDisplayUpdate;
			CDlgCancel	cDlgCancel;
			HWND		hwndCancel;
			nAllLineNum = pcEditView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();

			/* �i���\��&���~�_�C�A���O�̍쐬 */
			hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hWnd, IDD_REPLACERUNNING );
			::EnableWindow( m_hWnd, FALSE );
			::EnableWindow( ::GetParent( m_hWnd ), FALSE );
			::EnableWindow( ::GetParent( ::GetParent( m_hWnd ) ), FALSE );

			/* �v���O���X�o�[������ */
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS_REPLACE );
			::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
			nNewPos = 0;
 			::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );

			/* �u���������� */
			nReplaceNum = 0;
			hwndStatic = ::GetDlgItem( hwndCancel, IDC_STATIC_KENSUU );
//			wsprintf( szLabel, "%d", nReplaceNum );
			_itoa( nReplaceNum, szLabel, 10 );
			::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );
//			::SetWindowText( hwndStatic, szLabel );

			/* �t�@�C���̐擪�Ɉړ� */
			pcEditView->HandleCommand( F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0 );
			/* �e�L�X�g�I������ */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			pcEditView->DisableSelectArea( FALSE );
			/* �������� */
			pcEditView->HandleCommand( F_SEARCH_NEXT, bDisplayUpdate, 0, 0, 0, 0 );
			/* �e�L�X�g���I������Ă��邩 */
			while( pcEditView->IsTextSelected() ){
				/* �L�����Z�����ꂽ�� */
//				if( cDlgCancel.IsCanceled() ){
				if( cDlgCancel.m_bCANCEL ){
					break;
				}
//				if( 0 == ( nReplaceNum % 8 ) ){
					/* �������̃��[�U�[������\�ɂ��� */
					if( !::BlockingHook( hwndCancel ) ){
						return -1;
					}
//					if( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ){
//						if( msg.message == WM_QUIT ){
//							return -1;
//						}
//						if( !IsDialogMessage (hwndCancel, &msg ) ){
//							::TranslateMessage( &msg );
//							::DispatchMessage( &msg );
//						}
//					}
//				}
				if( 0 == ( nReplaceNum % 8 ) ){


//					if( 0 < nAllLineNum ){
						nNewPos = (pcEditView->m_nSelectLineFrom * 100) / nAllLineNum;
						::PostMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
//					}
					_itoa( nReplaceNum, szLabel, 10 );
					::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)(const char*)szLabel );
				}

//#ifdef _DEBUG
//				{
//					CRunningTimer* pcRunningTimer = new CRunningTimer( (const char*)"F_INSTEXT" );
//					gm_ProfileOutput = FALSE;
//#endif
					/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
					/* �e�L�X�g��\��t�� */
					pcEditView->HandleCommand( F_INSTEXT, bDisplayUpdate, (LPARAM)m_szText2, TRUE, 0, 0 );
//#ifdef _DEBUG
//					gm_ProfileOutput = TRUE;
//					delete pcRunningTimer;
//					gm_ProfileOutput = FALSE;
//				}
//#endif
				++nReplaceNum;

//#ifdef _DEBUG
//				{
//					CRunningTimer* pcRunningTimer = new CRunningTimer( (const char*)"F_SEARCH_NEXT" );
//					gm_ProfileOutput = FALSE;
//#endif
				
					/* �������� */
					pcEditView->HandleCommand( F_SEARCH_NEXT, bDisplayUpdate, 0, 0, 0, 0 );
//#ifdef _DEBUG
//					gm_ProfileOutput = TRUE;
//					delete pcRunningTimer;
//					gm_ProfileOutput = FALSE;
//				}
//#endif
			}
			if( 0 < nAllLineNum ){
				nNewPos = (pcEditView->m_nSelectLineFrom * 100) / nAllLineNum;
				::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
			}
			_itoa( nReplaceNum, szLabel, 10 );
			::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

			//�ĕ`��
			pcEditView->m_bDrawSWITCH = TRUE;
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
			

			if( !cDlgCancel.IsCanceled() ){
				nNewPos = 100;
				::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
			}
//			cDlgCancel.Close();
			cDlgCancel.CloseDialog( 0 );
			::EnableWindow( m_hWnd, TRUE );
			::EnableWindow( ::GetParent( m_hWnd ), TRUE );
			::EnableWindow( ::GetParent( ::GetParent( m_hWnd ) ), TRUE );


			/* �ĕ`�� */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );

			/* �A�N�e�B�u�ɂ��� */
			ActivateFrameWindow( m_hWnd );

			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_TOPMOST, GSTR_APPNAME,
				"%d�ӏ���u�����܂����B", nReplaceNum
			);

//			nNewPos = 100;
// 			::SendMessage( ::GetDlgItem( m_hWnd, IDC_PROGRESS_REPLACE ), PBM_SETPOS, nNewPos, 0 );

//			::ShowWindow( ::GetDlgItem( m_hWnd, IDC_PROGRESS_REPLACE ), SW_HIDE );

			if( !cDlgCancel.IsCanceled() ){
				if( m_bModal ){		/* ���[�_���_�C�A���O�� */
					/* �u���_�C�A���O����� */
					::EndDialog( m_hWnd, 0 );
				}else{
					/* �u�� �_�C�A���O�������I�ɕ��� */
					if( m_pShareData->m_Common.m_bAutoCloseDlgReplace ){
						::DestroyWindow( m_hWnd );
					}
				}
			}


			return TRUE;
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"�u���������w�肵�Ă��������B"
			);
		}
		return TRUE;
//	case IDCANCEL:
//		::EndDialog( hwndDlg, 0 );
//		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}


/*[EOF]*/
