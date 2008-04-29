#include "stdafx.h"
#include "CDraw_Comment.h"

#define SetNPos(N) pInfo->nPos=(N)
#define GetNPos() (pInfo->nPos+CLogicInt(0))

#define SetNBgn(N) pInfo->nBgn=(N)
#define GetNBgn() (pInfo->nBgn+0)



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDraw_LineComment::BeginColor(SDrawStrategyInfo* pInfo)
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

bool CDraw_LineComment::GetColorIndexImp(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cLineComment.Match( pInfo->nPos, pInfo->nLineLen, pInfo->pLine )	//@@@ 2002.09.22 YAZAKI
	){
		pInfo->nBgn = pInfo->nPos;

		pInfo->nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice

		/* 現在の色を指定 */
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		return true;
	}
	return false;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント１                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDraw_BlockComment::BeginColor(SDrawStrategyInfo* pInfo)
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
	return false;
}

bool CDraw_BlockComment::EndColor(SDrawStrategyInfo* pInfo)
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

bool CDraw_BlockComment::GetColorIndexImp(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, pInfo->nPos, pInfo->nLineLen, pInfo->pLine )	//@@@ 2002.09.22 YAZAKI
	){
		pInfo->nBgn = pInfo->nPos;
		pInfo->nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice

		/* 現在の色を指定 */
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		/* この物理行にブロックコメントの終端があるか */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, pInfo->nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ), pInfo->nLineLen, pInfo->pLine );	//@@@ 2002.09.22 YAZAKI

		return true;
	}
	return false;
}

bool CDraw_BlockComment::GetColorIndexImpEnd(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* この物理行にブロックコメントの終端があるか */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, pInfo->nPos, pInfo->nLineLen, pInfo->pLine );	//@@@ 2002.09.22 YAZAKI
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->nBgn = pInfo->nPos;
		pInfo->nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		/* 現在の色を指定 */
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント２                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDraw_BlockComment2::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	// ブロックコメント
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
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

bool CDraw_BlockComment2::EndColor(SDrawStrategyInfo* pInfo)
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

bool CDraw_BlockComment2::GetColorIndexImp(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, pInfo->nPos, pInfo->nLineLen, pInfo->pLine )	//@@@ 2002.09.22 YAZAKI
	){
		pInfo->nBgn = pInfo->nPos;
		pInfo->nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
		/* 現在の色を指定 */
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		/* この物理行にブロックコメントの終端があるか */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, pInfo->nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), pInfo->nLineLen, pInfo->pLine );	//@@@ 2002.09.22 YAZAKI

		return true;
	}
	return false;
}

bool CDraw_BlockComment2::GetColorIndexImpEnd(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* この物理行にブロックコメントの終端があるか */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, pInfo->nPos, pInfo->nLineLen, pInfo->pLine );	//@@@ 2002.09.22 YAZAKI
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->nBgn = pInfo->nPos;
		pInfo->nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		/* 現在の色を指定 */
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		return true;
	}
	return false;
}
