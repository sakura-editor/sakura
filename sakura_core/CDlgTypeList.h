//	$Id$
/************************************************************************

	CDlgTypeList.h
	Copyright (C) 1998-2000, Norio Nakatani

    Update: 1999/12/05  �č쐬
    CREATE: 1998/12/23  �V�K�쐬
************************************************************************/

class CDlgTypeList;

#ifndef _CDLGTYPELIST_H_
#define _CDLGTYPELIST_H_

const int PROP_TEMPCHANGE_FLAG = 0x10000;

#include "CDialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgTypeList : public CDialog
{
public:
	BOOL DoModal( HINSTANCE, HWND, int* );	/* ���[�_���_�C�A���O�̕\�� */

	int				m_nSettingType;
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnLbnDblclk( int );
	BOOL OnBnClicked( int );
	void SetData();	/* �_�C�A���O�f�[�^�̐ݒ� */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGTYPELIST_H_ */

/*[EOF]*/
