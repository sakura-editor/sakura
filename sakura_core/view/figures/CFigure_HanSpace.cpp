#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo

#include "CFigure_HanSpace.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_HanSpace                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_HanSpace::Match(const wchar_t* pText, int nTextLen) const
{
	if( pText[0] == L' ' ){
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ���p�X�y�[�X�`��
void CFigure_HanSpace::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	//�N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	CMyRect rcClip;
	const int Dx = pcView->GetTextMetrics().CalcTextWidth3(L" ", 1);
	const CLayoutXInt nCol = CLayoutXInt(Dx);
	if(pcView->GetTextArea().GenerateClipRect(&rcClip,*pDispPos,nCol))
	{
		//������"o"�̉��������o��
		CMyRect rcClipBottom=rcClip;
		rcClipBottom.top=rcClip.top+pcView->GetTextMetrics().GetHankakuHeight()/2;
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClipBottom,
//FIXME:�����Ⴄ
			L"o",
			1,
			&Dx
		);
		
		//�㔼���͕��ʂ̋󔒂ŏo�́i"o"�̏㔼���������j
		CMyRect rcClipTop=rcClip;
		rcClipTop.bottom=rcClip.top+pcView->GetTextMetrics().GetHankakuHeight()/2;
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClipTop,
			L" ",
			1,
			&Dx
		);
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(nCol);
}
