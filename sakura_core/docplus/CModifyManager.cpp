#include "StdAfx.h"
#include "docplus/CModifyManager.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"


void CModifyManager::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// �s�ύX��Ԃ����ׂă��Z�b�g
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

/* �s�ύX��Ԃ����ׂă��Z�b�g */
/*
  �E�ύX�t���OCDocLine�I�u�W�F�N�g�쐬���ɂ�TRUE�ł���
  �E�ύX�񐔂�CDocLine�I�u�W�F�N�g�쐬���ɂ�1�ł���

  �t�@�C����ǂݍ��񂾂Ƃ��͕ύX�t���O�� FALSE�ɂ���
  �t�@�C����ǂݍ��񂾂Ƃ��͕ύX�񐔂� 0�ɂ���

  �t�@�C�����㏑���������͕ύX�t���O�� FALSE�ɂ���
  �t�@�C�����㏑���������͕ύX�񐔂͕ς��Ȃ�

  �ύX�񐔂�Undo�����Ƃ���-1�����
  �ύX�񐔂�0�ɂȂ����ꍇ�͕ύX�t���O��FALSE�ɂ���
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


