/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2014, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "docplus/CFuncListManager.h"
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"

bool CFuncListManager::IsLineFuncList(const CDocLine* pcDocLine, bool bFlag) const
{
	return pcDocLine->m_sMark.m_cFuncList.GetFuncListMark() != bFlag;
}
bool CFuncListManager::GetLineFuncList(const CDocLine* pcDocLine) const
{
	return pcDocLine->m_sMark.m_cFuncList.GetFuncListMark();
}
void CFuncListManager::SetLineFuncList(CDocLine* pcDocLine, bool bFlag)
{
	pcDocLine->m_sMark.m_cFuncList = bFlag;
}

/*! 差分検索
	@author	MIK
	@date	2002.05.25
*/
bool CFuncListManager::SearchFuncListMark(
	const CDocLineMgr*	pcDocLineMgr,
	CLogicInt			nLineNum,		//!< 検索開始行
	ESearchDirection	bPrevOrNext,	//!< 検索方向
	CLogicInt*			pnLineNum 		//!< マッチ行
) const
{
	CLogicInt	nLinePos = nLineNum;

	if( bPrevOrNext == SEARCH_BACKWARD ){
		//後方検索(↑)
		nLinePos--;
		const CDocLine*	pDocLine = pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine ){
			if( GetLineFuncList(pDocLine) ){
				*pnLineNum = nLinePos;				/* マッチ行 */
				return true;
			}
			nLinePos--;
			pDocLine = pDocLine->GetPrevLine();
		}
	}else{
		//前方検索(↓)
		nLinePos++;
		const CDocLine*	pDocLine = pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine ){
			if( GetLineFuncList(pDocLine) ){
				*pnLineNum = nLinePos;				/* マッチ行 */
				return true;
			}
			nLinePos++;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

/* 関数リストマークをすべてリセット */
void CFuncListManager::ResetAllFucListMark(CDocLineMgr* pcDocLineMgr, bool bFlag)
{
	CDocLine* pDocLine = pcDocLineMgr->GetDocLineTop();
	while( pDocLine ){
		CDocLine* pDocLineNext = pDocLine->GetNextLine();
		SetLineFuncList(pDocLine, bFlag);
		pDocLine = pDocLineNext;
	}
}
