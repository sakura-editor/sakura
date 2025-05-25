/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

#include "CConvert.h"
#include "types/CType.h"	//SEncodingConfig

//! 文字コード変換 自動判別→SJIS
class CConvert_CodeAutoToSjis final : public CConvert{
	SEncodingConfig m_sEncodingConfig;

public:
	explicit CConvert_CodeAutoToSjis(const SEncodingConfig& sEncodingConfig ) noexcept;
	bool DoConvert(CNativeW* pcData) override;
};
