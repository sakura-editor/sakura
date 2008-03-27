/*!	@file
	メモリバッファクラス

	@author Norio Nakatani
	@date 1998/03/06 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, mik, misaka, Stonee, hor
	Copyright (C) 2002, Moca, sui, aroka, genta
	Copyright (C) 2003, genta, Moca, かろと
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
#include "CMemory.h"
#include "CEol.h"// 2002/2/3 aroka
#include "charcode.h"
#include <vector>
#include "charset/CShiftJis.h"
#include <mbstring.h>
#include "global.h"
#include "debug.h"
#include "CRunningTimer.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CMemory::_init_members()
{
	m_nDataBufSize = 0;
	m_pData = NULL;
	m_nDataLen = 0;
}

CMemory::CMemory()
{
	_init_members();
}

/*
	@note 格納データにはNULLを含むことができる
*/
CMemory::CMemory(
	const void*	pData,			//!< 格納データアドレス
	int			nDataLenBytes	//!< 格納データの有効長
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
//                          演算子                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

const CMemory& CMemory::operator = ( const CMemory& rhs )
{
	if( this != &rhs ){
		SetRawData( rhs );
	}
	return *this;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //




/*
|| バッファの最後にデータを追加する（protectメンバ
*/
void CMemory::_AddData( const void* pData, int nDataLen )
{
	if( NULL == m_pData ){
		return;
	}
	memcpy( &m_pData[m_nDataLen], pData, nDataLen );
	m_nDataLen += nDataLen;
	m_pData[m_nDataLen]   = '\0';
	m_pData[m_nDataLen+1] = '\0'; //終端'\0'を2つ付加する('\0''\0'==L'\0')。 2007.08.13 kobake 追加
	return;
}














/* 等しい内容か */
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



















/* !上位バイトと下位バイトを交換する

	@author Moca
	@date 2002/5/27
	
	@note	nBufLen が2の倍数でないときは、最後の1バイトは交換されない
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
	// 高速化のため
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
|| バッファサイズの調整
*/
void CMemory::AllocBuffer( int nNewDataLen )
{
	int		nWorkLen;
	char*	pWork = NULL;

	// 2バイト多くメモリ確保しておく('\0'またはL'\0'を入れるため) 2007.08.13 kobake 変更
	nWorkLen = nNewDataLen + 2;

	if( m_nDataBufSize == 0 ){
		/* 未確保の状態 */
		pWork = malloc_char( nWorkLen );
		m_nDataBufSize = nWorkLen;
	}else{
		/* 現在のバッファサイズより大きくなった場合のみ再確保する */
		if( m_nDataBufSize < nWorkLen ){
			pWork = (char*)realloc( m_pData, nWorkLen );
			m_nDataBufSize = nWorkLen;
		}else{
			return;
		}
	}


	if( NULL == pWork ){
		::MYMESSAGEBOX_A(	NULL, MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST, GSTR_APPNAME_A,
			"CMemory::AllocBuffer(nNewDataLen==%d)\nメモリ確保に失敗しました。\n", nNewDataLen
		);
		if( NULL != m_pData && 0 != nWorkLen ){
			/* 古いバッファを解放して初期化 */
			_Empty();
		}
		return;
	}
	m_pData = pWork;
	return;
}



/* バッファの内容を置き換える */
void CMemory::SetRawData( const void* pData, int nDataLen )
{
	_Empty();
	AllocBuffer( nDataLen );
	_AddData( pData, nDataLen );
	return;
}

/* バッファの内容を置き換える */
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


/* バッファの最後にデータを追加する（publicメンバ）*/
void CMemory::AppendRawData( const void* pData, int nDataLenBytes )
{
	if(nDataLenBytes<=0)return;
	AllocBuffer( m_nDataLen + nDataLenBytes );
	_AddData( pData, nDataLenBytes );
}

/* バッファの最後にデータを追加する（publicメンバ）*/
void CMemory::AppendRawData( const CMemory* pcmemData )
{
	if( this == pcmemData ){
		CMemory cm = *pcmemData;
		AppendRawData(&cm);
	}
	int	nDataLen;
	const void*	pData = pcmemData->GetRawPtr( &nDataLen );
	AllocBuffer( m_nDataLen + nDataLen );
	_AddData( pData, nDataLen );
}

void CMemory::_Empty( void )
{
	if( m_pData != NULL ){
		free( m_pData );
		m_pData = NULL;
	}
	m_nDataBufSize = 0;
	m_pData = NULL;
	m_nDataLen = 0;
	return;
}



void CMemory::_AppendSz(const char* str)
{
	int len=strlen(str);
	AllocBuffer( m_nDataLen + len );
	_AddData(str,len);
}


void CMemory::_SetRawLength(int nLength)
{
	assert(m_nDataLen <= m_nDataBufSize-2);
	m_nDataLen = nLength;
	assert(m_nDataLen <= m_nDataBufSize-2);
	m_pData[m_nDataLen  ]=0;
	m_pData[m_nDataLen+1]=0; //終端'\0'を2つ付加する('\0''\0'==L'\0')。
}
