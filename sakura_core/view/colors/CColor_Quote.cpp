#include "stdafx.h"
#include "CColor_Quote.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     クォーテーション                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_Quote::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( cStr.At(nPos) == m_cQuote &&
		TypeDataPtr->m_ColorInfoArr[this->GetStrategyColor()].m_bDisp	/* ダブルクォーテーション文字列を表示する */
	){
		m_bLastEscape = false;
		m_bDone = false;
		return true;
	}
	return false;
}

bool CColor_Quote::EndColor(const CStringRef& cStr, int nPos)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(m_bDone){
		return true;
	}

	//文字列区切り記号エスケープ方法 0=[\"][\']
	if( TypeDataPtr->m_nStringType == 0 ){
		//エスケープ無視
		if(m_bLastEscape){
			m_bLastEscape = false;
			return false;
		}
		//終端
		if(cStr.At(nPos)==m_cQuote){
			m_bDone = true;
			return false;
		}
		//エスケープ検出
		if(cStr.At(nPos)==L'\\'){
			m_bLastEscape = true;
		}
	}
	//文字列区切り記号エスケープ方法 1=[""]['']
	else if(TypeDataPtr->m_nStringType == 1){
		//エスケープ無視
		if(m_bLastEscape){
			m_bLastEscape = false;
			return false;
		}
		//終端
		if(cStr.At(nPos)==m_cQuote){
			//エスケープ検出
			if(cStr.At(nPos+1)==m_cQuote){
				m_bLastEscape = true;
				return false;
			}
			else{
				m_bDone = true;
				return false;
			}
		}
	}
	//想定外
	else{
		assert(0);
	}

	return false;
}
