#pragma once

#include "view/figures/CFigureStrategy.h"

//! �R���g���[���R�[�h�`��
class CFigure_CtrlCode : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;
	CLayoutInt GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_CTRLCODE; }
};
