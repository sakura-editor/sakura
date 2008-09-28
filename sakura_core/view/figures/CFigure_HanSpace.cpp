#include "stdafx.h"
#include "CFigure_HanSpace.h"
#include "types/CTypeSupport.h"

#ifdef NEW_ZENSPACE
#define _DispHanSpace _DispHanSpaceNew
#else
#define _DispHanSpace _DispHanSpaceOld
#endif

void _DispHanSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView );

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

CLayoutInt CFigure_HanSpace::GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const
{
	return CLayoutInt(1);
}

bool CFigure_HanSpace::DrawImp(SColorStrategyInfo* pInfo)
{
	_DispHanSpace(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �V�����`���̔��p�X�y�[�X�`��
void _DispHanSpaceNew( CGraphics& gr, DispPos* pDispPos, CEditView* pcView )
{
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,1))
	{
		//�w�i
		gr.SetBrushColor(GetBkColor(gr));
		::FillRect(gr,&rc,gr.GetCurrentBrush());
	
		//�l�p�`
		COLORREF c = ::GetTextColor(gr);
		rc.left+=1;
		rc.top+=1;
		rc.right-=1;
		rc.bottom-=1;
		for(int x=rc.left+1;x<rc.right-1;x+=2){
			ApiWrap::SetPixelSurely(gr,x,rc.bottom-1,c);
		}
		for(int y=(rc.top+rc.bottom)/2+1;y<rc.bottom-1;y+=2){
			ApiWrap::SetPixelSurely(gr,rc.left,y,c);
			ApiWrap::SetPixelSurely(gr,rc.right-1,y,c);
		}
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(1);
}

//! �Â��`���̔��p�X�y�[�X�`��
void _DispHanSpaceOld( CGraphics& gr, DispPos* pDispPos, CEditView* pcView )
{
	//�N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	CMyRect rcClip;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip,*pDispPos,1))
	{
		// �F����
		CTypeSupport cSupport(pcView,pcView->GetTextDrawer()._GetColorIdx(COLORIDX_SPACE));
		cSupport.SetGraphicsState_WhileThisObj(gr);
		
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
