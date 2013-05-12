/*!	@file
	@brief �^�C�v�ʐݒ�C���|�[�g�m�F�_�C�A���O

	@author Uchi
	@date 2010/4/17 �V�K�쐬
*/
/*
	Copyright (C) 2010, Uchi

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
#include "StdAfx.h"
#include "CDlgTypeAscertain.h"
#include "env/CDocTypeManager.h"
#include "util/shell.h"
#include "sakura.hh"
#include "sakura_rc.h"

//�����g�p�萔
static const int PROP_TEMPCHANGE_FLAG = 0x10000;

// �^�C�v�ʐݒ�C���|�[�g�m�F CDlgTypeAscertain.cpp
const DWORD p_helpids[] = {
	IDC_COMBO_COLORS,		HIDC_COMBO_COLORS,		//�F�w��
	IDOK,					HIDOK_DTA,				//OK
	IDCANCEL,				HIDCANCEL_DTA,			//�L�����Z��
	IDC_BUTTON_HELP,		HIDC_DTA_BUTTON_HELP,	//�w���v
//	IDC_STATIC,				-1,
	0, 0
};

//  Constructors
CDlgTypeAscertain::CDlgTypeAscertain()
	: m_psi(NULL)
{
}

// ���[�_���_�C�A���O�̕\��
int CDlgTypeAscertain::DoModal( HINSTANCE hInstance, HWND hwndParent, SAscertainInfo* psAscertainInfo )
{
	m_psi = psAscertainInfo;

	m_psi->nColorType = -1;

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_TYPE_ASCERTAIN, (LPARAM)NULL );
}

// �{�^���N���b�N
BOOL CDlgTypeAscertain::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�^�C�v�ʐݒ�C���|�[�g�v�̃w���v */
		MyWinHelp( GetHwnd(), m_pszHelpFile, HELP_CONTEXT, HLP000338 );
		return TRUE;
	case IDOK:
		TCHAR	buff1[_MAX_PATH + 20];
		wchar_t	buff2[_MAX_PATH + 20];

		m_psi->sColorFile = L"";
		m_psi->nColorType = Combo_GetCurSel( GetDlgItem( GetHwnd(), IDC_COMBO_COLORS ) ) - 1;
		if (m_psi->nColorType >= MAX_TYPES && Combo_GetLBText( ::GetDlgItem( GetHwnd(), IDC_COMBO_COLORS ), m_psi->nColorType + 1, buff1)) {
			if (_stscanf( buff1, _T("File -- %ls"), buff2 ) > 0) {
				m_psi->sColorFile = buff2;
				m_psi->nColorType = MAX_TYPES;
			}
		}
		::EndDialog( GetHwnd(), TRUE );
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgTypeAscertain::SetData( void )
{
	// �^�C�v���ݒ�
	::SetWindowTextW( ::GetDlgItem( GetHwnd(), IDC_STATIC_TYPE_TO   ), m_psi->sTypeNameTo.c_str() );
	::SetWindowTextW( ::GetDlgItem( GetHwnd(), IDC_STATIC_TYPE_FILE ), m_psi->sTypeNameFile.c_str() );

	int		nIdx;
	HWND	hwndCombo;
	TCHAR	szText[130];
	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_COLORS );
	/* �R���{�{�b�N�X����ɂ��� */
	Combo_ResetContent( hwndCombo );
	/* ��s�ڂ͂��̂܂� */
	Combo_AddString( hwndCombo, L"--���̂܂܃C���|�[�g--" );

	// �G�f�B�^���̐ݒ�
	for (nIdx = 0; nIdx < MAX_TYPES; ++nIdx) {
		STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));
		if (types.m_szTypeExts[0] != _T('\0')) {		/* �^�C�v�����F�g���q���X�g */
			auto_sprintf( szText, _T("%ts (%ts)"),
				types.m_szTypeName,	/* �^�C�v�����F���� */
				types.m_szTypeExts	/* �^�C�v�����F�g���q���X�g */
			);
		}
		else{
			auto_sprintf( szText, _T("%ts"),
				types.m_szTypeName	/* �^�C�v�����F�g�� */
			);
		}
		::Combo_AddString( hwndCombo, szText );
	}
	// �Ǎ��F�ݒ�t�@�C���ݒ�
	HANDLE	hFind;
	WIN32_FIND_DATA	wf;
	BOOL	bFind;
	TCHAR	sTrgCol[_MAX_PATH + 1];

	::SplitPath_FolderAndFile( m_psi->sImportFile.c_str(), sTrgCol, NULL );
	_tcscat( sTrgCol, _T("\\*.col") );
	for (bFind = ( ( hFind = FindFirstFile( sTrgCol, &wf ) ) != INVALID_HANDLE_VALUE );
		bFind;
		bFind = FindNextFile( hFind, &wf )) {
		if ( (wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			// �Ǎ��F�ݒ�t�@�C������
			auto_sprintf( szText, _T("File -- %ts"), wf.cFileName );
			::Combo_AddString( hwndCombo, szText );
		}
	}
	FindClose( hFind );

	// �R���{�{�b�N�X�̃f�t�H���g�I��
	Combo_SetCurSel( hwndCombo, 0 );
	return;
}

LPVOID CDlgTypeAscertain::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
