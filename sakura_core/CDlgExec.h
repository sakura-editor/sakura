//	$Id$
/************************************************************************

	CDlgExec.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/
#include "CDialog.h"

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

	char	m_szCommand[1024 + 1];	/* �R�}���h���C�� */
	BOOL	m_bGetStdout;	// �W���o�͂𓾂�


protected:
	/* �I�[�o�[���C�h? */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	BOOL OnBnClicked( int );


};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGEXEC_H_ */

/*[EOF]*/
