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
#ifndef SAKURA_CIOBRIDGE_E98C8096_06B2_41EF_AED2_8D9876BF25BA9_H_
#define SAKURA_CIOBRIDGE_E98C8096_06B2_41EF_AED2_8D9876BF25BA9_H_

#include "mem/CMemory.h"
#include "charset/CCodeBase.h"

class CIoBridge{
public:
	//! 内部実装のエンコードへ変換
	static EConvertResult FileToImpl(
		const CMemory&	cSrc,			//!< [in]  変換元メモリ
		CNativeW*		pDst,			//!< [out] 変換先メモリ(UNICODE)
		CCodeBase*		pCodeBase,		//!< [in]  変換元メモリの文字コードクラス
		int				nFlag			//!< [in]  bit 0: MIME Encodeされたヘッダをdecodeするかどうか
	);

	//! ファイルのエンコードへ変更
	static EConvertResult ImplToFile(
		const CNativeW&		cSrc,		//!< [in]  変換元メモリ(UNICODE)
		CMemory*			pDst,		//!< [out] 変換先メモリ
		CCodeBase*			pCodeBase	//!< [in]  変換先メモリの文字コードクラス
	);
};

#endif /* SAKURA_CIOBRIDGE_E98C8096_06B2_41EF_AED2_8D9876BF25BA9_H_ */
/*[EOF]*/
