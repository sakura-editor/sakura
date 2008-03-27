#pragma once

#include "CCodeBase.h"

class CUnicodeBe : public CCodeBase{
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return UnicodeBEToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToUnicodeBE(pDst); }	//!< UNICODE    → 特定コード 変換
	void GetBom(CMemory* pcmemBom);	//!< BOMデータ取得
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< 改行データ取得

public:
	//実装
	static EConvertResult UnicodeBEToUnicode(CMemory* pMem);	// UnicodeBE → Unicodeコード変換 //2007.08.13 kobake 追加
	static EConvertResult UnicodeToUnicodeBE(CMemory* pMem);	// Unicode   → UnicodeBEコード変換
};

class CEol;
SAKURA_CORE_API const wchar_t* GetNextLineWB( const wchar_t*, int, int*, int*, CEol* ); // GetNextLineのwchar_t版(ビックエンディアン用)
