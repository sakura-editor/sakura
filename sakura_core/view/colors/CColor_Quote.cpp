#include "stdafx.h"
#include "CColor_Quote.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �N�H�[�e�[�V����                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_Quote::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( cStr.At(nPos) == m_cQuote &&
		TypeDataPtr->m_ColorInfoArr[this->GetStrategyColor()].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
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

	//�������؂�L���G�X�P�[�v���@ 0=[\"][\']
	if( TypeDataPtr->m_nStringType == 0 ){
		//�G�X�P�[�v����
		if(m_bLastEscape){
			m_bLastEscape = false;
			return false;
		}
		//�I�[
		if(cStr.At(nPos)==m_cQuote){
			m_bDone = true;
			return false;
		}
		//�G�X�P�[�v���o
		if(cStr.At(nPos)==L'\\'){
			m_bLastEscape = true;
		}
	}
	//�������؂�L���G�X�P�[�v���@ 1=[""]['']
	else if(TypeDataPtr->m_nStringType == 1){
		//�G�X�P�[�v����
		if(m_bLastEscape){
			m_bLastEscape = false;
			return false;
		}
		//�I�[
		if(cStr.At(nPos)==m_cQuote){
			//�G�X�P�[�v���o
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
	//�z��O
	else{
		assert(0);
	}

	return false;
}
