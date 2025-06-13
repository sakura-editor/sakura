/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCONVERT_TOLOWER_D748EDC6_6018_49A9_8F5C_0841F1F0AD51_H_
#define SAKURA_CCONVERT_TOLOWER_D748EDC6_6018_49A9_8F5C_0841F1F0AD51_H_
#pragma once

#include "CConvert.h"

class CConvert_ToLower final : public CConvert{
public:
	bool DoConvert(CNativeW* pcData) override;
};
#endif /* SAKURA_CCONVERT_TOLOWER_D748EDC6_6018_49A9_8F5C_0841F1F0AD51_H_ */
