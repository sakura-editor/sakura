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

class CKeyMacroMgr;

#ifndef _CKEYMACROMGR_H_
#define _CKEYMACROMGR_H_

#include <windows.h>
#include "CMemory.h"

struct KeyMacroData {
	int		m_nFuncID;
	LPARAM	m_lParam1;
};
#define MAX_STRLEN			70
#define MAX_KEYMACRONUM		10000
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CKeyMacroMgr
{
public:
	/*
	||  Constructors
	*/
	CKeyMacroMgr();
	~CKeyMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void Clear( void );	/* �L�[�}�N���̃o�b�t�@���N���A���� */
	int Append( int , LPARAM );	/* �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ� */
	BOOL SaveKeyMacro( HINSTANCE , HWND , const char* );	/* �L�[�{�[�h�}�N���̕ۑ� */
	BOOL ExecKeyMacro( void* );	/* �L�[�{�[�h�}�N���̎��s */
	BOOL LoadKeyMacro( HINSTANCE, HWND , const char* );	/* �L�[�{�[�h�}�N���̓ǂݍ��� */

//	HINSTANCE		m_hInstance;
	int				m_nKeyMacroDataArrNum;
	KeyMacroData	m_pKeyMacroDataArr[MAX_KEYMACRONUM];
//	CMemory*		m_pKeyMacroDataArr_CMem[MAX_KEYMACRONUM];
//	CMemory			m_cmemKeyMacroDataArr[MAX_KEYMACRONUM];
	char			m_szKeyMacroDataArr[MAX_KEYMACRONUM][MAX_STRLEN];

protected:
	/*
	||  �����w���p�֐�
	*/

};



///////////////////////////////////////////////////////////////////////
#endif /* _CKEYMACROMGR_H_ */


/*[EOF]*/
