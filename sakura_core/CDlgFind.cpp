//	$Id$
/*!	@file
	@brief �����_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/12/12 �č쐬
	@date 2001/06/23 N.Nakatani �P��P�ʂŌ�������@�\������
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, JEPRO, hor, Stonee
	Copyright (C) 2002, MIK, hor

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CDlgFind.h"
#include "funccode.h"
#include "sakura_rc.h"
//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "CBregexp.h"
#include "CEditView.h"
#include "etc_uty.h"	//Stonee, 2001/03/12
#include "debug.h"// 2002/2/10 aroka �w�b�_����

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
//	MYTRACE( "CDlgFind::CDlgFind()\n" );
	m_bLoHiCase = FALSE;	/* �p�啶���Ɖp����������ʂ��� */
	m_bWordOnly = FALSE;	/* ��v����P��̂݌������� */
	m_bRegularExp = FALSE;	/* ���K�\�� */
	m_szText[0] = '\0';		/* ���������� */
	return;
}



//	/* ���[�_���_�C�A���O�̕\�� */
//	int CDlgFind::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
//	{
//		m_bRegularExp = m_pShareData->m_Common.m_bRegularExp;			/* 1==���K�\�� */
//		m_bLoHiCase = m_pShareData->m_Common.m_bLoHiCase;				/* 1==�p�啶���������̋�� */
//		m_bWordOnly = m_pShareData->m_Common.m_bWordOnly;				/* 1==�P��̂݌��� */
//		m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_bNOTIFYNOTFOUND;	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
//		return CDialog::DoModal( hInstance, hwndParent, IDD_FIND, lParam );
//	}


/* ���[�h���X�_�C�A���O�̕\�� */
HWND CDlgFind::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	m_bRegularExp = m_pShareData->m_Common.m_bRegularExp;			/* 1==���K�\�� */
	m_bLoHiCase = m_pShareData->m_Common.m_bLoHiCase;				/* 1==�p�啶���������̋�� */
	m_bWordOnly = m_pShareData->m_Common.m_bWordOnly;				/* 1==�P��̂݌��� */
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_bNOTIFYNOTFOUND;	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
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
//	MYTRACE( "CDlgFind::SetData()" );
	int		i;
	HWND	hwndCombo;

	/*****************************
	*           ������           *
	*****************************/
	// Here Jun. 26, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌������ɂ��jre.dll������폜

	/* ���[�U�[���R���{ �{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );


	/*****************************
	*         �f�[�^�ݒ�         *
	*****************************/
	/* ���������� */
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText );
	for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[i] );
	}
	/* �p�啶���Ɖp����������ʂ��� */
	::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, m_bLoHiCase );

	// 2001/06/23 Norio Nakatani
	/* �P��P�ʂŌ��� */
	::CheckDlgButton( m_hWnd, IDC_CHK_WORD, m_bWordOnly );

	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND );

	// From Here Jun. 29, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if( CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, false )
		&& m_bRegularExp){
		/* �p�啶���Ɖp����������ʂ��� */
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 1 );
//���K�\����ON�ł��A�啶������������ʂ���^���Ȃ���I���ł���悤�ɁB
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

	/* �����_�C�A���O�������I�ɕ��� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFind, m_pShareData->m_Common.m_bAutoCloseDlgFind );

	/* �擪�i�����j����Č��� 2002.01.26 hor */
	::CheckDlgButton( m_hWnd, IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_bSearchAll );

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
int CDlgFind::GetData( void )
{
//	MYTRACE( "CDlgFind::GetData()" );
//	int			i;
//	int			j;
//	CMemory*	pcmWork;
	//

	/* �p�啶���Ɖp����������ʂ��� */
	m_bLoHiCase = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_LOHICASE );

	// 2001/06/23 Norio Nakatani
	/* �P��P�ʂŌ��� */
	m_bWordOnly = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_WORD );

	/* ��v����P��̂݌������� */
	/* ���K�\�� */
	m_bRegularExp = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP );

	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND );

	m_pShareData->m_Common.m_bRegularExp = m_bRegularExp;			/* 1==���K�\�� */
	m_pShareData->m_Common.m_bLoHiCase = m_bLoHiCase;				/* 1==�p�啶���������̋�� */
	m_pShareData->m_Common.m_bWordOnly = m_bWordOnly;				/* 1==�P��̂݌��� */
	m_pShareData->m_Common.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */

	/* ���������� */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText, _MAX_PATH - 1 );

	/* �����_�C�A���O�������I�ɕ��� */
	m_pShareData->m_Common.m_bAutoCloseDlgFind = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgFind );

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
		//@@@ 2002.2.2 YAZAKI CShareData�Ɉړ�
		CShareData::getInstance()->AddToSearchKeyArr( (const char*)m_szText );
		if( FALSE == m_bModal ){
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
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_SEARCH_DIALOG) );	//Apr. 5, 2001 JEPRO �C���R���ǉ�
		break;
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

				// 2001/06/23 Norio Nakatani
				/* �P��P�ʂŌ��� */
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );
			}
		}else{
			/* �p�啶���Ɖp����������ʂ��� */
			//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
			//	Jan. 31, 2002 genta
			//	�啶���E�������̋�ʂ͐��K�\���̐ݒ�Ɋւ�炸�ۑ�����
			//::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 0 );

			// 2001/06/23 Norio Nakatani
			/* �P��P�ʂŌ��� */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), TRUE );
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
				pcEditView->HandleCommand( F_SEARCH_PREV, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
//				/* �ĕ`�� */
//				pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
				/* �����_�C�A���O�������I�ɕ��� */
				if( m_pShareData->m_Common.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
			}
//From Here Feb. 20, 2001 JEPRO �u�u���v�_�C�A���O�Ɠ����悤�Ɍx�����b�Z�[�W��\������悤�ɕύX
//		}else
//		if( 0 == nRet ){
//			::MessageBeep( MB_ICONHAND );
//			CloseDialog( 0 );
//		}
//		return TRUE;
//�����܂ŃR�����g�A�E�g���A����Ɉȉ���ǉ�
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"�����������w�肵�Ă��������B"
			);
		}
		return TRUE;
//To Here Feb. 20, 2001
	case IDC_BUTTON_SEARCHNEXT:		/* ������ */	//Feb. 13, 2001 JEPRO �{�^������[IDOK]��[IDC_BUTTON_SERACHNEXT]�ɕύX
		/* �_�C�A���O�f�[�^�̎擾 */
		nRet = GetData();
		if( 0 < nRet ){
			if( m_bModal ){		/* ���[�_���_�C�A���O�� */
				CloseDialog( 2 );
			}else{
				/* �������� */
				pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
//				/* �ĕ`�� */
//				pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
				/* �����_�C�A���O�������I�ɕ��� */
				if( m_pShareData->m_Common.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
				// 2001.12.03 hor
				//	�_�C�A���O����Ȃ��Ƃ��AIDC_COMBO_TEXT ��� Enter �����ꍇ��
				//	�L�����b�g���\������Ȃ��Ȃ�̂��������
				else{
				::SendMessage(m_hWnd,WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDC_COMBO_TEXT ),TRUE);
				}
			}
//From Here Feb. 20, 2001 JEPRO �u�u���v�_�C�A���O�Ɠ����悤�Ɍx�����b�Z�[�W��\������悤�ɕύX
//		}else
//		if( 0 == nRet ){
//			::MessageBeep( MB_ICONHAND );
//			CloseDialog( 0 );
//		}
//		return TRUE;
//�����܂ŃR�����g�A�E�g���A����Ɉȉ���ǉ�
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"�����������w�肵�Ă��������B"
			);
		}
		return TRUE;
//To Here Feb. 20, 2001
	case IDC_BUTTON_SETMARK:	//2002.01.16 hor �Y���s�}�[�N
		if( 0 < GetData() ){
			if( m_bModal ){		/* ���[�_���_�C�A���O�� */
				CloseDialog( 2 );
			}else{
				pcEditView->HandleCommand( F_BOOKMARK_PATTERN, FALSE, 0, 0, 0, 0 );
				/* �����_�C�A���O�������I�ɕ��� */
				if( m_pShareData->m_Common.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
				else{
				::SendMessage(m_hWnd,WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDC_COMBO_TEXT ),TRUE);
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

/*[EOF]*/
