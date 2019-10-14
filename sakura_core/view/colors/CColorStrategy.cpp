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

#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "view/colors/CColorStrategyPool.h"
#include "doc/layout/CLayout.h"
#include "window/CEditWnd.h"
#include "types/CTypeSupport.h"

bool _IsPosKeywordHead(const CStringRef& cStr, int nPos)
{
	return (nPos==0 || !IS_KEYWORD_CHAR(cStr.At(nPos-1)));
}

/*! 色の切り替え判定
	@retval true 色の変更あり
	@retval false 色の変更なし
*/
bool SColorStrategyInfo::CheckChangeColor(const CStringRef& cLineStr)
{
	CColorStrategyPool* pool = CColorStrategyPool::getInstance();
	pool->SetCurrentView(m_pcView);
	CColor_Found*  pcFound  = pool->GetFoundStrategy();
	CColor_Select* pcSelect = pool->GetSelectStrategy();
	bool bChange = false;

	//選択範囲色終了
	if(m_pStrategySelect){
		if(m_pStrategySelect->EndColor(cLineStr,this->GetPosInLogic())){
			m_pStrategySelect = NULL;
			bChange = true;
		}
	}
	//選択範囲色開始
	if(!m_pStrategySelect){
		if(pcSelect->BeginColorEx(cLineStr,this->GetPosInLogic(), m_pDispPos->GetLayoutLineRef(), this->GetLayout())){
			m_pStrategySelect = pcSelect;
			bChange = true;
		}
	}

	//検索色終了
	if(m_pStrategyFound){
		if(m_pStrategyFound->EndColor(cLineStr,this->GetPosInLogic())){
			m_pStrategyFound = NULL;
			bChange = true;
		}
	}

	//検索色開始
	if(!m_pStrategyFound){
		if(pcFound->BeginColor(cLineStr,this->GetPosInLogic())){
			m_pStrategyFound = pcFound;
			bChange = true;
		}
	}

	//色終了
	if(m_pStrategy){
		if(m_pStrategy->EndColor(cLineStr,this->GetPosInLogic())){
			m_pStrategy = NULL;
			bChange = true;
		}
	}

	//色開始
	if(!m_pStrategy){
		int size = pool->GetStrategyCount();
		for(int i = 0; i < size; i++ ){
			if(pool->GetStrategy(i)->BeginColor(cLineStr,this->GetPosInLogic())){
				m_pStrategy = pool->GetStrategy(i);
				bChange = true;
				break;
			}
		}
	}

	//カーソル行背景色
	CTypeSupport cCaretLineBg(m_pcView, COLORIDX_CARETLINEBG);
	if( cCaretLineBg.IsDisp() && !m_pcView->m_bMiniMap ){
		if(m_colorIdxBackLine==COLORIDX_CARETLINEBG){
			if( m_pDispPos->GetLayoutLineRef() != m_pcView->GetCaret().GetCaretLayoutPos().GetY2() ){
				m_colorIdxBackLine = COLORIDX_TEXT;
				bChange = true;
			}
		}else{
			if( m_pDispPos->GetLayoutLineRef() == m_pcView->GetCaret().GetCaretLayoutPos().GetY2() ){
				m_colorIdxBackLine = COLORIDX_CARETLINEBG;
				bChange = true;
			}
		}
	}
	//偶数行の背景色
	CTypeSupport cEvenLineBg(m_pcView, COLORIDX_EVENLINEBG);
	if( cEvenLineBg.IsDisp() && !m_pcView->m_bMiniMap && m_colorIdxBackLine != COLORIDX_CARETLINEBG ){
		if( m_colorIdxBackLine == COLORIDX_EVENLINEBG ){
			if( m_pDispPos->GetLayoutLineRef() % 2 == 0 ){
				m_colorIdxBackLine = COLORIDX_TEXT;
				bChange = true;
			}
		}else{
			if( m_pDispPos->GetLayoutLineRef() % 2 == 1 ){
				m_colorIdxBackLine = COLORIDX_EVENLINEBG;
				bChange = true;
			}
		}
	}
	if( m_pcView->m_bMiniMap ){
		CTypeSupport cPageViewBg(m_pcView, COLORIDX_PAGEVIEW);
		if( cPageViewBg.IsDisp() ){
			CEditView& cActiveView = m_pcView->m_pcEditWnd->GetActiveView();
			CLayoutInt curLine = m_pDispPos->GetLayoutLineRef();
			if( m_colorIdxBackLine == COLORIDX_PAGEVIEW ){
				if( cActiveView.GetTextArea().GetViewTopLine() <= curLine && curLine < cActiveView.GetTextArea().GetBottomLine() ){
				}else{
					m_colorIdxBackLine = COLORIDX_TEXT;
					bChange = true;
				}
			}else if( m_colorIdxBackLine == COLORIDX_TEXT ){
				if( cActiveView.GetTextArea().GetViewTopLine() <= curLine && curLine < cActiveView.GetTextArea().GetBottomLine() ){
					m_colorIdxBackLine = COLORIDX_PAGEVIEW;
					bChange = true;
				}
			}
		}
	}

	return bChange;
}

/*! 色の切り替え

	@date 2013.05.11 novice 実際の変更は呼び出し側で行う
*/
void SColorStrategyInfo::DoChangeColor(CColor3Setting *pcColor)
{
	if(m_pStrategySelect){
		m_cIndex.eColorIndex = m_pStrategySelect->GetStrategyColor();
	}else if(m_pStrategyFound){
		m_cIndex.eColorIndex = m_pStrategyFound->GetStrategyColor();
	}else{
		m_cIndex.eColorIndex = m_pStrategy->GetStrategyColorSafe();
	}

	if(m_pStrategyFound){
		m_cIndex.eColorIndex2 = m_pStrategyFound->GetStrategyColor();
	}else{
		m_cIndex.eColorIndex2 = m_pStrategy->GetStrategyColorSafe();
	}

	m_cIndex.eColorIndexBg = m_colorIdxBackLine;

	*pcColor = m_cIndex;
}

/*!
  iniの色設定を番号でなく文字列で書き出す。(added by Stonee, 2001/01/12, 2001/01/15)
  配列の順番は共有メモリ中のデータの順番と一致している。

  @note 数値による内部的対応は EColorIndexType EColorIndexType.h
  日本語名などは  ColorInfo_DEFAULT CDocTypeSetting.cpp
  CShareDataからglobalに移動
*/
const SColorAttributeData g_ColorAttributeArr[] =
{
	{L"TXT", COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_EFFECTS},
	{L"RUL", COLOR_ATTRIB_NO_EFFECTS},
	{L"CAR", COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// キャレット		// 2006.12.07 ryoji
	{L"IME", COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// IMEキャレット	// 2006.12.07 ryoji
	{L"CBK", COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{L"UND", COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{L"CVL", COLOR_ATTRIB_NO_BACK | ( COLOR_ATTRIB_NO_EFFECTS & ~COLOR_ATTRIB_NO_BOLD )}, // 2007.09.09 Moca カーソル位置縦線
	{L"NOT", COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{L"LNO", 0},
	{L"MOD", 0},
	{L"EBK", COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{L"TAB", 0},
	{L"SPC", 0},	//2002.04.28 Add By KK
	{L"ZEN", 0},
	{L"CTL", 0},
	{L"EOL", 0},
	{L"RAP", 0},
	{L"VER", 0},  // 2005.11.08 Moca 指定桁縦線
	{L"EOF", 0},
	{L"NUM", 0},	//@@@ 2001.02.17 by MIK 半角数値の強調
	{L"BRC", 0},	//対括弧	// 02/09/18 ai Add
	{L"SEL", 0},
	{L"FND", 0},
	{L"FN2", 0},
	{L"FN3", 0},
	{L"FN4", 0},
	{L"FN5", 0},
	{L"CMT", 0},
	{L"SQT", 0},
	{L"WQT", 0},
	{L"HDC", 0},
	{L"URL", 0},
	{L"KW1", 0},
	{L"KW2", 0},
	{L"KW3", 0},	//@@@ 2003.01.13 by MIK 強調キーワード3-10
	{L"KW4", 0},
	{L"KW5", 0},
	{L"KW6", 0},
	{L"KW7", 0},
	{L"KW8", 0},
	{L"KW9", 0},
	{L"KWA", 0},
	{L"RK1", 0},	//@@@ 2001.11.17 add MIK
	{L"RK2", 0},	//@@@ 2001.11.17 add MIK
	{L"RK3", 0},	//@@@ 2001.11.17 add MIK
	{L"RK4", 0},	//@@@ 2001.11.17 add MIK
	{L"RK5", 0},	//@@@ 2001.11.17 add MIK
	{L"RK6", 0},	//@@@ 2001.11.17 add MIK
	{L"RK7", 0},	//@@@ 2001.11.17 add MIK
	{L"RK8", 0},	//@@@ 2001.11.17 add MIK
	{L"RK9", 0},	//@@@ 2001.11.17 add MIK
	{L"RKA", 0},	//@@@ 2001.11.17 add MIK
	{L"DFA", 0},	//DIFF追加	//@@@ 2002.06.01 MIK
	{L"DFC", 0},	//DIFF変更	//@@@ 2002.06.01 MIK
	{L"DFD", 0},	//DIFF削除	//@@@ 2002.06.01 MIK
	{L"MRK", 0},	//ブックマーク	// 02/10/16 ai Add
	{L"PGV", COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{L"LAST", 0}	// Not Used
};

/*
 * カラー名からインデックス番号に変換する
 */
int GetColorIndexByName( const WCHAR *name )
{
	int	i;
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		if( wcscmp( name, g_ColorAttributeArr[i].szName ) == 0 ) return i;
	}
	return -1;
}

/*
 * インデックス番号からカラー名に変換する
 */
const WCHAR* GetColorNameByIndex( int index )
{
	return g_ColorAttributeArr[index].szName;
}
