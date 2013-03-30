/*!	@file
	@brief �w��s�ւ̃W�����v�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/05/31 �쐬
	@date 1999/12/05 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgJump;

#ifndef _CDLGJUMP_H_
#define _CDLGJUMP_H_

#include "dlg/CDialog.h"
//! �w��s�ւ̃W�����v�_�C�A���O�{�b�N�X
class CDlgJump : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgJump();
	/*
	||  Attributes & Operations
	*/
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
	int DoModal( HINSTANCE, HWND, LPARAM/* , BOOL */);	/* ���[�_���_�C�A���O�̕\�� */

	int			m_nLineNum;		/*!< �s�ԍ� */
	BOOL		m_bPLSQL;		/*!< PL/SQL�\�[�X�̗L���s�� */
	int			m_nPLSQL_E1;
	int			m_nPLSQL_E2;
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnNotify( WPARAM,  LPARAM );	//	Oct. 6, 2000 JEPRO added for Spin control
	BOOL OnCbnSelChange( HWND, int );
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGJUMP_H_ */



