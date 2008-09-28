#pragma once

#include "view/colors/CColorStrategy.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CColor_LineComment : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_COMMENT; }
	virtual void InitStrategyStatus(){}
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント１                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CColor_BlockComment : public CColorStrategy{
public:
	CColor_BlockComment(int nType) : m_nType(nType), m_nCOMMENTEND(0) { }
	virtual EColorIndexType GetStrategyColor() const{ return (EColorIndexType)(COLORIDX_BLOCK1 + m_nType); }
	virtual void InitStrategyStatus(){ m_nCOMMENTEND = 0; }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
private:
	int m_nType; //0 or 1 (コメント種)
	int m_nCOMMENTEND;
};


