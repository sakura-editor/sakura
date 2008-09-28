#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Numeric : public CColorStrategy{
public:
	CColor_Numeric() : m_nCOMMENTEND(0) { }
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_DIGIT; }
	virtual void InitStrategyStatus(){ m_nCOMMENTEND = 0; }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
private:
	int m_nCOMMENTEND;
};
