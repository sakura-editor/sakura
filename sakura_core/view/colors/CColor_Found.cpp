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
	//マッチ文字列終了検出
	if( m_nSelectEnd <= nPos ){
		// -- -- マッチ文字列を描画 -- -- //

		return true;
	}

	return false;
}


CColor_Found::CColor_Found()
: validColorNum( 0 )
{}

void CColor_Found::OnStartScanLogic()
{
	m_nSearchResult	= 0;
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
	//        検索ヒットフラグ設定 -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
	if(!pcView->m_sCurSearchOption.bRegularExp || (m_nSearchResult && m_nSearchStart < nPos)){
		m_nSearchResult = pcView->IsSearchString(
			cStr,
			CLogicInt(nPos),
			&m_nSearchStart,
			&m_nSearchEnd
		);
	}
	//マッチ文字列検出
	if( m_nSearchResult && m_nSearchStart==nPos){
		return true;
	}
	return false;
}

bool CColor_Found::EndColor(const CStringRef& cStr, int nPos)
{
	//マッチ文字列終了検出
	if( m_nSearchEnd <= nPos ){ //+ == では行頭文字の場合、m_nSearchEndも０であるために文字色の解除ができないバグを修正 2003.05.03 かろと
		// -- -- マッチ文字列を描画 -- -- //

		return true;
	}

	return false;
}

