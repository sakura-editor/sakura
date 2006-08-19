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

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "sakura_rc.h"
#include "CDlgReplace.h"
#include "debug.h"
#include "CEditView.h"
#include "etc_uty.h"
#include "global.h"
#include "funccode.h"		// Stonee, 2001/03/12
#include "CLayout.h"/// 2002/2/3 aroka
#include "CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����

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
	IDC_BUTTON_REPLACEALL_LINE,		HIDC_REP_BUTTON_REPLACEALL_LINE,	//���ׂčs�u��	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgReplace::CDlgReplace()
{
	m_bLoHiCase = FALSE;		/* �p�啶���Ɖp����������ʂ��� */
	m_bWordOnly = FALSE;		/* ��v����P��̂݌������� */
	m_bRegularExp = FALSE;		/* ���K�\�� */
	m_bSelectedArea = FALSE;	/* �I��͈͓��u�� */
	m_szText[0] = '\0';			/* ���������� */
	m_szText2[0] = '\0';		/* �u���㕶���� */
	m_nReplaceTarget = 0;		/* �u���Ώ� */		// 2001.12.03 hor
	m_nPaste = FALSE;			/* �\��t����H */	// 2001.12.03 hor
	m_nReplaceCnt = 0;			//���ׂĒu���̎��s����		// 2002.02.08 hor
	m_bCanceled = false;		//���ׂĒu���𒆒f������	// 2002.02.08 hor
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
	m_nEscCaretPosX_PHY = ((CEditView*)lParam)->m_nCaretPosX_PHY;	/* ����/�u���J�n���̃J�[�\���ʒu�ޔ�	02/07/28 ai */
	m_nEscCaretPosY_PHY = ((CEditView*)lParam)->m_nCaretPosY_PHY;	/* ����/�u���J�n���̃J�[�\���ʒu�ޔ�	02/07/28 ai */
	((CEditView*)lParam)->m_bSearch = TRUE;							/* ����/�u���J�n�ʒu�̓o�^�L��			02/07/28 ai */
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

// From Here 2001.12.03 hor
//	/* �I��͈͓��u�� */
//	if( m_pShareData->m_Common.m_bSelectedArea ){
//		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, TRUE );
//	}else{
//		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, TRUE );
//	}
// To Here 2001.12.03 hor

	// From Here Jun. 29, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if( CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, false )
		&& m_bRegularExp){
		/* �p�啶���Ɖp����������ʂ��� */
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 1 );
		//	���K�\���̂Ƃ����I���ł���悤�ɁB
//		::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
//		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

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

	/* �擪�i�����j����Č��� 2002.01.26 hor */
	::CheckDlgButton( m_hWnd, IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_bSearchAll );

	// From Here 2001.12.03 hor
	// �N���b�v�{�[�h����\��t����H
	::CheckDlgButton( m_hWnd, IDC_CHK_PASTE, m_nPaste );
	// �u���Ώ�
	if(m_nReplaceTarget==0){
		::CheckDlgButton( m_hWnd, IDC_RADIO_REPLACE, TRUE );
	}else
	if(m_nReplaceTarget==1){
		::CheckDlgButton( m_hWnd, IDC_RADIO_INSERT, TRUE );
	}else
	if(m_nReplaceTarget==2){
		::CheckDlgButton( m_hWnd, IDC_RADIO_ADD, TRUE );
	}
	// To Here 2001.12.03 hor

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
/* 0==����������  0���傫��==����   0��菬����==���̓G���[ */
int CDlgReplace::GetData( void )
{
//	int			i;
//	int			j;
//	CMemory*	pcmWork;

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
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText, sizeof( m_szText ));
	/* �u���㕶���� */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT2, m_szText2, sizeof( m_szText2 ));

	/* �u�� �_�C�A���O�������I�ɕ��� */
	m_pShareData->m_Common.m_bAutoCloseDlgReplace = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgReplace );

	/* �擪�i�����j����Č��� 2002.01.26 hor */
	m_pShareData->m_Common.m_bSearchAll = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_SEARCHALL );

	if( 0 < lstrlen( m_szText ) ){
		/* ���K�\���H */
		// From Here Jun. 26, 2001 genta
		//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
		int nFlag = 0x00;
		nFlag |= m_bLoHiCase ? 0x01 : 0x00;
		if( m_bRegularExp && !CheckRegexpSyntax( m_szText, m_hWnd, true, nFlag ) ){
			return -1;
		}
		// To Here Jun. 26, 2001 genta ���K�\�����C�u���������ւ�

		/* ���������� */
		//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeyArr()�ǉ��ɔ����ύX
		CShareData::getInstance()->AddToSearchKeyArr( m_szText );

		/* �u���㕶���� */
		//@@@ 2002.2.2 YAZAKI CShareData.AddToReplaceKeyArr()�ǉ��ɔ����ύX
		CShareData::getInstance()->AddToReplaceKeyArr( m_szText2 );

		// From Here 2001.12.03 hor
		// �N���b�v�{�[�h����\��t����H
		m_nPaste=IsDlgButtonChecked( m_hWnd, IDC_CHK_PASTE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), !m_nPaste );
		// �u���Ώ�
		m_nReplaceTarget=0;
		if(::IsDlgButtonChecked( m_hWnd, IDC_RADIO_INSERT )){
			m_nReplaceTarget=1;
		}else
		if(::IsDlgButtonChecked( m_hWnd, IDC_RADIO_ADD )){
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
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHPREV ), FALSE );	// 2001.12.03 hor �R�����g����
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHNEXT ), FALSE );	// 2001.12.03 hor �R�����g����
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_REPALCE ), FALSE );		// 2001.12.03 hor �R�����g����
		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, TRUE );
//		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, FALSE );						// 2001.12.03 hor �R�����g
	}else{
//		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_SELECTEDAREA ), FALSE );	// 2001.12.03 hor �R�����g
//		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, FALSE );					// 2001.12.03 hor �R�����g
		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, TRUE );
	}
	/* ���N���X�����o */
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );

}




BOOL CDlgReplace::OnBnClicked( int wID )
{
	CEditView*	pcEditView = (CEditView*)m_lParam;
//	int			nNewPos;
//	int			nReplaceNum;
//	char		szLabel[64];
//	int			nAllLineNum;
//	HWND		hwndProgress;
//	HWND		hwndStatic;
//	MSG			msg;

// From Here 2001.12.03 hor
//	int			colFrom;		//�I��͈͊J�n��
//	int			linFrom;		//�I��͈͊J�n�s
//	int			colTo,colToP;	//�I��͈͏I����
//	int			linTo,linToP;	//�I��͈͏I���s
//	int			colDif = 0;		//�u����̌�����
//	int			linDif = 0;		//�u����̍s����
//	int			colOld = 0;		//������̑I��͈͎���
//	int			linOld = 0;		//������̍s
//	int			lineCnt;		//�u���O�̍s��
//	int			linPrev = 0;	//�O��̌����s(��`) @@@2001.12.31 YAZAKI warning�ގ�
//	int			linNext;		//����̌����s(��`)
//	int			colTmp,linTmp,colLast,linLast;
//	int			bBeginBoxSelect; // ��`�I���H
//	const char*	pLine;
//	int			nLineLen;
//	const CLayout* pcLayout;
//	int			bLineOffset=FALSE;
//	int			bLineChecked=FALSE;

	switch( wID ){
	case IDC_CHK_PASTE:
		/* �e�L�X�g�̓\��t�� */
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHK_PASTE ) &&
			!pcEditView->m_pcEditDoc->IsEnablePaste() ){
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,"�N���b�v�{�[�h�ɗL���ȃf�[�^������܂���I");
			::CheckDlgButton( m_hWnd, IDC_CHK_PASTE, FALSE );
		}
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), !(::IsDlgButtonChecked( m_hWnd, IDC_CHK_PASTE)) );
		return TRUE;
	case IDC_RADIO_SELECTEDAREA:
		/* �͈͔͈� */
		if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_ALLAREA ) ){
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHPREV ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHNEXT ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_REPALCE ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHPREV ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHNEXT ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_REPALCE ), FALSE );
		}
		return TRUE;
	case IDC_RADIO_ALLAREA:
		/* �t�@�C���S�� */
		if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_ALLAREA ) ){
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHPREV ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHNEXT ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_REPALCE ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHPREV ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHNEXT ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_REPALCE ), FALSE );
		}
		return TRUE;
// To Here 2001.12.03 hor
	case IDC_BUTTON_HELP:
		/* �u�u���v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_REPLACE_DIALOG) );
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
			if( !CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
			}else{
			// To Here Jun. 26, 2001 genta

				/* �p�啶���Ɖp����������ʂ��� */
				//	Jan. 31, 2002 genta
				//	�啶���E�������̋�ʂ͐��K�\���̐ݒ�Ɋւ�炸�ۑ�����
				//::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
				//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

				// 2001/06/23 N.Nakatani
				/* �P��P�ʂŒT�� */
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );

			}
		}else{
			/* �p�啶���Ɖp����������ʂ��� */
			//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
			//	Jan. 31, 2002 genta
			//	�啶���E�������̋�ʂ͐��K�\���̐ݒ�Ɋւ�炸�ۑ�����
			//::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 0 );

			// 2001/06/23 N.Nakatani
			/* �P��P�ʂŒT�� */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), TRUE );

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
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

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
	case IDC_BUTTON_SEARCHNEXT:	/* ������ */
		if( 0 < GetData() ){

			// �����J�n�ʒu��o�^ 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
			/* �������� */
			pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
			/* �ĕ`�� */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );

			// 2001.12.03 hor
			//	�_�C�A���O����Ȃ��Ƃ��AIDC_COMBO_TEXT ��� Enter �����ꍇ��
			//	�L�����b�g���\������Ȃ��Ȃ�̂��������
			::SendMessage(m_hWnd,WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDC_COMBO_TEXT ),TRUE);
			// To Here 2001.12.03 hor
               
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"��������w�肵�Ă��������B"
			);
		}
		return TRUE;

	case IDC_BUTTON_SETMARK:	//2002.01.16 hor �Y���s�}�[�N
		if( 0 < GetData() ){
			pcEditView->HandleCommand( F_BOOKMARK_PATTERN, FALSE, 0, 0, 0, 0 );
			::SendMessage(m_hWnd,WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDC_COMBO_TEXT ),TRUE);
		}
		return TRUE;

	case IDC_BUTTON_REPALCE:	/* �u�� */
		if( 0 < GetData() ){

			// �u���J�n�ʒu��o�^ 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* �u�� */
			//@@@ 2002.2.2 YAZAKI �u���R�}���h��CEditView�ɐV��
			//@@@ 2002/04/08 YAZAKI �e�E�B���h�E�̃n���h����n���悤�ɕύX�B
			pcEditView->HandleCommand( F_REPLACE, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
			/* �ĕ`�� */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"��������w�肵�Ă��������B"
			);
		}
		return TRUE;
	case IDC_BUTTON_REPALCEALL:	/* ���ׂĒu�� */
	case IDC_BUTTON_REPLACEALL_LINE: /* ���ׂčs�u�� 2006.01.22 ����� */
		if( 0 < GetData() ){
			TCHAR *szUnit = _T("");	// �u�����\���̒P�� 2006.04.02 �����
			// �u���J�n�ʒu��o�^ 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* ���ׂčs�u�����̏��u�ǉ� 2006.01.22 ����� */
			if ( wID == IDC_BUTTON_REPLACEALL_LINE ) {
				pcEditView->HandleCommand( F_REPLACE_ALL_LINE, TRUE, 0, 0, 0, 0 );
				szUnit = _T("�s");
			} else {
				pcEditView->HandleCommand( F_REPLACE_ALL, TRUE, 0, 0, 0, 0 );
				szUnit = _T("�ӏ�");
			}
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );

			/* �A�N�e�B�u�ɂ��� */
			ActivateFrameWindow( m_hWnd );

			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_TOPMOST, GSTR_APPNAME,
				"%d%s��u�����܂����B", m_nReplaceCnt, szUnit);

//			nNewPos = 100;
// 			::SendMessage( ::GetDlgItem( m_hWnd, IDC_PROGRESS_REPLACE ), PBM_SETPOS, nNewPos, 0 );

//			::ShowWindow( ::GetDlgItem( m_hWnd, IDC_PROGRESS_REPLACE ), SW_HIDE );

//			if( !cDlgCancel.IsCanceled() ){
			if( !m_bCanceled ){
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

//@@@ 2002.01.18 add start
LPVOID CDlgReplace::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
