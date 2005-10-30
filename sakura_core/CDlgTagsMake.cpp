//	$Id$
/*!	@file
	@brief �^�O�t�@�C���쐬�_�C�A���O�{�b�N�X

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 2003, MIK

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
#include "CDlgTagsMake.h"
#include "etc_uty.h"
#include "debug.h"
#include "CEditDoc.h"
#include "global.h"
#include "funccode.h"
#include "mymessage.h"

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
	strcpy( m_szPath, "" );
	strcpy( m_szTagsCmdLine, "" );
	m_nTagsOpt = 0;
	return;
}

/* ���[�_���_�C�A���O�̕\�� */
int CDlgTagsMake::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam,
	const char*	pszPath		//�p�X
)
{
	strcpy( m_szPath, pszPath );

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_TAG_MAKE, lParam );
}

BOOL CDlgTagsMake::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* �w���v */
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TAGS_MAKE) );
		return TRUE;

	case IDC_BUTTON_TAG_MAKE_REF:	/* �Q�� */
		SelectFolder( m_hWnd );
		return TRUE;

	case IDOK:
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( m_hWnd, GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
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
	char	szPath[_MAX_PATH + 1];

	/* �t�H���_ */
	::GetDlgItemText( hwndDlg, IDC_EDIT_TAG_MAKE_FOLDER, szPath, _MAX_PATH );

	if( SelectDir( hwndDlg, "�^�O�쐬�t�H���_�̑I��", szPath, szPath ) )
	{
		//������\\�}�[�N��ǉ�����D
		int pos = strlen( szPath );
		if( pos > 0 && szPath[ pos - 1 ] != '\\' )
		{
			szPath[ pos     ] = '\\';
			szPath[ pos + 1 ] = '\0';
		}

		::SetDlgItemText( hwndDlg, IDC_EDIT_TAG_MAKE_FOLDER, szPath );
	}
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgTagsMake::SetData( void )
{
	//�쐬�t�H���_
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_EDIT_TAG_MAKE_FOLDER ), CB_LIMITTEXT, (WPARAM)sizeof( m_szPath ), 0 );
	::SetDlgItemText( m_hWnd, IDC_EDIT_TAG_MAKE_FOLDER, m_szPath );

	//�I�v�V����
	m_nTagsOpt = m_pShareData->m_nTagsOpt;
	if( m_nTagsOpt & 0x0001 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_TAG_MAKE_RECURSE, TRUE );

	//�R�}���h���C��
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_EDIT_TAG_MAKE_CMDLINE ), CB_LIMITTEXT, (WPARAM)sizeof( m_pShareData->m_szTagsCmdLine ), 0 );
	strcpy( m_szTagsCmdLine, m_pShareData->m_szTagsCmdLine );
	::SetDlgItemText( m_hWnd, IDC_EDIT_TAG_MAKE_CMDLINE, m_pShareData->m_szTagsCmdLine );

	return;
}

/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[ */
int CDlgTagsMake::GetData( void )
{
	//�t�H���_
	::GetDlgItemText( m_hWnd, IDC_EDIT_TAG_MAKE_FOLDER, m_szPath, sizeof( m_szPath ) );
	int length = strlen( m_szPath );
	if( length > 0 )
	{
		if( m_szPath[ length - 1 ] != '\\' ) strcat( m_szPath, "\\" );
	}

	//CTAGS�I�v�V����
	m_nTagsOpt = 0;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_TAG_MAKE_RECURSE ) == BST_CHECKED ) m_nTagsOpt |= 0x0001;
	m_pShareData->m_nTagsOpt = m_nTagsOpt;

	//�R�}���h���C��
	::GetDlgItemText( m_hWnd, IDC_EDIT_TAG_MAKE_CMDLINE, m_szTagsCmdLine, sizeof( m_szTagsCmdLine ) );
	strcpy( m_pShareData->m_szTagsCmdLine, m_szTagsCmdLine );

	return TRUE;
}

LPVOID CDlgTagsMake::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

/*[EOF]*/
