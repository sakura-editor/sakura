//	$Id$
/*!	@file
	@brief �L�[���蓖�ĂɊւ���N���X

	@author Norio Nakatani
	@date 1998/03/25 �V�K�쐬
	@date 1998/05/16 �N���X���Ƀf�[�^�������Ȃ��悤�ɕύX
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
class CKeyBind;

#ifndef _CKEYBIND_H_
#define _CKEYBIND_H_

#include <windows.h>
//#include "CMemory.h"// 2002/2/10 aroka
//#include "keycode.h"// 2002/2/10 aroka
class CMemory;// 2002/2/10 aroka

//! �L�[����ێ�����
struct KEYDATA {
	/*! �L�[�R�[�h	*/
	short			m_nKeyCode;
//	char*			m_pszKeyName;
	
	/*!	�L�[�̖��O	*/
	char			m_szKeyName[64];
	
	/*!	�Ή�����@�\�ԍ�

		SHIFT, CTRL, ALT�̂R�̃V�t�g��Ԃ̂��ꂼ��ɑ΂���
		�@�\�����蓖�Ă邽�߁A�z��ɂȂ��Ă���B
	*/
	/*short*/int	m_nFuncCodeArr[8];
};

class CFuncLookup;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �L�[���蓖�Ċ֘A���[�`��
	
	���ׂĂ̊֐���static�ŕێ�����f�[�^�͂Ȃ��B
*/
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
	//! �L�[���蓖�Ĉꗗ���쐬����
	static int CreateKeyBindList( HINSTANCE, int, KEYDATA*, CMemory&, CFuncLookup* );
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
