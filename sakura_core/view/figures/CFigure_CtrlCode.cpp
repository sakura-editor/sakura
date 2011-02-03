#include "stdafx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigure_CtrlCode.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_CtrlCode                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_CtrlCode::Match(const wchar_t* pText) const
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp){
		//当面はASCII制御文字（C0 Controls, IsHankaku()で半角扱い）だけを制御文字表示にする
		//そうしないと IsHankaku(0x0600)==false なのに iswcntrl(0x0600)!=0 のようなケースで表示桁がずれる
		//U+0600: ARABIC NUMBER SIGN
		if(!(pText[0] & 0xFF80) && WCODE::IsControlCode(pText[0])){
			return true;
		}
	}
	return false;
}

void CFigure_CtrlCode::DispSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans ) const
{
	//クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,1))
	{
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"･",
			1,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	//位置進める
	pDispPos->ForwardDrawCol(1);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_HanBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_HanBinary::Match(const wchar_t* pText) const
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp){
		int nLen = pText[1]? 2:1;	// ※ pText は常に終端よりも手前
		if(CNativeW::GetKetaOfChar(pText, nLen, 0) == 1){	// 半角
			ECharSet e;
			CheckUtf16leChar(pText, nLen, &e, UC_NONCHARACTER);
			if(e == CHARSET_BINARY){
				return true;
			}
		}
	}
	return false;
}

void CFigure_HanBinary::DispSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans ) const
{
	//クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,1))
	{
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"〓",
			1,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	//位置進める
	pDispPos->ForwardDrawCol(1);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_ZenBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_ZenBinary::Match(const wchar_t* pText) const
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp){
		int nLen = pText[1]? 2:1;	// ※ pText は常に終端よりも手前
		if(CNativeW::GetKetaOfChar(pText, nLen, 0) > 1){	// 全角
			ECharSet e;
			CheckUtf16leChar(pText, nLen, &e, UC_NONCHARACTER);
			if(e == CHARSET_BINARY){
				return true;
			}
		}
	}
	return false;
}

void CFigure_ZenBinary::DispSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans ) const
{
	//クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,2))
	{
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"〓",
			1,
			pcView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
	}

	//位置進める
	pDispPos->ForwardDrawCol(2);
}
