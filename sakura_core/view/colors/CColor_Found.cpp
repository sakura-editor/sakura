#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Found.h"
#include "types/CTypeSupport.h"
#include "view/CViewSelect.h"
#include <limits.h>


void CColor_Select::OnStartScanLogic()
{
	m_nSelectLine	= CLayoutInt(-1);
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

	// 2011.12.27 ���C�A�E�g�s����1�񂾂��m�F���Ă��Ƃ̓����o�[�ϐ����݂�
	if( m_nSelectLine == nLineNum ){
		if( m_nSelectStart <= nPos && nPos < m_nSelectEnd ){
			return true;
		}
		return false;
	}
	m_nSelectLine = nLineNum;
	CLayoutRange selectArea = view.GetSelectionInfo().GetSelectAreaLine(nLineNum, pcLayout);
	CLayoutInt nSelectFrom = selectArea.GetFrom().x;
	CLayoutInt nSelectTo = selectArea.GetTo().x;
	if( nSelectFrom == nSelectTo || -1 == nSelectFrom ){
		m_nSelectStart = -1;
		m_nSelectEnd = -1;
		return false;
	}
	CLogicInt nIdxFrom = view.LineColmnToIndex(pcLayout, nSelectFrom) + pcLayout->GetLogicOffset();
	CLogicInt nIdxTo = view.LineColmnToIndex(pcLayout, nSelectTo) + pcLayout->GetLogicOffset();
	m_nSelectStart = nIdxFrom;
	m_nSelectEnd = nIdxTo;
	if( m_nSelectStart <= nPos && nPos < m_nSelectEnd ){
		return true;
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


CColor_Found::CColor_Found()
: validColorNum( 0 )
{}

void CColor_Found::OnStartScanLogic()
{
	m_nSearchResult	= 1;
	m_nSearchStart	= CLogicInt(-1);
	m_nSearchEnd	= CLogicInt(-1);

	this->validColorNum = 0;
	const CEditDoc* const pDoc = CEditDoc::GetInstance(0);
	if( pDoc ) {
		const STypeConfig& doctype = pDoc->m_cDocType.GetDocumentAttribute();
		for( int color = COLORIDX_SEARCH; color <= COLORIDX_SEARCHTAIL; ++color ) {
			if( doctype.m_ColorInfoArr[ color ].m_bDisp ) {
				this->highlightColors[ this->validColorNum++ ] = EColorIndexType( color );
			}
		}
	}
}

bool CColor_Found::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;
	const CEditView* pcView = CColorStrategyPool::Instance()->GetCurrentView();
	if( !pcView->m_bCurSrchKeyMark || 0 == this->validColorNum ){
		return false;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//        �����q�b�g�t���O�ݒ� -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor ���K�\���̌���������}�[�N������������
	if(!pcView->m_sCurSearchOption.bRegularExp || (m_nSearchResult && m_nSearchStart < nPos)){
		m_nSearchResult = pcView->IsSearchString(
			cStr,
			CLogicInt(nPos),
			&m_nSearchStart,
			&m_nSearchEnd
		);
	}
	//�}�b�`�����񌟏o
	if( m_nSearchResult && m_nSearchStart==nPos){
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

