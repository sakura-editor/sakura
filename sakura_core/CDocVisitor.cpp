#include "stdafx.h"
#include "CDocVisitor.h"
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

	//改行コードを統一する
	if(cEol.IsValid()){
		CDocLine*	pcDocLine = m_pcDocRef->m_cDocLineMgr.GetDocLineTop();
		CLogicInt	nLine = CLogicInt(0);
		while( pcDocLine ){
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
			pcDocLine = pcDocLine->GetNextLine();
			nLine++;
		}
		//編集時入力改行コード
		CEditDoc::GetInstance(0)->m_cDocEditor.SetNewLineCode(cEol);
	}

	//アンドゥ記録
	if(pcOpeBlk){
		if(pcOpeBlk->GetNum()>0){
			m_pcDocRef->m_cDocEditor.m_cOpeBuf.AppendOpeBlk(pcOpeBlk);
		}
		else{
			delete pcOpeBlk;
		}
	}
}

