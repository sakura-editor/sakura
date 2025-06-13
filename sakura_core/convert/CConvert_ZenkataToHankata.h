/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCONVERT_ZENKATATOHANKATA_423F8C6C_7FB9_467C_B1DD_2A021A5BAFF8_H_
#define SAKURA_CCONVERT_ZENKATATOHANKATA_423F8C6C_7FB9_467C_B1DD_2A021A5BAFF8_H_
#pragma once

#include "CConvert.h"

//!全角カナ→半角カナ
class CConvert_ZenkataToHankata final : public CConvert{
public:
	bool DoConvert(CNativeW* pcData) override;
};
#endif /* SAKURA_CCONVERT_ZENKATATOHANKATA_423F8C6C_7FB9_467C_B1DD_2A021A5BAFF8_H_ */
