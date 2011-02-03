#pragma once

#include "view/figures/CFigureStrategy.h"

//! â¸çsï`âÊ
class CFigure_Eol : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	bool DrawImp(SColorStrategyInfo* pInfo);
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_EOL; }
	CEol m_cEol;
};
