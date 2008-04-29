/*!	@file

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgDebug;

#ifndef _CDLGDEBUG_H_
#define _CDLGDEBUG_H_

#include <windows.h>
#include "mem/CMemory.h"

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
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */
	int DoModal( HINSTANCE, HWND, const CNativeA& );	/* ���[�_���_�C�A���O�̕\�� */

	HINSTANCE	m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND		m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	HWND		m_hWnd;		/* ���̃_�C�A���O�̃n���h�� */

private:
	CNativeA		m_cmemDebugInfo;


protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGDEBUG_H_ */


