//	$Id$
/************************************************************************
	CDlgFind.cpp
	1999.12.2  �č쐬
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/

#include "CDlgFind.h"
#include "funccode.h"
#include "sakura_rc.h"
#include "CJre.h"
#include "CEditView.h"
#include "etc_uty.h"    //Stonee, 2001/03/12

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
	*  ������                    *
	*****************************/
	if( CJre::IsExist() ){	// jre.dll�����邩�ǂ����𔻒�
		CJre	cJre;
		WORD	wJreVersion;
		char	szMsg[256];
		cJre.Init();
		/* JRE32.DLL�̃o�[�W���� */
		wJreVersion = cJre.GetVersion();
		::wsprintf( szMsg, "jre32.dll Ver%x.%x", wJreVersion / 0x100, wJreVersion % 0x100 );
		::SetDlgItemText( m_hWnd, IDC_STATIC_JRE32VER, szMsg );
	}
	/* ���[�U�[���R���{ �{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );


	/*****************************
	*  �f�[�^�ݒ�                *
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
//	/* ��v����P��̂݌������� */
//	::CheckDlgButton( m_hWnd, IDC_CHK_WORDONLY, m_bWordOnly );
	/* ���K�\�� */
	::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, m_bRegularExp );

	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND );

	if( m_bRegularExp ){
		/* CJre�N���X�̏����� */
		CJre	cJre;
		cJre.Init();
		if( FALSE == cJre.IsExist() ){
			::MessageBeep( MB_ICONEXCLAMATION );
			::MessageBox( m_hWnd, "jre32.dll��������܂���B\n���K�\���𗘗p����ɂ�jre32.dll���K�v�ł��B\n", "���", MB_OK | MB_ICONEXCLAMATION );
			::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
		}else{
			/* �p�啶���Ɖp����������ʂ��� */
			::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );
		}
	}
	/* �����_�C�A���O�������I�ɕ��� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFind, m_pShareData->m_Common.m_bAutoCloseDlgFind );

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
int CDlgFind::GetData( void )
{
//	MYTRACE( "CDlgFind::GetData()" );
//	int			i;
//	int			j;
//	CMemory*	pcmWork;
	CJre	cJre;

	/* �p�啶���Ɖp����������ʂ��� */
	m_bLoHiCase = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_LOHICASE );
//	/* ��v����P��̂݌������� */
//	m_bWordOnly = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_WORDONLY );
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

	if( 0 < lstrlen( m_szText ) ){
		/* ���K�\���H */
		if( m_bRegularExp ){
			/* CJre�N���X�̏����� */
			cJre.Init();
			/* jre32.dll�̑��݃`�F�b�N */
			if( FALSE == cJre.IsExist() ){
				::MessageBox( m_hWnd, "jre32.dll��������܂���B\n���K�\���𗘗p����ɂ�jre32.dll���K�v�ł��B\n", "���", MB_OK | MB_ICONEXCLAMATION );
				return -1;
			}
			/* �����p�^�[���̃R���p�C�� */
			if( !cJre.Compile( m_szText ) ){
				char	szMsg[512];
				cJre.GetJreMessage( GJM_JPN, szMsg );
				::MessageBox( m_hWnd, szMsg, "���K�\���G���[���", MB_OK | MB_ICONSTOP );
				return -1;
			}
		}
		/* ���������� */
		AddToSearchKeyArr( (const char*)m_szText );
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
			/* CJre�N���X�̏����� */
			CJre	cJre;
			cJre.Init();
			if( FALSE == CJre::IsExist() ){
				/* JRE32.DLL�̃o�[�W���� */
				::SetDlgItemText( m_hWnd, IDC_STATIC_JRE32VER, "" );
				::MessageBeep( MB_ICONEXCLAMATION );
				::MessageBox( m_hWnd, "jre32.dll��������܂���B\n���K�\���𗘗p����ɂ�jre32.dll���K�v�ł��B\n", "���", MB_OK | MB_ICONEXCLAMATION );
				::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
			}else{
				CJre	cJre;
				WORD	wJreVersion;
				char	szMsg[256];
				cJre.Init();
				/* JRE32.DLL�̃o�[�W���� */
				wJreVersion = cJre.GetVersion();
				wsprintf( szMsg, "jre32.dll Ver%x.%x", wJreVersion / 0x100, wJreVersion % 0x100 );
				::SetDlgItemText( m_hWnd, IDC_STATIC_JRE32VER, szMsg );

				/* �p�啶���Ɖp����������ʂ��� */
				::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );
			}
		}else{
			/* �p�啶���Ɖp����������ʂ��� */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
			::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 0 );
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
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}


void CDlgFind::AddToSearchKeyArr( const char* pszKey )
{
	CMemory*	pcmWork;
	int			i;
	int			j;
	pcmWork = NULL;
	pcmWork = new CMemory( pszKey, lstrlen( pszKey ) );
	for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
		if( 0 == strcmp( pszKey, m_pShareData->m_szSEARCHKEYArr[i] ) ){
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
	pcmWork = NULL;
	return;
}


/*[EOF]*/
