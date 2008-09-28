#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Url : public CColorStrategy{
public:
	CColor_Url() : m_nCOMMENTEND(0) { }
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_URL; }
	virtual void InitStrategyStatus(){ m_nCOMMENTEND = 0; }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
private:
	int m_nCOMMENTEND;
};
