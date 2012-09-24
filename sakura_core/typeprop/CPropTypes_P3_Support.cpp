/*
	�^�C�v�ʐݒ� - �x��

	2008.04.12 kobake CPropTypes.cpp���番��
*/
#include "StdAfx.h"
#include "CPropTypes.h"
#include "dlg/CDlgOpenFile.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/file.h" // _IS_REL_PATH
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids3[] = {	//11500
	IDC_BUTTON_HOKANFILE_REF,		HIDC_BUTTON_HOKANFILE_REF,			//���͕⊮ �P��t�@�C���Q��
	IDC_COMBO_HOKAN_TYPE,			HIDC_COMBO_HOKAN_TYPE,				//���͕⊮�^�C�v
	IDC_CHECK_HOKANLOHICASE,		HIDC_CHECK_HOKANLOHICASE,			//���͕⊮�̉p�啶��������
	IDC_CHECK_HOKANBYFILE,			HIDC_CHECK_HOKANBYFILE,				//���݂̃t�@�C��������͕⊮
	IDC_EDIT_HOKANFILE,				HIDC_EDIT_HOKANFILE,				//�P��t�@�C����
	IDC_EDIT_TYPEEXTHELP,			HIDC_EDIT_TYPEEXTHELP,				//�O���w���v�t�@�C����	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENHELP,		HIDC_BUTTON_TYPEOPENHELP,			//�O���w���v�t�@�C���Q��	// 2006.08.06 ryoji
	IDC_EDIT_TYPEEXTHTMLHELP,		HIDC_EDIT_TYPEEXTHTMLHELP,			//�O��HTML�w���v�t�@�C����	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENEXTHTMLHELP,	HIDC_BUTTON_TYPEOPENEXTHTMLHELP,	//�O��HTML�w���v�t�@�C���Q��	// 2006.08.06 ryoji
	IDC_CHECK_TYPEHTMLHELPISSINGLE,	HIDC_CHECK_TYPEHTMLHELPISSINGLE,	//�r���[�A�𕡐��N�����Ȃ�	// 2006.08.06 ryoji
	IDC_COMBO_DEFAULT_CODETYPE,		HIDC_COMBO_DEFAULT_CODETYPE,		//�f�t�H���g�����R�[�h
	IDC_COMBO_DEFAULT_EOLTYPE,		HIDC_COMBO_DEFAULT_EOLTYPE,			//�f�t�H���g���s�R�[�h	// 2011.01.24 ryoji
	IDC_CHECK_DEFAULT_BOM,			HIDC_CHECK_DEFAULT_BOM,				//�f�t�H���gBOM	// 2011.01.24 ryoji
	IDC_CHECK_PRIOR_CESU8,			HIDC_CHECK_PRIOR_CESU8,				//�������ʎ���CESU-8��D�悷��
//	IDC_STATIC,						-1,
	0, 0
};

static const wchar_t* aszCodeStr[] = {
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
static const BOOL abBomEnable[] = {
	FALSE,
	FALSE,
	TRUE,
	TRUE,
	TRUE,
	TRUE
};
static const wchar_t* aszEolStr[] = {
	L"CR+LF",
	L"LF (UNIX)",
	L"CR (Mac)",
};
static const EEolType aeEolType[] = {
	EOL_CRLF,
	EOL_LF,
	EOL_CR,
};

struct SHokanMethod{
	int nMethod;
	std::wstring name;
};

static std::vector<SHokanMethod>* GetHokanMethodList()
{
	static std::vector<SHokanMethod> methodList;
	return &methodList;
}


// 2001/06/13 Start By asa-o: �^�C�v�ʐݒ�̎x���^�u�Ɋւ��鏈��

/* ���b�Z�[�W���� */
INT_PTR CPropSupport::DispatchEvent(
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
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		/* ���͕⊮ �P��t�@�C�� */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_HOKANFILE ), _MAX_PATH - 1 );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
//		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		case CBN_SELCHANGE:
			{
				int i;
				switch( wID ){
				case IDC_COMBO_DEFAULT_CODETYPE:
					// �����R�[�h�̕ύX��BOM�`�F�b�N�{�b�N�X�ɔ��f
					i = Combo_GetCurSel( (HWND) lParam );
					if( CB_ERR != i ){
						int nCheck = BST_UNCHECKED;
						if( abBomEnable[i] ){
							if( (aeCodeType[i] == CODE_UNICODE || aeCodeType[i] == CODE_UNICODEBE) )
								nCheck = BST_CHECKED;
						}
						::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, nCheck );
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), abBomEnable[i] );
					}
					break;
				}
			}
			break;

		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			/* �_�C�A���O�f�[�^�̎擾 p2 */
			GetData( hwndDlg );
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
			GetData( hwndDlg );
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
			MyWinHelp( (HWND)p->hItemHandle, m_pszHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids3 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//To Here  Jul. 05, 2001

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_pszHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids3 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CPropSupport::SetData( HWND hwndDlg )
{
	/* ���͕⊮ �P��t�@�C�� */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );

	{
		HWND hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_HOKAN_TYPE );
		std::vector<SHokanMethod>* pMedothList = GetHokanMethodList();
		ApiWrap::Combo_AddString( hCombo, L"�Ȃ�" );
		Combo_SetCurSel( hCombo, 0 );
		for( size_t i = 0; i < pMedothList->size(); i++ ){
			ApiWrap::Combo_AddString( hCombo, (*pMedothList)[i].name.c_str() );
			if( m_Types.m_nHokanType == (*pMedothList)[i].nMethod ){
				Combo_SetCurSel( hCombo, i + 1 );
			}
		}
	}

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

		// �u�����F������CESU-8��D��vm_Types.m_bPriorCesu8 ���`�F�b�N
		::CheckDlgButton( hwndDlg, IDC_CHECK_PRIOR_CESU8, m_Types.m_bPriorCesu8 );

		// �f�t�H���g�R�[�h�^�C�v�̃R���{�{�b�N�X�ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		for( i = 0; i < _countof(aszCodeStr); ++i ){
			ApiWrap::Combo_AddString( hCombo, aszCodeStr[i] );
		}
		for( i = 0; i < _countof(aeCodeType); ++i ){
			if( m_Types.m_eDefaultCodetype == aeCodeType[i] ){
				break;
			}
		}
		if( i == _countof(aeCodeType) ){
			i = 0;
		}
		Combo_SetCurSel( hCombo, i );

		// BOM �`�F�b�N�{�b�N�X�ݒ�
		if( !abBomEnable[i] )
			m_Types.m_bDefaultBom = FALSE;
		::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, m_Types.m_bDefaultBom );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), abBomEnable[i] );

		// �f�t�H���g���s�^�C�v�̃R���{�{�b�N�X�ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		for( i = 0; i < _countof(aszEolStr); ++i ){
			ApiWrap::Combo_AddString( hCombo, aszEolStr[i] );
		}
		for( i = 0; i < _countof(aeEolType); ++i ){
			if( m_Types.m_eDefaultEoltype == aeEolType[i] ){
				break;
			}
		}
		if( i == _countof(aeEolType) ){
			i = 0;
		}
		Combo_SetCurSel( hCombo, i );
	}
}

/* �_�C�A���O�f�[�^�̎擾 */
int CPropSupport::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = 2;

//	2001/06/19	asa-o
	/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
	m_Types.m_bHokanLoHiCase = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANLOHICASE );

	m_Types.m_bUseHokanByFile = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANBYFILE );

	/* ���͕⊮ �P��t�@�C�� */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile, _countof2( m_Types.m_szHokanFile ));

	// ���͕⊮���
	{
		HWND hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_HOKAN_TYPE );
		int i = Combo_GetCurSel( hCombo );
		if( 0 == i ){
			m_Types.m_nHokanType = 0;
		}else if( CB_ERR != i ){
			m_Types.m_nHokanType = (*GetHokanMethodList())[i - 1].nMethod;
		}
	}

	//@@@ 2002.2.2 YAZAKI
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp, _countof2( m_Types.m_szExtHelp ));
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp, _countof2( m_Types.m_szExtHtmlHelp ));
	m_Types.m_bHtmlHelpIsSingle = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE );

	/* �u�����R�[�h�v�O���[�v�̐ݒ� */
	{
		int i;
		HWND hCombo;

		// m_Types.m_bPriorCesu8 ��ݒ�
		m_Types.m_bPriorCesu8 = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_PRIOR_CESU8 );

		// m_Types.eDefaultCodetype ��ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		i = Combo_GetCurSel( hCombo );
		if( CB_ERR != i ){
			m_Types.m_eDefaultCodetype = aeCodeType[i];
		}

		// m_Types.m_bDefaultBom ��ݒ�
		m_Types.m_bDefaultBom = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DEFAULT_BOM );

		// m_Types.eDefaultEoltype ��ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		i = Combo_GetCurSel( hCombo );
		if( CB_ERR != i ){
			m_Types.m_eDefaultEoltype = aeEolType[i];
		}
	}


	return TRUE;
}

// 2001/06/13 End

/*! �⊮��ʂ̒ǉ�
/*/
void CPropSupport::AddHokanMethod(int nMethod, const WCHAR* szName)
{
	SHokanMethod item = { nMethod, std::wstring(szName) };
	GetHokanMethodList()->push_back(item);
}

