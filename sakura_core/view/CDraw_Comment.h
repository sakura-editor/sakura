#pragma once

#include "CDrawStrategy.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �s�R�����g                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CDraw_LineComment : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �u���b�N�R�����g�P                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CDraw_BlockComment : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool EndColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
	virtual bool GetColorIndexImpEnd(SColorInfo* pInfo);
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �u���b�N�R�����g�Q                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CDraw_BlockComment2 : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool EndColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
	virtual bool GetColorIndexImpEnd(SColorInfo* pInfo);
};

