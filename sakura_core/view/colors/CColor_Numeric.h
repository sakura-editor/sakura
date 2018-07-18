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
#ifndef SAKURA_CCOLOR_NUMERIC_19741AD7_75D8_455C_9E04_7A8901616E8B_H_
#define SAKURA_CCOLOR_NUMERIC_19741AD7_75D8_455C_9E04_7A8901616E8B_H_

#include "view/colors/CColorStrategy.h"

class CColor_Numeric : public CColorStrategy{
public:
	CColor_Numeric() : m_nCOMMENTEND(0) { }
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_DIGIT; }
	virtual void InitStrategyStatus(){ m_nCOMMENTEND = 0; }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
	virtual bool Disp() const { return m_pTypeData->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp; }
private:
	int m_nCOMMENTEND;
};

#endif /* SAKURA_CCOLOR_NUMERIC_19741AD7_75D8_455C_9E04_7A8901616E8B_H_ */
/*[EOF]*/
