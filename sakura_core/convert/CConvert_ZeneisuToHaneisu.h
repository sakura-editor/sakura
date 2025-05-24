/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCONVERT_ZENEISUTOHANEISU_D2E876FE_A781_4F64_B2CE_F6CBBF2FC563_H_
#define SAKURA_CCONVERT_ZENEISUTOHANEISU_D2E876FE_A781_4F64_B2CE_F6CBBF2FC563_H_
#pragma once

#include "CConvert.h"

//!全角英数→半角英数
class CConvert_ZeneisuToHaneisu final : public CConvert{
public:
	bool DoConvert(CNativeW* pcData) override;
};
#endif /* SAKURA_CCONVERT_ZENEISUTOHANEISU_D2E876FE_A781_4F64_B2CE_F6CBBF2FC563_H_ */
