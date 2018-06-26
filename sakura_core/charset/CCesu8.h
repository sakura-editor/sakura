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
#ifndef SAKURA_CCESU8_5E82F67D_CFE7_4B3D_9BA2_22FD3F31086F_H_
#define SAKURA_CCESU8_5E82F67D_CFE7_4B3D_9BA2_22FD3F31086F_H_

#include "CCodeBase.h"
#include "CUtf8.h"


class CCesu8 : public CCodeBase {
public:

	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){	//!< 特定コード → UNICODE    変換
		return CUtf8::CESU8ToUnicode(cSrc, pDst);
	}
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){	//!< UNICODE    → 特定コード 変換
		return CUtf8::UnicodeToCESU8(cSrc, pDst);
	}
	void GetBom(CMemory* pcmemBom);																			//!< BOMデータ取得
// GetEolはCCodeBaseに移動	2010/6/13 Uchi
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar){			//!< UNICODE → Hex 変換
		return CUtf8()._UnicodeToHex( cSrc, iSLen, pDst, psStatusbar, true );
	}

};

#endif /* SAKURA_CCESU8_5E82F67D_CFE7_4B3D_9BA2_22FD3F31086F_H_ */
/*[EOF]*/
