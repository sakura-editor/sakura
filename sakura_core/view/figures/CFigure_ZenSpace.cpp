#include "stdafx.h"
#include "CFigure_ZenSpace.h"
#include "types/CTypeSupport.h"

void _DispZenkakuSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView );


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigure_ZenSpace                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_ZenSpace::Match(const wchar_t* pText) const
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(pText[0] == L'　' && TypeDataPtr->m_ColorInfoArr[COLORIDX_ZENSPACE].m_bDisp){
		return true;
	}
	return false;
}

CLayoutInt CFigure_ZenSpace::GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const
{
	return CLayoutInt(2);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 全角スペース描画
void CFigure_ZenSpace::DispSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView ) const
{
	//クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,2))
	{
		//描画
		const wchar_t* szZenSpace =
			CTypeSupport(pcView,COLORIDX_ZENSPACE).IsDisp()?L"□":L"　";
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

	//位置進める
	pDispPos->ForwardDrawCol(2);
}
