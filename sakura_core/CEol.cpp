//	$Id$
/*!	@file
	CEOLクラスの実装

	@author genta
	@date 2000/05/15 新規作成 genta
	$Revision$
*/
/*
	Copyright (C) 2000-2001, genta

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
#include "CEol.h"

//-----------------------------------------------
//	固定データ
//-----------------------------------------------
/*! 行終端子のデータの配列 */
const char* CEOL::gm_pszEolDataArr[EOL_TYPE_NUM] = {
	"",
	"\x0d\x0\x0a\x0",	// EOL_CRLF_UNICODE
	"\x0d\x0a",			// EOL_CRLF
	"\x0a\x0d",			// EOL_LFCR
	"\x0a",				// EOL_LF
	"\x0d"				// EOL_CR
};

/*! 行終端子のデータ長の配列 */
const int CEOL::gm_pnEolLenArr[EOL_TYPE_NUM] = {
	LEN_EOL_NONE			,	// == 0
	LEN_EOL_CRLF_UNICODE	,	// == 4
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
	"CR0LF0",
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


/*[EOF]*/
