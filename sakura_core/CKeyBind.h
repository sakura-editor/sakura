//	$Id$
/************************************************************************

	CKeyBind.h

    �L�[���蓖�ĂɊւ���N���X
	for Windows
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/
class CKeyBind;

#ifndef _CKEYBIND_H_
#define _CKEYBIND_H_

#include <windows.h>
#include "CMemory.h"
#include "keycode.h"


struct KEYDATA {
	short			m_nKeyCode;
//	char*			m_pszKeyName;
	char			m_szKeyName[64];	
	/*short*/int	m_nFuncCodeArr[8];
};


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CKeyBind
{
public:
	/*
	||  Constructors
	*/
	CKeyBind();
	~CKeyBind();

	/*
	||  �Q�ƌn�����o�֐�
	*/
	static HACCEL CreateAccerelator( int, KEYDATA* );
	static int GetFuncCode( WORD, int, KEYDATA* );
	static int CreateKeyBindList( HINSTANCE, int, KEYDATA*, CMemory& );
	static int GetKeyStr( HINSTANCE, int, KEYDATA*, CMemory&, int );	/* �@�\�ɑΉ�����L�[���̎擾 */
	static int CKeyBind::GetKeyStrList( HINSTANCE, int, KEYDATA*, CMemory***, int );	/* �@�\�ɑΉ�����L�[���̎擾(����) */
	static char* GetMenuLabel( HINSTANCE, int, KEYDATA*, int, char*, BOOL );	/* ���j���[���x���̍쐬 */

	/*
	||  �X�V�n�����o�֐�
	*/


protected:
	/*
	||  �����o�ϐ�
	*/
//	HINSTANCE	m_hInstance;
//	CKeyData*	m_pKeyNameArr;
//	int			m_nKeyNameArrNum;

	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CKEYBIND_H_ */

/*[EOF]*/
