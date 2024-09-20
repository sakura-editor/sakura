/*!	@file
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

#include "window/CWnd.h"

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
	explicit CSplitBoxWnd(bool isVertical) noexcept;
	~CSplitBoxWnd() override = default;

	HWND Create(HINSTANCE hInstance, HWND hwndParent, int bVertical);

	static void Draw3dRect(HDC hdc, int x, int y, int cx, int cy, COLORREF clrTopLeft, COLORREF clrBottomRight);
	static void FillSolidRect(HDC hdc, int x, int y, int cx, int cy, COLORREF clr);

private:
	int			m_bVertical;	/* 垂直分割ボックスか */
	int			m_nDragPosY = -1;
	int			m_nDragPosX = -1;

protected:
	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	LRESULT OnPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;/* 描画処理 */
	LRESULT OnLButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;// WM_LBUTTONDOWN
	LRESULT OnMouseMove(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;// WM_MOUSEMOVE
	LRESULT OnLButtonUp(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;//WM_LBUTTONUP
	LRESULT OnLButtonDblClk(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;//WM_LBUTTONDBLCLK
};

class CVSplitBoxWnd final : public CSplitBoxWnd
{
public:
	CVSplitBoxWnd() noexcept : CSplitBoxWnd(true) {}
};

class CHSplitBoxWnd final : public CSplitBoxWnd
{
public:
	CHSplitBoxWnd() noexcept : CSplitBoxWnd(false) {}
};

#endif /* SAKURA_CSPLITBOXWND_D85ABC4D_AF8F_4B42_B1E5_BA066925314E_H_ */
