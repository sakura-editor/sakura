/*! @file */
/*
	Copyright (C) 2015, syat
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_CFIGURE_COMMA_DE8237CD_24C0_4A21_8599_5BE8B04BF7E6_H_
#define SAKURA_CFIGURE_COMMA_DE8237CD_24C0_4A21_8599_5BE8B04BF7E6_H_
#pragma once

#include "view/figures/CFigureStrategy.h"

//! カンマ描画（CSVモード）
class CFigure_Comma final : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText, int nTextLen) const override;
	bool Disp(void) const override;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const override;
	EColorIndexType GetColorIdx(void) const override { return COLORIDX_TAB; }
};
#endif /* SAKURA_CFIGURE_COMMA_DE8237CD_24C0_4A21_8599_5BE8B04BF7E6_H_ */
