/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCOLOR_URL_E9C938AC_E5C6_468E_925E_A7787635AB82_H_
#define SAKURA_CCOLOR_URL_E9C938AC_E5C6_468E_925E_A7787635AB82_H_
#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Url final : public CColorStrategy{
public:
	CColor_Url() : m_nCOMMENTEND(0) { }
	EColorIndexType GetStrategyColor() const override{ return COLORIDX_URL; }
	void InitStrategyStatus() override{ m_nCOMMENTEND = 0; }
	bool BeginColor(const CStringRef& cStr, int nPos) override;
	bool EndColor(const CStringRef& cStr, int nPos) override;
	bool Disp() const override{ return m_pTypeData->m_ColorInfoArr[COLORIDX_URL].m_bDisp; }
private:
	int m_nCOMMENTEND;
};
#endif /* SAKURA_CCOLOR_URL_E9C938AC_E5C6_468E_925E_A7787635AB82_H_ */
