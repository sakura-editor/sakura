/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "StdAfx.h"
#include "io/CIoBridge.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"
#include "CEol.h"

//! 内部実装のエンコードへ変換
EConvertResult CIoBridge::FileToImpl(
	const CMemory&		cSrc,		//!< [in]  変換元メモリ
	CNativeW*			pDst,		//!< [out] 変換先メモリ(UNICODE)
	CCodeBase*			pCode,		//!< [in]  変換元メモリの文字コード
	int					nFlag		//!< [in]  bit 0: MIME Encodeされたヘッダをdecodeするかどうか
)
{
	//任意の文字コードからUnicodeへ変換する
	EConvertResult ret = pCode->CodeToUnicode(cSrc,pDst);

	//結果
	return ret;
}

EConvertResult CIoBridge::ImplToFile(
	const CNativeW&		cSrc,		//!< [in]  変換元メモリ(UNICODE)
	CMemory*			pDst,		//!< [out] 変換先メモリ
	CCodeBase*			pCode		//!< [in]  変換先メモリの文字コード
)
{
	// Unicodeから任意の文字コードへ変換する
	EConvertResult ret = pCode->UnicodeToCode(cSrc,pDst);

	//結果
	return ret;
}
