//	$Id$
/*! @file
	End of Line種別の管理

	@author genta
	@date 2000/5/15 新規作成
	$Revision$
*/
/*
	Copyright (C) 2000-2001, genta

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
