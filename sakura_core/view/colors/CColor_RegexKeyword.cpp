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

#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_RegexKeyword.h"


bool CColor_RegexKeyword::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	int		nMatchLen;
	int		nMatchColor;

	const CEditView* pcView = CColorStrategyPool::getInstance()->GetCurrentView();

	//正規表現キーワード
	if( pcView->m_cRegexKeyword->RegexIsKeyword( cStr, nPos, &nMatchLen, &nMatchColor )
	){
		this->m_nCOMMENTEND = nPos + nMatchLen;  /* キーワード文字列の終端をセットする */
		this->m_nCOMMENTMODE = ToColorIndexType_RegularExpression(nMatchColor);
		return true;
	}
	return false;
}


bool CColor_RegexKeyword::EndColor(const CStringRef& cStr, int nPos)
{
	if( nPos == this->m_nCOMMENTEND ){
		return true;
	}
	return false;
}

void CColor_RegexKeyword::OnStartScanLogic()
{
	CEditView* pcView = CColorStrategyPool::getInstance()->GetCurrentView();
	if( m_pTypeData->m_bUseRegexKeyword ){
		pcView->m_cRegexKeyword->RegexKeyLineStart();
	}
}
