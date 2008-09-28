#pragma once

#include "view/figures/CFigureStrategy.h"

//! �S�p�X�y�[�X�`��
class CFigure_ZenSpace : public CFigure{
public:
	//traits
	bool Match(const wchar_t* pText) const;
	CLayoutInt GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const;

	//action
	bool DrawImp(SColorStrategyInfo* pInfo);
};
