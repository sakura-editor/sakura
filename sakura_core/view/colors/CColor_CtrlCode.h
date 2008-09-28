#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_CtrlCode : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_CTRLCODE; }
	virtual void InitStrategyStatus(){}
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
};
