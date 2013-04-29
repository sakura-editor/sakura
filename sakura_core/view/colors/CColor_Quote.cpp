#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Quote.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �N�H�[�e�[�V����                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_Quote::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	if( cStr.At(nPos) == m_cQuote ){
		if( m_pTypeData->m_ColorInfoArr[this->GetStrategyColor()].m_bDisp ){	/* �N�H�[�e�[�V�����������\������ */
			/* �N�H�[�e�[�V����������̏I�[�����邩 */
			this->m_nCOMMENTEND = m_pCEditDoc->m_cLayoutMgr.Match_Quote( m_cQuote, nPos + 1, cStr );
			return true;
		}
	}
	return false;
}

bool CColor_Quote::EndColor(const CStringRef& cStr, int nPos)
{
	if( 0 == this->m_nCOMMENTEND ){
		/* �N�H�[�e�[�V����������̏I�[�����邩 */
		this->m_nCOMMENTEND = m_pCEditDoc->m_cLayoutMgr.Match_Quote( m_cQuote, nPos, cStr );
	}
	else if( nPos == this->m_nCOMMENTEND ){
		return true;
	}

	return false;
}
