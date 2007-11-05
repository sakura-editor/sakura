#pragma once

#include "CCodeBase.h"

class CUtf8 : public CCodeBase{
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW2* pDst){ return UTF8ToUnicode(pSrc,pDst); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW2* pSrc, CMemory* pDst){ return UnicodeToUTF8(pSrc,pDst); }	//!< UNICODE    → 特定コード 変換


public:
	//実装
	static EConvertResult UTF8ToUnicode(const CMemory* pSrcMem, CNativeW2* pDstMem);		// UTF-8     → Unicodeコード変換 //2007.08.13 kobake 追加
	static EConvertResult UnicodeToUTF8(const CNativeW2* pSrcMem, CMemory* pDstMem);		// Unicode   → UTF-8コード変換
};
