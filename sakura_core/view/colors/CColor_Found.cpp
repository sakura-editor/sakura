#include "stdafx.h"
#include "CColor_Found.h"
#include "types/CTypeSupport.h"


void CColor_Found::OnStartScanLogic()
{
	m_bSearchFlg	= true;
	m_nSearchStart	= CLogicInt(-1);
	m_nSearchEnd	= CLogicInt(-1);
}

bool CColor_Found::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	const CEditView* pcView = CColorStrategyPool::Instance()->GetCurrentView();
	if( !pcView->m_bCurSrchKeyMark || !CTypeSupport(pcView,COLORIDX_SEARCH).IsDisp() ){
		return false;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//        �����q�b�g�t���O�ݒ� -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor ���K�\���̌���������}�[�N������������
	if(!pcView->m_sCurSearchOption.bRegularExp || (m_bSearchFlg && m_nSearchStart < nPos)){
		m_bSearchFlg = pcView->IsSearchString(
			cStr,
			CLogicInt(nPos),
			&m_nSearchStart,
			&m_nSearchEnd
		);
	}
	//�}�b�`�����񌟏o
	if( m_bSearchFlg && m_nSearchStart==nPos){
		return true;
	}
	return false;
}

bool CColor_Found::EndColor(const CStringRef& cStr, int nPos)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	//�}�b�`������I�����o
	if( m_nSearchEnd <= nPos ){ //+ == �ł͍s�������̏ꍇ�Am_nSearchEnd���O�ł��邽�߂ɕ����F�̉������ł��Ȃ��o�O���C�� 2003.05.03 �����
		// -- -- �}�b�`�������`�� -- -- //

		return true;
	}

	return false;
}
