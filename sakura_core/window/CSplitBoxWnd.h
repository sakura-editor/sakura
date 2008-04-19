/*!	@file
	@brief �����{�b�N�X�E�B���h�E�N���X

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
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �����{�b�N�X�E�B���h�E�N���X
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

//	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ���b�Z�[�W�f�B�X�p�b�`�� */



private:
	const TCHAR*	m_pszClassName;	/* �N���X�� */
	int			m_bVertical;	/* ���������{�b�N�X�� */
	int			m_nDragPosY;
	int			m_nDragPosX;
protected:
	/* ���z�֐� */

	/* ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q�� */
	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );/* �`�揈�� */
	LRESULT OnLButtonDown( HWND, UINT, WPARAM, LPARAM );// WM_LBUTTONDOWN
	LRESULT OnMouseMove( HWND, UINT, WPARAM, LPARAM );// WM_MOUSEMOVE
	LRESULT OnLButtonUp( HWND, UINT, WPARAM, LPARAM );//WM_LBUTTONUP
	LRESULT OnLButtonDblClk( HWND, UINT, WPARAM, LPARAM );//WM_LBUTTONDBLCLK


};


///////////////////////////////////////////////////////////////////////
#endif /* _CSPLITBOXWND_H_ */



