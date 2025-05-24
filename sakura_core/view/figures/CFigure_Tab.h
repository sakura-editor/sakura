/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFIGURE_TAB_4401678E_D165_4130_A973_CC40038CDE8E_H_
#define SAKURA_CFIGURE_TAB_4401678E_D165_4130_A973_CC40038CDE8E_H_
#pragma once

#include "view/figures/CFigureStrategy.h"

//! タブ描画
class CFigure_Tab final : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText, int nTextLen) const override;
	bool Disp(void) const override
	{
		return true;
	}

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const override;
	EColorIndexType GetColorIdx(void) const override { return COLORIDX_TAB; }
};
#endif /* SAKURA_CFIGURE_TAB_4401678E_D165_4130_A973_CC40038CDE8E_H_ */
