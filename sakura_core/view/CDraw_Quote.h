#pragma once

#include "CDrawStrategy.h"


class CDraw_SingleQuote : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};

class CDraw_DoubleQuote : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};

class CDraw_SingleQuoteEnd : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};

class CDraw_DoubleQuoteEnd : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};

