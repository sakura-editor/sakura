#pragma once

#include "view/figures/CFigureStrategy.h"

//! コントロールコード描画
class CFigure_CtrlCode : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_CTRLCODE; }
};

//! バイナリ半角描画
class CFigure_HanBinary : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_CTRLCODE; }
};

//! バイナリ全角描画
class CFigure_ZenBinary : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_CTRLCODE; }
};
