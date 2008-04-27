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
		// 2005.11.20 Moca�R�����g�̐F������ON/OFF�֌W�Ȃ��s���Ă����o�O���C��
		if( m_bDispComment && m_cLineComment.Match( nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_COMMENT;	/* �s�R�����g�ł��� */ // 2002/03/13 novice
		}
		else if( m_bDispComment && m_cLayoutBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK1;	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
			/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
			nCOMMENTEND = m_cLayoutBlockComment.Match_CommentTo( 0, nPos + m_cLayoutBlockComment.getBlockFromLen(0), nLineLen, pLine );
		}
		else if( m_bDispComment &&  m_cLayoutBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK2;	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
			/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
			nCOMMENTEND = m_cLayoutBlockComment.Match_CommentTo( 1, nPos + m_cLayoutBlockComment.getBlockFromLen(1), nLineLen, pLine );
		}
		else if( m_bDispSString && /* �V���O���N�H�[�e�[�V�����������\������ */
			pLine[nPos] == L'\''
		){
			nCOMMENTMODE = COLORIDX_SSTRING;	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
			/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
			nCOMMENTEND = Match_Quote( L'\'', nPos + 1, nLineLen, pLine );
		}
		else if( m_bDispWString && /* �_�u���N�H�[�e�[�V�����������\������ */
			pLine[nPos] == L'"'
		){
			nCOMMENTMODE = COLORIDX_WSTRING;	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
			/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
			nCOMMENTEND = Match_Quote( L'"', nPos + 1, nLineLen, pLine );
		}
		break;
	case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */ // 2002/03/13 novice
		break;
	case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
			nCOMMENTEND = m_cLayoutBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );
		}
		else if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	case COLORIDX_BLOCK2:	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
			nCOMMENTEND = m_cLayoutBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );
		}
		else if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
//#endif
	case COLORIDX_SSTRING:	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
			nCOMMENTEND = Match_Quote( L'\'', nPos, nLineLen, pLine );
		}
		else if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	case COLORIDX_WSTRING:	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
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
