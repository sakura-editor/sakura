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
#ifndef SAKURA_CCOLOR_REGEXKEYWORD_70C5BFF3_FC04_450E_BC07_F5E777B205C29_H_
#define SAKURA_CCOLOR_REGEXKEYWORD_70C5BFF3_FC04_450E_BC07_F5E777B205C29_H_

#include "view/colors/CColorStrategy.h"



class CColor_RegexKeyword : public CColorStrategy{
public:
	CColor_RegexKeyword() : m_nCOMMENTEND(0), m_nCOMMENTMODE(ToColorIndexType_RegularExpression(0)) { }
	virtual EColorIndexType GetStrategyColor() const{ return m_nCOMMENTMODE; }
	virtual void InitStrategyStatus(){ m_nCOMMENTEND = 0; m_nCOMMENTMODE = ToColorIndexType_RegularExpression(0); }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
	virtual bool Disp() const { return m_pTypeData->m_bUseRegexKeyword; }
	virtual void OnStartScanLogic();
private:
	int m_nCOMMENTEND;
	EColorIndexType m_nCOMMENTMODE;
};

#endif /* SAKURA_CCOLOR_REGEXKEYWORD_70C5BFF3_FC04_450E_BC07_F5E777B205C29_H_ */
/*[EOF]*/
