//	$Id$
/*! @file
	End of Line種別の管理

	@author genta
	@date 2000/5/15 新規作成
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

#ifndef _CEOL_H_
#define _CEOL_H_

#include "global.h"
#include "debug.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief End Of Line すなわち行末の改行コードを管理する。

	管理とは言ってもオブジェクト化することで安全に設定を行えたり関連情報の取得を
	オブジェクトに対するメソッドで行えるだけだが、グローバル変数への参照を
	クラス内部に閉じこめることができるのでそれなりに意味はあると思う。
*/
class SAKURA_CORE_API CEOL
{
	static const char* gm_pszEolDataArr[EOL_TYPE_NUM];
	static const int gm_pnEolLenArr[EOL_TYPE_NUM];
	static const char* gm_pszEolNameArr[EOL_TYPE_NUM];
public:

	//	設定関数
	void Init(void){
		m_enumEOLType = EOL_NONE;
	//	m_nEOLLen = 2; /* = CR+LF */
	}

	static enumEOLType GetEOLType( const char* pszData, int nDataLen );
	bool SetType( enumEOLType t);	//	Typeの設定
	void GetTypeFromString( const char* pszData, int nDataLen )
		{	SetType( GetEOLType( pszData, nDataLen ) ); }

	//	読み出し関数
	enumEOLType GetType(void) const { return m_enumEOLType; }	//!<	現在のTypeを取得
	int GetLen(void) const
		{ return gm_pnEolLenArr[ m_enumEOLType ]; }	//!<	現在のEOL長を取得
	const char* GetName(void) const
		{ return gm_pszEolNameArr[ m_enumEOLType ]; }	//!<	現在のEOLの名称取得
	const char* GetValue(void) const
		{ return gm_pszEolDataArr[ m_enumEOLType ]; }	//!<	現在のEOL文字列先頭へのポインタを取得

	//	利便性向上のためのOverload
	bool operator==( enumEOLType t ) const { return GetType() == t; }
	bool operator!=( enumEOLType t ) const { return GetType() != t; }
	const CEOL& operator=( const CEOL& t )
		{ m_enumEOLType = t.m_enumEOLType; return *this; }
	operator enumEOLType(void) const { return GetType(); }

	//	constructor
	CEOL(){ Init(); }
	CEOL( enumEOLType t ){ SetType(t); }

private:
	enumEOLType		m_enumEOLType;	//!< 改行コードの種類
	//int			m_nEOLLen;		/* 改行コードの長さ */
};

//	利便性向上のためのOverload
// inline bool operator==(m_enumEOLType t, const CEOL& c ){ return c == t; }
// inline bool operator!=(m_enumEOLType t, const CEOL& c ){ return c != t; }

#endif


/*[EOF]*/
