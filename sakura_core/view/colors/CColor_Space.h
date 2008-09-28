#pragma once

#include "view/colors/CColorStrategy.h"



class CColor_Tab : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_TAB; }
	virtual void InitStrategyStatus(){ }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
};


class CColor_ZenSpace : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_ZENSPACE; }
	virtual void InitStrategyStatus(){ }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
};

class CColor_HanSpace : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_SPACE; }
	virtual void InitStrategyStatus(){ }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
};

