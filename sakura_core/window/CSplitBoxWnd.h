/*!	@file
	@brief 分割ボックスウィンドウクラス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CSplitBoxWnd;

#ifndef _CSPLITBOXWND_H_
#define _CSPLITBOXWND_H_

#include "CWnd.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 分割ボックスウィンドウクラス
*/
class CSplitBoxWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CSplitBoxWnd();
	virtual ~CSplitBoxWnd();
	HWND Create(HINSTANCE hInstance, HWND hwndParent, int bVertical);

	static void Draw3dRect(HDC hdc, int x, int y, int cx, int cy, COLORREF clrTopLeft, COLORREF clrBottomRight);
	static void FillSolidRect(HDC hdc, int x, int y, int cx, int cy, COLORREF clr);

//	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージディスパッチャ */

private:
	int			m_bVertical;	/* 垂直分割ボックスか */
	int			m_nDragPosY;
	int			m_nDragPosX;
protected:
	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	LRESULT OnPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);/* 描画処理 */
	LRESULT OnLButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);// WM_LBUTTONDOWN
	LRESULT OnMouseMove(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);// WM_MOUSEMOVE
	LRESULT OnLButtonUp(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);//WM_LBUTTONUP
	LRESULT OnLButtonDblClk(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);//WM_LBUTTONDBLCLK
};

///////////////////////////////////////////////////////////////////////
#endif /* _CSPLITBOXWND_H_ */

