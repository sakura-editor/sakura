#pragma once

#include "CDrawStrategy.h"

class CDraw_Found : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool EndColor(SDrawStrategyInfo* pInfo);
};


