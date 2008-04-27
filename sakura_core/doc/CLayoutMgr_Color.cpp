#include "stdafx.h"
#include "CLayoutMgr.h"

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
		// 2005.11.20 Mocaコメントの色分けがON/OFF関係なく行われていたバグを修正
		if( m_bDispComment && m_cLineComment.Match( nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice
		}
		else if( m_bDispComment && m_cLayoutBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_cLayoutBlockComment.Match_CommentTo( 0, nPos + m_cLayoutBlockComment.getBlockFromLen(0), nLineLen, pLine );
		}
		else if( m_bDispComment &&  m_cLayoutBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_cLayoutBlockComment.Match_CommentTo( 1, nPos + m_cLayoutBlockComment.getBlockFromLen(1), nLineLen, pLine );
		}
		else if( m_bDispSString && /* シングルクォーテーション文字列を表示する */
			pLine[nPos] == L'\''
		){
			nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
			/* シングルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( L'\'', nPos + 1, nLineLen, pLine );
		}
		else if( m_bDispWString && /* ダブルクォーテーション文字列を表示する */
			pLine[nPos] == L'"'
		){
			nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
			/* ダブルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( L'"', nPos + 1, nLineLen, pLine );
		}
		break;
	case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
		break;
	case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_cLayoutBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );
		}
		else if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_cLayoutBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );
		}
		else if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
//#endif
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
