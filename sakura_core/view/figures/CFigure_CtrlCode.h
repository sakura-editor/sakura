#pragma once

#include "view/figures/CFigureStrategy.h"

//! �R���g���[���R�[�h�`��
class CFigure_CtrlCode : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_CTRLCODE; }
};

//! �o�C�i�����p�`��
class CFigure_HanBinary : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_CTRLCODE; }
};

//! �o�C�i���S�p�`��
class CFigure_ZenBinary : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_CTRLCODE; }
};
