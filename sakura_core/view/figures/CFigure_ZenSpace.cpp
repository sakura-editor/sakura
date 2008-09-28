#include "stdafx.h"
#include "CFigure_ZenSpace.h"
#include "types/CTypeSupport.h"

#ifdef NEW_ZENSPACE
#define _DispZenkakuSpace _DispZenkakuSpaceNew
#else
#define _DispZenkakuSpace _DispZenkakuSpaceOld
#endif

void _DispZenkakuSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView );


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

CLayoutInt CFigure_ZenSpace::GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const
{
	return CLayoutInt(2);
}

bool CFigure_ZenSpace::DrawImp(SColorStrategyInfo* pInfo)
{
	_DispZenkakuSpace(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �V�����`���̑S�p�X�y�[�X�`��
void _DispZenkakuSpaceNew( CGraphics& gr, DispPos* pDispPos, CEditView* pcView )
{
	const CTextArea& cArea = pcView->GetTextArea();
	//�N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	RECT rcChar;
	cArea.GenerateCharRect(&rcChar,*pDispPos,2);
	RECT rcClipping = rcChar;
	if(cArea.TrimRectByArea(&rcClipping)) //###############�{���͂����ŃN���b�s���O�v�Z�Ȃ񂩂��Ȃ��Ă��ǂ�
	{
		gr.PushClipping(rcClipping);

		//�w�i
		gr.SetBrushColor(::GetBkColor(gr));
		::FillRect(gr,&rcChar,gr.GetCurrentBrush());
		
		//�l�p�`
		COLORREF c = ::GetTextColor(gr);
		rcChar.left+=1;
		rcChar.top+=1;
		rcChar.right-=1;
		rcChar.bottom-=1;
		for(int x=rcChar.left+1;x<rcChar.right-1;x+=2){
			ApiWrap::SetPixelSurely(gr,x,rcChar.top,c);
			ApiWrap::SetPixelSurely(gr,x,rcChar.bottom-1,c);
		}
		for(int y=rcChar.top+1;y<rcChar.bottom-1;y+=2){
			ApiWrap::SetPixelSurely(gr,rcChar.left,y,c);
			ApiWrap::SetPixelSurely(gr,rcChar.right-1,y,c);
		}

		gr.PopClipping();
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(2);
}

//! �Â��`���̑S�p�X�y�[�X�`��
void _DispZenkakuSpaceOld( CGraphics& gr, DispPos* pDispPos, CEditView* pcView )
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
			ExtTextOutOption(),
			&rc,
			szZenSpace,
			wcslen(szZenSpace),
			pcView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(2);
}
