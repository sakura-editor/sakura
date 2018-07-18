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
#ifndef SAKURA_CCOLOR_COMMENT_5CBF888D_32A3_4713_8967_D6D49F6E569F_H_
#define SAKURA_CCOLOR_COMMENT_5CBF888D_32A3_4713_8967_D6D49F6E569F_H_

#include "view/colors/CColorStrategy.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CColor_LineComment : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_COMMENT; }
	virtual void InitStrategyStatus(){}
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
	virtual bool Disp() const { return m_pTypeData->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp; }
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント１                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CColor_BlockComment : public CColorStrategy{
public:
	CColor_BlockComment(EColorIndexType nType) : m_nType(nType), m_nCOMMENTEND(0){}
	virtual void Update(void)
	{
		const CEditDoc* pCEditDoc = CEditDoc::GetInstance(0);
		m_pTypeData = &pCEditDoc->m_cDocType.GetDocumentAttribute();
		m_pcBlockComment = &m_pTypeData->m_cBlockComments[m_nType - COLORIDX_BLOCK1];
	}
	virtual EColorIndexType GetStrategyColor() const{ return m_nType; }
	virtual void InitStrategyStatus(){ m_nCOMMENTEND = 0; }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
	virtual bool Disp() const { return m_pTypeData->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp; }
private:
	EColorIndexType m_nType;
	const CBlockComment* m_pcBlockComment;
	int m_nCOMMENTEND;
};

#endif /* SAKURA_CCOLOR_COMMENT_5CBF888D_32A3_4713_8967_D6D49F6E569F_H_ */
/*[EOF]*/
