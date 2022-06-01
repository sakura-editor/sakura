﻿/*!	@file
	@brief 分割ボックスウィンドウクラス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CSPLITBOXWND_D85ABC4D_AF8F_4B42_B1E5_BA066925314E_H_
#define SAKURA_CSPLITBOXWND_D85ABC4D_AF8F_4B42_B1E5_BA066925314E_H_
#pragma once

class CSplitBoxWnd;

#include "CWnd.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 分割ボックスウィンドウクラス
*/
class CSplitBoxWnd final : public CWnd
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
	LRESULT OnPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;/* 描画処理 */
	LRESULT OnLButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;// WM_LBUTTONDOWN
	LRESULT OnMouseMove(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;// WM_MOUSEMOVE
	LRESULT OnLButtonUp(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;//WM_LBUTTONUP
	LRESULT OnLButtonDblClk(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;//WM_LBUTTONDBLCLK
};
#endif /* SAKURA_CSPLITBOXWND_D85ABC4D_AF8F_4B42_B1E5_BA066925314E_H_ */
