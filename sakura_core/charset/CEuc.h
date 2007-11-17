#pragma once

#include "CCodeBase.h"

class CEuc : public CCodeBase{
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst){ *pDst->_GetMemory()=*pSrc; return EUCToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst){ *pDst=*pSrc->_GetMemory(); return UnicodeToEUC(pDst); }	//!< UNICODE    → 特定コード 変換

public:
	//実装
	static EConvertResult EUCToUnicode(CMemory* pMem);		// EUC       → Unicodeコード変換  //2007.08.13 kobake 追加
	static EConvertResult UnicodeToEUC(CMemory* pMem);		// Unicode   → EUCコード変換
	static void EUCToSJIS(CMemory* pMem);			// EUC       → SJISコード変換
	static void SJISToEUC(CMemory* pMem);			// SJIS      → EUCコード変換
};
