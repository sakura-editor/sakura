#pragma once

#include "CCodeBase.h"

class CUnicode : public CCodeBase{
public:
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return UnicodeToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToUnicode(pDst); }	//!< UNICODE    → 特定コード 変換
	void GetBom(CMemory* pcmemBom);	//!< BOMデータ取得
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< 改行データ取得

public:
	//実装
	static EConvertResult UnicodeToUnicode(CMemory* pMem);	// Unicode   → Unicode (何もしない)

public:
	//各種判定関数
	static bool IsUtf16SurrogHi( const uchar16_t );  // UTF16 文字長4 の場合の 0-1バイト目チェック
	static bool IsUtf16SurrogLow( const uchar16_t ); // UTF16 文字長4 の場合の 2-3バイト目チェック
};

inline bool CUnicode::IsUtf16SurrogHi( const uchar16_t wc )
{
//	return ( 0xd800 <= wc && wc <= 0xdbff );
	return ( (wc & 0xfc00) == 0xd800 );
}
inline bool CUnicode::IsUtf16SurrogLow( const uchar16_t wc )
{
//	return ( 0xdc00 <= wc && wc <= 0xdfff );
	return ( (wc & 0xfc00) == 0xdc00 );
}

