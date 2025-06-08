/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFIGURESTRATEGY_5D122522_91F9_4FC5_A31A_E78FB5DB0D0F_H_
#define SAKURA_CFIGURESTRATEGY_5D122522_91F9_4FC5_A31A_E78FB5DB0D0F_H_
#pragma once

#include <vector>
#include "view/colors/CColorStrategy.h" //SColorStrategyInfo

//$$レイアウト構築フロー(DoLayout)も CFigure で行うと整理しやすい
class CFigure{
public:
	virtual ~CFigure(){}
	virtual bool DrawImp(SColorStrategyInfo* pInfo) = 0;
	virtual bool Match(const wchar_t* pText, int nTextLen) const = 0;

	//! 色分け表示対象判定
	virtual bool Disp(void) const = 0;

	//! 設定更新
	virtual void Update(void)
	{
		CEditDoc* pCEditDoc = CEditDoc::GetInstance(0);
		m_pTypeData = &pCEditDoc->m_cDocType.GetDocumentAttribute();
	}

protected:
	const STypeConfig* m_pTypeData;
};

typedef int FigureRenderType;

//! 通常テキスト描画
class CFigure_Text final : public CFigure{
public:
	// 文字列を進める
	static FigureRenderType GetRenderType(SColorStrategyInfo* pInfo);

	static const FigureRenderType RenderType_None = -1;
	static bool IsRenderType_Block(FigureRenderType nRenderType){
		return (nRenderType != RenderType_None) && (nRenderType & 0x1);
	}
	static bool DrawImpBlock(SColorStrategyInfo* pInfo, int nPos, int nLength);
	static int FowardChars(SColorStrategyInfo* pInfo);

	bool DrawImp(SColorStrategyInfo* pInfo) override;
	bool Match(const wchar_t* pText, int nTextLen) const override
	{
		return true;
	}

	//! 色分け表示対象判定
	bool Disp(void) const override
	{
		return true;
	}
};

//! 各種空白（半角空白／全角空白／タブ／改行）描画用の基本クラス
class CFigureSpace : public CFigure{
public:
	bool DrawImp(SColorStrategyInfo* pInfo) override;

protected:
	virtual void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const = 0;
	virtual EColorIndexType GetColorIdx(void) const = 0;

public:
	//! 色分け表示対象判定
	bool Disp(void) const override
	{
		EColorIndexType nColorIndex = GetColorIdx();
		return m_pTypeData->m_ColorInfoArr[nColorIndex].m_bDisp;
	}

	void Update(void) override
	{
		CFigure::Update();

		EColorIndexType nColorIndex = GetColorIdx();
		if( m_pTypeData->m_ColorInfoArr[nColorIndex].m_bDisp ){
			m_nDispColorIndex = nColorIndex;
		}else{
			m_nDispColorIndex = COLORIDX_TEXT;
		}
	}

protected:
	EColorIndexType GetDispColorIdx(void) const{ return m_nDispColorIndex; }

	// 実装補助
	bool DrawImp_StyleSelect(SColorStrategyInfo* pInfo);
	void DrawImp_StylePop(SColorStrategyInfo* pInfo);
	void DrawImp_DrawUnderline(SColorStrategyInfo* pInfo, DispPos&);

protected:
	EColorIndexType m_nDispColorIndex;
};
#endif /* SAKURA_CFIGURESTRATEGY_5D122522_91F9_4FC5_A31A_E78FB5DB0D0F_H_ */
