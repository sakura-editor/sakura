#pragma once

#include "view/figures/CFigureStrategy.h"

//! ���s�`��
class CFigure_Eol : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	bool DrawImp(SColorStrategyInfo* pInfo);
	int GetSpaceColorType(const EColorIndexType& eCurColor) const{ return (COLORIDX_SEARCH == eCurColor)? 1: 0; }
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_EOL; }
	CEol m_cEol;
};
