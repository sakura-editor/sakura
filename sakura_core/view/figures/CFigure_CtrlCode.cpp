#include "stdafx.h"
#include "CFigure_CtrlCode.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_CtrlCode                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_CtrlCode::Match(const wchar_t* pText) const
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(WCODE::IsControlCode(pText[0]) && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp){
		return true;
	}
	return false;
}

CLayoutInt CFigure_CtrlCode::GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const
{
	return CLayoutInt(1);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! コントロールコード描画
void CFigure_CtrlCode::DispSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView ) const
{
	//クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,1))
	{
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rc,
			L"･",
			1,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	//位置進める
	pDispPos->ForwardDrawCol(1);
}
