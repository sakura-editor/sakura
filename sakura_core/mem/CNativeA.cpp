/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

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
	// 現在の文字列長を取得
	const auto currentLength = GetStringLength();

	// 可変長引数のポインタを取得
	va_list v;
	va_start(v, pszData);

	// 整形によって追加される文字数をカウント
	const int additional = cxx::_vscprintf(pszData, v);

	if (additional <= 0) return;

	// 現在の文字数 + 追加文字数が収まるようにバッファを拡張する
	const auto newCapacity = currentLength + additional;
	AllocStringBuffer( newCapacity );

	// 出力先を固定長バッファとして扱う
	auto buffer = std::span(&GetStringPtr()[currentLength], additional + 1);

	// 追加処理の実体はCRTに委譲。この関数は無効な書式を与えると即死する。
	const auto added = cxx::_vsprintf_s(buffer, pszData, v);

	int e = 0;
	if (added < 0) e = errno;

	// 可変長引数のポインタを解放
	va_end(v);

	if (added < 0) {
		DEBUG_TRACE(L"AppendStringF error. errno = %d", e);
		throw std::exception();
	}

	if (added <= 0) return;

	// 文字列終端を再設定する
	_SetRawLength(currentLength + added);
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
