//	$Id$
/*! @file
	End of Line��ʂ̊Ǘ�

	@author genta
	@date 2000/5/15 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 2000-2001, genta

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
