#pragma once

#include "CCodeBase.h"

class CUnicodeBe : public CCodeBase{
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst){ *pDst->_GetMemory()=*pSrc; return UnicodeBEToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst){ *pDst=*pSrc->_GetMemory(); return UnicodeToUnicodeBE(pDst); }	//!< UNICODE    → 特定コード 変換

public:
	//実装
	static EConvertResult UnicodeBEToUnicode(CMemory* pMem);	// UnicodeBE → Unicodeコード変換 //2007.08.13 kobake 追加
	static EConvertResult UnicodeToUnicodeBE(CMemory* pMem);	// Unicode   → UnicodeBEコード変換
};
