/*!	@file
	@brief �L�[�{�[�h�}�N��

	CMacro�̃C���X�^���X�ЂƂ��A1�R�}���h�ɂȂ�B

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, �S

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CMacro;
class CTextOutputStream;

#ifndef _CMACRO_H_
#define _CMACRO_H_

#include <windows.h>
#include <objidl.h>  // VARIANT��
#include "func/Funccode.h"

class CEditView;

/*! @brief �L�[�{�[�h�}�N����1�R�}���h

	���������X�g�\���ɂ��āA�����ł����Ă�悤�ɂ��Ă݂܂����B
	�X�^�b�N�ɂ���̂��ʗ�Ȃ̂�������܂���i�悭�킩��܂���j�B
	
	����A����\���������Ă�����Ȃ��悤�ɂ��悤�Ǝv�����̂ł����A���܂��܂����B
	
	���āA���̃N���X�͎��̂悤�ȑO��œ��삵�Ă���B

	@li �����̃��X�g���Am_pParamTop����̃��X�g�\���ŕێ��B
	@li ������V���ɒǉ�����ɂ́AAddParam()���g�p����B
	  AddParam�ɂǂ�Ȓl���n����Ă��悢�悤�ɏ�������R�g�B
	  �n���ꂽ�l�����l�Ȃ̂��A������ւ̃|�C���^�Ȃ̂��́Am_nFuncID�i�@�\ ID�j�ɂ���āA���̃N���X���Ŕ��ʂ��A��낵����邱�ƁB
	@li �����́ACMacro�����ł͂��ׂĕ�����ŕێ����邱�Ɓi���l97�́A"97"�Ƃ��ĕێ��j�i���܂̂Ƃ���j
*/
class CMacro
{
public:
	/*
	||  Constructors
	*/
	CMacro( EFunctionCode nFuncID );	//	�@�\ID���w�肵�ď�����
	~CMacro();

	void SetNext(CMacro* pNext){ m_pNext = pNext; }
	CMacro* GetNext(){ return m_pNext; }
	// 2007.07.20 genta : flags�ǉ�
	void Exec( CEditView* pcEditView, int flags ) const; //2007.09.30 kobake const�ǉ�
	void Save( HINSTANCE hInstance, CTextOutputStream& out ) const; //2007.09.30 kobake const�ǉ�
	
	void AddLParam( LPARAM lParam, const CEditView* pcEditView  );	//@@@ 2002.2.2 YAZAKI pcEditView���n��
	void AddStringParam( const WCHAR* lParam );
	void AddStringParam( const ACHAR* lParam ){ return AddStringParam(to_wchar(lParam)); }
	void AddIntParam( const int lParam );

	static void HandleCommand( CEditView* pcEditView, const EFunctionCode Index,	const WCHAR* Argument[], const int ArgSize );
	static bool HandleFunction(CEditView *View, int ID, VARIANT *Arguments, int ArgSize, VARIANT &Result);
#if 0
	/*
	||  Attributes & Operations
	*/
	static char* GetFuncInfoByID( HINSTANCE , int , char* , char* );	/* �@�\ID���֐����C�@�\�����{�� */
	static int GetFuncInfoByName( HINSTANCE , const char* , char* );	/* �֐������@�\ID�C�@�\�����{�� */
	static BOOL CanFuncIsKeyMacro( int );	/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */
#endif

protected:
	struct CMacroParam{
		WCHAR*			m_pData;
		CMacroParam*	m_pNext;
	};

	/*
	||  �����w���p�֐�
	*/
	EFunctionCode	m_nFuncID;		//	�@�\ID
	CMacroParam*	m_pParamTop;	//	�p�����[�^
	CMacroParam*	m_pParamBot;
	CMacro*			m_pNext;		//	���̃}�N���ւ̃|�C���^
};



///////////////////////////////////////////////////////////////////////
#endif /* _CMACRO_H_ */



