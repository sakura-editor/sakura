#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"

class CEuc : public CCodeBase{
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return EUCToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToEUC(pDst); }	//!< UNICODE    → 特定コード 変換
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< 改行データ取得

public:
	//実装
	static EConvertResult EUCToUnicode(CMemory* pMem);		// EUC       → Unicodeコード変換  //2007.08.13 kobake 追加
	static EConvertResult UnicodeToEUC(CMemory* pMem);		// Unicode   → EUCコード変換
	static void EUCToSJIS(CMemory* pMem);			// EUC       → SJISコード変換
	static void SJISToEUC(CMemory* pMem);			// SJIS      → EUCコード変換
};
