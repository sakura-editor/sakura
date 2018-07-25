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
	HWND Create( HINSTANCE , HWND , int );

	static void Draw3dRect( HDC , int , int , int , int , COLORREF , COLORREF );
	static void FillSolidRect( HDC , int , int , int , int , COLORREF );

//	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージディスパッチャ */



private:
	int			m_bVertical;	/* 垂直分割ボックスか */
	int			m_nDragPosY;
	int			m_nDragPosX;
protected:
	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );/* 描画処理 */
	LRESULT OnLButtonDown( HWND, UINT, WPARAM, LPARAM );// WM_LBUTTONDOWN
	LRESULT OnMouseMove( HWND, UINT, WPARAM, LPARAM );// WM_MOUSEMOVE
	LRESULT OnLButtonUp( HWND, UINT, WPARAM, LPARAM );//WM_LBUTTONUP
	LRESULT OnLButtonDblClk( HWND, UINT, WPARAM, LPARAM );//WM_LBUTTONDBLCLK


};


///////////////////////////////////////////////////////////////////////
#endif /* _CSPLITBOXWND_H_ */



