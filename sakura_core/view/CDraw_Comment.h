#pragma once

#include "CDrawStrategy.h"

class CDraw_LineComment : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};

class CDraw_BlockComment : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};

class CDraw_BlockCommentEnd : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};

class CDraw_BlockCommentEnd2 : public CDrawStrategy{
public:
	virtual bool EnterColor(SDrawStrategyInfo* pInfo);
};

