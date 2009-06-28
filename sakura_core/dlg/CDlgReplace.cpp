/*!	@file
	@brief �u���_�C�A���O

	@author Norio Nakatani
	@date 2001/06/23 N.Nakatani �P��P�ʂŌ�������@�\������
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, Stonee, hor, YAZAKI
	Copyright (C) 2002, MIK, hor, novice, genta, aroka, YAZAKI
	Copyright (C) 2006, �����, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "sakura_rc.h"
#include "dlg/CDlgReplace.h"
#include "debug/Debug.h"
#include "view/CEditView.h"
#include "global.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "doc/CLayout.h"/// 2002/2/3 aroka
#include "doc/CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "util/shell.h"
#include "util/window.h"

//�u�� CDlgReplace.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//11900
	IDC_BUTTON_SEARCHNEXT,			HIDC_REP_BUTTON_SEARCHNEXT,			//������
	IDC_BUTTON_SEARCHPREV,			HIDC_REP_BUTTON_SEARCHPREV,			//�㌟��
	IDC_BUTTON_REPALCE,				HIDC_REP_BUTTON_REPALCE,			//�u��
	IDC_BUTTON_REPALCEALL,			HIDC_REP_BUTTON_REPALCEALL,			//�S�u��
	IDCANCEL,						HIDCANCEL_REP,						//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_REP_BUTTON_HELP,				//�w���v
	IDC_CHK_PASTE,					HIDC_REP_CHK_PASTE,					//�N���b�v�{�[�h����\��t��
	IDC_CHK_WORD,					HIDC_REP_CHK_WORD,					//�P��P��
	IDC_CHK_LOHICASE,				HIDC_REP_CHK_LOHICASE,				//�啶��������
	IDC_CHK_REGULAREXP,				HIDC_REP_CHK_REGULAREXP,			//���K�\��
	IDC_CHECK_NOTIFYNOTFOUND,		HIDC_REP_CHECK_NOTIFYNOTFOUND,		//������Ȃ��Ƃ��ɒʒm
	IDC_CHECK_bAutoCloseDlgReplace,	HIDC_REP_CHECK_bAutoCloseDlgReplace,	//�����I�ɕ���
	IDC_COMBO_TEXT,					HIDC_REP_COMBO_TEXT,				//�u���O
	IDC_COMBO_TEXT2,				HIDC_REP_COMBO_TEXT2,				//�u����
	IDC_RADIO_REPLACE,				HIDC_REP_RADIO_REPLACE,				//�u���ΏہF�u��
	IDC_RADIO_INSERT,				HIDC_REP_RADIO_INSERT,				//�u���ΏہF�}��
	IDC_RADIO_ADD,					HIDC_REP_RADIO_ADD,					//�u���ΏہF�ǉ�
	IDC_RADIO_SELECTEDAREA,			HIDC_REP_RADIO_SELECTEDAREA,		//�͈́F�S��
	IDC_RADIO_ALLAREA,				HIDC_REP_RADIO_ALLAREA,				//�͈́F�I��͈�
	IDC_STATIC_JRE32VER,			HIDC_REP_STATIC_JRE32VER,			//���K�\���o�[�W����
	IDC_BUTTON_SETMARK,				HIDC_REP_BUTTON_SETMARK,			//2002.01.16 hor �����Y���s���}�[�N
	IDC_CHECK_SEARCHALL,			HIDC_REP_CHECK_SEARCHALL,			//2002.01.26 hor �擪�i�����j����Č���
	IDC_CHECK_CONSECUTIVEALL,		HIDC_REP_CHECK_CONSECUTIVEALL,		//�u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgReplace::CDlgReplace()
{
	m_sSearchOption.Reset();	// �����I�v�V����
	/*
	m_bLoHiCase = FALSE;		// �p�啶���Ɖp����������ʂ���
	m_bWordOnly = FALSE;		// ��v����P��̂݌�������
	m_bRegularExp = FALSE;		// ���K�\��
	*/
	m_bConsecutiveAll = FALSE;	// �u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	m_bSelectedArea = FALSE;	// �I��͈͓��u��
	m_szText[0] = L'\0';			// ����������
	m_szText2[0] = L'\0';		// �u���㕶����
	m_nReplaceTarget = 0;		// �u���Ώ�		// 2001.12.03 hor
	m_nPaste = FALSE;			// �\��t����H	// 2001.12.03 hor
	m_nReplaceCnt = 0;			//���ׂĒu���̎��s����		// 2002.02.08 hor
	m_bCanceled = false;		//���ׂĒu���𒆒f������	// 2002.02.08 hor
	return;
}

/* ���[�h���X�_�C�A���O�̕\�� */
HWND CDlgReplace::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam, BOOL bSelected )
{
	m_sSearchOption = m_pShareData->m_Common.m_sSearch.m_sSearchOption;		// �����I�v�V����
	m_bConsecutiveAll = m_pShareData->m_Common.m_sSearch.m_bConsecutiveAll;	// �u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	m_bSelectedArea = m_pShareData->m_Common.m_sSearch.m_bSelectedArea;		// �I��͈͓��u��
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	m_bSelected = bSelected;
	m_ptEscCaretPos_PHY = ((CEditView*)lParam)->GetCaret().GetCaretLogicPos();	// ����/�u���J�n���̃J�[�\���ʒu�ޔ�
	((CEditView*)lParam)->m_bSearch = TRUE;							// ����/�u���J�n�ʒu�̓o�^�L��			02/07/28 ai
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
//	GetHwnd() = hwndDlg;	/* ���̃_�C�A���O�̃n���h�� */

	/* ���������� */
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT, m_szText );
	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT );
	for( i = 0; i < m_pShareData->m_sSearchKeywords.m_aSearchKeys.size(); ++i ){
		Combo_AddString( hwndCombo, m_pShareData->m_sSearchKeywords.m_aSearchKeys[i] );
	}

	/* �u���㕶���� */
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT2, m_szText2 );
	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT2 );
	for( i = 0; i < m_pShareData->m_sSearchKeywords.m_aReplaceKeys.size(); ++i ){
		Combo_AddString( hwndCombo, m_pShareData->m_sSearchKeywords.m_aReplaceKeys[i] );
	}

	/* �p�啶���Ɖp����������ʂ��� */
	::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, m_sSearchOption.bLoHiCase );

	// 2001/06/23 N.Nakatani
	/* �P��P�ʂŒT�� */
	::CheckDlgButton( GetHwnd(), IDC_CHK_WORD, m_sSearchOption.bWordOnly );

	/* �u���ׂĒu���v�͒u���̌J�Ԃ� */	// 2007.01.16 ryoji
	::CheckDlgButton( GetHwnd(), IDC_CHECK_CONSECUTIVEALL, m_bConsecutiveAll );

	// From Here Jun. 29, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if( CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, false )
		&& m_sSearchOption.bRegularExp){
		/* �p�啶���Ɖp����������ʂ��� */
		::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 1 );

		// 2001/06/23 N.Nakatani
		/* �P��P�ʂŒT�� */
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), FALSE );
	}
	else {
		::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 0 );

		/*�u���ׂĒu���v�͒u���̌J�Ԃ� */
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_CONSECUTIVEALL ), FALSE );	// 2007.01.16 ryoji
	}
	// To Here Jun. 29, 2001 genta

	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND );


	/* �u�� �_�C�A���O�������I�ɕ��� */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_bAutoCloseDlgReplace, m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgReplace );

	/* �擪�i�����j����Č��� 2002.01.26 hor */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_sSearch.m_bSearchAll );

	// From Here 2001.12.03 hor
	// �N���b�v�{�[�h����\��t����H
	::CheckDlgButton( GetHwnd(), IDC_CHK_PASTE, m_nPaste );
	// �u���Ώ�
	if(m_nReplaceTarget==0){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_REPLACE, TRUE );
	}else
	if(m_nReplaceTarget==1){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_INSERT, TRUE );
	}else
	if(m_nReplaceTarget==2){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_ADD, TRUE );
	}
	// To Here 2001.12.03 hor

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
/* 0==����������  0���傫��==����   0��菬����==���̓G���[ */
int CDlgReplace::GetData( void )
{
	/* �p�啶���Ɖp����������ʂ��� */
	m_sSearchOption.bLoHiCase = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_LOHICASE ));

	// 2001/06/23 N.Nakatani
	/* �P��P�ʂŒT�� */
	m_sSearchOption.bWordOnly = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_WORD ));

	/* �u���ׂĒu���v�͒u���̌J�Ԃ� */	// 2007.01.16 ryoji
	m_bConsecutiveAll = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_CONSECUTIVEALL );

	/* ���K�\�� */
	m_sSearchOption.bRegularExp = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ));
	/* �I��͈͓��u�� */
	m_bSelectedArea = ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_SELECTEDAREA );
	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND );

	m_pShareData->m_Common.m_sSearch.m_sSearchOption = m_sSearchOption;		// �����I�v�V����
	m_pShareData->m_Common.m_sSearch.m_bConsecutiveAll = m_bConsecutiveAll;	// 1==�u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	m_pShareData->m_Common.m_sSearch.m_bSelectedArea = m_bSelectedArea;		// �I��͈͓��u��
	m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��

	/* ���������� */
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_TEXT, m_szText, _countof(m_szText));
	/* �u���㕶���� */
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_TEXT2, m_szText2, _countof(m_szText2));

	/* �u�� �_�C�A���O�������I�ɕ��� */
	m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgReplace = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_bAutoCloseDlgReplace );

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
		//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeyArr()�ǉ��ɔ����ύX
		CSearchKeywordManager().AddToSearchKeyArr( m_szText );

		/* �u���㕶���� */
		//@@@ 2002.2.2 YAZAKI CShareData.AddToReplaceKeyArr()�ǉ��ɔ����ύX
		CSearchKeywordManager().AddToReplaceKeyArr( m_szText2 );

		// From Here 2001.12.03 hor
		// �N���b�v�{�[�h����\��t����H
		m_nPaste=IsDlgButtonChecked( GetHwnd(), IDC_CHK_PASTE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT2 ), !m_nPaste );
		// �u���Ώ�
		m_nReplaceTarget=0;
		if(::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_INSERT )){
			m_nReplaceTarget=1;
		}else
		if(::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_ADD )){
			m_nReplaceTarget=2;
		}
		// To Here 2001.12.03 hor

		return 1;
	}else{
		return 0;
	}
}




BOOL CDlgReplace::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );
	//	Jun. 26, 2001 genta
	//	���̈ʒu�Ő��K�\���̏�����������K�v�͂Ȃ�
	//	���Ƃ̈�ѐ���ۂ��ߍ폜

	/* ���[�U�[���R���{ �{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	::SendMessage( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	::SendMessage( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT2 ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );

	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT2 ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );


	/* �e�L�X�g�I�𒆂� */
	if( m_bSelected ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHPREV ), FALSE );	// 2001.12.03 hor �R�����g����
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHNEXT ), FALSE );	// 2001.12.03 hor �R�����g����
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_REPALCE ), FALSE );		// 2001.12.03 hor �R�����g����
		::CheckDlgButton( GetHwnd(), IDC_RADIO_SELECTEDAREA, TRUE );
//		::CheckDlgButton( GetHwnd(), IDC_RADIO_ALLAREA, FALSE );						// 2001.12.03 hor �R�����g
	}else{
//		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_SELECTEDAREA ), FALSE );	// 2001.12.03 hor �R�����g
//		::CheckDlgButton( GetHwnd(), IDC_RADIO_SELECTEDAREA, FALSE );					// 2001.12.03 hor �R�����g
		::CheckDlgButton( GetHwnd(), IDC_RADIO_ALLAREA, TRUE );
	}
	/* ���N���X�����o */
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );

}




BOOL CDlgReplace::OnBnClicked( int wID )
{
	CEditView*	pcEditView = (CEditView*)m_lParam;

	switch( wID ){
	case IDC_CHK_PASTE:
		/* �e�L�X�g�̓\��t�� */
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_PASTE ) &&
			!pcEditView->m_pcEditDoc->m_cDocEditor.IsEnablePaste() ){
			OkMessage( GetHwnd(), _T("�N���b�v�{�[�h�ɗL���ȃf�[�^������܂���I") );
			::CheckDlgButton( GetHwnd(), IDC_CHK_PASTE, FALSE );
		}
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT2 ), !(::IsDlgButtonChecked( GetHwnd(), IDC_CHK_PASTE)) );
		return TRUE;
	case IDC_RADIO_SELECTEDAREA:
		/* �͈͔͈� */
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_ALLAREA ) ){
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHPREV ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHNEXT ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_REPALCE ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHPREV ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHNEXT ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_REPALCE ), FALSE );
		}
		return TRUE;
	case IDC_RADIO_ALLAREA:
		/* �t�@�C���S�� */
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_ALLAREA ) ){
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHPREV ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHNEXT ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_REPALCE ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHPREV ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHNEXT ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_REPALCE ), FALSE );
		}
		return TRUE;
// To Here 2001.12.03 hor
	case IDC_BUTTON_HELP:
		/* �u�u���v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_REPLACE_DIALOG) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//	case IDC_CHK_LOHICASE:	/* �啶���Ə���������ʂ��� */
//		MYTRACE_A( "IDC_CHK_LOHICASE\n" );
//		return TRUE;
//	case IDC_CHK_WORDONLY:	/* ��v����P��̂݌��� */
//		MYTRACE_A( "IDC_CHK_WORDONLY\n" );
//		break;
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

				// 2001/06/23 N.Nakatani
				/* �P��P�ʂŒT�� */
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), FALSE );

				/*�u���ׂĒu���v�͒u���̌J�Ԃ� */
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_CONSECUTIVEALL ), TRUE );	// 2007.01.16 ryoji
			}
		}else{
			/* �p�啶���Ɖp����������ʂ��� */
			//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_LOHICASE ), TRUE );
			//	Jan. 31, 2002 genta
			//	�啶���E�������̋�ʂ͐��K�\���̐ݒ�Ɋւ�炸�ۑ�����
			//::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 0 );

			// 2001/06/23 N.Nakatani
			/* �P��P�ʂŒT�� */
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), TRUE );

			/*�u���ׂĒu���v�͒u���̌J�Ԃ� */
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_CONSECUTIVEALL ), FALSE );	// 2007.01.16 ryoji
		}
		return TRUE;
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


	case IDC_BUTTON_SEARCHPREV:	/* �㌟�� */
		if( 0 < GetData() ){

			// �����J�n�ʒu��o�^ 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				/*
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				*/
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
			/* �O������ */
			pcEditView->GetCommander().HandleCommand( F_SEARCH_PREV, TRUE, (LPARAM)GetHwnd(), 0, 0, 0 );
			/* �ĕ`��i0�������}�b�`�ŃL�����b�g��\�����邽�߁j */
			CLayoutRange cRangeSel = pcEditView->GetCommander().GetSelect();
			if( cRangeSel.IsValid() && cRangeSel.IsOne() )	// ������Ȃ������Ƃ��̓X�e�[�^�X�o�[�̓��Y���b�Z�[�W���c������	// 2009.06.23 ryoji
				pcEditView->GetCommander().HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			OkMessage( GetHwnd(), _T("��������w�肵�Ă��������B") );
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:	/* ������ */
		if( 0 < GetData() ){

			// �����J�n�ʒu��o�^ 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				/*
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				*/
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
			/* �������� */
			pcEditView->GetCommander().HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)GetHwnd(), 0, 0, 0 );
			/* �ĕ`��i0�������}�b�`�ŃL�����b�g��\�����邽�߁j */
			CLayoutRange cRangeSel = pcEditView->GetCommander().GetSelect();
			if( cRangeSel.IsValid() && cRangeSel.IsOne() )	// ������Ȃ������Ƃ��̓X�e�[�^�X�o�[�̓��Y���b�Z�[�W���c������	// 2009.06.23 ryoji
				pcEditView->GetCommander().HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );

			// 2001.12.03 hor
			//	�_�C�A���O����Ȃ��Ƃ��AIDC_COMBO_TEXT ��� Enter �����ꍇ��
			//	�L�����b�g���\������Ȃ��Ȃ�̂��������
			::SendMessageAny(GetHwnd(),WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(GetHwnd(),IDC_COMBO_TEXT ),TRUE);
			// To Here 2001.12.03 hor
               
		}else{
			OkMessage( GetHwnd(), _T("��������w�肵�Ă��������B") );
		}
		return TRUE;

	case IDC_BUTTON_SETMARK:	//2002.01.16 hor �Y���s�}�[�N
		if( 0 < GetData() ){
			pcEditView->GetCommander().HandleCommand( F_BOOKMARK_PATTERN, FALSE, 0, 0, 0, 0 );
			::SendMessageAny(GetHwnd(),WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(GetHwnd(),IDC_COMBO_TEXT ),TRUE);
		}
		return TRUE;

	case IDC_BUTTON_REPALCE:	/* �u�� */
		if( 0 < GetData() ){

			// �u���J�n�ʒu��o�^ 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				/*
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				*/
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* �u�� */
			//@@@ 2002.2.2 YAZAKI �u���R�}���h��CEditView�ɐV��
			//@@@ 2002/04/08 YAZAKI �e�E�B���h�E�̃n���h����n���悤�ɕύX�B
			pcEditView->GetCommander().HandleCommand( F_REPLACE, TRUE, (LPARAM)GetHwnd(), 0, 0, 0 );
			/* �ĕ`�� */
			pcEditView->GetCommander().HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			OkMessage( GetHwnd(), _T("��������w�肵�Ă��������B") );
		}
		return TRUE;
	case IDC_BUTTON_REPALCEALL:	/* ���ׂĒu�� */
		if( 0 < GetData() ){
			// �u���J�n�ʒu��o�^ 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* ���ׂčs�u�����̏��u�́u���ׂĒu���v�͒u���̌J�Ԃ��I�v�V����OFF�̏ꍇ�ɂ��č폜 2007.01.16 ryoji */
			pcEditView->GetCommander().HandleCommand( F_REPLACE_ALL, TRUE, 0, 0, 0, 0 );
			pcEditView->GetCommander().HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );

			/* �A�N�e�B�u�ɂ��� */
			ActivateFrameWindow( GetHwnd() );

			TopOkMessage( GetHwnd(), _T("%d�ӏ���u�����܂����B"), m_nReplaceCnt);

			if( !m_bCanceled ){
				if( m_bModal ){		/* ���[�_���_�C�A���O�� */
					/* �u���_�C�A���O����� */
					::EndDialog( GetHwnd(), 0 );
				}else{
					/* �u�� �_�C�A���O�������I�ɕ��� */
					if( m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgReplace ){
						::DestroyWindow( GetHwnd() );
					}
				}
			}
			return TRUE;
		}else{
			OkMessage( GetHwnd(), _T("�u���������w�肵�Ă��������B") );
		}
		return TRUE;
//	case IDCANCEL:
//		::EndDialog( hwndDlg, 0 );
//		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

//@@@ 2002.01.18 add start
LPVOID CDlgReplace::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


