/*! @file */
/*
	Copyright (C) 2012, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CAutoScrollWnd.h"
#include "view/CEditView.h"
#include "sakura_rc.h"

CAutoScrollWnd::CAutoScrollWnd()
: CWnd(L"::CAutoScrollWnd")
{
	m_hCenterImg = nullptr;
	return;
}

CAutoScrollWnd::~CAutoScrollWnd()
{
}

HWND CAutoScrollWnd::Create( HINSTANCE hInstance, HWND hwndParent, bool bVertical, bool bHorizontal, const CMyPoint& point, CEditView* view )
{
	LPCWSTR pszClassName;

	m_cView = view;
	int idb, idc;
	if( bVertical ){
		if( bHorizontal ){
			idb = IDB_SCROLL_CENTER;
			idc = IDC_CURSOR_AUTOSCROLL_CENTER;
			pszClassName = L"SakuraAutoScrollCWnd";
		}else{
			idb = IDB_SCROLL_VERTICAL;
			idc = IDC_CURSOR_AUTOSCROLL_VERTICAL;
			pszClassName = L"SakuraAutoScrollVWnd";
		}
	}else{
		idb = IDB_SCROLL_HORIZONTAL;
		idc = IDC_CURSOR_AUTOSCROLL_HORIZONTAL;
		pszClassName = L"SakuraAutoScrollHWnd";
	}
	m_hCenterImg = (HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(idb), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	HCURSOR hCursor = ::LoadCursor(GetModuleHandle(nullptr), MAKEINTRESOURCE(idc));

	/* ウィンドウクラス作成 */
	RegisterWC(
		hInstance,
		nullptr,
		nullptr,
		hCursor,
		(HBRUSH)(COLOR_3DFACE + 1),
		nullptr,
		pszClassName
	);

	/* 基底クラスメンバ呼び出し */
	return CWnd::Create(
		/* 初期化 */
		hwndParent,
		0,
		pszClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName, // pointer to window name
		WS_CHILD | WS_VISIBLE, // window style
		point.x-16, // horizontal position of window
		point.y-16, // vertical position of window
		32, // window width
		32, // window height
		nullptr // handle to menu, or child-window identifier
	);
}

void CAutoScrollWnd::Close()
{
	this->DestroyWindow();

	if( m_hCenterImg ){
		::DeleteObject( m_hCenterImg );
		m_hCenterImg = nullptr;
	}
}

LRESULT CAutoScrollWnd::OnLButtonDown( [[maybe_unused]] HWND hWnd, [[maybe_unused]] UINT Msg, [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam )
{
	if( m_cView->m_nAutoScrollMode ){
		m_cView->AutoScrollExit();
	}
	return 0;
}

LRESULT CAutoScrollWnd::OnRButtonDown( [[maybe_unused]] HWND hWnd, [[maybe_unused]] UINT Msg, [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam )
{
	if( m_cView->m_nAutoScrollMode ){
		m_cView->AutoScrollExit();
	}
	return 0;
}

LRESULT CAutoScrollWnd::OnMButtonDown( [[maybe_unused]] HWND hWnd, [[maybe_unused]] UINT Msg, [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam )
{
	if( m_cView->m_nAutoScrollMode ){
		m_cView->AutoScrollExit();
	}
	return 0;
}

LRESULT CAutoScrollWnd::OnPaint( HWND hwnd, UINT, WPARAM, LPARAM )
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint( hwnd, &ps );
	HDC hdcBmp = ::CreateCompatibleDC( hdc );
	HBITMAP hBbmpOld = (HBITMAP)::SelectObject( hdcBmp, m_hCenterImg );
	::BitBlt( hdc, 0, 0, 32, 32, hdcBmp, 0, 0, SRCCOPY );
	::SelectObject( hdcBmp, hBbmpOld );
	::DeleteObject( hdcBmp );
	::EndPaint(hwnd, &ps);
	return 0;
}
