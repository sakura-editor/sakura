﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
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
