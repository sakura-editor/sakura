//	$Id$
/************************************************************************

	CEOL.h
    End of Line種別の管理
	Copyright (C) 2000, genta

    UPDATE:
    CREATE: 2000/05/15  新規作成	genta

************************************************************************/
#ifndef _CEOL_H_
#define _CEOL_H_

#include "global.h"
#include "debug.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
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
		{	SetType( GetEOLType( pszData, nDataLen )); }

	//	読み出し関数
	enumEOLType GetType(void) const { return m_enumEOLType; }	//	現在のTypeを取得
	int GetLen(void) const
		{ return gm_pnEolLenArr[ m_enumEOLType ]; }	//	現在のEOL長を取得
	const char* GetName(void) const
		{ return gm_pszEolNameArr[ m_enumEOLType ]; }	//	現在のEOLの名称取得
	const char* GetValue(void) const
		{ return gm_pszEolDataArr[ m_enumEOLType ]; }	//	現在のEOL文字列先頭へのポインタを取得

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
	enumEOLType		m_enumEOLType;	/* 改行コードの種類 */
	//int			m_nEOLLen;		/* 改行コードの長さ */
};

//	利便性向上のためのOverload
// inline bool operator==(m_enumEOLType t, const CEOL& c ){ return c == t; }
// inline bool operator!=(m_enumEOLType t, const CEOL& c ){ return c != t; }

#endif
