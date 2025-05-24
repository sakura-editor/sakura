/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CMYWND_AA99D3B1_9B1C_4DB5_A62C_AEE66BEFBD52_H_
#define SAKURA_CMYWND_AA99D3B1_9B1C_4DB5_A62C_AEE66BEFBD52_H_
#pragma once

/*
	MFCのCWnd的なクラス。

	2008.01.26 kobake 作成
*/

class CMyWnd{
public:
	CMyWnd() : m_hWnd(NULL) { }

	void SetHwnd(HWND hwnd){ m_hWnd = hwnd; }
	HWND GetHwnd() const{ return m_hWnd; }
	static HWND GetSafeHwnd(const CMyWnd* wnd) { return wnd ? wnd->m_hWnd : NULL; }
	void InvalidateRect(LPCRECT lpRect, BOOL bErase = TRUE){ ::InvalidateRect(m_hWnd, lpRect, bErase); }
	int ScrollWindowEx(int dx, int dy, const RECT* prcScroll, const RECT* prcClip, HRGN hrgnUpdate, RECT* prcUpdate, UINT uFlags)
	{
		return ::ScrollWindowEx(m_hWnd, dx, dy, prcScroll, prcClip, hrgnUpdate, prcUpdate, uFlags);
	}
	HDC GetDC() const
	{
		return ::GetDC(m_hWnd);
	}
	int ReleaseDC(HDC hdc) const
	{
		return ::ReleaseDC(m_hWnd,hdc);
	}
	HWND GetAncestor(UINT gaFlags) const
	{
		return ::GetAncestor(m_hWnd,gaFlags);
	}
	BOOL CreateCaret(HBITMAP hBitmap, int nWidth, int nHeight)
	{
		return ::CreateCaret(m_hWnd, hBitmap, nWidth, nHeight);
	}
	BOOL ClientToScreen(LPPOINT lpPoint) const
	{
		return ::ClientToScreen(m_hWnd, lpPoint);
	}

	BOOL UpdateWindow()
	{
		return ::UpdateWindow(m_hWnd);
	}
	HWND SetFocus()
	{
		return ::SetFocus(m_hWnd);
	}
	BOOL GetClientRect(LPRECT lpRect) const
	{
		return ::GetClientRect(m_hWnd, lpRect);
	}
private:
	HWND m_hWnd;
};
#endif /* SAKURA_CMYWND_AA99D3B1_9B1C_4DB5_A62C_AEE66BEFBD52_H_ */
