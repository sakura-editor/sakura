/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CUNICODEBE_484B0FE6_3896_4E2E_83BA_CC8718999735_H_
#define SAKURA_CUNICODEBE_484B0FE6_3896_4E2E_83BA_CC8718999735_H_

#include "CCodeBase.h"
#include "CUnicode.h"
#include "CEol.h"

class CUnicodeBe : public CCodeBase{
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ return UnicodeBEToUnicode(cSrc, pDst); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ return UnicodeToUnicodeBE(cSrc, pDst); }	//!< UNICODE    → 特定コード 変換
	void GetBom(CMemory* pcmemBom);	//!< BOMデータ取得
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< 改行データ取得

public:

	inline static EConvertResult UnicodeBEToUnicode(const CMemory& cSrc, CNativeW* pDst)
		{ return CUnicode::_UnicodeToUnicode_in(cSrc, pDst, true); }	// UnicodeBE → Unicodeコード変換 //2007.08.13 kobake 追加
	inline static EConvertResult UnicodeToUnicodeBE(const CNativeW& cSrc, CMemory* pDst)
		{ return CUnicode::_UnicodeToUnicode_out(cSrc, pDst, true); }	// Unicode   → UnicodeBEコード変換

};

#endif /* SAKURA_CUNICODEBE_484B0FE6_3896_4E2E_83BA_CC8718999735_H_ */
/*[EOF]*/
