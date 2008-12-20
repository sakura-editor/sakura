#include "stdafx.h"
#include <mbstring.h>
#include "CLayoutMgr.h"
#include "charset/charcode.h"
#include "debug/Debug.h"
#include <commctrl.h>
#include "debug/CRunningTimer.h"
#include "doc/CLayout.h"/// 2002/2/10 aroka
#include "doc/CDocLine.h"/// 2002/2/10 aroka
#include "doc/CDocLineMgr.h"// 2002/2/10 aroka
#include "mem/CMemory.h"/// 2002/2/10 aroka
#include "mem/CMemoryIterator.h"
#include "doc/CEditDoc.h" /// 2003/07/20 genta
#include "util/window.h"
#include "view/colors/CColorStrategy.h"


//レイアウト中の禁則タイプ	//@@@ 2002.04.20 MIK
#define	KINSOKU_TYPE_NONE			0	//なし
#define	KINSOKU_TYPE_WORDWRAP		1	//ワードラップ中
#define	KINSOKU_TYPE_KINSOKU_HEAD	2	//行頭禁則中
#define	KINSOKU_TYPE_KINSOKU_TAIL	3	//行末禁則中
#define	KINSOKU_TYPE_KINSOKU_KUTO	4	//句読点ぶら下げ中


//2008.07.27 kobake
static bool _GetKeywordLength(
	const CStringRef&	cLineStr,		//!< [in]
	CLogicInt			nPos,			//!< [in]
	CLogicInt*			p_nWordBgn,		//!< [out]
	CLogicInt*			p_nWordLen,		//!< [out]
	CLayoutInt*			p_nWordKetas	//!< [out]
)
{
	//キーワード長をカウントする
	CLogicInt nWordBgn = nPos;
	CLogicInt nWordLen = CLogicInt(0);
	CLayoutInt nWordKetas = CLayoutInt(0);
	while(nPos<cLineStr.GetLength() && IS_KEYWORD_CHAR(cLineStr.At(nPos))){
		CLayoutInt k = CNativeW::GetKetaOfChar( cLineStr, nPos);
		if(0 == k)k = CLayoutInt(1);

		nWordLen+=1;
		nWordKetas+=k;
		nPos++;
	}
	//結果
	if(nWordLen>0){
		*p_nWordBgn = nWordBgn;
		*p_nWordLen = nWordLen;
		*p_nWordKetas = nWordKetas;
		return true;
	}
	else{
		return false;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      部品ステータス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CLayout* CLayoutMgr::SLayoutWork::_CreateLayout(CLayoutMgr* mgr)
{
	return mgr->CreateLayout(
		this->pcDocLine,
		CLogicPoint(this->nBgn, this->nCurLine),
		this->nPos - this->nBgn,
		this->pcColorStrategy_Prev->GetStrategyColorSafe(),
		this->nIndent
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           部品                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CLayoutMgr::_DoKinsokuSkip(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	int	nEol = pWork->pcDocLine->GetEol().GetLen();

	if( KINSOKU_TYPE_NONE != pWork->nKinsokuType )
	{
		//禁則処理の最後尾に達したら禁則処理中を解除する
		if( pWork->nPos >= pWork->nWordBgn + pWork->nWordLen )
		{
			if( pWork->nKinsokuType == KINSOKU_TYPE_KINSOKU_KUTO && pWork->nPos == pWork->nWordBgn + pWork->nWordLen )
			{
				if( ! (m_sTypeConfig.m_bKinsokuRet && (pWork->nPos == pWork->cLineStr.GetLength() - nEol) && nEol ) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
				{
					(this->*pfOnLine)(pWork);
				}
			}

			pWork->nWordLen = CLogicInt(0);
			pWork->nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
		}
		return true;
	}
	else{
		return false;
	}
}

void CLayoutMgr::_DoWordWrap(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	if( m_sTypeConfig.m_bWordWrap	/* 英文ワードラップをする */
	 && pWork->nKinsokuType == KINSOKU_TYPE_NONE )
	{
		/* 英単語の先頭か */
		if( pWork->nPos >= pWork->nBgn && IS_KEYWORD_CHAR(pWork->cLineStr.At(pWork->nPos)) ){
			// キーワード長を取得
			CLayoutInt nWordKetas;
			_GetKeywordLength(
				pWork->cLineStr, pWork->nPos,
				&pWork->nWordBgn, &pWork->nWordLen, &nWordKetas
			);

			pWork->nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK

			if( pWork->nPosX+nWordKetas>=m_sTypeConfig.m_nMaxLineKetas && pWork->nPos-pWork->nBgn>0 )
			{
				(this->*pfOnLine)(pWork);
			}
		}
	}
}

void CLayoutMgr::_DoKutoBurasage(SLayoutWork* pWork)
{
	if( m_sTypeConfig.m_bKinsokuKuto && (m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX < 2) && (pWork->nKinsokuType == KINSOKU_TYPE_NONE) )
	{
		// 2007.09.07 kobake   レイアウトとロジックの区別
		CLayoutInt nCharKetas = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos );

		if( IsKinsokuPosKuto(m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX, nCharKetas) && IsKinsokuKuto( pWork->cLineStr.At(pWork->nPos) ) )
		{
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 1;
			pWork->nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
		}
	}
}

void CLayoutMgr::_DoGyotoKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	if( m_sTypeConfig.m_bKinsokuHead
	 && (m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX < 4)
	 && ( pWork->nPosX > pWork->nIndent )	//	2004.04.09 pWork->nPosXの解釈変更のため，行頭チェックも変更
	 && (pWork->nKinsokuType == KINSOKU_TYPE_NONE) )
	{
		// 2007.09.07 kobake   レイアウトとロジックの区別
		CLayoutInt nCharKetas2 = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos );
		CLayoutInt nCharKetas3 = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos+1 );

		if( IsKinsokuPosHead( m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX, nCharKetas2, nCharKetas3 )
		 && IsKinsokuHead( pWork->cLineStr.At(pWork->nPos+1) )
		 && ! IsKinsokuHead( pWork->cLineStr.At(pWork->nPos) )	//1文字前が行頭禁則でない
		 && ! IsKinsokuKuto( pWork->cLineStr.At(pWork->nPos) ) )	//句読点でない
		{
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 2;
			pWork->nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;

			(this->*pfOnLine)(pWork);
		}
	}
}

void CLayoutMgr::_DoGyomatsuKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	if( m_sTypeConfig.m_bKinsokuTail
	 && (m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX < 4)
	 && ( pWork->nPosX > pWork->nIndent )	//	2004.04.09 pWork->nPosXの解釈変更のため，行頭チェックも変更
	 && (pWork->nKinsokuType == KINSOKU_TYPE_NONE) )
	{	/* 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう) */
		CLayoutInt nCharKetas2 = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos );
		CLayoutInt nCharKetas3 = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos+1 );

		if( IsKinsokuPosTail(m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX, nCharKetas2, nCharKetas3) && IsKinsokuTail(pWork->cLineStr.At(pWork->nPos)) ){
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 1;
			pWork->nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
			
			(this->*pfOnLine)(pWork);
		}
	}
}

//折り返す場合はtrueを返す
bool CLayoutMgr::_DoTab(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	//	Sep. 23, 2002 genta せっかく作ったので関数を使う
	CLayoutInt nCharKetas = GetActualTabSpace( pWork->nPosX );
	if( pWork->nPosX + nCharKetas > m_sTypeConfig.m_nMaxLineKetas ){
		(this->*pfOnLine)(pWork);
		return true;
	}
	pWork->nPosX += nCharKetas;
	pWork->nPos += CLogicInt(1);
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          準処理                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CLayoutMgr::_MakeOneLine(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	int	nEol = pWork->pcDocLine->GetEol().GetLen(); //########そのうち不要になる
	int nEol_1 = nEol - 1;
	if( 0 >	nEol_1 ){
		nEol_1 = 0;
	}

	if(pWork->pcColorStrategy)pWork->pcColorStrategy->InitStrategyStatus();

	//1ロジック行を消化するまでループ
	while( pWork->nPos < pWork->cLineStr.GetLength() - CLogicInt(nEol_1) ){
		//	インデント幅の計算コストを下げるための方策
		if ( pWork->pLayout && pWork->pLayout != pWork->pLayoutCalculated && pWork->nBgn ){
			//	計算
			//	Oct, 1, 2002 genta Indentサイズを取得するように変更
			pWork->nIndent = (this->*m_getIndentOffset)( pWork->pLayout );

			//	計算済み
			pWork->pLayoutCalculated = pWork->pLayout;
		}

SEARCH_START:;
		//禁則処理中ならスキップする	@@@ 2002.04.20 MIK
		if(_DoKinsokuSkip(pWork, pfOnLine)){ }
		else{
			// ワードラップ処理
			_DoWordWrap(pWork, pfOnLine);

			// 句読点のぶらさげ
			_DoKutoBurasage(pWork);

			// 行頭禁則
			_DoGyotoKinsoku(pWork, pfOnLine);

			// 行末禁則
			_DoGyomatsuKinsoku(pWork, pfOnLine);
		}

		//@@@ 2002.09.22 YAZAKI
		bool bGotoSEARCH_START = CColorStrategyPool::Instance()->CheckColorMODE( &pWork->pcColorStrategy, pWork->nPos, pWork->cLineStr );
		if ( bGotoSEARCH_START )
			goto SEARCH_START;
		
		if( pWork->cLineStr.At(pWork->nPos) == WCODE::TAB ){
			if(_DoTab(pWork, pfOnLine)){
				continue;
			}
		}
		else{
			if( pWork->nPos>=pWork->cLineStr.GetLength() ){
				break;
			}
			// 2007.09.07 kobake   ロジック幅とレイアウト幅を区別
			CLayoutInt nCharKetas = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos );
//			if( 0 == nCharKetas ){				// 削除 サロゲートペア対策	2008/7/5 Uchi
//				nCharKetas = CLayoutInt(1);
//			}

			if( pWork->nPosX + nCharKetas > m_sTypeConfig.m_nMaxLineKetas ){
				if( pWork->nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
				{
					if( ! (m_sTypeConfig.m_bKinsokuRet && (pWork->nPos == pWork->cLineStr.GetLength() - nEol) && nEol) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
					{
						(this->*pfOnLine)(pWork);
						continue;
					}
				}
			}
			pWork->nPos+= 1;
			pWork->nPosX += nCharKetas;
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       本処理(全体)                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CLayoutMgr::_OnLine1(SLayoutWork* pWork)
{
	AddLineBottom( pWork->_CreateLayout(this) );
	m_nLineTypeBot = pWork->pcColorStrategy->GetStrategyColorSafe();
	pWork->pLayout = m_pLayoutBot;
	pWork->pcColorStrategy_Prev = pWork->pcColorStrategy;
	pWork->nBgn = pWork->nPos;
	// 2004.03.28 Moca pWork->nPosXはインデント幅を含むように変更(TAB位置調整のため)
	pWork->nPosX = pWork->nIndent = (this->*m_getIndentOffset)( pWork->pLayout );
	pWork->pLayoutCalculated = pWork->pLayout;
}

/*!
	現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します

	@date 2004.04.03 Moca TABが使われると折り返し位置がずれるのを防ぐため，
		nPosXがインデントを含む幅を保持するように変更．m_sTypeConfig.m_nMaxLineKetasは
		固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．
*/
void CLayoutMgr::_DoLayout()
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::_DoLayout" );

	/*	表示上のX位置
		2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
	*/
	int			nAllLineNum;

	if( GetListenerCount() != 0 ){
		NotifyProgress(0);
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( NULL ) )return;
	}

	_Empty();
	Init();
	
	//	Nov. 16, 2002 genta
	//	折り返し幅 <= TAB幅のとき無限ループするのを避けるため，
	//	TABが折り返し幅以上の時はTAB=4としてしまう
	//	折り返し幅の最小値=10なのでこの値は問題ない
	if( m_sTypeConfig.m_nTabSpace >= m_sTypeConfig.m_nMaxLineKetas ){
		m_sTypeConfig.m_nTabSpace = CLayoutInt(4);
	}

	nAllLineNum = m_pcDocLineMgr->GetLineCount();

	SLayoutWork	_sWork;
	SLayoutWork* pWork = &_sWork;
	pWork->pcDocLine				= m_pcDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr変更
	pWork->pLayout					= m_pLayoutBot;
	pWork->pcColorStrategy			= NULL;
	pWork->pcColorStrategy_Prev		= NULL;
	pWork->nCurLine					= CLogicInt(0);

	while( NULL != pWork->pcDocLine ){
		pWork->cLineStr		= pWork->pcDocLine->GetStringRefWithEOL();
		pWork->nKinsokuType	= KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
		pWork->nBgn			= CLogicInt(0);
		pWork->nPos			= CLogicInt(0);
		pWork->nWordBgn		= CLogicInt(0);
		pWork->nWordLen		= CLogicInt(0);
		pWork->nPosX		= CLayoutInt(0);	// 表示上のX位置
		pWork->nIndent		= CLayoutInt(0);	// インデント幅
		pWork->pLayoutCalculated	= NULL;		// インデント幅計算済みのCLayout.


		_MakeOneLine(pWork, &CLayoutMgr::_OnLine1);

		if( pWork->nPos - pWork->nBgn > 0 ){
// 2002/03/13 novice
			if( pWork->pcColorStrategy->GetStrategyColorSafe() == COLORIDX_COMMENT ){	/* 行コメントである */
				pWork->pcColorStrategy = NULL;
			}
			AddLineBottom( pWork->_CreateLayout(this) );
			m_nLineTypeBot = pWork->pcColorStrategy->GetStrategyColorSafe();
			pWork->pcColorStrategy_Prev = pWork->pcColorStrategy;
		}

		// 次の行へ
		pWork->nCurLine++;
		pWork->pcDocLine = pWork->pcDocLine->GetNextLine();
		
		// 処理中のユーザー操作を可能にする
		if( GetListenerCount()!=0 && 0 < nAllLineNum && 0 == ( pWork->nCurLine % 1024 ) ){
			NotifyProgress(pWork->nCurLine * 100 / nAllLineNum);
			if( !::BlockingHook( NULL ) )return;
		}

// 2002/03/13 novice
		if( pWork->pcColorStrategy_Prev->GetStrategyColorSafe() == COLORIDX_COMMENT ){	/* 行コメントである */
			pWork->pcColorStrategy_Prev = NULL;
		}
		pWork->pcColorStrategy = pWork->pcColorStrategy_Prev;
	}

	m_nPrevReferLine = CLayoutInt(0);
	m_pLayoutPrevRefer = NULL;

	if( GetListenerCount()!=0 ){
		NotifyProgress(0);
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( NULL ) )return;
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     本処理(範囲指定)                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CLayoutMgr::_OnLine2(SLayoutWork* pWork)
{
	//@@@ 2002.09.23 YAZAKI 最適化
	if( pWork->bNeedChangeCOMMENTMODE ){
		pWork->pLayout = pWork->pLayout->GetNextLayout();
		pWork->pLayout->SetColorTypePrev(pWork->pcColorStrategy_Prev->GetStrategyColorSafe());
		(*pWork->pnExtInsLineNum)++;								//	再描画してほしい行数+1
	}
	else {
		pWork->pLayout = InsertLineNext( pWork->pLayout, pWork->_CreateLayout(this) );
	}
	pWork->pcColorStrategy_Prev = pWork->pcColorStrategy;

	pWork->nBgn = pWork->nPos;
	// 2004.03.28 Moca pWork->nPosXはインデント幅を含むように変更(TAB位置調整のため)
	pWork->nPosX = pWork->nIndent = (this->*m_getIndentOffset)( pWork->pLayout );
	pWork->pLayoutCalculated = pWork->pLayout;
	if( ( pWork->ptDelLogicalFrom.GetY2() == pWork->nCurLine && pWork->ptDelLogicalFrom.GetX2() < pWork->nPos ) ||
		( pWork->ptDelLogicalFrom.GetY2() < pWork->nCurLine )
	){
		(pWork->nModifyLayoutLinesNew)++;;
	}
}

/*!
	指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
	
	@date 2002.10.07 YAZAKI rename from "DoLayout3_New"
	@date 2004.04.03 Moca TABが使われると折り返し位置がずれるのを防ぐため，
		pWork->nPosXがインデントを含む幅を保持するように変更．m_sTypeConfig.m_nMaxLineKetasは
		固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．

	@note 2004.04.03 Moca
		_DoLayoutとは違ってレイアウト情報がリスト中間に挿入されるため，
		挿入後にm_nLineTypeBotへコメントモードを指定してはならない
		代わりに最終行のコメントモードを終了間際に確認している．
*/
CLayoutInt CLayoutMgr::DoLayout_Range(
	CLayout*		pLayoutPrev,
	CLogicInt		nLineNum,
	CLogicPoint		_ptDelLogicalFrom,
	EColorIndexType	nCurrentLineType,
	CLayoutInt*		_pnExtInsLineNum
)
{
	*_pnExtInsLineNum = CLayoutInt(0);

	CLogicInt	nLineNumWork = CLogicInt(0);

	// 2006.12.01 Moca 途中にまで再構築した場合にEOF位置がずれたまま
	//	更新されないので，範囲にかかわらず必ずリセットする．
	m_nEOFColumn = CLayoutInt(-1);
	m_nEOFLine = CLayoutInt(-1);

	SLayoutWork _sWork;
	SLayoutWork* pWork = &_sWork;
	pWork->pLayout					= pLayoutPrev;
	pWork->pcColorStrategy			= CColorStrategyPool::Instance()->GetStrategyByColor(nCurrentLineType);
	pWork->pcColorStrategy_Prev		= pWork->pcColorStrategy;
	pWork->bNeedChangeCOMMENTMODE	= false;
	if( NULL == pWork->pLayout ){
		pWork->nCurLine = CLogicInt(0);
	}else{
		pWork->nCurLine = pWork->pLayout->GetLogicLineNo() + CLogicInt(1);
	}
	pWork->pcDocLine				= m_pcDocLineMgr->GetLine( pWork->nCurLine );
	pWork->nModifyLayoutLinesNew	= CLayoutInt(0);
	//引数
	pWork->ptDelLogicalFrom		= _ptDelLogicalFrom;
	pWork->pnExtInsLineNum		= _pnExtInsLineNum;

	if(pWork->pcColorStrategy)pWork->pcColorStrategy->InitStrategyStatus();

	while( NULL != pWork->pcDocLine ){
		pWork->cLineStr		= pWork->pcDocLine->GetStringRefWithEOL();
		pWork->nKinsokuType	= KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
		pWork->nBgn			= CLogicInt(0);
		pWork->nPos			= CLogicInt(0);
		pWork->nWordBgn		= CLogicInt(0);
		pWork->nWordLen		= CLogicInt(0);
		pWork->nPosX		= CLayoutInt(0);			// 表示上のX位置
		pWork->nIndent		= CLayoutInt(0);			// インデント幅
		pWork->pLayoutCalculated	= pWork->pLayout;	// インデント幅計算済みのCLayout.

		_MakeOneLine(pWork, &CLayoutMgr::_OnLine2);

		if( pWork->nPos - pWork->nBgn > 0 ){
// 2002/03/13 novice
			if( pWork->pcColorStrategy->GetStrategyColorSafe() == COLORIDX_COMMENT ){	/* 行コメントである */
				pWork->pcColorStrategy = NULL;
			}
			//@@@ 2002.09.23 YAZAKI 最適化
			_OnLine2(pWork);
		}

		nLineNumWork++;
		pWork->nCurLine++;

		/* 目的の行数(nLineNum)に達したか、または通り過ぎた（＝行数が増えた）か確認 */
		//@@@ 2002.09.23 YAZAKI 最適化
		if( nLineNumWork >= nLineNum ){
			if( pWork->pLayout && pWork->pLayout->m_pNext 
				&& ( pWork->pcColorStrategy_Prev->GetStrategyColorSafe() != pWork->pLayout->m_pNext->GetColorTypePrev() )
			){
				//	COMMENTMODEが異なる行が増えましたので、次の行→次の行と更新していきます。
				pWork->bNeedChangeCOMMENTMODE = true;
			}else{
				break;	//	while( NULL != pWork->pcDocLine ) 終了
			}
		}
		pWork->pcDocLine = pWork->pcDocLine->GetNextLine();
// 2002/03/13 novice
		if( pWork->pcColorStrategy_Prev->GetStrategyColorSafe() == COLORIDX_COMMENT ){	/* 行コメントである */
			pWork->pcColorStrategy_Prev = NULL;
		}
		pWork->pcColorStrategy = pWork->pcColorStrategy_Prev;
	}


	// 2004.03.28 Moca EOFだけの論理行の直前の行の色分けが確認・更新された
	if( pWork->nCurLine == m_pcDocLineMgr->GetLineCount() ){
		m_nLineTypeBot = pWork->pcColorStrategy_Prev->GetStrategyColorSafe();
		// 2006.10.01 Moca 最終行が変更された。EOF位置情報を破棄する。
		m_nEOFColumn = CLayoutInt(-1);
		m_nEOFLine = CLayoutInt(-1);
	}

// 1999.12.22 レイアウト情報がなくなる訳ではないので
//	m_nPrevReferLine = 0;
//	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	return pWork->nModifyLayoutLinesNew;
}
