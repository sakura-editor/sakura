/*!	@file
	@brief GREP�_�C�A���O�{�b�N�X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta
	Copyright (C) 2002, MIK, genta, Moca, YAZAKI
	Copyright (C) 2003, Moca
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>
#include "sakura_rc.h"
#include "dlg/CDlgGrep.h"
#include "debug/Debug.h"
#include "global.h"
#include "funccode.h"		// Stonee, 2001/03/12
#include "util/module.h"
#include "util/file.h"
#include "util/shell.h"
#include "CBregexp.h"
#include "util/os.h"
#include "CSakuraEnvironment.h"

//GREP CDlgGrep.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12000
	IDC_BUTTON_FOLDER,				HIDC_GREP_BUTTON_FOLDER,			//�t�H���_
	IDC_BUTTON_CURRENTFOLDER,		HIDC_GREP_BUTTON_CURRENTFOLDER,		//���t�H���_
	IDOK,							HIDOK_GREP,							//����
	IDCANCEL,						HIDCANCEL_GREP,						//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_GREP_BUTTON_HELP,				//�w���v
	IDC_CHK_WORD,					HIDC_GREP_CHK_WORD,					//�P��P��
	IDC_CHK_SUBFOLDER,				HIDC_GREP_CHK_SUBFOLDER,			//�T�u�t�H���_������
	IDC_CHK_FROMTHISTEXT,			HIDC_GREP_CHK_FROMTHISTEXT,			//���̃t�@�C������
	IDC_CHK_LOHICASE,				HIDC_GREP_CHK_LOHICASE,				//�啶��������
	IDC_CHK_REGULAREXP,				HIDC_GREP_CHK_REGULAREXP,			//���K�\��
	IDC_COMBO_CHARSET,				HIDC_GREP_COMBO_CHARSET,			//�����R�[�h�Z�b�g
	IDC_COMBO_TEXT,					HIDC_GREP_COMBO_TEXT,				//����
	IDC_COMBO_FILE,					HIDC_GREP_COMBO_FILE,				//�t�@�C��
	IDC_COMBO_FOLDER,				HIDC_GREP_COMBO_FOLDER,				//�t�H���_
	IDC_RADIO_OUTPUTLINE,			HIDC_GREP_RADIO_OUTPUTLINE,			//���ʏo�́F�s�P��
	IDC_RADIO_OUTPUTMARKED,			HIDC_GREP_RADIO_OUTPUTMARKED,		//���ʏo�́F�Y������
	IDC_RADIO_OUTPUTSTYLE1,			HIDC_GREP_RADIO_OUTPUTSTYLE1,		//���ʏo�͌`���F�m�[�}��
	IDC_RADIO_OUTPUTSTYLE2,			HIDC_GREP_RADIO_OUTPUTSTYLE2,		//���ʏo�͌`���F�t�@�C����
	IDC_STATIC_JRE32VER,			HIDC_GREP_STATIC_JRE32VER,			//���K�\���o�[�W����
	IDC_CHK_DEFAULTFOLDER,			HIDC_GREP_CHK_DEFAULTFOLDER,		//�t�H���_�̏����l���J�����g�t�H���_�ɂ���
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgGrep::CDlgGrep()
{
	m_bSubFolder = FALSE;				// �T�u�t�H���_�������������
	m_bFromThisText = FALSE;			// ���̕ҏW���̃e�L�X�g���猟������
	m_sSearchOption.Reset();			// �����I�v�V����
	m_nGrepCharSet = CODE_SJIS;			// �����R�[�h�Z�b�g
	m_bGrepOutputLine = TRUE;			// �s���o�͂��邩�Y�����������o�͂��邩
	m_nGrepOutputStyle = 1;				// Grep: �o�͌`��

	wcscpy( m_szText, m_pShareData->m_aSearchKeys[0] );		/* ���������� */
	_tcscpy( m_szFile, m_pShareData->m_aGrepFiles[0] );		/* �����t�@�C�� */
	_tcscpy( m_szFolder, m_pShareData->m_aGrepFolders[0] );	/* �����t�H���_ */
	return;
}



/* ���[�_���_�C�A���O�̕\�� */
int CDlgGrep::DoModal( HINSTANCE hInstance, HWND hwndParent, const TCHAR* pszCurrentFilePath )
{
	m_bSubFolder = m_pShareData->m_Common.m_sSearch.m_bGrepSubFolder;			// Grep: �T�u�t�H���_������
	m_sSearchOption = m_pShareData->m_Common.m_sSearch.m_sSearchOption;		// �����I�v�V����
	m_nGrepCharSet = m_pShareData->m_Common.m_sSearch.m_nGrepCharSet;			// �����R�[�h�Z�b�g
	m_bGrepOutputLine = m_pShareData->m_Common.m_sSearch.m_bGrepOutputLine;	// �s���o�͂��邩�Y�����������o�͂��邩
	m_nGrepOutputStyle = m_pShareData->m_Common.m_sSearch.m_nGrepOutputStyle;	// Grep: �o�͌`��


	_tcscpy(m_szCurrentFilePath, pszCurrentFilePath);

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_GREP, NULL );
}

//	2007.02.09 bosagami
LRESULT CALLBACK OnFolderProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
WNDPROC g_pOnFolderProc;

BOOL CDlgGrep::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );

	/* ���[�U�[���R���{�{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	::SendMessage( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	::SendMessage( ::GetDlgItem( GetHwnd(), IDC_COMBO_FILE ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	::SendMessage( ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );

	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_COMBO_FILE ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );

	/* �_�C�A���O�̃A�C�R�� */
//2002.02.08 Grep�A�C�R�����傫���A�C�R���Ə������A�C�R����ʁX�ɂ���B
	HICON	hIconBig, hIconSmall;
	//	Dec, 2, 2002 genta �A�C�R���ǂݍ��ݕ��@�ύX
	hIconBig   = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, false );
	hIconSmall = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)hIconBig );

	// 2002/09/22 Moca Add
	int i;
	/* �����R�[�h�Z�b�g�I���R���{�{�b�N�X������ */
	CCodeTypesForCombobox cCodeTypes;
	for( i = 0; i < cCodeTypes.GetCount(); ++i ){
		int idx = ::SendMessage( ::GetDlgItem( GetHwnd(), IDC_COMBO_CHARSET ), CB_ADDSTRING,   0, (LPARAM)cCodeTypes.GetName(i) );
		::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_COMBO_CHARSET ), CB_SETITEMDATA, idx, cCodeTypes.GetCode(i) );
	}
	//	2007.02.09 bosagami
	HWND hFolder = ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER );
	DragAcceptFiles(hFolder, true);
	g_pOnFolderProc = (WNDPROC)GetWindowLongPtr(hFolder, GWLP_WNDPROC);
	SetWindowLongPtr(hFolder, GWLP_WNDPROC, (LONG_PTR)OnFolderProc);


	/* ���N���X�����o */
//	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}

/*! @brief �t�H���_�w��EditBox�̃R�[���o�b�N�֐�

	@date 2007.02.09 bosagami �V�K�쐬
	@date 2007.09.02 genta �f�B���N�g���`�F�b�N������
*/
LRESULT CALLBACK OnFolderProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	if(msg == WM_DROPFILES) 
	do {
		//	From Here 2007.09.02 genta 
		SFilePath sPath;
		if( DragQueryFile((HDROP)wparam, 0, NULL, 0 ) > _countof2(sPath) - 1 ){
			// skip if the length of the path exceeds buffer capacity
			break;
		}
		DragQueryFile((HDROP)wparam, 0, sPath, _countof2(sPath) - 1);

		//�t�@�C���p�X�̉���
		CSakuraEnvironment::ResolvePath(sPath);
		
		//	�t�@�C�����h���b�v���ꂽ�ꍇ�̓t�H���_��؂�o��
		//	�t�H���_�̏ꍇ�͍Ōオ������̂�split���Ă͂����Ȃ��D
		if( IsFileExists( sPath, true )){	//	��2������true���ƃf�B���N�g���͑ΏۊO
			SFilePath szWork;
			SplitPath_FolderAndFile( sPath, szWork, NULL );
			_tcscpy( sPath, szWork );
		}

		SetWindowText(hwnd, sPath);
	}
	while(0);

	return  CallWindowProc((WNDPROC)g_pOnFolderProc,hwnd,msg,wparam,lparam);
}

BOOL CDlgGrep::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �uGrep�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_GREP_DIALOG) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDC_CHK_FROMTHISTEXT:	/* ���̕ҏW���̃e�L�X�g���猟������ */
		if( 0 < (int)_tcslen(m_szCurrentFilePath ) ){
			if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_FROMTHISTEXT ) ){
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_FILE ), FALSE );
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER ), FALSE );
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_FOLDER ), FALSE );
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_SUBFOLDER ), FALSE );
				::CheckDlgButton( GetHwnd(), IDC_CHK_SUBFOLDER, 0 );
			}else{
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_FILE ), TRUE );
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER ), TRUE );
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_FOLDER ), TRUE );
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_SUBFOLDER ), TRUE );
			}
			TCHAR	szWorkFolder[MAX_PATH];
			TCHAR	szWorkFile[MAX_PATH];
			// 2003.08.01 Moca �t�@�C�����̓X�y�[�X�Ȃǂ͋�؂�L���ɂȂ�̂ŁA""�ň͂��A�G�X�P�[�v����
			szWorkFile[0] = _T('"');
			SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile + 1 );
			_tcscat( szWorkFile, _T("\"") ); // 2003.08.01 Moca
			::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, szWorkFolder );
			::DlgItem_SetText( GetHwnd(), IDC_COMBO_FILE, szWorkFile );
		}
		return TRUE;
	case IDC_BUTTON_CURRENTFOLDER:	/* ���ݕҏW���̃t�@�C���̃t�H���_ */
		/* �t�@�C�����J���Ă��邩 */
		if( 0 < _tcslen( m_szCurrentFilePath ) ){
			TCHAR	szWorkFolder[MAX_PATH];
			TCHAR	szWorkFile[MAX_PATH];
			SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile );
			::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, szWorkFolder );
		}
		else{
			/* ���݂̃v���Z�X�̃J�����g�f�B���N�g�����擾���܂� */
			TCHAR	szWorkFolder[MAX_PATH];
			::GetCurrentDirectory( _countof( szWorkFolder ) - 1, szWorkFolder );
			::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, szWorkFolder );
		}
		return TRUE;


//	case IDC_CHK_LOHICASE:	/* �p�啶���Ɖp����������ʂ��� */
//		MYTRACE_A( "IDC_CHK_LOHICASE\n" );
//		return TRUE;
	case IDC_CHK_REGULAREXP:	/* ���K�\�� */
//		MYTRACE_A( "IDC_CHK_REGULAREXP ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) = %d\n", ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) ){
			// From Here Jun. 26, 2001 genta
			//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
			if( !CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 0 );
			}else{
				//	To Here Jun. 26, 2001 genta
				/* �p�啶���Ɖp����������ʂ��� */
				//	���K�\���̂Ƃ����I���ł���悤�ɁB
//				::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 1 );
//				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_LOHICASE ), FALSE );

				//2001/06/23 N.Nakatani
				/* �P��P�ʂŌ��� */
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), FALSE );
			}
		}else{
			/* �p�啶���Ɖp����������ʂ��� */
			//	���K�\���̂Ƃ����I���ł���悤�ɁB
//			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_LOHICASE ), TRUE );
//			::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 0 );


//2001/06/23 N.Nakatani
//�P��P�ʂ�grep���������ꂽ��R�����g���O���Ǝv���܂�
//2002/03/07�������Ă݂��B
			/* �P��P�ʂŌ��� */
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), TRUE );

		}
		return TRUE;

	case IDC_BUTTON_FOLDER:
		/* �t�H���_�Q�ƃ{�^�� */
		{
			TCHAR	szFolder[MAX_PATH];
			/* �����t�H���_ */
			::DlgItem_GetText( GetHwnd(), IDC_COMBO_FOLDER, szFolder, _MAX_PATH - 1 );
			if( 0 == _tcslen( szFolder ) ){
				::GetCurrentDirectory( _countof( szFolder ), szFolder );
			}
			if( SelectDir( GetHwnd(), _T("��������t�H���_��I��ł�������"), szFolder, szFolder ) ){
				::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, szFolder );
			}
		}

		return TRUE;
	case IDC_CHK_DEFAULTFOLDER:
		/* �t�H���_�̏����l���J�����g�t�H���_�ɂ��� */
		{
			m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder = ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_DEFAULTFOLDER );
		}
		return TRUE;
	case IDOK:
		/* �_�C�A���O�f�[�^�̎擾 */
		if( GetData() ){
//			::EndDialog( hwndDlg, TRUE );
			CloseDialog( TRUE );
		}
		return TRUE;
	case IDCANCEL:
//		::EndDialog( hwndDlg, FALSE );
		CloseDialog( FALSE );
		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}



/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgGrep::SetData( void )
{
	m_pShareData = CShareData::getInstance()->GetShareData();

	/* ���������� */
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT, m_szText );
	HWND	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT );
	for( int i = 0; i < m_pShareData->m_aSearchKeys.size(); ++i ){
		Combo_AddString( hwndCombo, m_pShareData->m_aSearchKeys[i] );
	}

	/* �����t�@�C�� */
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_FILE, m_szFile );
	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_FILE );
	for( int i = 0; i < m_pShareData->m_aGrepFiles.size(); ++i ){
		Combo_AddString( hwndCombo, m_pShareData->m_aGrepFiles[i] );
	}

	/* �����t�H���_ */
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, m_szFolder );
	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER );
	for( int i = 0; i < m_pShareData->m_aGrepFolders.size(); ++i ){
		Combo_AddString( hwndCombo, m_pShareData->m_aGrepFolders[i] );
	}

	if((0 == _tcslen( m_pShareData->m_aGrepFolders[0] ) || m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder ) &&
		0 < _tcslen( m_szCurrentFilePath )
	){
		TCHAR	szWorkFolder[MAX_PATH];
		TCHAR	szWorkFile[MAX_PATH];
		SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile );
		::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, szWorkFolder );
	}

	/* ���̕ҏW���̃e�L�X�g���猟������ */
	::CheckDlgButton( GetHwnd(), IDC_CHK_FROMTHISTEXT, m_bFromThisText );
	if( 0 < _tcslen( m_szCurrentFilePath ) ){
		if( m_bFromThisText ){
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_FILE ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_FOLDER ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_SUBFOLDER ), FALSE );
			TCHAR	szWorkFolder[MAX_PATH];
			TCHAR	szWorkFile[MAX_PATH];
			SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile );
			::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, szWorkFolder );
			::DlgItem_SetText( GetHwnd(), IDC_COMBO_FILE, szWorkFile );
		}else{
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_FILE ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_FOLDER ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_SUBFOLDER ), TRUE );
		}
	}

	/* �T�u�t�H���_������������� */
	::CheckDlgButton( GetHwnd(), IDC_CHK_SUBFOLDER, m_bSubFolder );

	/* �p�啶���Ɖp����������ʂ��� */
	::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, m_sSearchOption.bLoHiCase );

	// 2001/06/23 N.Nakatani �����_�ł�Grep�ł͒P��P�ʂ̌����̓T�|�[�g�ł��Ă��܂���
	// 2002/03/07 �e�X�g�T�|�[�g
	/* ��v����P��̂݌������� */
	::CheckDlgButton( GetHwnd(), IDC_CHK_WORD, m_sSearchOption.bWordOnly );
//	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ) , false );	//�`�F�b�N�{�b�N�X���g�p�s�ɂ���


	/* �����R�[�h�������� */
//	::CheckDlgButton( GetHwnd(), IDC_CHK_KANJICODEAUTODETECT, m_bKanjiCode_AutoDetect );

	// 2002/09/22 Moca Add
	/* �����R�[�h�Z�b�g */
	{
		int		nIdx, nCurIdx;
		ECodeType nCharSet;
		HWND	hWndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_CHARSET );
		nCurIdx = ::SendMessageAny( hWndCombo , CB_GETCURSEL, 0, 0 );
		CCodeTypesForCombobox cCodeTypes;
		for( nIdx = 0; nIdx < cCodeTypes.GetCount(); nIdx++ ){
			nCharSet = (ECodeType)::SendMessageAny( hWndCombo, CB_GETITEMDATA, nIdx, 0 );
			if( nCharSet == m_nGrepCharSet ){
				nCurIdx = nIdx;
			}
		}
		::SendMessageAny( hWndCombo, CB_SETCURSEL, (WPARAM)nCurIdx, 0 );
	}

	/* �s���o�͂��邩�Y�����������o�͂��邩 */
	if( m_bGrepOutputLine ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUTLINE, TRUE );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUTMARKED, TRUE );
	}

	/* Grep: �o�͌`�� */
	if( 1 == m_nGrepOutputStyle ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUTSTYLE1, TRUE );
	}else
	if( 2 == m_nGrepOutputStyle ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUTSTYLE2, TRUE );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUTSTYLE1, TRUE );
	}

	// From Here Jun. 29, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if( CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, false )
		&& m_sSearchOption.bRegularExp){
		/* �p�啶���Ɖp����������ʂ��� */
		::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 1 );
		//	���K�\���̂Ƃ����I���ł���悤�ɁB
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

	if( 0 < _tcslen( m_szCurrentFilePath ) ){
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_FROMTHISTEXT ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_FROMTHISTEXT ), FALSE );
	}

	// �t�H���_�̏����l���J�����g�t�H���_�ɂ���
	::CheckDlgButton( GetHwnd(), IDC_CHK_DEFAULTFOLDER, m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder );
	if( m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder ) OnBnClicked( IDC_BUTTON_CURRENTFOLDER );

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[  */
int CDlgGrep::GetData( void )
{
	m_pShareData = CShareData::getInstance()->GetShareData();

	/* �T�u�t�H���_�������������*/
	m_bSubFolder = ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_SUBFOLDER );

	m_pShareData->m_Common.m_sSearch.m_bGrepSubFolder = m_bSubFolder;		/* Grep�F�T�u�t�H���_������ */

	/* ���̕ҏW���̃e�L�X�g���猟������ */
	m_bFromThisText = ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_FROMTHISTEXT );
	/* �p�啶���Ɖp����������ʂ��� */
	m_sSearchOption.bLoHiCase = (0!=::IsDlgButtonChecked( GetHwnd(), IDC_CHK_LOHICASE ));

	//2001/06/23 N.Nakatani
	/* �P��P�ʂŌ��� */
	m_sSearchOption.bWordOnly = (0!=::IsDlgButtonChecked( GetHwnd(), IDC_CHK_WORD ));

	/* ���K�\�� */
	m_sSearchOption.bRegularExp = (0!=::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ));

	/* �����R�[�h�������� */
//	m_bKanjiCode_AutoDetect = ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_KANJICODEAUTODETECT );

	/* �����R�[�h�Z�b�g */
	{
		int		nIdx;
		HWND	hWndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_CHARSET );
		nIdx = ::SendMessageAny( hWndCombo, CB_GETCURSEL, 0, 0 );
		m_nGrepCharSet = (ECodeType)::SendMessageAny( hWndCombo, CB_GETITEMDATA, nIdx, 0 );
	}


	/* �s���o�͂��邩�Y�����������o�͂��邩 */
	m_bGrepOutputLine = ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_OUTPUTLINE );

	/* Grep: �o�͌`�� */
	if( TRUE == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_OUTPUTSTYLE1 ) ){
		m_nGrepOutputStyle = 1;				/* Grep: �o�͌`�� */
	}
	if( TRUE == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_OUTPUTSTYLE2 ) ){
		m_nGrepOutputStyle = 2;				/* Grep: �o�͌`�� */
	}



	/* ���������� */
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_TEXT, m_szText, _countof(m_szText) );
	/* �����t�@�C�� */
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_FILE, m_szFile, _countof2(m_szFile) );
	/* �����t�H���_ */
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_FOLDER, m_szFolder, _countof2(m_szFolder) );

	m_pShareData->m_Common.m_sSearch.m_sSearchOption = m_sSearchOption;		// �����I�v�V����
	m_pShareData->m_Common.m_sSearch.m_nGrepCharSet = m_nGrepCharSet;			// �����R�[�h��������
	m_pShareData->m_Common.m_sSearch.m_bGrepOutputLine = m_bGrepOutputLine;	// �s���o�͂��邩�Y�����������o�͂��邩
	m_pShareData->m_Common.m_sSearch.m_nGrepOutputStyle = m_nGrepOutputStyle;	// Grep: �o�͌`��


//��߂܂���
//	if( 0 == wcslen( m_szText ) ){
//		WarningMessage(	GetHwnd(), _T("�����̃L�[���[�h���w�肵�Ă��������B") );
//		return FALSE;
//	}
	/* ���̕ҏW���̃e�L�X�g���猟������ */
	if( 0 == _tcslen( m_szFile ) ){
		//	Jun. 16, 2003 Moca
		//	�����p�^�[�����w�肳��Ă��Ȃ��ꍇ�̃��b�Z�[�W�\������߁A
		//	�u*.*�v���w�肳�ꂽ���̂ƌ��Ȃ��D
		_tcscpy( m_szFile, _T("*.*") );
	}
	if( 0 == _tcslen( m_szFolder ) ){
		WarningMessage(	GetHwnd(), _T("�����Ώۃt�H���_���w�肵�Ă��������B") );
		return FALSE;
	}

	{
		//�J�����g�f�B���N�g����ۑ��B���̃u���b�N���甲����Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕��������B
		CCurrentDirectoryBackupPoint cCurDirBackup;
		
		// ���΃p�X����΃p�X
		if( !::SetCurrentDirectory( m_szFolder ) ){
			WarningMessage(	GetHwnd(), _T("�����Ώۃt�H���_������������܂���B") );
			return FALSE;
		}
		::GetCurrentDirectory( MAX_PATH, m_szFolder );
	}

//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeyArr()�ǉ��ɔ����ύX
	/* ���������� */
	if( 0 < wcslen( m_szText ) ){
		// From Here Jun. 26, 2001 genta
		//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
		int nFlag = 0;
		nFlag |= m_sSearchOption.bLoHiCase ? 0x01 : 0x00;
		if( m_sSearchOption.bRegularExp  && !CheckRegexpSyntax( m_szText, GetHwnd(), true, nFlag) ){
			return FALSE;
		}
		// To Here Jun. 26, 2001 genta ���K�\�����C�u���������ւ�
		CShareData::getInstance()->AddToSearchKeyArr( m_szText );
	}

	// ���̕ҏW���̃e�L�X�g���猟������ꍇ�A�����Ɏc���Ȃ�	Uchi 2008/5/23
	if (!m_bFromThisText) {
		/* �����t�@�C�� */
		CShareData::getInstance()->AddToGrepFileArr( m_szFile );

		/* �����t�H���_ */
		CShareData::getInstance()->AddToGrepFolderArr( m_szFolder );
	}

	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgGrep::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


