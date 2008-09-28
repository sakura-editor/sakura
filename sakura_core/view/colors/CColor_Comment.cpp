#include "stdafx.h"
#include "CColor_Comment.h"
#include "doc/CLayout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_LineComment::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	// 行コメント
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cLineComment.Match( nPos, cStr.GetLength(), cStr.GetPtr() )	//@@@ 2002.09.22 YAZAKI
	){
		return true;
	}
	return false;
}

bool CColor_LineComment::EndColor(const CStringRef& cStr, int nPos)
{
	//文字列終端
	if( nPos >= cStr.GetLength() ){
		return true;
	}

	//改行
	if( WCODE::IsLineDelimiter(cStr.At(nPos)) ){
		return true;
	}

	return false;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント１                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_BlockComment::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	// ブロックコメント
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cBlockComments[m_nType].Match_CommentFrom( nPos, cStr )	//@@@ 2002.09.22 YAZAKI
	){
		/* この物理行にブロックコメントの終端があるか */	//@@@ 2002.09.22 YAZAKI
		this->m_nCOMMENTEND = TypeDataPtr->m_cBlockComments[m_nType].Match_CommentTo(
			nPos + wcslen( TypeDataPtr->m_cBlockComments[m_nType].getBlockCommentFrom() ), //$$note:高速化：毎回wcslen呼ばなくても…
			cStr
		);

		return true;
	}
	return false;
}

bool CColor_BlockComment::EndColor(const CStringRef& cStr, int nPos)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == this->m_nCOMMENTEND ){
		/* この物理行にブロックコメントの終端があるか */
		this->m_nCOMMENTEND = TypeDataPtr->m_cBlockComments[m_nType].Match_CommentTo(
			0,
			cStr
		);
	}
	else if( nPos >= this->m_nCOMMENTEND ){
		return true;
	}
	return false;
}

