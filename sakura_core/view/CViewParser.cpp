#include "StdAfx.h"
#include "CViewParser.h"
#include "doc/CEditDoc.h"
#include "doc/layout/CLayout.h"
#include "view/CEditView.h"
#include "charset/charcode.h"

/*
	�J�[�\�����O�̒P����擾 �P��̒�����Ԃ��܂�
	�P���؂�
*/
int CViewParser::GetLeftWord( CNativeW* pcmemWord, int nMaxWordLen ) const
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nIdx;
	CLogicInt		nIdxTo;

	CNativeW	cmemWord;
	CLayoutInt	nCurLine;
	int			nCharChars;
	const CLayout* pcLayout;

	nCurLine = m_pEditView->GetCaret().GetCaretLayoutPos().GetY2();
	pLine = m_pEditView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCurLine, &nLineLen, &pcLayout );
	if( NULL == pLine ){
//		return 0;
		nIdxTo = CLogicInt(0);
	}else{
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */
		nIdxTo = m_pEditView->LineColmnToIndex( pcLayout, m_pEditView->GetCaret().GetCaretLayoutPos().GetX2() );
	}
	if( 0 == nIdxTo || NULL == pLine ){
		if( nCurLine <= 0 ){
			return 0;
		}
		nCurLine--;
		pLine = m_pEditView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCurLine, &nLineLen );
		if( NULL == pLine ){
			return 0;
		}
		if( WCODE::IsLineDelimiter(pLine[nLineLen - 1]) ){
			return 0;
		}

		nCharChars = &pLine[nLineLen] - CNativeW::GetCharPrev( pLine, nLineLen, &pLine[nLineLen] );
		if( 0 == nCharChars ){
			return 0;
		}
		nIdxTo = nLineLen;
		nIdx = nIdxTo - CLogicInt(nCharChars);
	}
	else{
		nCharChars = &pLine[nIdxTo] - CNativeW::GetCharPrev( pLine, nLineLen, &pLine[nIdxTo] );
		if( 0 == nCharChars ){
			return 0;
		}
		nIdx = nIdxTo - CLogicInt(nCharChars);
	}

	if( 1 == nCharChars ){
		if( WCODE::IsWordDelimiter(pLine[nIdx]) ){
			return 0;
		}
	}

	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	CLayoutRange sRange;
	int nResult=m_pEditView->m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		nCurLine,
		nIdx,
		&sRange,
		&cmemWord,
		pcmemWord
	);
	if( nResult ){
		pcmemWord->AppendString( &pLine[nIdx], nCharChars );

		return pcmemWord->GetStringLength();
	}else{
		return 0;
	}
}


/*!
	�L�����b�g�ʒu�̒P����擾
	�P���؂�

	@param[out] pcmemWord �L�����b�g�ʒu�̒P��
	@return true: �����Cfalse: ���s
	
	@date 2006.03.24 fon (CEditView::Command_SELECTWORD�𗬗p)
*/
BOOL CViewParser::GetCurrentWord(
		CNativeW* pcmemWord
) const
{
	const CLayout*	pcLayout = m_pEditView->m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_pEditView->GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		return false;	/* �P��I���Ɏ��s */
	}

	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
	CLogicInt		nIdx = m_pEditView->LineColmnToIndex( pcLayout, m_pEditView->GetCaret().GetCaretLayoutPos().GetX2() );

	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	CLayoutRange sRange;
	int nResult=m_pEditView->m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		m_pEditView->GetCaret().GetCaretLayoutPos().GetY2(),
		nIdx,
		&sRange,
		pcmemWord,
		NULL
	);
	if( nResult ){
		return true;	/* �P��I���ɐ��� */
	}
	else {
		return false;	/* �P��I���Ɏ��s */
	}
}

