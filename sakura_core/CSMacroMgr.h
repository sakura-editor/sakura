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

class CSMacroMgr;

#ifndef _CSMACROMGR_H_
#define _CSMACROMGR_H_

#include <windows.h>
#include <vector>
#include "CMemory.h"
#include "CShareData.h"
#include "CKeyMacroMgr.h"

using namespace std;

class CEditView;

//#define MAX_STRLEN			70
//#define MAX_SMacroNUM		10000
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CSMacroMgr
{
	//	�f�[�^�̌^�錾
	typedef vector<KeyMacroData> KeyMacroList;

	/*! �}�N���P�̏����i�[���� */
	struct Macro1 {
		KeyMacroList m_mac; //!< �}�N���{��
		vector<string> m_strlist; //!< �⏕������
		bool m_flag; //!< �ǂݍ��ݍς݂��ǂ�����\���t���O
	
		Macro1() : m_flag(false){}
		
		void Reset(void){
			m_flag = false;
			m_strlist.clear();
			m_mac.clear();
		}
		
		bool IsReady(void) const { return m_flag; }
	};
	
public:

	/*
	||  Constructors
	*/
	CSMacroMgr();
	~CSMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void ClearAll( void );	/* �L�[�}�N���̃o�b�t�@���N���A���� */

	/*! �L�[�{�[�h�}�N���̎��s */
	BOOL Exec( HINSTANCE hInstance, CEditView* pViewClass, int idx );
	
	//	�o�^�C���^�[�t�F�[�X
	//  �L���E�����̐؂�ւ�
	// Oct. 4, 2001 deleted by genta
//	void Enable(int idx, bool state){
//		if( 0 <= idx || idx < MAX_CUSTMACRO )
//			m_pShareData->m_MacroTable[idx].Enable( state );
//	}
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
	
	//!	�\�����̐ݒ�
	BOOL SetName(int idx, const char *);
	
	//!	�\�����̎擾
	const char* GetName(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_MacroTable[idx].m_szName : NULL;
	}
	
	//!	�t�@�C�����̐ݒ�
	BOOL SetFile(int idx, const char *);
	
	/*!	@brief �t�@�C�����̎擾
	
		@param idx [in] �}�N���ԍ�
	*/
	const char* GetFile(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_MacroTable[idx].m_szFile : NULL;
	}

protected:
	/*! �L�[�{�[�h�}�N���̓ǂݍ��� */
	BOOL Load( CSMacroMgr::Macro1& mbuf, HINSTANCE hInstance, const char* pszPath );
	
	/*! �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ� */
	int Append( CSMacroMgr::Macro1& mbuf, int nFuncID, LPARAM lParam1 );

private:
	vector<Macro1> m_vMacro;

	DLLSHAREDATA*	m_pShareData;

};



///////////////////////////////////////////////////////////////////////
#endif /* _CSMacroMGR_H_ */


/*[EOF]*/
