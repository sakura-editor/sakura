/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCONVERT_TOZENHIRA_31D2B3A3_CB3F_43A5_A027_4FEE96D6731C_H_
#define SAKURA_CCONVERT_TOZENHIRA_31D2B3A3_CB3F_43A5_A027_4FEE96D6731C_H_
#pragma once

#include "CConvert.h"

//!できる限り全角ひらがなにする
class CConvert_ToZenhira final : public CConvert{
public:
	bool DoConvert(CNativeW* pcData) override;
};
#endif /* SAKURA_CCONVERT_TOZENHIRA_31D2B3A3_CB3F_43A5_A027_4FEE96D6731C_H_ */
