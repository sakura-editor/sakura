#include "stdafx.h"
#include "CDiffManager.h"


EDiffMark CDiffLineGetter::GetLineDiffMark() const{ return (EDiffMark)m_pcDocLine->m_sMark.m_cDiffmarked; }
void CDiffLineSetter::SetLineDiffMark(EDiffMark mark){ m_pcDocLine->m_sMark.m_cDiffmarked = mark; }


/*!	差分表示の全解除
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

/*! 差分検索
	@author	MIK
	@date	2002.05.25
*/
bool CDiffLineMgr::SearchDiffMark(
	CLogicInt			nLineNum,		//!< 検索開始行
	ESearchDirection	bPrevOrNext,	//!< 0==前方検索 1==後方検索
	CLogicInt*			pnLineNum 		//!< マッチ行
)
{
	CLogicInt	nLinePos = nLineNum;

	//前方検索
	if( bPrevOrNext == SEARCH_BACKWARD )
	{
		nLinePos--;
		CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine )
		{
			if( CDiffLineGetter(pDocLine).GetLineDiffMark() != 0 )
			{
				*pnLineNum = nLinePos;				/* マッチ行 */
				return true;
			}
			nLinePos--;
			pDocLine = pDocLine->GetPrevLine();
		}
	}
	//後方検索
	else
	{
		nLinePos++;
		CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine )
		{
			if( CDiffLineGetter(pDocLine).GetLineDiffMark() != 0 )
			{
				*pnLineNum = nLinePos;				/* マッチ行 */
				return true;
			}
			nLinePos++;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

/*!	差分情報を行範囲指定で登録する。
	@author	MIK
	@date	2002/05/25
*/
void CDiffLineMgr::SetDiffMarkRange( EDiffMark nMode, CLogicInt nStartLine, CLogicInt nEndLine )
{
	CDiffManager::Instance()->SetDiffUse(true);

	if( nStartLine < CLogicInt(0) ) nStartLine = CLogicInt(0);

	//最終行より後に削除行あり
	CLogicInt	nLines = m_pcDocLineMgr->GetLineCount();
	if( nLines <= nEndLine )
	{
		nEndLine = nLines - CLogicInt(1);
		CDocLine*	pCDocLine = m_pcDocLineMgr->GetLine( nEndLine );
		if( pCDocLine ) CDiffLineSetter(pCDocLine).SetLineDiffMark(MARK_DIFF_DEL_EX);
	}

	//行範囲にマークをつける
	for( CLogicInt i = nStartLine; i <= nEndLine; i++ )
	{
		CDocLine*	pCDocLine = m_pcDocLineMgr->GetLine( i );
		if( pCDocLine ) CDiffLineSetter(pCDocLine).SetLineDiffMark(nMode);
	}

	return;
}
