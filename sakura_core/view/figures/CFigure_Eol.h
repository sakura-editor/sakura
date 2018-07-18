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
#ifndef SAKURA_CFIGURE_EOL_F6A22B9F_DDED_4BE9_9A2C_62931DB224F6_H_
#define SAKURA_CFIGURE_EOL_F6A22B9F_DDED_4BE9_9A2C_62931DB224F6_H_

#include "view/figures/CFigureStrategy.h"

//! 改行描画
class CFigure_Eol : public CFigureSpace{
public:
	//traits
	bool Match(const wchar_t* pText, int nTextLen) const;
	bool Disp(void) const
	{
		return true;
	}

	//action
	bool DrawImp(SColorStrategyInfo* pInfo);
	void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const {};
	EColorIndexType GetColorIdx(void) const{ return COLORIDX_EOL; }
};

#endif /* SAKURA_CFIGURE_EOL_F6A22B9F_DDED_4BE9_9A2C_62931DB224F6_H_ */
/*[EOF]*/
