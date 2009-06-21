/*!	@file
	@brief �L�[�{�[�h�}�N��(���ڎ��s�p)

	@author genta
	@date Sep. 29, 2001 �쐬
*/
/*
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2005, FILE
	Copyright (C) 2007, ryoji

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

#ifndef _CSMACROMGR_H_
#define _CSMACROMGR_H_

#include <windows.h>
#include <wtypes.h> //VARTYPE

#include "CKeyMacroMgr.h"
#include "env/CShareData.h"

//using namespace std;

class CEditView;

#define TEMP_KEYMACRO	-2	//	�ꎞ�}�N���i�ۑ����ł��Ȃ��j
#define STAND_KEYMACRO	-1	//	�W���}�N���i�ۑ����ł���j
#define SAVED_KEYMACRO	0	//	�ۑ����ꂽ�}�N��

const int INVALID_MACRO_IDX	= -2;	//!< �����ȃ}�N���̃C���f�b�N�X�ԍ� @date Sep. 15, 2005 FILE

//	�֐�����CSMacroMgr������
struct MacroFuncInfo {
	EFunctionCode	m_nFuncID;
	const WCHAR*	m_pszFuncName;
	VARTYPE			m_varArguments[4];	//!< �����̌^�̔z��
	VARTYPE			m_varResult;		//!< �߂�l�̌^ VT_EMPTY�Ȃ�procedure�Ƃ������Ƃ�
	wchar_t*		m_pszData;
};

/*-----------------------------------------------------------------------
�N���X�̐錾

@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
-----------------------------------------------------------------------*/
class CSMacroMgr
{
	//	�f�[�^�̌^�錾
	CMacroManagerBase* m_cSavedKeyMacro[MAX_CUSTMACRO];	//	�L�[�}�N�����J�X�^�����j���[�̐������Ǘ�
	//	Jun. 16, 2002 genta
	//	�L�[�}�N���ɕW���}�N���ȊO�̃}�N����ǂݍ��߂�悤��
	CMacroManagerBase* m_pKeyMacro;	//	�W���́i�ۑ����ł���j�L�[�}�N�����Ǘ�

	//�@�ꎞ�}�N���i���O���w�肵�ă}�N�����s�j���Ǘ�
	CMacroManagerBase* m_pTempMacro;

public:

	/*
	||  Constructors
	*/
	CSMacroMgr();
	~CSMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void Clear( int idx );
	void ClearAll( void );	/* �L�[�}�N���̃o�b�t�@���N���A���� */

	/*! �L�[�{�[�h�}�N���̎��s */
	BOOL Exec( int idx, HINSTANCE hInstance, CEditView* pcEditView );
	
	//!	���s�\���HCShareData�ɖ₢���킹
	bool IsEnabled(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].IsEnabled() : false;
	}
	
	//!	�\�����閼�O�̎擾
	const TCHAR* GetTitle(int idx) const
	{
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].GetTitle() : NULL;	// 2007.11.02 ryoji
	}
	
	//!	�\�����̎擾
	const TCHAR* GetName(int idx) const
	{
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].m_szName : NULL;
	}
	
	/*!	@brief �t�@�C�����̎擾
	
		@param idx [in] �}�N���ԍ�
	*/
	const TCHAR* GetFile(int idx) const
	{
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].m_szFile : NULL;
	}

	/*! �L�[�{�[�h�}�N���̓ǂݍ��� */
	BOOL Load( int idx, HINSTANCE hInstance, const TCHAR* pszPath, const TCHAR* pszType );
	BOOL Save( int idx, HINSTANCE hInstance, const TCHAR* pszPath );
	void UnloadAll(void);

	/*! �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ� */
	int Append( int idx, EFunctionCode nFuncID, LPARAM lParam1, CEditView* pcEditView );

	/*
	||  Attributes & Operations
	*/
	static WCHAR* GetFuncInfoByID( HINSTANCE , int , WCHAR* , WCHAR* );	/* �@�\ID���֐����C�@�\�����{�� */
	static EFunctionCode GetFuncInfoByName( HINSTANCE , const WCHAR* , WCHAR* );	/* �֐������@�\ID�C�@�\�����{�� */
	static BOOL CanFuncIsKeyMacro( int );	/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */
	
	//	Jun. 16, 2002 genta
	static const MacroFuncInfo* GetFuncInfoByID( int );
	
	bool IsSaveOk(void);

	//	Sep. 15, 2005 FILE	���s���}�N���̃C���f�b�N�X�ԍ����� (INVALID_MACRO_IDX:����)
	int GetCurrentIdx( void ) const {
		return m_CurrentIdx;
	}
	void SetCurrentIdx( int idx ) {
		m_CurrentIdx = idx;
	}

	//  Oct. 22, 2008 syat �ꎞ�}�N������
	CMacroManagerBase* SetTempMacro( CMacroManagerBase *newMacro );

private:
	DLLSHAREDATA*	m_pShareData;
	CMacroManagerBase** Idx2Ptr(int idx);

	/*!	���s���}�N���̃C���f�b�N�X�ԍ� (INVALID_MACRO_IDX:����)
		@date Sep. 15, 2005 FILE
	*/
	int m_CurrentIdx;

public:
	static MacroFuncInfo	m_MacroFuncInfoArr[];
	static MacroFuncInfo	m_MacroFuncInfoNotCommandArr[];
};



///////////////////////////////////////////////////////////////////////
#endif /* _CSMacroMGR_H_ */



