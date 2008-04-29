#pragma once

#include "CDrawStrategy.h"


class CDraw_SingleQuote : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool EndColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
	virtual bool GetColorIndexImpEnd(SColorInfo* pInfo);
};

class CDraw_DoubleQuote : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool EndColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
	virtual bool GetColorIndexImpEnd(SColorInfo* pInfo);
};

