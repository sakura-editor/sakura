//	$Id$
/*!	@file
	@brief �A�E�g���C����̓_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/06/23 �V�K�쐬
	@date 1998/12/04 �č쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, hor
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgFuncList;

#ifndef _CDLGFUNCLIST_H_
#define _CDLGFUNCLIST_H_

#include <windows.h>
#include "CDialog.h"
//#include "CFuncInfoArr.h" // 2002/2/10 aroka �w�b�_����
//#include "CShareData.h" // 2002/2/10 aroka
class CFuncInfoArr; // 2002/2/10 aroka


//!	�A�E�g���C����̓_�C�A���O�{�b�N�X
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
	/* �u�b�N�}�[�N�ƁA�A�E�g���C����؂�ւ��� */
	void ChangeListType( int nOutLineType );

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
	BOOL OnJump( bool bCheckAutoClose = true );	//	bCheckAutoClose�F�u���̃_�C�A���O�������I�ɕ���v���`�F�b�N���邩�ǂ���
	void SetTreeCpp( HWND );	/* �c���[�R���g���[���̏������FC++���\�b�h�c���[ */
	void SetTreeJava( HWND, BOOL );	/* �c���[�R���g���[���̏������FJava���\�b�h�c���[ */
	void SetTree();					/* �c���[�R���g���[���̏������F�ėp�i */
#if 0
2002.04.01 YAZAKI SetTreeTxt()�ASetTreeTxtNest()�͔p�~�BGetTreeTextNext�͂��Ƃ��Ǝg�p����Ă��Ȃ������B
	void SetTreeTxt( HWND );	/* �c���[�R���g���[���̏������F�e�L�X�g�g�s�b�N�c���[ */
	int SetTreeTxtNest( HWND, HTREEITEM, int, int, HTREEITEM*, int );
	void GetTreeTextNext( HWND, HTREEITEM, int );
#endif

	// 2001.12.03 hor
//	void SetTreeBookMark( HWND );		/* �c���[�R���g���[���̏������F�u�b�N�}�[�N */
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	void Key2Command( WORD );		//	�L�[���쁨�R�}���h�ϊ�

private:
	//	May 18, 2001 genta
	/*!
		@brief �A�E�g���C����͎��

		0: List, 1: Tree
	*/
	int	m_nViewType;

	// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 1/4
	// (������Ȃ̂łǂȂ����C�����肢���܂�)
	bool m_bWaitTreeProcess;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGFUNCLIST_H_ */


/*[EOF]*/
