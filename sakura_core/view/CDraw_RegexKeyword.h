#pragma once

#include "CDrawStrategy.h"



class CDraw_RegexKeyword : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};


class CDraw_RegexKeywordEnd : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};

