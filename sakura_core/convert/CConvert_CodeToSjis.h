/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

#include "CConvert.h"
#include "charset/charset.h"

//! 文字コード変換 xxx→SJIS
class CConvert_CodeToSjis final : public CConvert{
	ECodeType m_eCodeType;

public:
	explicit CConvert_CodeToSjis(ECodeType eCoceType) noexcept;
	bool DoConvert(CNativeW* pcData) override;
};
