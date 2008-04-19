/*!	@file
	@brief �t�@�C���v���p�e�B�_�C�A���O

	@author Norio Nakatani
	@date 1999/02/31 �V�K�쐬
	@date 1999/12/05 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgProperty;

#ifndef _CDLGPROPERTY_H_
#define _CDLGPROPERTY_H_

#include "dlg/CDialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgProperty : public CDialog
{
public:
	int DoModal( HINSTANCE, HWND, LPARAM  );	/* ���[�_���_�C�A���O�̕\�� */
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnBnClicked( int );
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};
///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPROPERTY_H_ */


