#pragma once

#include "CDrawStrategy.h"

class CDraw_Found : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};

class CDraw_FoundEnd : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};

