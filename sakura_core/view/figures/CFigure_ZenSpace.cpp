#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigure_ZenSpace.h"
#include "types/CTypeSupport.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigure_ZenSpace                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_ZenSpace::Match(const wchar_t* pText) const
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(pText[0] == L'�@' && TypeDataPtr->m_ColorInfoArr[COLORIDX_ZENSPACE].m_bDisp){
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �S�p�X�y�[�X�`��
void CFigure_ZenSpace::DispSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans ) const
{
	//�N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,2))
	{
		//�`��
		const wchar_t* szZenSpace =
			CTypeSupport(pcView,COLORIDX_ZENSPACE).IsDisp()?L"��":L"�@";
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			szZenSpace,
			wcslen(szZenSpace),
			pcView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(2);
}
