/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFIGURE_HANSPACE_38751BA0_6F58_4929_A24D_1937F2FB3E6A_H_
#define SAKURA_CFIGURE_HANSPACE_38751BA0_6F58_4929_A24D_1937F2FB3E6A_H_
#pragma once

#include "view/figures/CFigureStrategy.h"

//! 半角スペース描画
class CFigure_HanSpace final : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText, int nTextLen) const override;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool trans) const override;
	EColorIndexType GetColorIdx(void) const override { return COLORIDX_SPACE; }
};
#endif /* SAKURA_CFIGURE_HANSPACE_38751BA0_6F58_4929_A24D_1937F2FB3E6A_H_ */
