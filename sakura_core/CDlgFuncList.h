//	$Id$
/************************************************************************

	CDlgFuncList.h

    Update: 1998/12/04  �č쐬
    CREATE: 1998/06/23  �V�K�쐬
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CDlgFuncList;

#ifndef _CDLGFUNCLIST_H_
#define _CDLGFUNCLIST_H_

#include <windows.h>
#include "CDialog.h"
#include "CFuncInfoArr.h"
#include "CShareData.h"


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgFuncList : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgFuncList();
	/*
	||  Attributes & Operations
	*/
	HWND DoModeless( HINSTANCE, HWND, LPARAM, CFuncInfoArr*, int, int, int );/* ���[�h���X�_�C�A���O�̕\�� */
	void ChangeView( LPARAM );	/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */

	CFuncInfoArr*	m_pcFuncInfoArr;	/* �֐����z�� */
	int				m_nCurLine;			/* ���ݍs */
	int				m_nSortCol;			/* �\�[�g�����ԍ� */
	int				m_nListType;		/* �ꗗ�̎�� */
	CMemory			m_cmemClipText;		/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
	int				m_bLineNumIsCRLF;	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
protected:
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	BOOL OnNotify( WPARAM, LPARAM );
	BOOL OnSize( WPARAM, LPARAM );
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */

	/*
	||  �����w���p�֐�
	*/
	BOOL OnJump( void );
	void SetTreeCpp( HWND );	/* �c���[�R���g���[���̏������F�@C++���\�b�h�c���[ */
	void SetTreeJava( HWND, BOOL );	/* �c���[�R���g���[���̏������F�@Java���\�b�h�c���[ */
	void SetTreeTxt( HWND );	/* �c���[�R���g���[���̏������F�@�e�L�X�g�g�s�b�N�c���[ */
	int SetTreeTxtNest( HWND, HTREEITEM, int, int, HTREEITEM*, int );
	void GetTreeTextNext( HWND, HTREEITEM, int );

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGFUNCLIST_H_ */

/*[EOF]*/
