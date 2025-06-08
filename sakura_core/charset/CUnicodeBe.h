/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CUNICODEBE_26648A4B_A79C_4739_ADD3_DC7833A91E18_H_
#define SAKURA_CUNICODEBE_26648A4B_A79C_4739_ADD3_DC7833A91E18_H_
#pragma once

#include "CCodeBase.h"
#include "CUnicode.h"

class CUnicodeBe : public CCodeBase{
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst) override{ return UnicodeBEToUnicode(cSrc, pDst); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst) override{ return UnicodeToUnicodeBE(cSrc, pDst); }	//!< UNICODE    → 特定コード 変換

public:

	inline static EConvertResult UnicodeBEToUnicode(const CMemory& cSrc, CNativeW* pDst)
		{ return CUnicode::_UnicodeToUnicode_in(cSrc, pDst, true); }	// UnicodeBE → Unicodeコード変換 //2007.08.13 kobake 追加
	inline static EConvertResult UnicodeToUnicodeBE(const CNativeW& cSrc, CMemory* pDst)
		{ return CUnicode::_UnicodeToUnicode_out(cSrc, pDst, true); }	// Unicode   → UnicodeBEコード変換
};
#endif /* SAKURA_CUNICODEBE_26648A4B_A79C_4739_ADD3_DC7833A91E18_H_ */
