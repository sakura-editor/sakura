#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "CColor_Comment.h"
#include "CColor_Quote.h"
#include "CColor_RegexKeyword.h"
#include "CColor_Found.h"
#include "CColor_Url.h"
#include "CColor_Numeric.h"
#include "CColor_KeywordSet.h"
#include "CColor_Found.h"
#include "doc/layout/CLayout.h"
#include "window/CEditWnd.h"
#include "types/CTypeSupport.h"




bool _IsPosKeywordHead(const CStringRef& cStr, int nPos)
{
	return (nPos==0 || !IS_KEYWORD_CHAR(cStr.At(nPos-1)));
}


CLogicInt SColorStrategyInfo::GetPosInLayout() const
{
	return nPosInLogic - pDispPos->GetLayoutRef()->GetLogicOffset();
}

const CDocLine* SColorStrategyInfo::GetDocLine() const
{
	return pDispPos->GetLayoutRef()->GetDocLineRef();
}

const CLayout* SColorStrategyInfo::GetLayout() const
{
	return pDispPos->GetLayoutRef();
}

void SColorStrategyInfo::DoChangeColor(const CStringRef& cLineStr)
{
	CColorStrategyPool* pool = CColorStrategyPool::getInstance();
	pool->SetCurrentView(this->pcView);
	CColor_Found*  pcFound  = pool->GetFoundStrategy();
	CColor_Select* pcSelect = pool->GetSelectStrategy();

	//選択範囲色終了
	if(this->pStrategySelect){
		if(this->pStrategySelect->EndColor(cLineStr,this->GetPosInLogic())){
			this->pStrategySelect = NULL;
			ChangeColor2(GetCurrentColor(), GetCurrentColor2());
		}
	}
	//選択範囲色開始
	if(!this->pStrategySelect){
		if(pcSelect->BeginColorEx(cLineStr,this->GetPosInLogic(), this->pDispPos->GetLayoutLineRef(), this->GetLayout())){
			this->pStrategySelect = pcSelect;
			ChangeColor2(GetCurrentColor(), GetCurrentColor2());
		}
	}

	//検索色終了
	if(this->pStrategyFound){
		if(this->pStrategyFound->EndColor(cLineStr,this->GetPosInLogic())){
			this->pStrategyFound = NULL;
			ChangeColor2(GetCurrentColor(), GetCurrentColor2());
		}
	}

	//検索色開始
	if(!this->pStrategyFound){
		if(pcFound->BeginColor(cLineStr,this->GetPosInLogic())){
			this->pStrategyFound = pcFound;
			ChangeColor2(GetCurrentColor(), GetCurrentColor2());
		}
	}

	//色終了
	if(this->pStrategy){
		if(this->pStrategy->EndColor(cLineStr,this->GetPosInLogic())){
			this->pStrategy = NULL;
			ChangeColor2(GetCurrentColor(), GetCurrentColor2());
		}
	}

	//色開始
	if(!this->pStrategy){
		int size = pool->GetStrategyCount();
		for(int i = 0; i < size; i++ ){
			if(pool->GetStrategy(i)->BeginColor(cLineStr,this->GetPosInLogic())){
				this->pStrategy = pool->GetStrategy(i);
				ChangeColor2(GetCurrentColor(), GetCurrentColor2());
				break;
			}
		}
	}
	
	//カーソル行背景色
	CTypeSupport cCaretLineBg(this->pcView, COLORIDX_CARETLINEBG);
	if( cCaretLineBg.IsDisp() ){
		if(m_colorIdxBackLine==COLORIDX_CARETLINEBG){
			if( this->pDispPos->GetLayoutLineRef() != this->pcView->GetCaret().GetCaretLayoutPos().GetY2() ){
				m_colorIdxBackLine = COLORIDX_TEXT;
				ChangeColor2(GetCurrentColor(), GetCurrentColor2());
			}
		}else{
			if( this->pDispPos->GetLayoutLineRef() == this->pcView->GetCaret().GetCaretLayoutPos().GetY2() ){
				m_colorIdxBackLine = COLORIDX_CARETLINEBG;
				ChangeColor2(GetCurrentColor(), GetCurrentColor2());
			}
		}
	}
}

EColorIndexType SColorStrategyInfo::GetCurrentColor() const
{
	if(pStrategySelect){
		return pStrategySelect->GetStrategyColor();
	}else if(pStrategyFound){
		return pStrategyFound->GetStrategyColor();
	}
	return pStrategy->GetStrategyColorSafe();
}

EColorIndexType SColorStrategyInfo::GetCurrentColor2() const
{
	if(pStrategyFound){
		return pStrategyFound->GetStrategyColor();
	}
	return pStrategy->GetStrategyColorSafe();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          プール                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CColorStrategyPool::CColorStrategyPool()
{
	m_pcView = &(CEditWnd::getInstance()->GetView(0));
	m_pcSelectStrategy = new CColor_Select();
	m_pcFoundStrategy = new CColor_Found();
//	m_vStrategies.push_back(new CColor_Found);				// マッチ文字列
	m_vStrategies.push_back(new CColor_RegexKeyword);		// 正規表現キーワード
	m_vStrategies.push_back(new CColor_LineComment);		// 行コメント
	m_vStrategies.push_back(new CColor_BlockComment(COLORIDX_BLOCK1));	// ブロックコメント
	m_vStrategies.push_back(new CColor_BlockComment(COLORIDX_BLOCK2));	// ブロックコメント2
	m_vStrategies.push_back(new CColor_SingleQuote);		// シングルクォーテーション文字列
	m_vStrategies.push_back(new CColor_DoubleQuote);		// ダブルクォーテーション文字列
	m_vStrategies.push_back(new CColor_Url);				// URL
	m_vStrategies.push_back(new CColor_Numeric);			// 半角数字
	m_vStrategies.push_back(new CColor_KeywordSet);			// キーワードセット

	// 設定更新
	OnChangeSetting();

	// CheckColorMODE 用
	m_pcLineComment = (CColor_LineComment*)GetStrategyByColor(COLORIDX_COMMENT);	// 行コメント
	m_pcBlockComment1 = (CColor_BlockComment*)GetStrategyByColor(COLORIDX_BLOCK1);	// ブロックコメント
	m_pcBlockComment2 = (CColor_BlockComment*)GetStrategyByColor(COLORIDX_BLOCK2);	// ブロックコメント2
	m_pcSingleQuote = (CColor_SingleQuote*)GetStrategyByColor(COLORIDX_SSTRING);	// シングルクォーテーション文字列
	m_pcDoubleQuote = (CColor_DoubleQuote*)GetStrategyByColor(COLORIDX_WSTRING);	// ダブルクォーテーション文字列
}

CColorStrategyPool::~CColorStrategyPool()
{
	SAFE_DELETE(m_pcSelectStrategy);
	SAFE_DELETE(m_pcFoundStrategy);
	int size = (int)m_vStrategies.size();
	for(int i = 0; i < size; i++ ){
		delete m_vStrategies[i];
	}
	m_vStrategies.clear();
}

CColorStrategy*	CColorStrategyPool::GetStrategyByColor(EColorIndexType eColor) const
{
	if( COLORIDX_SEARCH <= eColor && eColor <= COLORIDX_SEARCHTAIL ){
		return m_pcFoundStrategy;
	}
	int size = (int)m_vStrategies.size();
	for(int i = 0; i < size; i++ ){
		if(m_vStrategies[i]->GetStrategyColor()==eColor){
			return m_vStrategies[i];
		}
	}
	return NULL;
}

void CColorStrategyPool::NotifyOnStartScanLogic()
{
	m_pcSelectStrategy->OnStartScanLogic();
	m_pcFoundStrategy->OnStartScanLogic();
	int size = GetStrategyCount();
	for(int i = 0; i < size; i++ ){
		GetStrategy(i)->OnStartScanLogic();
	}
}


// 2005.11.20 Mocaコメントの色分けがON/OFF関係なく行われていたバグを修正
bool CColorStrategyPool::CheckColorMODE(
	CColorStrategy**	ppcColorStrategy,	//!< [in/out]
	int					nPos,
	const CStringRef&	cLineStr
)
{
	//色終了
	if(*ppcColorStrategy){
		if((*ppcColorStrategy)->EndColor(cLineStr,nPos)){
			*ppcColorStrategy = NULL;
			return true;
		}
	}

	//色開始
	if(!*ppcColorStrategy){
		// CheckColorMODE はレイアウト処理全体のボトルネックになるくらい頻繁に呼び出される
		// 基本クラスからの動的仮想関数呼び出しを使用すると無視できないほどのオーバヘッドになる模様
		// ここはエレガントさよりも性能優先で個々の派生クラスから BeginColor() を呼び出す
		if(m_pcLineComment->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcLineComment; return false; }
		if(m_pcBlockComment1->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcBlockComment1; return false; }
		if(m_pcBlockComment2->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcBlockComment2; return false; }
		if(m_pcSingleQuote->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcSingleQuote; return false; }
		if(m_pcDoubleQuote->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcDoubleQuote; return false; }
	}

	return false;
}

/*! 設定更新
*/
void CColorStrategyPool::OnChangeSetting(void)
{
	m_pcSelectStrategy->Update();
	m_pcFoundStrategy->Update();
	int size = (int)m_vStrategies.size();
	for(int i = 0; i < size; i++){
		m_vStrategies[i]->Update();
	}
}

/*!
  iniの色設定を番号でなく文字列で書き出す。(added by Stonee, 2001/01/12, 2001/01/15)
  配列の順番は共有メモリ中のデータの順番と一致している。

  @note 数値による内部的対応は EColorIndexType CColorStrategy.h
  日本語名などは  ColorInfo_DEFAULT CDocTypeSetting.cpp
  CShareDataからglobalに移動
*/
const SColorAttributeData g_ColorAttributeArr[] =
{
	{_T("TXT"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_EFFECTS},
	{_T("RUL"), COLOR_ATTRIB_NO_EFFECTS},
	{_T("CAR"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// キャレット		// 2006.12.07 ryoji
	{_T("IME"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// IMEキャレット	// 2006.12.07 ryoji
	{_T("CBK"), COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{_T("UND"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{_T("CVL"), COLOR_ATTRIB_NO_BACK | ( COLOR_ATTRIB_NO_EFFECTS & ~COLOR_ATTRIB_NO_BOLD )}, // 2007.09.09 Moca カーソル位置縦線
	{_T("LNO"), 0},
	{_T("MOD"), 0},
	{_T("TAB"), 0},
	{_T("SPC"), 0},	//2002.04.28 Add By KK
	{_T("ZEN"), 0},
	{_T("CTL"), 0},
	{_T("EOL"), 0},
	{_T("RAP"), 0},
	{_T("VER"), 0},  // 2005.11.08 Moca 指定桁縦線
	{_T("EOF"), 0},
	{_T("NUM"), 0},	//@@@ 2001.02.17 by MIK 半角数値の強調
	{_T("BRC"), 0},	//対括弧	// 02/09/18 ai Add
	{_T("SEL"), 0},
	{_T("FND"), 0},
	{_T("FN2"), 0},
	{_T("FN3"), 0},
	{_T("FN4"), 0},
	{_T("FN5"), 0},
	{_T("CMT"), 0},
	{_T("SQT"), 0},
	{_T("WQT"), 0},
	{_T("URL"), 0},
	{_T("KW1"), 0},
	{_T("KW2"), 0},
	{_T("KW3"), 0},	//@@@ 2003.01.13 by MIK 強調キーワード3-10
	{_T("KW4"), 0},
	{_T("KW5"), 0},
	{_T("KW6"), 0},
	{_T("KW7"), 0},
	{_T("KW8"), 0},
	{_T("KW9"), 0},
	{_T("KWA"), 0},
	{_T("RK1"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK2"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK3"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK4"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK5"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK6"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK7"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK8"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK9"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RKA"), 0},	//@@@ 2001.11.17 add MIK
	{_T("DFA"), 0},	//DIFF追加	//@@@ 2002.06.01 MIK
	{_T("DFC"), 0},	//DIFF変更	//@@@ 2002.06.01 MIK
	{_T("DFD"), 0},	//DIFF削除	//@@@ 2002.06.01 MIK
	{_T("MRK"), 0},	//ブックマーク	// 02/10/16 ai Add
	{_T("LAST"), 0}	// Not Used
};



/*
 * カラー名からインデックス番号に変換する
 */
int GetColorIndexByName( const TCHAR *name )
{
	int	i;
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		if( _tcscmp( name, g_ColorAttributeArr[i].szName ) == 0 ) return i;
	}
	return -1;
}

/*
 * インデックス番号からカラー名に変換する
 */
const TCHAR* GetColorNameByIndex( int index )
{
	return g_ColorAttributeArr[index].szName;
}
