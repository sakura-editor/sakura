#pragma once

#include "view/colors/CColorStrategy.h"



class CColor_RegexKeyword : public CColorStrategy{
public:
	CColor_RegexKeyword() : m_nCOMMENTEND(0), m_nCOMMENTMODE(EColorIndexType(COLORIDX_REGEX_FIRST + COLORIDX_DEFAULT)) { }
	virtual EColorIndexType GetStrategyColor() const{ return m_nCOMMENTMODE; }
	virtual void InitStrategyStatus(){ m_nCOMMENTEND = 0; m_nCOMMENTMODE = EColorIndexType(COLORIDX_REGEX_FIRST + COLORIDX_DEFAULT); }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
private:
	int m_nCOMMENTEND;
	EColorIndexType m_nCOMMENTMODE;
};
