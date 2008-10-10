#pragma once

#include "view/figures/CFigureStrategy.h"

//! ƒ^ƒu•`‰æ
class CFigure_Tab : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;
	CLayoutInt GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_TAB; }
};
