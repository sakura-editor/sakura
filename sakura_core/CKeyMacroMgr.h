//	$Id$
/************************************************************************

	CKeyMacroMgr.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/
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
