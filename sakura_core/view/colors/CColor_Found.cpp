#include "stdafx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Found.h"
#include "types/CTypeSupport.h"
#include "view/CViewSelect.h"
#include <limits.h>


void CColor_Select::OnStartScanLogic()
{
	m_bSelectFlg	= true;
	m_nSelectStart	= CLogicInt(-1);
	m_nSelectEnd	= CLogicInt(-1);
}

bool CColor_Select::BeginColor(const CStringRef& cStr, int nPos)
{
	assert(0);
	return false;
}

bool CColor_Select::BeginColorEx(const CStringRef& cStr, int nPos, CLayoutInt nLineNum, const CLayout* pcLayout)
{

	if(!cStr.IsValid())return false;

	const CEditView& view = *(CColorStrategyPool::Instance()->GetCurrentView());
	if( !view.GetSelectionInfo().IsTextSelected() || !CTypeSupport(&view,COLORIDX_SELECT).IsDisp() ){
		return false;
	}

	CLayoutRange selectArea = view.GetSelectionInfo().GetSelectAreaLine(nLineNum, pcLayout);
	CLayoutInt nSelectFrom = selectArea.GetFrom().x;
	CLayoutInt nSelectTo = selectArea.GetTo().x;
	if( nSelectFrom == nSelectTo ){
		return false;
	}
	if( -1 == nSelectFrom ){
		return false;
	}
	CLogicInt nIdxFrom = view.LineColmnToIndex(pcLayout, nSelectFrom) + pcLayout->GetLogicOffset();
	if( nIdxFrom <= nPos ){
		CLogicInt nIdxTo = view.LineColmnToIndex(pcLayout, nSelectTo) + pcLayout->GetLogicOffset();
		if( nPos < nIdxTo ){
			m_nSelectStart = nIdxFrom;
			m_nSelectEnd = nIdxTo;
			return true;
		}
	}
	return false;
}

bool CColor_Select::EndColor(const CStringRef& cStr, int nPos)
{
	//�}�b�`������I�����o
	if( m_nSelectEnd <= nPos ){
		// -- -- �}�b�`�������`�� -- -- //

		return true;
	}

	return false;
}


void CColor_Found::OnStartScanLogic()
{
	m_bSearchFlg	= true;
	m_nSearchStart	= CLogicInt(-1);
	m_nSearchEnd	= CLogicInt(-1);
}

bool CColor_Found::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;
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
	//�}�b�`������I�����o
	if( m_nSearchEnd <= nPos ){ //+ == �ł͍s�������̏ꍇ�Am_nSearchEnd���O�ł��邽�߂ɕ����F�̉������ł��Ȃ��o�O���C�� 2003.05.03 �����
		// -- -- �}�b�`�������`�� -- -- //

		return true;
	}

	return false;
}

