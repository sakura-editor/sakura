/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCOLORSTRATEGY_96B6EB56_C928_4B89_8841_166AAAB8D760_H_
#define SAKURA_CCOLORSTRATEGY_96B6EB56_C928_4B89_8841_166AAAB8D760_H_
#pragma once

// 要先行定義
#include "EColorIndexType.h"
#include "uiparts/CGraphics.h"

#include "view/DispPos.h"
#include "util/design_template.h"

struct DispPos;

class CColorStrategy;
class CColor_Found;
class CColor_Select;
class CColor_LineComment;
class CColor_BlockComment;
class CColor_SingleQuote;
class CColor_DoubleQuote;
class CColor_Heredoc;
class CEditView;
class CStringRef;

bool _IsPosKeywordHead(const CStringRef& cStr, int nPos);

//! 正規表現キーワードのEColorIndexType値を作る関数
inline EColorIndexType ToColorIndexType_RegularExpression(const int nRegexColorIndex)
{
	return (EColorIndexType)(COLORIDX_REGEX_FIRST + nRegexColorIndex);
}

//! 正規表現キーワードのEColorIndexType値を色番号に戻す関数
inline int ToColorInfoArrIndex_RegularExpression(const EColorIndexType eRegexColorIndex)
{
	return eRegexColorIndex - COLORIDX_REGEX_FIRST;
}

/*! 色定数を色番号に変換する関数

	@date 2013.05.08 novice 範囲外のときはテキストを選択する
*/
inline int ToColorInfoArrIndex(const EColorIndexType eColorIndex)
{
	if( eColorIndex>=0 && eColorIndex<COLORIDX_LAST )
		return eColorIndex;
	else if( eColorIndex & COLORIDX_BLOCK_BIT )
		return COLORIDX_COMMENT;
	else if( eColorIndex & COLORIDX_REGEX_BIT )
		return ToColorInfoArrIndex_RegularExpression( eColorIndex );

	assert(0); // ここには来ない
	return COLORIDX_TEXT;
}

// カラー名＜＞インデックス番号の変換	//@@@ 2002.04.30
int GetColorIndexByName( const WCHAR *name );
const WCHAR* GetColorNameByIndex( int index );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           基底                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 色設定
struct CColor3Setting {
	EColorIndexType eColorIndex;    //!< 選択を含む現在の色
	EColorIndexType eColorIndex2;   //!< 選択以外の現在の色
	EColorIndexType eColorIndexBg;  //!< 背景色
};

struct SColorStrategyInfo{
	explicit SColorStrategyInfo(HDC hDC = nullptr)
		: m_gr(hDC)
	{
	}

	//参照
	CEditView*	m_pcView = nullptr;
	CGraphics	m_gr = nullptr;	//(SColorInfoでは未使用)

	//スキャン位置
	LPCWSTR			m_pLineOfLogic;
	CLogicInt		m_nPosInLogic;

	//描画位置
	DispPos*		m_pDispPos = nullptr;
	DispPos			m_sDispPosBegin{ 0, 0 };

	//色変え
	CColorStrategy*		m_pStrategy = nullptr;
	CColor_Found*		m_pStrategyFound = nullptr;
	CColor_Select*		m_pStrategySelect = nullptr;
	EColorIndexType		m_colorIdxBackLine = COLORIDX_TEXT;
	CColor3Setting		m_cIndex = { COLORIDX_TEXT, COLORIDX_TEXT, COLORIDX_TEXT };

	//! 色の切り替え
	bool CheckChangeColor(const CStringRef& cLineStr);
	void DoChangeColor(CColor3Setting *pcColor);

	EColorIndexType GetCurrentColor()   const noexcept { return m_cIndex.eColorIndex; }
	EColorIndexType GetCurrentColor2()  const noexcept { return m_cIndex.eColorIndex2; }
	EColorIndexType GetCurrentColorBg() const noexcept { return m_cIndex.eColorIndexBg; }

	//! 現在のスキャン位置
	CLogicInt GetPosInLogic() const noexcept
	{
		return m_nPosInLogic;
	}
	const CDocLine* GetDocLine() const noexcept
	{
		if (const auto layout = m_pDispPos->GetLayoutRef()) {
			return layout->GetDocLineRef();
		}
		return nullptr;
	}
	const CLayout* GetLayout() const noexcept
	{
		return m_pDispPos->GetLayoutRef();
	}
};

class CColorStrategy{
public:
	virtual ~CColorStrategy() = default;

	//! 色定義
	virtual EColorIndexType GetStrategyColor() const = 0;
	virtual CLayoutColorInfo* GetStrategyColorInfo() const{
		return nullptr;
	}
	//! 色切り替え開始を検出したら、その直前までの描画を行い、さらに色設定を行う。
	virtual void InitStrategyStatus() = 0;
	virtual void SetStrategyColorInfo([[maybe_unused]] const CLayoutColorInfo* info = nullptr) { }
	virtual bool BeginColor([[maybe_unused]] const CStringRef& cStr, [[maybe_unused]] int nPos) { return false; }
	virtual bool EndColor  ([[maybe_unused]] const CStringRef& cStr, [[maybe_unused]] int nPos) { return true; }
	virtual bool Disp() const = 0;
	//イベント
	virtual void OnStartScanLogic(){}

	//! 設定更新
	virtual void Update(void)
	{
		const CEditDoc* pCEditDoc = CEditDoc::getInstance();
		m_pTypeData = &pCEditDoc->m_cDocType.GetDocumentAttribute();
	}

	//#######ラップ
	static EColorIndexType GetStrategyColorSafe(const CColorStrategy *strategy) {
		if (strategy) {
			return strategy->GetStrategyColor();
		}
		return COLORIDX_TEXT;
	}
	static CLayoutColorInfo* GetStrategyColorInfoSafe(const CColorStrategy *strategy) {
		if (strategy) {
			return strategy->GetStrategyColorInfo();
		}
		return nullptr;
	}

protected:
	const STypeConfig* m_pTypeData = nullptr;
};

class CColorStrategyPool : public TSingleton<CColorStrategyPool>{
	friend class TSingleton<CColorStrategyPool>;
	CColorStrategyPool();
	virtual ~CColorStrategyPool();

public:

	//取得
	CColorStrategy*	GetStrategy(int nIndex) const noexcept { return m_vStrategiesDisp[nIndex]; }
	int				GetStrategyCount() const noexcept { return (int)m_vStrategiesDisp.size(); }
	CColorStrategy*	GetStrategyByColor(EColorIndexType eColor) const;

	//特定取得
	CColor_Found*   GetFoundStrategy()  const noexcept { return m_pcFoundStrategy; }
	CColor_Select*  GetSelectStrategy() const noexcept { return m_pcSelectStrategy; }

	//イベント
	void NotifyOnStartScanLogic();

	/*
	|| 色分け
	*/
	//@@@ 2002.09.22 YAZAKI
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	void CheckColorMODE( CColorStrategy** ppcColorStrategy, int nPos, const CStringRef& cLineStr );
	bool IsSkipBeforeLayout();	// レイアウトが行頭からチェックしなくていいか判定

	//設定変更
	void OnChangeSetting(void);

	//ビューの設定・取得
	CEditView* GetCurrentView(void) const noexcept { return m_pcView; }
	void SetCurrentView(CEditView* pcView) { m_pcView = pcView; }

	//範囲を持つ色分けがあるかどうか
	bool HasRangeBasedColorStrategies() const noexcept;

private:
	std::vector<CColorStrategy*>	m_vStrategies;
	std::vector<CColorStrategy*>	m_vStrategiesDisp;	//!< 色分け表示対象
	CColor_Found*					m_pcFoundStrategy = nullptr;
	CColor_Select*					m_pcSelectStrategy = nullptr;

	// 範囲を持つ色分け
	// 追加/削除した時はHasRangeBasedColorStrategiesをメンテして下さい
	CColor_LineComment*				m_pcLineComment = nullptr;
	CColor_BlockComment*			m_pcBlockComment1 = nullptr;
	CColor_BlockComment*			m_pcBlockComment2 = nullptr;
	CColor_SingleQuote*				m_pcSingleQuote = nullptr;
	CColor_DoubleQuote*				m_pcDoubleQuote = nullptr;
	CColor_Heredoc*					m_pcHeredoc = nullptr;

	CEditView*						m_pcView = nullptr;

	bool	m_bSkipBeforeLayoutGeneral = false;
	bool	m_bSkipBeforeLayoutFound = false;
};

#endif /* SAKURA_CCOLORSTRATEGY_96B6EB56_C928_4B89_8841_166AAAB8D760_H_ */
