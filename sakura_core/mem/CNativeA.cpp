/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CNativeA.h"
#include "debug/Debug1.h"

CNativeA::CNativeA( const char* szData, size_t cchData )
{
	SetString(szData, cchData);
}

CNativeA::CNativeA(const char* szData)
{
	SetString(szData);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ設定インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// バッファの内容を置き換える
void CNativeA::SetString( const char* pszData )
{
	if( pszData != nullptr ){
		std::string_view data( pszData );
		SetString( data.data(), data.length() );
	}else{
		Reset();
	}
}

// バッファの内容を置き換える。nLenは文字単位。
void CNativeA::SetString( const char* pData, size_t nDataLen )
{
	SetRawData( pData, nDataLen );
}

// バッファの内容を置き換える
void CNativeA::SetNativeData( const CNativeA& cNative )
{
	SetRawData( cNative.GetRawPtr(), cNative.GetRawLength() );
}

// バッファの最後にデータを追加する
void CNativeA::AppendString( const char* pszData )
{
	AppendString(pszData, strlen(pszData));
}

//! バッファの最後にデータを追加する。nLengthは文字単位。
void CNativeA::AppendString( const char* pszData, size_t nLength )
{
	AppendRawData( pszData, nLength );
}

//! バッファの最後にデータを追加する (フォーマット機能付き)
void CNativeA::AppendStringF(const char* pszData, ...)
{
	char buf[2048];

	// 整形
	va_list v;
	va_start(v, pszData);
	int len = _vsnprintf_s(buf, _countof(buf), _TRUNCATE, pszData, v);
	int e = errno;
	va_end(v);

	if (len == -1) {
		DEBUG_TRACE(L"AppendStringF error. errno = %d", e);
		throw std::exception();
	}

	// 追加
	AppendString( buf, len );
}

const CNativeA& CNativeA::operator = ( char cChar )
{
	char pszChar[2];
	pszChar[0] = cChar;
	pszChar[1] = '\0';
	SetRawData( pszChar, 1 );
	return *this;
}

//! バッファの最後にデータを追加する
void CNativeA::AppendNativeData( const CNativeA& cNative )
{
	AppendRawData( cNative.GetRawPtr(), cNative.GetRawLength() );
}

//! (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。
void CNativeA::AllocStringBuffer( size_t nDataLen )
{
	AllocBuffer( nDataLen );
}

const CNativeA& CNativeA::operator += ( char ch )
{
	char szChar[2]={ch,'\0'};
	AppendString(szChar);
	return *this;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ取得インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int CNativeA::GetStringLength() const
{
	return GetRawLength();
}

// 任意位置の文字取得。nIndexは文字単位。
char CNativeA::operator[]( size_t nIndex ) const
{
	if( nIndex < static_cast<size_t>(GetStringLength()) ){
		return GetStringPtr()[nIndex];
	}else{
		return 0;
	}
}
