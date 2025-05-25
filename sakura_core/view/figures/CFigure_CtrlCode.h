/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFIGURE_CTRLCODE_53EB409B_17F7_4B7F_9AD2_A00C29CDC792_H_
#define SAKURA_CFIGURE_CTRLCODE_53EB409B_17F7_4B7F_9AD2_A00C29CDC792_H_
#pragma once

#include "view/figures/CFigureStrategy.h"

//! コントロールコード描画
class CFigure_CtrlCode : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText, int nTextLen) const override;

	//action
	bool DrawImp(SColorStrategyInfo* pInfo) override;
	virtual void DispSpaceEx(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans, int width) const;
	virtual wchar_t GetAlternateChar() const{ return L'･'; }
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const override
	{
		assert(0);
	}
	EColorIndexType GetColorIdx(void) const override { return COLORIDX_CTRLCODE; }
};
#endif /* SAKURA_CFIGURE_CTRLCODE_53EB409B_17F7_4B7F_9AD2_A00C29CDC792_H_ */
