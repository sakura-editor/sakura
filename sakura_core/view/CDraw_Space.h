#pragma once

#include "CDrawStrategy.h"



class CDraw_Tab : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
};


class CDraw_ZenSpace : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
};

class CDraw_HanSpace : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
};

