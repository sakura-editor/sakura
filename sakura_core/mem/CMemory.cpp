﻿/*!	@file
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
	Copyright (C) 2018-2021, Sakura Editor Organization

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

#include "StdAfx.h"
#include "mem/CMemory.h"
#include "_main/global.h"
#include "CSelectLang.h"
#include "util/MessageBoxF.h"
#include "debug/Debug2.h"
#include "config/app_constants.h"
#include "String_define.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * @brief デフォルトコンストラクタ
 */
CMemory::CMemory() noexcept
	: m_pRawData(NULL)
	, m_nRawLen(0)
	, m_nDataBufSize(0)
{
}

/*
	@note 格納データにはNULLを含むことができる
*/
CMemory::CMemory(
	const void*	pData,			//!< 格納データアドレス
	int			nDataLenBytes	//!< 格納データの有効長
)	: CMemory()
{
	SetRawData( pData, nDataLenBytes );
}

/*!
 * @brief コピーコンストラクタ
 */
CMemory::CMemory(const CMemory& rhs)
	: CMemory()
{
	SetRawData(rhs);
}

/*!
 * @brief ムーブコンストラクタ
 */
CMemory::CMemory(CMemory&& other) noexcept
	: CMemory()
{
	swap(other);
}

CMemory::~CMemory()
{
	_Empty();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
|| バッファの最後にデータを追加する（protectメンバ
*/
void CMemory::_AddData( const void* pData, int nDataLen )
{
	if( NULL == m_pRawData ){
		return;
	}
	memcpy( &m_pRawData[m_nRawLen], pData, nDataLen );
	m_nRawLen += nDataLen;
	m_pRawData[m_nRawLen]   = '\0';
	m_pRawData[m_nRawLen+1] = '\0'; //終端'\0'を2つ付加する('\0''\0'==L'\0')。 2007.08.13 kobake 追加
	return;
}

/* 等しい内容か */
int CMemory::IsEqual(const CMemory& cmem1, const CMemory& cmem2)
{
	const int nLen1 = cmem1.GetRawLength();
	const int nLen2 = cmem2.GetRawLength();
	if( nLen1 == nLen2 ){
		const char* psz1 = reinterpret_cast<const char*>(cmem1.GetRawPtr());
		const char* psz2 = reinterpret_cast<const char*>(cmem2.GetRawPtr());
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
void CMemory::SwapHLByte( char* pData, const int nDataLen ){
	unsigned char *p;
	unsigned char *p_end;
	unsigned int *pdwchar;
	unsigned int *pdw_end;
	unsigned char*	pBuf;
	int			nBufLen;

	//pBuf = (unsigned char*)GetRawPtr( &nBufLen );
	pBuf = reinterpret_cast<unsigned char*>( pData );
	nBufLen = nDataLen;

	if( nBufLen < 2){
		return;
	}
	// 高速化のため
	pdwchar = (unsigned int*)pBuf;
	if( (size_t)pBuf % 2 == 0){
		if( (size_t)pBuf % 4 == 2 ){
			std::swap( pBuf[0], pBuf[1] );
			pdwchar = (unsigned int*)(pBuf + 2);
		}
		pdw_end = (unsigned int*)(pBuf + nBufLen - sizeof(unsigned int));

		for(; pdwchar <= pdw_end ; ++pdwchar ){
			pdwchar[0] = ((pdwchar[0] & (unsigned int)0xff00ff00) >> 8) |
						 ((pdwchar[0] & (unsigned int)0x00ff00ff) << 8);
		}
	}
	p = (unsigned char*)pdwchar;
	p_end = pBuf + nBufLen - 2;
	
	for(; p <= p_end ; p+=2){
		std::swap( p[0], p[1] );
	}
	return;
}

/* !上位バイトと下位バイトを交換する

	@author Moca
	@date 2002/5/27
	
	@note	nBufLen が2の倍数でないときは、最後の1バイトは交換されない
*/
void CMemory::SwapHLByte( void ){
	int nBufLen = GetRawLength();
	char* pBuf = reinterpret_cast<char*>( GetRawPtr() );
	SwapHLByte( pBuf, nBufLen );
	return;
/*
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
*/
}

bool CMemory::SwabHLByte( const CMemory& mem )
{
	if( this == &mem ){
		SwapHLByte();
		return true;
	}
	int nSize = mem.GetRawLength();
	if( m_pRawData && nSize + 2 <= m_nDataBufSize ) {
		// データが短い時はバッファの再利用
		_SetRawLength(0);
	}else{
		_Empty();
	}
	AllocBuffer(nSize);
	char* pSrc = reinterpret_cast<char*>(const_cast<void*>(mem.GetRawPtr()));
	char* pDst = reinterpret_cast<char*>(GetRawPtr());
	if( pDst == NULL ){
		return false;
	}
	_swab(pSrc, pDst, nSize);
	_SetRawLength(nSize);
	return true;
}

/*
|| バッファサイズの調整
*/
void CMemory::AllocBuffer( int nNewDataLen )
{
	int		nWorkLen;
	char*	pWork = NULL;

	// 2バイト多くメモリ確保しておく('\0'またはL'\0'を入れるため) 2007.08.13 kobake 変更
	nWorkLen = ((nNewDataLen + 2) + 7) & (~7); // 8Byteごとに整列

	if( m_nDataBufSize == 0 ){
		/* 未確保の状態 */
		pWork = malloc_char( nWorkLen );
	}else{
		/* 現在のバッファサイズより大きくなった場合のみ再確保する */
		if( m_nDataBufSize < nWorkLen ){
			// 2014.06.25 有効データ長が0の場合はfree & malloc
			if( m_nRawLen == 0 ){
				free( m_pRawData );
				m_pRawData = NULL;
				pWork = malloc_char( nWorkLen );
			}else{
				pWork = (char*)realloc( m_pRawData, nWorkLen );
			}
		}else{
			return;
		}
	}

	if( NULL == pWork ){
		::MYMESSAGEBOX(	NULL, MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST, GSTR_APPNAME,
			LS(STR_ERR_DLGMEM1), nNewDataLen
		);
		if( NULL != m_pRawData && 0 != nWorkLen ){
			/* 古いバッファを解放して初期化 */
			_Empty();
		}
		return;
	}
	m_pRawData = pWork;
	m_nDataBufSize = nWorkLen;
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
	int nDataLen = pcmemData.GetRawLength();
	const void*	pData = pcmemData.GetRawPtr();
	_Empty();
	AllocBuffer( nDataLen );
	_AddData( pData, nDataLen );
	return;
}

/*! バッファの内容を置き換える */
void CMemory::SetRawDataHoldBuffer( const void* pData, int nDataLen )
{
	// this 重複不可
	assert( m_pRawData != pData );
	if( m_nRawLen != 0 ){
		_SetRawLength(0);
	}
	AllocBuffer( nDataLen );
	_AddData( pData, nDataLen );
	return;
}

/*! バッファの内容を置き換える */
void CMemory::SetRawDataHoldBuffer( const CMemory& pcmemData )
{
	if( this == &pcmemData ){
		return;
	}
	int	nDataLen = pcmemData.GetRawLength();
	const void*	pData = pcmemData.GetRawPtr();
	SetRawDataHoldBuffer( pData, nDataLen );
	return;
}

/* バッファの最後にデータを追加する（publicメンバ）*/
void CMemory::AppendRawData( const void* pData, int nDataLenBytes )
{
	if(nDataLenBytes<=0)return;
	AllocBuffer( m_nRawLen + nDataLenBytes );
	_AddData( pData, nDataLenBytes );
}

/* バッファの最後にデータを追加する（publicメンバ）*/
void CMemory::AppendRawData( const CMemory* pcmemData )
{
	if( this == pcmemData ){
		CMemory cm = *pcmemData;
		AppendRawData(&cm);
	}
	int	nDataLen = pcmemData->GetRawLength();
	const void*	pData = pcmemData->GetRawPtr();
	AllocBuffer( m_nRawLen + nDataLen );
	_AddData( pData, nDataLen );
}

void CMemory::_Empty( void )
{
	free( m_pRawData );
	m_pRawData = NULL;
	m_nDataBufSize = 0;
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
	if (m_pRawData == NULL || m_nDataBufSize <= 0)
	{
		// バッファが確保されていない状態の場合、有効データサイズを 0 にする要求しか来ないはず
		assert(nLength == 0);
		return;
	}
	assert(m_nRawLen <= m_nDataBufSize-2);		// m_nRawLen を変更する前に必要な条件が成立しているか確認する
	m_nRawLen = nLength;
	assert(m_nRawLen <= m_nDataBufSize-2);		// m_nRawLen を変更した後も必要な条件が成立しているか確認する
	m_pRawData[m_nRawLen  ]=0;
	m_pRawData[m_nRawLen+1]=0; //終端'\0'を2つ付加する('\0''\0'==L'\0')。
}
