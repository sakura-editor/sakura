/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCESU8_25BC6708_E05F_44A8_936C_8C2FE260AA72_H_
#define SAKURA_CCESU8_25BC6708_E05F_44A8_936C_8C2FE260AA72_H_
#pragma once

#include "CCodeBase.h"
#include "CUtf8.h"

class CCesu8 : public CCodeBase {
public:

	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst) override{	//!< 特定コード → UNICODE    変換
		return CUtf8::CESU8ToUnicode(cSrc, pDst);
	}
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst) override{	//!< UNICODE    → 特定コード 変換
		return CUtf8::UnicodeToCESU8(cSrc, pDst);
	}
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, WCHAR* pDst, const CommonSetting_Statusbar* psStatusbar) override {			//!< UNICODE → Hex 変換
		return CUtf8()._UnicodeToHex( cSrc, iSLen, pDst, psStatusbar, true );
	}
};
#endif /* SAKURA_CCESU8_25BC6708_E05F_44A8_936C_8C2FE260AA72_H_ */
