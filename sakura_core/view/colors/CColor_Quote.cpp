#include "stdafx.h"
#include "CColor_Quote.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     クォーテーション                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_Quote::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	if( cStr.At(nPos) == m_cQuote ){
		const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
		const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
		if( TypeDataPtr->m_ColorInfoArr[this->GetStrategyColor()].m_bDisp ){	/* クォーテーション文字列を表示する */
			/* クォーテーション文字列の終端があるか */
			this->m_nCOMMENTEND = pcDoc->m_cLayoutMgr.Match_Quote( m_cQuote, nPos + 1, cStr );
			return true;
		}
	}
	return false;
}

bool CColor_Quote::EndColor(const CStringRef& cStr, int nPos)
{
	if( 0 == this->m_nCOMMENTEND ){
		/* クォーテーション文字列の終端があるか */
		const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
		this->m_nCOMMENTEND = pcDoc->m_cLayoutMgr.Match_Quote( m_cQuote, nPos, cStr );
	}
	else if( nPos == this->m_nCOMMENTEND ){
		return true;
	}

	return false;
}
