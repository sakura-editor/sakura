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
	//        検索ヒットフラグ設定 -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
	if(!pcView->m_sCurSearchOption.bRegularExp || (m_bSearchFlg && m_nSearchStart < nPos)){
		m_bSearchFlg = pcView->IsSearchString(
			cStr,
			CLogicInt(nPos),
			&m_nSearchStart,
			&m_nSearchEnd
		);
	}
	//マッチ文字列検出
	if( m_bSearchFlg && m_nSearchStart==nPos){
		return true;
	}
	return false;
}

bool CColor_Found::EndColor(const CStringRef& cStr, int nPos)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	//マッチ文字列終了検出
	if( m_nSearchEnd <= nPos ){ //+ == では行頭文字の場合、m_nSearchEndも０であるために文字色の解除ができないバグを修正 2003.05.03 かろと
		// -- -- マッチ文字列を描画 -- -- //

		return true;
	}

	return false;
}
