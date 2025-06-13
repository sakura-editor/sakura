/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCONVERT_HANKATATOZENHIRA_E5BFC342_4B93_4E34_9380_3E81149A315D_H_
#define SAKURA_CCONVERT_HANKATATOZENHIRA_E5BFC342_4B93_4E34_9380_3E81149A315D_H_
#pragma once

#include "CConvert.h"

//!半角カナ→全角ひらがな
class CConvert_HankataToZenhira final : public CConvert{
public:
	bool DoConvert(CNativeW* pcData) override;
};
#endif /* SAKURA_CCONVERT_HANKATATOZENHIRA_E5BFC342_4B93_4E34_9380_3E81149A315D_H_ */
