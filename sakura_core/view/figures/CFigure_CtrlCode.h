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
#ifndef SAKURA_CFIGURE_CTRLCODE_9CEA1624_5811_4A1D_8607_1FA8A8439D15_H_
#define SAKURA_CFIGURE_CTRLCODE_9CEA1624_5811_4A1D_8607_1FA8A8439D15_H_

#include "view/figures/CFigureStrategy.h"

//! �R���g���[���R�[�h�`��
class CFigure_CtrlCode : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_CTRLCODE; }
};

//! �o�C�i�����p�`��
class CFigure_HanBinary : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_CTRLCODE; }
};

//! �o�C�i���S�p�`��
class CFigure_ZenBinary : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_CTRLCODE; }
};

#endif /* SAKURA_CFIGURE_CTRLCODE_9CEA1624_5811_4A1D_8607_1FA8A8439D15_H_ */
/*[EOF]*/
