/*! @file
	@brief �E�B���h�E�̈ʒu�Ƒ傫���_�C�A���O

	@author Moca
	@date 2004/05/13 �쐬
*/
/*
	Copyright (C) 2004, genta, Moca
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
#include "sakura_rc.h"
#include "sakura.hh"
#include "dlg/CDlgWinSize.h"
#include "util/shell.h"

static const DWORD p_helpids[] = {	// 2006.10.10 ryoji
	IDOK,						HIDOK_WINSIZE,				// ����
	IDC_BUTTON_HELP,			HIDC_BUTTON_WINSIZE_HELP,	// �w���v
	IDC_EDIT_WX,				HIDC_EDIT_WX,				// ��
	IDC_EDIT_WY,				HIDC_EDIT_WY,				// ����
	IDC_EDIT_SX,				HIDC_EDIT_SX,				// X���W
	IDC_EDIT_SY,				HIDC_EDIT_SY,				// Y���W
//	IDC_CHECK_WINPOS,			HIDC_CHECK_WINPOS,
	IDC_RADIO_WINSIZE_DEF,		HIDC_RADIO_WINSIZE_DEF,		// �傫��/�w�肵�Ȃ�
	IDC_RADIO_WINSIZE_SAVE,		HIDC_RADIO_WINSIZE_SAVE,	// �傫��/�p������
	IDC_RADIO_WINSIZE_SET,		HIDC_RADIO_WINSIZE_SET,		// �傫��/���ڎw��
	IDC_RADIO_WINPOS_DEF,		HIDC_RADIO_WINPOS_DEF,		// �ʒu/�w�肵�Ȃ�
	IDC_RADIO_WINPOS_SAVE,		HIDC_RADIO_WINPOS_SAVE, 	// �ʒu/�p������
	IDC_RADIO_WINPOS_SET,		HIDC_RADIO_WINPOS_SET,  	// �ʒu/���ڎw��
	IDC_COMBO_WINTYPE,			HIDC_COMBO_WINTYPE,
	0, 0
};

CDlgWinSize::CDlgWinSize()
{
	return;
}

CDlgWinSize::~CDlgWinSize()
{
	return;
}


/* !���[�_���_�C�A���O�̕\��
	@param nSaveWinSize [in/out] �E�B���h�E�ʒu�p��
	@param nSaveWinPos  [in/out] �E�B���h�E�T�C�Y�p��
	@param nWinSizeType [in/out] �E�B���h�E�̎��s���̑傫��
	@param rc [in/out] ���A�����A���A��
*/
int CDlgWinSize::DoModal( HINSTANCE hInstance, HWND hwndParent,
	int &nSaveWinSize, int &nSaveWinPos, int &nWinSizeType, RECT &rc )
{
	m_nSaveWinSize = nSaveWinSize;
	m_nSaveWinPos  = nSaveWinPos;
	m_nWinSizeType = nWinSizeType;
	m_rc = rc;
	(void)CDialog::DoModal( hInstance, hwndParent, IDD_WINPOSSIZE, NULL );
	nSaveWinSize = m_nSaveWinSize;
	nSaveWinPos  = m_nSaveWinPos;
	nWinSizeType = m_nWinSizeType;
	rc = m_rc;
	return TRUE;
}

/*! ����������
*/
BOOL CDlgWinSize::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );

	Combo_AddString( ::GetDlgItem( GetHwnd(), IDC_COMBO_WINTYPE ), L"����" );
	Combo_AddString( ::GetDlgItem( GetHwnd(), IDC_COMBO_WINTYPE ), L"�ő剻" );
	Combo_AddString( ::GetDlgItem( GetHwnd(), IDC_COMBO_WINTYPE ), L"(�ŏ���)" );

	LPARAM range = (LPARAM) MAKELONG((short) 30000, (short) 0 );
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_SPIN_SX ), UDM_SETRANGE, 0, range );
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_SPIN_SY ), UDM_SETRANGE, 0, range );
	// �E�B���h�E�̍��W�́A�}�C�i�X�l���L���B
	range = (LPARAM) MAKELONG((short) 30000, (short) -30000 );
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_SPIN_WX ), UDM_SETRANGE, 0, range );
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_SPIN_WY ), UDM_SETRANGE, 0, range );

	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}


BOOL CDlgWinSize::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:	// 2006/09/09 novice id�C��
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, HLP000286 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDC_RADIO_WINPOS_DEF:
	case IDC_RADIO_WINPOS_SAVE:
	case IDC_RADIO_WINPOS_SET:
	case IDC_RADIO_WINSIZE_DEF:
	case IDC_RADIO_WINSIZE_SAVE:
	case IDC_RADIO_WINSIZE_SET:
		RenewItemState();
		return TRUE;
	case IDOK:
	case IDCANCEL:
		GetData();
	}
	return CDialog::OnBnClicked( wID );
}

/*! @brief �_�C�A���O�{�b�N�X�Ƀf�[�^��ݒ�
*/
void CDlgWinSize::SetData( void )
{
	switch( m_nSaveWinSize ){
	case 1:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINSIZE_SAVE, TRUE );
		break;
	case 2:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINSIZE_SET, TRUE );
		break;
	default:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINSIZE_DEF, TRUE );
	}

	switch( m_nSaveWinPos ){
	case 1:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINPOS_SAVE, TRUE );
		break;
	case 2:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINPOS_SET, TRUE );
		break;
	default:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINPOS_DEF, TRUE );
	}

	int nCurIdx = 0;
	switch( m_nWinSizeType ){
	case SIZE_MINIMIZED:
		nCurIdx = 2;
		break;
	case SIZE_MAXIMIZED:
		nCurIdx = 1;
		break;
	default:
		nCurIdx = 0;
	}
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_COMBO_WINTYPE ), CB_SETCURSEL, (WPARAM)nCurIdx, 0 );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_SX, m_rc.right,  TRUE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_SY, m_rc.bottom, TRUE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_WX, m_rc.top,  TRUE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_WY, m_rc.left, TRUE );
	RenewItemState();
}


/*! �_�C�A���O�{�b�N�X�̃f�[�^��ǂݏo��
*/
int CDlgWinSize::GetData( void )
{
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINSIZE_DEF ) ){
		m_nSaveWinSize = 0;
	}else
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINSIZE_SAVE ) ){
		m_nSaveWinSize = 1;
	}else
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINSIZE_SET ) ){
		m_nSaveWinSize = 2;
	}
	
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINPOS_DEF ) ){
		m_nSaveWinPos = 0;
	}else
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINPOS_SAVE ) ){
		m_nSaveWinPos = 1;
	}else
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINPOS_SET ) ){
		m_nSaveWinPos = 2;
	}

	int nCurIdx;
	nCurIdx = ::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_COMBO_WINTYPE ), CB_GETCURSEL, 0, 0 );
	switch( nCurIdx ){
	case 2:
		m_nWinSizeType = SIZE_MINIMIZED;
		break;
	case 1:
		m_nWinSizeType = SIZE_MAXIMIZED;
		break;
	default:
		m_nWinSizeType = SIZE_RESTORED;
	}
	m_rc.right  = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_SX, NULL, TRUE );
	m_rc.bottom = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_SY, NULL, TRUE );
	m_rc.top    = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_WX, NULL, TRUE );
	m_rc.left   = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_WY, NULL, TRUE );
	return TRUE;
}


/*! ���p�\�E�s�̏�Ԃ��X�V����
*/
void CDlgWinSize::RenewItemState( void )
{
	BOOL state;
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINPOS_SET ) ){
		state = TRUE;
	}else{
		state = FALSE;
	}
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_WX ), state );
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_WY ), state );

	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINSIZE_SET ) ){
		state = TRUE;
	}else{
		state = FALSE;
	}
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_WINTYPE ), state );
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_SX ), state );
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_SY ), state );
}

LPVOID CDlgWinSize::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}


