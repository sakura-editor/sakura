//	$Id$
/*!	@file
	�c�[���`�b�v
	
	@author Norio Nakatani
	@date 1998/10/30 �V�K�쐬
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

class CTipWnd;

#ifndef _CTIPWND_H_
#define _CTIPWND_H_

#include "CWnd.h"
//#include <windows.h>
#include "CMemory.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CTipWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CTipWnd();
	~CTipWnd();
	void Create( HINSTANCE, HWND );	/* ������ */

	/*
	||  Attributes & Operations
	*/
	void Show( int, int, char* );	/* Tip��\�� */
	void Hide( void );	/* Tip������ */
//	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */

	char*		m_pszClassName;	/* Mutex�쐬�p�E�E�B���h�E�N���X�� */
//	HINSTANCE	m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
//	HWND		m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
//	HWND		m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */
	HFONT		m_hFont;
//	HFONT		m_hFontOld;

	CMemory		m_cKey;			/* �L�[�̓��e�f�[�^ */
	BOOL		m_KeyWasHit;	/* �L�[���q�b�g������ */
	
	CMemory		m_cInfo;		/* Tip�̓��e�f�[�^ */

protected:
	/*
	||  �����w���p�֐�
	*/
	void ComputeWindowSize( HDC, HFONT, const char*, RECT* );	/* �E�B���h�E�̃T�C�Y�����߂� */
	void DrawTipText( HDC, HFONT, const char* );	/* �E�B���h�E�̃e�L�X�g��\�� */

	/* ���z�֐� */

	/* ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q�� */
	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );/* �`�揈�� */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CTIPWND_H_ */

/*[EOF]*/
