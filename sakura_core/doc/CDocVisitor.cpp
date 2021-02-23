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
#include "CDocVisitor.h"
#include "doc/CEditDoc.h"
#include "cmd/CViewCommander_inline.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"
#include "COpeBlk.h"

//! 改行コードを統一する
void CDocVisitor::SetAllEol(CEol cEol)
{
	CEditView* pcView = &CEditWnd::getInstance()->GetActiveView();

	//アンドゥ記録開始
	if(!pcView->m_bDoing_UndoRedo){
		if(pcView->m_cCommander.GetOpeBlk() == NULL){
			pcView->m_cCommander.SetOpeBlk(new COpeBlk());
		}
		pcView->m_cCommander.GetOpeBlk()->AddRef();
	}

	//カーソル位置記憶
	CLayoutInt		nViewTopLine = pcView->GetTextArea().GetViewTopLine();
	CLayoutInt		nViewLeftCol = pcView->GetTextArea().GetViewLeftCol();
	CLayoutPoint	ptCaretPosXY = pcView->GetCaret().GetCaretLayoutPos();
	CLayoutInt		nCaretPosX_Prev = pcView->GetCaret().m_nCaretPosX_Prev;

	bool bReplace = false;

	//改行コードを統一する
	if(cEol.IsValid()){
		CLogicInt	nLine = CLogicInt(0);
		COpeBlk* pcOpeBlk = pcView->m_bDoing_UndoRedo ? NULL : pcView->m_cCommander.GetOpeBlk();
		for (;;) {
			CDocLine* pcDocLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLine); //#######非効率
			if(!pcDocLine)break;
			//改行を置換
			if(pcDocLine->GetEol()!=EOL_NONE && pcDocLine->GetEol()!=cEol){
				CLogicRange sRange;
				sRange.SetFrom(CLogicPoint(pcDocLine->GetLengthWithoutEOL(),nLine));
				sRange.SetTo(CLogicPoint(pcDocLine->GetLengthWithEOL(),nLine));
				pcView->ReplaceData_CEditView2(
					sRange,
					cEol.GetValue2(),
					cEol.GetLen(),
					false,
					pcOpeBlk,
					true
				);
				bReplace = true;
			}
			nLine++;
		}
		//編集時入力改行コード
		CEditDoc::GetInstance(0)->m_cDocEditor.SetNewLineCode(cEol);
	}

	if( bReplace ){
		m_pcDocRef->m_cLayoutMgr._DoLayout(false);
		m_pcDocRef->m_pcEditWnd->ClearViewCaretPosInfo();
		if( m_pcDocRef->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
			m_pcDocRef->m_cLayoutMgr.CalculateTextWidth();
		}else{
			m_pcDocRef->m_cLayoutMgr.ClearLayoutLineWidth();
		}
	}
	//アンドゥ記録
	if(pcView->m_cCommander.GetOpeBlk()){
		if(pcView->m_cCommander.GetOpeBlk()->GetNum()>0){
			// カーソル位置復元
			pcView->GetTextArea().SetViewTopLine(nViewTopLine);
			pcView->GetTextArea().SetViewLeftCol(nViewLeftCol);
			pcView->GetCaret().MoveCursor( ptCaretPosXY, true );
			pcView->GetCaret().m_nCaretPosX_Prev = nCaretPosX_Prev;
			pcView->m_cCommander.GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					pcView->GetCaret().GetCaretLogicPos()
				)
			);
		}
		pcView->SetUndoBuffer();
	}
}
