/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CUNICODE_58DBF44E_DCA1_43C3_B825_EAF692A045DC_H_
#define SAKURA_CUNICODE_58DBF44E_DCA1_43C3_B825_EAF692A045DC_H_
#pragma once

// IsUtf16SurrogHi()、IsUtf16SurrogLow() 関数をcharset/codechecker.h に移動

#include "CCodeBase.h"

class CUnicode : public CCodeBase{
public:
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst) override{	//!< 特定コード → UNICODE    変換
		return UnicodeToUnicode_in(cSrc, pDst);
	}
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst) override{	//!< UNICODE    → 特定コード 変換
		return UnicodeToUnicode_out(cSrc, pDst);
	}

public:
	//実装
	static EConvertResult _UnicodeToUnicode_in(const CMemory& cSrc, CNativeW* pDstMem, const bool bBigEndian);		// Unicode   → Unicode (入力側)
	static EConvertResult _UnicodeToUnicode_out(const CNativeW& cSrc, CMemory* pDstMem, const bool bBigEndian);	// Unicode   → Unicode (出力側)
	inline static EConvertResult UnicodeToUnicode_in(const CMemory& cSrc, CNativeW* pDst){ return _UnicodeToUnicode_in(cSrc, pDst, false); }
	inline static EConvertResult UnicodeToUnicode_out(const CNativeW& cSrc, CMemory* pDst){ return _UnicodeToUnicode_out(cSrc, pDst, false); }
};
#endif /* SAKURA_CUNICODE_58DBF44E_DCA1_43C3_B825_EAF692A045DC_H_ */
