//	$Id$
/*!	@file

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

class CDlgDebug;

#ifndef _CDLGDEBUG_H_
#define _CDLGDEBUG_H_

#include <windows.h>
#include "CMemory.h"

//#define MAX_SRCHTXT 1024
//#define MAX_TEXTARR 32

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgDebug
{
public:
	/*
	||  Constructors
	*/
	CDlgDebug();
	~CDlgDebug();

	/*
	||  Attributes & Operations
	*/
	BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */
	int DoModal( HINSTANCE, HWND, CMemory& );	/* ���[�_���_�C�A���O�̕\�� */

	HINSTANCE	m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND		m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	HWND		m_hWnd;		/* ���̃_�C�A���O�̃n���h�� */

	CMemory		m_cmemDebugInfo;


protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGDEBUG_H_ */

/*[EOF]*/
