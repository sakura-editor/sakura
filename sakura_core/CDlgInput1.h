//	$Id$
/************************************************************************

	CDlgInput1.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CDlgInput1;

#ifndef _CDLGINPUT1_H_
#define _CDLGINPUT1_H_

#include <windows.h>
#include "CMemory.h"


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgInput1
{
public:
	/*
	||  Constructors
	*/
	CDlgInput1();
	~CDlgInput1();
	BOOL DoModal( HINSTANCE , HWND , const char* , const char* , int , char*  );	/* ���[�h���X�_�C�A���O�̕\�� */
	/*
	||  Attributes & Operations
	*/
	BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */

	HINSTANCE	m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND		m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	HWND		m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */

	const char*	m_pszTitle;		/* �_�C�A���O�^�C�g�� */
	const char*	m_pszMessage;	/* ���b�Z�[�W */
	int			m_nMaxTextLen;	/* ���̓T�C�Y��� */
//	char*		m_pszText;		/* �e�L�X�g */
	CMemory		m_cmemText;		/* �e�L�X�g */
protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGINPUT1_H_ */

/*[EOF]*/
