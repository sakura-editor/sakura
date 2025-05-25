/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
