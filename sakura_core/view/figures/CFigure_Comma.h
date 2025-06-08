/*! @file */
/*
	Copyright (C) 2015, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFIGURE_COMMA_DE8237CD_24C0_4A21_8599_5BE8B04BF7E6_H_
#define SAKURA_CFIGURE_COMMA_DE8237CD_24C0_4A21_8599_5BE8B04BF7E6_H_
#pragma once

#include "view/figures/CFigureStrategy.h"

//! カンマ描画（CSVモード）
class CFigure_Comma final : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText, int nTextLen) const override;
	bool Disp(void) const override;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const override;
	EColorIndexType GetColorIdx(void) const override { return COLORIDX_TAB; }
};
#endif /* SAKURA_CFIGURE_COMMA_DE8237CD_24C0_4A21_8599_5BE8B04BF7E6_H_ */
