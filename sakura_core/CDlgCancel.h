//	$Id$
/*!	@file
	�L�����Z���{�^���_�C�A���O

	@author Norio Nakatani
	@date 1998/09/09 �쐬
    @date 1999/12/02 �č쐬
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
/************************************************************************

	CDlgCancel.h

	UPDATE:	1999.12/02 �č쐬
	CREATE: 1998.09/09 �V�K�쐬
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/

class CDlgCancel;

#ifndef _CDLGCANCEL_H_
#define _CDLGCANCEL_H_

//#include <windows.h>
//#include "CMemory.h"
#include "CDialog.h"


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgCancel : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgCancel();
//	void Create( HINSTANCE, HWND );	/* ������ */

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, int );	/* ���[�h���X�_�C�A���O�̕\�� */
	HWND DoModeless( HINSTANCE, HWND, int );	/* ���[�h���X�_�C�A���O�̕\�� */

//	HWND Open( LPCTSTR );
//	void Close( void );	/* ���[�h���X�_�C�A���O�̍폜 */
	BOOL IsCanceled( void ){ return m_bCANCEL; } /* IDCANCEL�{�^���������ꂽ���H */
//	BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */

//	HINSTANCE	m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
//	HWND		m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
//	HWND		m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */
	BOOL		m_bCANCEL;		/* IDCANCEL�{�^���������ꂽ */

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGCANCEL_H_ */


/*[EOF]*/
