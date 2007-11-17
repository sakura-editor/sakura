#pragma once

#include "CCodeBase.h"

class CShiftJis : public CCodeBase{
public:

public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst){ *pDst->_GetMemory()=*pSrc; return SJISToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst){ *pDst=*pSrc->_GetMemory(); return UnicodeToSJIS(pDst); }	//!< UNICODE    → 特定コード 変換

public:
	//実装
	static EConvertResult SJISToUnicode(CMemory* pMem);		// SJIS      → Unicodeコード変換
	static EConvertResult UnicodeToSJIS(CMemory* pMem);		// Unicode   → SJISコード変換
	// 2005-09-02 D.S.Koba
	// 2007.08.14 kobake CMemoryからCShiftJisへ移動
	static int GetSizeOfChar( const char* pData, int nDataLen, int nIdx ); //!< 指定した位置の文字が何バイト文字かを返す

protected:
	static int MemSJISToUnicode( char**, const char*, int );	/* ASCII&SJIS文字列をUnicode に変換 */
};
