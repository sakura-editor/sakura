//	e$Id$
/*!	@file
	@brief �o�[�W�������_�C�A���O

	@author Norio Nakatani
	@date	1998/3/13 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 200, genta, jepro

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CDlgAbout.h"
#include "sakura_rc.h" // 2002/2/10 aroka ���A

#include "CBREGEXP.h"
#include "CPPA.h"
//	Dec. 2, 2002 genta
#include "etc_uty.h"

// �o�[�W������� CDlgAbout.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12900
	IDC_BUTTON_CONTACT,		HIDC_ABOUT_BUTTON_CONTACT,
	IDOK,					HIDOK_ABOUT,
	IDC_EDIT_ABOUT,			HIDC_ABOUT_EDIT_ABOUT,
//	IDC_STATIC_URL_UR,		12970,
//	IDC_STATIC_URL_ORG,		12971,
//	IDC_STATIC_UPDATE,		12972,
//	IDC_STATIC_VER,			12973,
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

//	From Here Feb. 7, 2002 genta
#if defined(__BORLANDC__)
#  define COMPILER_TYPE "B"
#elif defined(__GNUG__)
#  define COMPILER_TYPE "G"
#elif defined(_MSC_VER)
#  define COMPILER_TYPE "V"
#else
#  define COMPILER_TYPE "U"
#endif
//	To Here Feb. 7, 2002 genta

//	From Here Nov. 7, 2000 genta
/*!
	�W���ȊO�̃��b�Z�[�W��ߑ�����
*/
INT_PTR CDlgAbout::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_SETCURSOR:
		//	�J�[�\�����R���g���[����ɗ���
		if( (HWND)wParam == GetDlgItem(hWnd, IDC_STATIC_URL_UR ) ){
			if( nCursorState != 1 ){
				nCursorState = 1;
				//	�ĕ`�悳����K�v������
				::InvalidateRect( (HWND)wParam, NULL, TRUE );
			}
		}
		else if( (HWND)wParam == GetDlgItem(hWnd, IDC_STATIC_URL_ORG ) ){
			if( nCursorState != 2 ){
				nCursorState = 2;
				//	�ĕ`�悳����K�v������
				::InvalidateRect( (HWND)wParam, NULL, TRUE );
			}
		}
		else {
			if( nCursorState != 0 ){
				nCursorState = 0;
				//	�ĕ`�悳����K�v������
				::InvalidateRect( GetDlgItem(hWnd, IDC_STATIC_URL_UR ), NULL, TRUE );
				::InvalidateRect( GetDlgItem(hWnd, IDC_STATIC_URL_ORG ), NULL, TRUE );
			}
		}
		result = TRUE;
		break;
	case WM_CTLCOLORSTATIC:
		if( nCursorState == 1 &&
			(HWND)lParam == GetDlgItem(hWnd, IDC_STATIC_URL_UR ) ){
			::SetTextColor( (HDC)wParam, RGB(0,0,0xff) );
			result = (INT_PTR)(HBRUSH)GetStockObject(NULL_BRUSH);
		}
		else if( nCursorState == 2 &&
			(HWND)lParam == GetDlgItem(hWnd, IDC_STATIC_URL_ORG ) ){
			::SetTextColor( (HDC)wParam, RGB(0,0,0xff) );
			result = (INT_PTR)(HBRUSH)GetStockObject(NULL_BRUSH);
		}
		break;
	case WM_COMMAND:
		//	CDialog�W���ł͏E���Ȃ�����
		switch( LOWORD(wParam) ){
		case IDC_STATIC_URL_UR:
		case IDC_STATIC_URL_ORG:
			//	Web Browser�̋N��
			{
				char buf[512];
				::GetWindowText( ::GetDlgItem( hWnd, LOWORD( wParam ) ), buf, 512 );
				ShellExecute( hWnd, NULL, buf, NULL, NULL, SW_SHOWNORMAL );
			}
			result = TRUE;
		}
		break;
	}
	return result;
}
//	To Here Nov. 7, 2000 genta

/* ���[�_���_�C�A���O�̕\�� */
int CDlgAbout::DoModal( HINSTANCE hInstance, HWND hwndParent )
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_ABOUT, NULL );
}

BOOL CDlgAbout::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;

	char			szMsg[2048];
	char			szFile[_MAX_PATH];
	WIN32_FIND_DATA	wfd;
	SYSTEMTIME		systimeL;

	/* ���̎��s�t�@�C���̏�� */
	::GetModuleFileName( ::GetModuleHandle( NULL ), szFile, sizeof( szFile ) );
	
	//	2003.10.04 Moca �n���h���̃N���[�Y�Y��
	::ZeroMemory( &wfd, sizeof( wfd ));
	HANDLE hFind = ::FindFirstFile( szFile, &wfd );
	if( hFind != INVALID_HANDLE_VALUE ){
		FindClose( hFind );
	}
	
	::FileTimeToLocalFileTime( &wfd.ftLastWriteTime, &wfd.ftLastWriteTime );
	::FileTimeToSystemTime( &wfd.ftLastWriteTime, &systimeL );
	/* �o�[�W������� */
	//	Nov. 6, 2000 genta	Unofficial Release�̃o�[�W�����Ƃ��Đݒ�
	//	Jun. 8, 2001 genta	GPL���ɔ����AOfficial��Release�Ƃ��Ă̓�����ݎn�߂�
	//	Feb. 7, 2002 genta �R���p�C�����ǉ�
	wsprintf( szMsg, "Ver. %d.%d.%d.%d " COMPILER_TYPE,
		HIWORD( m_pShareData->m_dwProductVersionMS ),
		LOWORD( m_pShareData->m_dwProductVersionMS ),
		HIWORD( m_pShareData->m_dwProductVersionLS ),
		LOWORD( m_pShareData->m_dwProductVersionLS )
	);
	::SetDlgItemText( m_hWnd, IDC_STATIC_VER, szMsg );

	/* �X�V����� */
	wsprintf( szMsg, "Last Update: %d/%d/%d %02d:%02d:%02d",
		systimeL.wYear,
		systimeL.wMonth,
		systimeL.wDay,
		systimeL.wHour,
		systimeL.wMinute,
		systimeL.wSecond
	);
	::SetDlgItemText( m_hWnd, IDC_STATIC_UPDATE, szMsg );

	//	Nov. 7, 2000 genta �J�[�\���ʒu�̏���ێ�
	nCursorState = 0;

	//	From Here Jun. 8, 2001 genta
	//	Edit Box�Ƀ��b�Z�[�W��ǉ�����D
	int desclen = ::LoadString( m_hInstance, IDS_ABOUT_DESCRIPTION, szMsg, sizeof( szMsg ) );
	if( desclen > 0 ){
		::SetDlgItemText( m_hWnd, IDC_EDIT_ABOUT, szMsg );
	}
	//	To Here Jun. 8, 2001 genta

	//	From Here Dec. 2, 2002 genta
	//	�A�C�R�����J�X�^�}�C�Y�A�C�R���ɍ��킹��
	HICON hIcon = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );
	HWND hIconWnd = GetDlgItem( m_hWnd, IDC_STATIC_MYICON );
	
	if( hIconWnd != NULL && hIcon != NULL ){
		::SendMessage( hIconWnd, STM_SETICON, (WPARAM)hIcon, 0 );
	}
	//	To Here Dec. 2, 2002 genta

	/* ���N���X�����o */
	return CDialog::OnInitDialog( m_hWnd, wParam, lParam );
}


BOOL CDlgAbout::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_CONTACT:
		/* �u����ҘA����v�̃w���v */	//Jan. 12, 2001 jepro `���'�̑O��`��'��t����
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, 8 );
		return TRUE;
//Jan. 12, 2001 JEPRO UR1.2.20.2 (Nov. 7, 2000) ����ȉ��̃{�^���͍폜����Ă���̂ŃR�����g�A�E�g����
//	case IDC_BUTTON_DOWNLOAD:
//		/* �u�ŐV�o�[�W�����̃_�E�����[�h�v�̃w���v  */
//		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, 112 );
//		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

//@@@ 2002.01.18 add start
LPVOID CDlgAbout::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
