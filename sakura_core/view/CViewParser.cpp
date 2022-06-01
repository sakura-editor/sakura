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
#include "CViewParser.h"
#include "doc/CEditDoc.h"
#include "doc/layout/CLayout.h"
#include "view/CEditView.h"
#include "charset/charcode.h"

/*
	カーソル直前の単語を取得 単語の長さを返します
	単語区切り
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
		/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */
		nIdxTo = m_pEditView->LineColumnToIndex( pcLayout, m_pEditView->GetCaret().GetCaretLayoutPos().GetX2() );
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
		bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
		if( WCODE::IsLineDelimiter(pLine[nLineLen - 1], bExtEol) ){
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

	/* 現在位置の単語の範囲を調べる */
	CLayoutRange sRange;
	bool bResult = m_pEditView->m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		nCurLine,
		nIdx,
		&sRange,
		&cmemWord,
		pcmemWord
	);
	if( bResult ){
		pcmemWord->AppendString( &pLine[nIdx], nCharChars );

		return pcmemWord->GetStringLength();
	}else{
		return 0;
	}
}

/*!
	キャレット位置の単語を取得
	単語区切り

	@param[out] pcmemWord キャレット位置の単語
	@return true: 成功，false: 失敗
	
	@date 2006.03.24 fon (CEditView::Command_SELECTWORDを流用)
*/
bool CViewParser::GetCurrentWord(
	CNativeW* pcmemWord
) const
{
	const CLayout*	pcLayout = m_pEditView->m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_pEditView->GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		return false;	/* 単語選択に失敗 */
	}

	/* 指定された桁に対応する行のデータ内の位置を調べる */
	CLogicInt		nIdx = m_pEditView->LineColumnToIndex( pcLayout, m_pEditView->GetCaret().GetCaretLayoutPos().GetX2() );

	/* 現在位置の単語の範囲を調べる */
	CLayoutRange sRange;
	bool bResult = m_pEditView->m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		m_pEditView->GetCaret().GetCaretLayoutPos().GetY2(),
		nIdx,
		&sRange,
		pcmemWord,
		NULL
	);

	return bResult;
}
