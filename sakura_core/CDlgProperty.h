//	$Id$
/************************************************************************

	CDlgProperty.h
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE: 1999.12/05  �č쐬
    CREATE: 1999.02/31  �V�K�쐬
************************************************************************/

class CDlgProperty;

#ifndef _CDLGPROPERTY_H_
#define _CDLGPROPERTY_H_

#include "CDialog.h"
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
};
///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPROPERTY_H_ */

/*[EOF]*/
