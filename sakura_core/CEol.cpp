//	$Id$
/*!	@file
	@brief CEOL�N���X�̎���

	@author genta
	@date 2000/05/15 �V�K�쐬 genta
	$Revision$
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

//-----------------------------------------------
//	�Œ�f�[�^
//-----------------------------------------------
/*! �s�I�[�q�̃f�[�^�̔z�� */
const char* CEOL::gm_pszEolDataArr[EOL_TYPE_NUM] = {
	"",
	"\x0d\x0a",			// EOL_CRLF
	"\x0a\x0d",			// EOL_LFCR
	"\x0a",				// EOL_LF
	"\x0d"				// EOL_CR
};

/*! �s�I�[�q�̃f�[�^�̔z��(Unicode��) 2000/05/09 Frozen */
const wchar_t* CEOL::gm_pszEolUnicodeDataArr[EOL_TYPE_NUM] = {
	L"",
	L"\x0d\x0a",		// EOL_CRLF
	L"\x0a\x0d",		// EOL_LFCR
	L"\x0a",			// EOL_LF
	L"\x0d"				// EOL_CR
};

/*! �s�I�[�q�̃f�[�^�̔z��(UnicodeBE��) 2000.05.30 Moca */
const wchar_t* CEOL::gm_pszEolUnicodeBEDataArr[EOL_TYPE_NUM] = {
	L"",
	(const wchar_t*)"\x00\x0d\x00\x0a\x00",		// EOL_CRLF
	(const wchar_t*)"\x00\x0a\x00\x0d\x00",		// EOL_LFCR
	(const wchar_t*)"\x00\x0a\x00",				// EOL_LF
	(const wchar_t*)"\x00\x0d\x00"				// EOL_CR
};

/*! �s�I�[�q�̃f�[�^���̔z�� */
const int CEOL::gm_pnEolLenArr[EOL_TYPE_NUM] = {
	LEN_EOL_NONE			,	// == 0
	LEN_EOL_CRLF			,	// == 2
	LEN_EOL_LFCR			,	// == 2
	LEN_EOL_LF				,	// == 1
	LEN_EOL_CR					// == 1
//	LEN_EOL_UNKNOWN				// == 0
};

/* �s�I�[�q�̕\�����̔z�� */
const char* CEOL::gm_pszEolNameArr[EOL_TYPE_NUM] = {
	//	May 12, 2000 genta
	//	�������̓s����g���h��������
	"���s��",
	"CRLF",
	"LFCR",
	"LF",
	"CR"
};
//-----------------------------------------------
//	������
//-----------------------------------------------
/*!
	�s����ʂ̐ݒ�B
	@param t �s�����
	@retval true ����I���B�ݒ肪���f���ꂽ�B
	@retval false �ُ�I���B�����I��CRLF�ɐݒ�B
*/
bool CEOL::SetType( enumEOLType t )
{
	if( t < EOL_NONE || EOL_CR < t ){
		//	�ُ�l
		m_enumEOLType = EOL_CRLF;
		return false;
	}
	//	�������l
	m_enumEOLType = t;
	return true;
}

/*!
	�s�I�[�q�̎�ނ𒲂ׂ�B
	@param pszData �����Ώە�����ւ̃|�C���^
	@param nDataLen �����Ώە�����̒���
	@return ���s�R�[�h�̎�ށB�I�[�q��������Ȃ������Ƃ���EOL_NONE��Ԃ��B
*/
enumEOLType CEOL::GetEOLType( const char* pszData, int nDataLen )
{
	int	i;
	/* ���s�R�[�h�̒����𒲂ׂ� */
	for( i = 1; i < EOL_TYPE_NUM; ++i ){
		if( gm_pnEolLenArr[i] <= nDataLen
		 && 0 == memcmp( pszData, gm_pszEolDataArr[i], gm_pnEolLenArr[i] )
		){
			return gm_pnEolTypeArr[i];
		}
	}
	return EOL_NONE;
}

/*!
	�s�[�q�̎�ނ𒲂ׂ�Unicode��
	@param pszData �����Ώە�����ւ̃|�C���^
	@param nDataLen �����Ώە�����̒���(wchar_t�̒���)
	@return ���s�R�[�h�̎�ށB�I�[�q��������Ȃ������Ƃ���EOL_NONE��Ԃ��B
*/
enumEOLType CEOL::GetEOLTypeUni( const wchar_t* pszData, int nDataLen )
{
	int	i;
	/* ���s�R�[�h�̒����𒲂ׂ� */
	for( i = 1; i < EOL_TYPE_NUM; ++i ){
		if( gm_pnEolLenArr[i] <= nDataLen
		 && 0 == memcmp( pszData, gm_pszEolUnicodeDataArr[i], gm_pnEolLenArr[i] * sizeof( wchar_t ) )
		){
			return gm_pnEolTypeArr[i];
		}
	}
	return EOL_NONE;
}

/*
	�s�[�q�̎�ނ𒲂ׂ�UnicodeBE��
	@param pszData �����Ώە�����ւ̃|�C���^
	@param nDataLen �����Ώە�����̒���(wchar_t�̒���)
	@return ���s�R�[�h�̎�ށB�I�[�q��������Ȃ������Ƃ���EOL_NONE��Ԃ��B
*/
enumEOLType CEOL::GetEOLTypeUniBE( const wchar_t* pszData, int nDataLen )
{
	int	i;
	/* ���s�R�[�h�̒����𒲂ׂ� */
	for( i = 1; i < EOL_TYPE_NUM; ++i ){
		if( gm_pnEolLenArr[i] <= nDataLen
		 && 0 == memcmp( pszData, gm_pszEolUnicodeBEDataArr[i], gm_pnEolLenArr[i] * sizeof( wchar_t ) )
		){
			return gm_pnEolTypeArr[i];
		}
	}
	return EOL_NONE;
}

/*[EOF]*/
