/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CIOBRIDGE_5C6D9CA0_13F8_415A_A948_6A6EF323BE02_H_
#define SAKURA_CIOBRIDGE_5C6D9CA0_13F8_415A_A948_6A6EF323BE02_H_
#pragma once

#include "mem/CMemory.h"
#include "charset/CCodeBase.h"

class CIoBridge{
public:
	//! 内部実装のエンコードへ変換
	static EConvertResult FileToImpl(
		const CMemory&	cSrc,			//!< [in]  変換元メモリ
		CNativeW*		pDst,			//!< [out] 変換先メモリ(UNICODE)
		CCodeBase*		pCodeBase,		//!< [in]  変換元メモリの文字コードクラス
		int				nFlag			//!< [in]  bit 0: MIME Encodeされたヘッダーをdecodeするかどうか
	);

	//! ファイルのエンコードへ変更
	static EConvertResult ImplToFile(
		const CNativeW&		cSrc,		//!< [in]  変換元メモリ(UNICODE)
		CMemory*			pDst,		//!< [out] 変換先メモリ
		CCodeBase*			pCodeBase	//!< [in]  変換先メモリの文字コードクラス
	);
};
#endif /* SAKURA_CIOBRIDGE_5C6D9CA0_13F8_415A_A948_6A6EF323BE02_H_ */
