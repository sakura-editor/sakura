#pragma once

#include "view/figures/CFigureStrategy.h"

//! �S�p�X�y�[�X�`��
class CFigure_ZenSpace : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_ZENSPACE; }
};
