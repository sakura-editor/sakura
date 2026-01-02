/*! @file */
/*
	Copyright (C) 2011, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCOLOR_HEREDOC_88B5007B_1615_411C_AED2_73E7AF92C146_H_
#define SAKURA_CCOLOR_HEREDOC_88B5007B_1615_411C_AED2_73E7AF92C146_H_
#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Heredoc final : public CColorStrategy{
public:
	EColorIndexType GetStrategyColor() const override{ return COLORIDX_HEREDOC; }
	CLayoutColorInfo* GetStrategyColorInfo() const override;
	void InitStrategyStatus() override{ m_nCOMMENTEND = 0; }
	void SetStrategyColorInfo(const CLayoutColorInfo*) override;
	bool BeginColor(const CStringRef& cStr, int nPos) override;
	bool Disp() const override{ return m_pTypeData->m_ColorInfoArr[COLORIDX_HEREDOC].m_bDisp; }
	bool EndColor(const CStringRef& cStr, int nPos) override;
private:
	std::wstring	m_id;
	LPCWSTR			m_pszId = nullptr;
	size_t			m_nSize = 0;
	size_t			m_nCOMMENTEND = 0;
};

#endif /* SAKURA_CCOLOR_HEREDOC_88B5007B_1615_411C_AED2_73E7AF92C146_H_ */
