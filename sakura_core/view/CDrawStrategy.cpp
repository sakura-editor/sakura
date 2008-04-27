#include "stdafx.h"
#include "CDrawStrategy.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"

#define SetNPos(N) pInfo->nPos=(N)
#define GetNPos() (pInfo->nPos+CLogicInt(0))

#define SetNBgn(N) pInfo->nBgn=(N)
#define GetNBgn() (pInfo->nBgn+0)



bool CDraw_URL::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int	nUrlLen;
	
	if( pInfo->bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URLを表示する */
	 && IsURL( &pInfo->pLine[pInfo->nPos], pInfo->nLineLen - pInfo->nPos, &nUrlLen )	/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
	){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_URL);	/* URLモード */ // 2002/03/13 novice
		pInfo->nCOMMENTEND = pInfo->nPos + nUrlLen;
		return true;
	}
	return false;
}


bool CDraw_Numeric::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int	nnn;
	
	if( pInfo->bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
		&& (nnn = IsNumber( pInfo->pLine, pInfo->nPos, pInfo->nLineLen )) > 0 )		/* 半角数字を表示する */
	{
		/* キーワード文字列の終端をセットする */
		int i = pInfo->nPos + nnn;
		pInfo->DrawToHere();
		/* 現在の色を指定 */
		pInfo->ChangeColor(COLORIDX_DIGIT);	/* 半角数値である */ // 2002/03/13 novice
		pInfo->nCOMMENTEND = i;
		return true;
	}
	return false;
}

bool CDraw_CtrlCode::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* コントロールコードを色分け */
		&& COLORIDX_CTRLCODE != pInfo->nCOMMENTMODE // 2002/03/13 novice
		&&WCODE::isControlCode(pInfo->pLine[pInfo->nPos]))
	{
		pInfo->DrawToHere();

		pInfo->nCOMMENTMODE_OLD = pInfo->nCOMMENTMODE;
		pInfo->nCOMMENTEND_OLD = pInfo->nCOMMENTEND;

		pInfo->ChangeColor(COLORIDX_CTRLCODE);
		/* コントロールコード列の終端を探す */
		int i;
		for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
			if(!WCODE::isControlCode(pInfo->pLine[i])){
				break;
			}
		}
		pInfo->nCOMMENTEND = i;
		pInfo->nCharChars = CLogicInt(1);
		return true;
	}

	return false;
}

bool CDraw_CtrlColorEnd::EnterColor(SDrawStrategyInfo* pInfo)
{
	if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->DrawToHere();
		pInfo->ChangeColor(pInfo->nCOMMENTMODE_OLD);
		pInfo->nCOMMENTEND = pInfo->nCOMMENTEND_OLD;
		return true;
	}
	return false;
}

bool CDraw_KeywordSet::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	
	if( pInfo->bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* キーワードセット */
		TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* 強調キーワードを表示する */ // 2002/03/13 novice
		IS_KEYWORD_CHAR( pInfo->pLine[pInfo->nPos] )
	){
		//	Mar 4, 2001 genta comment out
		//	pInfo->bKeyWordTop = false;
		/* キーワード文字列の終端を探す */
		int i;
		for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
			if( IS_KEYWORD_CHAR( pInfo->pLine[i] ) ){
			}else{
				break;
			}
		}
		/* キーワードが登録単語ならば、色を変える */
		int j = i - pInfo->nPos;
		/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
		int nIdx = GetDllShareData().m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
			TypeDataPtr->m_nKeyWordSetIdx[0],
			&pInfo->pLine[pInfo->nPos],
			j
		);
		if( nIdx != -1 ){
			pInfo->DrawToHere();

			/* 現在の色を指定 */
			pInfo->ChangeColor(COLORIDX_KEYWORD1);	/* 強調キーワード1 */ // 2002/03/13 novice
			pInfo->nCOMMENTEND = i;
		}
		else{		//MIK START ADD 2000.12.01 second keyword & binary search
			// 2005.01.13 MIK 強調キーワード数追加に伴う配列化
			for( int my_i = 1; my_i < 10; my_i++ )
			{
				if(TypeDataPtr->m_nKeyWordSetIdx[ my_i ] != -1 && /* キーワードセット */							//MIK 2000.12.01 second keyword
					TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + my_i].m_bDisp)									//MIK
				{																							//MIK
					/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */						//MIK
					nIdx = GetDllShareData().m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
						TypeDataPtr->m_nKeyWordSetIdx[ my_i ] ,													//MIK
						&pInfo->pLine[pInfo->nPos],																		//MIK
						j																					//MIK
					);																						//MIK
					if( nIdx != -1 ){																		//MIK
						pInfo->DrawToHere();
						/* 現在の色を指定 */																//MIK
						pInfo->ChangeColor((EColorIndexType)(COLORIDX_KEYWORD1 + my_i));	/* 強調キーワード2 */ // 2002/03/13 novice		//MIK
						pInfo->nCOMMENTEND = i;																	//MIK
						break;
					}																						//MIK
				}																							//MIK
				else
				{
					if(TypeDataPtr->m_nKeyWordSetIdx[my_i] == -1 )
						break;
				}
			}
		}			//MIK END
		return true;
	}
	return false;
}



bool CDraw_ColorEnd::EnterColor(SDrawStrategyInfo* pInfo)
{
	if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_TEXT); // 2002/03/13 novice
		return true;
	}
	return false;
}





bool CDraw_LineEnd::EnterColor(SDrawStrategyInfo* pInfo)
{
	CTypeSupport cTextType(pInfo->pcView,COLORIDX_TEXT);
	const CLayout*	pcLayout2; //ワーク用CLayoutポインタ
	pcLayout2 = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );
	int nLineHeight = pInfo->pcView->GetTextMetrics().GetHankakuDy();  //行の縦幅？

	if( pInfo->nPos >= pInfo->nLineLen - pcLayout2->GetLayoutEol().GetLen() ){
		if( pInfo->DrawToHere() ){
			pInfo->nBgn = pInfo->nPos + 1;

			// 行末背景描画
			RECT rcClip;
			if(pInfo->pcView->GetTextArea().GenerateClipRectRight(&rcClip,*pInfo->pDispPos)){
				cTextType.FillBack(pInfo->hdc,rcClip);
			}

			// 改行記号の表示
			pInfo->pcView->GetTextDrawer().DispEOL(pInfo->hdc,pInfo->pDispPos,pcLayout2->GetLayoutEol(),pInfo->bSearchStringMode);

			// 2006.04.29 Moca 選択処理のため縦線処理を追加
			pInfo->pcView->GetTextDrawer().DispVerticalLines(
				pInfo->hdc,
				pInfo->pDispPos->GetDrawPos().y,
				pInfo->pDispPos->GetDrawPos().y + nLineHeight,
				CLayoutInt(0),
				CLayoutInt(-1)
			);
			if( pInfo->pcView->GetSelectionInfo().IsTextSelected() ){
				/* テキスト反転 */
				pInfo->pcView->DispTextSelected(
					pInfo->hdc,
					pInfo->pDispPos->GetLayoutLineRef(),
					CMyPoint(pInfo->sDispPosBegin.GetDrawPos().x, pInfo->pDispPos->GetDrawPos().y),
					pInfo->pDispPos->GetDrawCol()
				);
			}
		}

		return true; //goto end_of_line;
	}
	return false;
}

