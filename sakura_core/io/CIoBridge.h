#pragma once

#include "CMemory.h"
#include "charset/CCodeBase.h"

class CIoBridge{
public:
	//! 内部実装のエンコードへ変換
	static EConvertResult FileToImpl(
		const CMemory&	cSrc,			//!< [in]  変換元メモリ
		CNativeW*		pDst,			//!< [out] 変換先メモリ(UNICODE)
		ECodeType		eCharcodeFrom,	//!< [in]  変換元メモリの文字コード
		int				nFlag			//!< [in]  bit 0: MIME Encodeされたヘッダをdecodeするかどうか
	);

	//! ファイルのエンコードへ変更
	static EConvertResult ImplToFile(
		const CNativeW&		cSrc,		//!< [in]  変換元メモリ(UNICODE)
		CMemory*			pDst,		//!< [out] 変換先メモリ
		ECodeType			eCharcodeTo	//!< [in]  変換先メモリの文字コード
	);
};

