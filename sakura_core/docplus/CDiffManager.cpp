﻿/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "docplus/CDiffManager.h"
#include "types/CTypeSupport.h"
#include "window/CEditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CDiffLineGetter                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EDiffMark CDiffLineGetter::GetLineDiffMark() const{ return (EDiffMark)m_pcDocLine->m_sMark.m_cDiffmarked; }

/*! 行の差分マークに対応した色を返す -> pnColorIndex
	
	色設定が無い場合は pnColorIndex を変更せずに false を返す。	
*/
bool CDiffLineGetter::GetDiffColor(EColorIndexType* pnColorIndex) const
{
	EDiffMark type = GetLineDiffMark();
	CEditView* pView = &CEditWnd::getInstance()->GetActiveView();

	//DIFF差分マーク表示	//@@@ 2002.05.25 MIK
	if( type ){
		switch( type ){
		case MARK_DIFF_APPEND:	//追加
			if( CTypeSupport(pView,COLORIDX_DIFF_APPEND).IsDisp() ){
				*pnColorIndex = COLORIDX_DIFF_APPEND;
				return true;
			}
			break;
		case MARK_DIFF_CHANGE:	//変更
			if( CTypeSupport(pView,COLORIDX_DIFF_CHANGE).IsDisp() ){
				*pnColorIndex = COLORIDX_DIFF_CHANGE;
				return true;
			}
			break;
		case MARK_DIFF_DELETE:	//削除
		case MARK_DIFF_DEL_EX:	//削除
			if( CTypeSupport(pView,COLORIDX_DIFF_DELETE).IsDisp() ){
				*pnColorIndex = COLORIDX_DIFF_DELETE;
				return true;
			}
			break;
		}
	}
	return false;
}

/*! DIFFマーク描画

	引数は仮。（無駄な引数ありそう）
*/
bool CDiffLineGetter::DrawDiffMark(CGraphics& gr, int y, int nLineHeight, COLORREF color) const
{
	EDiffMark type = GetLineDiffMark();

	if( type )	//DIFF差分マーク表示	//@@@ 2002.05.25 MIK
	{
		int	cy = y + nLineHeight / 2;

		gr.PushPen(color, 0);

		switch( type )
		{
		case MARK_DIFF_APPEND:	//追加
			::MoveToEx( gr, 3, cy, NULL );
			::LineTo  ( gr, 6, cy );
			::MoveToEx( gr, 4, cy - 2, NULL );
			::LineTo  ( gr, 4, cy + 3 );
			break;

		case MARK_DIFF_CHANGE:	//変更
			::MoveToEx( gr, 3, cy - 4, NULL );
			::LineTo  ( gr, 3, cy );
			::MoveToEx( gr, 3, cy + 2, NULL );
			::LineTo  ( gr, 3, cy + 3 );
			break;

		case MARK_DIFF_DELETE:	//削除
			cy -= 3;
			::MoveToEx( gr, 3, cy, NULL );
			::LineTo  ( gr, 5, cy );
			::LineTo  ( gr, 3, cy + 2 );
			::LineTo  ( gr, 3, cy );
			::LineTo  ( gr, 7, cy + 4 );
			break;
		
		case MARK_DIFF_DEL_EX:	//削除(EOF)
			cy += 3;
			::MoveToEx( gr, 3, cy, NULL );
			::LineTo  ( gr, 5, cy );
			::LineTo  ( gr, 3, cy - 2 );
			::LineTo  ( gr, 3, cy );
			::LineTo  ( gr, 7, cy - 4 );
			break;
		}

		gr.PopPen();

		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CDiffLineSetter                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CDiffLineSetter::SetLineDiffMark(EDiffMark mark){ m_pcDocLine->m_sMark.m_cDiffmarked = mark; }

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       CDiffLineMgr                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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

	CDiffManager::getInstance()->SetDiffUse(false);
}

/*! 差分検索
	@author	MIK
	@date	2002.05.25
*/
bool CDiffLineMgr::SearchDiffMark(
	CLogicInt			nLineNum,		//!< 検索開始行
	ESearchDirection	bPrevOrNext,	//!< 検索方向
	CLogicInt*			pnLineNum 		//!< マッチ行
)
{
	CLogicInt	nLinePos = nLineNum;

	// 後方検索
	if( bPrevOrNext == SEARCH_BACKWARD )
	{
		nLinePos--;
		const CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
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
	// 前方検索
	else
	{
		nLinePos++;
		const CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
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
	CDiffManager::getInstance()->SetDiffUse(true);

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
