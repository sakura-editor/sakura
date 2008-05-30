/*!	@file
	�������o�b�t�@�N���X

	@author Norio Nakatani
	@date 1998/03/06 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, mik, misaka, Stonee, hor
	Copyright (C) 2002, Moca, sui, aroka, genta
	Copyright (C) 2003, genta, Moca, �����
	Copyright (C) 2004, Moca
	Copyright (C) 2005, Moca, D.S.Koba

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
#include <stdio.h>
#include <windows.h>
#include <ctype.h>
#include <locale.h>
#include <limits.h>
#include "mem/CMemory.h"
#include "CEol.h"// 2002/2/3 aroka
#include "charset/charcode.h"
#include <vector>
#include "charset/CShiftJis.h"
#include <mbstring.h>
#include "global.h"
#include "debug/debug.h"
#include "debug/CRunningTimer.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CMemory::_init_members()
{
	m_nDataBufSize = 0;
	m_pRawData = NULL;
	m_nRawLen = 0;
}

CMemory::CMemory()
{
	_init_members();
}

/*
	@note �i�[�f�[�^�ɂ�NULL���܂ނ��Ƃ��ł���
*/
CMemory::CMemory(
	const void*	pData,			//!< �i�[�f�[�^�A�h���X
	int			nDataLenBytes	//!< �i�[�f�[�^�̗L����
)
{
	_init_members();
	SetRawData( pData, nDataLenBytes );
}

CMemory::CMemory(const CMemory& rhs)
{
	_init_members();
	SetRawData(rhs);
}


CMemory::~CMemory()
{
	_Empty();
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���Z�q                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

const CMemory& CMemory::operator = ( const CMemory& rhs )
{
	if( this != &rhs ){
		SetRawData( rhs );
	}
	return *this;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //




/*
|| �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����iprotect�����o
*/
void CMemory::_AddData( const void* pData, int nDataLen )
{
	if( NULL == m_pRawData ){
		return;
	}
	memcpy( &m_pRawData[m_nRawLen], pData, nDataLen );
	m_nRawLen += nDataLen;
	m_pRawData[m_nRawLen]   = '\0';
	m_pRawData[m_nRawLen+1] = '\0'; //�I�['\0'��2�t������('\0''\0'==L'\0')�B 2007.08.13 kobake �ǉ�
	return;
}














/* ���������e�� */
int CMemory::IsEqual( CMemory& cmem1, CMemory& cmem2 )
{
	const char*	psz1;
	const char*	psz2;
	int		nLen1;
	int		nLen2;

	psz1 = (const char*)cmem1.GetRawPtr( &nLen1 );
	psz2 = (const char*)cmem2.GetRawPtr( &nLen2 );
	if( nLen1 == nLen2 ){
		if( 0 == memcmp( psz1, psz2, nLen1 ) ){
			return TRUE;
		}
	}
	return FALSE;
}



















/* !��ʃo�C�g�Ɖ��ʃo�C�g����������

	@author Moca
	@date 2002/5/27
	
	@note	nBufLen ��2�̔{���łȂ��Ƃ��́A�Ō��1�o�C�g�͌�������Ȃ�
*/
void CMemory::SwapHLByte( void ){
	unsigned char *p;
	unsigned char ctemp;
	unsigned char *p_end;
	unsigned int *pdwchar;
	unsigned int *pdw_end;
	unsigned char*	pBuf;
	int			nBufLen;

	pBuf = (unsigned char*)GetRawPtr( &nBufLen );

	if( nBufLen < 2){
		return;
	}
	// �������̂���
	if( (size_t)pBuf % 2 == 0){
		if( (size_t)pBuf % 4 == 2 ){
			ctemp = pBuf[0];
			pBuf[0]  = pBuf[1];
			pBuf[1]  = ctemp;
			pdwchar = (unsigned int*)(pBuf + 2);
		}else{
			pdwchar = (unsigned int*)pBuf;
		}
		pdw_end = (unsigned int*)(pdwchar + nBufLen / sizeof(int)) - 1;

		for(; pdwchar <= pdw_end ; ++pdwchar ){
			pdwchar[0] = ((pdwchar[0] & (unsigned int)0xff00ff00) >> 8) |
						 ((pdwchar[0] & (unsigned int)0x00ff00ff) << 8);
		}
	}
	p = (unsigned char*)pdwchar;
	p_end = pBuf + nBufLen - 2;
	
	for(; p <= p_end ; p+=2){
		ctemp = p[0];
		p[0]  = p[1];
		p[1]  = ctemp;
	}
	return;
}



/*
|| �o�b�t�@�T�C�Y�̒���
*/
void CMemory::AllocBuffer( int nNewDataLen )
{
	int		nWorkLen;
	char*	pWork = NULL;

	// 2�o�C�g�����������m�ۂ��Ă���('\0'�܂���L'\0'�����邽��) 2007.08.13 kobake �ύX
	nWorkLen = nNewDataLen + 2;

	if( m_nDataBufSize == 0 ){
		/* ���m�ۂ̏�� */
		pWork = malloc_char( nWorkLen );
		m_nDataBufSize = nWorkLen;
	}else{
		/* ���݂̃o�b�t�@�T�C�Y���傫���Ȃ����ꍇ�̂ݍĊm�ۂ��� */
		if( m_nDataBufSize < nWorkLen ){
			pWork = (char*)realloc( m_pRawData, nWorkLen );
			m_nDataBufSize = nWorkLen;
		}else{
			return;
		}
	}


	if( NULL == pWork ){
		::MYMESSAGEBOX(	NULL, MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST, GSTR_APPNAME,
			_T("CMemory::AllocBuffer(nNewDataLen==%d)\n�������m�ۂɎ��s���܂����B\n"), nNewDataLen
		);
		if( NULL != m_pRawData && 0 != nWorkLen ){
			/* �Â��o�b�t�@��������ď����� */
			_Empty();
		}
		return;
	}
	m_pRawData = pWork;
	return;
}



/* �o�b�t�@�̓��e��u�������� */
void CMemory::SetRawData( const void* pData, int nDataLen )
{
	_Empty();
	AllocBuffer( nDataLen );
	_AddData( pData, nDataLen );
	return;
}

/* �o�b�t�@�̓��e��u�������� */
void CMemory::SetRawData( const CMemory& pcmemData )
{
	const void*	pData;
	int		nDataLen;
	pData = pcmemData.GetRawPtr( &nDataLen );
	_Empty();
	AllocBuffer( nDataLen );
	_AddData( pData, nDataLen );
	return;
}


/* �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����ipublic�����o�j*/
void CMemory::AppendRawData( const void* pData, int nDataLenBytes )
{
	if(nDataLenBytes<=0)return;
	AllocBuffer( m_nRawLen + nDataLenBytes );
	_AddData( pData, nDataLenBytes );
}

/* �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����ipublic�����o�j*/
void CMemory::AppendRawData( const CMemory* pcmemData )
{
	if( this == pcmemData ){
		CMemory cm = *pcmemData;
		AppendRawData(&cm);
	}
	int	nDataLen;
	const void*	pData = pcmemData->GetRawPtr( &nDataLen );
	AllocBuffer( m_nRawLen + nDataLen );
	_AddData( pData, nDataLen );
}

void CMemory::_Empty( void )
{
	if( m_pRawData != NULL ){
		free( m_pRawData );
		m_pRawData = NULL;
	}
	m_nDataBufSize = 0;
	m_pRawData = NULL;
	m_nRawLen = 0;
	return;
}



void CMemory::_AppendSz(const char* str)
{
	int len=strlen(str);
	AllocBuffer( m_nRawLen + len );
	_AddData(str,len);
}


void CMemory::_SetRawLength(int nLength)
{
	assert(m_nRawLen <= m_nDataBufSize-2);
	m_nRawLen = nLength;
	assert(m_nRawLen <= m_nDataBufSize-2);
	m_pRawData[m_nRawLen  ]=0;
	m_pRawData[m_nRawLen+1]=0; //�I�['\0'��2�t������('\0''\0'==L'\0')�B
}
