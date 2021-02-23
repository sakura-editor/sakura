/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "docplus/CModifyManager.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"

void CModifyManager::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// 行変更状態をすべてリセット
	CModifyVisitor().ResetAllModifyFlag(&pcDoc->m_cDocLineMgr, pcDoc->m_cDocEditor.m_cOpeBuf.GetCurrentPointer());
}

bool CModifyVisitor::IsLineModified(const CDocLine* pcDocLine, int saveSeq) const
{
	return pcDocLine->m_sMark.m_cModified.GetSeq() != saveSeq;
}
int CModifyVisitor::GetLineModifiedSeq(const CDocLine* pcDocLine) const
{
	return pcDocLine->m_sMark.m_cModified.GetSeq();
}
void CModifyVisitor::SetLineModified(CDocLine* pcDocLine, int seq)
{
	pcDocLine->m_sMark.m_cModified = seq;
}

/* 行変更状態をすべてリセット */
/*
  ・変更フラグCDocLineオブジェクト作成時にはTRUEである
  ・変更回数はCDocLineオブジェクト作成時には1である

  ファイルを読み込んだときは変更フラグを FALSEにする
  ファイルを読み込んだときは変更回数を 0にする

  ファイルを上書きした時は変更フラグを FALSEにする
  ファイルを上書きした時は変更回数は変えない

  変更回数はUndoしたときに-1される
  変更回数が0になった場合は変更フラグをFALSEにする
*/
void CModifyVisitor::ResetAllModifyFlag(CDocLineMgr* pcDocLineMgr, int seq)
{
	CDocLine* pDocLine = pcDocLineMgr->GetDocLineTop();
	while( pDocLine ){
		CDocLine* pDocLineNext = pDocLine->GetNextLine();
		SetLineModified(pDocLine, seq);
		pDocLine = pDocLineNext;
	}
}
