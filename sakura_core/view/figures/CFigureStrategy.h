#pragma once

#include "view/colors/CColorStrategy.h" //SColorStrategyInfo
#include "util/design_template.h"
#include <vector>

struct SColorStrategyInfo;

//$$レイアウト構築フロー(DoLayout)も CFigure で行うと整理しやすい
class CFigure{
public:
	virtual ~CFigure(){}
	virtual bool DrawImp(SColorStrategyInfo* pInfo) = 0;
	virtual bool Match(const wchar_t* pText) const = 0;
};

//! 各種空白（半角空白／全角空白／タブ／改行）描画用の基本クラス
class CFigureSpace : public CFigure{
public:
	virtual bool DrawImp(SColorStrategyInfo* pInfo);
	virtual int GetSpaceColorType(const EColorIndexType& eCurColor) const
	{
		//return ( COLORIDX_SEARCH == eCurColor || IsRegularExpression(eCurColor) )? 1: 0;
		if( COLORIDX_SEARCH == eCurColor ) return 1;	// 検索強調文字列の中では現在の背景色に合わせる

		// 通常テキストと同一背景色が指定されている場合は通常テキスト以外の場所でも現在の背景色に合わせる
		// ※ちょっと色をずらした設定にするだけで指定背景色にできる
		const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
		const ColorInfo* pcColorInfoArr = pcDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr;
		if( pcColorInfoArr[GetColorIdx()].m_colBACK == pcColorInfoArr[COLORIDX_TEXT].m_colBACK ) return 1;

		return 0;
	}
	virtual void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const = 0;
	virtual EColorIndexType GetColorIdx(void) const = 0;
};



class CFigureManager : public TSingleton<CFigureManager>{
public:
	CFigureManager();
	virtual ~CFigureManager();
	CFigure& GetFigure(const wchar_t* pText);

private:
	std::vector<CFigure*>	m_vFigures;
};
