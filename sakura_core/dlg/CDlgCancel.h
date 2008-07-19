/*!	@file
	@brief �L�����Z���{�^���_�C�A���O

	@author Norio Nakatani
	@date 1998/09/09 �쐬
    @date 1999/12/02 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgCancel;

#ifndef _CDLGCANCEL_H_
#define _CDLGCANCEL_H_

#include "dlg/CDialog.h"


/*!
	@brief �L�����Z���{�^���_�C�A���O
*/
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
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� *//* BOOL->INT_PTR 2008/7/18 Uchi*/
	void DeleteAsync( void );	/* �����j����x�����s���� */	// 2008.05.28 ryoji

//	HINSTANCE	m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
//	HWND		m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
//	HWND		m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */
	BOOL		m_bCANCEL;		/* IDCANCEL�{�^���������ꂽ */
	bool		m_bAutoCleanup;	/* �����㏈���^ */	// 2008.05.28 ryoji

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGCANCEL_H_ */



