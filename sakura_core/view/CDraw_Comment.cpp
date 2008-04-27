#include "stdafx.h"
#include "CDraw_Comment.h"

#define SetNPos(N) pInfo->nPos=(N)
#define GetNPos() (pInfo->nPos+CLogicInt(0))

#define SetNBgn(N) pInfo->nBgn=(N)
#define GetNBgn() (pInfo->nBgn+0)



bool CDraw_LineComment::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	// �s�R�����g
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cLineComment.Match( GetNPos(), pInfo->nLineLen, pInfo->pLine )	//@@@ 2002.09.22 YAZAKI
	){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_COMMENT);	// �s�R�����g
		return true;
	}
	return false;
}




bool CDraw_BlockComment::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	// �u���b�N�R�����g
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, pInfo->nPos, pInfo->nLineLen, pInfo->pLine )	//@@@ 2002.09.22 YAZAKI
	){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_BLOCK1);	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice

		/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */	//@@@ 2002.09.22 YAZAKI
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo(
			0,
			pInfo->nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ),
			pInfo->nLineLen,
			pInfo->pLine
		);

		return true;
	}
	else if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, pInfo->nPos, pInfo->nLineLen, pInfo->pLine )	//@@@ 2002.09.22 YAZAKI
	){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_BLOCK2);	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice

		/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, pInfo->nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), pInfo->nLineLen, pInfo->pLine );	//@@@ 2002.09.22 YAZAKI

		return true;
	}
	return false;
}


bool CDraw_BlockCommentEnd::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, pInfo->nPos, pInfo->nLineLen, pInfo->pLine );	//@@@ 2002.09.22 YAZAKI
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_TEXT); // 2002/03/13 novice
		return true;
	}
	return false;
}


bool CDraw_BlockCommentEnd2::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, pInfo->nPos, pInfo->nLineLen, pInfo->pLine );	//@@@ 2002.09.22 YAZAKI
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_TEXT); // 2002/03/13 novice
		return true;
	}
	return false;
}
