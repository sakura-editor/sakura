//	$Id$
/************************************************************************

	CEol.cpp	CEOL�N���X�̎���

	2000/05/15 �V�K�쐬	genta
	Copyright (C) 2000, genta

************************************************************************/
#include "CEol.h"

//-----------------------------------------------
//	�Œ�f�[�^
//-----------------------------------------------
/* �s�I�[�q�̃f�[�^�̔z�� */
const char* CEOL::gm_pszEolDataArr[EOL_TYPE_NUM] = {
	"",
	"\x0d\x0\x0a\x0",	// EOL_CRLF_UNICODE
	"\x0d\x0a",			// EOL_CRLF		
	"\x0a\x0d",			// EOL_LFCR		
	"\x0a",				// EOL_LF			
	"\x0d"				// EOL_CR			
};

/* �s�I�[�q�̃f�[�^���̔z�� */
const int CEOL::gm_pnEolLenArr[EOL_TYPE_NUM] = {
	LEN_EOL_NONE			,	// == 0
	LEN_EOL_CRLF_UNICODE	,	// == 4
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
	"CR0LF0",
	"CRLF",
	"LFCR",
	"LF",
	"CR"
};
//-----------------------------------------------
//	������
//-----------------------------------------------

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

/* �s�I�[�q�̎�ނ𒲂ׂ� */
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
