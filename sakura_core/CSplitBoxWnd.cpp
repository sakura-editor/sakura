//	$Id$
/*!	@file
	�����{�b�N�X�E�B���h�E�N���X
	
	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "CSplitBoxWnd.h"
#include "debug.h"

CSplitBoxWnd::CSplitBoxWnd()
{
	strcat( m_szClassInheritances, "::CSplitBoxWnd" );
	m_pszClassName = NULL;
//	m_hWnd = NULL;
	m_bVertical = TRUE;	/* ���������{�b�N�X�� */
	return;
}


CSplitBoxWnd::~CSplitBoxWnd()
{
//	if( NULL != m_hWnd ){
//		::DestroyWindow( m_hWnd );
//		m_hWnd = NULL;
//	}
	return;
}



HWND CSplitBoxWnd::Create( HINSTANCE hInstance, HWND hwndParent, int bVertical )
{
	int			nCxHScroll;
	int			nCyHScroll;
	int			nCxVScroll;
	int			nCyVScroll;
//	WNDCLASS	wc;
	RECT		rc;
	HCURSOR		hCursor;

	/* ������ */
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;

//	/* ������ */
//	Init(
//		hInstance,	// handle to application instance
//		hwndParent	 // handle to parent or owner window
//	);
	/* �E�B���h�E�N���X�쐬 */
	if( bVertical ){
		m_pszClassName = "VSplitBoxWnd";
		hCursor = ::LoadCursor( NULL, IDC_SIZENS );
//		lpfnWndProc = (WNDPROC)VSplitBoxWndProc;
	}else{
		m_pszClassName = "HSplitBoxWnd";
		hCursor = ::LoadCursor( NULL, IDC_SIZEWE );
//		lpfnWndProc = (WNDPROC)HSplitBoxWndProc;
	}
	RegisterWC( 
		/* WNDCLASS�p */
		NULL,// Handle to the class icon. 
		NULL,	//Handle to a small icon  
		hCursor,// Handle to the class cursor. 
		(HBRUSH)(COLOR_3DFACE + 1),// Handle to the class background brush. 
		NULL/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file. 
		m_pszClassName// Pointer to a null-terminated string or is an atom.
	);

	m_bVertical = bVertical;
	/* �V�X�e���}�g���b�N�X�̎擾 */
	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );	/* �����X�N���[���o�[�̕� */
	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );	/* �����X�N���[���o�[�̕� */
	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );	/* �����X�N���[���o�[�̍��� */

	/* �e�E�B���h�E�̃N���C�A���g�̈�̃T�C�Y���擾 */
	::GetClientRect( m_hwndParent, &rc );

	/* ���N���X�����o�Ăяo�� */
	return CWnd::Create( 
		/* CreateWindowEx()�p */
		0, // extended window style
		m_pszClassName,	// Pointer to a null-terminated string or is an atom.
		m_pszClassName, // pointer to window name
		WS_CHILD | WS_VISIBLE, // window style
		bVertical ? ( rc.right - nCxVScroll ):( 0 ), // horizontal position of window
		bVertical ? ( 0                     ):( rc.bottom - nCyHScroll ), // vertical position of window
		bVertical ? ( nCxVScroll ):( 7 ), // window width
		bVertical ? ( 7          ):( nCyHScroll ), // window height
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
//	FillSolidRect( hdc, x, y, cx - 1, 1, clrTopLeft);
//	FillSolidRect( hdc, x, y, 1, cy - 1, clrTopLeft);
//	FillSolidRect( hdc, x + cx, y, -1, cy, clrBottomRight);
//	FillSolidRect( hdc, x, y + cy, cx, -1, clrBottomRight);
	return;
}




void CSplitBoxWnd::FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr )
{
	RECT	rc;
	::SetBkColor( hdc, clr );
	::SetRect( &rc, x, y, x + cx, y + cy );
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
	return;
}




// WM_PAINT
LRESULT CSplitBoxWnd::OnPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	int			nCxHScroll;
	int			nCyHScroll;
	int			nCxVScroll;
	int			nCyVScroll;
	int			nVSplitHeight;	/* ���������{�b�N�X�̍��� */
	int			nHSplitWidth;	/* ���������{�b�N�X�̕� */

	hdc = ::BeginPaint( hwnd, &ps );

	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );

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
	int			nCxVScroll;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
//	::SetFocus( m_hwndParent );
	::SetCapture( hwnd );
	if( m_bVertical ){
		m_nDragPosY = 1;

		hdc = ::GetDC( ::GetParent( m_hwndParent ) );
		::SetBkColor( hdc, RGB(0, 0, 0) );
		hBrush = ::CreateSolidBrush( RGB(255,255,255) );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

		::SetROP2( hdc, R2_XORPEN );
		::SetBkMode( hdc, TRANSPARENT );
		::GetClientRect( ::GetParent( m_hwndParent ), &rc );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );	/* �����X�N���[���o�[�̕� */
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
		rc.bottom -= nCyHScroll;

		rc2.left = -1;
		rc2.top = m_nDragPosY;
		rc2.right = rc.right;
		rc2.bottom = rc2.top + 6;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( ::GetParent( m_hwndParent ), hdc );
	}else{
		m_nDragPosX = 1;

		hdc = ::GetDC( ::GetParent( m_hwndParent ) );
		::SetBkColor( hdc, RGB(0, 0, 0) );
		hBrush = ::CreateSolidBrush( RGB(255,255,255) );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

		::SetROP2( hdc, R2_XORPEN );
		::SetBkMode( hdc, TRANSPARENT );
		::GetClientRect( ::GetParent( m_hwndParent ), &rc );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );	/* �����X�N���[���o�[�̕� */
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
//		rc.bottom -= nCyHScroll;

		rc2.left = m_nDragPosX;
		rc2.top = 0;
		rc2.right = rc2.left + 6;
		rc2.bottom = rc.bottom;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( ::GetParent( m_hwndParent ), hdc );

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
		::GetClientRect( ::GetParent( m_hwndParent ), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
		rc.bottom -= nCyHScroll;

		::GetCursorPos( &po );

		po_top.x = 0;
		po_top.y = 0;
		::ClientToScreen( ::GetParent( m_hwndParent ), &po_top );
		if( po.y < po_top.y ){
			po.y = po_top.y;
		}

		po_top.x = 0;
		po_top.y = rc.bottom;
		::ClientToScreen( ::GetParent( m_hwndParent ), &po_top );
		if( po.y > po_top.y - 6 ){
			po.y = po_top.y - 6;
		}

		::ScreenToClient( ::GetParent( m_hwndParent ), &po );
		xPos = po.x;
		yPos = po.y;

		if( yPos != m_nDragPosY ){
//			MYTRACE( "xPos=%d yPos=%d\n", xPos, yPos );

			hdc = ::GetDC( ::GetParent( m_hwndParent ) );
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
			::ReleaseDC( ::GetParent( m_hwndParent ), hdc );

		}
	}else{
		::GetClientRect( ::GetParent( m_hwndParent ), &rc );
//		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
//		rc.bottom -= nCyHScroll;
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );	/* �����X�N���[���o�[�̕� */
		rc.right -= nCxVScroll;

		::GetCursorPos( &po );

		po_top.x = 0;
		po_top.y = 0;
		::ClientToScreen( ::GetParent( m_hwndParent ), &po_top );
		if( po.x < po_top.x ){
			po.x = po_top.x;
		}

		po_top.x = rc.right;
		po_top.y = 0;
		::ClientToScreen( ::GetParent( m_hwndParent ), &po_top );
		if( po.x > po_top.x - 6 ){
			po.x = po_top.x - 6;
		}

		::ScreenToClient( ::GetParent( m_hwndParent ), &po );
		xPos = po.x;
		yPos = po.y;

		if( xPos != m_nDragPosX ){
//			MYTRACE( "xPos=%d yPos=%d\n", xPos, yPos );

			hdc = ::GetDC( ::GetParent( m_hwndParent ) );
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
			::ReleaseDC( ::GetParent( m_hwndParent ), hdc );
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
		::GetClientRect( ::GetParent( m_hwndParent ), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
		rc.bottom -= nCyHScroll;

		hdc = ::GetDC( ::GetParent( m_hwndParent ) );
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
		::ReleaseDC( ::GetParent( m_hwndParent ), hdc );
	
		/* �e�E�B���h�E�ɁA���b�Z�[�W���|�X�g���� */
		::PostMessage( m_hwndParent, MYWM_DOSPLIT, (WPARAM)0, (LPARAM)m_nDragPosY );
	
	}else{
		::GetClientRect( ::GetParent( m_hwndParent ), &rc );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );	/* �����X�N���[���o�[�̕� */
		rc.right -= nCxVScroll;

		hdc = ::GetDC( ::GetParent( m_hwndParent ) );
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
		::ReleaseDC( m_hwndParent, hdc );

		/* �e�E�B���h�E�ɁA���b�Z�[�W���|�X�g���� */
		::PostMessage( m_hwndParent, MYWM_DOSPLIT, (WPARAM)m_nDragPosX, (LPARAM)0 );
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
		::GetClientRect( m_hwndParent, &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
		rc.bottom -= nCyHScroll;
	
		/* �e�E�B���h�E�ɁA���b�Z�[�W���|�X�g���� */
		::PostMessage( m_hwndParent, MYWM_DOSPLIT, (WPARAM)0, (LPARAM)(rc.bottom / 2) );
	}else{
		::GetClientRect( m_hwndParent, &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* �����X�N���[���o�[�̍��� */
		rc.bottom -= nCyHScroll;

		/* �e�E�B���h�E�ɁA���b�Z�[�W���|�X�g���� */
		::PostMessage( m_hwndParent, MYWM_DOSPLIT, (WPARAM)(rc.right / 2), (LPARAM)0 );
	}
	return 0L;
}


/*[EOF]*/

