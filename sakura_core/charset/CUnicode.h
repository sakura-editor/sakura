#pragma once

#include "CCodeBase.h"

class CUnicode : public CCodeBase{
public:
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst){ *pDst->_GetMemory()=*pSrc; return UnicodeToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst){ *pDst=*pSrc->_GetMemory(); return UnicodeToUnicode(pDst); }	//!< UNICODE    → 特定コード 変換

public:
	//実装
	static EConvertResult UnicodeToUnicode(CMemory* pMem);	// Unicode   → Unicode (何もしない)
};

