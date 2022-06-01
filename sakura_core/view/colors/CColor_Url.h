﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
