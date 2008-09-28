#pragma once

#include "view/figures/CFigureStrategy.h"

//! ���s�`��
class CFigure_Eol : public CFigure{
public:
	//traits
	bool DrawImp(SColorStrategyInfo* pInfo);
	bool Match(const wchar_t* pText) const;

	//action
	CLayoutInt GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const;
};
