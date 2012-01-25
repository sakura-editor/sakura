#include "StdAfx.h"
#include "io/CIoBridge.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"

//! 内部実装のエンコードへ変換
EConvertResult CIoBridge::FileToImpl(
	const CMemory&	cSrc,			//!< [in]  変換元メモリ
	CNativeW*		pDst,			//!< [out] 変換先メモリ(UNICODE)
	ECodeType		eCharcodeFrom,	//!< [in]  変換元メモリの文字コード
	int				nFlag			//!< [in]  bit 0: MIME Encodeされたヘッダをdecodeするかどうか
)
{
	//任意の文字コードからUnicodeへ変換する
	CCodeBase* pCode=CCodeFactory::CreateCodeBase(eCharcodeFrom,nFlag);
	EConvertResult ret = pCode->CodeToUnicode(cSrc,pDst);
	delete pCode;

	//結果
	return ret;
}

EConvertResult CIoBridge::ImplToFile(
	const CNativeW&		cSrc,		//!< [in]  変換元メモリ(UNICODE)
	CMemory*			pDst,		//!< [out] 変換先メモリ
	ECodeType			eCharcodeTo	//!< [in]  変換先メモリの文字コード
)
{
	// Unicodeから任意の文字コードへ変換する
	CCodeBase* pCode=CCodeFactory::CreateCodeBase(eCharcodeTo,0);
	EConvertResult ret = pCode->UnicodeToCode(cSrc,pDst);
	delete pCode;

	//結果
	return ret;
}


