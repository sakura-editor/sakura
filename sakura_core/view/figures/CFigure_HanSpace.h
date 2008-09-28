#pragma once

#include "view/figures/CFigureStrategy.h"

//! ���p�X�y�[�X�`��
class CFigure_HanSpace : public CFigure{
public:
	//traits
	bool Match(const wchar_t* pText) const;
	CLayoutInt GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const;

	//action
	bool DrawImp(SColorStrategyInfo* pInfo);
};
