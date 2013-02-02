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

#include "StdAfx.h"
#include "dlg/CDlgAbout.h"
#include "_os/HandCursor.h"
#include "util/file.h"
#include "util/module.h"
#include "sakura_rc.h" // 2002/2/10 aroka ���A
#include "sakura.hh"

// �o�[�W������� CDlgAbout.cpp	//@@@ 2002.01.07 add start MIK
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
// 2010.04.15 Moca icc/dmc��ǉ���CPU�𕪗�
#if defined(_M_IA64)
#  define TARGET_M_SUFFIX "_I64"
#elif defined(_M_AMD64)
#  define TARGET_M_SUFFIX "_A64"
#else
#  define TARGET_M_SUFFIX ""
#endif

#if defined(__BORLANDC__)
#  define COMPILER_TYPE "B"
#  define COMPILER_VER  __BORLANDC__ 
#elif defined(__GNUG__)
#  define COMPILER_TYPE "G"
#  define COMPILER_VER (__GNUC__ * 10000 + __GNUC_MINOR__  * 100 + __GNUC_PATCHLEVEL__)
#elif __INTEL_COMPILER
#  define COMPILER_TYPE "I"
#  define COMPILER_VER __INTEL_COMPILER
#elif __DMC__
#  define COMPILER_TYPE "D"
#  define COMPILER_VER __DMC__
#elif defined(_MSC_VER)
#  define COMPILER_TYPE "V"
#  define COMPILER_VER _MSC_VER
#else
#  define COMPILER_TYPE "U"
#  define COMPILER_VER 0
#endif
//	To Here Feb. 7, 2002 genta

#ifdef _UNICODE
	#define TARGET_STRING_MODEL "W"
#else
	#define TARGET_STRING_MODEL "A"
#endif

#ifdef _DEBUG
	#define TARGET_DEBUG_MODE "D"
#else
	#define TARGET_DEBUG_MODE "R"
#endif

#define TSTR_TARGET_MODE _T(TARGET_STRING_MODEL) _T(TARGET_DEBUG_MODE)

#ifdef _WIN32_WINDOWS
	#define MY_WIN32_WINDOWS _WIN32_WINDOWS
#else
	#define MY_WIN32_WINDOWS 0
#endif

#ifdef _WIN32_WINNT
	#define MY_WIN32_WINNT _WIN32_WINNT
#else
	#define MY_WIN32_WINNT 0
#endif

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
		// EDIT�� READONLY �� DISABLE�̏ꍇ WM_CTLCOLORSTATIC �ɂȂ�܂�
		if( (HWND)lParam == GetDlgItem(hWnd, IDC_EDIT_ABOUT) ){
			::SetTextColor( (HDC)wParam, RGB( 102, 102, 102 ) );
		} else {
			::SetTextColor( (HDC)wParam, RGB( 0, 0, 0 ) );
        }
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

/*! ����������
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
BOOL CDlgAbout::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );

	TCHAR			szMsg[2048];
	TCHAR			szFile[_MAX_PATH];

	/* ���̎��s�t�@�C���̏�� */
	::GetModuleFileName( NULL, szFile, _countof( szFile ) );
	
	//	Oct. 22, 2005 genta �^�C���X�^���v�擾�̋��ʊ֐����p

	/* �o�[�W������� */
	//	Nov. 6, 2000 genta	Unofficial Release�̃o�[�W�����Ƃ��Đݒ�
	//	Jun. 8, 2001 genta	GPL���ɔ����AOfficial��Release�Ƃ��Ă̓�����ݎn�߂�
	//	Feb. 7, 2002 genta �R���p�C�����ǉ�
	//	2004.05.13 Moca �o�[�W�����ԍ��́A�v���Z�X���ƂɎ擾����
	//	2010.04.15 Moca �R���p�C�����𕪗�/WIN�w�b�_,N_SHAREDATA_VERSION�ǉ�

	// �ȉ��̌`���ŏo��
	//�T�N���G�f�B�^   Ver. 2.0.0.0
	//
	//      Share Ver: 96
	//      Compile Info: V 1400  WR WIN600/I601/C000/N600
	//      Last Modified: 1999/9/9 00:00:00
	//      (�����SKR_PATCH_INFO�̕����񂪂��̂܂ܕ\��)
	CNativeT cmemMsg;
	cmemMsg.AppendString(_T("�T�N���G�f�B�^   "));

	DWORD dwVersionMS, dwVersionLS;
	GetAppVersionInfo( NULL, VS_VERSION_INFO,
		&dwVersionMS, &dwVersionLS );
	auto_sprintf( szMsg, _T("Ver. %d.%d.%d.%d\r\n"),
		HIWORD( dwVersionMS ),
		LOWORD( dwVersionMS ),
		HIWORD( dwVersionLS ),
		LOWORD( dwVersionLS )
	);
	cmemMsg.AppendString( szMsg );

	cmemMsg.AppendString( _T("\r\n") );

	auto_sprintf( szMsg,  _T("      Share Ver: %3d\r\n"),
		N_SHAREDATA_VERSION
	);
	cmemMsg.AppendString( szMsg );

	cmemMsg.AppendString( _T("      Compile Info: ") );
	int Compiler_ver = COMPILER_VER;
	auto_sprintf( szMsg, _T(COMPILER_TYPE) _T(TARGET_M_SUFFIX) _T(" %d  ")
			TSTR_TARGET_MODE _T(" WIN%03x/I%03x/C%03x/N%03x\r\n"),
		Compiler_ver,
		WINVER, _WIN32_IE, MY_WIN32_WINDOWS, MY_WIN32_WINNT
	);
	cmemMsg.AppendString( szMsg );

	/* �X�V����� */
	CFileTime cFileTime;
	GetLastWriteTimestamp( szFile, &cFileTime );
	auto_sprintf( szMsg,  _T("      Last Modified: %d/%d/%d %02d:%02d:%02d\r\n"),
		cFileTime->wYear,
		cFileTime->wMonth,
		cFileTime->wDay,
		cFileTime->wHour,
		cFileTime->wMinute,
		cFileTime->wSecond
	);
	cmemMsg.AppendString( szMsg );

// �p�b�`(�����r�W����)�̏����R���p�C�����ɓn����悤�ɂ���
#ifdef SKR_PATCH_INFO
	cmemMsg.AppendString( _T("      ") );
	const TCHAR* ptszPatchInfo = to_tchar(SKR_PATCH_INFO);
	int patchInfoLen = auto_strlen(ptszPatchInfo);
	cmemMsg.AppendString( ptszPatchInfo, t_min(80, patchInfoLen) );
#endif
	cmemMsg.AppendString( _T("\r\n"));

	::DlgItem_SetText( GetHwnd(), IDC_EDIT_VER, cmemMsg.GetStringPtr() );

	//	From Here Jun. 8, 2001 genta
	//	Edit Box�Ƀ��b�Z�[�W��ǉ�����D
	int desclen = ::LoadString( m_hInstance, IDS_ABOUT_DESCRIPTION, szMsg, _countof( szMsg ) );
	if( desclen > 0 ){
		::DlgItem_SetText( GetHwnd(), IDC_EDIT_ABOUT, szMsg );
	}
	//	To Here Jun. 8, 2001 genta

	//	From Here Dec. 2, 2002 genta
	//	�A�C�R�����J�X�^�}�C�Y�A�C�R���ɍ��킹��
	HICON hIcon = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );
	HWND hIconWnd = GetDlgItem( GetHwnd(), IDC_STATIC_MYICON );
	
	if( hIconWnd != NULL && hIcon != NULL ){
		StCtl_SetIcon( hIconWnd, hIcon );
	}
	//	To Here Dec. 2, 2002 genta

	// URL�E�B���h�E���T�u�N���X������
	m_UrlUrWnd.SetSubclassWindow( GetDlgItem( GetHwnd(), IDC_STATIC_URL_UR ) );

	//	Oct. 22, 2005 genta ����҃z�[���y�[�W�������Ȃ����̂ō폜
	//m_UrlOrgWnd.SubclassWindow( GetDlgItem( GetHwnd(), IDC_STATIC_URL_ORG ) );

	/* ���N���X�����o */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}


BOOL CDlgAbout::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_COPY:
		{
			HWND hwndEditVer = GetDlgItem( GetHwnd(), IDC_EDIT_VER );
	 		EditCtl_SetSel( hwndEditVer, 0, -1); 
	 		SendMessage( hwndEditVer, WM_COPY, 0, 0 );
	 		EditCtl_SetSel( hwndEditVer, -1, 0); 
 		}
		return TRUE;
	}
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgAbout::OnStnClicked( int wID )
{
	switch( wID ){
	//	2006.07.27 genta ����ҘA����̃{�^�����폜 (�w���v����폜����Ă��邽��)
	case IDC_STATIC_URL_UR:
//	case IDC_STATIC_URL_ORG:	del 2008/7/4 Uchi
		//	Web Browser�̋N��
		{
			TCHAR buf[512];
			::GetWindowText( ::GetDlgItem( GetHwnd(), wID ), buf, _countof(buf) );
			::ShellExecute( GetHwnd(), NULL, buf, NULL, NULL, SW_SHOWNORMAL );
			return TRUE;
		}
	}
	/* ���N���X�����o */
	return CDialog::OnStnClicked( wID );
}

//@@@ 2002.01.18 add start
LPVOID CDlgAbout::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

BOOL CUrlWnd::SetSubclassWindow( HWND hWnd )
{
	// STATIC�E�B���h�E���T�u�N���X������
	// ����STATIC�� WS_TABSTOP, SS_NOTIFY �X�^�C���̂��̂��g�p���邱��
	if( GetHwnd() != NULL )
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
	hFont = (HFONT)SendMessageAny( hWnd, WM_GETFONT, (WPARAM)0, (LPARAM)0 );
	GetObject( hFont, sizeof(lf), &lf );
	lf.lfUnderline = TRUE;
	m_hFont = CreateFontIndirect( &lf );
	if(m_hFont != NULL)
		SendMessageAny( hWnd, WM_SETFONT, (WPARAM)m_hFont, (LPARAM)FALSE );

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
		SetHandCursor();		// Hand Cursor��ݒ� 2013/1/29 Uchi
		return (LRESULT)0;
	case WM_LBUTTONDOWN:
		// �L�[�{�[�h�t�H�[�J�X�������ɓ��Ă�
		SendMessageAny( GetParent(hWnd), WM_NEXTDLGCTL, (WPARAM)hWnd, (LPARAM)1 );
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
			SendMessageAny( hWnd, WM_MOUSEMOVE, 0, MAKELONG( pt.x, pt.y ) );
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
		GetWindowText( hWnd, szText, _countof(szText) );
		hFont = (HFONT)SendMessageAny( hWnd, WM_GETFONT, (WPARAM)0, (LPARAM)0 );

		// �e�L�X�g�`��
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, pUrlWnd->m_bHilighted? RGB( 0x84, 0, 0 ): RGB( 0, 0, 0xff ) );
		hOldFont = (HFONT)SelectObject( hdc, (HGDIOBJ)hFont );
		TextOut( hdc, 2, 0, szText, _tcslen( szText ) );
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
			ExtTextOutW_AnyBuild( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
		}else{
			// �e��WM_CTLCOLORSTATIC�𑗂��Ĕw�i�u���V���擾���A�w�i�`�悷��
			HBRUSH hbr;
			HBRUSH hbrOld;
			hbr = (HBRUSH)SendMessageAny( GetParent( hWnd ), WM_CTLCOLORSTATIC, wParam, (LPARAM)hWnd );
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


