#include "stdafx.h"
#include "docplus/CDiffManager.h"


EDiffMark CDiffLineGetter::GetLineDiffMark() const{ return (EDiffMark)m_pcDocLine->m_sMark.m_cDiffmarked; }
void CDiffLineSetter::SetLineDiffMark(EDiffMark mark){ m_pcDocLine->m_sMark.m_cDiffmarked = mark; }


/*!	�����\���̑S����
	@author	MIK
	@date	2002.05.25
*/
void CDiffLineMgr::ResetAllDiffMark()
{
	CDocLine* pDocLine = m_pcDocLineMgr->GetDocLineTop();
	while( pDocLine )
	{
		pDocLine->m_sMark.m_cDiffmarked = MARK_DIFF_NONE;
		pDocLine = pDocLine->GetNextLine();
	}

	CDiffManager::Instance()->SetDiffUse(false);
}

/*! ��������
	@author	MIK
	@date	2002.05.25
*/
bool CDiffLineMgr::SearchDiffMark(
	CLogicInt			nLineNum,		//!< �����J�n�s
	ESearchDirection	bPrevOrNext,	//!< 0==�O������ 1==�������
	CLogicInt*			pnLineNum 		//!< �}�b�`�s
)
{
	CLogicInt	nLinePos = nLineNum;

	//�O������
	if( bPrevOrNext == SEARCH_BACKWARD )
	{
		nLinePos--;
		CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine )
		{
			if( CDiffLineGetter(pDocLine).GetLineDiffMark() != 0 )
			{
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return true;
			}
			nLinePos--;
			pDocLine = pDocLine->GetPrevLine();
		}
	}
	//�������
	else
	{
		nLinePos++;
		CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine )
		{
			if( CDiffLineGetter(pDocLine).GetLineDiffMark() != 0 )
			{
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return true;
			}
			nLinePos++;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

/*!	���������s�͈͎w��œo�^����B
	@author	MIK
	@date	2002/05/25
*/
void CDiffLineMgr::SetDiffMarkRange( EDiffMark nMode, CLogicInt nStartLine, CLogicInt nEndLine )
{
	CDiffManager::Instance()->SetDiffUse(true);

	if( nStartLine < CLogicInt(0) ) nStartLine = CLogicInt(0);

	//�ŏI�s����ɍ폜�s����
	CLogicInt	nLines = m_pcDocLineMgr->GetLineCount();
	if( nLines <= nEndLine )
	{
		nEndLine = nLines - CLogicInt(1);
		CDocLine*	pCDocLine = m_pcDocLineMgr->GetLine( nEndLine );
		if( pCDocLine ) CDiffLineSetter(pCDocLine).SetLineDiffMark(MARK_DIFF_DEL_EX);
	}

	//�s�͈͂Ƀ}�[�N������
	for( CLogicInt i = nStartLine; i <= nEndLine; i++ )
	{
		CDocLine*	pCDocLine = m_pcDocLineMgr->GetLine( i );
		if( pCDocLine ) CDiffLineSetter(pCDocLine).SetLineDiffMark(nMode);
	}

	return;
}
