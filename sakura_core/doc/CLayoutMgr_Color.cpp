#include "stdafx.h"
#include "CLayoutMgr.h"

// 2005.11.20 Moca�R�����g�̐F������ON/OFF�֌W�Ȃ��s���Ă����o�O���C��
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
		// �s�R�����g	// 2002/03/13 novice
		if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp && m_sTypeConfig.m_cLineComment.Match( nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_COMMENT;
		}
		// �u���b�N�R�����g1	// 2002/03/13 novice
		else if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp && m_sTypeConfig.m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK1;
			nCOMMENTEND = m_sTypeConfig.m_cBlockComment.Match_CommentTo( 0, nPos + m_sTypeConfig.m_cBlockComment.getBlockFromLen(0), nLineLen, pLine );	/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
		}
		// �u���b�N�R�����g2	// 2002/03/13 novice
		else if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&  m_sTypeConfig.m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK2;
			nCOMMENTEND = m_sTypeConfig.m_cBlockComment.Match_CommentTo( 1, nPos + m_sTypeConfig.m_cBlockComment.getBlockFromLen(1), nLineLen, pLine );	/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
		}
		// �V���O���N�H�[�e�[�V����������
		else if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp && pLine[nPos] == L'\'' ){
			nCOMMENTMODE = COLORIDX_SSTRING;
			nCOMMENTEND = Match_Quote( L'\'', nPos + 1, nLineLen, pLine );	/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
		}
		// �_�u���N�H�[�e�[�V����������
		else if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp && pLine[nPos] == L'"' ){
			nCOMMENTMODE = COLORIDX_WSTRING;
			nCOMMENTEND = Match_Quote( L'"', nPos + 1, nLineLen, pLine );	/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
		}
		break;
	case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */ // 2002/03/13 novice
		break;
	case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
			nCOMMENTEND = m_sTypeConfig.m_cBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );
		}
		else if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	case COLORIDX_BLOCK2:	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
			nCOMMENTEND = m_sTypeConfig.m_cBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );
		}
		else if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
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
