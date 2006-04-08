//	$Id$
/*!	@file
	@brief CEOLクラスの実装

	@author genta
	@date 2000/05/15 新規作成 genta
	$Revision$
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

//-----------------------------------------------
//	固定データ
//-----------------------------------------------
/*! 行終端子のデータの配列 */
const char* CEOL::gm_pszEolDataArr[EOL_TYPE_NUM] = {
	"",
	"\x0d\x0a",			// EOL_CRLF
	"\x0a\x0d",			// EOL_LFCR
	"\x0a",				// EOL_LF
	"\x0d"				// EOL_CR
};

/*! 行終端子のデータの配列(Unicode版) 2000/05/09 Frozen */
const wchar_t* CEOL::gm_pszEolUnicodeDataArr[EOL_TYPE_NUM] = {
	L"",
	L"\x0d\x0a",		// EOL_CRLF
	L"\x0a\x0d",		// EOL_LFCR
	L"\x0a",			// EOL_LF
	L"\x0d"				// EOL_CR
};

/*! 行終端子のデータの配列(UnicodeBE版) 2000.05.30 Moca */
const wchar_t* CEOL::gm_pszEolUnicodeBEDataArr[EOL_TYPE_NUM] = {
	L"",
	(const wchar_t*)"\x00\x0d\x00\x0a\x00",		// EOL_CRLF
	(const wchar_t*)"\x00\x0a\x00\x0d\x00",		// EOL_LFCR
	(const wchar_t*)"\x00\x0a\x00",				// EOL_LF
	(const wchar_t*)"\x00\x0d\x00"				// EOL_CR
};

/*! 行終端子のデータ長の配列 */
const int CEOL::gm_pnEolLenArr[EOL_TYPE_NUM] = {
	LEN_EOL_NONE			,	// == 0
	LEN_EOL_CRLF			,	// == 2
	LEN_EOL_LFCR			,	// == 2
	LEN_EOL_LF				,	// == 1
	LEN_EOL_CR					// == 1
//	LEN_EOL_UNKNOWN				// == 0
};

/* 行終端子の表示名の配列 */
const char* CEOL::gm_pszEolNameArr[EOL_TYPE_NUM] = {
	//	May 12, 2000 genta
	//	文字幅の都合上“無”を漢字に
	"改行無",
	"CRLF",
	"LFCR",
	"LF",
	"CR"
};
//-----------------------------------------------
//	実装部
//-----------------------------------------------
/*!
	行末種別の設定。
	@param t 行末種別
	@retval true 正常終了。設定が反映された。
	@retval false 異常終了。強制的にCRLFに設定。
*/
bool CEOL::SetType( enumEOLType t )
{
	if( t < EOL_NONE || EOL_CR < t ){
		//	異常値
		m_enumEOLType = EOL_CRLF;
		return false;
	}
	//	正しい値
	m_enumEOLType = t;
	return true;
}

/*!
	行終端子の種類を調べる。
	@param pszData 調査対象文字列へのポインタ
	@param nDataLen 調査対象文字列の長さ
	@return 改行コードの種類。終端子が見つからなかったときはEOL_NONEを返す。
*/
enumEOLType CEOL::GetEOLType( const char* pszData, int nDataLen )
{
	int	i;
	/* 改行コードの長さを調べる */
	for( i = 1; i < EOL_TYPE_NUM; ++i ){
		if( gm_pnEolLenArr[i] <= nDataLen
		 && 0 == memcmp( pszData, gm_pszEolDataArr[i], gm_pnEolLenArr[i] )
		){
			return gm_pnEolTypeArr[i];
		}
	}
	return EOL_NONE;
}

/*!
	行端子の種類を調べるUnicode版
	@param pszData 調査対象文字列へのポインタ
	@param nDataLen 調査対象文字列の長さ(wchar_tの長さ)
	@return 改行コードの種類。終端子が見つからなかったときはEOL_NONEを返す。
*/
enumEOLType CEOL::GetEOLTypeUni( const wchar_t* pszData, int nDataLen )
{
	int	i;
	/* 改行コードの長さを調べる */
	for( i = 1; i < EOL_TYPE_NUM; ++i ){
		if( gm_pnEolLenArr[i] <= nDataLen
		 && 0 == memcmp( pszData, gm_pszEolUnicodeDataArr[i], gm_pnEolLenArr[i] * sizeof( wchar_t ) )
		){
			return gm_pnEolTypeArr[i];
		}
	}
	return EOL_NONE;
}

/*
	行端子の種類を調べるUnicodeBE版
	@param pszData 調査対象文字列へのポインタ
	@param nDataLen 調査対象文字列の長さ(wchar_tの長さ)
	@return 改行コードの種類。終端子が見つからなかったときはEOL_NONEを返す。
*/
enumEOLType CEOL::GetEOLTypeUniBE( const wchar_t* pszData, int nDataLen )
{
	int	i;
	/* 改行コードの長さを調べる */
	for( i = 1; i < EOL_TYPE_NUM; ++i ){
		if( gm_pnEolLenArr[i] <= nDataLen
		 && 0 == memcmp( pszData, gm_pszEolUnicodeBEDataArr[i], gm_pnEolLenArr[i] * sizeof( wchar_t ) )
		){
			return gm_pnEolTypeArr[i];
		}
	}
	return EOL_NONE;
}

/*[EOF]*/
