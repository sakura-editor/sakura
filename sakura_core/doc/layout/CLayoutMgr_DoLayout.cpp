/*! @file */
#include "StdAfx.h"
#include "doc/CEditDoc.h" /// 2003/07/20 genta
#include "doc/layout/CLayoutMgr.h"
#include "doc/layout/CLayout.h"/// 2002/2/10 aroka
#include "doc/logic/CDocLine.h"/// 2002/2/10 aroka
#include "doc/logic/CDocLineMgr.h"// 2002/2/10 aroka
#include "charset/charcode.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "view/colors/CColorStrategyPool.h"
#include "util/window.h"
#include "debug/CRunningTimer.h"

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
	while(nPos<cLineStr.GetLength() && IS_KEYWORD_CHAR(cLineStr.At(nPos))){
		CLayoutInt k = cLayoutMgr.GetLayoutXOfChar(cLineStr, nPos);
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
		this->colorPrev,
		this->nIndent,
		this->nPosX,
		this->exInfoPrev.DetachColorInfo()
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           部品                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CLayoutMgr::_DoWordWrap(SLayoutWork& sWork, PF_OnLine pfOnLine)
{
	if( sWork.eKinsokuType == KINSOKU_TYPE_NONE )
	{
		/* 英単語の先頭か */
		if( sWork.nPos >= sWork.nBgn && IS_KEYWORD_CHAR(sWork.cLineStr.At(sWork.nPos)) ){
			// キーワード長を取得
			CLayoutInt nWordKetas = CLayoutInt(0);
			_GetKeywordLength( *this,
				sWork.cLineStr, sWork.nPos,
				&sWork.nWordBgn, &sWork.nWordLen, &nWordKetas
			);

			sWork.eKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK

			if( sWork.nPosX+nWordKetas >= GetMaxLineLayout() && sWork.nPos - sWork.nBgn > 0 )
			{
				(this->*pfOnLine)(sWork);
			}
		}
	}
}

void CLayoutMgr::_DoKutoBurasage(SLayoutWork& sWork)
{
	if( (GetMaxLineLayout() - sWork.nPosX < 2) && (sWork.eKinsokuType == KINSOKU_TYPE_NONE) )
	{
		// 2007.09.07 kobake   レイアウトとロジックの区別
		CLayoutInt nCharKetas = GetLayoutXOfChar( sWork.cLineStr, sWork.nPos );

		if( IsKinsokuPosKuto(GetMaxLineLayout() - sWork.nPosX, nCharKetas) && IsKinsokuKuto( sWork.cLineStr.At(sWork.nPos) ) )
		{
			sWork.nWordBgn = sWork.nPos;
			sWork.nWordLen = 1;
			sWork.eKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
		}
	}
}

void CLayoutMgr::_DoGyotoKinsoku(SLayoutWork& sWork, PF_OnLine pfOnLine)
{
	if( (sWork.nPos+1 < sWork.cLineStr.GetLength())	// 2007.02.17 ryoji 追加
	 && (GetMaxLineLayout() - sWork.nPosX < 4)
	 && ( sWork.nPosX > sWork.nIndent )	//	2004.04.09 sWork.nPosXの解釈変更のため，行頭チェックも変更
	 && (sWork.eKinsokuType == KINSOKU_TYPE_NONE) )
	{
		// 2007.09.07 kobake   レイアウトとロジックの区別
		CLayoutInt nCharKetas2 = GetLayoutXOfChar( sWork.cLineStr, sWork.nPos );
		CLayoutInt nCharKetas3 = GetLayoutXOfChar( sWork.cLineStr, sWork.nPos+1 );

		if( IsKinsokuPosHead( GetMaxLineLayout() - sWork.nPosX, nCharKetas2, nCharKetas3 )
		 && IsKinsokuHead( sWork.cLineStr.At(sWork.nPos+1) )
		 && ! IsKinsokuHead( sWork.cLineStr.At(sWork.nPos) )	//1文字前が行頭禁則でない
		 && ! IsKinsokuKuto( sWork.cLineStr.At(sWork.nPos) ) )	//句読点でない
		{
			sWork.nWordBgn = sWork.nPos;
			sWork.nWordLen = 2;
			sWork.eKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;

			(this->*pfOnLine)(sWork);
		}
	}
}

void CLayoutMgr::_DoGyomatsuKinsoku(SLayoutWork& sWork, PF_OnLine pfOnLine)
{
	if( (sWork.nPos+1 < sWork.cLineStr.GetLength())	// 2007.02.17 ryoji 追加
	 && (GetMaxLineKetas() - sWork.nPosX < 4)
	 && ( sWork.nPosX > sWork.nIndent )	//	2004.04.09 sWork.nPosXの解釈変更のため，行頭チェックも変更
	 && (sWork.eKinsokuType == KINSOKU_TYPE_NONE) )
	{	/* 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう) */
		CLayoutInt nCharKetas2 = GetLayoutXOfChar( sWork.cLineStr, sWork.nPos );
		CLayoutInt nCharKetas3 = GetLayoutXOfChar( sWork.cLineStr, sWork.nPos+1 );

		if( IsKinsokuPosTail(GetMaxLineLayout() - sWork.nPosX, nCharKetas2, nCharKetas3) && IsKinsokuTail(sWork.cLineStr.At(sWork.nPos)) ){
			sWork.nWordBgn = sWork.nPos;
			sWork.nWordLen = 1;
			sWork.eKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
			
			(this->*pfOnLine)(sWork);
		}
	}
}

//折り返す場合はtrueを返す
bool CLayoutMgr::_DoTab(SLayoutWork& sWork, PF_OnLine pfOnLine)
{
	//	Sep. 23, 2002 genta せっかく作ったので関数を使う
	CLayoutInt nCharKetas = GetActualTabSpace( sWork.nPosX );
	if( sWork.nPosX + nCharKetas > GetMaxLineLayout() ){
		(this->*pfOnLine)(sWork);
		return true;
	}
	sWork.nPosX += nCharKetas;
	sWork.nPos += CLogicInt(1);
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          準処理                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

enum {
	ConfigFlag_WordWrap = 1,
	ConfigFlag_KinsokuKuto = 1 << 1,
	ConfigFlag_KinsokuHead = 1 << 2,
	ConfigFlag_KinsokuTail = 1 << 3,
	ConfigFlag_KinsokuRet = 1 << 4,
};

void CLayoutMgr::_MakeOneLine(SLayoutWork& sWork, PF_OnLine pfOnLine, const int flags)
{
	int	nEol = sWork.pcDocLine->GetEol().GetLen(); //########そのうち不要になる
	int nEol_1 = nEol - 1;
	if( 0 >	nEol_1 ){
		nEol_1 = 0;
	}
	const wchar_t* pLineStr = sWork.cLineStr.GetPtr();
	const int lineLength = sWork.cLineStr.GetLength();
	CLogicInt nLength = lineLength - CLogicInt(nEol_1);

	if(sWork.pcColorStrategy)sWork.pcColorStrategy->InitStrategyStatus();
	CColorStrategyPool& color = *CColorStrategyPool::getInstance();

	const auto maxLineLayout = GetMaxLineLayout();
	//1ロジック行を消化するまでループ
	while( sWork.nPos < nLength ){
		// インデント幅は_OnLineで計算済みなのでここからは削除

		if (flags) {
			//禁則処理中ならスキップする	@@@ 2002.04.20 MIK
			if(_DoKinsokuSkip(sWork, pfOnLine)){ }
			else{
				// 英文ワードラップをする
				if(flags & ConfigFlag_WordWrap){
					_DoWordWrap(sWork, pfOnLine);
				}

				// 句読点のぶらさげ
				if(flags & ConfigFlag_KinsokuKuto){
					_DoKutoBurasage(sWork);
				}

				// 行頭禁則
				if(flags & ConfigFlag_KinsokuHead){
					_DoGyotoKinsoku(sWork, pfOnLine);
				}

				// 行末禁則
				if(flags & ConfigFlag_KinsokuTail){
					_DoGyomatsuKinsoku(sWork, pfOnLine);
				}
			}
		}

		//@@@ 2002.09.22 YAZAKI
		color.CheckColorMODE( sWork.pcColorStrategy, sWork.nPos, sWork.cLineStr );

		if( pLineStr[sWork.nPos] == WCODE::TAB ){
			if(_DoTab(sWork, pfOnLine)){
				continue;
			}
		}
		else{
			// 2007.09.07 kobake   ロジック幅とレイアウト幅を区別
			CLayoutInt nCharKetas = GetLayoutXOfChar( pLineStr, lineLength, sWork.nPos );
//			if( 0 == nCharKetas ){				// 削除 サロゲートペア対策	2008/7/5 Uchi
//				nCharKetas = CLayoutInt(1);
//			}

			if( sWork.nPosX + nCharKetas > maxLineLayout ){
				if( sWork.eKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
				{
					if( ! ((flags & ConfigFlag_KinsokuRet) && (sWork.nPos == lineLength - nEol) && nEol) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
					{
						(this->*pfOnLine)(sWork);
						continue;
					}
				}
			}
			sWork.nPos += 1;
			sWork.nPosX += nCharKetas;
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       本処理(全体)                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CLayoutMgr::_OnLine1(SLayoutWork& sWork)
{
	AddLineBottom( sWork._CreateLayout(this) );
	sWork.pLayout = m_pLayoutBot;
	sWork.colorPrev = sWork.pcColorStrategy->GetStrategyColorSafe();
	sWork.exInfoPrev.SetColorInfo(sWork.pcColorStrategy->GetStrategyColorInfoSafe());
	sWork.nBgn = sWork.nPos;
	// 2004.03.28 Moca sWork.nPosXはインデント幅を含むように変更(TAB位置調整のため)
	sWork.nPosX = sWork.nIndent = (this->*m_getIndentOffset)( sWork.pLayout );
}

/*!
	現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します

	@date 2004.04.03 Moca TABが使われると折り返し位置がずれるのを防ぐため，
		nPosXがインデントを含む幅を保持するように変更．m_nMaxLineKetasは
		固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．
*/
void CLayoutMgr::_DoLayout(bool bBlockingHook)
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::_DoLayout" );

	/*	表示上のX位置
		2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
	*/
	const int nAllLineNum = m_pcDocLineMgr->GetLineCount();
	const int nListenerCount = GetListenerCount();

	if( nListenerCount != 0 ){
		NotifyProgress(0);
		/* 処理中のユーザー操作を可能にする */
		if( bBlockingHook ){
			if( !::BlockingHook( NULL ) )return;
		}
	}

	_Empty();
	Init();
	
	//	Nov. 16, 2002 genta
	//	折り返し幅 <= TAB幅のとき無限ループするのを避けるため，
	//	TABが折り返し幅以上の時はTAB=4としてしまう
	//	折り返し幅の最小値=10なのでこの値は問題ない
	if( GetTabSpaceKetas() >= GetMaxLineKetas() ){
		m_nTabSpace = CKetaXInt(4);
	}

	SLayoutWork	sWork;
	sWork.pcDocLine				= m_pcDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr変更
	sWork.pLayout				= NULL;
	sWork.pcColorStrategy		= NULL;
	sWork.colorPrev				= COLORIDX_DEFAULT;
	sWork.nCurLine				= CLogicInt(0);

	constexpr DWORD userInterfaceInterval = 33;
	DWORD prevTime = GetTickCount() + userInterfaceInterval;

	int flags = 0;
	if (m_pTypeConfig->m_bWordWrap) flags |= ConfigFlag_WordWrap;
	if (m_pTypeConfig->m_bKinsokuKuto) flags |= ConfigFlag_KinsokuKuto;
	if (m_pTypeConfig->m_bKinsokuHead) flags |= ConfigFlag_KinsokuHead;
	if (m_pTypeConfig->m_bKinsokuTail) flags |= ConfigFlag_KinsokuTail;
	if (m_pTypeConfig->m_bKinsokuRet) flags |= ConfigFlag_KinsokuRet;

	while( NULL != sWork.pcDocLine ){
		sWork.cLineStr		= sWork.pcDocLine->GetStringRefWithEOL();
		sWork.eKinsokuType	= KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
		sWork.nBgn			= CLogicInt(0);
		sWork.nPos			= CLogicInt(0);
		sWork.nWordBgn		= CLogicInt(0);
		sWork.nWordLen		= CLogicInt(0);
		sWork.nPosX			= CLayoutInt(0);	// 表示上のX位置
		sWork.nIndent		= CLayoutInt(0);	// インデント幅

		_MakeOneLine(sWork, &CLayoutMgr::_OnLine1, flags);

		if( sWork.nPos - sWork.nBgn > 0 ){
// 2002/03/13 novice
			AddLineBottom( sWork._CreateLayout(this) );
			sWork.colorPrev = sWork.pcColorStrategy->GetStrategyColorSafe();
			sWork.exInfoPrev.SetColorInfo(sWork.pcColorStrategy->GetStrategyColorInfoSafe());
		}

		// 次の行へ
		sWork.nCurLine++;
		sWork.pcDocLine = sWork.pcDocLine->GetNextLine();
		
		// 処理中のユーザー操作を可能にする
		if( nListenerCount !=0 && 0 < nAllLineNum) {
			DWORD currTime = GetTickCount();
			DWORD diffTime = currTime - prevTime;
			if( diffTime >= userInterfaceInterval ){
				prevTime = currTime;
				NotifyProgress(::MulDiv( sWork.nCurLine, 100 , nAllLineNum ) );
				if( bBlockingHook ){
					if( !::BlockingHook( NULL ) )return;
				}
			}
		}

// 2002/03/13 novice
	}

	// 2011.12.31 Botの色分け情報は最後に設定
	m_nLineTypeBot = sWork.pcColorStrategy->GetStrategyColorSafe();
	m_cLayoutExInfoBot.SetColorInfo(sWork.pcColorStrategy->GetStrategyColorInfoSafe());

	m_nPrevReferLine = CLayoutInt(0);
	m_pLayoutPrevRefer = NULL;

	if( nListenerCount !=0 ){
		NotifyProgress(0);
		/* 処理中のユーザー操作を可能にする */
		if( bBlockingHook ){
			if( !::BlockingHook( NULL ) )return;
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     本処理(範囲指定)                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CLayoutMgr::_OnLine2(SLayoutWork& sWork)
{
	//@@@ 2002.09.23 YAZAKI 最適化
	if( sWork.bNeedChangeCOMMENTMODE ){
		sWork.pLayout = sWork.pLayout->GetNextLayout();
		sWork.pLayout->SetColorTypePrev(sWork.colorPrev);
		sWork.pLayout->GetLayoutExInfo()->SetColorInfo(sWork.exInfoPrev.DetachColorInfo());
		(*sWork.pnExtInsLineNum)++;								//	再描画してほしい行数+1
	}
	else {
		sWork.pLayout = InsertLineNext( sWork.pLayout, sWork._CreateLayout(this) );
	}
	sWork.colorPrev = sWork.pcColorStrategy->GetStrategyColorSafe();
	sWork.exInfoPrev.SetColorInfo(sWork.pcColorStrategy->GetStrategyColorInfoSafe());

	sWork.nBgn = sWork.nPos;
	// 2004.03.28 Moca sWork.nPosXはインデント幅を含むように変更(TAB位置調整のため)
	sWork.nPosX = sWork.nIndent = (this->*m_getIndentOffset)( sWork.pLayout );
	if( ( sWork.ptDelLogicalFrom.GetY2() == sWork.nCurLine && sWork.ptDelLogicalFrom.GetX2() < sWork.nPos ) ||
		( sWork.ptDelLogicalFrom.GetY2() < sWork.nCurLine )
	){
		(sWork.nModifyLayoutLinesNew)++;
	}
}

/*!
	指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
	
	@date 2002.10.07 YAZAKI rename from "DoLayout3_New"
	@date 2004.04.03 Moca TABが使われると折り返し位置がずれるのを防ぐため，
		sWork.nPosXがインデントを含む幅を保持するように変更．m_nMaxLineKetasは
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

	SLayoutWork sWork;
	sWork.pLayout					= pLayoutPrev;
	sWork.pcColorStrategy			= CColorStrategyPool::getInstance()->GetStrategyByColor(nCurrentLineType);
	sWork.colorPrev				= nCurrentLineType;
	sWork.exInfoPrev.SetColorInfo(colorInfo);
	sWork.bNeedChangeCOMMENTMODE	= false;
	if( NULL == sWork.pLayout ){
		sWork.nCurLine = CLogicInt(0);
	}else{
		sWork.nCurLine = sWork.pLayout->GetLogicLineNo() + CLogicInt(1);
	}
	sWork.pcDocLine				= m_pcDocLineMgr->GetLine( sWork.nCurLine );
	sWork.nModifyLayoutLinesNew	= CLayoutInt(0);
	//引数
	sWork.ptDelLogicalFrom		= _ptDelLogicalFrom;
	sWork.pnExtInsLineNum		= _pnExtInsLineNum;

	if(sWork.pcColorStrategy){
		sWork.pcColorStrategy->InitStrategyStatus();
		sWork.pcColorStrategy->SetStrategyColorInfo(colorInfo);
	}

	int flags = 0;
	if (m_pTypeConfig->m_bWordWrap) flags |= ConfigFlag_WordWrap;
	if (m_pTypeConfig->m_bKinsokuKuto) flags |= ConfigFlag_KinsokuKuto;
	if (m_pTypeConfig->m_bKinsokuHead) flags |= ConfigFlag_KinsokuHead;
	if (m_pTypeConfig->m_bKinsokuTail) flags |= ConfigFlag_KinsokuTail;
	if (m_pTypeConfig->m_bKinsokuRet) flags |= ConfigFlag_KinsokuRet;

	while( NULL != sWork.pcDocLine ){
		sWork.cLineStr		= sWork.pcDocLine->GetStringRefWithEOL();
		sWork.eKinsokuType	= KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
		sWork.nBgn			= CLogicInt(0);
		sWork.nPos			= CLogicInt(0);
		sWork.nWordBgn		= CLogicInt(0);
		sWork.nWordLen		= CLogicInt(0);
		sWork.nPosX			= CLayoutInt(0);			// 表示上のX位置
		sWork.nIndent		= CLayoutInt(0);			// インデント幅

		_MakeOneLine(sWork, &CLayoutMgr::_OnLine2, flags);

		if( sWork.nPos - sWork.nBgn > 0 ){
// 2002/03/13 novice
			//@@@ 2002.09.23 YAZAKI 最適化
			_OnLine2(sWork);
		}

		nLineNumWork++;
		sWork.nCurLine++;

		/* 目的の行数(nLineNum)に達したか、または通り過ぎた（＝行数が増えた）か確認 */
		//@@@ 2002.09.23 YAZAKI 最適化
		if( nLineNumWork >= nLineNum ){
			if( sWork.pLayout && sWork.pLayout->GetNextLayout() ){
				if( sWork.colorPrev != sWork.pLayout->GetNextLayout()->GetColorTypePrev() ){
					//	COMMENTMODEが異なる行が増えましたので、次の行→次の行と更新していきます。
					sWork.bNeedChangeCOMMENTMODE = true;
				}else if( sWork.exInfoPrev.GetColorInfo() && sWork.pLayout->GetNextLayout()->GetColorInfo()
				 && !sWork.exInfoPrev.GetColorInfo()->IsEqual(sWork.pLayout->GetNextLayout()->GetColorInfo()) ){
					sWork.bNeedChangeCOMMENTMODE = true;
				}else if( sWork.exInfoPrev.GetColorInfo() && NULL == sWork.pLayout->GetNextLayout()->GetColorInfo() ){
					sWork.bNeedChangeCOMMENTMODE = true;
				}else if( NULL == sWork.exInfoPrev.GetColorInfo() && sWork.pLayout->GetNextLayout()->GetColorInfo() ){
					sWork.bNeedChangeCOMMENTMODE = true;
				}else{
					break;
				}
			}else{
				break;	//	while( NULL != sWork.pcDocLine ) 終了
			}
		}
		sWork.pcDocLine = sWork.pcDocLine->GetNextLine();
// 2002/03/13 novice
	}

	// 2004.03.28 Moca EOFだけの論理行の直前の行の色分けが確認・更新された
	if( sWork.nCurLine == m_pcDocLineMgr->GetLineCount() ){
		m_nLineTypeBot = sWork.pcColorStrategy->GetStrategyColorSafe();
		m_cLayoutExInfoBot.SetColorInfo(sWork.pcColorStrategy->GetStrategyColorInfoSafe());
	}

	// 2009.08.28 nasukoji	テキストが編集されたら最大幅を算出する
	CalculateTextWidth_Range(pctwArg);

// 1999.12.22 レイアウト情報がなくなる訳ではないので
//	m_nPrevReferLine = 0;
//	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	return sWork.nModifyLayoutLinesNew;
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
