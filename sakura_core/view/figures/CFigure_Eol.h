/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFIGURE_EOL_C51A4502_29AE_4D38_8056_5B0CFCC3686B_H_
#define SAKURA_CFIGURE_EOL_C51A4502_29AE_4D38_8056_5B0CFCC3686B_H_
#pragma once

#include "view/figures/CFigureStrategy.h"

//! 改行描画
class CFigure_Eol final : public CFigureSpace{
	using Me = CFigure_Eol;

public:
	CFigure_Eol() noexcept = default;
	CFigure_Eol(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CFigure_Eol(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CFigure_Eol()
	{
		if (m_hPen) {
			::DeleteObject(m_hPen);
		}
	}
	//traits
	bool Match(const wchar_t* pText, int nTextLen) const override;
	bool Disp(void) const override
	{
		return true;
	}

	//action
	bool DrawImp(SColorStrategyInfo* pInfo) override;
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const  override {};
	EColorIndexType GetColorIdx(void) const override { return COLORIDX_EOL; }

private:
	HPEN m_hPen = NULL;
	COLORREF m_clrPen;
};
#endif /* SAKURA_CFIGURE_EOL_C51A4502_29AE_4D38_8056_5B0CFCC3686B_H_ */
