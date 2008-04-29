/*!	@file
	@brief �����_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/12/12 �č쐬
	@date 2001/06/23 N.Nakatani �P��P�ʂŌ�������@�\������
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, JEPRO, hor, Stonee
	Copyright (C) 2002, MIK, hor, YAZAKI, genta
	Copyright (C) 2005, zenryaku
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "dlg/CDlgFind.h"
#include "funccode.h"
#include "sakura_rc.h"
//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "CBregexp.h"
#include "view/CEditView.h"
#include "debug/Debug.h"// 2002/2/10 aroka �w�b�_����
#include "util/shell.h"

//���� CDlgFind.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//11800
	IDC_BUTTON_SEARCHNEXT,			HIDC_FIND_BUTTON_SEARCHNEXT,		//��������
	IDC_BUTTON_SEARCHPREV,			HIDC_FIND_BUTTON_SEARCHPREV,		//�O������
	IDCANCEL,						HIDCANCEL_FIND,						//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_FIND_BUTTON_HELP,				//�w���v
	IDC_CHK_WORD,					HIDC_FIND_CHK_WORD,					//�P��P��
	IDC_CHK_LOHICASE,				HIDC_FIND_CHK_LOHICASE,				//�啶��������
	IDC_CHK_REGULAREXP,				HIDC_FIND_CHK_REGULAREXP,			//���K�\��
	IDC_CHECK_NOTIFYNOTFOUND,		HIDC_FIND_CHECK_NOTIFYNOTFOUND,		//������Ȃ��Ƃ��ɒʒm
	IDC_CHECK_bAutoCloseDlgFind,	HIDC_FIND_CHECK_bAutoCloseDlgFind,	//�����I�ɕ���
	IDC_COMBO_TEXT,					HIDC_FIND_COMBO_TEXT,				//����������
	IDC_STATIC_JRE32VER,			HIDC_FIND_STATIC_JRE32VER,			//���K�\���o�[�W����
	IDC_BUTTON_SETMARK,				HIDC_FIND_BUTTON_SETMARK,			//2002.01.16 hor �����Y���s���}�[�N
	IDC_CHECK_SEARCHALL,			HIDC_FIND_CHECK_SEARCHALL,			//2002.01.26 hor �擪�i�����j����Č���
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgFind::CDlgFind()
{
	m_sSearchOption.Reset();
	m_szText[0] = L'\0';		/* ���������� */
	return;
}


/* ���[�h���X�_�C�A���O�̕\�� */
HWND CDlgFind::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	m_sSearchOption = m_pShareData->m_Common.m_sSearch.m_sSearchOption;		// �����I�v�V����
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	m_ptEscCaretPos_PHY = ((CEditView*)lParam)->GetCaret().GetCaretLogicPos();	// �����J�n���̃J�[�\���ʒu�ޔ�
	((CEditView*)lParam)->m_bSearch = TRUE;							// �����J�n�ʒu�̓o�^�L��		02/07/28 ai
	return CDialog::DoModeless( hInstance, hwndParent, IDD_FIND, lParam, SW_SHOW );
}

/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
void CDlgFind::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}



/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgFind::SetData( void )
{
//	MYTRACE_A( "CDlgFind::SetData()" );
	int		i;
	HWND	hwndCombo;

	/*****************************
	*           ������           *
	*****************************/
	// Here Jun. 26, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌������ɂ��jre.dll������폜

	/* ���[�U�[���R���{ �{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	::SendMessage( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT ), CB_LIMITTEXT, _MAX_PATH - 1, 0 );
	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, TRUE, 0 );


	/*****************************
	*         �f�[�^�ݒ�         *
	*****************************/
	/* ���������� */
	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT );
	::SendMessageAny( hwndCombo, CB_RESETCONTENT, 0, 0 );
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT, m_szText );
	for( i = 0; i < m_pShareData->m_aSearchKeys.size(); ++i ){
		Combo_AddString( hwndCombo, m_pShareData->m_aSearchKeys[i] );
	}

	/* �p�啶���Ɖp����������ʂ��� */
	::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, m_sSearchOption.bLoHiCase );

	// 2001/06/23 Norio Nakatani
	/* �P��P�ʂŌ��� */
	::CheckDlgButton( GetHwnd(), IDC_CHK_WORD, m_sSearchOption.bWordOnly );

	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND );

	// From Here Jun. 29, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if( CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, false )
		&& m_sSearchOption.bRegularExp){
		/* �p�啶���Ɖp����������ʂ��� */
		::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 1 );
//���K�\����ON�ł��A�啶������������ʂ���^���Ȃ���I���ł���悤�ɁB
//		::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 1 );
//		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_LOHICASE ), FALSE );

		// 2001/06/23 N.Nakatani
		/* �P��P�ʂŒT�� */
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), FALSE );
	}
	else {
		::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 0 );
	}
	// To Here Jun. 29, 2001 genta

	/* �����_�C�A���O�������I�ɕ��� */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_bAutoCloseDlgFind, m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind );

	/* �擪�i�����j����Č��� 2002.01.26 hor */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_sSearch.m_bSearchAll );

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
int CDlgFind::GetData( void )
{
//	MYTRACE_A( "CDlgFind::GetData()" );
//	int			i;
//	int			j;
//	CMemory*	pcmWork;
	//

	/* �p�啶���Ɖp����������ʂ��� */
	m_sSearchOption.bLoHiCase = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_LOHICASE ));

	// 2001/06/23 Norio Nakatani
	/* �P��P�ʂŌ��� */
	m_sSearchOption.bWordOnly = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_WORD ));

	/* ��v����P��̂݌������� */
	/* ���K�\�� */
	m_sSearchOption.bRegularExp = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ));

	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND );

	m_pShareData->m_Common.m_sSearch.m_sSearchOption = m_sSearchOption;		// �����I�v�V����
	m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��

	/* ���������� */
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_TEXT, m_szText, _countof( m_szText ));

	/* �����_�C�A���O�������I�ɕ��� */
	m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_bAutoCloseDlgFind );

	/* �擪�i�����j����Č��� 2002.01.26 hor */
	m_pShareData->m_Common.m_sSearch.m_bSearchAll = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_SEARCHALL );

	if( 0 < wcslen( m_szText ) ){
		/* ���K�\���H */
		// From Here Jun. 26, 2001 genta
		//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
		int nFlag = 0x00;
		nFlag |= m_sSearchOption.bLoHiCase ? 0x01 : 0x00;
		if( m_sSearchOption.bRegularExp && !CheckRegexpSyntax( m_szText, GetHwnd(), true, nFlag ) ){
			return -1;
		}
		// To Here Jun. 26, 2001 genta ���K�\�����C�u���������ւ�

		/* ���������� */
		//@@@ 2002.2.2 YAZAKI CShareData�Ɉړ�
		CShareData::getInstance()->AddToSearchKeyArr( m_szText );
		if( !m_bModal ){
			/* �_�C�A���O�f�[�^�̐ݒ� */
			SetData();
		}
		return 1;
	}else{
		return 0;
	}
}



BOOL CDlgFind::OnBnClicked( int wID )
{
	int			nRet;
	CEditView*	pcEditView = (CEditView*)m_lParam;
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�����v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_SEARCH_DIALOG) );	//Apr. 5, 2001 JEPRO �C���R���ǉ�	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		break;
	case IDC_CHK_REGULAREXP:	/* ���K�\�� */
//		MYTRACE_A( "IDC_CHK_REGULAREXP ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) = %d\n", ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) ){

			// From Here Jun. 26, 2001 genta
			//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
			if( !CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 0 );
			}else{
			// To Here Jun. 26, 2001 genta

				/* �p�啶���Ɖp����������ʂ��� */
				//	Jan. 31, 2002 genta
				//	�啶���E�������̋�ʂ͐��K�\���̐ݒ�Ɋւ�炸�ۑ�����
				//::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 1 );
				//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_LOHICASE ), FALSE );

				// 2001/06/23 Norio Nakatani
				/* �P��P�ʂŌ��� */
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), FALSE );
			}
		}else{
			/* �p�啶���Ɖp����������ʂ��� */
			//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_LOHICASE ), TRUE );
			//	Jan. 31, 2002 genta
			//	�啶���E�������̋�ʂ͐��K�\���̐ݒ�Ɋւ�炸�ۑ�����
			//::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 0 );

			// 2001/06/23 Norio Nakatani
			/* �P��P�ʂŌ��� */
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), TRUE );
		}
		break;
	case IDC_BUTTON_SEARCHPREV:	/* �㌟�� */	//Feb. 13, 2001 JEPRO �{�^������[IDC_BUTTON1]��[IDC_BUTTON_SERACHPREV]�ɕύX
		/* �_�C�A���O�f�[�^�̎擾 */
		nRet = GetData();
		if( 0 < nRet ){
			if( m_bModal ){		/* ���[�_���_�C�A���O�� */
				CloseDialog( 1 );
			}else{

				/* �O������ */
				pcEditView->GetCommander().HandleCommand( F_SEARCH_PREV, TRUE, (LPARAM)GetHwnd(), 0, 0, 0 );
				/* �ĕ`�� 2005.04.06 zenryaku 0�������}�b�`�ŃL�����b�g��\�����邽�� */
				pcEditView->GetCommander().HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );

				// 02/06/26 ai Start
				// �����J�n�ʒu��o�^
				if( TRUE == pcEditView->m_bSearch ){
					// �����J�n���̃J�[�\���ʒu�o�^�����ύX 02/07/28 ai start
					pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
					/*
					pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
					pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
					*/
					pcEditView->m_bSearch = FALSE;
					// 02/07/28 ai end
				}//  02/06/26 ai End

				/* �����_�C�A���O�������I�ɕ��� */
				if( m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
			}
		}
		else{
			OkMessage( GetHwnd(), _T("�����������w�肵�Ă��������B") );
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:		/* ������ */	//Feb. 13, 2001 JEPRO �{�^������[IDOK]��[IDC_BUTTON_SERACHNEXT]�ɕύX
		/* �_�C�A���O�f�[�^�̎擾 */
		nRet = GetData();
		if( 0 < nRet ){
			if( m_bModal ){		/* ���[�_���_�C�A���O�� */
				CloseDialog( 2 );
			}
			else{
				/* �������� */
				pcEditView->GetCommander().HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)GetHwnd(), 0, 0, 0 );
			
				/* �ĕ`�� 2005.04.06 zenryaku 0�������}�b�`�ŃL�����b�g��\�����邽�� */
				pcEditView->GetCommander().HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );

				// �����J�n�ʒu��o�^
				if( TRUE == pcEditView->m_bSearch ){
					// �����J�n���̃J�[�\���ʒu�o�^�����ύX 02/07/28 ai start
					pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
					pcEditView->m_bSearch = FALSE;
				}

				/* �����_�C�A���O�������I�ɕ��� */
				if( m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
				// 2001.12.03 hor
				//	�_�C�A���O����Ȃ��Ƃ��AIDC_COMBO_TEXT ��� Enter �����ꍇ��
				//	�L�����b�g���\������Ȃ��Ȃ�̂��������
				else{
					::SendMessageAny(GetHwnd(),WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(GetHwnd(),IDC_COMBO_TEXT ),TRUE);
				}
			}
		}
		else{
			OkMessage( GetHwnd(), _T("�����������w�肵�Ă��������B") );
		}
		return TRUE;
	case IDC_BUTTON_SETMARK:	//2002.01.16 hor �Y���s�}�[�N
		if( 0 < GetData() ){
			if( m_bModal ){		/* ���[�_���_�C�A���O�� */
				CloseDialog( 2 );
			}else{
				pcEditView->GetCommander().HandleCommand( F_BOOKMARK_PATTERN, FALSE, 0, 0, 0, 0 );
				/* �����_�C�A���O�������I�ɕ��� */
				if( m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
				else{
					::SendMessageAny(GetHwnd(),WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(GetHwnd(),IDC_COMBO_TEXT ),TRUE);
				}
			}
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}



//@@@ 2002.01.18 add start
LPVOID CDlgFind::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


