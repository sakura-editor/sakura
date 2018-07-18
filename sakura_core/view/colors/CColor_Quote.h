/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CCOLOR_QUOTE_26330E31_5ADC_4753_92DD_7567B7BA4451_H_
#define SAKURA_CCOLOR_QUOTE_26330E31_5ADC_4753_92DD_7567B7BA4451_H_

#include "view/colors/CColorStrategy.h"



class CColor_Quote : public CColorStrategy{
public:
	CColor_Quote(wchar_t cQuote) : m_cQuote(cQuote), m_nCOMMENTEND(-1),m_nColorTypeIndex(0) {
		m_szQuote[0] = cQuote;
		m_szQuote[1] = cQuote;
		m_szQuote[2] = cQuote;
	}
	virtual void Update(void);
	virtual EColorIndexType GetStrategyColor() const = 0;
	virtual CLayoutColorInfo* GetStrategyColorInfo() const;
	virtual void InitStrategyStatus(){ m_nCOMMENTEND = -1; }
	virtual void SetStrategyColorInfo(const CLayoutColorInfo*);
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
	virtual bool Disp() const { return m_pTypeData->m_ColorInfoArr[this->GetStrategyColor()].m_bDisp; }

	static bool IsCppRawString(const CStringRef& cStr, int nPos);
	static int Match_Quote( wchar_t wcQuote, int nPos, const CStringRef& cLineStr, int escapeType, bool* pbEscapeEnd = NULL );
	static int Match_QuoteStr( const wchar_t* szQuote, int nQuoteLen, int nPos, const CStringRef& cLineStr, bool bEscape );
private:

	wchar_t m_cQuote;
	wchar_t m_szQuote[3];
	int m_nCOMMENTEND;
	std::wstring m_tag;

	int m_nStringType;
	int m_nEscapeType;
	bool* m_pbEscapeEnd;
	bool m_bEscapeEnd;
protected:
	int m_nColorTypeIndex;
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

#endif /* SAKURA_CCOLOR_QUOTE_26330E31_5ADC_4753_92DD_7567B7BA4451_H_ */
/*[EOF]*/
