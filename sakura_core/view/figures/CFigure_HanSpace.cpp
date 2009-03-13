#include "stdafx.h"
#include "CFigure_HanSpace.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_HanSpace                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_HanSpace::Match(const wchar_t* pText) const
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(pText[0] == L' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp){
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ���p�X�y�[�X�`��
void CFigure_HanSpace::DispSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView ) const
{
	//�N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	CMyRect rcClip;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip,*pDispPos,1))
	{
		//������"o"�̉��������o��
		CMyRect rcClipBottom=rcClip;
		rcClipBottom.top=rcClip.top+rcClip.Height()/2;
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClipBottom,
			L"o",
			1,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
		
		//�㔼���͕��ʂ̋󔒂ŏo�́i"o"�̏㔼���������j
		CMyRect rcClipTop=rcClip;
		rcClipTop.bottom=rcClip.top+rcClip.Height()/2;
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClipTop,
			L" ",
			1,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(1);
}
