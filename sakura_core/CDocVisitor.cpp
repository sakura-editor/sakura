#include "stdafx.h"
#include "CDocVisitor.h"
#include "COpeBlk.h"

//! ���s�R�[�h�𓝈ꂷ��
void CDocVisitor::SetAllEol(CEol cEol)
{
	CEditView* pcView = &CEditWnd::Instance()->GetActiveView();

	//�A���h�D�L�^�J�n
	COpeBlk* pcOpeBlk = NULL;
	if(!pcView->m_bDoing_UndoRedo){
		pcOpeBlk = new COpeBlk();
	}

	//���s�R�[�h�𓝈ꂷ��
	if(cEol.IsValid()){
		CDocLine*	pcDocLine = m_pcDocRef->m_cDocLineMgr.GetDocLineTop();
		CLogicInt	nLine = CLogicInt(0);
		while( pcDocLine ){
			//���s��u��
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
		//�ҏW�����͉��s�R�[�h
		CEditDoc::GetInstance(0)->m_cDocEditor.SetNewLineCode(cEol);
	}

	//�A���h�D�L�^
	if(pcOpeBlk){
		if(pcOpeBlk->GetNum()>0){
			m_pcDocRef->m_cDocEditor.m_cOpeBuf.AppendOpeBlk(pcOpeBlk);
		}
		else{
			delete pcOpeBlk;
		}
	}
}

