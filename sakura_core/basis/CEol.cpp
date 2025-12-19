/*!	@file
	@brief CEolクラスの実装

	@author genta
	@date 2000/05/15 新規作成 genta
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2000, Frozen, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CEol.h"

//-----------------------------------------------
//	固定データ
//-----------------------------------------------

const SEolDefinition g_aEolTable[] = {
	{ L"改行無",	L"",			"",			0 },
	{ L"CRLF",		L"\x0d\x0a",	"\x0d\x0a",	2 },
	{ L"LF",		L"\x0a",		"\x0a",		1 },
	{ L"CR",		L"\x0d",		"\x0d",		1 },
	{ L"NEL",		L"\x85",		"",			1 },
	{ L"LS",		L"\u2028",		"",			1 },
	{ L"PS",		L"\u2029",		"",			1 },
};

struct SEolDefinitionForUniFile{
	const char*	m_szDataW;
	const char* m_szDataWB;
	size_t		m_nLen;

	bool StartsWithW(const char* pData, size_t nLen) const{ return m_nLen<=nLen && 0==memcmp(pData,m_szDataW,m_nLen); }
	bool StartsWithWB(const char* pData, size_t nLen) const{ return m_nLen<=nLen && 0==memcmp(pData,m_szDataWB,m_nLen); }
};
static const SEolDefinitionForUniFile g_aEolTable_uni_file[] = {
	{ "",					"", 					0 },
	{ "\x0d\x00\x0a\x00",	"\x00\x0d\x00\x0a",		4 },
	{ "\x0a\x00",			"\x00\x0a",				2 },
	{ "\x0d\x00",			"\x00\x0d",				2 },
	{ "\x85\x00",			"\x00\x85",				2 },
	{ "\x28\x20",			"\x20\x28",				2 },
	{ "\x29\x20",			"\x20\x29",				2 },
};

//-----------------------------------------------
//	実装補助
//-----------------------------------------------

/*!
	行終端子の種類を調べる。
	@param pszData 調査対象文字列へのポインタ
	@param nDataLen 調査対象文字列の長さ
	@return 改行コードの種類。終端子が見つからなかったときはEEolType::noneを返す。
*/
template <class T>
EEolType GetEOLType( const T* pszData, size_t nDataLen )
{
	for( size_t i = 1; i < EOL_TYPE_NUM; ++i ){
		if( g_aEolTable[i].StartsWith(pszData, nDataLen) ){
			return static_cast<EEolType>(i);
		}
	}
	return EEolType::none;
}

/*
	ファイルを読み込むときに使用するもの
*/

EEolType _GetEOLType_uni( const char* pszData, size_t nDataLen )
{
	for( size_t i = 1; i < EOL_TYPE_NUM; ++i ){
		if( g_aEolTable_uni_file[i].StartsWithW(pszData, nDataLen) ){
			return static_cast<EEolType>(i);
		}
	}
	return EEolType::none;
}

EEolType _GetEOLType_unibe( const char* pszData, size_t nDataLen )
{
	for( size_t i = 1; i < EOL_TYPE_NUM; ++i ){
		if( g_aEolTable_uni_file[i].StartsWithWB(pszData, nDataLen) ){
			return static_cast<EEolType>(i);
		}
	}
	return EEolType::none;
}

//-----------------------------------------------
//	実装部
//-----------------------------------------------

//! 現在のEOLの名称取得
[[nodiscard]] LPCWSTR CEol::GetName() const noexcept
{
	return g_aEolTable[static_cast<size_t>(m_eEolType)].m_szName;
}

//! 現在のEOL文字列先頭へのポインタを取得
[[nodiscard]] LPCWSTR CEol::GetValue2() const noexcept
{
	return g_aEolTable[static_cast<size_t>(m_eEolType)].m_szDataW;
}

//! 現在のEOL文字列長を取得。文字単位。
[[nodiscard]] CLogicInt	CEol::GetLen() const noexcept
{
	return CLogicInt(g_aEolTable[static_cast<size_t>(m_eEolType)].m_nLen);
}

void CEol::SetTypeByString( const wchar_t* pszData, size_t nDataLen )
{
	SetType( GetEOLType( pszData, nDataLen ) );
}

void CEol::SetTypeByString( const char* pszData, size_t nDataLen )
{
	SetType( GetEOLType( pszData, nDataLen ) );
}

void CEol::SetTypeByStringForFile_uni( const char* pszData, size_t nDataLen )
{
	SetType( _GetEOLType_uni( pszData, nDataLen ) );
}

void CEol::SetTypeByStringForFile_unibe( const char* pszData, size_t nDataLen )
{
	SetType( _GetEOLType_unibe( pszData, nDataLen ) );
}

bool operator == ( const CEol& lhs, const CEol& rhs ) noexcept
{
	return lhs.operator==(static_cast<EEolType>(rhs));
}

bool operator != ( const CEol& lhs, const CEol& rhs ) noexcept
{
	return !(lhs == rhs);
}

bool operator == ( EEolType lhs, const CEol& rhs ) noexcept
{
	return rhs.operator==(lhs);
}

bool operator != ( EEolType lhs, const CEol& rhs ) noexcept
{
	return !(lhs == rhs);
}
