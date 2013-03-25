#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigureStrategy.h"
#include "CFigure_Tab.h"
#include "CFigure_Eol.h"
#include "CFigure_ZenSpace.h"
#include "CFigure_HanSpace.h"
#include "CFigure_CtrlCode.h"
#include "view/CViewFont.h"
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
							pInfo->GetDocLine()->GetLengthWithoutEOL(),
							nIdx
						);
		bool bTrans = pInfo->pcView->IsBkBitmap() && CTypeSupport(pInfo->pcView, COLORIDX_TEXT).GetBackColor() == GetBkColor(pInfo->gr);
		pInfo->pcView->GetTextDrawer().DispText(
			pInfo->gr,
			pInfo->pDispPos,
			&pInfo->pLineOfLogic[nIdx],
			nLength,
			bTrans
		);
		pInfo->nPosInLogic += nLength;
		return true;
	}
	bool Match(const wchar_t* pText) const
	{
		return true;
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
	bool bTrans = DrawImp_StyleSelect(pInfo);
	DispPos sPos(*pInfo->pDispPos);	// 現在位置を覚えておく
	DispSpace(pInfo->gr, pInfo->pDispPos,pInfo->pcView, bTrans);	// 空白描画
	DrawImp_StylePop(pInfo);
	DrawImp_DrawUnderline(pInfo, sPos);
	// 1文字前提
	pInfo->nPosInLogic += CNativeW::GetSizeOfChar(	// 行末以外はここでスキャン位置を１字進める
		pInfo->pLineOfLogic,
		pInfo->GetDocLine()->GetLengthWithoutEOL(),
		pInfo->GetPosInLogic()
		);
	return true;
}

bool CFigureSpace::DrawImp_StyleSelect(SColorStrategyInfo* pInfo)
{
	// この DrawImp はここ（基本クラス）でデフォルト動作を実装しているが
	// 仮想関数なので派生クラス側のオーバーライドで個別に仕様変更可能
	CEditView* pcView = pInfo->pcView;

	CTypeSupport cCurrentType(pcView, pInfo->GetCurrentColor());	// 周辺の色（現在の指定色/選択色）
	CTypeSupport cCurrentType2(pcView, pInfo->GetCurrentColor2());	// 周辺の色（現在の指定色）
	CTypeSupport cTextType(pcView, COLORIDX_TEXT);				// テキストの指定色
	CTypeSupport cSpaceType(pcView, pcView->GetTextDrawer()._GetColorIdx(GetColorIdx()));	// 空白の指定色
	CTypeSupport cCurrentTypeBg(pcView, pInfo->GetCurrentColorBg());
	CTypeSupport& cCurrentType1 = (cCurrentType.GetBackColor() == cTextType.GetBackColor() ? cCurrentTypeBg: cCurrentType);
	CTypeSupport& cCurrentType3 = (cCurrentType2.GetBackColor() == cTextType.GetBackColor() ? cCurrentTypeBg: cCurrentType2);

	// 空白記号類は特に明示指定した部分以外はなるべく周辺の指定に合わせるようにしてみた	// 2009.05.30 ryoji
	// 例えば、下線を指定していない場合、正規表現キーワード内なら正規表現キーワード側の下線指定に従うほうが自然な気がする。
	// （そのほうが空白記号の「表示」をチェックしていない場合の表示に近い）
	//
	// 前景色・背景色の扱い
	// ・通常テキストとは異なる色が指定されている場合は空白記号の側の指定色を使う
	// ・通常テキストと同じ色が指定されている場合は周辺の色指定に合わせる
	// 太字の扱い
	// ・空白記号か周辺のどちらか一方でも太字指定されていれば「前景色・背景色の扱い」で決定した前景色で太字にする
	// 下線の扱い
	// ・空白記号で下線指定されていれば「前景色・背景色の扱い」で決定した前景色で下線を引く
	// ・空白記号で下線指定されておらず周辺で下線指定されていれば周辺の前景色で下線を引く
	// [選択]レンダリング中
	// ・混合色の場合は従来通り。
	COLORREF crText;
	COLORREF crBack;
	bool blendColor = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2() && cCurrentType.GetTextColor() == cCurrentType.GetBackColor(); // 選択混合色
	if( blendColor ){
		CTypeSupport& cText = cSpaceType.GetTextColor() == cTextType.GetTextColor() ? cCurrentType2 : cSpaceType;
		CTypeSupport& cBack = cSpaceType.GetBackColor() == cTextType.GetBackColor() ? cCurrentType3 : cSpaceType;
		crText = pcView->GetTextColorByColorInfo2(cCurrentType.GetColorInfo(), cText.GetColorInfo());
		crBack = pcView->GetBackColorByColorInfo2(cCurrentType.GetColorInfo(), cBack.GetColorInfo());
	}else{
		CTypeSupport& cText = cSpaceType.GetTextColor() == cTextType.GetTextColor() ? cCurrentType : cSpaceType;
		CTypeSupport& cBack = cSpaceType.GetBackColor() == cTextType.GetBackColor() ? cCurrentType1 : cSpaceType;
		crText = cText.GetTextColor();
		crBack = cBack.GetBackColor();
	}
	//cSpaceType.SetGraphicsState_WhileThisObj(pInfo->gr);

	pInfo->gr.PushTextForeColor(crText);
	pInfo->gr.PushTextBackColor(crBack);
	// Figureが下線指定ならこちらで下線を指定。元の色のほうが下線指定なら、DrawImp_DrawUnderlineで下線だけ指定
	pInfo->gr.PushMyFont(
		pInfo->pcView->GetFontset().ChooseFontHandle(cSpaceType.IsFatFont() || cCurrentType.IsFatFont(), cSpaceType.HasUnderLine())
	);
	bool bTrans = pcView->IsBkBitmap() && cTextType.GetBackColor() == crBack;
	return bTrans;
}

void CFigureSpace::DrawImp_StylePop(SColorStrategyInfo* pInfo)
{
	pInfo->gr.PopTextForeColor();
	pInfo->gr.PopTextBackColor();
	pInfo->gr.PopMyFont();
}

void CFigureSpace::DrawImp_DrawUnderline(SColorStrategyInfo* pInfo, DispPos& sPos)
{
	CEditView* pcView = pInfo->pcView;

	CTypeSupport cCurrentType(pcView, pInfo->GetCurrentColor());	// 周辺の色
	bool blendColor = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2() && cCurrentType.GetTextColor() == cCurrentType.GetBackColor(); // 選択混合色

	CTypeSupport colorStyle(pcView, blendColor ? pInfo->GetCurrentColor2() : pInfo->GetCurrentColor());	// 周辺の色
	CTypeSupport cSpaceType(pcView, pcView->GetTextDrawer()._GetColorIdx(GetColorIdx()));	// 空白の指定色

	if( !cSpaceType.HasUnderLine() && colorStyle.HasUnderLine() )
	{
		// 下線を周辺の前景色で描画する
		pInfo->gr.PushMyFont(
			pInfo->pcView->GetFontset().ChooseFontHandle(false, true)
		);

		int nLength = (Int)(pInfo->pDispPos->GetDrawCol() - sPos.GetDrawCol());
		wchar_t* pszText = new wchar_t[nLength];
		for( int i = 0; i < nLength; i++ )
			pszText[i] = L' ';
		pInfo->pcView->GetTextDrawer().DispText(
			pInfo->gr,
			&sPos,
			pszText,
			nLength,
			true		// 背景は透明
		);
		delete []pszText;

		pInfo->gr.PopMyFont();
	}
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
	m_vFigures.push_back(new CFigure_CtrlCode());
	m_vFigures.push_back(new CFigure_HanBinary());
	m_vFigures.push_back(new CFigure_ZenBinary());
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

