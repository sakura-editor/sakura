/*!	@file
	@brief �^�O�t�@�C���쐬�_�C�A���O�{�b�N�X

	@author MIK
	@date 2003.5.12
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "stdafx.h"
#include <string.h>
#include "sakura_rc.h"
#include "dlg/CDlgTagsMake.h"
#include "debug/Debug.h"
#include "doc/CEditDoc.h"
#include "global.h"
#include "funccode.h"
#include "mymessage.h"
#include "util/shell.h"

#include "sakura.hh"
const DWORD p_helpids[] = {	//13700
	IDC_EDIT_TAG_MAKE_FOLDER,	HIDC_EDIT_TAG_MAKE_FOLDER,	//�^�O�쐬�t�H���_
	IDC_BUTTON_TAG_MAKE_REF,	HIDC_BUTTON_TAG_MAKE_REF,	//�Q��
	IDC_EDIT_TAG_MAKE_CMDLINE,	HIDC_EDIT_TAG_MAKE_CMDLINE,	//�R�}���h���C��
	IDC_CHECK_TAG_MAKE_RECURSE,	HIDC_CHECK_TAG_MAKE_RECURSE,	//�T�u�t�H���_���Ώ�
	IDOK,						HIDC_TAG_MAKE_IDOK,
	IDCANCEL,					HIDC_TAG_MAKE_IDCANCEL,
	IDC_BUTTON_HELP,			HIDC_BUTTON_TAG_MAKE_HELP,
//	IDC_STATIC,						-1,
	0, 0
};

CDlgTagsMake::CDlgTagsMake()
{
	_tcscpy( m_szPath, _T("") );
	_tcscpy( m_szTagsCmdLine, _T("") );
	m_nTagsOpt = 0;
	return;
}

/* ���[�_���_�C�A���O�̕\�� */
int CDlgTagsMake::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	const TCHAR*	pszPath		//�p�X
)
{
	_tcscpy( m_szPath, pszPath );

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_TAG_MAKE, lParam );
}

BOOL CDlgTagsMake::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* �w���v */
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TAGS_MAKE) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;

	case IDC_BUTTON_TAG_MAKE_REF:	/* �Q�� */
		SelectFolder( GetHwnd() );
		return TRUE;

	case IDOK:
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;

	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

/*!
	�t�H���_��I������
	
	@param hwndDlg [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void CDlgTagsMake::SelectFolder( HWND hwndDlg )
{
	TCHAR	szPath[_MAX_PATH + 1];

	/* �t�H���_ */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TAG_MAKE_FOLDER, szPath, _MAX_PATH );

	if( SelectDir( hwndDlg, _T("�^�O�쐬�t�H���_�̑I��"), szPath, szPath ) )
	{
		//������\\�}�[�N��ǉ�����D
		int pos = _tcslen( szPath );
		if( pos > 0 && szPath[ pos - 1 ] != _T('\\') )
		{
			szPath[ pos     ] = _T('\\');
			szPath[ pos + 1 ] = _T('\0');
		}

		::DlgItem_SetText( hwndDlg, IDC_EDIT_TAG_MAKE_FOLDER, szPath );
	}
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgTagsMake::SetData( void )
{
	//�쐬�t�H���_
	::SendMessage( ::GetDlgItem( GetHwnd(), IDC_EDIT_TAG_MAKE_FOLDER ), CB_LIMITTEXT, _countof( m_szPath ), 0 );
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_TAG_MAKE_FOLDER, m_szPath );

	//�I�v�V����
	m_nTagsOpt = m_pShareData->m_nTagsOpt;
	if( m_nTagsOpt & 0x0001 ) ::CheckDlgButton( GetHwnd(), IDC_CHECK_TAG_MAKE_RECURSE, TRUE );

	//�R�}���h���C��
	::SendMessage( ::GetDlgItem( GetHwnd(), IDC_EDIT_TAG_MAKE_CMDLINE ), CB_LIMITTEXT, _countof( m_pShareData->m_szTagsCmdLine ), 0 );
	_tcscpy( m_szTagsCmdLine, m_pShareData->m_szTagsCmdLine );
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_TAG_MAKE_CMDLINE, m_pShareData->m_szTagsCmdLine );

	return;
}

/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[ */
int CDlgTagsMake::GetData( void )
{
	//�t�H���_
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_TAG_MAKE_FOLDER, m_szPath, _countof( m_szPath ) );
	int length = _tcslen( m_szPath );
	if( length > 0 )
	{
		if( m_szPath[ length - 1 ] != _T('\\') ) _tcscat( m_szPath, _T("\\") );
	}

	//CTAGS�I�v�V����
	m_nTagsOpt = 0;
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_TAG_MAKE_RECURSE ) == BST_CHECKED ) m_nTagsOpt |= 0x0001;
	m_pShareData->m_nTagsOpt = m_nTagsOpt;

	//�R�}���h���C��
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_TAG_MAKE_CMDLINE, m_szTagsCmdLine, _countof( m_szTagsCmdLine ) );
	_tcscpy( m_pShareData->m_szTagsCmdLine, m_szTagsCmdLine );

	return TRUE;
}

LPVOID CDlgTagsMake::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}


