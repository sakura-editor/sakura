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

class CSplitBoxWnd;

#ifndef _CSPLITBOXWND_H_
#define _CSPLITBOXWND_H_

#include "CWnd.h"
//#include <windows.h>
#include "mymessage.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
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



public:
	const char*	m_pszClassName;	/* �N���X�� */
//	HINSTANCE	m_hInstance;	/* �C���X�^���X�n���h�� */
//	HWND		m_hWnd;			/* �E�B���h�E�n���h�� */
//	HWND		m_hwndParent; 	/* �e�E�B���h�E�n���h�� */
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


/*[EOF]*/
