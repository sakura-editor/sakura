/*!	@file
	@brief �o�[�W�������_�C�A���O

	@author Norio Nakatani
	@date	1998/3/13 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, genta, jepro
	Copyright (C) 2002, MIK, genta, aroka
	Copyright (C) 2003, Moca
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CDlgAbout.h"
#include "sakura_rc.h" // 2002/2/10 aroka ���A

#include "CBREGEXP.h"
#include "CPPA.h"
//	Dec. 2, 2002 genta
#include "etc_uty.h"

// �o�[�W������� CDlgAbout.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12900
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
// 2006.01.17 Moca COMPILER_VER��ǉ�
#if defined(__BORLANDC__)
#  define COMPILER_TYPE "B"
#  define COMPILER_VER  __BORLANDC__ 
#elif defined(__GNUG__)
#  define COMPILER_TYPE "G"
#  define COMPILER_VER (__GNUC__ * 10000 + __GNUC_MINOR__  * 100 + __GNUC_PATCHLEVEL__)
#elif defined(_MSC_VER)
#  if defined(_M_IA64)
#    define COMPILER_TYPE "V_I64"
#  elif defined(_M_AMD64)
#    define COMPILER_TYPE "V_A64"
#  else
#    define COMPILER_TYPE "V"
#  endif
#  define COMPILER_VER _MSC_VER
#else
#  define COMPILER_TYPE "U"
#  define COMPILER_VER 0
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
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		return (INT_PTR)GetStockObject( WHITE_BRUSH );
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
	//WIN32_FIND_DATA	wfd;
	FILETIME		lastTime;
	SYSTEMTIME		systimeL;

	/* ���̎��s�t�@�C���̏�� */
	::GetModuleFileName( ::GetModuleHandle( NULL ), szFile, sizeof( szFile ) );
	
	//	Oct. 22, 2005 genta �^�C���X�^���v�擾�̋��ʊ֐����p
	//	2003.10.04 Moca �n���h���̃N���[�Y�Y��
	//::ZeroMemory( &wfd, sizeof( wfd ));
	//HANDLE hFind = ::FindFirstFile( szFile, &wfd );
	//if( hFind != INVALID_HANDLE_VALUE ){
	//	FindClose( hFind );
	//}
	if( !GetLastWriteTimestamp( szFile, lastTime )){
		lastTime.dwLowDateTime = lastTime.dwHighDateTime = 0;
	}

	::FileTimeToLocalFileTime( &lastTime, &lastTime );
	::FileTimeToSystemTime( &lastTime, &systimeL );
	/* �o�[�W������� */
	//	Nov. 6, 2000 genta	Unofficial Release�̃o�[�W�����Ƃ��Đݒ�
	//	Jun. 8, 2001 genta	GPL���ɔ����AOfficial��Release�Ƃ��Ă̓�����ݎn�߂�
	//	Feb. 7, 2002 genta �R���p�C�����ǉ�
	//	2004.05.13 Moca �o�[�W�����ԍ��́A�v���Z�X���ƂɎ擾����
	DWORD dwVersionMS, dwVersionLS;
	GetAppVersionInfo( NULL, VS_VERSION_INFO,
		&dwVersionMS, &dwVersionLS );

	int ComPiler_ver = COMPILER_VER;
	wsprintf( szMsg, "Ver. %d.%d.%d.%d (" COMPILER_TYPE " %d)",
		HIWORD( dwVersionMS ),
		LOWORD( dwVersionMS ),
		HIWORD( dwVersionLS ),
		LOWORD( dwVersionLS ),
		ComPiler_ver
	);
	::SetDlgItemText( m_hWnd, IDC_STATIC_VER, szMsg );

	/* �X�V����� */
	wsprintf( szMsg, "Last Modified: %d/%d/%d %02d:%02d:%02d",
		systimeL.wYear,
		systimeL.wMonth,
		systimeL.wDay,
		systimeL.wHour,
		systimeL.wMinute,
		systimeL.wSecond
	);
	::SetDlgItemText( m_hWnd, IDC_STATIC_UPDATE, szMsg );

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

	// URL�E�B���h�E���T�u�N���X������
	m_UrlUrWnd.SubclassWindow( GetDlgItem( m_hWnd, IDC_STATIC_URL_UR ) );

	//	Oct. 22, 2005 genta ����҃z�[���y�[�W�������Ȃ����̂ō폜
	//m_UrlOrgWnd.SubclassWindow( GetDlgItem( m_hWnd, IDC_STATIC_URL_ORG ) );

	/* ���N���X�����o */
	return CDialog::OnInitDialog( m_hWnd, wParam, lParam );
}


BOOL CDlgAbout::OnBnClicked( int wID )
{
	switch( wID ){
	//	2006.07.27 genta ����ҘA����̃{�^�����폜 (�w���v����폜����Ă��邽��)
	case IDC_STATIC_URL_UR:
	case IDC_STATIC_URL_ORG:
		//	Web Browser�̋N��
		{
			char buf[512];
			::GetWindowText( ::GetDlgItem( m_hWnd, wID ), buf, 512 );
			ShellExecute( m_hWnd, NULL, buf, NULL, NULL, SW_SHOWNORMAL );
			return TRUE;
		}
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

BOOL CUrlWnd::SubclassWindow( HWND hWnd )
{
	// STATIC�E�B���h�E���T�u�N���X������
	// ����STATIC�� WS_TABSTOP, SS_NOTIFY �X�^�C���̂��̂��g�p���邱��
	if( m_hWnd != NULL )
		return FALSE;
	if( !IsWindow( hWnd ) )
		return FALSE;

	// �T�u�N���X�������s����
	LONG_PTR lptr;
	SetLastError( 0 );
	lptr = SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)this );
	if( lptr == 0 && GetLastError() != 0 )
		return FALSE;
	m_pOldProc = (WNDPROC)SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR)UrlWndProc );
	if( m_pOldProc == NULL )
		return FALSE;
	m_hWnd = hWnd;

	// �����t���t�H���g�ɕύX����
	HFONT hFont;
	LOGFONT lf;
	hFont = (HFONT)SendMessage( hWnd, WM_GETFONT, (WPARAM)0, (LPARAM)0 );
	GetObject( hFont, sizeof(lf), &lf );
	lf.lfUnderline = TRUE;
	m_hFont = CreateFontIndirect( &lf );
	if(m_hFont != NULL)
		SendMessage( hWnd, WM_SETFONT, (WPARAM)m_hFont, (LPARAM)FALSE );

	return TRUE;
}

LRESULT CALLBACK CUrlWnd::UrlWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	CUrlWnd* pUrlWnd = (CUrlWnd*)GetWindowLongPtr( hWnd, GWLP_USERDATA );

	HDC hdc;
	POINT pt;
	RECT rc;

	switch ( msg ) {
	case WM_SETCURSOR:
		// �J�[�\���`��ύX
		HCURSOR hCursor;
		hCursor = LoadCursor( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDC_CURSOR_HAND ) );
		SetCursor( hCursor );
		return (LRESULT)0;
	case WM_LBUTTONDOWN:
		// �L�[�{�[�h�t�H�[�J�X�������ɓ��Ă�
		SendMessage( GetParent(hWnd), WM_NEXTDLGCTL, (WPARAM)hWnd, (LPARAM)1 );
		break;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		// �ĕ`��
		InvalidateRect( hWnd, NULL, TRUE );
		UpdateWindow( hWnd );
		break;
	case WM_GETDLGCODE:
		// �f�t�H���g�v�b�V���{�^���̂悤�ɐU�����iEnter�L�[�̗L�����j
		// �����L�[�͖������iIE�̃o�[�W�������_�C�A���O�Ɠ��l�j
		return DLGC_DEFPUSHBUTTON | DLGC_WANTARROWS;
	case WM_MOUSEMOVE:
		// �J�[�\�����E�B���h�E���ɓ�������^�C�}�[�N��
		// �E�B���h�E�O�ɏo����^�C�}�[�폜
		// �e�^�C�~���O�ōĕ`��
		BOOL bHilighted;
		pt.x = LOWORD( lParam );
		pt.y = HIWORD( lParam );
		GetClientRect( hWnd, &rc );
		bHilighted = PtInRect( &rc, pt );
		if( bHilighted != pUrlWnd->m_bHilighted ){
			pUrlWnd->m_bHilighted = bHilighted;
			InvalidateRect( hWnd, NULL, TRUE );
			if( pUrlWnd->m_bHilighted )
				SetTimer( hWnd, 1, 200, NULL );
			else
				KillTimer( hWnd, 1 );
		}
		break;
	case WM_TIMER:
		// �J�[�\�����E�B���h�E�O�ɂ���ꍇ�ɂ� WM_MOUSEMOVE �𑗂�
		GetCursorPos( &pt );
		ScreenToClient( hWnd, &pt );
		GetClientRect( hWnd, &rc );
		if( !PtInRect( &rc, pt ) )
			SendMessage( hWnd, WM_MOUSEMOVE, 0, MAKELONG( pt.x, pt.y ) );
		break;
	case WM_PAINT:
		// �E�B���h�E�̕`��
		PAINTSTRUCT ps;
		HFONT hFont;
		HFONT hOldFont;
		TCHAR szText[512];

		hdc = BeginPaint( hWnd, &ps );

		// ���݂̃N���C�A���g��`�A�e�L�X�g�A�t�H���g���擾����
		GetClientRect( hWnd, &rc );
		GetWindowText( hWnd, szText, 512 );
		hFont = (HFONT)SendMessage( hWnd, WM_GETFONT, (WPARAM)0, (LPARAM)0 );

		// �e�L�X�g�`��
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, pUrlWnd->m_bHilighted? RGB( 0x84, 0, 0 ): RGB( 0, 0, 0xff ) );
		hOldFont = (HFONT)SelectObject( hdc, (HGDIOBJ)hFont );
		TextOut( hdc, 2, 0, szText, lstrlen( szText ) );
		SelectObject( hdc, (HGDIOBJ)hOldFont );

		// �t�H�[�J�X�g�`��
		if( GetFocus() == hWnd )
			DrawFocusRect( hdc, &rc );

		EndPaint( hWnd, &ps );
		return (LRESULT)0;
	case WM_ERASEBKGND:
		hdc = (HDC)wParam;
		GetClientRect( hWnd, &rc );

		// �w�i�`��
		if( pUrlWnd->m_bHilighted ){
			// �n�C���C�g���w�i�`��
			SetBkColor( hdc, RGB( 0xff, 0xff, 0 ) );
			ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
		}else{
			// �e��WM_CTLCOLORSTATIC�𑗂��Ĕw�i�u���V���擾���A�w�i�`�悷��
			HBRUSH hbr;
			HBRUSH hbrOld;
			hbr = (HBRUSH)SendMessage( GetParent( hWnd ), WM_CTLCOLORSTATIC, wParam, (LPARAM)hWnd );
			hbrOld = (HBRUSH)SelectObject( hdc, hbr );
			FillRect( hdc, &rc, hbr );
			SelectObject( hdc, hbrOld );
		}
		return (LRESULT)1;
	case WM_DESTROY:
		// ��n��
		KillTimer( hWnd, 1 );
		SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR)pUrlWnd->m_pOldProc );
		if( pUrlWnd->m_hFont != NULL )
			DeleteObject( pUrlWnd->m_hFont );
		pUrlWnd->m_hWnd = NULL;
		pUrlWnd->m_hFont = NULL;
		pUrlWnd->m_bHilighted = FALSE;
		pUrlWnd->m_pOldProc = NULL;
		return (LRESULT)0;
	}

	return CallWindowProc( pUrlWnd->m_pOldProc, hWnd, msg, wParam, lParam );
}
//@@@ 2002.01.18 add end

/*[EOF]*/
