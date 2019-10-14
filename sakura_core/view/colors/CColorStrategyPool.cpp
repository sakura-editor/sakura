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
#include "view/colors/CColorStrategyPool.h"
#include "window/CEditWnd.h"

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
	m_vStrategies.push_back(new CColor_Heredoc);			// ヒアドキュメント
	m_vStrategies.push_back(new CColor_BlockComment(COLORIDX_BLOCK1));	// ブロックコメント
	m_vStrategies.push_back(new CColor_BlockComment(COLORIDX_BLOCK2));	// ブロックコメント2
	m_vStrategies.push_back(new CColor_LineComment);		// 行コメント
	m_vStrategies.push_back(new CColor_SingleQuote);		// シングルクォーテーション文字列
	m_vStrategies.push_back(new CColor_DoubleQuote);		// ダブルクォーテーション文字列
	m_vStrategies.push_back(new CColor_Url);				// URL
	m_vStrategies.push_back(new CColor_Numeric);			// 半角数字
	m_vStrategies.push_back(new CColor_KeywordSet);			// キーワードセット

	// 設定更新
	OnChangeSetting();
}

CColorStrategyPool::~CColorStrategyPool()
{
	SAFE_DELETE(m_pcSelectStrategy);
	SAFE_DELETE(m_pcFoundStrategy);
	m_vStrategiesDisp.clear();
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
	int size = (int)m_vStrategiesDisp.size();
	for(int i = 0; i < size; i++ ){
		if(m_vStrategiesDisp[i]->GetStrategyColor()==eColor){
			return m_vStrategiesDisp[i];
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

/*! 設定更新
*/
void CColorStrategyPool::OnChangeSetting(void)
{
	m_vStrategiesDisp.clear();

	m_pcSelectStrategy->Update();
	m_pcFoundStrategy->Update();
	int size = (int)m_vStrategies.size();
	for(int i = 0; i < size; i++){
		m_vStrategies[i]->Update();

		// 色分け表示対象であれば登録
		if( m_vStrategies[i]->Disp() ){
			m_vStrategiesDisp.push_back(m_vStrategies[i]);
		}
	}

	// CheckColorMODE 用
	m_pcHeredoc = static_cast<CColor_Heredoc*>(GetStrategyByColor(COLORIDX_HEREDOC));
	m_pcBlockComment1 = static_cast<CColor_BlockComment*>(GetStrategyByColor(COLORIDX_BLOCK1));	// ブロックコメント
	m_pcBlockComment2 = static_cast<CColor_BlockComment*>(GetStrategyByColor(COLORIDX_BLOCK2));	// ブロックコメント2
	m_pcLineComment = static_cast<CColor_LineComment*>(GetStrategyByColor(COLORIDX_COMMENT));	// 行コメント
	m_pcSingleQuote = static_cast<CColor_SingleQuote*>(GetStrategyByColor(COLORIDX_SSTRING));	// シングルクォーテーション文字列
	m_pcDoubleQuote = static_cast<CColor_DoubleQuote*>(GetStrategyByColor(COLORIDX_WSTRING));	// ダブルクォーテーション文字列

	// 色分けをしない場合に、処理をスキップできるように確認する
	const STypeConfig& type = CEditDoc::GetInstance(0)->m_cDocType.GetDocumentAttribute();
	EColorIndexType bSkipColorTypeTable[] = {
		COLORIDX_DIGIT,
		COLORIDX_COMMENT,
		COLORIDX_SSTRING,
		COLORIDX_WSTRING,
		COLORIDX_HEREDOC,
		COLORIDX_URL,
		COLORIDX_KEYWORD1,
		COLORIDX_KEYWORD2,
		COLORIDX_KEYWORD3,
		COLORIDX_KEYWORD4,
		COLORIDX_KEYWORD5,
		COLORIDX_KEYWORD6,
		COLORIDX_KEYWORD7,
		COLORIDX_KEYWORD8,
		COLORIDX_KEYWORD9,
		COLORIDX_KEYWORD10,
	};
	m_bSkipBeforeLayoutGeneral = true;
	int nKeyword1;
	int bUnuseKeyword = false;
	for(int n = 0; n < _countof(bSkipColorTypeTable); n++ ){
		if( COLORIDX_KEYWORD1 == bSkipColorTypeTable[n] ){
			nKeyword1 = n;
		}
		if( COLORIDX_KEYWORD1 <= bSkipColorTypeTable[n] && bSkipColorTypeTable[n] <= COLORIDX_KEYWORD10 ){
			if( type.m_nKeyWordSetIdx[n - nKeyword1] == -1 ){
				bUnuseKeyword = true; // -1以降は無効
			}
			if( !bUnuseKeyword && type.m_ColorInfoArr[bSkipColorTypeTable[n]].m_bDisp ){
				m_bSkipBeforeLayoutGeneral = false;
				break;
			}
		}else if( type.m_ColorInfoArr[bSkipColorTypeTable[n]].m_bDisp ){
			m_bSkipBeforeLayoutGeneral = false;
			break;
		}
	}
	if( m_bSkipBeforeLayoutGeneral ){
		if( type.m_bUseRegexKeyword ){
			m_bSkipBeforeLayoutGeneral = false;
		}
	}
	m_bSkipBeforeLayoutFound = true;
	for(int n = COLORIDX_SEARCH; n <= COLORIDX_SEARCHTAIL; n++ ){
		if( type.m_ColorInfoArr[n].m_bDisp ){
			m_bSkipBeforeLayoutFound = false;
			break;
		}
	}
}

bool CColorStrategyPool::IsSkipBeforeLayout()
{
	if( !m_bSkipBeforeLayoutGeneral ){
		return false;
	}
	if( !m_bSkipBeforeLayoutFound && m_pcView->m_bCurSrchKeyMark ){
		return false;
	}
	return true;
}

