#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigure_ZenSpace.h"
#include "types/CTypeSupport.h"

void Draw_ZenSpace( CGraphics& gr, const CMyRect& rc );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigure_ZenSpace                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_ZenSpace::Match(const wchar_t* pText, int nTextLen) const
{
	if( pText[0] == L'　' ){
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 全角スペース描画
void CFigure_ZenSpace::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	// 2010.09.21 PP用実装追加
	// プロポーショナルでは、全角SPと□の幅が違うことがある。違う場合は独自に描画
	CTypeSupport cZenSpace(pcView, COLORIDX_ZENSPACE);

	int dx[1];
	dx[0] = pcView->GetTextMetrics().CalcTextWidth3(L"　", 1);

	RECT rc;
	//クリッピング矩形を計算。画面外なら描画しない
	if(pcView->GetTextArea().GenerateClipRect(&rc, *pDispPos, CHabaXInt(dx[0])))
	{
		int u25a1Dx = pcView->GetTextMetrics().CalcTextWidth3(L"□", 1);
		bool bDrawMySelf = dx[0] != u25a1Dx;
		const wchar_t* pZenSp = (bDrawMySelf ? L"　" : L"□");
		int fontNo = WCODE::GetFontNo(*pZenSp);
		if( fontNo ){
			SFONT sFont;
			sFont.m_sFontAttr = gr.GetCurrentMyFontAttr();
			sFont.m_hFont = pcView->GetFontset().ChooseFontHandle(fontNo, sFont.m_sFontAttr);
			gr.PushMyFont(sFont);
		}
		int nHeightMargin = pcView->GetTextMetrics().GetCharHeightMarginByFontNo(fontNo);
		//描画
		ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y + nHeightMargin,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			pZenSp,
			1,
			dx
		);
		if( fontNo ){
			gr.PopMyFont();
		}
		if( bDrawMySelf ){
			gr.PushClipping(rc); // FIXME: 正確にはCombineRgn RGN_AND が必要
			
			// 全角SPの大きさ指定
			CMyRect rcZenSp;
			// 注：ベースライン無視
			rcZenSp.SetPos(pDispPos->GetDrawPos().x, pDispPos->GetDrawPos().y);
			rcZenSp.SetSize(dx[0]- pcView->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nColumnSpace,
				pcView->GetTextMetrics().GetHankakuHeight());

			// 描画
			// 文字色や太字かどうかを現在の DC から調べる	// 2009.05.29 ryoji 
			// （検索マッチ等の状況に柔軟に対応するため、ここは記号の色指定には決め打ちしない）
			Draw_ZenSpace(gr, rcZenSp);

			// リージョン破棄
			gr.PopClipping();
			
			// To Here 2003.08.17 ryoji 改行文字が欠けないように
		}
		
	}

	//位置進める
	pDispPos->ForwardDrawCol(CLayoutXInt(dx[0]));
}

void Draw_ZenSpace( CGraphics& gr, const CMyRect& rc )
{
	TEXTMETRIC tm;
	tm.tmAscent = 0;
	::GetTextMetrics(gr, &tm);
	// 正方形にする
	CMyRect rc2;
	int minWidth = std::max<int>(1, std::min<int>(tm.tmAscent, std::min<int>(rc.Height(), rc.Width())) - 2);
	minWidth -= (minWidth + 5) / 10;
	rc2.SetPos(
		rc.left + (rc.Width() - minWidth) / 2,
		rc.top  + tm.tmAscent - minWidth
	);
	rc2.SetSize(minWidth, minWidth);
	gr.PushPen(::GetTextColor(gr), 1);
	gr.DrawRect(rc2);
	gr.PopPen();
}
