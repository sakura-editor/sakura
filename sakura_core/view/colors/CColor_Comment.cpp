#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Comment.h"
#include "doc/layout/CLayout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �s�R�����g                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_LineComment::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	// �s�R�����g
	if( m_pTypeData->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		m_pTypeData->m_cLineComment.Match( nPos, cStr )	//@@@ 2002.09.22 YAZAKI
	){
		return true;
	}
	return false;
}

bool CColor_LineComment::EndColor(const CStringRef& cStr, int nPos)
{
	//������I�[
	if( nPos >= cStr.GetLength() ){
		return true;
	}

	//���s
	if( WCODE::IsLineDelimiter(cStr.At(nPos)) ){
		return true;
	}

	return false;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �u���b�N�R�����g�P                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_BlockComment::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	// �u���b�N�R�����g
	if( m_pTypeData->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		m_pcBlockComment->Match_CommentFrom( nPos, cStr )	//@@@ 2002.09.22 YAZAKI
	){
		/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */	//@@@ 2002.09.22 YAZAKI
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
		/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
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

