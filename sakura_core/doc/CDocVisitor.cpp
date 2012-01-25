#include "StdAfx.h"
#include "CDocVisitor.h"
#include "doc/CEditDoc.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"
#include "COpeBlk.h"


//! 改行コードを統一する
void CDocVisitor::SetAllEol(CEol cEol)
{
	CEditView* pcView = &CEditWnd::Instance()->GetActiveView();

	//アンドゥ記録開始
	COpeBlk* pcOpeBlk = NULL;
	if(!pcView->m_bDoing_UndoRedo){
		pcOpeBlk = new COpeBlk();
	}

	//カーソル位置記憶
	CLayoutInt		nViewTopLine = pcView->GetTextArea().GetViewTopLine();
	CLayoutInt		nViewLeftCol = pcView->GetTextArea().GetViewLeftCol();
	CLayoutPoint	ptCaretPosXY = pcView->GetCaret().GetCaretLayoutPos();
	CLayoutInt		nCaretPosX_Prev = pcView->GetCaret().m_nCaretPosX_Prev;

	//改行コードを統一する
	if(cEol.IsValid()){
		CLogicInt	nLine = CLogicInt(0);
		while( 1 ){
			CDocLine* pcDocLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLine); //#######非効率
			if(!pcDocLine)break;
			//改行を置換
			if(pcDocLine->GetEol()!=EOL_NONE && pcDocLine->GetEol()!=cEol){
				CLogicRange sRange;
				CNativeW cstrDummy;
				sRange.SetFrom(CLogicPoint(pcDocLine->GetLengthWithoutEOL(),nLine));
				sRange.SetTo(CLogicPoint(pcDocLine->GetLengthWithEOL(),nLine));
				pcView->ReplaceData_CEditView2(
					sRange,
					&cstrDummy,
					cEol.GetValue2(),
					cEol.GetLen(),
					false,
					pcOpeBlk
				);
			}
			nLine++;
		}
		//編集時入力改行コード
		CEditDoc::GetInstance(0)->m_cDocEditor.SetNewLineCode(cEol);
	}

	//アンドゥ記録
	if(pcOpeBlk){
		if(pcOpeBlk->GetNum()>0){
			// カーソル位置復元
			pcView->GetTextArea().SetViewTopLine(nViewTopLine);
			pcView->GetTextArea().SetViewLeftCol(nViewLeftCol);
			pcView->GetCaret().MoveCursor( ptCaretPosXY, TRUE );
			pcView->GetCaret().m_nCaretPosX_Prev = nCaretPosX_Prev;
			pcOpeBlk->AppendOpe(
				new CMoveCaretOpe(
					pcView->GetCaret().GetCaretLogicPos(),
					pcView->GetCaret().GetCaretLogicPos()
				)
			);

			m_pcDocRef->m_cDocEditor.m_cOpeBuf.AppendOpeBlk(pcOpeBlk);
		}
		else{
			delete pcOpeBlk;
		}
	}
}

