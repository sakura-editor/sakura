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
#ifndef SAKURA_CFIGURE_HANSPACE_5C46367B_9CE4_47DF_A1A1_7AF7BB1D5C92_H_
#define SAKURA_CFIGURE_HANSPACE_5C46367B_9CE4_47DF_A1A1_7AF7BB1D5C92_H_

#include "view/figures/CFigureStrategy.h"

//! 半角スペース描画
class CFigure_HanSpace : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText, int nTextLen) const;

	//action
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool trans) const;
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_SPACE; }
};

#endif /* SAKURA_CFIGURE_HANSPACE_5C46367B_9CE4_47DF_A1A1_7AF7BB1D5C92_H_ */
/*[EOF]*/
