#include "stdafx.h"
#include "CDraw_RegexKeyword.h"

#define SetNPos(N) pInfo->nPos=(N)
#define GetNPos() (pInfo->nPos+CLogicInt(0))

#define SetNBgn(N) pInfo->nBgn=(N)
#define GetNBgn() (pInfo->nBgn+0)


bool CDraw_RegexKeyword::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int		nMatchLen;
	int		nMatchColor;

	//���K�\���L�[���[�h
	if( TypeDataPtr->m_bUseRegexKeyword
	 && pInfo->pcView->m_cRegexKeyword->RegexIsKeyword( pInfo->pLine, pInfo->nPos, pInfo->nLineLen, &nMatchLen, &nMatchColor )
	){
		pInfo->DrawToHere();
		/* ���݂̐F���w�� */
		pInfo->ChangeColor(MakeColorIndexType_RegularExpression(nMatchColor));	/* �F�w�� */	//@@@ 2002.01.04 upd
		pInfo->nCOMMENTEND = pInfo->nPos + nMatchLen;  /* �L�[���[�h������̏I�[���Z�b�g���� */
		return true;
	}
	return false;
}


bool CDraw_RegexKeyword::EndColor(SDrawStrategyInfo* pInfo)
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

bool CDraw_RegexKeyword::GetColorIndexImp(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int		nMatchLen;
	int		nMatchColor;

	//���K�\���L�[���[�h
	if( TypeDataPtr->m_bUseRegexKeyword
	 && pInfo->pcView->m_cRegexKeyword->RegexIsKeyword( pInfo->pLine, pInfo->nPos, pInfo->nLineLen, &nMatchLen, &nMatchColor ) )
	{
		/* ���݂̐F���w�� */
		pInfo->nBgn = pInfo->nPos;
		pInfo->nCOMMENTMODE = MakeColorIndexType_RegularExpression(nMatchColor);	/* �F�w�� */	//@@@ 2002.01.04 upd
		pInfo->nCOMMENTEND = pInfo->nPos + nMatchLen;  /* �L�[���[�h������̏I�[���Z�b�g���� */
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );	//@@@ 2002.01.04
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		return true;
	}
	return false;
}

