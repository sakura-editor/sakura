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

	const CEditView& view = *(CColorStrategyPool::getInstance()->GetCurrentView());
	if( !view.GetSelectionInfo().IsTextSelected() || !CTypeSupport(&view,COLORIDX_SELECT).IsDisp() ){
		return false;
	}

	// 2011.12.27 レイアウト行頭で1回だけ確認してあとはメンバー変数をみる
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
	CLogicInt nIdxFrom = view.LineColumnToIndex(pcLayout, nSelectFrom) + pcLayout->GetLogicOffset();
	CLogicInt nIdxTo = view.LineColumnToIndex(pcLayout, nSelectTo) + pcLayout->GetLogicOffset();
	m_nSelectStart = nIdxFrom;
	m_nSelectEnd = nIdxTo;
	if( m_nSelectStart <= nPos && nPos < m_nSelectEnd ){
		return true;
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
	m_nSearchResult	= 1;
	m_nSearchStart	= CLogicInt(-1);
	m_nSearchEnd	= CLogicInt(-1);

	this->validColorNum = 0;
	for( int color = COLORIDX_SEARCH; color <= COLORIDX_SEARCHTAIL; ++color ) {
		if( m_pTypeData->m_ColorInfoArr[ color ].m_bDisp ) {
			this->highlightColors[ this->validColorNum++ ] = EColorIndexType( color );
		}
	}
}

bool CColor_Found::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;
	const CEditView* pcView = CColorStrategyPool::getInstance()->GetCurrentView();
	if( !pcView->m_bCurSrchKeyMark || 0 == this->validColorNum ){
		return false;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//        検索ヒットフラグ設定 -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
	if( pcView->m_sCurSearchOption.bWordOnly || (m_nSearchResult && m_nSearchStart < nPos) ){
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

