/*
	�^�C�v�ʐݒ� - �x��

	2008.04.12 kobake CPropTypes.cpp���番��
*/
#include "StdAfx.h"
#include "CPropTypes.h"
#include "sakura.hh"
#include "util/module.h"
#include "util/shell.h"

static const DWORD p_helpids3[] = {	//11500
	IDC_BUTTON_HOKANFILE_REF,		HIDC_BUTTON_HOKANFILE_REF,			//���͕⊮ �P��t�@�C���Q��
	IDC_CHECK_HOKANLOHICASE,		HIDC_CHECK_HOKANLOHICASE,			//���͕⊮�̉p�啶��������
	IDC_CHECK_HOKANBYFILE,			HIDC_CHECK_HOKANBYFILE,				//���݂̃t�@�C��������͕⊮
	IDC_EDIT_HOKANFILE,				HIDC_EDIT_HOKANFILE,				//�P��t�@�C����
	IDC_EDIT_TYPEEXTHELP,			HIDC_EDIT_TYPEEXTHELP,				//�O���w���v�t�@�C����	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENHELP,		HIDC_BUTTON_TYPEOPENHELP,			//�O���w���v�t�@�C���Q��	// 2006.08.06 ryoji
	IDC_EDIT_TYPEEXTHTMLHELP,		HIDC_EDIT_TYPEEXTHTMLHELP,			//�O��HTML�w���v�t�@�C����	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENEXTHTMLHELP,	HIDC_BUTTON_TYPEOPENEXTHTMLHELP,	//�O��HTML�w���v�t�@�C���Q��	// 2006.08.06 ryoji
	IDC_CHECK_TYPEHTMLHELPISSINGLE,	HIDC_CHECK_TYPEHTMLHELPISSINGLE,	//�r���[�A�𕡐��N�����Ȃ�	// 2006.08.06 ryoji
	IDC_COMBO_DEFAULT_CODETYPE,		HIDC_COMBO_DEFAULT_CODETYPE,		//�f�t�H���g�����R�[�h
	IDC_CHECK_PRIOR_CESU8,			HIDC_CHECK_PRIOR_CESU8,				//�������ʎ���CESU-8��D�悷��
//	IDC_STATIC,						-1,
	0, 0
};

// 2001/06/13 Start By asa-o: �^�C�v�ʐݒ�̎x���^�u�Ɋւ��鏈��

/* ���b�Z�[�W���� */
INT_PTR CPropTypes::DispatchEvent_Support(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p2 */
		SetData_Support( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		/* ���͕⊮ �P��t�@�C�� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_HOKANFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );
		/* �L�[���[�h�w���v �����t�@�C�� *///	del 2008/7/4 Uchi
//		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
//		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			/* �_�C�A���O�f�[�^�̎擾 p2 */
			GetData_Support( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_HOKANFILE_REF:	/* ���͕⊮ �P��t�@�C���́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
					// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					if( _IS_REL_PATH( m_Types.m_szHokanFile ) ){
						GetInidirOrExedir( szPath, m_Types.m_szHokanFile );
					}else{
						_tcscpy( szPath, m_Types.m_szHokanFile );
					}
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.*"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szHokanFile, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENHELP:	/* �O���w���v�P�́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
					// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					if( _IS_REL_PATH( m_Types.m_szExtHelp ) ){
						GetInidirOrExedir( szPath, m_Types.m_szExtHelp, TRUE );
					}else{
						_tcscpy( szPath, m_Types.m_szExtHelp );
					}
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.hlp"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szExtHelp, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp );
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENEXTHTMLHELP:	/* �O��HTML�w���v�́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
					// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					if( _IS_REL_PATH( m_Types.m_szExtHtmlHelp ) ){
						GetInidirOrExedir( szPath, m_Types.m_szExtHtmlHelp, TRUE );
					}else{
						_tcscpy( szPath, m_Types.m_szExtHtmlHelp );
					}
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.chm;*.col"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szExtHtmlHelp, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp );
					}
				}
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
//		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
//		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:	//Jul. 03, 2001 JEPRO �x���^�u�̃w���v��L����
			OnHelp( hwndDlg, IDD_PROPTYPESP2 );
			return TRUE;
		case PSN_KILLACTIVE:
			/* �_�C�A���O�f�[�^�̎擾 p2 */
			GetData_Support( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
		case PSN_SETACTIVE:
			m_nPageNum = 2;
			return TRUE;
		}
		break;

//From Here Jul. 05, 2001 JEPRO: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids3 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//To Here  Jul. 05, 2001

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids3 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CPropTypes::SetData_Support( HWND hwndDlg )
{
	/* ���͕⊮ �P��t�@�C�� */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );

//	2001/06/19 asa-o
	/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANLOHICASE, m_Types.m_bHokanLoHiCase );

	// 2003.06.25 Moca �t�@�C������̕⊮�@�\
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANBYFILE, m_Types.m_bUseHokanByFile );

	//@@@ 2002.2.2 YAZAKI
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp );
	::CheckDlgButton( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE, m_Types.m_bHtmlHelpIsSingle );

	/* �u�����R�[�h�v�O���[�v�̐ݒ� */
	{
		int i;
		HWND hCombo;
		static const wchar_t* aszStr[] = {
			L"SJIS",
			L"EUC",
			L"UTF-8",
			L"CESU-8",
			L"Unicode",
			L"UnicodeBE"
		};
		static const ECodeType aeCodeType[] = {
			CODE_SJIS,
			CODE_EUC,
			CODE_UTF8,
			CODE_CESU8,
			CODE_UNICODE,
			CODE_UNICODEBE
		};

		// �u�����F������CESU-8��D��vm_Types.m_bPriorCesu8 ���`�F�b�N
		::CheckDlgButton( hwndDlg, IDC_CHECK_PRIOR_CESU8, m_Types.m_bPriorCesu8 );

		// �f�t�H���g�R�[�h�^�C�v�̃R���{�{�b�N�X�ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		for( i = 0; i < _countof(aszStr); ++i ){
			ApiWrap::Combo_AddString( hCombo, aszStr[i] );
		}
		for( i = 0; i < _countof(aeCodeType); ++i ){
			if( m_Types.m_eDefaultCodetype == aeCodeType[i] ){
				break;
			}
		}
		if( i == _countof(aeCodeType) ){
			i = 0;
		}
		SendMessageAny( hCombo, CB_SETCURSEL, (WPARAM)i, 0 );
	}
}

/* �_�C�A���O�f�[�^�̎擾 */
int CPropTypes::GetData_Support( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = 2;

//	2001/06/19	asa-o
	/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
	m_Types.m_bHokanLoHiCase = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANLOHICASE );

	m_Types.m_bUseHokanByFile = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANBYFILE );

	/* ���͕⊮ �P��t�@�C�� */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile, _countof2( m_Types.m_szHokanFile ));

	//@@@ 2002.2.2 YAZAKI
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp, _countof2( m_Types.m_szExtHelp ));
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp, _countof2( m_Types.m_szExtHtmlHelp ));
	m_Types.m_bHtmlHelpIsSingle = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE );

	/* �u�����R�[�h�v�O���[�v�̐ݒ� */
	{
		int i;
		HWND hCombo;
		static const wchar_t* aszStr[] = {
			L"SJIS",
			L"EUC",
			L"UTF-8",
			L"CESU-8",
			L"Unicode",
			L"UnicodeBE"
		};
		static const ECodeType aeCodeType[] = {
			CODE_SJIS,
			CODE_EUC,
			CODE_UTF8,
			CODE_CESU8,
			CODE_UNICODE,
			CODE_UNICODEBE
		};

		// m_Types.m_bPriorCesu8 ��ݒ�
		m_Types.m_bPriorCesu8 = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_PRIOR_CESU8 );

		// m_Types.eDefaultCodetype ��ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		i = ::SendMessageAny( hCombo, CB_GETCURSEL, 0, 0 );
		if( CB_ERR != i ){
			m_Types.m_eDefaultCodetype = aeCodeType[i];
		}
	}


	return TRUE;
}

// 2001/06/13 End

