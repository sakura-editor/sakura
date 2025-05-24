/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCOLOR_FOUND_8094BA64_0ACB_4776_A190_D68729D3FABC_H_
#define SAKURA_CCOLOR_FOUND_8094BA64_0ACB_4776_A190_D68729D3FABC_H_
#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Select final : public CColorStrategy{
public:
	EColorIndexType GetStrategyColor() const override{ return COLORIDX_SELECT; }
	//色替え
	void InitStrategyStatus() override{ }
	bool BeginColor(const CStringRef& cStr, int nPos) override;
	bool Disp() const override{ return true; }
	bool EndColor(const CStringRef& cStr, int nPos) override;

	virtual bool BeginColorEx(const CStringRef& cStr, int nPos, CLayoutInt, const CLayout*);

	//イベント
	void OnStartScanLogic() override;

private:
	CLayoutInt		m_nSelectLine;
	CLogicInt		m_nSelectStart;
	CLogicInt		m_nSelectEnd;
};

class CColor_Found final : public CColorStrategy{
public:
	CColor_Found();
	EColorIndexType GetStrategyColor() const override
	{ return this->validColorNum != 0 ? this->highlightColors[ (m_nSearchResult - 1) % this->validColorNum ] : COLORIDX_DEFAULT; }
	//色替え
	void InitStrategyStatus() override{ } //############要検証
	bool BeginColor(const CStringRef& cStr, int nPos) override;
	bool Disp() const override{ return true; }
	bool EndColor(const CStringRef& cStr, int nPos) override;
	//イベント
	void OnStartScanLogic() override;

private:
	int				m_nSearchResult;
	CLogicInt		m_nSearchStart;
	CLogicInt		m_nSearchEnd;
	EColorIndexType highlightColors[ COLORIDX_SEARCHTAIL - COLORIDX_SEARCH + 1 ]; ///< チェックが付いている検索文字列色の配列。
	unsigned validColorNum; ///< highlightColorsの何番目の要素までが有効か。
};
#endif /* SAKURA_CCOLOR_FOUND_8094BA64_0ACB_4776_A190_D68729D3FABC_H_ */
