#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Select : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_SELECT; }
	//色替え
	virtual void InitStrategyStatus(){ }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);

	virtual bool BeginColorEx(const CStringRef& cStr, int nPos, CLayoutInt, const CLayout*);

	//イベント
	virtual void OnStartScanLogic();

private:
	CLayoutInt		m_nSelectLine;
	CLogicInt		m_nSelectStart;
	CLogicInt		m_nSelectEnd;
};

class CColor_Found : public CColorStrategy{
public:
	CColor_Found();
	virtual EColorIndexType GetStrategyColor() const
	{ return this->validColorNum != 0 ? this->highlightColors[ (m_nSearchResult - 1) % this->validColorNum ] : COLORIDX_DEFAULT; }
	//色替え
	virtual void InitStrategyStatus(){ } //############要検証
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
	//イベント
	virtual void OnStartScanLogic();

private:
	int				m_nSearchResult;
	CLogicInt		m_nSearchStart;
	CLogicInt		m_nSearchEnd;
	EColorIndexType highlightColors[ COLORIDX_SEARCHTAIL - COLORIDX_SEARCH + 1 ]; ///< チェックが付いている検索文字列色の配列。
	unsigned validColorNum; ///< highlightColorsの何番目の要素までが有効か。
};
