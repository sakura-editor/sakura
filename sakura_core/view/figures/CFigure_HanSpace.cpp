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
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 新しい形式の半角スペース描画
void _DispHanSpaceNew( CGraphics& gr, DispPos* pDispPos, CEditView* pcView )
{
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,1))
	{
		//背景
		gr.SetBrushColor(GetBkColor(gr));
		::FillRect(gr,&rc,gr.GetCurrentBrush());
	
		//四角形
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

	//位置進める
	pDispPos->ForwardDrawCol(1);
}

//! 古い形式の半角スペース描画
void _DispHanSpaceOld( CGraphics& gr, DispPos* pDispPos, CEditView* pcView )
{
	//クリッピング矩形を計算。画面外なら描画しない
	CMyRect rcClip;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip,*pDispPos,1))
	{
		// 色決定
		CTypeSupport cSupport(pcView,pcView->GetTextDrawer()._GetColorIdx(COLORIDX_SPACE));
		cSupport.SetGraphicsState_WhileThisObj(gr);
		
		//小文字"o"の下半分を出力
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
		
		//上半分は普通の空白で出力（"o"の上半分を消す）
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

	//位置進める
	pDispPos->ForwardDrawCol(1);
}
