/*
	Copyright (C) 2011, Moca

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
#ifndef SAKURA_CCOLOR_HEREDOC_H_
#define SAKURA_CCOLOR_HEREDOC_H_

#include "view/colors/CColorStrategy.h"

class CColor_Heredoc : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_HEREDOC; }
	virtual CLayoutColorInfo* GetStrategyColorInfo() const;
	virtual void InitStrategyStatus(){ m_nCOMMENTEND = 0; }
	virtual void SetStrategyColorInfo(const CLayoutColorInfo*);
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool Disp() const { return m_pTypeData->m_ColorInfoArr[COLORIDX_HEREDOC].m_bDisp; }
	virtual bool EndColor(const CStringRef& cStr, int nPos);
private:
	std::wstring m_id;
	int	     m_nSize;
	const wchar_t* m_pszId;
	int m_nCOMMENTEND;
};

#endif
