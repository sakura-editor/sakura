#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"

class CUtf8 : public CCodeBase{
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ return UTF8ToUnicode(cSrc,pDst); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ return UnicodeToUTF8(cSrc,pDst); }	//!< UNICODE    → 特定コード 変換
	void GetBom(CMemory* pcmemBom);																			//!< BOMデータ取得
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< 改行データ取得
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst);			//!< UNICODE → Hex 変換

public:
	//実装
	static EConvertResult UTF8ToUnicode(const CMemory& cSrcMem, CNativeW* pDstMem);		// UTF-8     → Unicodeコード変換 //2007.08.13 kobake 追加
	static EConvertResult UnicodeToUTF8(const CNativeW& cSrcMem, CMemory* pDstMem);		// Unicode   → UTF-8コード変換
};
