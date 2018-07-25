#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigure_CtrlCode.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_CtrlCode                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_CtrlCode::Match(const wchar_t* pText, int nTextLen) const
{
	//当面はASCII制御文字（C0 Controls, IsHankaku()で半角扱い）だけを制御文字表示にする
	//そうしないと IsHankaku(0x0600)==false なのに iswcntrl(0x0600)!=0 のようなケースで表示桁がずれる
	//U+0600: ARABIC NUMBER SIGN
	if(WCODE::IsControlCode(pText[0])){
		return true;
	}
	return false;
}

bool CFigure_CtrlCode::DrawImp(SColorStrategyInfo* pInfo)
{
	bool bTrans = DrawImp_StyleSelect(pInfo);
	DispPos sPos(*pInfo->m_pDispPos);	// 現在位置を覚えておく
	int width = pInfo->m_pcView->GetTextMetrics().CalcTextWidth3(&pInfo->m_pLineOfLogic[pInfo->GetPosInLogic()], 1);
	DispSpaceEx(pInfo->m_gr, pInfo->m_pDispPos,pInfo->m_pcView, bTrans, width);	// 空白描画
	DrawImp_StylePop(pInfo);
	DrawImp_DrawUnderline(pInfo, sPos);
	// 1文字前提
	pInfo->m_nPosInLogic += CNativeW::GetSizeOfChar(	// 行末以外はここでスキャン位置を１字進める
		pInfo->m_pLineOfLogic,
		pInfo->GetDocLine()->GetLengthWithoutEOL(),
		pInfo->GetPosInLogic()
		);
	return true;
}

void CFigure_CtrlCode::DispSpaceEx(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans, int width) const
{
	int dx[1];
	dx[0] = width;

	RECT rc;
	//クリッピング矩形を計算。画面外なら描画しない
	if(pcView->GetTextArea().GenerateClipRect(&rc, *pDispPos, CHabaXInt(dx[0])))
	{
		//描画
		int fontNo = WCODE::GetFontNo(GetAlternateChar());
		if( fontNo ){
			SFONT sFont;
			sFont.m_sFontAttr = gr.GetCurrentMyFontAttr();
			sFont.m_hFont = pcView->GetFontset().ChooseFontHandle(fontNo, sFont.m_sFontAttr);
			gr.PushMyFont(sFont);
		}
		int nHeightMargin = pcView->GetTextMetrics().GetCharHeightMarginByFontNo(fontNo);
		wchar_t wc[1] = {GetAlternateChar()};
		ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y + nHeightMargin,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			wc,
			1,
			dx
		);
		if( fontNo ){
			gr.PopMyFont();
		}
	}
	pDispPos->ForwardDrawCol(CLayoutXInt(dx[0]));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_HanBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_HanBinary::Match(const wchar_t* pText, int nTextLen) const
{
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_ZenBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_ZenBinary::Match(const wchar_t* pText, int nTextLen) const
{
	return false;
}
