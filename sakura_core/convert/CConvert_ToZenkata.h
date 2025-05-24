/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCONVERT_TOZENKATA_E29B7690_ADDA_4A97_9F09_BBBBBB4C1356_H_
#define SAKURA_CCONVERT_TOZENKATA_E29B7690_ADDA_4A97_9F09_BBBBBB4C1356_H_
#pragma once

#include "CConvert.h"

//!できる限り全角カタカナにする
class CConvert_ToZenkata final : public CConvert{
public:
	bool DoConvert(CNativeW* pcData) override;
};
#endif /* SAKURA_CCONVERT_TOZENKATA_E29B7690_ADDA_4A97_9F09_BBBBBB4C1356_H_ */
