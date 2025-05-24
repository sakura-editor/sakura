/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCONVERT_781CEC40_5400_4D47_959B_0718AEA82A9B_H_
#define SAKURA_CCONVERT_781CEC40_5400_4D47_959B_0718AEA82A9B_H_
#pragma once

#include "Funccode_enum.h"		// EFunctionCode
#include "basis/SakuraBasis.h"	// CKetaXInt
#include "charset/charcode.h"	// CCharWidthCache
#include "mem/CNativeW.h"
#include "types/CType.h"		// SEncodingConfig

/*!
	各種変換機能呼出の窓口となるクラス
 */
class CConversionFacade {
	int m_nTabWidth;
	int m_nStartColumn;
	bool m_bEnableExtEol;
	SEncodingConfig m_sEncodingConfig;
	CCharWidthCache& m_cCharWidthCache;

public:
	explicit CConversionFacade(
		CKetaXInt nTabWidth,
		int nStartColumn,
		bool bEnableExtEol,
		const SEncodingConfig& sEncodingConfig,
		CCharWidthCache& cCharWidthCache
	);

	//! 機能種別によるバッファの変換
	bool ConvMemory(EFunctionCode eFuncCode, CNativeW& cData) noexcept;

private:
	//! 変換機能を呼び出す
	bool CallConvert(EFunctionCode eFuncCode, CNativeW* pcData) noexcept;
};

/*!
	各種コンバータの基底クラス

	@date 2007/10/02 kobake CEditViewから分離
 */
class CConvert{
public:
	CConvert() noexcept = default;
	CConvert(const CConvert&) = delete;
	CConvert& operator = (const CConvert&) = delete;
	CConvert(CConvert&&) noexcept = delete;
	CConvert& operator = (CConvert&&) noexcept = delete;
	virtual ~CConvert() noexcept = default;

	//実装
	virtual bool DoConvert(CNativeW* pcData) = 0;
};

#endif /* SAKURA_CCONVERT_781CEC40_5400_4D47_959B_0718AEA82A9B_H_ */
