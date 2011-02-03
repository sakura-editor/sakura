#pragma once

#include "view/figures/CFigureStrategy.h"

//! ���p�X�y�[�X�`��
class CFigure_HanSpace : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool trans) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_SPACE; }
};
