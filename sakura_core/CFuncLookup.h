//	$Id$
/*!	@file
	@brief �\���p�����񓙂̎擾

	�@�\���C�@�\���ށC�@�\�ԍ��Ȃǂ̕ϊ��D�ݒ��ʂł̕\���p�������p�ӂ���D

	@author genta
	@date Oct. 1, 2001
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

#ifndef __CNAMELOOKUP_H__
#define __CNAMELOOKUP_H__

#include <windows.h>
#include "global.h"
#include "Funccode.h"
#include "CSMacroMgr.h"

/*!
	@brief �\���p�����񓙂̎擾

	�@�\�C�@�\���ނƈʒu�C�@�\�ԍ��C������Ȃǂ̑Ή����W�񂷂�D
*/
class SAKURA_CORE_API CFuncLookup {

public:
	//	Oct. 15, 2001 genta �����ǉ�
	CFuncLookup( HINSTANCE hInst, CSMacroMgr* SMacroMgr, Common* pCom )
		: m_pcSMacroMgr( SMacroMgr ), m_hInstance( hInst ), m_pCommon( pCom ) {}
	CFuncLookup() : m_pcSMacroMgr( NULL ), m_hInstance( NULL ) {}

	void Init( HINSTANCE hInst, CSMacroMgr* SMacroMgr, Common* pCom ){
		m_pcSMacroMgr = SMacroMgr;
		m_hInstance = hInst;
		m_pCommon = pCom;
	}

	int Pos2FuncCode( int category, int position ) const;
	bool Pos2FuncName( int category, int position, char *ptr, int bufsize ) const;
	bool Funccode2Name( int funccode, char *ptr, int bufsize ) const ;
	const char* Category2Name( int category ) const;

	void SetCategory2Combo( HWND hComboBox ) const ;
	void SetListItem( HWND hListBox, int category ) const;
	
	int GetCategoryCount(void) const {
		return nsFuncCode::nFuncKindNum + 2;
	}
	
	int GetItemCount(int category) const;


private:
	HINSTANCE m_hInstance;	//!< �����񃊃\�[�X�����C���X�^���X
	CSMacroMgr* m_pcSMacroMgr;	//!< �}�N���Ǘ��N���X
	
	Common* m_pCommon;	//! ���ʐݒ�f�[�^�̈�ւ̃|�C���^

};

#endif
/* [EOF] */
