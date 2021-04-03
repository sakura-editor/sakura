/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo

#include "CFigure_HanSpace.h"
#include "types/CTypeSupport.h"
#include "apiwrap/StdApi.h"

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
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 半角スペース描画
void CFigure_HanSpace::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	//クリッピング矩形を計算。画面外なら描画しない
	CMyRect rcClip;
	const int Dx = pcView->GetTextMetrics().CalcTextWidth3(L" ", 1);
	const CLayoutXInt nCol = CLayoutXInt(Dx);
	if(pcView->GetTextArea().GenerateClipRect(&rcClip,*pDispPos,nCol))
	{
		//小文字"o"の下半分を出力
		CMyRect rcClipBottom=rcClip;
		rcClipBottom.top=rcClip.top+pcView->GetTextMetrics().GetHankakuHeight()/2;
		::ExtTextOut(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClipBottom,
//FIXME:幅が違う
			L"o",
			1,
			&Dx
		);
		
		//上半分は普通の空白で出力（"o"の上半分を消す）
		CMyRect rcClipTop=rcClip;
		rcClipTop.bottom=rcClip.top+pcView->GetTextMetrics().GetHankakuHeight()/2;
		::ExtTextOut(
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

	//位置進める
	pDispPos->ForwardDrawCol(nCol);
}
