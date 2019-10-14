/*! @file */
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
#ifndef SAKURA_CCOLORSTRATEGY_POOL_H_
#define SAKURA_CCOLORSTRATEGY_POOL_H_

#include "util/design_template.h"
#include <vector>
#include "CColor_Comment.h"
#include "CColor_Quote.h"
#include "CColor_RegexKeyword.h"
#include "CColor_Found.h"
#include "CColor_Url.h"
#include "CColor_Numeric.h"
#include "CColor_KeywordSet.h"
#include "CColor_Found.h"
#include "CColor_Heredoc.h"

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
	// 2005.11.20 Mocaコメントの色分けがON/OFF関係なく行われていたバグを修正
	void CheckColorMODE(
		CColorStrategy*&		pcColorStrategy,	//!< [in,out]
		int					nPos,
		const CStringRef&	cLineStr
	)
	{
		if (!m_bNeedToCheckColorMODE)
			return;

		//色終了
		if(pcColorStrategy){
			if(pcColorStrategy->EndColor(cLineStr,nPos)){
				pcColorStrategy = NULL;
			}
		}

		//色開始
		if(!pcColorStrategy){
			// CheckColorMODE はレイアウト処理全体のボトルネックになるくらい頻繁に呼び出される
			// 基本クラスからの動的仮想関数呼び出しを使用すると無視できないほどのオーバヘッドになる模様
			// ここはエレガントさよりも性能優先で個々の派生クラスから BeginColor() を呼び出す
			if(m_pcHeredoc && m_pcHeredoc->BeginColor(cLineStr,nPos)){ pcColorStrategy = m_pcHeredoc; return; }
			if(m_pcBlockComment1 && m_pcBlockComment1->BeginColor(cLineStr,nPos)){ pcColorStrategy = m_pcBlockComment1; return; }
			if(m_pcBlockComment2 && m_pcBlockComment2->BeginColor(cLineStr,nPos)){ pcColorStrategy = m_pcBlockComment2; return; }
			if(m_pcLineComment && m_pcLineComment->BeginColor(cLineStr,nPos)){ pcColorStrategy = m_pcLineComment; return; }
			if(m_pcSingleQuote && m_pcSingleQuote->BeginColor(cLineStr,nPos)){ pcColorStrategy = m_pcSingleQuote; return; }
			if(m_pcDoubleQuote && m_pcDoubleQuote->BeginColor(cLineStr,nPos)){ pcColorStrategy = m_pcDoubleQuote; return; }
		}
	}

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
	bool	m_bNeedToCheckColorMODE;

	CEditView*						m_pcView;

	bool	m_bSkipBeforeLayoutGeneral;
	bool	m_bSkipBeforeLayoutFound;
};

#endif /* SAKURA_CCOLORSTRATEGY_POOL_H_ */
/*[EOF]*/
