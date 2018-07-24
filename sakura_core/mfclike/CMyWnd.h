/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CMYWND_2BBCE2D2_F4DC_4809_BFFE_476824F2E0ABR_H_
#define SAKURA_CMYWND_2BBCE2D2_F4DC_4809_BFFE_476824F2E0ABR_H_

/*
	MFCのCWnd的なクラス。

	2008.01.26 kobake 作成
*/

class CMyWnd{
public:
	CMyWnd() : m_hWnd(NULL) { }

	void SetHwnd(HWND hwnd){ m_hWnd = hwnd; }
	HWND GetHwnd() const{ return m_hWnd; }
	HWND GetSafeHwnd() const{ return this?m_hWnd:NULL; }
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

#endif /* SAKURA_CMYWND_2BBCE2D2_F4DC_4809_BFFE_476824F2E0ABR_H_ */
/*[EOF]*/
