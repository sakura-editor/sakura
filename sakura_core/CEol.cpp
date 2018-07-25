/*!	@file
	@brief CEolクラスの実装

	@author genta
	@date 2000/05/15 新規作成 genta
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
#include "StdAfx.h"
#include "CEol.h"

/*! 行終端子の配列 */
const EEolType gm_pnEolTypeArr[EOL_TYPE_NUM] = {
	EOL_NONE			,	// == 0
	EOL_CRLF			,	// == 2
	EOL_LF				,	// == 1
	EOL_CR				,	// == 1
	EOL_NEL				,	// == 1
	EOL_LS				,	// == 1
	EOL_PS					// == 1
};


//-----------------------------------------------
//	固定データ
//-----------------------------------------------

struct SEolDefinition{
	const TCHAR*	m_szName;
	const WCHAR*	m_szDataW;
	const ACHAR*	m_szDataA;
	int				m_nLen;

	bool StartsWith(const WCHAR* pData, int nLen) const{ return m_nLen<=nLen && 0==auto_memcmp(pData,m_szDataW,m_nLen); }
	bool StartsWith(const ACHAR* pData, int nLen) const{ return m_nLen<=nLen && m_szDataA[0] != '\0' && 0==auto_memcmp(pData,m_szDataA,m_nLen); }
};
static const SEolDefinition g_aEolTable[] = {
	{ _T("改行無"),	L"",			"",			0 },
	{ _T("CRLF"),	L"\x0d\x0a",	"\x0d\x0a",	2 },
	{ _T("LF"),		L"\x0a",		"\x0a",		1 },
	{ _T("CR"),		L"\x0d",		"\x0d",		1 },
	{ _T("NEL"),	L"\x85",		"",			1 },
	{ _T("LS"),		L"\u2028",		"",			1 },
	{ _T("PS"),		L"\u2029",		"",			1 },
};



struct SEolDefinitionForUniFile{
	const char*	m_szDataW;
	const char* m_szDataWB;
	int			m_nLen;

	bool StartsWithW(const char* pData, int nLen) const{ return m_nLen<=nLen && 0==memcmp(pData,m_szDataW,m_nLen); }
	bool StartsWithWB(const char* pData, int nLen) const{ return m_nLen<=nLen && 0==memcmp(pData,m_szDataWB,m_nLen); }
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
	@return 改行コードの種類。終端子が見つからなかったときはEOL_NONEを返す。
*/
template <class T>
EEolType GetEOLType( const T* pszData, int nDataLen )
{
	for( int i = 1; i < EOL_TYPE_NUM; ++i ){
		if( g_aEolTable[i].StartsWith(pszData, nDataLen) )
			return gm_pnEolTypeArr[i];
	}
	return EOL_NONE;
}


/*
	ファイルを読み込むときに使用するもの
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
		if( g_aEolTable_uni_file[i].StartsWithWB(pszData, nDataLen) )
			return gm_pnEolTypeArr[i];
	}
	return EOL_NONE;
}

//-----------------------------------------------
//	実装部
//-----------------------------------------------


//! 現在のEOL長を取得。文字単位。
CLogicInt CEol::GetLen() const
{
	return CLogicInt(g_aEolTable[ m_eEolType ].m_nLen);
}

//! 現在のEOLの名称取得
const TCHAR* CEol::GetName() const
{
	return g_aEolTable[ m_eEolType ].m_szName;
}

//!< 現在のEOL文字列先頭へのポインタを取得
const wchar_t* CEol::GetValue2() const
{
	return g_aEolTable[ m_eEolType ].m_szDataW;
}

/*!
	行末種別の設定。
	@param t 行末種別
	@retval true 正常終了。設定が反映された。
	@retval false 異常終了。強制的にCRLFに設定。
*/
bool CEol::SetType( EEolType t )
{
	if( t < EOL_NONE || EOL_CODEMAX <= t ){
		//	異常値
		m_eEolType = EOL_CRLF;
		return false;
	}
	//	正しい値
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


