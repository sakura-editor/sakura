/*!	@file
	@brief CEol�N���X�̎���

	@author genta
	@date 2000/05/15 �V�K�쐬 genta
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2000, Frozen, Moca

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
#include "stdafx.h"
#include "CEol.h"

/*! �s�I�[�q�̔z�� */
const enumEOLType gm_pnEolTypeArr[EOL_TYPE_NUM] = {
	EOL_NONE			,	// == 0
	EOL_CRLF			,	// == 2
	EOL_LF				,	// == 1
	EOL_CR					// == 1
};


//-----------------------------------------------
//	�Œ�f�[�^
//-----------------------------------------------

struct SEolDefinition{
	const TCHAR*	m_szName;
	const WCHAR*	m_szDataW;
	const ACHAR*	m_szDataA;
	int				m_nLen;

	bool StartsWith(const WCHAR* pData, int nLen) const{ return m_nLen<=nLen && 0==auto_memcmp(pData,m_szDataW,m_nLen); }
	bool StartsWith(const ACHAR* pData, int nLen) const{ return m_nLen<=nLen && 0==auto_memcmp(pData,m_szDataA,m_nLen); }
};
static const SEolDefinition g_aEolTable[] = {
	_T("���s��"),	L"",			"",			0,
	_T("CRLF"),		L"\x0d\x0a",	"\x0d\x0a",	2,
	_T("LF"),		L"\x0a",		"\x0a",		1,
	_T("CR"),		L"\x0d",		"\x0d",		1,
};



struct SEolDefinitionForUniFile{
	const char*	m_szDataW;
	const char* m_szDataWB;
	int			m_nLen;

	bool StartsWithW(const char* pData, int nLen) const{ return m_nLen<=nLen && 0==memcmp(pData,m_szDataW,m_nLen); }
	bool StartsWithWB(const char* pData, int nLen) const{ return m_nLen<=nLen && 0==memcmp(pData,m_szDataWB,m_nLen); }
};
static const SEolDefinitionForUniFile g_aEolTable_uni_file[] = {
	"",					"", 					0,
	"\x0d\x00\x0a\x00",	"\x00\x0d\x00\x0a",		4,
	"\x0a\x00",			"\x00\x0a",				2,
	"\x0d\x00",			"\x00\x0d",				2,
};





//-----------------------------------------------
//	�����⏕
//-----------------------------------------------

/*!
	�s�I�[�q�̎�ނ𒲂ׂ�B
	@param pszData �����Ώە�����ւ̃|�C���^
	@param nDataLen �����Ώە�����̒���
	@return ���s�R�[�h�̎�ށB�I�[�q��������Ȃ������Ƃ���EOL_NONE��Ԃ��B
*/
template <class T>
enumEOLType GetEOLType( const T* pszData, int nDataLen )
{
	for( int i = 1; i < EOL_TYPE_NUM; ++i ){
		if( g_aEolTable[i].StartsWith(pszData, nDataLen) )
			return gm_pnEolTypeArr[i];
	}
	return EOL_NONE;
}


/*
	�t�@�C����ǂݍ��ނƂ��Ɏg�p�������
*/

EEolType _GetEOLType_uni( const char* pszData, int nDataLen )
{
	for( int i = 1; i < EOL_TYPE_NUM; ++i ){
		if( g_aEolTable_uni_file[i].StartsWithW(pszData, nDataLen) )
			return gm_pnEolTypeArr[i];
	}
	return EOL_NONE;
}

EEolType _GetEOLType_unibe( const char* pszData, int nDataLen )
{
	for( int i = 1; i < EOL_TYPE_NUM; ++i ){
		bool bret = g_aEolTable_uni_file[i].StartsWithWB(pszData, nDataLen);
		if( bret == true )
			return gm_pnEolTypeArr[i];
	}
	return EOL_NONE;
}

//-----------------------------------------------
//	������
//-----------------------------------------------


//! ���݂�EOL�����擾�B�����P�ʁB
CLogicInt CEol::GetLen() const
{
	return CLogicInt(g_aEolTable[ m_eEolType ].m_nLen);
}

//! ���݂�EOL�̖��̎擾
const TCHAR* CEol::GetName() const
{
	return g_aEolTable[ m_eEolType ].m_szName;
}

//!< ���݂�EOL������擪�ւ̃|�C���^���擾
const wchar_t* CEol::GetValue2() const
{
	return g_aEolTable[ m_eEolType ].m_szDataW;
}

/*!
	�s����ʂ̐ݒ�B
	@param t �s�����
	@retval true ����I���B�ݒ肪���f���ꂽ�B
	@retval false �ُ�I���B�����I��CRLF�ɐݒ�B
*/
bool CEol::SetType( enumEOLType t )
{
	if( t < EOL_NONE || EOL_CR < t ){
		//	�ُ�l
		m_eEolType = EOL_CRLF;
		return false;
	}
	//	�������l
	m_eEolType = t;
	return true;
}

void CEol::SetTypeByString( const wchar_t* pszData, int nDataLen )
{
	SetType( GetEOLType( pszData, nDataLen ) );
}

void CEol::SetTypeByString( const char* pszData, int nDataLen )
{
	SetType( GetEOLType( pszData, nDataLen ) );
}

void CEol::SetTypeByStringForFile_uni( const char* pszData, int nDataLen )
{
	SetType( _GetEOLType_uni( pszData, nDataLen ) );
}

void CEol::SetTypeByStringForFile_unibe( const char* pszData, int nDataLen )
{
	SetType( _GetEOLType_unibe( pszData, nDataLen ) );
}


