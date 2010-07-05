/*!	@file
	@brief �L�[���蓖�ĂɊւ���N���X

	@author Norio Nakatani
	@date 1998/03/25 �V�K�쐬
	@date 1998/05/16 �N���X���Ƀf�[�^�������Ȃ��悤�ɕύX
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
class CMemory;// 2002/2/10 aroka

#include "func/Funccode.h"

//! �L�[����ێ�����
struct KEYDATA {
	/*! �L�[�R�[�h	*/
	short			m_nKeyCode;
	
	/*!	�L�[�̖��O	*/
	TCHAR			m_szKeyName[64];
	
	/*!	�Ή�����@�\�ԍ�

		SHIFT, CTRL, ALT�̂R�̃V�t�g��Ԃ̂��ꂼ��ɑ΂���
		�@�\�����蓖�Ă邽�߁A�z��ɂȂ��Ă���B
	*/
	EFunctionCode	m_nFuncCodeArr[8];
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
	static EFunctionCode GetFuncCode( WORD nAccelCmd, int nKeyNameArrNum, KEYDATA* pKeyNameArr, BOOL bGetDefFuncCode = TRUE );
	static EFunctionCode GetFuncCodeAt( KEYDATA& KeyData, int nState, BOOL bGetDefFuncCode = TRUE );	/* ����̃L�[��񂩂�@�\�R�[�h���擾���� */	// 2007.02.24 ryoji
	static EFunctionCode GetDefFuncCode( int nKeyCode, int nState );	/* �L�[�̃f�t�H���g�@�\���擾���� */	// 2007.02.22 ryoji

	//! �L�[���蓖�Ĉꗗ���쐬����
	static int CreateKeyBindList( HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, CNativeW& cMemList, CFuncLookup* pcFuncLookup, BOOL bGetDefFuncCode = TRUE );
	static int GetKeyStr( HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, CNativeT& cMemList, int nFuncId, BOOL bGetDefFuncCode = TRUE );	/* �@�\�ɑΉ�����L�[���̎擾 */
	static int GetKeyStrList( HINSTANCE	hInstance, int nKeyNameArrNum,KEYDATA* pKeyNameArr, CNativeT*** pppcMemList, int nFuncId, BOOL bGetDefFuncCode = TRUE );	/* �@�\�ɑΉ�����L�[���̎擾(����) */
	static TCHAR* GetMenuLabel( HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, int nFuncId, TCHAR* pszLabel, const TCHAR* pszKey, BOOL bKeyStr, BOOL bGetDefFuncCode = TRUE );	/* ���j���[���x���̍쐬 */	// add pszKey	2010/5/17 Uchi

	static TCHAR* MakeMenuLabel(const TCHAR* sName, const TCHAR* sKey);

protected:
	/*
	||  �����w���p�֐�
	*/
	static bool GetKeyStrSub(int& nKeyNameArrBegin, int nKeyNameArrEnd, KEYDATA* pKeyNameArr,
			int nShiftState, CNativeT& cMemList, int nFuncId, BOOL bGetDefFuncCode );
};



///////////////////////////////////////////////////////////////////////
#endif /* _CKEYBIND_H_ */



