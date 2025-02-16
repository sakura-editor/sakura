/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCONVERT_HANEISUTOZENEISU_7EDA89FC_8E96_45AA_9612_106430257F7C_H_
#define SAKURA_CCONVERT_HANEISUTOZENEISU_7EDA89FC_8E96_45AA_9612_106430257F7C_H_
#pragma once

#include "CConvert.h"

//!半角英数→全角英数
class CConvert_HaneisuToZeneisu final : public CConvert{
public:
	bool DoConvert(CNativeW* pcData) override;
};
#endif /* SAKURA_CCONVERT_HANEISUTOZENEISU_7EDA89FC_8E96_45AA_9612_106430257F7C_H_ */
