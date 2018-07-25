/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CCOLORSTRATEGY_BC7B5956_A0AF_4C9C_9C0E_07FE658028AC9_H_
#define SAKURA_CCOLORSTRATEGY_BC7B5956_A0AF_4C9C_9C0E_07FE658028AC9_H_

// 要先行定義
// #include "view/CEditView.h"
#include "EColorIndexType.h"
#include "uiparts/CGraphics.h"

class	CEditView;

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
int GetColorIndexByName( const TCHAR *name );
const TCHAR* GetColorNameByIndex( int index );


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           基底                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DispPos;
class CColorStrategy;
#include "view/DispPos.h"

class CColor_Found;
class CColor_Select;

//! 色設定
struct CColor3Setting {
	EColorIndexType eColorIndex;    //!< 選択を含む現在の色
	EColorIndexType eColorIndex2;   //!< 選択以外の現在の色
	EColorIndexType eColorIndexBg;  //!< 背景色
};

struct SColorStrategyInfo{
	SColorStrategyInfo() : m_sDispPosBegin(0,0), m_pStrategy(NULL), m_pStrategyFound(NULL), m_pStrategySelect(NULL), m_colorIdxBackLine(COLORIDX_TEXT) {
		m_cIndex.eColorIndex = COLORIDX_TEXT;
		m_cIndex.eColorIndex2 = COLORIDX_TEXT;
		m_cIndex.eColorIndexBg = COLORIDX_TEXT;
	}

	//参照
	CEditView*	m_pcView;
	CGraphics	m_gr;	//(SColorInfoでは未使用)

	//スキャン位置
	LPCWSTR			m_pLineOfLogic;
	CLogicInt		m_nPosInLogic;

	//描画位置
	DispPos*		m_pDispPos;
	DispPos			m_sDispPosBegin;

	//色変え
	CColorStrategy*		m_pStrategy;
	CColor_Found*		m_pStrategyFound;
	CColor_Select*		m_pStrategySelect;
	EColorIndexType		m_colorIdxBackLine;
	CColor3Setting		m_cIndex;

	//! 色の切り替え
	bool CheckChangeColor(const CStringRef& cLineStr);
	void DoChangeColor(CColor3Setting *pcColor);
	EColorIndexType GetCurrentColor() const { return m_cIndex.eColorIndex; }
	EColorIndexType GetCurrentColor2() const { return m_cIndex.eColorIndex2; }
	EColorIndexType GetCurrentColorBg() const{ return m_cIndex.eColorIndexBg; }

	//! 現在のスキャン位置
	CLogicInt GetPosInLogic() const
	{
		return m_nPosInLogic;
	}
	const CDocLine* GetDocLine() const
	{
		return m_pDispPos->GetLayoutRef()->GetDocLineRef();
	}
	const CLayout* GetLayout() const
	{
		return m_pDispPos->GetLayoutRef();
	}
};

class CColorStrategy{
public:
	virtual ~CColorStrategy(){}
	//! 色定義
	virtual EColorIndexType GetStrategyColor() const = 0;
	virtual CLayoutColorInfo* GetStrategyColorInfo() const{
		return NULL;
	}
	//! 色切り替え開始を検出したら、その直前までの描画を行い、さらに色設定を行う。
	virtual void InitStrategyStatus() = 0;
	virtual void SetStrategyColorInfo(const CLayoutColorInfo* = NULL){};
	virtual bool BeginColor(const CStringRef& cStr, int nPos){ return false; }
	virtual bool EndColor(const CStringRef& cStr, int nPos){ return true; }
	virtual bool Disp() const = 0;
	//イベント
	virtual void OnStartScanLogic(){}

	//! 設定更新
	virtual void Update(void)
	{
		const CEditDoc* pCEditDoc = CEditDoc::GetInstance(0);
		m_pTypeData = &pCEditDoc->m_cDocType.GetDocumentAttribute();
	}

	//#######ラップ
	EColorIndexType GetStrategyColorSafe() const{ if(this)return GetStrategyColor(); else return COLORIDX_TEXT; }
	CLayoutColorInfo* GetStrategyColorInfoSafe() const{
		if(this){
			return GetStrategyColorInfo();
		}
		return NULL;
	}

protected:
	const STypeConfig* m_pTypeData;
};

#include "util/design_template.h"
#include <vector>
class CColor_LineComment;
class CColor_BlockComment;
class CColor_BlockComment;
class CColor_SingleQuote;
class CColor_DoubleQuote;
class CColor_Heredoc;

class CColorStrategyPool : public TSingleton<CColorStrategyPool>{
	friend class TSingleton<CColorStrategyPool>;
	CColorStrategyPool();
	virtual ~CColorStrategyPool();

public:

	//取得
	CColorStrategy*	GetStrategy(int nIndex) const{ return m_vStrategiesDisp[nIndex]; }
	int				GetStrategyCount() const{ return (int)m_vStrategiesDisp.size(); }
	CColorStrategy*	GetStrategyByColor(EColorIndexType eColor) const;

	//特定取得
	CColor_Found*   GetFoundStrategy() const{ return m_pcFoundStrategy; }
	CColor_Select*  GetSelectStrategy() const{ return m_pcSelectStrategy; }

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
	CEditView* GetCurrentView(void) const{ return m_pcView; }
	void SetCurrentView(CEditView* pcView) { m_pcView = pcView; }

private:
	std::vector<CColorStrategy*>	m_vStrategies;
	std::vector<CColorStrategy*>	m_vStrategiesDisp;	//!< 色分け表示対象
	CColor_Found*					m_pcFoundStrategy;
	CColor_Select*					m_pcSelectStrategy;

	CColor_LineComment*				m_pcLineComment;
	CColor_BlockComment*			m_pcBlockComment1;
	CColor_BlockComment*			m_pcBlockComment2;
	CColor_SingleQuote*				m_pcSingleQuote;
	CColor_DoubleQuote*				m_pcDoubleQuote;
	CColor_Heredoc*					m_pcHeredoc;

	CEditView*						m_pcView;

	bool	m_bSkipBeforeLayoutGeneral;
	bool	m_bSkipBeforeLayoutFound;
};

#endif /* SAKURA_CCOLORSTRATEGY_BC7B5956_A0AF_4C9C_9C0E_07FE658028AC9_H_ */
/*[EOF]*/
