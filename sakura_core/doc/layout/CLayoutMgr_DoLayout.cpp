/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "doc/CEditDoc.h" /// 2003/07/20 genta
#include "doc/layout/CLayoutMgr.h"
#include "doc/layout/CLayout.h"/// 2002/2/10 aroka
#include "doc/logic/CDocLine.h"/// 2002/2/10 aroka
#include "doc/logic/CDocLineMgr.h"// 2002/2/10 aroka
#include "charset/charcode.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "util/window.h"
#include "debug/CRunningTimer.h"
#include <atomic>
#include <thread>

//2008.07.27 kobake
static bool _GetKeywordLength(
	const CLayoutMgr&	cLayoutMgr,
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
	while(nPos<cLineStr.GetLength() && IS_KEYWORD_CHAR(cLineStr[nPos])){
		CLogicXInt nCharSize = CNativeW::GetSizeOfChar( cLineStr, nPos );
		CLayoutInt k = cLayoutMgr.GetLayoutXOfChar(cLineStr, nPos);

		nWordLen += nCharSize;
		nWordKetas+=k;
		nPos += nCharSize;
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

/*!
	@brief 行頭禁則の処理位置であるか調べる

	@param[in] nRest 現在行に挿入可能な文字の総幅
	@param[in] nCharKetas 現在の位置にある文字の幅
	@param[in] nCharKetas2 次の位置にある文字の幅
	@return 処理が必要な位置ならばtrue
*/
[[nodiscard]] static bool _IsKinsokuPosHead( CLayoutInt nRest, CLayoutInt nCharKetas, CLayoutInt nCharKetas2 )
{
	return nRest < nCharKetas + nCharKetas2;
}

/*!
	@brief 行末禁則の処理位置であるか調べる

	@param[in] nRest 現在行に挿入可能な文字の総幅
	@param[in] nCharKetas 現在の位置にある文字の幅
	@param[in] nCharKetas2 次の位置にある文字の幅
	@return 処理が必要な位置ならばtrue
*/
[[nodiscard]] static bool _IsKinsokuPosTail( CLayoutInt nRest, CLayoutInt nCharKetas, CLayoutInt nCharKetas2 )
{
	return nRest < nCharKetas + nCharKetas2;
}

/*!
	@brief 句読点ぶら下げの処理位置であるか調べる

	@param[in] nRest 現在行に挿入可能な文字の総幅
	@param[in] nCharChars 現在の位置にある文字の幅
	@return 処理が必要な位置ならばtrue
*/
[[nodiscard]] static bool _IsKinsokuPosKuto( CLayoutInt nRest, CLayoutInt nCharChars )
{
	return nRest < nCharChars;
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
		this->colorPrev,
		this->nIndent,
		this->nPosX,
		this->exInfoPrev.DetachColorInfo()
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           部品                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CLayoutMgr::_DoKinsokuSkip(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	if( KINSOKU_TYPE_NONE != pWork->eKinsokuType )
	{
		//禁則処理の最後尾に達したら禁則処理中を解除する
		if( pWork->nPos >= pWork->nWordBgn + pWork->nWordLen )
		{
			if( pWork->eKinsokuType == KINSOKU_TYPE_KINSOKU_KUTO && pWork->nPos == pWork->nWordBgn + pWork->nWordLen )
			{
				int	nEol = pWork->pcDocLine->GetEol().GetLen();

				if( ! (m_pTypeConfig->m_bKinsokuRet && (pWork->nPos == pWork->cLineStr.GetLength() - nEol) && nEol ) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
				{
					(this->*pfOnLine)(pWork);
				}
			}

			pWork->nWordLen = CLogicInt(0);
			pWork->eKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
		}
		return true;
	}
	else{
		return false;
	}
}

void CLayoutMgr::_DoWordWrap(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	if( pWork->eKinsokuType == KINSOKU_TYPE_NONE )
	{
		/* 英単語の先頭か */
		if( pWork->nPos >= pWork->nBgn && IS_KEYWORD_CHAR(pWork->cLineStr.At(pWork->nPos)) ){
			// キーワード長を取得
			CLayoutInt nWordKetas = CLayoutInt(0);
			_GetKeywordLength( *this,
				pWork->cLineStr, pWork->nPos,
				&pWork->nWordBgn, &pWork->nWordLen, &nWordKetas
			);

			pWork->eKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK

			if( pWork->nPosX+nWordKetas >= GetMaxLineLayout() && pWork->nPos - pWork->nBgn > 0 )
			{
				(this->*pfOnLine)(pWork);
			}
		}
	}
}

void CLayoutMgr::_DoKutoBurasage(SLayoutWork* pWork) const
{
	// 現在位置が行末付近で禁則処理の実行中でないこと
	if( GetMaxLineLayout() - pWork->nPosX < 2 * GetWidthPerKeta() && pWork->eKinsokuType == KINSOKU_TYPE_NONE )
	{
		// 2007.09.07 kobake   レイアウトとロジックの区別
		CLayoutInt nCharKetas = GetLayoutXOfChar( pWork->cLineStr, pWork->nPos );

		if( _IsKinsokuPosKuto( GetMaxLineLayout() - pWork->nPosX, nCharKetas ) && IsKinsokuKuto( pWork->cLineStr.At( pWork->nPos ) ) )
		{
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = CNativeW::GetSizeOfChar( pWork->cLineStr, pWork->nPos );
			pWork->eKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
		}
	}
}

void CLayoutMgr::_DoGyotoKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	// 現在位置が行末付近かつ行頭ではなく、禁則処理の実行中でないこと
	if( (pWork->nPos+1 < pWork->cLineStr.GetLength())	// 2007.02.17 ryoji 追加
	 && ( GetMaxLineLayout() - pWork->nPosX < 4 * GetWidthPerKeta() )
	 && ( pWork->nPosX > pWork->nIndent )	//	2004.04.09 pWork->nPosXの解釈変更のため，行頭チェックも変更
	 && (pWork->eKinsokuType == KINSOKU_TYPE_NONE) )
	{
		// 2007.09.07 kobake   レイアウトとロジックの区別
		CLogicXInt nCharSize = CNativeW::GetSizeOfChar( pWork->cLineStr, pWork->nPos );
		CLayoutXInt nCharKetas1 = GetLayoutXOfChar( pWork->cLineStr, pWork->nPos );
		CLayoutXInt nCharKetas2 = GetLayoutXOfChar( pWork->cLineStr, pWork->nPos + nCharSize );

		if( _IsKinsokuPosHead( GetMaxLineLayout() - pWork->nPosX, nCharKetas1, nCharKetas2 )
		 && IsKinsokuHead( pWork->cLineStr.At( pWork->nPos + nCharSize ) )
		 && !IsKinsokuHead( pWork->cLineStr[ pWork->nPos ] )		// 1字前が行頭禁則の対象でないこと
		 && !IsKinsokuKuto( pWork->cLineStr[ pWork->nPos ] ) )	// 1字前が句読点ぶら下げの対象でないこと
		{
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = nCharSize + CNativeW::GetSizeOfChar( pWork->cLineStr, pWork->nPos + nCharSize );
			pWork->eKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;

			(this->*pfOnLine)(pWork);
		}
	}
}

void CLayoutMgr::_DoGyomatsuKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	// 現在位置が行末付近かつ行頭ではなく、禁則処理の実行中でないこと
	if( (pWork->nPos+1 < pWork->cLineStr.GetLength())	// 2007.02.17 ryoji 追加
	 && ( GetMaxLineLayout() - pWork->nPosX < 4 * GetWidthPerKeta() )
	 && ( pWork->nPosX > pWork->nIndent )	//	2004.04.09 pWork->nPosXの解釈変更のため，行頭チェックも変更
	 && (pWork->eKinsokuType == KINSOKU_TYPE_NONE) )
	{
		CLogicXInt nCharSize = CNativeW::GetSizeOfChar( pWork->cLineStr, pWork->nPos );
		CLayoutXInt nCharKetas1 = GetLayoutXOfChar( pWork->cLineStr, pWork->nPos );
		CLayoutXInt nCharKetas2 = GetLayoutXOfChar( pWork->cLineStr, pWork->nPos + nCharSize );

		if( _IsKinsokuPosTail( GetMaxLineLayout() - pWork->nPosX, nCharKetas1, nCharKetas2 ) && IsKinsokuTail( pWork->cLineStr[ pWork->nPos ] ) )
		{
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = nCharSize;
			pWork->eKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;

			(this->*pfOnLine)(pWork);
		}
	}
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
	CLogicInt nLength = pWork->cLineStr.GetLength() - CLogicInt(nEol_1);

	if(pWork->pcColorStrategy)pWork->pcColorStrategy->InitStrategyStatus();
	CColorStrategyPool& color = *CColorStrategyPool::getInstance();

	const bool bCheckColorEnabled = color.HasRangeBasedColorStrategies();
	const bool bKinsokuEnabled = m_pTypeConfig->m_bWordWrap
		|| m_pTypeConfig->m_bKinsokuKuto
		|| m_pTypeConfig->m_bKinsokuHead
		|| m_pTypeConfig->m_bKinsokuTail;

	//1ロジック行を消化するまでループ
	while( pWork->nPos < nLength ){
		// インデント幅は_OnLineで計算済みなのでここからは削除

		// 禁則関係の設定がONかつ現在禁則処理中でなければ次の禁則の開始をチェック
		if( bKinsokuEnabled && !_DoKinsokuSkip(pWork, pfOnLine) ){
			// 英文ワードラップをする
			if( m_pTypeConfig->m_bWordWrap ){
				_DoWordWrap(pWork, pfOnLine);
			}

			// 句読点のぶらさげ
			if( m_pTypeConfig->m_bKinsokuKuto ){
				_DoKutoBurasage(pWork);
			}

			// 行頭禁則
			if( m_pTypeConfig->m_bKinsokuHead ){
				_DoGyotoKinsoku(pWork, pfOnLine);
			}

			// 行末禁則
			if( m_pTypeConfig->m_bKinsokuTail ){
				_DoGyomatsuKinsoku(pWork, pfOnLine);
			}
		}

		if( bCheckColorEnabled ){
			// 範囲を持つ色分けの開始終了チェック
			color.CheckColorMODE( &pWork->pcColorStrategy, pWork->nPos, pWork->cLineStr );
		}

		const auto& ch = pWork->cLineStr[pWork->nPos];
		CLayoutInt nCharKetas {0};
		if( ch == WCODE::TAB || (ch == L',' && m_tsvInfo.m_nTsvMode == TSV_MODE_CSV) ){
			nCharKetas = GetActualTsvSpace( pWork->nPosX, ch );
		}else{
			nCharKetas = GetLayoutXOfChar( pWork->cLineStr, pWork->nPos );
		}

		if( pWork->nPosX + nCharKetas > GetMaxLineLayout() ){
			if( pWork->eKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
			{
				if( ! (m_pTypeConfig->m_bKinsokuRet && (pWork->nPos == pWork->cLineStr.GetLength() - nEol) && nEol) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
				{
					(this->*pfOnLine)(pWork);
					continue;
				}
			}
		}
		pWork->nPos += CNativeW::GetSizeOfChar( pWork->cLineStr, pWork->nPos );
		pWork->nPosX += nCharKetas;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       本処理(全体)                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CLayoutMgr::_OnLine1(SLayoutWork* pWork)
{
	AddLineBottom( pWork->_CreateLayout(this) );
	pWork->pLayout = m_pLayoutBot;
	pWork->colorPrev = CColorStrategy::GetStrategyColorSafe(pWork->pcColorStrategy);
	pWork->exInfoPrev.SetColorInfo(CColorStrategy::GetStrategyColorInfoSafe(pWork->pcColorStrategy));
	pWork->nBgn = pWork->nPos;
	// 2004.03.28 Moca pWork->nPosXはインデント幅を含むように変更(TAB位置調整のため)
	pWork->nPosX = pWork->nIndent = (this->*m_getIndentOffset)( pWork->pLayout );
}

/*!
	現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します

	@date 2004.04.03 Moca TABが使われると折り返し位置がずれるのを防ぐため，
		nPosXがインデントを含む幅を保持するように変更．m_nMaxLineKetasは
		固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．
*/
void CLayoutMgr::_DoLayout(bool bBlockingHook)
{
	MY_RUNNINGTIMER( cRunningTimer, L"CLayoutMgr::_DoLayout" );

	_Empty();
	Init();

	const CLogicInt nAllLineCount = m_pcDocLineMgr->GetLineCount();

	int nWorkerThreadCount = 0;
	if (nAllLineCount < 1000) {
		// 行数が多くなければマルチスレッド処理するまでもない
		nWorkerThreadCount = 0;
	} else if (CColorStrategyPool::getInstance()->HasRangeBasedColorStrategies()) {
		// 行をまたぐ可能性のある色分けが有効の場合、途中で処理単位が分割されて
		// しまうと色分けがおかしくなってしまうためやむなくマルチスレッド処理の対象外とする
		nWorkerThreadCount = 0;
	} else {
		nWorkerThreadCount = (std::max)(1, (int)std::thread::hardware_concurrency()) - 1;
	}

	// 末尾側の行から順番にワーカースレッドに割り当てていき最後 (先頭行～) はメインスレッドで実行
	// nWorkerThreadCount = 0 の場合は全行をメインスレッドが実行
	//
	// 1000行のテキストを10スレッドで処理する場合のイメージ：
	// 行番号
	//    1 ┐
	//  ... ├ メインスレッドで処理 (i = 0)
	//  100 ┘
	//  101 ┐
	//  ... ├ ワーカースレッド1で処理 (i = 1)
	//  200 ┘
	//  ... ...
	//  901 ┐
	//  ... ├ ワーカースレッド9で処理 (i = 9) ↑こっちからループ
	// 1000 ┘

	std::vector<std::thread> vecWorkerThreads;
	std::vector<CLayoutMgr> vecWorkerLayoutMgrs(nWorkerThreadCount);

	std::atomic<bool> bCanceled = false;
	auto pbCanceled = bBlockingHook ? &bCanceled : nullptr;

	int nLineIndexEnd = nAllLineCount;
	const CDocLine* pDocLineEnd = nullptr;
	for (int i = nWorkerThreadCount; i >= 0; i--) {
		const auto nLineIndexBegin = (int)((double)nAllLineCount / (nWorkerThreadCount + 1) * i);
		const int nLineCount = nLineIndexEnd - nLineIndexBegin;
		CDocLine* pDocLineBegin = m_pcDocLineMgr->GetLine(CLogicInt(nLineIndexBegin));

		if (i == 0) {
			// 最後はメインスレッドで処理
			_DoLayoutSub(pDocLineBegin, pDocLineEnd, nLineIndexBegin, nLineCount, pbCanceled);
		} else {
			// ワーカースレッドを起こして処理
			auto pcLayoutMgr = &vecWorkerLayoutMgrs[nWorkerThreadCount - i];
			pcLayoutMgr->_Prepare(*this);
			vecWorkerThreads.emplace_back([pcLayoutMgr, pDocLineBegin, pDocLineEnd, nLineIndexBegin, nLineCount, pbCanceled]{
				pcLayoutMgr->_DoLayoutSub(pDocLineBegin, pDocLineEnd, nLineIndexBegin, nLineCount, pbCanceled);
			});
		}
		nLineIndexEnd = nLineIndexBegin;
		pDocLineEnd = pDocLineBegin;
	}

	// 各々の結果をまとめる
	// 処理が途中で中断されていた場合でもワーカースレッドのjoinは必要なのでここはやる
	for (int i = (nWorkerThreadCount - 1); i >= 0; i--) {
		vecWorkerThreads[i].join();
		_AppendAsMove(vecWorkerLayoutMgrs[i]);
	}
}

/*!
	@brief 指定された行範囲をレイアウトします
*/
void CLayoutMgr::_DoLayoutSub(CDocLine* pDocLineBegin, const CDocLine* pDocLineEnd, int nLineIndex, int nLineCount, std::atomic<bool>* pbCanceled)
{
	const int nListenerCount = GetListenerCount();

	if( nListenerCount != 0 ){
		if( nLineIndex == 0 ){
			// メインスレッドは進捗更新とユーザー操作受け付け
			NotifyProgress(0);
			if( pbCanceled != nullptr && !::BlockingHook( NULL )){
				// 中断された
				pbCanceled->store(true);
				return;
			}
		}else{
			// ワーカースレッドは中断検知のみ
			if( pbCanceled != nullptr && pbCanceled->load() ){ return; }
		}
	}

	//	Nov. 16, 2002 genta
	//	折り返し幅 <= TAB幅のとき無限ループするのを避けるため，
	//	TABが折り返し幅以上の時はTAB=4としてしまう
	//	折り返し幅の最小値=10なのでこの値は問題ない
	if( GetTabSpaceKetas() >= GetMaxLineKetas() ){
		m_nTabSpace = CKetaXInt(4);
	}

	SLayoutWork	_sWork;
	SLayoutWork* pWork = &_sWork;
	pWork->pcDocLine				= pDocLineBegin;
	pWork->pLayout					= NULL;
	pWork->pcColorStrategy			= NULL;
	pWork->colorPrev				= COLORIDX_DEFAULT;
	pWork->nCurLine					= CLogicInt( nLineIndex );

	constexpr DWORD userInterfaceInterval = 33;
	ULONGLONG prevTime = GetTickCount64() + userInterfaceInterval;

	while( pWork->pcDocLine != pDocLineEnd ){
		pWork->cLineStr		= pWork->pcDocLine->GetStringRefWithEOL();
		pWork->eKinsokuType	= KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
		pWork->nBgn			= CLogicInt(0);
		pWork->nPos			= CLogicInt(0);
		pWork->nWordBgn		= CLogicInt(0);
		pWork->nWordLen		= CLogicInt(0);
		pWork->nPosX		= CLayoutInt(0);	// 表示上のX位置
		pWork->nIndent		= CLayoutInt(0);	// インデント幅

		_MakeOneLine(pWork, &CLayoutMgr::_OnLine1);

		if( pWork->nPos - pWork->nBgn > 0 ){
// 2002/03/13 novice
			AddLineBottom( pWork->_CreateLayout(this) );
			pWork->colorPrev = CColorStrategy::GetStrategyColorSafe(pWork->pcColorStrategy);
			pWork->exInfoPrev.SetColorInfo(CColorStrategy::GetStrategyColorInfoSafe(pWork->pcColorStrategy));
		}

		// 次の行へ
		pWork->nCurLine++;
		pWork->pcDocLine = pWork->pcDocLine->GetNextLine();

		// 処理中のユーザー操作を可能にする
		if( nListenerCount != 0 && 0 < nLineCount ) {
			if( nLineIndex == 0 ){
				const ULONGLONG currTime = GetTickCount64();
				const ULONGLONG diffTime = currTime - prevTime;
				if( diffTime >= userInterfaceInterval ){
					prevTime = currTime;
					NotifyProgress( ::MulDiv( pWork->nCurLine, 100, nLineCount ) );
					if( pbCanceled != nullptr && !::BlockingHook( NULL ) ){
						pbCanceled->store( true );
						return;
					}
				}
			}else{
				if( pbCanceled != nullptr && pbCanceled->load() ){ return; }
			}
		}

// 2002/03/13 novice
	}

	// 2011.12.31 Botの色分け情報は最後に設定
	m_nLineTypeBot = CColorStrategy::GetStrategyColorSafe(pWork->pcColorStrategy);
	m_cLayoutExInfoBot.SetColorInfo(CColorStrategy::GetStrategyColorInfoSafe(pWork->pcColorStrategy));

	m_nPrevReferLine = CLayoutInt(0);
	m_pLayoutPrevRefer = NULL;

	if( nListenerCount != 0 ){
		if( nLineIndex == 0 ){
			NotifyProgress(0);
			if( pbCanceled != nullptr && !::BlockingHook( NULL ) ){
				pbCanceled->store(true);
				return;
			}
		}else{
			if( pbCanceled != nullptr && pbCanceled->load() ){ return; }
		}
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
		pWork->pLayout->SetColorTypePrev(pWork->colorPrev);
		pWork->pLayout->GetLayoutExInfo()->SetColorInfo(pWork->exInfoPrev.DetachColorInfo());
		(*pWork->pnExtInsLineNum)++;								//	再描画してほしい行数+1
	}
	else {
		pWork->pLayout = InsertLineNext( pWork->pLayout, pWork->_CreateLayout(this) );
	}
	pWork->colorPrev = CColorStrategy::GetStrategyColorSafe(pWork->pcColorStrategy);
	pWork->exInfoPrev.SetColorInfo(CColorStrategy::GetStrategyColorInfoSafe(pWork->pcColorStrategy));

	pWork->nBgn = pWork->nPos;
	// 2004.03.28 Moca pWork->nPosXはインデント幅を含むように変更(TAB位置調整のため)
	pWork->nPosX = pWork->nIndent = (this->*m_getIndentOffset)( pWork->pLayout );
	if( ( pWork->ptDelLogicalFrom.GetY2() == pWork->nCurLine && pWork->ptDelLogicalFrom.GetX2() < pWork->nPos ) ||
		( pWork->ptDelLogicalFrom.GetY2() < pWork->nCurLine )
	){
		(pWork->nModifyLayoutLinesNew)++;
	}
}

/*!
	指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする

	@date 2002.10.07 YAZAKI rename from "DoLayout3_New"
	@date 2004.04.03 Moca TABが使われると折り返し位置がずれるのを防ぐため，
		pWork->nPosXがインデントを含む幅を保持するように変更．m_nMaxLineKetasは
		固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．
	@date 2009.08.28 nasukoji	テキスト最大幅の算出に対応

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
	CLayoutColorInfo*	colorInfo,
	const CalTextWidthArg*	pctwArg,
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
	pWork->pcColorStrategy			= CColorStrategyPool::getInstance()->GetStrategyByColor(nCurrentLineType);
	pWork->colorPrev				= nCurrentLineType;
	pWork->exInfoPrev.SetColorInfo(colorInfo);
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

	if(pWork->pcColorStrategy){
		pWork->pcColorStrategy->InitStrategyStatus();
		pWork->pcColorStrategy->SetStrategyColorInfo(colorInfo);
	}

	while( NULL != pWork->pcDocLine ){
		pWork->cLineStr		= pWork->pcDocLine->GetStringRefWithEOL();
		pWork->eKinsokuType	= KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
		pWork->nBgn			= CLogicInt(0);
		pWork->nPos			= CLogicInt(0);
		pWork->nWordBgn		= CLogicInt(0);
		pWork->nWordLen		= CLogicInt(0);
		pWork->nPosX		= CLayoutInt(0);			// 表示上のX位置
		pWork->nIndent		= CLayoutInt(0);			// インデント幅

		_MakeOneLine(pWork, &CLayoutMgr::_OnLine2);

		if( pWork->nPos - pWork->nBgn > 0 ){
// 2002/03/13 novice
			//@@@ 2002.09.23 YAZAKI 最適化
			_OnLine2(pWork);
		}

		nLineNumWork++;
		pWork->nCurLine++;

		/* 目的の行数(nLineNum)に達したか、または通り過ぎた（＝行数が増えた）か確認 */
		//@@@ 2002.09.23 YAZAKI 最適化
		if( nLineNumWork >= nLineNum ){
			if( pWork->pLayout && pWork->pLayout->GetNextLayout() ){
				if( pWork->colorPrev != pWork->pLayout->GetNextLayout()->GetColorTypePrev() ){
					//	COMMENTMODEが異なる行が増えましたので、次の行→次の行と更新していきます。
					pWork->bNeedChangeCOMMENTMODE = true;
				}else if( pWork->exInfoPrev.GetColorInfo() && pWork->pLayout->GetNextLayout()->GetColorInfo()
				 && !pWork->exInfoPrev.GetColorInfo()->IsEqual(pWork->pLayout->GetNextLayout()->GetColorInfo()) ){
					pWork->bNeedChangeCOMMENTMODE = true;
				}else if( pWork->exInfoPrev.GetColorInfo() && NULL == pWork->pLayout->GetNextLayout()->GetColorInfo() ){
					pWork->bNeedChangeCOMMENTMODE = true;
				}else if( NULL == pWork->exInfoPrev.GetColorInfo() && pWork->pLayout->GetNextLayout()->GetColorInfo() ){
					pWork->bNeedChangeCOMMENTMODE = true;
				}else{
					break;
				}
			}else{
				break;	//	while( NULL != pWork->pcDocLine ) 終了
			}
		}
		pWork->pcDocLine = pWork->pcDocLine->GetNextLine();
// 2002/03/13 novice
	}

	// 2004.03.28 Moca EOFだけの論理行の直前の行の色分けが確認・更新された
	if( pWork->nCurLine == m_pcDocLineMgr->GetLineCount() ){
		m_nLineTypeBot = CColorStrategy::GetStrategyColorSafe(pWork->pcColorStrategy);
		m_cLayoutExInfoBot.SetColorInfo(CColorStrategy::GetStrategyColorInfoSafe(pWork->pcColorStrategy));
	}

	// 2009.08.28 nasukoji	テキストが編集されたら最大幅を算出する
	CalculateTextWidth_Range(pctwArg);

// 1999.12.22 レイアウト情報がなくなる訳ではないので
//	m_nPrevReferLine = 0;
//	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	return pWork->nModifyLayoutLinesNew;
}

/*!
	@brief テキストが編集されたら最大幅を算出する

	@param[in] pctwArg テキスト最大幅算出用構造体

	@note 「折り返さない」選択時のみテキスト最大幅を算出する．
	      編集された行の範囲について算出する（下記を満たす場合は全行）
	      　削除行なし時：最大幅の行を行頭以外にて改行付きで編集した
	      　削除行あり時：最大幅の行を含んで編集した
	      pctwArg->nDelLines が負数の時は削除行なし．

	@date 2009.08.28 nasukoji	新規作成
*/
void CLayoutMgr::CalculateTextWidth_Range( const CalTextWidthArg* pctwArg )
{
	if( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){	// 「折り返さない」
		CLayoutInt	nCalTextWidthLinesFrom(0);	// テキスト最大幅の算出開始レイアウト行
		CLayoutInt	nCalTextWidthLinesTo(0);	// テキスト最大幅の算出終了レイアウト行
		BOOL bCalTextWidth        = TRUE;		// テキスト最大幅の算出要求をON
		CLayoutInt nInsLineNum    = m_nLines - pctwArg->nAllLinesOld;		// 追加削除行数

		// 削除行なし時：最大幅の行を行頭以外にて改行付きで編集した
		// 削除行あり時：最大幅の行を含んで編集した

		if(( pctwArg->nDelLines < CLayoutInt(0)  && Int(m_nTextWidth) &&
		     Int(nInsLineNum) && Int(pctwArg->ptLayout.x) && m_nTextWidthMaxLine == pctwArg->ptLayout.y )||
		   ( pctwArg->nDelLines >= CLayoutInt(0) && Int(m_nTextWidth) &&
		     pctwArg->ptLayout.y <= m_nTextWidthMaxLine && m_nTextWidthMaxLine <= pctwArg->ptLayout.y + pctwArg->nDelLines ))
		{
			// 全ラインを走査する
			nCalTextWidthLinesFrom = -1;
			nCalTextWidthLinesTo   = -1;
		}else if( Int(nInsLineNum) || Int(pctwArg->bInsData) ){		// 追加削除行 または 追加文字列あり
			// 追加削除行のみを走査する
			nCalTextWidthLinesFrom = pctwArg->ptLayout.y;

			// 最終的に編集された行数（3行削除2行追加なら2行追加）
			// 　1行がMAXLINEKETASを超える場合行数が合わなくなるが、超える場合はその先の計算自体が
			// 　不要なので計算を省くためこのままとする。
			CLayoutInt nEditLines = nInsLineNum + ((pctwArg->nDelLines > 0) ? pctwArg->nDelLines : CLayoutInt(0));
			nCalTextWidthLinesTo   = pctwArg->ptLayout.y + ((nEditLines > 0) ? nEditLines : CLayoutInt(0));

			// 最大幅の行が上下するのを計算
			if( Int(m_nTextWidth) && Int(nInsLineNum) && m_nTextWidthMaxLine >= pctwArg->ptLayout.y )
				m_nTextWidthMaxLine += nInsLineNum;
		}else{
			// 最大幅以外の行を改行を含まずに（1行内で）編集した
			bCalTextWidth = FALSE;		// テキスト最大幅の算出要求をOFF
		}

#if defined( _DEBUG )
		static int testcount = 0;
		testcount++;

		// テキスト最大幅を算出する
		if( bCalTextWidth ){
//			MYTRACE_W( L"CLayoutMgr::DoLayout_Range(%d) nCalTextWidthLinesFrom=%d nCalTextWidthLinesTo=%d\n", testcount, nCalTextWidthLinesFrom, nCalTextWidthLinesTo );
			CalculateTextWidth( FALSE, nCalTextWidthLinesFrom, nCalTextWidthLinesTo );
//			MYTRACE_W( L"CLayoutMgr::DoLayout_Range() m_nTextWidthMaxLine=%d\n", m_nTextWidthMaxLine );
		}else{
//			MYTRACE_W( L"CLayoutMgr::DoLayout_Range(%d) FALSE\n", testcount );
		}
#else
		// テキスト最大幅を算出する
		if( bCalTextWidth )
			CalculateTextWidth( FALSE, nCalTextWidthLinesFrom, nCalTextWidthLinesTo );
#endif
	}
}

/*!
	@brief otherの内容を元にレイアウト処理の準備をする

	@note _DoLayoutのマルチスレッド対応のために作成 (それ以外の用途での使用は想定していない)
*/
void CLayoutMgr::_Prepare( const CLayoutMgr& other )
{
	_Empty();
	Init();

	m_pcDocLineMgr			= other.m_pcDocLineMgr;
	m_tsvInfo				= other.m_tsvInfo;
	m_pcEditDoc				= other.m_pcEditDoc;

	// 設定関係
	m_pTypeConfig			= other.m_pTypeConfig;
	m_nMaxLineKetas			= other.m_nMaxLineKetas;
	m_nTabSpace				= other.m_nTabSpace;
	m_nCharLayoutXPerKeta	= other.m_nCharLayoutXPerKeta;
	m_nSpacing				= other.m_nSpacing;
	m_pszKinsokuHead_1		= other.m_pszKinsokuHead_1;
	m_pszKinsokuTail_1		= other.m_pszKinsokuTail_1;
	m_pszKinsokuKuto_1		= other.m_pszKinsokuKuto_1;
	m_getIndentOffset		= other.m_getIndentOffset;

	// メモリプールは共用する
	m_layoutMemRes			= other.m_layoutMemRes;

	// Initで初期化されないので念のため
	m_nTextWidth			= CLayoutInt(0);
	m_nTextWidthMaxLine		= CLayoutInt(0);
}

/*!
	@brief otherの内容を末尾に連結する (連結後はotherの内容は空になる)

	@note _DoLayoutのマルチスレッド対応のために作成 (それ以外の用途での使用は想定していない)
*/
void CLayoutMgr::_AppendAsMove( CLayoutMgr& other )
{
	if (other.GetTopLayout() == nullptr) { return; }

	// 双方向リンクをつなぐ
	other.GetTopLayout()->m_pPrev = m_pLayoutBot;
	if (m_pLayoutBot != nullptr) {
		m_pLayoutBot->m_pNext = other.GetTopLayout();
	}

	// 先頭/末尾を更新
	if (m_pLayoutTop == nullptr) {
		m_pLayoutTop = other.GetTopLayout();
	}
	m_pLayoutBot = other.GetBottomLayout();

	m_nLines += other.GetLineCount();

	// 所有権が移ったので空っぽにしておく
	other.Init();
}
