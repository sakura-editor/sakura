/*!	@file
	@brief �\���p�����񓙂̎擾

	�@�\���C�@�\���ށC�@�\�ԍ��Ȃǂ̕ϊ��D�ݒ��ʂł̕\���p�������p�ӂ���D

	@author genta
	@date Oct. 1, 2001
*/
/*
	Copyright (C) 2001, genta
	Copyright (C) 2002, aroka
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

#ifndef __CNAMELOOKUP_H__
#define __CNAMELOOKUP_H__

#include <windows.h>
#include "global.h"
#include "func/Funccode.h"
struct MacroRec;// 2007.11.02 ryoji
struct CommonSetting;// 2002/2/10 aroka

//! �}�N�����
struct MacroRec {
	TCHAR	m_szName[MACRONAME_MAX];	//!< �\����
	TCHAR	m_szFile[_MAX_PATH+1];	//!< �t�@�C����(�f�B���N�g�����܂܂Ȃ�)
	bool	m_bReloadWhenExecute;	//	���s���ɓǂݍ��݂Ȃ������i�f�t�H���gon�j
	
	bool IsEnabled() const { return m_szFile[0] != _T('\0'); }
	const TCHAR* GetTitle() const { return m_szName[0] == _T('\0') ? m_szFile: m_szName; }	// 2007.11.02 ryoji �ǉ�
};

/*!
	@brief �\���p�����񓙂̎擾

	�@�\�C�@�\���ނƈʒu�C�@�\�ԍ��C������Ȃǂ̑Ή����W�񂷂�D
*/
class SAKURA_CORE_API CFuncLookup {

public:
	//	Oct. 15, 2001 genta �����ǉ�
	// 2007.11.02 ryoji �����ύX�iCSMacroMgr->MacroRec�j
//	CFuncLookup( HINSTANCE hInst, MacroRec* pMacroRec, CommonSetting* pCom )
//		: m_pMacroRec( pMacroRec ), m_pCommon( pCom ) {}
	CFuncLookup() : m_pMacroRec( NULL ){}

	void Init( MacroRec* pMacroRec, CommonSetting* pCom ){
		m_pMacroRec = pMacroRec;
		m_pCommon = pCom;
	}

	EFunctionCode Pos2FuncCode( int category, int position, bool bGetUnavailable = true ) const;	// 2007.10.31 ryoji bGetUnavailable�p�����[�^�ǉ�
	bool Pos2FuncName( int category, int position, WCHAR* ptr, int bufsize ) const;
	bool Funccode2Name( int funccode, WCHAR* ptr, int bufsize ) const ;
	const TCHAR* Category2Name( int category ) const;

	void SetCategory2Combo( HWND hComboBox ) const ;
	void SetListItem( HWND hListBox, int category ) const;
	
	int GetCategoryCount(void) const {
		return nsFuncCode::nFuncKindNum + 3;	//���ށ{�O���}�N���{�J�X�^�����j���[�{�v���O�C��
	}
	
	int GetItemCount(int category) const;


private:
	MacroRec* m_pMacroRec;	//!< �}�N�����	// 2007.11.02 ryoji �����o�ύX�iCSMacroMgr->MacroRec�j
	
	CommonSetting* m_pCommon;	//! ���ʐݒ�f�[�^�̈�ւ̃|�C���^

};

#endif
/* [EOF] */
