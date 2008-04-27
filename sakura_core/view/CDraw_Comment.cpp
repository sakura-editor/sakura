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

	// 行コメント
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cLineComment.Match( GetNPos(), pInfo->nLineLen, pInfo->pLine )	//@@@ 2002.09.22 YAZAKI
	){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_COMMENT);	// 行コメント
		return true;
	}
	return false;
}




bool CDraw_BlockComment::EnterColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	// ブロックコメント
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, pInfo->nPos, pInfo->nLineLen, pInfo->pLine )	//@@@ 2002.09.22 YAZAKI
	){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_BLOCK1);	/* ブロックコメント1である */ // 2002/03/13 novice

		/* この物理行にブロックコメントの終端があるか */	//@@@ 2002.09.22 YAZAKI
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
		pInfo->ChangeColor(COLORIDX_BLOCK2);	/* ブロックコメント2である */ // 2002/03/13 novice

		/* この物理行にブロックコメントの終端があるか */
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
		/* この物理行にブロックコメントの終端があるか */
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
		/* この物理行にブロックコメントの終端があるか */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, pInfo->nPos, pInfo->nLineLen, pInfo->pLine );	//@@@ 2002.09.22 YAZAKI
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_TEXT); // 2002/03/13 novice
		return true;
	}
	return false;
}
