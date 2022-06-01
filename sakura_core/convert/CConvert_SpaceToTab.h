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
#ifndef SAKURA_CCONVERT_SPACETOTAB_AA8D9341_7190_4332_AA23_C0D9AA4DC8D0_H_
#define SAKURA_CCONVERT_SPACETOTAB_AA8D9341_7190_4332_AA23_C0D9AA4DC8D0_H_
#pragma once

#include "CConvert.h"

class CCharWidthCache;

class CConvert_SpaceToTab final : public CConvert{
public:
	CConvert_SpaceToTab(int nTabWidth, int nStartColumn, bool bExtEol, CCharWidthCache& cache)
		: m_nTabWidth(nTabWidth), m_nStartColumn(nStartColumn),
		  m_bExtEol(bExtEol), m_cCache(cache)
	{
	}

	bool DoConvert(CNativeW* pcData) override;

private:
	int m_nTabWidth;
	int m_nStartColumn;
	bool m_bExtEol;
	CCharWidthCache& m_cCache;
};
#endif /* SAKURA_CCONVERT_SPACETOTAB_AA8D9341_7190_4332_AA23_C0D9AA4DC8D0_H_ */
