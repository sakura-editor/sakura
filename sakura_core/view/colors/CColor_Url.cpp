#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Url.h"
#include "parse/CWordParse.h"
#include "doc/CEditDoc.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           URL                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_Url::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int	nUrlLen;
	
	if( _IsPosKeywordHead(cStr,nPos) && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URL��\������ */
	 && IsURL( cStr.GetPtr() + nPos, cStr.GetLength() - nPos, &nUrlLen )	/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
	){
		this->m_nCOMMENTEND = nPos + nUrlLen;
		return true;
	}
	return false;
}

bool CColor_Url::EndColor(const CStringRef& cStr, int nPos)
{
	if( nPos == this->m_nCOMMENTEND ){
		return true;
	}
	return false;
}

