/*!	@file
	@brief 1�s���̓_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/05/31 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgInput1;

#ifndef _CDLGINPUT1_H_
#define _CDLGINPUT1_H_

#include <windows.h>
#include "mem/CMemory.h"


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �P�s���̓_�C�A���O�{�b�N�X
*/
class CDlgInput1
{
public:
	/*
	||  Constructors
	*/
	CDlgInput1();
	~CDlgInput1();
	BOOL DoModal( HINSTANCE , HWND , const TCHAR* , const TCHAR* , int , TCHAR*  );	/* ���[�h���X�_�C�A���O�̕\�� */
	BOOL DoModal( HINSTANCE , HWND , const TCHAR* , const TCHAR* , int , NOT_TCHAR*  );	/* ���[�h���X�_�C�A���O�̕\�� */
	/*
	||  Attributes & Operations
	*/
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */

	HINSTANCE	m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND		m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	HWND		m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */

	const TCHAR*	m_pszTitle;		/* �_�C�A���O�^�C�g�� */
	const TCHAR*	m_pszMessage;	/* ���b�Z�[�W */
	int			m_nMaxTextLen;	/* ���̓T�C�Y��� */
//	char*		m_pszText;		/* �e�L�X�g */
	CNativeT	m_cmemText;		/* �e�L�X�g */
	SFilePath	m_szHelpFile;	//@@@ 2002.01.07 add
protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGINPUT1_H_ */


