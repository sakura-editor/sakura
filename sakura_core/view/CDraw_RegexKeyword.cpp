#include "stdafx.h"
#include "CDraw_RegexKeyword.h"

#define SetNPos(N) pInfo->nPos=(N)
#define GetNPos() (pInfo->nPos+CLogicInt(0))

#define SetNBgn(N) pInfo->nBgn=(N)
#define GetNBgn() (pInfo->nBgn+0)


bool CDraw_RegexKeyword::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int		nMatchLen;
	int		nMatchColor;

	//正規表現キーワード
	if( TypeDataPtr->m_bUseRegexKeyword
	 && pInfo->pcView->m_cRegexKeyword->RegexIsKeyword( pInfo->pLine, pInfo->nPos, pInfo->nLineLen, &nMatchLen, &nMatchColor )
	){
		pInfo->DrawToHere();
		/* 現在の色を指定 */
		pInfo->ChangeColor(MakeColorIndexType_RegularExpression(nMatchColor));	/* 色指定 */	//@@@ 2002.01.04 upd
		pInfo->nCOMMENTEND = pInfo->nPos + nMatchLen;  /* キーワード文字列の終端をセットする */
		return true;
	}
	return false;
}


bool CDraw_RegexKeywordEnd::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_TEXT); // 2002/03/13 novice
		return true;
	}
	return false;
}

