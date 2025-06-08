/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFIGURE_ZENSPACE_6176BBA4_68C9_41A1_B944_7F6EE0E5E4A4_H_
#define SAKURA_CFIGURE_ZENSPACE_6176BBA4_68C9_41A1_B944_7F6EE0E5E4A4_H_
#pragma once

#include "view/figures/CFigureStrategy.h"

//! 全角スペース描画
class CFigure_ZenSpace final : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText, int nTextLen) const override;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const override;
	EColorIndexType GetColorIdx(void) const override { return COLORIDX_ZENSPACE; }
};
#endif /* SAKURA_CFIGURE_ZENSPACE_6176BBA4_68C9_41A1_B944_7F6EE0E5E4A4_H_ */
