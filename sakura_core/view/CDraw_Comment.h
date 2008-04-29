#pragma once

#include "CDrawStrategy.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CDraw_LineComment : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント１                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CDraw_BlockComment : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool EndColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
	virtual bool GetColorIndexImpEnd(SColorInfo* pInfo);
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント２                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CDraw_BlockComment2 : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool EndColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
	virtual bool GetColorIndexImpEnd(SColorInfo* pInfo);
};

