#include "stdafx.h"
#include "CColor_RegexKeyword.h"


bool CColor_RegexKeyword::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int		nMatchLen;
	int		nMatchColor;

	const CEditView* pcView = &CEditWnd::Instance()->GetActiveView();

	//���K�\���L�[���[�h
	if( TypeDataPtr->m_bUseRegexKeyword
	 && pcView->m_cRegexKeyword->RegexIsKeyword( cStr, nPos, &nMatchLen, &nMatchColor )
	){
		this->m_nCOMMENTEND = nPos + nMatchLen;  /* �L�[���[�h������̏I�[���Z�b�g���� */
		return true; //#######MakeColorIndexType_RegularExpression(nMatchColor);
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

