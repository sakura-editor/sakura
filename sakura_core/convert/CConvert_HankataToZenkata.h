/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCONVERT_HANKATATOZENKATA_2C7E51E1_CA18_41C1_867C_E0CC9CD2138D_H_
#define SAKURA_CCONVERT_HANKATATOZENKATA_2C7E51E1_CA18_41C1_867C_E0CC9CD2138D_H_
#pragma once

#include "CConvert.h"

//!半角カナ→全角カナ
class CConvert_HankataToZenkata final : public CConvert{
public:
	bool DoConvert(CNativeW* pcData) override;
};
#endif /* SAKURA_CCONVERT_HANKATATOZENKATA_2C7E51E1_CA18_41C1_867C_E0CC9CD2138D_H_ */
