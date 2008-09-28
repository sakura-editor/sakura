#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Eol : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_EOL; }
	virtual void InitStrategyStatus(){}
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
};
