/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCONVERT_TOUPPER_B5706A27_CFA5_42C8_8B8A_5FA05967AABC_H_
#define SAKURA_CCONVERT_TOUPPER_B5706A27_CFA5_42C8_8B8A_5FA05967AABC_H_
#pragma once

#include "CConvert.h"

class CConvert_ToUpper final : public CConvert{
public:
	bool DoConvert(CNativeW* pcData) override;
};
#endif /* SAKURA_CCONVERT_TOUPPER_B5706A27_CFA5_42C8_8B8A_5FA05967AABC_H_ */
