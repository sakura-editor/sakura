/*
	Copyright (C) 2018-2019 Sakura Editor Organization

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
#ifndef SAKURA_CCOLOR_CPREPROCESSOR_H_
#define SAKURA_CCOLOR_CPREPROCESSOR_H_

#include "view/colors/CColorStrategy.h"
#include "CWriteManager.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    C プリプロセッサ                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CColor_CPreprocessor final : public CColorStrategy{
public:
	CColor_CPreprocessor() = default;
	~CColor_CPreprocessor();
	virtual void Update(void);
	virtual EColorIndexType GetStrategyColor() const { return COLORIDX_CPREPROCESSOR; }
	virtual void InitStrategyStatus(){  }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
	virtual bool Disp() const {
		// タイプ別設定 『カラー』プロパティのCプリプロセッサのリストアイテムのチェックが付いているか
		if (!m_pTypeData->m_ColorInfoArr[COLORIDX_CPREPROCESSOR].m_bDisp)
			return false;
		return true;
	}
};

#endif /* SAKURA_CCOLOR_CPREPROCESSOR_H_ */
/*[EOF]*/
