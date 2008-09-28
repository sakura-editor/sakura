#include "stdafx.h"
#include "CColor_Eol.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"

bool CColor_Eol::BeginColor(const CStringRef& cStr, int nPos)
{
	return false;
	/*
	###########################################################
	const CLayout*	pcLayout2; //���[�N�pCLayout�|�C���^
	pcLayout2 = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );
	int nLineHeight = pInfo->pcView->GetTextMetrics().GetHankakuDy();  //�s�̏c���H

	if(!cStr.IsValid()){
		if(pInfo->pDispPos->GetLayoutLineRef()==CEditDoc::GetInstance(0)->m_cLayoutMgr.GetLineCount()){
			return COLORIDX_EOL;
		}
		else{
			return false;
		}
	}
	else if( pInfo->nPosInLogic >= pInfo->pDispPos->GetLayoutRef()->GetDocLineRef()->GetLengthWithoutEOL() ){
		return COLORIDX_EOL;
	}
	return false;
	*/
}

bool CColor_Eol::EndColor(const CStringRef& cStr, int nPos)
{
	if(nPos>=cStr.GetLength()){
		return true;
	}
	return false;
}
