//	$Id$
/************************************************************************

	CMacro.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CMacro;

#ifndef _CMACRO_H_
#define _CMACRO_H_

#include <windows.h>
#include "debug.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CMacro
{
public:
	/*
	||  Constructors
	*/
	CMacro();
	~CMacro();

	/*
	||  Attributes & Operations
	*/
	static char* CMacro::GetFuncInfoByID( HINSTANCE , int , char* , char* );	/* �@�\ID���֐����C�@�\�����{�� */
	static int GetFuncInfoByName( HINSTANCE , const char* , char* );	/* �֐������@�\ID�C�@�\�����{�� */
	static BOOL CanFuncIsKeyMacro( int );	/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */
	

protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CMACRO_H_ */

/*[EOF]*/

