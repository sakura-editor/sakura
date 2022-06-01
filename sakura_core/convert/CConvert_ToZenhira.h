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
