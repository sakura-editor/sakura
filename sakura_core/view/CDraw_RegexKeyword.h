#pragma once

#include "CDrawStrategy.h"



class CDraw_RegexKeyword : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool EndColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
};
