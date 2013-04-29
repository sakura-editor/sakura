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
	if( m_pTypeData->m_bUseRegexKeyword
	 && pcView->m_cRegexKeyword->RegexIsKeyword( cStr, nPos, &nMatchLen, &nMatchColor )
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

