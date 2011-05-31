#pragma once

#include "view/colors/CColorStrategy.h" //SColorStrategyInfo
#include "util/design_template.h"
#include <vector>


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
protected:
	virtual void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const = 0;
	virtual EColorIndexType GetColorIdx(void) const = 0;

	// 実装補助
	bool DrawImp_StyleSelect(SColorStrategyInfo* pInfo);
	void DrawImp_StylePop(SColorStrategyInfo* pInfo);
	void DrawImp_DrawUnderline(SColorStrategyInfo* pInfo, DispPos&);
};



class CFigureManager : public TSingleton<CFigureManager>{
public:
	CFigureManager();
	virtual ~CFigureManager();
	CFigure& GetFigure(const wchar_t* pText);

private:
	std::vector<CFigure*>	m_vFigures;
};
