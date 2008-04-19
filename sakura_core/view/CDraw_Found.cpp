#include "stdafx.h"
#include "CDraw_Found.h"
#include "CTypeSupport.h"

#define SetNPos(N) pInfo->nPos=(N)
#define GetNPos() (pInfo->nPos+CLogicInt(0))

#define SetNBgn(N) pInfo->nBgn=(N)
#define GetNBgn() (pInfo->nBgn+0)



bool CDraw_Found::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const Types* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	CTypeSupport cSearchType(pInfo->pcView,COLORIDX_SEARCH);

	if( pInfo->pcView->m_bCurSrchKeyMark && cSearchType.IsDisp() ){
	}
	else{
		return false;
	}

top:
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//        検索ヒットフラグ設定 -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
	if(!pInfo->bSearchStringMode && (!pInfo->pcView->m_sCurSearchOption.bRegularExp || (pInfo->bSearchFlg && pInfo->nSearchStart < pInfo->nPos))){
		pInfo->bSearchFlg=pInfo->pcView->IsSearchString(
			pInfo->pLine,
			pInfo->nLineLen,
			pInfo->nPos,
			&pInfo->nSearchStart,
			&pInfo->nSearchEnd
		);
	}
	//マッチ文字列検出
	if( !pInfo->bSearchStringMode && pInfo->bSearchFlg && pInfo->nSearchStart==pInfo->nPos){
		// -- -- マッチ文字列直前までを描画 -- -- //

		pInfo->DrawToHere();
		pInfo->bSearchStringMode = true;

		// 現在の色を指定
		pInfo->pcView->SetCurrentColor( pInfo->hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
	}
	//マッチ文字列終了検出
	else if( pInfo->bSearchStringMode && pInfo->nSearchEnd <= pInfo->nPos ){ //+ == では行頭文字の場合、pInfo->nSearchEndも０であるために文字色の解除ができないバグを修正 2003.05.03 かろと
		// -- -- マッチ文字列を描画 -- -- //

		// 検索した文字列の終わりまできたら、文字色を標準に戻す処理
		pInfo->DrawToHere();
		/* 現在の色を指定 */
		pInfo->pcView->SetCurrentColor( pInfo->hdc, pInfo->nCOMMENTMODE );
		pInfo->bSearchStringMode = false;

		goto top;
	}
	return false;
}




bool CDraw_FoundEnd::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const Types* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	return false;
}
