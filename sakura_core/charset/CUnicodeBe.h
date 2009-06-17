#pragma once

#include "CCodeBase.h"
#include "CUnicode.h"

class CUnicodeBe : public CCodeBase{
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return UnicodeBEToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToUnicodeBE(pDst); }	//!< UNICODE    → 特定コード 変換
	void GetBom(CMemory* pcmemBom);	//!< BOMデータ取得
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< 改行データ取得

public:

	inline static EConvertResult UnicodeBEToUnicode(CMemory* pMem)
		{ return CUnicode::_UnicodeToUnicode_in(pMem,true); }	// UnicodeBE → Unicodeコード変換 //2007.08.13 kobake 追加
	inline static EConvertResult UnicodeToUnicodeBE(CMemory* pMem)
		{ return CUnicode::_UnicodeToUnicode_out(pMem,true); }	// Unicode   → UnicodeBEコード変換

};
