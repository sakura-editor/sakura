/*!	@file
	@brief �O���R�}���h���s�_�C�A���O

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "dlg/CDialog.h"

#ifndef _CDLGEXEC_H_
#define _CDLGEXEC_H_

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgExec : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgExec();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM );	/* ���[�_���_�C�A���O�̕\�� */

	TCHAR	m_szCommand[1024 + 1];	/* �R�}���h���C�� */
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
//	BOOL	m_bGetStdout;	// �W���o�͂𓾂�


protected:
	/* �I�[�o�[���C�h? */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add


};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGEXEC_H_ */


