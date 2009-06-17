#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"
#include "CUtf8.h"


class CCesu8 : public CCodeBase {
public:

	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){	//!< 特定コード → UNICODE    変換
		*pDst->_GetMemory()=cSrc; return CUtf8::CESU8ToUnicode(pDst->_GetMemory());
	}
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){	//!< UNICODE    → 特定コード 変換
		*pDst=*cSrc._GetMemory(); return CUtf8::UnicodeToCESU8(pDst);
	}
	void GetBom(CMemory* pcmemBom);																			//!< BOMデータ取得
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< 改行データ取得
	inline EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst){			//!< UNICODE → Hex 変換
		return CUtf8()._UnicodeToHex( cSrc, iSLen, pDst, true );
	}

};
