#pragma once

#include "view/colors/CColorStrategy.h"



class CColor_Quote : public CColorStrategy{
public:
	CColor_Quote(wchar_t cQuote) : m_cQuote(cQuote), m_nCOMMENTEND(0) { }
	virtual EColorIndexType GetStrategyColor() const = 0;
	virtual void InitStrategyStatus(){ m_nCOMMENTEND = 0; }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
private:
	wchar_t m_cQuote;
	int m_nCOMMENTEND;
};


class CColor_SingleQuote : public CColor_Quote{
public:
	CColor_SingleQuote() : CColor_Quote(L'\'') { }
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_SSTRING; }
};

class CColor_DoubleQuote : public CColor_Quote{
public:
	CColor_DoubleQuote() : CColor_Quote(L'"') { }
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_WSTRING; }
};
