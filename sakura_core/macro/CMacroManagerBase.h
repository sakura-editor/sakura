/*!	@file
	@brief �}�N���G���W��

	@author genta
	@date 2002.4.29
*/
/*
	Copyright (C) 2002, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef __CMACROMGR_BASE_H_
#define __CMACROMGR_BASE_H_

#include <Windows.h>
class CEditView;
/*!
	@brief �}�N������������G���W�������̊��N���X

*/
class CMacroManagerBase {
public:

	/*! �L�[�{�[�h�}�N���̎��s
	
		@param[in] pcEditView �}�N�����s�Ώۂ̕ҏW�E�B���h�E
		@param[in] flags �}�N�����s�����D
		
		@date 2007.07.20 genta �}�N�����s������n�����߂�flags��ǉ�
	*/
	virtual void ExecKeyMacro( class CEditView* pcEditView, int flags ) const = 0;
	
	/*! �L�[�{�[�h�}�N�����t�@�C������ǂݍ���

		@param hInstance [in]
		@param pszPath [in] �t�@�C����
	*/
	virtual BOOL LoadKeyMacro( HINSTANCE hInstance, const TCHAR* pszPath) = 0;

	/*! �L�[�{�[�h�}�N���𕶎��񂩂�ǂݍ���

		@param hInstance [in]
		@param pszCode [in] �}�N���R�[�h
	*/
	virtual BOOL LoadKeyMacroStr( HINSTANCE hInstance, const TCHAR* pszCode ) = 0;

	//static CMacroManagerBase* Creator( const char* str );
	//�������z�N���X�͎��̉��ł��Ȃ��̂�Factory�͕s�v�B
	//�p����N���X�ł͕K�v�B
	
	//	�f�X�g���N�^��virtual��Y�ꂸ��
	virtual ~CMacroManagerBase();
	

protected:
	//!	Load�ς݂��ǂ�����\���t���O true...Load�ς݁Afalse...��Load�B
	bool m_nReady;

public:
	/*!	Load�ς݂��ǂ���

		@retval true Load�ς�
		@retval false ��Load
	*/
	bool IsReady(){ return m_nReady; }

	// Constructor
	CMacroManagerBase();

};

#endif
