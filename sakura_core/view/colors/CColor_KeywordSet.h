#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_KeywordSet : public CColorStrategy{
public:
	CColor_KeywordSet();
	virtual EColorIndexType GetStrategyColor() const{ return (EColorIndexType)(COLORIDX_KEYWORD1 + m_nKeywordIndex); }
	virtual void InitStrategyStatus(){ m_nCOMMENTEND = 0; }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
private:
	int m_nKeywordIndex;
	int m_nCOMMENTEND;
};
