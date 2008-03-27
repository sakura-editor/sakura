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
#include "stdafx.h"
#include "CEol.h"

/*! 行終端子の配列 */
const enumEOLType gm_pnEolTypeArr[EOL_TYPE_NUM] = {
	EOL_NONE			,	// == 0
	EOL_CRLF			,	// == 2
	EOL_LF				,	// == 1
	EOL_CR					// == 1
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
	bool StartsWith(const ACHAR* pData, int nLen) const{ return m_nLen<=nLen && 0==auto_memcmp(pData,m_szDataA,m_nLen); }
};
static const SEolDefinition g_aEolTable[] = {
	_T("改行無"),	L"",			"",			0,
	_T("CRLF"),		L"\x0d\x0a",	"\x0d\x0a",	2,
	_T("LF"),		L"\x0a",		"\x0a",		1,
	_T("CR"),		L"\x0d",		"\x0d",		1,
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
enumEOLType GetEOLType( const T* pszData, int nDataLen )
{
	for( int i = 1; i < EOL_TYPE_NUM; ++i ){
		if( g_aEolTable[i].StartsWith(pszData, nDataLen) )
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
bool CEol::SetType( enumEOLType t )
{
	if( t < EOL_NONE || EOL_CR < t ){
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


/*[EOF]*/
