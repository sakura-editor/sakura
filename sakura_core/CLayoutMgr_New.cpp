/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI, novice
	Copyright (C) 2003, genta
	Copyright (C) 2004, Moca, genta
	Copyright (C) 2005, D.S.Koba, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <mbstring.h>
#include "CLayoutMgr.h"
#include "charcode.h"
#include "debug.h"
#include <commctrl.h>
#include "CRunningTimer.h"
#include "CLayout.h"/// 2002/2/10 aroka
#include "CDocLine.h"/// 2002/2/10 aroka
#include "CDocLineMgr.h"// 2002/2/10 aroka
#include "CMemory.h"/// 2002/2/10 aroka
#include "CMemoryIterator.h"
#include "CEditDoc.h" /// 2003/07/20 genta
#include "util/window.h"

//レイアウト中の禁則タイプ	//@@@ 2002.04.20 MIK
#define	KINSOKU_TYPE_NONE			0	//なし
#define	KINSOKU_TYPE_WORDWRAP		1	//ワードラップ中
#define	KINSOKU_TYPE_KINSOKU_HEAD	2	//行頭禁則中
#define	KINSOKU_TYPE_KINSOKU_TAIL	3	//行末禁則中
#define	KINSOKU_TYPE_KINSOKU_KUTO	4	//句読点ぶら下げ中



/*!
	現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します

	@date 2004.04.03 Moca TABが使われると折り返し位置がずれるのを防ぐため，
		nPosXがインデントを含む幅を保持するように変更．nMaxLineKetasは
		固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．
*/
void CLayoutMgr::DoLayout(
		HWND	hwndProgress
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::DoLayout" );

	CLogicInt	nLineNum;
	CDocLine*	pCDocLine;
	/*	表示上のX位置
		2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
	*/
	CLayoutInt			nPosX; //レイアウト単位
	EColorIndexType		nCOMMENTMODE;
	EColorIndexType		nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	int			nAllLineNum;

	CLayoutInt	nIndent;			//	インデント幅
	CLayout*	pLayoutCalculated;	//	インデント幅計算済みのCLayout.
	CLayoutInt	nMaxLineKetas;

// 2002/03/13 novice
	nCOMMENTMODE = COLORIDX_TEXT;
	nCOMMENTMODE_Prev = COLORIDX_TEXT;

	if( NULL != hwndProgress ){
		::PostMessageAny( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( NULL ) ){
			return;
		}
	}

	Empty();
	Init();
	nLineNum = CLogicInt(0);
	
	//	Nov. 16, 2002 genta
	//	折り返し幅 <= TAB幅のとき無限ループするのを避けるため，
	//	TABが折り返し幅以上の時はTAB=4としてしまう
	//	折り返し幅の最小値=10なのでこの値は問題ない
	if( m_nTabSpace >= m_nMaxLineKetas ){
		m_nTabSpace = CLayoutInt(4);
	}

	pCDocLine = m_pcDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr変更

	nCOMMENTEND = 0;
	nAllLineNum = m_pcDocLineMgr->GetLineCount();

	/*
		2004.03.28 Moca TAB計算を正しくするためにインデントを幅で調整することはしない
		nMaxLineKetasは変更しないので，ここでm_nMaxLineKetasを設定する．
	*/
	nMaxLineKetas = m_nMaxLineKetas;

	while( NULL != pCDocLine ){
		CLogicInt		nLineLen;
		const wchar_t*	pLine = pCDocLine->m_cLine.GetStringPtr( &nLineLen );
		nPosX = CLayoutInt(0);

		CLogicInt	nBgn = CLogicInt(0);
		CLogicInt	nPos = CLogicInt(0); // メモリ上の位置(offset) ロジック単位

		CLogicInt	nWordBgn = CLogicInt(0);
		CLogicInt	nWordLen = CLogicInt(0);

		int			nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK

		int	nEol = pCDocLine->m_cEol.GetLen();
		int nEol_1 = nEol - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		nIndent = CLayoutInt(0);	//	インデント幅
		pLayoutCalculated = NULL;	//	インデント幅計算済みのCLayout.

		while( nPos < nLineLen - CLogicInt(nEol_1) ){
			//	インデント幅の計算コストを下げるための方策
			if ( m_pLayoutBot && m_pLayoutBot!=pLayoutCalculated && nBgn )
			{
				//	計算
				//	Oct, 1, 2002 genta Indentサイズを取得するように変更
				nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
				
				//	計算済み
				pLayoutCalculated = m_pLayoutBot;
			}


			SEARCH_START:;
			
			//禁則処理中ならスキップする	@@@ 2002.04.20 MIK
			if( KINSOKU_TYPE_NONE != nKinsokuType )
			{
				//禁則処理の最後尾に達したら禁則処理中を解除する
				if( nPos >= nWordBgn + nWordLen )
				{
					if( nKinsokuType == KINSOKU_TYPE_KINSOKU_KUTO && nPos == nWordBgn + nWordLen )
					{
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol ) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{
							AddLineBottom(
								CreateLayout(
									pCDocLine,
									CLogicPoint(nBgn, nLineNum),
									nPos - nBgn,
									nCOMMENTMODE_Prev,
									nIndent
								)
							);
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
							nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
							pLayoutCalculated = m_pLayoutBot;
						}
					}
					
					nWordLen = CLogicInt(0);
					nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
				}
			}
			else
			{
			
				/* ワードラップ処理 */
				if( m_bWordWrap	/* 英文ワードラップをする */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
					/* 英単語の先頭か */
					int nCharChars = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( nPos>=nBgn && nCharChars==1 && IS_KEYWORD_CHAR(pLine[nPos]) ){
						// 2007.09.07 kobake レイアウトとロジックの区別
						// キーワード文字列の終端を探す
						// -> iLogic  (ロジック単位終端)
						// -> iLayout (レイアウト単位終端)
						CLogicInt	iLogic = nPos + CLogicInt(1);
						CLayoutInt	iLayout = nPosX + CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos);
						while( iLogic < nLineLen )
						{
							CLogicInt nCharChars2 = CNativeW2::GetSizeOfChar( pLine, nLineLen, iLogic );
							if( 0 == nCharChars2 ){
								nCharChars2 = CLogicInt(1);
							}
							CLayoutInt nCharKetas = CNativeW2::GetKetaOfChar( pLine, nLineLen, iLogic);

							if( nCharChars2 != 1 )break;
							if( !IS_KEYWORD_CHAR( pLine[iLogic] ) )break;

							iLogic+=nCharChars2;
							iLayout+=nCharKetas;
						}
						nWordBgn = nPos;
						nWordLen = iLogic - nPos;
						CLayoutInt nWordKetas = iLayout - nPosX;

						nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK
						
						if( nPosX+nWordKetas>=nMaxLineKetas && nPos-nBgn>0 )
						{
							AddLineBottom(
								CreateLayout(
									pCDocLine,
									CLogicPoint(nBgn, nLineNum),
									nPos - nBgn,
									nCOMMENTMODE_Prev,
									nIndent
								)
							);
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
							nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
							pLayoutCalculated = m_pLayoutBot;
						}
					}
				}

				//@@@ 2002.04.07 MIK start
				/* 句読点のぶらさげ */
				if( m_bKinsokuKuto && (nMaxLineKetas - nPosX < 2) && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					// 2007.09.07 kobake   レイアウトとロジックの区別
					CLogicInt  nCharChars2 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas  = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos );

					if( IsKinsokuPosKuto( nMaxLineKetas - nPosX, nCharKetas ) && IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
					}
				}

				/* 行頭禁則 */
				if( m_bKinsokuHead
				 && (nMaxLineKetas - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosXの解釈変更のため，行頭チェックも変更
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					CLogicInt nCharChars2 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos );
					CLogicInt nCharChars3 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );
					CLayoutInt nCharKetas2 = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas3 = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosHead( nMaxLineKetas - nPosX, nCharKetas2, nCharKetas3 )
					 && IsKinsokuHead( &pLine[nPos+nCharChars2], nCharChars3 )
					 && ! IsKinsokuHead( &pLine[nPos], nCharChars2 )	//1文字前が行頭禁則でない
					 && ! IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )	//句読点でない
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2 + nCharChars3;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;
						AddLineBottom( CreateLayout(pCDocLine, CLogicPoint(nBgn, nLineNum), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						m_nLineTypeBot = nCOMMENTMODE;
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
						nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
						pLayoutCalculated = m_pLayoutBot;
					}
				}

				/* 行末禁則 */
				if( m_bKinsokuTail
				 && (nMaxLineKetas - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosXの解釈変更のため，行頭チェックも変更
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう) */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					CLogicInt nCharChars2 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos );
					CLogicInt nCharChars3 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );
					CLayoutInt nCharKetas2 = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas3 = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosTail( nMaxLineKetas - nPosX, nCharKetas2, nCharKetas3 )
						&& IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
						AddLineBottom( CreateLayout(pCDocLine, CLogicPoint(nBgn, nLineNum), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						m_nLineTypeBot = nCOMMENTMODE;
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
						nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
						pLayoutCalculated = m_pLayoutBot;
					}
				}
				//@@@ 2002.04.08 MIK end
			}	//if( KINSOKU_TYPE_NONE != nKinsokuTyoe ) 禁則処理中

			//@@@ 2002.09.22 YAZAKI
			bool bGotoSEARCH_START = CheckColorMODE( nCOMMENTMODE, nCOMMENTEND, nPos, nLineLen, pLine );
			if ( bGotoSEARCH_START )
				goto SEARCH_START;
			
			if( pLine[nPos] == WCODE::TAB ){
				//	Sep. 23, 2002 genta せっかく作ったので関数を使う
				CLayoutInt nCharKetas = GetActualTabSpace( nPosX );
				if( nPosX + nCharKetas > nMaxLineKetas ){
					AddLineBottom( CreateLayout(pCDocLine, CLogicPoint(nBgn, nLineNum), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
					m_nLineTypeBot = nCOMMENTMODE;
					nCOMMENTMODE_Prev = nCOMMENTMODE;
					nBgn = nPos;
					// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
					nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
					pLayoutCalculated = m_pLayoutBot;
					continue;
				}
				nPosX += nCharKetas;
				nPos += CLogicInt(1);
			}else{
				// 2005-09-02 D.S.Koba GetSizeOfChar
				// 2007.09.07 kobake   ロジック幅とレイアウト幅を区別
				CLogicInt nCharChars2 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos );
				if( 0 == nCharChars2 ){
					nCharChars2 = CLogicInt(1);
					break;	//@@@ 2002.04.16 MIK
				}
				CLayoutInt nCharKetas = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos );
				if( 0 == nCharKetas ){
					nCharKetas = CLayoutInt(1);
				}

				if( nPosX + nCharKetas > nMaxLineKetas ){
					if( nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
					{
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{	//@@@ 2002.04.14 MIK
							AddLineBottom( CreateLayout(pCDocLine, CLogicPoint(nBgn, nLineNum), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
							nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
							pLayoutCalculated = m_pLayoutBot;
							continue;
						}	//@@@ 2002.04.14 MIK
					}
				}
				nPos+= CLogicInt(nCharChars2);
				nPosX += nCharKetas;
			}
		}
		if( nPos - nBgn > 0 ){
// 2002/03/13 novice
			if( nCOMMENTMODE == COLORIDX_COMMENT ){	/* 行コメントである */
				nCOMMENTMODE = COLORIDX_TEXT;
			}
			AddLineBottom( CreateLayout(pCDocLine, CLogicPoint(nBgn, nLineNum), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
			m_nLineTypeBot = nCOMMENTMODE;
			nCOMMENTMODE_Prev = nCOMMENTMODE;
		}
		nLineNum++;
		if( NULL != hwndProgress && 0 < nAllLineNum && 0 == ( nLineNum % 1024 ) ){
			::PostMessageAny( hwndProgress, PBM_SETPOS, nLineNum * 100 / nAllLineNum , 0 );
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( NULL ) ){
				return;
			}
		}
//		pLine = m_pcDocLineMgr->GetNextLinrStr( &nLineLen );
		pCDocLine = pCDocLine->m_pNext;;
// 2002/03/13 novice
		if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* 行コメントである */
			nCOMMENTMODE_Prev = COLORIDX_TEXT;
		}
		nCOMMENTMODE = nCOMMENTMODE_Prev;
		nCOMMENTEND = 0;
	}
	m_nPrevReferLine = CLayoutInt(0);
	m_pLayoutPrevRefer = NULL;

	if( NULL != hwndProgress ){
		::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( NULL ) ){
			return;
		}
	}
	return;
}





/*!
	指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
	
	@date 2002.10.07 YAZAKI rename from "DoLayout3_New"
	@date 2004.04.03 Moca TABが使われると折り返し位置がずれるのを防ぐため，
		nPosXがインデントを含む幅を保持するように変更．nMaxLineKetasは
		固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．

	@note 2004.04.03 Moca
		DoLayoutとは違ってレイアウト情報がリスト中間に挿入されるため，
		挿入後にm_nLineTypeBotへコメントモードを指定してはならない
		代わりに最終行のコメントモードを終了間際に確認している．
*/
CLayoutInt CLayoutMgr::DoLayout_Range(
	CLayout*		pLayoutPrev,
	CLogicInt		nLineNum,
	CLogicPoint		ptDelLogicalFrom,
	EColorIndexType	nCurrentLineType,
	CLayoutInt*		pnExtInsLineNum
)
{
	*pnExtInsLineNum = CLayoutInt(0);

	bool		bNeedChangeCOMMENTMODE = false;	//@@@ 2002.09.23 YAZAKI bAddを名称変更
	CLayoutInt	nMaxLineKetas= m_nMaxLineKetas;
	CLogicInt	nLineNumWork = CLogicInt(0);

	CLayout*	pLayout = pLayoutPrev;

	CLogicInt	nCurLine;
	if( NULL == pLayout ){
		nCurLine = CLogicInt(0);
	}else{
		nCurLine = pLayout->GetLogicLineNo() + CLogicInt(1);
	}

	EColorIndexType		nCOMMENTMODE = nCurrentLineType;
	EColorIndexType		nCOMMENTMODE_Prev = nCOMMENTMODE;

	CDocLine*	pCDocLine = m_pcDocLineMgr->GetLineInfo( nCurLine );

	int			nCOMMENTEND = 0;
	CLayoutInt	nModifyLayoutLinesNew = CLayoutInt(0);

	// 2006.12.01 Moca 途中にまで再構築した場合にEOF位置がずれたまま
	//	更新されないので，範囲にかかわらず必ずリセットする．
	m_nEOFColumn = CLayoutInt(-1);
	m_nEOFLine = CLayoutInt(-1);

	while( NULL != pCDocLine ){
		CLogicInt		nLineLen;
		const wchar_t*	pLine = pCDocLine->m_cLine.GetStringPtr( &nLineLen );
		CLayoutInt		nPosX = CLayoutInt(0); //表示上のX位置

		// メモリ上の位置(offset)
		CLogicInt	nBgn = CLogicInt(0);
		CLogicInt	nPos = CLogicInt(0);

		CLogicInt	nWordBgn = CLogicInt(0);
		CLogicInt	nWordLen = CLogicInt(0);

		int			nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK

		int	nEol = pCDocLine->m_cEol.GetLen();
		int nEol_1 = nEol - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		CLayoutInt	nIndent = CLayoutInt(0);				//	インデント幅
		CLayout*	pLayoutCalculated = pLayout;	//	インデント幅計算済みのCLayout.

		while( nPos < nLineLen - CLogicInt(nEol_1) ){
			//	インデント幅の計算コストを下げるための方策
			if ( pLayout && pLayout != pLayoutCalculated ){

				 //	計算
				//	Oct, 1, 2002 genta Indentサイズを取得するように変更
				nIndent = (this->*m_getIndentOffset)( pLayout );

				//	計算済み
				pLayoutCalculated = pLayout;
			}

			SEARCH_START:;
			
			//禁則処理中ならスキップする	@@@ 2002.04.20 MIK
			if( KINSOKU_TYPE_NONE != nKinsokuType )
			{
				//禁則処理の最後尾に達したら禁則処理中を解除する
				if( nPos >= nWordBgn + nWordLen )
				{
					if( nKinsokuType==KINSOKU_TYPE_KINSOKU_KUTO && nPos==nWordBgn+nWordLen )
					{
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol ) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{
							//@@@ 2002.09.23 YAZAKI 最適化
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->m_pNext;
								pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
								(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
							nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
								( ptDelLogicalFrom.GetY2() < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
						}
					}

					nWordLen = CLogicInt(0);
					nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
				}
			}
			else
			{
			
				/* ワードラップ処理 */
				if( m_bWordWrap	/* 英文ワードラップをする */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
					/* 英単語の先頭か */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( nPos >= nBgn && nCharChars == 1 && IS_KEYWORD_CHAR(pLine[nPos]) ){
						/* キーワード文字列の終端を探す */
						// 2007.09.07 kobake レイアウトとロジックの区別
						CLogicInt	iLogic = nPos + CLogicInt(1);
						CLayoutInt	iLayout = nPosX + CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos);
						while(iLogic < nLineLen){
							CLogicInt nCharChars2 = CNativeW2::GetSizeOfChar( pLine, nLineLen, iLogic );
							if( 0 == nCharChars2 ){
								nCharChars2 = CLogicInt(1);
							}
							CLayoutInt nCharKetas = CNativeW2::GetKetaOfChar( pLine, nLineLen, iLogic);
							if( 0 == nCharKetas ){
								nCharKetas = CLayoutInt(1);
							}

							if( nCharChars2 != 1 )break;
							if( !IS_KEYWORD_CHAR(pLine[iLogic]) )break;

							iLogic+=nCharChars2;
							iLayout+=nCharKetas;
						}
						nWordBgn = nPos;
						nWordLen = iLogic - nPos;
						CLayoutInt nWordKetas = iLayout - nPosX;
						nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK

						if( nPosX+nWordKetas>=nMaxLineKetas && nPos-nBgn>0 )
						{
							//@@@ 2002.09.23 YAZAKI 最適化
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->m_pNext;
								pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
								(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
							nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
								( ptDelLogicalFrom.GetY2() < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
//?							continue;
						}
					}
				}

				//@@@ 2002.04.07 MIK start
				/* 句読点のぶらさげ */
				if( m_bKinsokuKuto
				 && (nMaxLineKetas - nPosX < 2)
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					// 2007.09.07 kobake   レイアウトとロジックの区別
					CLogicInt  nCharChars2 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos );

					if( IsKinsokuPosKuto( nMaxLineKetas - nPosX, nCharKetas )
						&& IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
					}
				}

				/* 行頭禁則 */
				if( m_bKinsokuHead
				 && (nMaxLineKetas - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosXの解釈変更のため，行頭チェックも変更
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					// 2007.09.07 kobake   レイアウトとロジックの区別
					CLogicInt  nCharChars2 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas2 = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos );
					CLogicInt  nCharChars3 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );
					CLayoutInt nCharKetas3 = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosHead( nMaxLineKetas - nPosX, nCharKetas2, nCharKetas3 )
					 && IsKinsokuHead( &pLine[nPos+nCharChars2], nCharChars3 )
					 && ! IsKinsokuHead( &pLine[nPos], nCharChars2 )	//1文字前が行頭禁則でない
					 && ! IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )	//句読点でない
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2 + nCharChars3;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;
						//@@@ 2002.09.23 YAZAKI 最適化
						if( bNeedChangeCOMMENTMODE ){
							pLayout = pLayout->m_pNext;
							pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
							(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
						}
						else {
							pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						}
						nCOMMENTMODE_Prev = nCOMMENTMODE;

						nBgn = nPos;
						// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
						nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
						pLayoutCalculated = pLayout;
						if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
							( ptDelLogicalFrom.GetY2() < nCurLine )
						){
							(nModifyLayoutLinesNew)++;;
						}
					}
				}

				/* 行末禁則 */
				if( m_bKinsokuTail
				 && (nMaxLineKetas - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosXの解釈変更のため，行頭チェックも変更
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう) */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					// 2007.09.07 kobake   レイアウトとロジックの区別
					CLogicInt  nCharChars2 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas2 = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos );
					CLogicInt  nCharChars3 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );
					CLayoutInt nCharKetas3 = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosTail( nMaxLineKetas - nPosX, nCharKetas2, nCharKetas3 )
						&& IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
						//@@@ 2002.09.23 YAZAKI 最適化
						if( bNeedChangeCOMMENTMODE ){
							pLayout = pLayout->m_pNext;
							pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
							(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
						}
						else {
							pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						}
						nCOMMENTMODE_Prev = nCOMMENTMODE;

						nBgn = nPos;
						// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
						nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
						pLayoutCalculated = pLayout;
						if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
							( ptDelLogicalFrom.GetY2() < nCurLine )
						){
							(nModifyLayoutLinesNew)++;;
						}
					}
				}
				//@@@ 2002.04.08 MIK end
			}	// if( nKinsokuType != KINSOKU_TYPE_NONE )

			//@@@ 2002.09.22 YAZAKI
			bool bGotoSEARCH_START = CheckColorMODE( nCOMMENTMODE, nCOMMENTEND, nPos, nLineLen, pLine );
			if ( bGotoSEARCH_START )
				goto SEARCH_START;

			CLayoutInt nCharKetas;
			if( pLine[nPos] == WCODE::TAB ){
				//	Sep. 23, 2002 genta せっかく作ったので関数を使う
				nCharKetas = GetActualTabSpace( nPosX );
				if( nPosX + nCharKetas > nMaxLineKetas ){
					//@@@ 2002.09.23 YAZAKI 最適化
					if( bNeedChangeCOMMENTMODE ){
						pLayout = pLayout->m_pNext;
						pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
						(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
					}
					else {
						pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
					}
					nCOMMENTMODE_Prev = nCOMMENTMODE;

					nBgn = nPos;
					// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
					nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
					pLayoutCalculated = pLayout;
					if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
						( ptDelLogicalFrom.GetY2() < nCurLine )
					){
						(nModifyLayoutLinesNew)++;;
					}
					continue;
				}
				nPos++;
			}
			else{
				// 2005-09-02 D.S.Koba GetSizeOfChar
				// 2007.09.07 kobake   レイアウトとロジックの区別
				nCharKetas = CNativeW2::GetKetaOfChar( pLine, nLineLen, nPos );
				if( 0 == nCharKetas ){
					nCharKetas = CLayoutInt(1);
				}
				CLogicInt nCharChars2 = CNativeW2::GetSizeOfChar( pLine, nLineLen, nPos);
				if( 0 == nCharChars2 ){
					break;
				}
				if( nPosX + nCharKetas > nMaxLineKetas ){
					if( nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
					{
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{	//@@@ 2002.04.14 MIK
							//@@@ 2002.09.23 YAZAKI 最適化
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->m_pNext;
								pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
								(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
							nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
								( ptDelLogicalFrom.GetY2() < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
							continue;
						}
					}
				}
				nPos+= nCharChars2;
			}

			nPosX += nCharKetas;
		}
		if( nPos - nBgn > 0 ){
// 2002/03/13 novice
			if( nCOMMENTMODE == COLORIDX_COMMENT ){	/* 行コメントである */
				nCOMMENTMODE = COLORIDX_TEXT;
			}
			//@@@ 2002.09.23 YAZAKI 最適化
			if( bNeedChangeCOMMENTMODE ){
				pLayout = pLayout->m_pNext;
				pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
				(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
			}
			else {
				pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
			}
			nCOMMENTMODE_Prev = nCOMMENTMODE;

			if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
				( ptDelLogicalFrom.GetY2() < nCurLine )
			){
				(nModifyLayoutLinesNew)++;;
			}
		}

		nLineNumWork++;
		nCurLine++;

		/* 目的の行数(nLineNum)に達したか、または通り過ぎた（＝行数が増えた）か確認 */
		//@@@ 2002.09.23 YAZAKI 最適化
		if( nLineNumWork >= nLineNum ){
			if( NULL != pLayout
			 && NULL != pLayout->m_pNext
			 && ( nCOMMENTMODE_Prev != pLayout->m_pNext->GetColorTypePrev() )
			){
				//	COMMENTMODEが異なる行が増えましたので、次の行→次の行と更新していきます。
				bNeedChangeCOMMENTMODE = true;
			}else{
				break;	//	while( NULL != pCDocLine ) 終了
			}
		}
		pCDocLine = pCDocLine->m_pNext;
// 2002/03/13 novice
		if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* 行コメントである */
			nCOMMENTMODE_Prev = COLORIDX_TEXT;
		}
		nCOMMENTMODE = nCOMMENTMODE_Prev;
		nCOMMENTEND = 0;

	}

	// 2004.03.28 Moca EOFだけの論理行の直前の行の色分けが確認・更新された
	if( nCurLine == m_pcDocLineMgr->GetLineCount() ){
		m_nLineTypeBot = nCOMMENTMODE_Prev;
		// 2006.10.01 Moca 最終行が変更された。EOF位置情報を破棄する。
		m_nEOFColumn = CLayoutInt(-1);
		m_nEOFLine = CLayoutInt(-1);
	}

// 1999.12.22 レイアウト情報がなくなる訳ではないので
//	m_nPrevReferLine = 0;
//	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	return nModifyLayoutLinesNew;
}

/*!
	行頭禁則文字に該当するかを調べる．

	@param[in] pLine 調べる文字へのポインタ
	@param[in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool CLayoutMgr::IsKinsokuHead( const wchar_t *pLine, CLogicInt length )
{
	if(length==1 && m_pszKinsokuHead_1.size()){
		wchar_t wc=pLine[0];
		return wcschr(&m_pszKinsokuHead_1[0],wc)!=NULL;
	}
	else{
		return false;
	}
}

/*!
	行末禁則文字に該当するかを調べる．

	@param[in] pLine 調べる文字へのポインタ
	@param[in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool CLayoutMgr::IsKinsokuTail( const wchar_t *pLine, CLogicInt length )
{
	if(length==1 && m_pszKinsokuTail_1.size()){
		wchar_t wc=pLine[0];
		return wcschr(&m_pszKinsokuTail_1[0],wc)!=NULL;
	}
	else{
		return false;
	}
}

/*!
	句読点か

	@param[in] c1 調べる文字1バイト目
	@param[in] c2 調べる文字2バイト目
	@retval true 句読点である
	@retval false 句読点でない
*/

bool CLayoutMgr::IsKutoTen( wchar_t wc )
{
	//句読点定義
	static const wchar_t *KUTOTEN=
		L"｡､,."
		L"。、，．"
	;

	const wchar_t* p;
	for(p=KUTOTEN;*p;p++){
		if(*p==wc)return true;
	}
	return false;
}

/*!
	禁則対象句読点に該当するかを調べる．

	@param [in] pLine  調べる文字へのポインタ
	@param [in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool CLayoutMgr::IsKinsokuKuto( const wchar_t *pLine, CLogicInt length )
{
	if(length==1 && m_pszKinsokuKuto_1.size()){
		wchar_t wc=pLine[0];
		return wcschr(&m_pszKinsokuKuto_1[0],wc)!=NULL;
	}
	else{
		return false;
	}
}

/*!
	@param[in] nRest 行の残り文字数
	@param[in] nCharChars 現在位置の文字サイズ
	@param[in] nCharChars2 現在位置の次の文字サイズ

	@date 2005-08-20 D.S.Koba DoLayout()とDoLayout_Range()から分離
*/
bool CLayoutMgr::IsKinsokuPosHead(
	CLayoutInt nRest,
	CLayoutInt nCharKetas,
	CLayoutInt nCharKetas2
)
{
	switch( (Int)nRest )
	{
	//    321012  ↓マジックナンバー
	// 3 "る）" : 22 "）"の2バイト目で折り返しのとき
	// 2  "Z）" : 12 "）"の2バイト目で折り返しのとき
	// 2  "る）": 22 "）"で折り返しのとき
	// 2  "る)" : 21 ")"で折り返しのとき
	// 1   "Z）": 12 "）"で折り返しのとき
	// 1   "Z)" : 11 ")"で折り返しのとき
	//↑何文字前か？
	// ※ただし、"るZ"部分が禁則なら処理しない。
	case 3:	// 3文字前
		if( nCharKetas == 2 && nCharKetas2 == 2 ){
			return true;
		}
		break;
	case 2:	// 2文字前
		if( nCharKetas == 2 ){
			return true;
		}
		else if( nCharKetas == 1 && nCharKetas2 == 2 ){
			return true;
		}
		break;
	case 1:	// 1文字前
		if( nCharKetas == 1 ){
			return true;
		}
		break;
	}
	return false;
}

/*!
	@param[in] nRest 行の残り文字数
	@param[in] nCharChars 現在位置の文字サイズ
	@param[in] nCharChars2 現在位置の次の文字サイズ

	@date 2005-08-20 D.S.Koba DoLayout()とDoLayout_Range()から分離
*/
bool CLayoutMgr::IsKinsokuPosTail(
	CLayoutInt nRest,
	CLayoutInt nCharKetas,
	CLayoutInt nCharKetas2
)
{
	switch( (Int)nRest )
	{
	case 3:	// 3文字前
		if( nCharKetas == 2 && nCharKetas2 == 2){
			// "（あ": "あ"の2バイト目で折り返しのとき
			return true;
		}
		break;
	case 2:	// 2文字前
		if( nCharKetas == 2 ){
			// "（あ": "あ"で折り返しのとき
			return true;
		}
		else if( nCharKetas == 1 && nCharKetas2 == 2){
			// "(あ": "あ"の2バイト目で折り返しのとき
			return true;
		}
		break;
	case 1:	// 1文字前
		if( nCharKetas == 1 ){
			// "(あ": "あ"で折り返しのとき
			return true;
		}
		break;
	}
	return false;
}

int CLayoutMgr::Match_Quote( wchar_t wcQuote, int nPos, int nLineLen, const wchar_t* pLine )
{
	int nCharChars;
	int i;
	for( i = nPos; i < nLineLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeW2::GetSizeOfChar( pLine, nLineLen, i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && pLine[i] == L'\\' ){
				++i;
			}else
			if( 1 == nCharChars && pLine[i] == wcQuote ){
				return i + 1;
			}
		}else
		if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && pLine[i] == wcQuote ){
				if( i + 1 < nLineLen && pLine[i + 1] == wcQuote ){
					++i;
				}else{
					return i + 1;
				}
			}
		}
		if( 2 == nCharChars ){
			++i;
		}
	}
	return nLineLen;
}

bool CLayoutMgr::CheckColorMODE( EColorIndexType& nCOMMENTMODE, int &nCOMMENTEND, int nPos, int nLineLen, const wchar_t* pLine )
{
	switch( nCOMMENTMODE ){
	case COLORIDX_TEXT: // 2002/03/13 novice
		// 2005.11.20 Mocaコメントの色分けがON/OFF関係なく行われていたバグを修正
		if( m_bDispComment && m_cLineComment.Match( nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice
		}
		else if( m_bDispComment && m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_cBlockComment.Match_CommentTo( 0, nPos + m_cBlockComment.getBlockFromLen(0), nLineLen, pLine );
		}
		else if( m_bDispComment &&  m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_cBlockComment.Match_CommentTo( 1, nPos + m_cBlockComment.getBlockFromLen(1), nLineLen, pLine );
		}
		else if( m_bDispSString && /* シングルクォーテーション文字列を表示する */
			pLine[nPos] == L'\''
		){
			nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
			/* シングルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( L'\'', nPos + 1, nLineLen, pLine );
		}
		else if( m_bDispWString && /* ダブルクォーテーション文字列を表示する */
			pLine[nPos] == L'"'
		){
			nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
			/* ダブルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( L'"', nPos + 1, nLineLen, pLine );
		}
		break;
	case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
		break;
	case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_cBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );
		}else
		if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_cBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );
		}else
		if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
//#endif
	case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* シングルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( L'\'', nPos, nLineLen, pLine );
		}else
		if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* ダブルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( L'"', nPos, nLineLen, pLine );
		}else
		if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	}
	return false;
}

/*!
	@brief 行の長さを計算する (2行目以降の字下げ無し)
	
	字下げを行わないので，常に0を返す．
	引数は使わない．
	
	@return 1行の表示文字数 (常に0)
	
	@author genta
	@date 2002.10.01
*/
CLayoutInt CLayoutMgr::getIndentOffset_Normal( CLayout* )
{
	return CLayoutInt(0);
}

/*!
	@brief インデント幅を計算する (Tx2x)
	
	前の行の最後のTABの位置をインデント位置として返す．
	ただし，残り幅が6文字未満の場合はインデントを行わない．
	
	@author Yazaki
	@return インデントすべき文字数
	
	@date 2002.10.01 
	@date 2002.10.07 YAZAKI 名称変更, 処理見直し
*/
CLayoutInt CLayoutMgr::getIndentOffset_Tx2x( CLayout* pLayoutPrev )
{
	//	前の行が無いときは、インデント不要。
	if ( pLayoutPrev == NULL ) return CLayoutInt(0);

	CLayoutInt nIpos = pLayoutPrev->GetIndent();

	//	前の行が折り返し行ならばそれに合わせる
	if( pLayoutPrev->GetLogicOffset() > 0 )
		return nIpos;
	
	CMemoryIterator it( pLayoutPrev, m_nTabSpace );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && it.getCurrentChar() == WCODE::TAB ){
			nIpos = it.getColumn() + it.getColumnDelta();
		}
		it.addDelta();
	}
	if ( m_nMaxLineKetas - nIpos < 6 ){
		nIpos = pLayoutPrev->GetIndent();	//	あきらめる
	}
	return nIpos;	//	インデント
}

/*!
	@brief インデント幅を計算する (スペース字下げ版)
	
	論理行行頭のホワイトスペースの終わりインデント位置として返す．
	ただし，残り幅が6文字未満の場合はインデントを行わない．
	
	@author genta
	@return インデントすべき文字数
	
	@date 2002.10.01 
*/
CLayoutInt CLayoutMgr::getIndentOffset_LeftSpace( CLayout* pLayoutPrev )
{
	//	前の行が無いときは、インデント不要。
	if ( pLayoutPrev == NULL ) return CLayoutInt(0);

	//	インデントの計算
	CLayoutInt nIpos = pLayoutPrev->GetIndent();
	
	//	Oct. 5, 2002 genta
	//	折り返しの3行目以降は1つ前の行のインデントに合わせる．
	if( pLayoutPrev->GetLogicOffset() > 0 )
		return nIpos;
	
	//	2002.10.07 YAZAKI インデントの計算
	CMemoryIterator it( pLayoutPrev, m_nTabSpace );

	//	Jul. 20, 2003 genta 自動インデントに準じた動作にする
	bool bZenSpace = m_pcEditDoc->GetDocumentAttribute().m_bAutoIndent_ZENSPACE != FALSE ? 1 : 0;
	const wchar_t* szSpecialIndentChar = m_pcEditDoc->GetDocumentAttribute().m_szIndentChars;
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && WCODE::isIndentChar(it.getCurrentChar(),bZenSpace) )
		{
			//	インデントのカウントを継続する
		}
		//	Jul. 20, 2003 genta インデント対象文字
		else if( szSpecialIndentChar[0] != L'\0' ){
			wchar_t buf[3]; // 文字の長さは1 or 2
			wmemcpy( buf, it.getCurrentPos(), it.getIndexDelta() );
			buf[ it.getIndexDelta() ] = L'\0';
			if( NULL != wcsstr( szSpecialIndentChar, buf )){
				//	インデントのカウントを継続する
			}
			else {
				nIpos = it.getColumn();	//	終了
				break;
			}
		}
		else {
			nIpos = it.getColumn();	//	終了
			break;
		}
		it.addDelta();
	}
	if ( m_nMaxLineKetas - nIpos < 6 ){
		nIpos = pLayoutPrev->GetIndent();	//	あきらめる
	}
	return nIpos;	//	インデント
}

/*[EOF]*/
