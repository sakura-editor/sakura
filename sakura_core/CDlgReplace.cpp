//	$Id$
/*!	@file
	@brief �u���_�C�A���O

	@author Norio Nakatani
	@date 2001/06/23 N.Nakatani �P��P�ʂŌ�������@�\������
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, Stonee, hor

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
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
	m_nReplaceTarget=0;			/* �u���Ώ� */		// 2001.12.03 hor
	m_nPaste=FALSE;				/* �\��t����H */	// 2001.12.03 hor
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

	/* �擪�i�����j����Č��� 2002.01.26 hor */
	m_pShareData->m_Common.m_bSearchAll = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_SEARCHALL );

	if( 0 < lstrlen( m_szText ) ){
		/* ���K�\���H */
		// From Here Jun. 26, 2001 genta
		//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
		if( m_bRegularExp && !CheckRegexpSyntax( m_szText, m_hWnd, true ) ){
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
	int			nNewPos;
	int			nReplaceNum;
	char		szLabel[64];
	int			nAllLineNum;
	HWND		hwndProgress;
	HWND		hwndStatic;
//	MSG			msg;

// From Here 2001.12.03 hor
	int			colFrom;		//�I��͈͊J�n��
	int			linFrom;		//�I��͈͊J�n�s
	int			colTo,colToP;	//�I��͈͏I����
	int			linTo,linToP;	//�I��͈͏I���s
	int			colDif = 0;		//�u����̌�����
	int			linDif = 0;		//�u����̍s����
	int			colOld = 0;		//������̑I��͈͎���
	int			linOld = 0;		//������̍s
	int			lineCnt;		//�u���O�̍s��
	int			linPrev = 0;	//�O��̌����s(��`) @@@2001.12.31 YAZAKI warning�ގ�
	int			linNext;		//����̌����s(��`)
	int			colTmp,linTmp,colLast,linLast;
	int			bBeginBoxSelect; // ��`�I���H
	const char*	pLine;
	int			nLineLen;
	const CLayout* pcLayout;
	int			bLineOffset=FALSE;
	int			bLineChecked=FALSE;

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
			if( !CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
			}else{
			// To Here Jun. 26, 2001 genta

				/* �p�啶���Ɖp����������ʂ��� */
				//	Jan. 31, 2002 genta
				//	�啶���E�������̋�ʂ͐��K�\���̐ݒ�Ɋւ�炸�ۑ�����
				//::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

				// 2001/06/23 N.Nakatani
				/* �P��P�ʂŒT�� */
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );

			}
		}else{
			/* �p�啶���Ɖp����������ʂ��� */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
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
			pcEditView->HandleCommand( F_BOOKMARK_PATTERN, NULL, 0, 0, 0, 0 );
			::SendMessage(m_hWnd,WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDC_COMBO_TEXT ),TRUE);
		}
		return TRUE;

	case IDC_BUTTON_REPALCE:	/* �u�� */
		if( 0 < GetData() ){
			// From Here 2001.12.03 hor
			if( m_nPaste && !pcEditView->m_pcEditDoc->IsEnablePaste()){
				::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,"�N���b�v�{�[�h�ɗL���ȃf�[�^������܂���I");
				::CheckDlgButton( m_hWnd, IDC_CHK_PASTE, FALSE );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), TRUE );
				return TRUE;
			}

			// 2002.01.09 hor
			// �I���G���A������΁A���̐擪�ɃJ�[�\�����ڂ�
			if( pcEditView->IsTextSelected() ){
				if( pcEditView->m_bBeginBoxSelect ){
					pcEditView->MoveCursor( pcEditView->m_nSelectColmFrom,
											pcEditView->m_nSelectLineFrom,
											TRUE );
				} else {
					pcEditView->HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );
				}
			}
			// To Here 2002.01.09 hor
			
			// ��`�I���H
//			bBeginBoxSelect = pcEditView->m_bBeginBoxSelect;

			/* �J�[�\�����ړ� */
			//pcEditView->HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );	//�H�H�H
			// To Here 2001.12.03 hor

			/* �e�L�X�g�I������ */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			pcEditView->DisableSelectArea( TRUE );

			/* �������� */
			pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );

			/* �e�L�X�g���I������Ă��邩 */
			if( pcEditView->IsTextSelected() ){
				// From Here 2001.12.03 hor
				if(m_nReplaceTarget==1){	//�}���ʒu�ֈړ�
					colTmp = pcEditView->m_nSelectColmTo - pcEditView->m_nSelectColmFrom;
					linTmp = pcEditView->m_nSelectLineTo - pcEditView->m_nSelectLineFrom;
					pcEditView->m_nSelectColmFrom=-1;
					pcEditView->m_nSelectLineFrom=-1;
					pcEditView->m_nSelectColmTo	 =-1;
					pcEditView->m_nSelectLineTo	 =-1;
				}else
				if(m_nReplaceTarget==2){	//�ǉ��ʒu�ֈړ�
					if(m_bRegularExp){
						//������̕��������s������玟�̍s�̐擪�ֈړ�
						pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
							pcEditView->m_nSelectColmTo,
							pcEditView->m_nSelectLineTo,
							&colTmp,
							&linTmp
						);
						pLine = pcEditView->m_pcEditDoc->m_cLayoutMgr.GetLineStr2( pcEditView->m_nSelectLineTo, &nLineLen, &pcLayout );
						if( NULL != pLine &&
							colTmp >= nLineLen - (pcLayout->m_cEol.GetLen()) ){
							pcEditView->m_nSelectColmTo=0;
							pcEditView->m_nSelectLineTo++;
						}
					}
					pcEditView->m_nCaretPosX = pcEditView->m_nSelectColmTo;
					pcEditView->m_nCaretPosY = pcEditView->m_nSelectLineTo;
					pcEditView->m_nSelectColmFrom=-1;
					pcEditView->m_nSelectLineFrom=-1;
					pcEditView->m_nSelectColmTo	 =-1;
					pcEditView->m_nSelectLineTo	 =-1;
				}
				/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
				/* �e�L�X�g��\��t�� */
				//pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)m_szText2, FALSE, 0, 0 );
				if(m_nPaste){
					pcEditView->HandleCommand( F_PASTE, 0, 0, 0, 0, 0 );
				}else{
					// 2002/01/19 novice ���K�\���ɂ�镶����u��
					if( pcEditView->m_bCurSrchRegularExp ){ /* �����^�u��  1==���K�\�� */
						CMemory cmemory;
						CBregexp cRegexp;
						char*	RegRepOut;

						if( !InitRegexp( m_hWnd, cRegexp, true ) ){
							return 0;
						}

						if( FALSE == pcEditView->GetSelectedData( cmemory, FALSE, NULL, FALSE /*, EOL_NONE 2002/1/26 novice */ ) ){
							::MessageBeep( MB_ICONHAND );
						}
						// �ϊ���̕������ʂ̈����ɂ��܂��� 2002.01.26 hor
						if( cRegexp.Replace( m_szText, m_szText2, cmemory.m_pData, cmemory.m_nDataLen ,&RegRepOut) ){
							pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)RegRepOut, FALSE, 0, 0 );
							delete [] RegRepOut;
						}
					}else{
						pcEditView->HandleCommand( F_INSTEXT, FALSE, (LPARAM)m_szText2, FALSE, 0, 0 );
					}
				}
				// �}����̌����J�n�ʒu�𒲐�
				if(m_nReplaceTarget==1){
					pcEditView->m_nCaretPosX+=colTmp;
					pcEditView->m_nCaretPosY+=linTmp;
				}
				// To Here 2001.12.03 hor
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
	case IDC_BUTTON_REPALCEALL:	/* ���ׂĒu�� */
		if( 0 < GetData() ){

		// From Here 2001.12.03 hor
			if( m_nPaste && !pcEditView->m_pcEditDoc->IsEnablePaste() ){
				::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,"�N���b�v�{�[�h�ɗL���ȃf�[�^������܂���I");
				::CheckDlgButton( m_hWnd, IDC_CHK_PASTE, FALSE );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), TRUE );
				return TRUE;
			}
			// ��`�I���H
			bBeginBoxSelect = pcEditView->m_bBeginBoxSelect;
		// To Here 2001.12.03 hor


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

		// From Here 2001.12.03 hor
			if (m_bSelectedArea) {
				/* �I��͈͒u�� */
				/* �I��͈͊J�n�ʒu�̎擾 */
				colFrom = pcEditView->m_nSelectColmFrom;
				linFrom = pcEditView->m_nSelectLineFrom;
				colTo   = pcEditView->m_nSelectColmTo;
				linTo   = pcEditView->m_nSelectLineTo;
				pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
					colTo,
					linTo,
					&colToP,
					&linToP
				);
				//�I��͈͊J�n�ʒu�ֈړ�
				pcEditView->MoveCursor( colFrom, linFrom, bDisplayUpdate );
			}else{
				/* �t�@�C���S�̒u�� */
				/* �t�@�C���̐擪�Ɉړ� */
				pcEditView->HandleCommand( F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0 );
			}
			colLast=pcEditView->m_nCaretPosX;
			linLast=pcEditView->m_nCaretPosY;

			/* �e�L�X�g�I������ */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			pcEditView->DisableSelectArea( bDisplayUpdate );
			/* �������� */
			pcEditView->HandleCommand( F_SEARCH_NEXT, bDisplayUpdate, 0, 0, 0, 0 );
		// To Here 2001.12.03 hor

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

				// From Here 2001.12.03 hor
					/* ������̈ʒu���m�F */
					if(m_bSelectedArea){
						if (bBeginBoxSelect) {
						// ��`�I��
						//	o ���C�A�E�g���W���`�F�b�N���Ȃ���u������
						//	o �܂�Ԃ�������ƕςɂȂ邩���E�E�E
						//
							// �������̍s�����L��
							lineCnt=pcEditView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
							// ������͈̔͏I�[
							colOld = pcEditView->m_nSelectColmTo;
							linOld = pcEditView->m_nSelectLineTo;
							// �O��̌����s�ƈႤ�H
							if(linOld!=linPrev){
								colDif=0;
							}
							linPrev=linOld;
							// �s�͔͈͓��H
							if ((linTo+linDif == linOld && colTo+colDif < colOld) ||
								(linTo+linDif <  linOld)) {
								break;
							}
							// ���͔͈͓��H
							if(!((colFrom<=pcEditView->m_nSelectColmFrom)&&
							     (colOld<=colTo+colDif))){
								if(colOld<colTo+colDif){
									linNext=pcEditView->m_nSelectLineTo;
								}else{
									linNext=pcEditView->m_nSelectLineTo+1;
								}
								//���̌����J�n�ʒu�փV�t�g
								pcEditView->m_nCaretPosX=colFrom;
								pcEditView->m_nCaretPosY=linNext;
								//pcEditView->DisableSelectArea( bDisplayUpdate );
								//pcEditView->MoveCursor( colFrom, linNext, bDisplayUpdate );
								pcEditView->HandleCommand( F_SEARCH_NEXT, bDisplayUpdate, 0, 0, 0, 0 );
								colDif=0;
								continue;
							}
						}else{
						// ���ʂ̑I��
						//	o �������W���`�F�b�N���Ȃ���u������
						//
							// �������̍s�����L��
							lineCnt=pcEditView->m_pcEditDoc->m_cDocLineMgr.GetLineCount();
							// ������͈̔͏I�[
							pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
								pcEditView->m_nSelectColmTo,
								pcEditView->m_nSelectLineTo,
								&colOld,
								&linOld
							);
							// �s�͔͈͓��H
							if ((linToP+linDif == linOld && colToP+colDif < colOld) ||
								(linToP+linDif <  linOld)) {
								break;
							}
						}
					}

					if(m_nReplaceTarget==1){	//�}���ʒu�Z�b�g
						colTmp = pcEditView->m_nSelectColmTo - pcEditView->m_nSelectColmFrom;
						linTmp = pcEditView->m_nSelectLineTo - pcEditView->m_nSelectLineFrom;
						pcEditView->m_nSelectColmFrom=-1;
						pcEditView->m_nSelectLineFrom=-1;
						pcEditView->m_nSelectColmTo	 =-1;
						pcEditView->m_nSelectLineTo	 =-1;
					}else
					if(m_nReplaceTarget==2){	//�ǉ��ʒu�Z�b�g
						if(!bLineChecked){
							//������̈ʒu�����s������玟�̍s�̐擪�ɃI�t�Z�b�g
							pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
								pcEditView->m_nSelectColmTo,
								pcEditView->m_nSelectLineTo,
								&colTmp,
								&linTmp
							);
							if(m_bRegularExp){
								pLine = pcEditView->m_pcEditDoc->m_cLayoutMgr.GetLineStr2( pcEditView->m_nSelectLineTo, &nLineLen, &pcLayout );
								if( NULL != pLine &&
									colTmp >= nLineLen - (pcLayout->m_cEol.GetLen()) ){
									bLineOffset=TRUE;
								}
							}
							bLineChecked=TRUE;
						}
						if(bLineOffset){
							pcEditView->m_nCaretPosX = 0;
							pcEditView->m_nCaretPosY ++;
							pcEditView->m_nCaretPosX_PHY = 0;
							pcEditView->m_nCaretPosY_PHY ++;
						}else{
							pcEditView->m_nCaretPosX = pcEditView->m_nSelectColmTo;
							pcEditView->m_nCaretPosY = pcEditView->m_nSelectLineTo;
						}
						pcEditView->m_nSelectColmFrom=-1;
						pcEditView->m_nSelectLineFrom=-1;
						pcEditView->m_nSelectColmTo	 =-1;
						pcEditView->m_nSelectLineTo	 =-1;
					}

					/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
					/* �e�L�X�g��\��t�� */
					//pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)m_szText2, FALSE, 0, 0 );
					if(m_nPaste){
						pcEditView->HandleCommand( F_PASTE, 0, 0, 0, 0, 0 );
					}else{
						// 2002/01/19 novice ���K�\���ɂ�镶����u��
						if( pcEditView->m_bCurSrchRegularExp ){ /* �����^�u��  1==���K�\�� */
							CMemory cmemory;
							CBregexp cRegexp;
							char*	RegRepOut;

							if( !InitRegexp( m_hWnd, cRegexp, true ) ){
								return 0;
							}

							if( FALSE == pcEditView->GetSelectedData( cmemory, FALSE, NULL, FALSE /*, EOL_NONE 2002/1/26 novice */ ) ){
								::MessageBeep( MB_ICONHAND );
							}

							// �ϊ���̕������ʂ̈����ɂ��܂��� 2002.01.26 hor
							if( cRegexp.Replace( m_szText, m_szText2, cmemory.m_pData, cmemory.m_nDataLen ,&RegRepOut) ){
								pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)RegRepOut, FALSE, 0, 0 );
								delete [] RegRepOut;
							}
						}else{
							pcEditView->HandleCommand( F_INSTEXT, bDisplayUpdate, (LPARAM)m_szText2, TRUE, 0, 0 );
						}
					}

					// �}����̈ʒu����
					if(m_nReplaceTarget==1){
						pcEditView->m_nCaretPosX+=colTmp;
						pcEditView->m_nCaretPosY+=linTmp;
						if (!bBeginBoxSelect) {
							pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
								pcEditView->m_nCaretPosX,
								pcEditView->m_nCaretPosY,
								&pcEditView->m_nCaretPosX_PHY,
								&pcEditView->m_nCaretPosY_PHY
							);
						}
					}

					// �Ō�ɒu�������ʒu���L��
					colLast=pcEditView->m_nCaretPosX;
					linLast=pcEditView->m_nCaretPosY;

					/* �u����̈ʒu���m�F */
					if(m_bSelectedArea){
						// �������u���̍s�␳�l�擾
						if(bBeginBoxSelect){
							colDif += colLast - colOld;
							linDif += pcEditView->m_pcEditDoc->m_cLayoutMgr.GetLineCount() - lineCnt;
						}else{
							colTmp=pcEditView->m_nCaretPosX_PHY;
							linTmp=pcEditView->m_nCaretPosY_PHY;
							linDif += pcEditView->m_pcEditDoc->m_cDocLineMgr.GetLineCount() - lineCnt;
							if(linToP+linDif==linTmp){
								colDif += colTmp - colOld;
							}
						}
					}
				// To Here 2001.12.03 hor

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

			if( !cDlgCancel.IsCanceled() ){
				nNewPos = 100;
				::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
			}
//			cDlgCancel.Close();
			cDlgCancel.CloseDialog( 0 );
			::EnableWindow( m_hWnd, TRUE );
			::EnableWindow( ::GetParent( m_hWnd ), TRUE );
			::EnableWindow( ::GetParent( ::GetParent( m_hWnd ) ), TRUE );


		// From Here 2001.12.03 hor

			/* �e�L�X�g�I������ */
			pcEditView->DisableSelectArea( TRUE );

			/* �J�[�\���E�I��͈͕��� */
			if((!m_bSelectedArea) ||			// �t�@�C���S�̒u��
			   (cDlgCancel.IsCanceled())) {		// �L�����Z�����ꂽ
				// �Ō�ɒu������������̉E��
				pcEditView->MoveCursor( colLast, linLast, TRUE );
			}else{
				if (bBeginBoxSelect) {
				// ��`�I��
					pcEditView->m_bBeginBoxSelect=bBeginBoxSelect;
					linTo+=linDif;
					if(linTo<0)linTo=0;
				}else{
				// ���ʂ̑I��
					colToP+=colDif;
					if(colToP<0)colToP=0;
					linToP+=linDif;
					if(linToP<0)linToP=0;
					pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
						colToP,
						linToP,
						&colTo,
						&linTo
					);
				}
				if(linFrom<linTo || colFrom<colTo){
					pcEditView->m_nSelectLineFrom = linFrom;
					pcEditView->m_nSelectColmFrom = colFrom;
					pcEditView->m_nSelectLineTo   = linTo;
					pcEditView->m_nSelectColmTo   = colTo;
				}
				pcEditView->MoveCursor( colTo, linTo, TRUE );
			}
		// To Here 2001.12.03 hor

			// �ĕ`��
			pcEditView->m_bDrawSWITCH = TRUE;
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

//@@@ 2002.01.18 add start
LPVOID CDlgReplace::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
