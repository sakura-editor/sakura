//	$Id$
/*!	@file
	�L�[�{�[�h�}�N��(���ڎ��s�p)

	@author genta
	
	@date Sep. 29, 2001
	$Revision$
*/
/*
	Copyright (C) 2001, genta

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

#include "CKeyMacroMgr.h"
#include "CShareData.h"

//using namespace std;

class CEditView;

#define STAND_KEYMACRO	-1	//	�W���}�N���i�ۑ����ł���j
#define SAVED_KEYMACRO	0	//	�ۑ����ꂽ�}�N��

//	�֐�����CSMacroMgr������
struct MacroFuncInfo {
	int  	m_nFuncID;
	char *	m_pszFuncName;
//		char *	m_pszFuncParam;
	VARTYPE	m_varArguments[4]; //�����̌^�̔z��
//		int		m_ArgumentCount; //�����̐�
	VARTYPE	m_varResult; //�߂�l�̌^ VT_EMPTY�Ȃ�procedure�Ƃ������Ƃ�
	char *	m_pszData;
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
		m_pShareData->m_MacroTable[idx].IsEnabled() : false;
	}
	
	//!	�\�����閼�O�̎擾
	const char* GetTitle(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
			( m_pShareData->m_MacroTable[idx].m_szName[0] == '\0' ?
				m_pShareData->m_MacroTable[idx].m_szFile : 
				m_pShareData->m_MacroTable[idx].m_szName)
			: NULL;
	}
	
	//!	�\�����̎擾
	const char* GetName(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_MacroTable[idx].m_szName : NULL;
	}
	
	/*!	@brief �t�@�C�����̎擾
	
		@param idx [in] �}�N���ԍ�
	*/
	const char* GetFile(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_MacroTable[idx].m_szFile : NULL;
	}

	/*! �L�[�{�[�h�}�N���̓ǂݍ��� */
	BOOL Load( int idx, HINSTANCE hInstance, const char* pszPath );
	BOOL Save( int idx, HINSTANCE hInstance, const char* pszPath );
	
	/*! �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ� */
	int Append( int idx, int nFuncID, LPARAM lParam1, CEditView* pcEditView );

	/*
	||  Attributes & Operations
	*/
	static char* GetFuncInfoByID( HINSTANCE , int , char* , char* );	/* �@�\ID���֐����C�@�\�����{�� */
	static int GetFuncInfoByName( HINSTANCE , const char* , char* );	/* �֐������@�\ID�C�@�\�����{�� */
	static BOOL CanFuncIsKeyMacro( int );	/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */
	
	//	Jun. 16, 2002 genta
	static const MacroFuncInfo* GetFuncInfoByID( int );
	
	BOOL IsSaveOk(void);

private:
	DLLSHAREDATA*	m_pShareData;
	CMacroManagerBase** Idx2Ptr(int idx);

public:
	static MacroFuncInfo	m_MacroFuncInfoArr[];
	static MacroFuncInfo	m_MacroFuncInfoNotCommandArr[];
};



///////////////////////////////////////////////////////////////////////
#endif /* _CSMacroMGR_H_ */


/*[EOF]*/
