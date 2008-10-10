#pragma once

#include "view/colors/CColorStrategy.h"



class CColor_Tab : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_TAB; }
	virtual void InitStrategyStatus(){ }
};


class CColor_ZenSpace : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_ZENSPACE; }
	virtual void InitStrategyStatus(){ }
};

class CColor_HanSpace : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_SPACE; }
	virtual void InitStrategyStatus(){ }
};

