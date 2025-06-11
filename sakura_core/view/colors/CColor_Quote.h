/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCOLOR_QUOTE_DA1E69BE_19D1_4B16_BCF0_1516B00565D1_H_
#define SAKURA_CCOLOR_QUOTE_DA1E69BE_19D1_4B16_BCF0_1516B00565D1_H_
#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Quote : public CColorStrategy{
public:
	CColor_Quote(wchar_t cQuote) : m_cQuote(cQuote), m_nCOMMENTEND(-1),m_nColorTypeIndex(0) {
		m_szQuote[0] = cQuote;
		m_szQuote[1] = cQuote;
		m_szQuote[2] = cQuote;
	}
	void Update(void) override;
	CLayoutColorInfo* GetStrategyColorInfo() const override;
	void InitStrategyStatus() override{ m_nCOMMENTEND = -1; }
	void SetStrategyColorInfo(const CLayoutColorInfo*) override;
	bool BeginColor(const CStringRef& cStr, int nPos) override;
	bool EndColor(const CStringRef& cStr, int nPos) override;
	bool Disp() const override{ return m_pTypeData->m_ColorInfoArr[this->GetStrategyColor()].m_bDisp; }

	static bool IsCppRawString(const CStringRef& cStr, int nPos);
	static int Match_Quote( wchar_t wcQuote, int nPos, const CStringRef& cLineStr, int escapeType, bool* pbEscapeEnd = nullptr );
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

class CColor_SingleQuote final : public CColor_Quote{
public:
	CColor_SingleQuote() : CColor_Quote(L'\'') { }
	EColorIndexType GetStrategyColor() const override{ return COLORIDX_SSTRING; }
};

class CColor_DoubleQuote final : public CColor_Quote{
public:
	CColor_DoubleQuote() : CColor_Quote(L'"') { }
	EColorIndexType GetStrategyColor() const override{ return COLORIDX_WSTRING; }
};
#endif /* SAKURA_CCOLOR_QUOTE_DA1E69BE_19D1_4B16_BCF0_1516B00565D1_H_ */
