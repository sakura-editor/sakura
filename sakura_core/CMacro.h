//	$Id$
/*!	@file
	�L�[�{�[�h�}�N��

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

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

