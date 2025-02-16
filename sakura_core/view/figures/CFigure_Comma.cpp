/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigure_Comma.h"
#include "types/CTypeSupport.h"
#include "apiwrap/StdApi.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CFigure_Comma                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_Comma::Match(const wchar_t* pText, int nTextLen) const
{
	if ( pText[0] == L',' ) {
		return true;
	}
	return false;
}

bool CFigure_Comma::Disp(void) const
{
	return m_pTypeData->m_nTsvMode == TSV_MODE_CSV;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! カンマ描画
*/
void CFigure_Comma::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	DispPos& sPos=*pDispPos;

	//必要なインターフェース
	const CLayoutMgr& pcLayoutMgr = pcView->GetDocument()->m_cLayoutMgr;
	const CTextMetrics* pMetrics=&pcView->GetTextMetrics();
	const CTextArea* pArea=&pcView->GetTextArea();

	int nLineHeight = pMetrics->GetHankakuDy();

	CTypeSupport cTabType(pcView,COLORIDX_TAB);

	// これから描画するタブ幅
	CLayoutXInt nTabLayoutWidth = pcLayoutMgr.GetActualTsvSpace(sPos.GetDrawCol(), L',');
	size_t nTabDispWidth = static_cast<Int>(nTabLayoutWidth) / pcLayoutMgr.GetWidthPerKeta();
	if( pcView->m_bMiniMap ){
		CLayoutMgr mgrTemp;
		mgrTemp.SetTabSpaceInfo(pcLayoutMgr.GetTabSpaceKetas(), static_cast<CLayoutXInt>(pMetrics->GetHankakuWidth()));
		nTabLayoutWidth = mgrTemp.GetActualTabSpace(sPos.GetDrawCol());
		nTabDispWidth = static_cast<Int>(nTabLayoutWidth) / mgrTemp.GetWidthPerKeta();
	}

	// タブ記号領域
	RECT rcClip2;
	rcClip2.left = sPos.GetDrawPos().x;
	rcClip2.right = rcClip2.left + static_cast<Int>(nTabLayoutWidth);
	if( rcClip2.left < pArea->GetAreaLeft() ){
		rcClip2.left = pArea->GetAreaLeft();
	}
	rcClip2.top = sPos.GetDrawPos().y;
	rcClip2.bottom = sPos.GetDrawPos().y + nLineHeight;

	if( pArea->IsRectIntersected(rcClip2) ){
		if( cTabType.IsDisp() ){	//CSVモード
			std::wstring szViewString = L",";
			if (szViewString.length() < nTabDispWidth) {
				szViewString.append(nTabDispWidth - szViewString.length(), L' ');
			}
			const INT* lpDx;
			if( szViewString.length() > 64 ) {
				static std::vector<int> anHankakuDx; //!< 半角用文字間隔配列
				anHankakuDx.resize(szViewString.length(), pMetrics->GetHankakuDx());
				lpDx = &anHankakuDx[0];
			}else {
				lpDx = pMetrics->GetDxArray_AllHankaku();
			}
			::ExtTextOut(
				gr,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
				&rcClip2,
				szViewString.c_str(),
				static_cast<UINT>(szViewString.length()),
				lpDx
			);
		}
	}

	//Xを進める
	sPos.ForwardDrawCol(nTabLayoutWidth);
}
