#pragma once

#include "view/colors/CColorStrategy.h" //SColorStrategyInfo
#include "util/design_template.h"
#include <vector>

struct SColorStrategyInfo;

//$$レイアウト構築フロー(DoLayout)も CFigure で行うと整理しやすい
class CFigure{
public:
	virtual ~CFigure(){}
	virtual bool DrawImp(SColorStrategyInfo* pInfo) = 0;
	virtual bool Match(const wchar_t* pText) const = 0;
	virtual CLayoutInt GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const = 0;
};



class CFigureManager : public TSingleton<CFigureManager>{
public:
	CFigureManager();
	virtual ~CFigureManager();
	CFigure& GetFigure(const wchar_t* pText);

private:
	std::vector<CFigure*>	m_vFigures;
};
