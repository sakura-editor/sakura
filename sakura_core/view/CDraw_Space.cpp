#include "stdafx.h"
#include "CDraw_Space.h"

#define SetNPos(N) pInfo->nPos=(N)
#define GetNPos() (pInfo->nPos+CLogicInt(0))

#define SetNBgn(N) pInfo->nBgn=(N)
#define GetNBgn() (pInfo->nBgn+0)


bool CDraw_Tab::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	if( pInfo->pLine[pInfo->nPos] == WCODE::TAB ){
		if( pInfo->DrawToHere() ){
			// 色決定
			EColorIndexType	nColorIdx;
			if( pInfo->bSearchStringMode ){
				nColorIdx = COLORIDX_SEARCH;
			}else{
				nColorIdx = COLORIDX_TAB;
			}

			// タブ表示
			pInfo->pcView->GetTextDrawer().DispTab( pInfo->hdc, pInfo->pDispPos, nColorIdx );
		}
		pInfo->nBgn = pInfo->nPos + 1;
		pInfo->nCharChars = CLogicInt(1);
		return true;
	}
	return false;
}


bool CDraw_ZenSpace::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	if( WCODE::isZenkakuSpace(pInfo->pLine[pInfo->nPos]) && (pInfo->nCOMMENTMODE < 1000 || pInfo->nCOMMENTMODE > 1099) )
	{	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04
		if( pInfo->DrawToHere() ){
			// 全角空白を表示する
			pInfo->pcView->GetTextDrawer().DispZenkakuSpace(pInfo->hdc,pInfo->pDispPos,pInfo->bSearchStringMode);
		}
		//文字進める
		pInfo->nBgn = pInfo->nPos + 1;
		pInfo->nCharChars = CLogicInt(1);
		return true;
	}
	return false;
}


bool CDraw_HanSpace::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	if (pInfo->pLine[pInfo->nPos] == L' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp 
		 && (pInfo->nCOMMENTMODE < 1000 || pInfo->nCOMMENTMODE > 1099) )
	{
		if( pInfo->DrawToHere() ){
			pInfo->pcView->GetTextDrawer().DispHankakuSpace(pInfo->hdc,pInfo->pDispPos,pInfo->bSearchStringMode);
		}
		pInfo->nBgn = pInfo->nPos + 1;
		pInfo->nCharChars = CLogicInt(1);
		return true;
	}
	return false;
}

