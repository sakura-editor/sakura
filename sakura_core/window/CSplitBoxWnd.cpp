/*!	@file
	@brief �����{�b�N�X�E�B���h�E�N���X

	@author Norio Nakatani

	@date 2002/2/3 aroka ���g�p�R�[�h����
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "window/CSplitBoxWnd.h"


CSplitBoxWnd::CSplitBoxWnd()
: CWnd(_T("::CSplitBoxWnd"))
{
	m_pszClassName = NULL;
	m_bVertical = TRUE;	/* ���������{�b�N�X�� */
	return;
}


CSplitBoxWnd::~CSplitBoxWnd()
{
}



HWND CSplitBoxWnd::Create( HINSTANCE hInstance, HWND hwndParent, int bVertical )
{
	int			nCyHScroll;
	int			nCxVScroll;
	RECT		rc;
	HCURSOR		hCursor;

	/* �E�B���h�E�N���X�쐬 */
	if( bVertical ){
		m_pszClassName = _T("VSplitBoxWnd");
		hCursor = ::LoadCursor( NULL, IDC_SIZENS );
	}
	else{
		m_pszClassName = _T("HSplitBoxWnd");
		hCursor = ::LoadCursor( NULL, IDC_SIZEWE );
	}
	RegisterWC(
		hInstance,
		NULL,	// Handle to the class icon.
		NULL,	// Handle to a small icon
		hCursor,// Handle to the class cursor.
		(HBRUSH)(COLOR_3DFACE + 1),// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		m_pszClassName// Pointer to a null-terminated string or is an atom.
	);

	m_bVertical = bVertical;
	/* �V�X�e���}�g���b�N�X�̎擾 */
	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );	/* �����X�N���[���o�[�̕� */

	/* �e�E�B���h�E�̃N���C�A���g�̈�̃T�C�Y���擾 */
	::GetClientRect( GetParentHwnd(), &rc );

	/* ���N���X�����o�Ăяo�� */
	return CWnd::Create(
		hwndParent,
		0, // extended window style
		m_pszClassName,	// Pointer to a null-terminated string or is an atom.
		m_pszClassName, // pointer to window name
		WS_CHILD | WS_VISIBLE, // window style
		bVertical ? ( rc.right - nCxVScroll ):( 0 ), // horizontal position of window
		bVertical ? ( 0 ):( rc.bottom - nCyHScroll ), // vertical position of window
		bVertical ? ( nCxVScroll ):( 7 ), // window width
		bVertical ? ( 7 ):( nCyHScroll ), // window height
		NULL // handle to menu, or child-window identifier
	);
}




/* �`�揈�� */
void CSplitBoxWnd::Draw3dRect( HDC hdc, int x, int y, int cx, int cy,
	COLORREF clrTopLeft, COLORREF clrBottomRight )
{
	HBRUSH	hBrush;
	RECT	rc;
	hBrush = ::CreateSolidBrush( clrTopLeft );
	::SetRect( &rc, x, y, x + cx - 1, y + 1 );
	::FillRect( hdc, &rc, hBrush );
	::SetRect( &rc, x, y, x + 1, y + cy - 1 );
	::FillRect( hdc, &rc, hBrush );
	::DeleteObject( hBrush );

	hBrush = ::CreateSolidBrush( clrBottomRight );
	::SetRect( &rc, x + cx - 1, y, x + cx, y + cy );
	::FillRect( hdc, &rc, hBrush );
	::SetRect( &rc, x, y + cy - 1, x + cx, y + cy );
	::FillRect( hdc, &rc, hBrush );
	::DeleteObject( hBrush );
	return;
}




void CSplitBoxWnd::FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr )
{
	RECT	rc;
	::SetBkColor( hdc, clr );
	::SetRect( &rc, x, y, x + cx, y + cy );
	::ExtTextOutW_AnyBuild( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
	return;
}




// WM_PAINT
LRESULT CSplitBoxWnd::OnPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	int			nCyHScroll;
	int			nCxVScroll;
	int			nVSplitHeight;	/* ���������{�b�N�X�̍��� */
	int			nHSplitWidth;	/* ���������{�b�N�X�̕� */

	hdc = ::BeginPaint( hwnd, &ps );

	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );

	nVSplitHeight = 7;	/* ���������{�b�N�X�̍��� */
	nHSplitWidth = 7;	/* ���������{�b�N�X�̕� */

	if( m_bVertical ){
		/* ���������{�b�N�X�̕`�� */
		Draw3dRect( hdc, 0, 0, nCxVScroll, nVSplitHeight,
			::GetSysColor( COLOR_3DLIGHT ), ::GetSysColor( COLOR_3DDKSHADOW )
		 );
		Draw3dRect( hdc, 1, 1, nCxVScroll - 2, nVSplitHeight - 2,
			::GetSysColor( COLOR_3DHILIGHT ), ::GetSysColor( COLOR_3DSHADOW )
		 );
	}else{
		/* ���������{�b�N�X�̕`�� */
		Draw3dRect( hdc, 0, 0, nHSplitWidth, nCyHScroll,
			::GetSysColor( COLOR_3DLIGHT ), ::GetSysColor( COLOR_3DDKSHADOW )
		 );

		Draw3dRect( hdc, 1, 1, nHSplitWidth - 2, nCyHScroll - 2,
			::GetSysColor( COLOR_3DHILIGHT ), ::GetSysColor( COLOR_3DSHADOW )
		 );
	}

	::EndPaint(hwnd, &ps);
	return 0L;
}




//WM_LBUTTONDOWN
LRESULT CSplitBoxWnd::OnLButtonDown( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC			hdc;
	RECT		rc;
	RECT		rc2;
	int			nCyHScroll;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	::SetCapture( hwnd );
	if( m_bVertical ){
		m_nDragPosY = 1;

		hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
		::SetBkColor( hdc, RGB(0, 0, 0) );
		hBrush = ::CreateSolidBrush( RGB(255,255,255) );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

		::SetROP2( hdc, R2_XORPEN );
		::SetBkMode( hdc, TRANSPARENT );
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
		rc.bottom -= nCyHScroll;

		rc2.left = -1;
		rc2.top = m_nDragPosY;
		rc2.right = rc.right;
		rc2.bottom = rc2.top + 6;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( ::GetParent( GetParentHwnd() ), hdc );
	}else{
		m_nDragPosX = 1;

		hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
		::SetBkColor( hdc, RGB(0, 0, 0) );
		hBrush = ::CreateSolidBrush( RGB(255,255,255) );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

		::SetROP2( hdc, R2_XORPEN );
		::SetBkMode( hdc, TRANSPARENT );
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );

		rc2.left = m_nDragPosX;
		rc2.top = 0;
		rc2.right = rc2.left + 6;
		rc2.bottom = rc.bottom;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( ::GetParent( GetParentHwnd() ), hdc );

	}
	return 0L;
}



//WM_MOUSEMOVE
LRESULT CSplitBoxWnd::OnMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC			hdc;
	int			xPos;
	int			yPos;
	RECT		rc;
	RECT		rc2;
	int			nCyHScroll;
	int			nCxVScroll;
	POINT		po;
	POINT		po_top;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	if( hwnd != ::GetCapture() ){
		return 0L;
	}
	if( m_bVertical ){
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
		rc.bottom -= nCyHScroll;

		::GetCursorPos( &po );

		po_top.x = 0;
		po_top.y = 0;
		::ClientToScreen( ::GetParent( GetParentHwnd() ), &po_top );
		if( po.y < po_top.y ){
			po.y = po_top.y;
		}

		po_top.x = 0;
		po_top.y = rc.bottom;
		::ClientToScreen( ::GetParent( GetParentHwnd() ), &po_top );
		if( po.y > po_top.y - 6 ){
			po.y = po_top.y - 6;
		}

		::ScreenToClient( ::GetParent( GetParentHwnd() ), &po );
		xPos = po.x;
		yPos = po.y;

		if( yPos != m_nDragPosY ){
//			MYTRACE( _T("xPos=%d yPos=%d\n"), xPos, yPos );

			hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
			::SetBkColor( hdc, RGB(0, 0, 0) );
			hBrush = ::CreateSolidBrush( RGB(255,255,255) );
			hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

			::SetROP2( hdc, R2_XORPEN );
			::SetBkMode( hdc, TRANSPARENT );
			rc2.left = -1;
			rc2.top = m_nDragPosY;
			rc2.right = rc.right;
			rc2.bottom = rc2.top + 6;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

			m_nDragPosY =  po.y;

			rc2.left = -1;
			rc2.top = m_nDragPosY;
			rc2.right = rc.right;
			rc2.bottom = rc2.top + 6;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

			::SelectObject( hdc, hBrushOld );
			::DeleteObject( hBrush );
			::ReleaseDC( ::GetParent( GetParentHwnd() ), hdc );

		}
	}else{
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );	/* �����X�N���[���o�[�̕� */
		rc.right -= nCxVScroll;

		::GetCursorPos( &po );

		po_top.x = 0;
		po_top.y = 0;
		::ClientToScreen( ::GetParent( GetParentHwnd() ), &po_top );
		if( po.x < po_top.x ){
			po.x = po_top.x;
		}

		po_top.x = rc.right;
		po_top.y = 0;
		::ClientToScreen( ::GetParent( GetParentHwnd() ), &po_top );
		if( po.x > po_top.x - 6 ){
			po.x = po_top.x - 6;
		}

		::ScreenToClient( ::GetParent( GetParentHwnd() ), &po );
		xPos = po.x;
		yPos = po.y;

		if( xPos != m_nDragPosX ){
//			MYTRACE( _T("xPos=%d yPos=%d\n"), xPos, yPos );

			hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
			::SetBkColor( hdc, RGB(0, 0, 0) );
			hBrush = ::CreateSolidBrush( RGB(255,255,255) );
			hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

			::SetROP2( hdc, R2_XORPEN );
			::SetBkMode( hdc, TRANSPARENT );

			rc2.left = m_nDragPosX;
			rc2.top = 0;
			rc2.right = rc2.left + 6;
			rc2.bottom = rc.bottom;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

			m_nDragPosX =  po.x;

			rc2.left = m_nDragPosX;
			rc2.top = 0;
			rc2.right = rc2.left + 6;
			rc2.bottom = rc.bottom;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

			::SelectObject( hdc, hBrushOld );
			::DeleteObject( hBrush );
			::ReleaseDC( ::GetParent( GetParentHwnd() ), hdc );
		}
	}
	return 0L;
}




//WM_LBUTTONUP
LRESULT CSplitBoxWnd::OnLButtonUp( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC			hdc;
	RECT		rc;
	RECT		rc2;
	int			nCyHScroll;
	int			nCxVScroll;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	if( hwnd != ::GetCapture() ){
		return 0L;
	}
	if( m_bVertical ){
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
		rc.bottom -= nCyHScroll;

		hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
		::SetBkColor( hdc, RGB(0, 0, 0) );
		hBrush = ::CreateSolidBrush( RGB(255,255,255) );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

		::SetROP2( hdc, R2_XORPEN );
		::SetBkMode( hdc, TRANSPARENT );
		rc2.left = -1;
		rc2.top = m_nDragPosY;
		rc2.right = rc.right;
		rc2.bottom = rc2.top + 6;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( ::GetParent( GetParentHwnd() ), hdc );

		/* �e�E�B���h�E�ɁA���b�Z�[�W���|�X�g���� */
		::PostMessageAny( GetParentHwnd(), MYWM_DOSPLIT, (WPARAM)0, (LPARAM)m_nDragPosY );

	}else{
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );	/* �����X�N���[���o�[�̕� */
		rc.right -= nCxVScroll;

		hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
		::SetBkColor( hdc, RGB(0, 0, 0) );
		hBrush = ::CreateSolidBrush( RGB(255,255,255) );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

		::SetROP2( hdc, R2_XORPEN );
		::SetBkMode( hdc, TRANSPARENT );

		rc2.left = m_nDragPosX;
		rc2.top = 0;
		rc2.right = rc2.left + 6;
		rc2.bottom = rc.bottom;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( GetParentHwnd(), hdc );

		/* �e�E�B���h�E�ɁA���b�Z�[�W���|�X�g���� */
		::PostMessageAny( GetParentHwnd(), MYWM_DOSPLIT, (WPARAM)m_nDragPosX, (LPARAM)0 );
	}
	::ReleaseCapture();
	return 0L;
}




//WM_LBUTTONDBLCLK
LRESULT CSplitBoxWnd::OnLButtonDblClk( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	RECT		rc;
	int			nCyHScroll;
	if( m_bVertical ){
		::GetClientRect( GetParentHwnd(), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
		rc.bottom -= nCyHScroll;

		/* �e�E�B���h�E�ɁA���b�Z�[�W���|�X�g���� */
		::PostMessageAny( GetParentHwnd(), MYWM_DOSPLIT, (WPARAM)0, (LPARAM)(rc.bottom / 2) );
	}
	else{
		::GetClientRect( GetParentHwnd(), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
		rc.bottom -= nCyHScroll;

		/* �e�E�B���h�E�ɁA���b�Z�[�W���|�X�g���� */
		::PostMessageAny( GetParentHwnd(), MYWM_DOSPLIT, (WPARAM)(rc.right / 2), (LPARAM)0 );
	}
	return 0L;
}



