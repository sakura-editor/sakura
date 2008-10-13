#include "stdafx.h"
#include "view/figures/CFigureStrategy.h"
#include "CFigure_Tab.h"
#include "CFigure_Eol.h"
#include "CFigure_ZenSpace.h"
#include "CFigure_HanSpace.h"
#include "doc/CLayout.h"
#include "charset/charcode.h"
#include "types/CTypeSupport.h"


//! 通常テキスト描画
class CFigure_Text : public CFigure{
public:
	bool DrawImp(SColorStrategyInfo* pInfo)
	{
		int nIdx = pInfo->GetPosInLogic();
		int nLength =	CNativeW::GetSizeOfChar(	// サロゲートペア対策	2008.10.12 ryoji
							pInfo->pLineOfLogic,
							pInfo->GetLayout()->GetLengthWithoutEOL(),
							nIdx
						);
		pInfo->pcView->GetTextDrawer().DispText(
			pInfo->gr,
			pInfo->pDispPos,
			&pInfo->pLineOfLogic[nIdx],
			nLength
		);
		pInfo->nPosInLogic += nLength;
		return true;
	}
	bool Match(const wchar_t* pText) const
	{
		return true;
	}
	CLayoutInt GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const
	{
		return CLayoutInt(WCODE::IsZenkaku(pText[0])?2:1);
	}
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画統合                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigureSpace                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool CFigureSpace::DrawImp(SColorStrategyInfo* pInfo)
{
	// この DrawImp や GetSpaceColorType は基本クラスでデフォルト動作を実装しているが
	// 仮想関数なので派生クラス側のオーバーライドで仕様変更可能
	EColorIndexType eCurColor = pInfo->GetCurrentColor();
	int nType = GetSpaceColorType(eCurColor);

	// nType	0:空白記号に指定された色を使う, 1:背景色だけ現在の背景色に合わせる
	// この値については必要に応じて仕様変更・拡張してください
	// 例）2:記号を表示しない（←文字色を現在の背景色と同じにする）
	CTypeSupport cSupport(pInfo->pcView, pInfo->pcView->GetTextDrawer()._GetColorIdx(GetColorIdx()));
	cSupport.SetGraphicsState_WhileThisObj(pInfo->gr);
	if( nType == 1 )
	{
		CTypeSupport cSupportSpecial(pInfo->pcView, eCurColor);
		pInfo->gr.PushTextBackColor(cSupportSpecial.GetBackColor());
	}
	DispSpace(pInfo->gr, pInfo->pDispPos,pInfo->pcView);
	if( nType == 1 )
		pInfo->gr.PopTextBackColor();
	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigureManager                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
CFigureManager::CFigureManager()
{
	m_vFigures.push_back(new CFigure_Tab());
	m_vFigures.push_back(new CFigure_HanSpace());
	m_vFigures.push_back(new CFigure_ZenSpace());
	m_vFigures.push_back(new CFigure_Eol());
	m_vFigures.push_back(new CFigure_Text());
}

CFigureManager::~CFigureManager()
{
	for(int i=0;i<(int)m_vFigures.size();i++){
		SAFE_DELETE(m_vFigures[i]);
	}
	m_vFigures.clear();
}

//$$ 高速化可能
CFigure& CFigureManager::GetFigure(const wchar_t* pText)
{
	for(int i=0;i<(int)m_vFigures.size();i++){
		CFigure* pcFigure = m_vFigures[i];
		if(pcFigure->Match(pText)){
			return *pcFigure;
		}
	}
	assert(0);
	static CFigure_Text cDummy;
	return cDummy;
}

