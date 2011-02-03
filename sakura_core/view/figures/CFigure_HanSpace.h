#pragma once

#include "view/figures/CFigureStrategy.h"

//! 半角スペース描画
class CFigure_HanSpace : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool trans) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_SPACE; }
};
