#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Comment.h"
#include "doc/layout/CLayout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_LineComment::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	// 行コメント
	if( m_pTypeData->m_cLineComment.Match( nPos, cStr )	//@@@ 2002.09.22 YAZAKI
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
	if( WCODE::IsLineDelimiter(cStr.At(nPos), GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
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

	// ブロックコメント
	if( m_pcBlockComment->Match_CommentFrom( nPos, cStr )	//@@@ 2002.09.22 YAZAKI
	){
		/* この物理行にブロックコメントの終端があるか */	//@@@ 2002.09.22 YAZAKI
		this->m_nCOMMENTEND = m_pcBlockComment->Match_CommentTo(
			nPos + m_pcBlockComment->getBlockFromLen(),
			cStr
		);

		return true;
	}
	return false;
}

bool CColor_BlockComment::EndColor(const CStringRef& cStr, int nPos)
{
	if( 0 == this->m_nCOMMENTEND ){
		/* この物理行にブロックコメントの終端があるか */
		this->m_nCOMMENTEND = m_pcBlockComment->Match_CommentTo(
			nPos,
			cStr
		);
	}
	else if( nPos == this->m_nCOMMENTEND ){
		return true;
	}
	return false;
}

