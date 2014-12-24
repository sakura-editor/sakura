/*
	Copyright (C) 2008, kobake
	Copyright (C) 2014, Moca

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

/*! ��������
	@author	MIK
	@date	2002.05.25
*/
bool CFuncListManager::SearchFuncListMark(
	const CDocLineMgr*	pcDocLineMgr,
	CLogicInt			nLineNum,		//!< �����J�n�s
	ESearchDirection	bPrevOrNext,
	CLogicInt*			pnLineNum 		//!< �}�b�`�s
) const
{
	CLogicInt	nLinePos = nLineNum;

	if( bPrevOrNext == SEARCH_BACKWARD ){
		//�������(��)
		nLinePos--;
		const CDocLine*	pDocLine = pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine ){
			if( GetLineFuncList(pDocLine) ){
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return true;
			}
			nLinePos--;
			pDocLine = pDocLine->GetPrevLine();
		}
	}else{
		//�O������(��)
		nLinePos++;
		const CDocLine*	pDocLine = pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine ){
			if( GetLineFuncList(pDocLine) ){
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return true;
			}
			nLinePos++;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

/* �֐����X�g�}�[�N�����ׂă��Z�b�g */
void CFuncListManager::ResetAllFucListMark(CDocLineMgr* pcDocLineMgr, bool bFlag)
{
	CDocLine* pDocLine = pcDocLineMgr->GetDocLineTop();
	while( pDocLine ){
		CDocLine* pDocLineNext = pDocLine->GetNextLine();
		SetLineFuncList(pDocLine, bFlag);
		pDocLine = pDocLineNext;
	}
}


