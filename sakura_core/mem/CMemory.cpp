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
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "mem/CMemory.h"

#include <algorithm>
#include <cstdlib>

#include "_main/global.h"
#include "config/app_constants.h"
#include "util/MessageBoxF.h"
#include "debug/Debug2.h"
#include "CSelectLang.h"
#include "String_define.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	@note 格納データにはNULLを含むことができる
*/
CMemory::CMemory(
	const void*	pData,			//!< 格納データアドレス
	size_t		nDataLen		//!< 格納データの有効長
)
{
	SetRawData( pData, nDataLen );
}

/*!
 * @brief コピーコンストラクタ
 */
CMemory::CMemory( const CMemory& rhs )
{
	SetRawData(rhs);
}

/*!
 * @brief ムーブコンストラクタ
 */
CMemory::CMemory( CMemory&& other ) noexcept
{
	swap(other);
}

CMemory::~CMemory() noexcept
{
	Reset();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 等しい内容か */
bool CMemory::IsEqual(const CMemory& cmem1, const CMemory& cmem2)
{
	const auto nLen1 = cmem1.GetRawLength();
	const auto nLen2 = cmem2.GetRawLength();
	if( nLen1 == nLen2 ){
		const auto psz1 = cmem1.GetRawPtr();
		const auto psz2 = cmem2.GetRawPtr();
		if( 0 == memcmp( psz1, psz2, nLen1 ) ){
			return true;
		}
	}
	return false;
}

/* !上位バイトと下位バイトを交換する

	@author Moca
	@date 2002/5/27

	@note nDataLenが2の倍数でないときは、最後の1バイトは交換されない
*/
void CMemory::SwapHLByte( char* pData, const size_t nDataLen ) noexcept
{
	CMemory cmem( pData, nDataLen );
	cmem.SwapHLByte();
	if( cmem.GetRawPtr() != nullptr ){
		::memcpy( pData, cmem.GetRawPtr(), nDataLen );
	}
}

/* !
	データをWORD値の配列とみなしてエンディアンを反転する

	@author Moca
	@date 2002/5/27
	
	@note サイズが2の倍数でないときは、最後の1バイトは交換されない
*/
void CMemory::SwapHLByte( void ) noexcept
{
	auto p0 = reinterpret_cast<uint16_t*>(GetRawPtr());
	const auto p1 = p0 + GetRawLength() / 2;
	std::for_each(p0, p1, []( auto& w ) { w = ::_byteswap_ushort( w ); } );
}

/*!
	バッファサイズの調整

	@date 2007/08/13 kobake 変更
	@date 2014/06/25 有効データ長が0の場合はfree & malloc
 */
void CMemory::AllocBuffer( size_t nNewDataLen )
{
	// 確保サイズは 8byte 単位に切り上げる
	// また、必要サイズは 2byte 多く見積もっておく(終端L'\0'を入れるため)
	size_t nAllocSize = ((nNewDataLen + sizeof(wchar_t)) + 7) & (~7);

	// 既に必要サイズを確保できている場合、直ちに抜ける
	if( nAllocSize < m_nDataBufSize ){
		return;
	}

	// メモリ確保済み、かつ、有効データ長が0の場合は一旦クリアする
	if( m_pRawData != nullptr && m_nRawLen == 0 ){
		Reset();
	}

	void* pAllocated = nullptr;

	if( m_nDataBufSize == 0 && nAllocSize <= INT_MAX ){
		pAllocated = ::malloc( nAllocSize );
	}else if( nAllocSize <= INT_MAX ){
		pAllocated = ::realloc( m_pRawData, nAllocSize );
	}

	if( pAllocated != nullptr ){
		m_pRawData = static_cast<std::byte*>(pAllocated);
		m_nDataBufSize = static_cast<decltype(m_nDataBufSize)>(nAllocSize);
	}else{
		// "CMemory::AllocBuffer(nNewDataLen==%d)\nメモリ確保に失敗しました。\n"
		TopCustomMessage(
			nullptr,
			MB_OKCANCEL | MB_ICONQUESTION,
			LS(STR_ERR_DLGMEM1),
			nNewDataLen
		);

		if( m_pRawData != nullptr && nAllocSize != 0 ){
			Reset();
		}
	}
}

/* バッファの内容を置き換える */
void CMemory::SetRawData( const void* pData, size_t nDataLen )
{
	if( m_pRawData != pData ){
		// バッファをクリアして末尾に追加
		Reset();
		AppendRawData( pData, nDataLen );
	}else if( m_pRawData == nullptr ){
		// インスタンスコピーするとバッファが確保される既存の挙動を変えないために追加
		// これを入れないと取得したCNativeWのNULLチェックを忘れてGetStringPtr()する不作法なコードが落ちます。
		// 論理的には無駄な処理であるため、いつか削除したいです。
		AppendRawData( nullptr, 0 );
	}
}

/* バッファの内容を置き換える */
void CMemory::SetRawData( const CMemory& cmemData )
{
	// バッファの内容を置き換える
	SetRawData( cmemData.GetRawPtr(), cmemData.GetRawLength() );
}

/*! バッファの内容を置き換える */
void CMemory::SetRawDataHoldBuffer( const void* pData, size_t nDataLen )
{
	if( m_pRawData != pData ){
		// 有効データ長をクリアして末尾(先頭)に追加
		if( m_nRawLen != 0 ){
			_SetRawLength( 0 );
		}
		AppendRawData( pData, nDataLen );
	}
}

/*! バッファの内容を置き換える */
void CMemory::SetRawDataHoldBuffer( const CMemory& cmemData )
{
	// バッファの内容を置き換える
	SetRawDataHoldBuffer( cmemData.GetRawPtr(), cmemData.GetRawLength() );
}

/* バッファの最後にデータを追加する（publicメンバ）*/
void CMemory::AppendRawData( const void* pData, size_t nDataLen )
{
	// メモリが足りなければ確保する
	if( m_nDataBufSize <= m_nRawLen + nDataLen + sizeof(wchar_t) ){
		AllocBuffer( m_nRawLen + nDataLen );
	}

	// メモリを確保できた場合のみコピーする
	if( m_pRawData != nullptr && m_nRawLen + nDataLen + sizeof(wchar_t) <= m_nDataBufSize ){
		::memcpy( &m_pRawData[m_nRawLen], pData, nDataLen );
		_SetRawLength( m_nRawLen + nDataLen );
	}
}

void CMemory::Reset( void ) noexcept
{
	::free( m_pRawData );
	m_pRawData = nullptr;
	m_nRawLen = 0;
	m_nDataBufSize = 0;
}

void CMemory::_AppendSz( std::string_view str )
{
	// バッファの最後にデータを追加する
	AppendRawData( str.data(), str.length() );
}

void CMemory::_SetRawLength( size_t nLength )
{
	if( m_pRawData != nullptr && nLength + sizeof(wchar_t) <= m_nDataBufSize ){
		m_nRawLen = static_cast<decltype(m_nRawLen)>(nLength);
		wchar_t chNul = L'\0'; //ワイド文字のNUL終端を付加する
		::memcpy( &m_pRawData[m_nRawLen], &chNul, sizeof(wchar_t) );
	}
}

void CMemory::swap( CMemory& left ) noexcept {
	std::swap(m_nDataBufSize, left.m_nDataBufSize);
	std::swap(m_pRawData, left.m_pRawData);
	std::swap(m_nRawLen, left.m_nRawLen);
}

//! コピー代入演算子
CMemory& CMemory::operator = ( const CMemory& rhs )
{
	if( this != &rhs ){
		SetRawData( rhs );
	}
	return *this;
}

//! ムーブ代入演算子
CMemory& CMemory::operator = ( CMemory&& rhs ) noexcept
{
	if( this != &rhs ){
		Reset();
		swap( rhs );
	}
	return *this;
}
