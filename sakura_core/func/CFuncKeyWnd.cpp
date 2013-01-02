/*!	@file
	@brief �t�@���N�V�����L�[�E�B���h�E

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, MIK, Moca
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2004, novice
	Copyright (C) 2006, aroka, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "_main/global.h"
#include "func/CFuncKeyWnd.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "window/CEditWnd.h"
#include "doc/CEditDoc.h"
#include "util/input.h"
#include "util/window.h"

#define IDT_FUNCWND 1248
#define TIMER_TIMEOUT 100
#define TIMER_CHECKFUNCENABLE 300

/****
LRESULT CALLBACK CFuncKeyWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	CFuncKeyWnd*	pCFuncKeyWnd;
	pCFuncKeyWnd = ( CFuncKeyWnd* )::GetWindowLongPtr( hwnd, GWLP_USERDATA );
	if( NULL != pCFuncKeyWnd ){
		return pCFuncKeyWnd->DispatchEvent( hwnd, uMsg, wParam, lParam );
	}
	return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
}
***/






//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CFuncKeyWnd::CFuncKeyWnd()
: CWnd(_T("::CFuncKeyWnd"))
{
	int		i;
	LOGFONT	lf;
	m_pszClassName = _T("CFuncKeyWnd");
	m_pCEditDoc = NULL;
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	m_nCurrentKeyState = -1;
	for( i = 0; i < _countof(m_szFuncNameArr); ++i ){
		wcscpy( m_szFuncNameArr[i], LTEXT("") );
	}
//	2002.11.04 Moca Open()���Őݒ�
//	m_nButtonGroupNum = 4;

	for( i = 0; i < _countof( m_hwndButtonArr ); ++i ){
		m_hwndButtonArr[i] = NULL;
	}

	/* �\���p�t�H���g */
	/* LOGFONT�̏����� */
	memset_raw( &lf, 0, sizeof(lf) );
	lf.lfHeight			= DpiPointsToPixels(-9);	// 2009.10.01 ryoji ��DPI�Ή��i�|�C���g������Z�o�j
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0x0;
	lf.lfUnderline		= 0x0;
	lf.lfStrikeOut		= 0x0;
	lf.lfCharSet		= 0x80;
	lf.lfOutPrecision	= 0x3;
	lf.lfClipPrecision	= 0x2;
	lf.lfQuality		= 0x1;
	lf.lfPitchAndFamily	= 0x31;
	_tcscpy( lf.lfFaceName, _T("�l�r �o�S�V�b�N") );
	m_hFont = ::CreateFontIndirect( &lf );

	m_bSizeBox = false;
	m_hwndSizeBox = NULL;
	m_nTimerCount = 0;

	return;
}




CFuncKeyWnd::~CFuncKeyWnd()
{
	/* �\���p�t�H���g */
	::DeleteObject( m_hFont );
	return;
}




/* �E�B���h�E �I�[�v�� */
HWND CFuncKeyWnd::Open( HINSTANCE hInstance, HWND hwndParent, CEditDoc* pCEditDoc, bool bSizeBox )
{
	m_pCEditDoc = pCEditDoc;
	m_bSizeBox = bSizeBox;
	m_hwndSizeBox = NULL;
	m_nCurrentKeyState = -1;

	// 2002.11.04 Moca �ύX�ł���悤��
	m_nButtonGroupNum = m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum;
	if( 1 > m_nButtonGroupNum || 12 < m_nButtonGroupNum ){
		m_nButtonGroupNum = 4;
	}

	/* �E�B���h�E�N���X�쐬 */
	RegisterWC(
		hInstance,
		NULL,// Handle to the class icon.
		NULL,	//Handle to a small icon
		::LoadCursor( NULL, IDC_ARROW ),// Handle to the class cursor.
		(HBRUSH)(COLOR_3DFACE + 1),// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		m_pszClassName// Pointer to a null-terminated string or is an atom.
	);

	/* ���N���X�����o�Ăяo�� */
	CWnd::Create(
		hwndParent,
		0, // extended window style
		m_pszClassName,	// Pointer to a null-terminated string or is an atom.
		m_pszClassName, // pointer to window name
		WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPCHILDREN, // window style	// 2006.06.17 ryoji WS_CLIPCHILDREN �ǉ�	// 2007.03.08 ryoji WS_VISIBLE ����
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		0, // window width	// 2007.02.05 ryoji 100->0�i���[�ȃT�C�Y�ň�u�\��������茩���Ȃ��ق��������j
		::GetSystemMetrics( SM_CYMENU ), // window height
		NULL // handle to menu, or child-window identifier
	);


	m_hwndSizeBox = NULL;
	if( m_bSizeBox ){
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 						/* no extended styles			*/
			_T("SCROLLBAR"),				/* scroll bar control class		*/
			NULL,						/* text for window title bar	*/
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,							/* horizontal position			*/
			0,							/* vertical position			*/
			200,						/* width of the scroll bar		*/
			CW_USEDEFAULT,				/* default height				*/
			GetHwnd(), 					/* handle of main window		*/
			(HMENU) NULL,				/* no menu for a scroll bar 	*/
			GetAppInstance(),				/* instance owning this window	*/
			(LPVOID) NULL				/* pointer not needed			*/
		);
	}

	/* �{�^���̐��� */
	CreateButtons();

	Timer_ONOFF( true ); // 20060126 aroka
	OnTimer( GetHwnd(), WM_TIMER, IDT_FUNCWND, ::GetTickCount() );	// ����X�V	// 2006.12.20 ryoji

	return GetHwnd();
}




/* �E�B���h�E �N���[�Y */
void CFuncKeyWnd::Close( void )
{
	this->DestroyWindow();
}





///* WM_SIZE���� */
//void CFuncKeyWnd::OnSize(
//	WPARAM	wParam,	// first message parameter
//	LPARAM	lParam 	// second message parameter

// WM_SIZE����
LRESULT CFuncKeyWnd::OnSize( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int			nButtonWidth;
	int			nButtonHeight;
	int			i;
	int			nX;
//	RECT		rc;
	int			nButtonNum;
	RECT		rcParent;

	if( NULL == GetHwnd() ){
		return 0L;
	}

	nButtonNum = _countof( m_hwndButtonArr );

	/* �{�^���̃T�C�Y���v�Z */
	nButtonWidth = CalcButtonSize();

	::GetWindowRect( GetHwnd(), &rcParent );
	nButtonHeight = rcParent.bottom - rcParent.top - 2;

	nX = 1;
	for( i = 0; i < nButtonNum; ++i ){
		if( 0 < i  && 0 == ( i % m_nButtonGroupNum ) ){
			nX += 12;
		}
		::MoveWindow( m_hwndButtonArr[i], nX, 1, nButtonWidth, nButtonHeight, TRUE );
		nX += nButtonWidth + 1;
	}
	::InvalidateRect( GetHwnd(), NULL, TRUE );	//�ĕ`�悵�ĂˁB	//@@@ 2003.06.11 MIK
	return 0L;
}


#if 0//////////////////////////////////////////////////////////////
LRESULT CFuncKeyWnd::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
//	if( NULL == GetHwnd() ){
//		return 0L;
//	}

	int		i;
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl;
	switch ( uMsg ){

	case WM_TIMER:		return OnTimer( hwnd, uMsg, wParam, lParam );
	case WM_COMMAND:	return OnCommand( hwnd, uMsg, wParam, lParam );
	case WM_SIZE:		return OnSize( hwnd, uMsg, wParam, lParam );
	case WM_DESTROY:	return OnDestroy( hwnd, uMsg, wParam, lParam );

	default:
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}
#endif//////////////////////////////////////////////////////////////



LRESULT CFuncKeyWnd::OnCommand( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	int		i;
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl;

	wNotifyCode = HIWORD(wParam);	// notification code
	wID = LOWORD(wParam);			// item, control, or accelerator identifier
	hwndCtl = (HWND) lParam;		// handle of control
//	switch( wNotifyCode ){
//	case BN_PUSHED:
		for( i = 0; i < _countof( m_hwndButtonArr ); ++i ){
			if( hwndCtl == m_hwndButtonArr[i] ){
				if( 0 != m_nFuncCodeArr[i] ){
					::SendMessageCmd( GetParentHwnd(), WM_COMMAND, MAKELONG( m_nFuncCodeArr[i], 0 ),  (LPARAM)hwnd );
				}
				break;
			}
		}
		::SetFocus( GetParentHwnd() );
//		break;
//	}
	return 0L;
}


// WM_TIMER�^�C�}�[�̏���
LRESULT CFuncKeyWnd::OnTimer( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
// 	HWND hwnd,	// handle of window for timer messages
//	UINT uMsg,	// WM_TIMER message
//	UINT idEvent,	// timer identifier
//	DWORD dwTime 	// current system time


	//	return;
	if( NULL == GetHwnd() ){
		return 0;
	}

	if( ::GetActiveWindow() != GetParentHwnd() && m_nCurrentKeyState != -1 ) {	//	2002/06/02 MIK	// 2006.12.20 ryoji ����X�V�͏�������
		return 0;
	}

	int			nIdx;
//	int			nFuncId;
	int			i;

// novice 2004/10/10
	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();
	/* ALT,Shift,Ctrl�L�[�̏�Ԃ��ω������� */
	if( nIdx != m_nCurrentKeyState ){
		m_nTimerCount = TIMER_CHECKFUNCENABLE + 1;

		/* �t�@���N�V�����L�[�̋@�\�����擾 */
		for( i = 0; i < _countof( m_szFuncNameArr ); ++i ){
			// 2007.02.22 ryoji CKeyBind::GetFuncCode()���g��
			EFunctionCode	nFuncCode = CKeyBind::GetFuncCode(
					(((VK_F1 + i) | ((WORD)((BYTE)(nIdx))) << 8)),
					m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
					m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
			);
			if( nFuncCode != m_nFuncCodeArr[i] ){
				m_nFuncCodeArr[i] = nFuncCode;
				if( 0 == m_nFuncCodeArr[i] ){
					wcscpy( m_szFuncNameArr[i], LTEXT("") );
				}else{
					//	Oct. 2, 2001 genta
					m_pCEditDoc->m_cFuncLookup.Funccode2Name(
						m_nFuncCodeArr[i],
						m_szFuncNameArr[i],
						_countof(m_szFuncNameArr[i]) - 1
					);
				}
				Wnd_SetText( m_hwndButtonArr[i], m_szFuncNameArr[i] );
			}
		}
	}
	m_nTimerCount += TIMER_TIMEOUT;
	if( m_nTimerCount > TIMER_CHECKFUNCENABLE ||
		nIdx != m_nCurrentKeyState
	){
		m_nTimerCount = 0;
		/* �@�\�����p�\�����ׂ� */
		for( i = 0; i < _countof(	m_szFuncNameArr ); ++i ){
			if( IsFuncEnable( (CEditDoc*)m_pCEditDoc, m_pShareData, m_nFuncCodeArr[i]  ) ){
				::EnableWindow( m_hwndButtonArr[i], TRUE );
			}else{
				::EnableWindow( m_hwndButtonArr[i], FALSE );
			}
		}
	}
//	MYTRACE_A( "\n" );
	m_nCurrentKeyState = nIdx;
	return 0;
}


// WM_DESTROY����
LRESULT CFuncKeyWnd::OnDestroy( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int i;

	/* �^�C�}�[���폜 */
	Timer_ONOFF( false ); // 20060126 aroka

	/* �{�^�����폜 */
	for( i = 0; i < _countof( m_hwndButtonArr ); ++i ){
		if( NULL != m_hwndButtonArr[i] ){
			::DestroyWindow( m_hwndButtonArr[i]	);
			m_hwndButtonArr[i] = NULL;
		}
	}

	/* �T�C�Y�{�b�N�X���폜 */
	if( NULL != m_hwndSizeBox ){
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
	}

	_SetHwnd(NULL);

	return 0L;
}



/*! �{�^���̃T�C�Y���v�Z */
int CFuncKeyWnd::CalcButtonSize( void )
{
	int			nButtonNum;
	RECT		rc;
	int			nCxHScroll;
	int			nCyHScroll;
	int			nCxVScroll;
	int			nCyVScroll;
	::GetWindowRect( GetHwnd(), &rc );

	nButtonNum = _countof( m_hwndButtonArr );

	if( NULL == m_hwndSizeBox ){
//		return ( rc.right - rc.left - nButtonNum - ( (nButtonNum + m_nButtonGroupNum - 1) / m_nButtonGroupNum - 1 ) * 12 ) / nButtonNum;
		nCxVScroll = 0;
	}else{
		/* �T�C�Y�{�b�N�X�̈ʒu�A�T�C�Y�ύX */
		nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
		nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );
		::MoveWindow( m_hwndSizeBox,  rc.right - rc.left - nCxVScroll, rc.bottom - rc.top - nCyHScroll, nCxVScroll, nCyHScroll, TRUE );
//		::MoveWindow( m_hwndSizeBox,  0, 0, nCxVScroll, nCyHScroll, TRUE );

//		return ( rc.right - rc.left - nCxVScroll = - nButtonNum -  ( (nButtonNum + m_nButtonGroupNum - 1) / m_nButtonGroupNum - 1 ) * 12 ) / nButtonNum;
	}
	return ( rc.right - rc.left - nCxVScroll - nButtonNum -  ( (nButtonNum + m_nButtonGroupNum - 1) / m_nButtonGroupNum - 1 ) * 12 ) / nButtonNum;

}



/*! �{�^���̐���
	@date 2007.02.05 ryoji �{�^���̐����ʒu�E���̐ݒ菈�����폜�iOnSize�ōĔz�u�����̂ŕs�v�j
*/
void CFuncKeyWnd::CreateButtons( void )
{
	RECT	rcParent;
	int		nButtonHeight;
	int		i;

	::GetWindowRect( GetHwnd(), &rcParent );
	nButtonHeight = nButtonHeight = rcParent.bottom - rcParent.top - 2;

	for( i = 0; i < _countof(	m_nFuncCodeArr ); ++i ){
		m_nFuncCodeArr[i] = F_0;
	}

	for( i = 0; i < _countof( m_hwndButtonArr ); ++i ){
		m_hwndButtonArr[i] = ::CreateWindow(
			_T("BUTTON"),						// predefined class
			_T(""),								// button text
			WS_VISIBLE | WS_CHILD | BS_LEFT,	// styles
			// Size and position values are given explicitly, because
			// the CW_USEDEFAULT constant gives zero values for buttons.
			0,					// starting x position
			0 + 1,				// starting y position
			0,					// button width
			nButtonHeight,		// button height
			GetHwnd(),				// parent window
			NULL,				// No menu
			(HINSTANCE) GetWindowLongPtr(GetHwnd(), GWLP_HINSTANCE),	// Modified by KEITA for WIN64 2003.9.6
			NULL				// pointer not needed
		);
		/* �t�H���g�ύX */
		::SendMessageAny( m_hwndButtonArr[i], WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0) );
	}
	m_nCurrentKeyState = -1;
	return;
}




/*! �T�C�Y�{�b�N�X�̕\���^��\���؂�ւ� */
void CFuncKeyWnd::SizeBox_ONOFF( bool bSizeBox )
{

	RECT		rc;
	::GetWindowRect( GetHwnd(), &rc );
	if( m_bSizeBox == bSizeBox ){
		return;
	}
	if( m_bSizeBox ){
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
		m_bSizeBox = false;
		OnSize( NULL, 0, 0, 0 );
	}else{
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 						/* no extended styles			*/
			_T("SCROLLBAR"),				/* scroll bar control class		*/
			NULL,						/* text for window title bar	*/
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,							/* horizontal position			*/
			0,							/* vertical position			*/
			200,						/* width of the scroll bar		*/
			CW_USEDEFAULT,				/* default height				*/
			GetHwnd(), 				/* handle of main window		*/
			(HMENU) NULL,				/* no menu for a scroll bar 	*/
			GetAppInstance(),				/* instance owning this window	*/
			(LPVOID) NULL			/* pointer not needed				*/
		);
		::ShowWindow( m_hwndSizeBox, SW_SHOW );
		m_bSizeBox = true;
		OnSize( NULL, 0, 0, 0 );
	}
	return;
}



// �^�C�}�[�̍X�V���J�n�^��~����B 20060126 aroka
// �t�@���N�V�����L�[�\���̓^�C�}�[�ɂ��X�V���Ă��邪�A
// �A�v���̃t�H�[�J�X���O�ꂽ�Ƃ��ɐe�E�B���h�E����ON/OFF��
//	�Ăяo���Ă��炤���Ƃɂ��A�]�v�ȕ��ׂ��~�������B
void CFuncKeyWnd::Timer_ONOFF( bool bStart )
{
	if( NULL != GetHwnd() ){
		if( bStart ){
			/* �^�C�}�[���N�� */
			if( 0 == ::SetTimer( GetHwnd(), IDT_FUNCWND, TIMER_TIMEOUT, NULL ) ){
				WarningMessage(	GetHwnd(), _T("CFuncKeyWnd::Open()\n�^�C�}�[���N���ł��܂���B\n�V�X�e�����\�[�X���s�����Ă���̂�������܂���B") );
			}
		} else {
			/* �^�C�}�[���폜 */
			::KillTimer( GetHwnd(), IDT_FUNCWND );
			m_nCurrentKeyState = -1;
		}
	}
	return;
}



