/*! @file */
/*
	Copyright (C) 2012, Moca

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
#ifndef SAKURA_CAUTOSCROLLWND_H_
#define SAKURA_CAUTOSCROLLWND_H_

#include "CWnd.h"
class CEditView;

class CAutoScrollWnd: public CWnd
{
public:
	CAutoScrollWnd();
	virtual ~CAutoScrollWnd();
	HWND Create( HINSTANCE hInstance, HWND hwndParent, bool bVertical, bool bHorizontal,
				 const CMyPoint& point, CEditView* view );
	void Close();

private:
	HBITMAP	m_hCenterImg;
	CEditView*	m_cView;
protected:
	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	LRESULT OnLButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
};
#endif
