//	$Id$
/*!	@file
	@brief �L�[�{�[�h�}�N��
	CMacro�̃C���X�^���X�ЂƂ��A1�R�}���h�ɂȂ�B

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CMacro;

#ifndef _CMACRO_H_
#define _CMACRO_H_

#include <windows.h>
#include "CEditView.h"

/*! @brief �L�[�{�[�h�}�N����1�R�}���h

	���������X�g�\���ɂ��āA�����ł����Ă�悤�ɂ��Ă݂܂����B
	�X�^�b�N�ɂ���̂��ʗ�Ȃ̂�������܂���i�悭�킩��܂���j�B
	
	����A����\���������Ă�����Ȃ��悤�ɂ��悤�Ǝv�����̂ł����A���܂��܂����B
	
	���āA���̃N���X�͎��̂悤�ȑO��œ��삵�Ă���B

	@li �����̃��X�g���Am_pParamTop����̃��X�g�\���ŕێ��B
	@li ������V���ɒǉ�����ɂ́AAddParam()���g�p����B
	  AddParam�ɂǂ�Ȓl���n����Ă��悢�悤�ɏ�������R�g�B
	  �n���ꂽ�l�����l�Ȃ̂��A������ւ̃|�C���^�Ȃ̂��́Am_nFuncID�i�@�\ID�j�ɂ���āA���̃N���X���Ŕ��ʂ��A��낵����邱�ƁB
	@li �����́ACMacro�����ł͂��ׂĕ�����ŕێ����邱�Ɓi���l97�́A"97"�Ƃ��ĕێ��j�i���܂̂Ƃ���j
*/
class CMacro
{
public:
	/*
	||  Constructors
	*/
	CMacro( int nFuncID );	//	�@�\ID���w�肵�ď�����
	~CMacro();

	void SetNext(CMacro* pNext){ m_pNext = pNext; };
	CMacro* GetNext(){ return m_pNext;};
	void Exec( CEditView* pcEditView );
	void Save( HINSTANCE hInstance, HFILE hFile );
	
	void AddLParam( LPARAM lParam );
	void AddParam( const char* lParam );
	void AddParam( const int lParam );

	/*
	||  Attributes & Operations
	*/
	static char* GetFuncInfoByID( HINSTANCE , int , char* , char* );	/* �@�\ID���֐����C�@�\�����{�� */
	static int GetFuncInfoByName( HINSTANCE , const char* , char* );	/* �֐������@�\ID�C�@�\�����{�� */
	static BOOL CanFuncIsKeyMacro( int );	/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */


protected:
	/*
	||  �����w���p�֐�
	*/
	int		m_nFuncID;	//	�@�\ID
	struct CMacroParam{
		char* m_pData;
		CMacroParam* m_pNext;
	} *m_pParamTop, *m_pParamBot;			//	�p�����[�^
	CMacro* m_pNext;	//	���̃}�N���ւ̃|�C���^
//	CMacro* m_pPrev;	�O�̃}�N���ɖ߂邱�Ƃ͖����H
};



///////////////////////////////////////////////////////////////////////
#endif /* _CMACRO_H_ */

/*[EOF]*/

