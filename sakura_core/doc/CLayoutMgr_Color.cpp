#include "stdafx.h"
#include "CLayoutMgr.h"

// 2005.11.20 Mocaコメントの色分けがON/OFF関係なく行われていたバグを修正
bool CLayoutMgr::_CheckColorMODE(
	EColorIndexType*	_nCOMMENTMODE,	//!< [in/out]
	int*				_nCOMMENTEND,	//!< [in/out]
	int					nPos,
	int					nLineLen,
	const wchar_t*		pLine
)
{
	EColorIndexType&	nCOMMENTMODE = *_nCOMMENTMODE;
	int&				nCOMMENTEND  = *_nCOMMENTEND;

	switch( nCOMMENTMODE ){
	case COLORIDX_TEXT: // 2002/03/13 novice
		// 行コメント	// 2002/03/13 novice
		if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp && m_sTypeConfig.m_cLineComment.Match( nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_COMMENT;
		}
		// ブロックコメント1	// 2002/03/13 novice
		else if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp && m_sTypeConfig.m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK1;
			nCOMMENTEND = m_sTypeConfig.m_cBlockComment.Match_CommentTo( 0, nPos + m_sTypeConfig.m_cBlockComment.getBlockFromLen(0), nLineLen, pLine );	/* この物理行にブロックコメントの終端があるか */
		}
		// ブロックコメント2	// 2002/03/13 novice
		else if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&  m_sTypeConfig.m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK2;
			nCOMMENTEND = m_sTypeConfig.m_cBlockComment.Match_CommentTo( 1, nPos + m_sTypeConfig.m_cBlockComment.getBlockFromLen(1), nLineLen, pLine );	/* この物理行にブロックコメントの終端があるか */
		}
		// シングルクォーテーション文字列
		else if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp && pLine[nPos] == L'\'' ){
			nCOMMENTMODE = COLORIDX_SSTRING;
			nCOMMENTEND = Match_Quote( L'\'', nPos + 1, nLineLen, pLine );	/* シングルクォーテーション文字列の終端があるか */
		}
		// ダブルクォーテーション文字列
		else if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp && pLine[nPos] == L'"' ){
			nCOMMENTMODE = COLORIDX_WSTRING;
			nCOMMENTEND = Match_Quote( L'"', nPos + 1, nLineLen, pLine );	/* ダブルクォーテーション文字列の終端があるか */
		}
		break;
	case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
		break;
	case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_sTypeConfig.m_cBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );
		}
		else if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_sTypeConfig.m_cBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );
		}
		else if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* シングルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( L'\'', nPos, nLineLen, pLine );
		}
		else if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* ダブルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( L'"', nPos, nLineLen, pLine );
		}
		else if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	}
	return false;
}
