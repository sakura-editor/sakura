//	$Id$
/*! @file
	End of Line��ʂ̊Ǘ�

	@author genta
	@date 2000/5/15 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 2000-2001, genta

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

#ifndef _CEOL_H_
#define _CEOL_H_

#include "global.h"
#include "debug.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief End Of Line ���Ȃ킿�s���̉��s�R�[�h���Ǘ�����B

	�Ǘ��Ƃ͌����Ă��I�u�W�F�N�g�����邱�Ƃň��S�ɐݒ���s������֘A���̎擾��
	�I�u�W�F�N�g�ɑ΂��郁�\�b�h�ōs���邾�������A�O���[�o���ϐ��ւ̎Q�Ƃ�
	�N���X�����ɕ����߂邱�Ƃ��ł���̂ł���Ȃ�ɈӖ��͂���Ǝv���B
*/
class SAKURA_CORE_API CEOL
{
	static const char* gm_pszEolDataArr[EOL_TYPE_NUM];
	static const int gm_pnEolLenArr[EOL_TYPE_NUM];
	static const char* gm_pszEolNameArr[EOL_TYPE_NUM];
public:

	//	�ݒ�֐�
	void Init(void){
		m_enumEOLType = EOL_NONE;
	//	m_nEOLLen = 2; /* = CR+LF */
	}

	static enumEOLType GetEOLType( const char* pszData, int nDataLen );
	bool SetType( enumEOLType t);	//	Type�̐ݒ�
	void GetTypeFromString( const char* pszData, int nDataLen )
		{	SetType( GetEOLType( pszData, nDataLen ) ); }

	//	�ǂݏo���֐�
	enumEOLType GetType(void) const { return m_enumEOLType; }	//!<	���݂�Type���擾
	int GetLen(void) const
		{ return gm_pnEolLenArr[ m_enumEOLType ]; }	//!<	���݂�EOL�����擾
	const char* GetName(void) const
		{ return gm_pszEolNameArr[ m_enumEOLType ]; }	//!<	���݂�EOL�̖��̎擾
	const char* GetValue(void) const
		{ return gm_pszEolDataArr[ m_enumEOLType ]; }	//!<	���݂�EOL������擪�ւ̃|�C���^���擾

	//	���֐�����̂��߂�Overload
	bool operator==( enumEOLType t ) const { return GetType() == t; }
	bool operator!=( enumEOLType t ) const { return GetType() != t; }
	const CEOL& operator=( const CEOL& t )
		{ m_enumEOLType = t.m_enumEOLType; return *this; }
	operator enumEOLType(void) const { return GetType(); }

	//	constructor
	CEOL(){ Init(); }
	CEOL( enumEOLType t ){ SetType(t); }

private:
	enumEOLType		m_enumEOLType;	//!< ���s�R�[�h�̎��
	//int			m_nEOLLen;		/* ���s�R�[�h�̒��� */
};

//	���֐�����̂��߂�Overload
// inline bool operator==(m_enumEOLType t, const CEOL& c ){ return c == t; }
// inline bool operator!=(m_enumEOLType t, const CEOL& c ){ return c != t; }

#endif


/*[EOF]*/
