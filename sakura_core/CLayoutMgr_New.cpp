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
	Copyright (C) 2006, Moca
	Copyright (C) 2009, nasukoji
	Copyright (C) 2010, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <mbstring.h>
#include "CLayoutMgr.h"
#include "charcode.h"
#include "etc_uty.h"
#include "Debug.h"
#include <commctrl.h>
#include "CRunningTimer.h"
#include "CLayout.h"/// 2002/2/10 aroka
#include "CDocLine.h"/// 2002/2/10 aroka
#include "CDocLineMgr.h"// 2002/2/10 aroka
#include "CMemory.h"/// 2002/2/10 aroka
#include "CMemoryIterator.h"
#include "CEditDoc.h" /// 2003/07/20 genta

//レイアウト中の禁則タイプ	//@@@ 2002.04.20 MIK
#define	KINSOKU_TYPE_NONE			0	//なし
#define	KINSOKU_TYPE_WORDWRAP		1	//ワードラップ中
#define	KINSOKU_TYPE_KINSOKU_HEAD	2	//行頭禁則中
#define	KINSOKU_TYPE_KINSOKU_TAIL	3	//行末禁則中
#define	KINSOKU_TYPE_KINSOKU_KUTO	4	//句読点ぶら下げ中



/*!
	現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します

	@date 2004.04.03 Moca TABが使われると折り返し位置がずれるのを防ぐため，
		nPosXがインデントを含む幅を保持するように変更．nMaxLineSizeは
		固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．
*/
void CLayoutMgr::_DoLayout(
		HWND	hwndProgress
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::DoLayout" );

	int			nLineNum;
	int			nLineLen;
	CDocLine*	pCDocLine;
	const char* pLine;
	int			nBgn;
	//!	メモリ上の位置(offset)
	int			nPos;
	/*	表示上のX位置
		2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
	*/
	int			nPosX;
	EColorIndexType	nCOMMENTMODE;
	EColorIndexType	nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	int			nWordBgn;
	int			nWordLen;
	int			nAllLineNum;
	int			nKinsokuType;	//@@@ 2002.04.20 MIK

	int			nIndent;			//	インデント幅
	CLayout*	pLayoutCalculated;	//	インデント幅計算済みのCLayout.
	int			nMaxLineSize;

// 2002/03/13 novice
	nCOMMENTMODE = COLORIDX_TEXT;
	nCOMMENTMODE_Prev = COLORIDX_TEXT;

	if( NULL != hwndProgress ){
		::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::SendMessage( hwndProgress, PBM_SETPOS, 0, 0 );
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( NULL ) ){
			return;
		}
	}

	_Empty();
	Init();
	nLineNum = 0;
	
	//	Nov. 16, 2002 genta
	//	折り返し幅 <= TAB幅のとき無限ループするのを避けるため，
	//	TABが折り返し幅以上の時はTAB=4としてしまう
	//	折り返し幅の最小値=10なのでこの値は問題ない
	if( m_sTypeConfig.m_nTabSpace >= m_sTypeConfig.m_nMaxLineKetas ){
		m_sTypeConfig.m_nTabSpace = 4;
	}

//	pLine = m_pcDocLineMgr->GetFirstLinrStr( &nLineLen );
	pCDocLine = m_pcDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr変更

// 2005-08-20 D.S.Koba 削除
//// 2002/03/13 novice
//	if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* 行コメントである */
//		nCOMMENTMODE_Prev = COLORIDX_TEXT;
//	}
//	nCOMMENTMODE = nCOMMENTMODE_Prev;
	nCOMMENTEND = 0;
	nAllLineNum = m_pcDocLineMgr->GetLineCount();

	/*
		2004.03.28 Moca TAB計算を正しくするためにインデントを幅で調整することはしない
		nMaxLineSizeは変更しないので，ここでm_nMaxLineKetasを設定する．
	*/
	nMaxLineSize = m_sTypeConfig.m_nMaxLineKetas;

	while( NULL != pCDocLine ){
		pLine = pCDocLine->m_cLine.GetStringPtr( &nLineLen );
		nPosX = 0;
		nBgn = 0;
		nPos = 0;
		nWordBgn = 0;
		nWordLen = 0;
		nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK

		int	nEol = pCDocLine->m_cEol.GetLen();
		int nEol_1 = nEol - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		nIndent = 0;				//	インデント幅
		pLayoutCalculated = NULL;	//	インデント幅計算済みのCLayout.

		while( nPos < nLineLen - nEol_1 ){
			//	インデント幅の計算コストを下げるための方策
			if ( m_pLayoutBot && 
				 m_pLayoutBot != pLayoutCalculated &&
				 nBgn ){
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
					if( nKinsokuType == KINSOKU_TYPE_KINSOKU_KUTO
					 && nPos == nWordBgn + nWordLen )
					{
						if( ! (m_sTypeConfig.m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol ) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{
							AddLineBottom( CreateLayout(pCDocLine, CLogicPoint( nBgn, nLineNum ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
							nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
							pLayoutCalculated = m_pLayoutBot;
						}
					}
					
					nWordLen = 0;
					nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
				}
			}
			else
			{
			
				/* ワードラップ処理 */
				if( m_sTypeConfig.m_bWordWrap	/* 英文ワードラップをする */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
//				if( 0 == nWordLen ){
					/* 英単語の先頭か */
					//int nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					int nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( nPos >= nBgn &&
						nCharChars == 1 &&
//						( pLine[nPos] == '#' || __iscsym( pLine[nPos] ) )
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						/* キーワード文字列の終端を探す */
						int	i;
						// 2005-08-27 D.S.Koba 簡略化
						for( i = nPos + 1; i < nLineLen; ++i ){
							//int nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
							int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars2 ){
								nCharChars2 = 1;
							}
							if( nCharChars2 != 1 || !IS_KEYWORD_CHAR( pLine[i] ) ){
								break;
							}
						}
						nWordBgn = nPos;
						nWordLen = i - nPos;
						nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK
						if( nPosX + i - nPos >= nMaxLineSize
						 && nPos - nBgn > 0
						){
							AddLineBottom( CreateLayout(pCDocLine, CLogicPoint( nBgn, nLineNum ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
							nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
							pLayoutCalculated = m_pLayoutBot;
//?							continue;
						}
					}
//				}else{
//					if( nPos == nWordBgn + nWordLen ){
//						nWordLen = 0;
//					}
//				}
				}

				//@@@ 2002.04.07 MIK start
				/* 句読点のぶらさげ */
				if( m_sTypeConfig.m_bKinsokuKuto
				 && (nMaxLineSize - nPosX < 2)
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					
					if( IsKinsokuPosKuto( nMaxLineSize - nPosX, nCharChars2 )
						&& IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
					}
				}

				/* 行頭禁則 */
				if( m_sTypeConfig.m_bKinsokuHead
				 && (nMaxLineSize - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosXの解釈変更のため，行頭チェックも変更
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					int nCharChars3 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosHead( nMaxLineSize - nPosX, nCharChars2, nCharChars3 )
					 && IsKinsokuHead( &pLine[nPos+nCharChars2], nCharChars3 )
					 && ! IsKinsokuHead( &pLine[nPos], nCharChars2 )	//1文字前が行頭禁則でない
					 && ! IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )	//句読点でない
					{
						//nPos += nCharChars2 + nCharChars3; nPosX += nCharChars2 + nCharChars3;
						nWordBgn = nPos;
						nWordLen = nCharChars2 + nCharChars3;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;
						AddLineBottom( CreateLayout(pCDocLine, CLogicPoint( nBgn, nLineNum ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
						m_nLineTypeBot = nCOMMENTMODE;
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
						nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
						pLayoutCalculated = m_pLayoutBot;
					}
				}

				/* 行末禁則 */
				if( m_sTypeConfig.m_bKinsokuTail
				 && (nMaxLineSize - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosXの解釈変更のため，行頭チェックも変更
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう) */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					int nCharChars3 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosTail( nMaxLineSize - nPosX, nCharChars2, nCharChars3 )
						&& IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
						AddLineBottom( CreateLayout(pCDocLine, CLogicPoint( nBgn, nLineNum ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
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
			
			if( pLine[nPos] == TAB ){
				//	Sep. 23, 2002 genta せっかく作ったので関数を使う
				int nCharChars2 = GetActualTabSpace( nPosX );
				if( nPosX + nCharChars2 > nMaxLineSize ){
					AddLineBottom( CreateLayout(pCDocLine, CLogicPoint( nBgn, nLineNum ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
					m_nLineTypeBot = nCOMMENTMODE;
					nCOMMENTMODE_Prev = nCOMMENTMODE;
					nBgn = nPos;
					// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
					nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
					pLayoutCalculated = m_pLayoutBot;
					continue;
				}
				nPosX += nCharChars2;
				nCharChars2 = 1;
				nPos+= nCharChars2;
			}else{
				// 2005-09-02 D.S.Koba GetSizeOfChar
				int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
				if( 0 == nCharChars2 ){
					nCharChars2 = 1;
					break;	//@@@ 2002.04.16 MIK
				}
				if( nPosX + nCharChars2 > nMaxLineSize ){
					if( nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
					{
						if( ! (m_sTypeConfig.m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{	//@@@ 2002.04.14 MIK
							AddLineBottom( CreateLayout(pCDocLine, CLogicPoint( nBgn, nLineNum ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
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
				nPos+= nCharChars2;
				nPosX += nCharChars2;
			}
		}
		if( nPos - nBgn > 0 ){
// 2002/03/13 novice
			if( nCOMMENTMODE == COLORIDX_COMMENT ){	/* 行コメントである */
				nCOMMENTMODE = COLORIDX_TEXT;
			}
			AddLineBottom( CreateLayout(pCDocLine, CLogicPoint( nBgn, nLineNum ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
			m_nLineTypeBot = nCOMMENTMODE;
			nCOMMENTMODE_Prev = nCOMMENTMODE;
		}
		nLineNum++;
		if( NULL != hwndProgress && 0 < nAllLineNum && 0 == ( nLineNum % 1024 ) ){
			::SendMessage( hwndProgress, PBM_SETPOS, nLineNum * 100 / nAllLineNum , 0 );
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( NULL ) ){
				return;
			}
		}
//		pLine = m_pcDocLineMgr->GetNextLinrStr( &nLineLen );
		pCDocLine = pCDocLine->m_pNext;
// 2002/03/13 novice
		if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* 行コメントである */
			nCOMMENTMODE_Prev = COLORIDX_TEXT;
		}
		nCOMMENTMODE = nCOMMENTMODE_Prev;
		nCOMMENTEND = 0;
	}
	m_nPrevReferLine = 0;
	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	if( NULL != hwndProgress ){
		::SendMessage( hwndProgress, PBM_SETPOS, 0, 0 );
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
		nPosXがインデントを含む幅を保持するように変更．nMaxLineSizeは
		固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．
	@date 2009.08.28 nasukoji	テキスト最大幅の算出に対応
		「折り返さない」選択時のみテキスト最大幅を算出する．

	@note 2004.04.03 Moca
		DoLayoutとは違ってレイアウト情報がリスト中間に挿入されるため，
		挿入後にm_nLineTypeBotへコメントモードを指定してはならない
		代わりに最終行のコメントモードを終了間際に確認している．
*/
int CLayoutMgr::DoLayout_Range(
	CLayout* pLayoutPrev,
	int		nLineNum,
	int		nDelLogicalLineFrom,
	int		nDelLogicalColFrom,
	EColorIndexType	nCurrentLineType,
	const CalTextWidthArg*	pctwArg,
	int*	pnExtInsLineNum
)
{
	int			nLineNumWork;
	int			nLineLen;
	int			nCurLine;
	CDocLine*	pCDocLine;
	const char* pLine;
	int			nBgn;
	//!	メモリ上の位置(offset)
	int			nPos;
	/*	表示上のX位置
		2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
	*/
	int			nPosX;
	CLayout*	pLayout;
	int			nModifyLayoutLinesNew;
	EColorIndexType	nCOMMENTMODE;
	EColorIndexType	nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	bool		bNeedChangeCOMMENTMODE = false;	//@@@ 2002.09.23 YAZAKI bAddを名称変更
	int			nWordBgn;
	int			nWordLen;
	int			nKinsokuType;	//@@@ 2002.04.20 MIK

	int			nIndent;			//	インデント幅
	CLayout*	pLayoutCalculated;	//	インデント幅計算済みのCLayout.
	
	//	2004.04.09 genta 関数内では値が変化しないのでループの外に出す
	int			nMaxLineSize = m_sTypeConfig.m_nMaxLineKetas;

	nLineNumWork = 0;
	*pnExtInsLineNum = 0;
	//	Jun. 22, 2005 Moca 最後の行[EOFのみ]のためにはlinenum==0でも処理が必要
	//if( 0 == nLineNum ){
	//	return 0;
	//}
	pLayout = pLayoutPrev;
	if( NULL == pLayout ){
		nCurLine = 0;
	}else{
		nCurLine = pLayout->m_ptLogicPos.y + 1;
	}
	nCOMMENTMODE = nCurrentLineType;
	nCOMMENTMODE_Prev = nCOMMENTMODE;

//	pLine = m_pcDocLineMgr->GetLineStr( nCurLine, &nLineLen );
	pCDocLine = m_pcDocLineMgr->GetLine( nCurLine );



//	if( nCOMMENTMODE_Prev == 1 ){	/* 行コメントである */
//		nCOMMENTMODE_Prev = 0;
//	}
//	nCOMMENTMODE = nCOMMENTMODE_Prev;
	nCOMMENTEND = 0;

	nModifyLayoutLinesNew = 0;

	// 2006.12.01 Moca 途中にまで再構築した場合にEOF位置がずれたまま
	//	更新されないので，範囲にかかわらず必ずリセットする．
	m_nEOFColumn = -1;
	m_nEOFLine = -1;

	while( NULL != pCDocLine ){
		pLine = pCDocLine->m_cLine.GetStringPtr( &nLineLen );
		nPosX = 0;
		nBgn = 0;
		nPos = 0;
		nWordBgn = 0;
		nWordLen = 0;
		nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK

		int	nEol = pCDocLine->m_cEol.GetLen();
		int nEol_1 = nEol - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		nIndent = 0;				//	インデント幅
		pLayoutCalculated = pLayout;	//	インデント幅計算済みのCLayout.

		while( nPos < nLineLen - nEol_1 ){
			//	インデント幅の計算コストを下げるための方策
			if ( pLayout && 
				 pLayout != pLayoutCalculated ){
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
					if( nKinsokuType == KINSOKU_TYPE_KINSOKU_KUTO
					 && nPos == nWordBgn + nWordLen )
					{
						if( ! (m_sTypeConfig.m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol ) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{
							//@@@ 2002.09.23 YAZAKI 最適化
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->m_pNext;
								pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
								(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint( nBgn, nCurLine ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
							nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;
							}
						}
					}

					nWordLen = 0;
					nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
				}
			}
			else
			{
			
				/* ワードラップ処理 */
				if( m_sTypeConfig.m_bWordWrap	/* 英文ワードラップをする */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
					/* 英単語の先頭か */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( nPos >= nBgn &&
						nCharChars == 1 &&
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						/* キーワード文字列の終端を探す */
						int	i;
						// 2005-08-27 D.S.Koba 簡略化
						for( i = nPos + 1; i < nLineLen; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars2 ){
								nCharChars2 = 1;
							}
							if( nCharChars2 != 1 || !IS_KEYWORD_CHAR( pLine[i] ) ){
								break;
							}
						}
						nWordBgn = nPos;
						nWordLen = i - nPos;
						nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK

						if( nPosX + i - nPos >= nMaxLineSize
						 && nPos - nBgn > 0
						){
							//@@@ 2002.09.23 YAZAKI 最適化
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->m_pNext;
								pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
								(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint( nBgn, nCurLine ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
							nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;
							}
//?							continue;
						}
					}
				}

				//@@@ 2002.04.07 MIK start
				/* 句読点のぶらさげ */
				if( m_sTypeConfig.m_bKinsokuKuto
				 && (nMaxLineSize - nPosX < 2)
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );

					if( IsKinsokuPosKuto( nMaxLineSize - nPosX, nCharChars2 )
						&& IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
					}
				}

				/* 行頭禁則 */
				if( m_sTypeConfig.m_bKinsokuHead
				 && (nMaxLineSize - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosXの解釈変更のため，行頭チェックも変更
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					int nCharChars3 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosHead( nMaxLineSize - nPosX, nCharChars2, nCharChars3 )
					 && IsKinsokuHead( &pLine[nPos+nCharChars2], nCharChars3 )
					 && ! IsKinsokuHead( &pLine[nPos], nCharChars2 )	//1文字前が行頭禁則でない
					 && ! IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )	//句読点でない
					{
						//nPos += nCharChars2 + nCharChars3; nPosX += nCharChars2 + nCharChars3;
						nWordBgn = nPos;
						nWordLen = nCharChars2 + nCharChars3;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;
						//@@@ 2002.09.23 YAZAKI 最適化
						if( bNeedChangeCOMMENTMODE ){
							pLayout = pLayout->m_pNext;
							pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
							(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
						}
						else {
							pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint( nBgn, nCurLine ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
						}
						nCOMMENTMODE_Prev = nCOMMENTMODE;

						nBgn = nPos;
						// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
						nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
						pLayoutCalculated = pLayout;
						if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
							( nDelLogicalLineFrom < nCurLine )
						){
							(nModifyLayoutLinesNew)++;
						}
					}
				}

				/* 行末禁則 */
				if( m_sTypeConfig.m_bKinsokuTail
				 && (nMaxLineSize - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosXの解釈変更のため，行頭チェックも変更
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう) */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					int nCharChars3 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosTail( nMaxLineSize - nPosX, nCharChars2, nCharChars3 )
						&& IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
						//@@@ 2002.09.23 YAZAKI 最適化
						if( bNeedChangeCOMMENTMODE ){
							pLayout = pLayout->m_pNext;
							pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
							(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
						}
						else {
							pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint( nBgn, nCurLine ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
						}
						nCOMMENTMODE_Prev = nCOMMENTMODE;

						nBgn = nPos;
						// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
						nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
						pLayoutCalculated = pLayout;
						if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
							( nDelLogicalLineFrom < nCurLine )
						){
							(nModifyLayoutLinesNew)++;
						}
					}
				}
				//@@@ 2002.04.08 MIK end
			}	// if( nKinsokuType != KINSOKU_TYPE_NONE )

			//@@@ 2002.09.22 YAZAKI
			bool bGotoSEARCH_START = CheckColorMODE( nCOMMENTMODE, nCOMMENTEND, nPos, nLineLen, pLine );
			if ( bGotoSEARCH_START )
				goto SEARCH_START;

			int nCharChars;
			if( pLine[nPos] == TAB ){
				//	Sep. 23, 2002 genta せっかく作ったので関数を使う
				nCharChars = GetActualTabSpace( nPosX );
				if( nPosX + nCharChars > nMaxLineSize ){
					//@@@ 2002.09.23 YAZAKI 最適化
					if( bNeedChangeCOMMENTMODE ){
						pLayout = pLayout->m_pNext;
						pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
						(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
					}
					else {
						pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint( nBgn, nCurLine ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
					}
					nCOMMENTMODE_Prev = nCOMMENTMODE;

					nBgn = nPos;
					// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
					nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
					pLayoutCalculated = pLayout;
					if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
						( nDelLogicalLineFrom < nCurLine )
					){
						(nModifyLayoutLinesNew)++;
					}
					continue;
				}
				nPos++;
			}else{
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
				if( 0 == nCharChars ){
					nCharChars = 1;
					break;	//@@@ 2002.04.16 MIK
				}
				if( nPosX + nCharChars > nMaxLineSize ){
					if( nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
					{
						if( ! (m_sTypeConfig.m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{	//@@@ 2002.04.14 MIK
							//@@@ 2002.09.23 YAZAKI 最適化
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->m_pNext;
								pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
								(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint( nBgn, nCurLine ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
							nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;
							}
							continue;
						}
					}
				}
				nPos+= nCharChars;
			}

			nPosX += nCharChars;
		}
		if( nPos - nBgn > 0 ){
// 2002/03/13 novice
			if( nCOMMENTMODE == COLORIDX_COMMENT ){	/* 行コメントである */
				nCOMMENTMODE = COLORIDX_TEXT;
			}
			//@@@ 2002.09.23 YAZAKI 最適化
			if( bNeedChangeCOMMENTMODE ){
				pLayout = pLayout->m_pNext;
				pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
				(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
			}
			else {
				pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint( nBgn, nCurLine ), nPos - nBgn, nCOMMENTMODE_Prev, nIndent, nPosX) );
			}
			nCOMMENTMODE_Prev = nCOMMENTMODE;

			if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
				( nDelLogicalLineFrom < nCurLine )
			){
				(nModifyLayoutLinesNew)++;
			}
		}

		nLineNumWork++;
		nCurLine++;

		/* 目的の行数(nLineNum)に達したか、または通り過ぎた（＝行数が増えた）か確認 */
		//@@@ 2002.09.23 YAZAKI 最適化
		if( nLineNumWork >= nLineNum ){
			if( NULL != pLayout
			 && NULL != pLayout->m_pNext
			 && ( nCOMMENTMODE_Prev != pLayout->m_pNext->m_nTypePrev )
			){
				//	COMMENTMODEが異なる行が増えましたので、次の行→次の行と更新していきます。
				bNeedChangeCOMMENTMODE = true;
			}else{
				break;	//	while( NULL != pCDocLine ) 終了
			}
		}
//		pLine = m_pcDocLineMgr->GetNextLinrStr( &nLineLen );
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
		m_nEOFColumn = -1;
		m_nEOFLine = -1;
	}

	// 2009.08.28 nasukoji	テキストが編集されたら最大幅を算出する
	CalculateTextWidth_Range(pctwArg);

// 1999.12.22 レイアウト情報がなくなる訳ではないので
//	m_nPrevReferLine = 0;
//	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	return nModifyLayoutLinesNew;
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
		int nCalTextWidthLinesFrom;		// テキスト最大幅の算出開始レイアウト行
		int nCalTextWidthLinesTo;		// テキスト最大幅の算出終了レイアウト行
		BOOL bCalTextWidth = TRUE;		// テキスト最大幅の算出要求をON
		int nInsLineNum    = m_nLines - pctwArg->nAllLinesOld;		// 追加削除行数

		// 削除行なし時：最大幅の行を行頭以外にて改行付きで編集した
		// 削除行あり時：最大幅の行を含んで編集した
		if(( pctwArg->nDelLines < 0  && m_nTextWidth &&
		     nInsLineNum && pctwArg->ptLayout.x && m_nTextWidthMaxLine == pctwArg->ptLayout.y )||
		   ( pctwArg->nDelLines >= 0 && m_nTextWidth &&
		     pctwArg->ptLayout.y <= m_nTextWidthMaxLine && m_nTextWidthMaxLine <= pctwArg->ptLayout.y + pctwArg->nDelLines ))
		{
			// 全ラインを走査する
			nCalTextWidthLinesFrom = -1;
			nCalTextWidthLinesTo   = -1;
		}else if( nInsLineNum || pctwArg->bInsData ){		// 追加削除行 または 追加文字列あり
			// 追加削除行のみを走査する
			nCalTextWidthLinesFrom = pctwArg->ptLayout.y;

			// 最終的に編集された行数（3行削除2行追加なら2行追加）
			// 　1行がMAXLINEKETASを超える場合行数が合わなくなるが、超える場合はその先の計算自体が
			// 　不要なので計算を省くためこのままとする。
			int nEditLines = nInsLineNum + ((pctwArg->nDelLines > 0) ? pctwArg->nDelLines : 0);
			nCalTextWidthLinesTo   = pctwArg->ptLayout.y + ((nEditLines > 0) ? nEditLines : 0);

			// 最大幅の行が上下するのを計算
			if( m_nTextWidth && nInsLineNum && m_nTextWidthMaxLine >= pctwArg->ptLayout.y )
				m_nTextWidthMaxLine += nInsLineNum;
		}else{
			// 最大幅以外の行を改行を含まずに（1行内で）編集した
			bCalTextWidth = FALSE;		// テキスト最大幅の算出要求をOFF
		}

#ifdef _DEBUG
		static int testcount = 0;
		testcount++;

		// テキスト最大幅を算出する
		if( bCalTextWidth ){
//			MYTRACE( _T("CLayoutMgr::DoLayout_Range(%d) nCalTextWidthLinesFrom=%d nCalTextWidthLinesTo=%d\n"), testcount, nCalTextWidthLinesFrom, nCalTextWidthLinesTo );
			CalculateTextWidth( FALSE, nCalTextWidthLinesFrom, nCalTextWidthLinesTo );
//			MYTRACE( _T("CLayoutMgr::DoLayout_Range() m_nTextWidthMaxLine=%d\n"), m_nTextWidthMaxLine );
		}else{
//			MYTRACE( _T("CLayoutMgr::DoLayout_Range(%d) FALSE\n"), testcount );
		}
#else
		// テキスト最大幅を算出する
		if( bCalTextWidth )
			CalculateTextWidth( FALSE, nCalTextWidthLinesFrom, nCalTextWidthLinesTo );
#endif
	}
}

/*!
	行頭禁則文字に該当するかを調べる．

	@param[in] pLine 調べる文字へのポインタ
	@param[in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool CLayoutMgr::IsKinsokuHead( const char *pLine, int length )
{
	const unsigned char	*p;

	if(      length == 1 ) p = (const unsigned char *)m_pszKinsokuHead_1;
	else if( length == 2 ) p = (const unsigned char *)m_pszKinsokuHead_2;
	else return false;

	if( ! p ) return false;

	for( ; *p; p += length )
	{
		if( memcmp( pLine, p, length ) == 0 ) return true;
	}

	return false;
}

/*!
	行末禁則文字に該当するかを調べる．

	@param[in] pLine 調べる文字へのポインタ
	@param[in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool CLayoutMgr::IsKinsokuTail( const char *pLine, int length )
{
	const unsigned char	*p;
	
	if(      length == 1 ) p = (const unsigned char *)m_pszKinsokuTail_1;
	else if( length == 2 ) p = (const unsigned char *)m_pszKinsokuTail_2;
	else return false;
	
	if( ! p ) return false;

	for( ; *p; p += length )
	{
		if( memcmp( pLine, p, length ) == 0 ) return true;
	}

	return false;
}

/*!
	禁則対象句読点に該当するかを調べる．

	@param [in] pLine 調べる文字へのポインタ
	@param [in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool CLayoutMgr::IsKinsokuKuto( const char *pLine, int length )
{
	const unsigned char	*p;
	
	if(      length == 1 ) p = (const unsigned char *)m_pszKinsokuKuto_1;
	else if( length == 2 ) p = (const unsigned char *)m_pszKinsokuKuto_2;
	else return false;
	
	if( ! p ) return false;

	for( ; *p; p += length )
	{
		if( memcmp( pLine, p, length ) == 0 ) return true;
	}

	return false;
}

/*!
	@date 2005-08-20 D.S.Koba _DoLayout()とDoLayout_Range()から分離
*/
bool CLayoutMgr::IsKinsokuPosHead(
	const int nRest,		//!< [in] 行の残り文字数
	const int nCharKetas,	//!< [in] 現在位置の文字サイズ
	const int nCharKetas2	//!< [in] 現在位置の次の文字サイズ
)
{
	switch( nRest )
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
	@date 2005-08-20 D.S.Koba DoLayout()とDoLayout_Range()から分離
*/
bool CLayoutMgr::IsKinsokuPosTail(
	const int nRest,		//!< [in] 行の残り文字数
	const int nCharKetas,	//!< [in] 現在位置の文字サイズ
	const int nCharKetas2	//!< [in] 現在位置の次の文字サイズ
)
{
	switch( nRest )
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

int CLayoutMgr::Match_Quote( char szQuote, int nPos, int nLineLen, const char* pLine )
{
	int nCharChars;
	int i;
	for( i = nPos; i < nLineLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( (const char *)pLine, nLineLen, i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if(	m_sTypeConfig.m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && pLine[i] == _T('\\') ){
				++i;
			}else
			if( 1 == nCharChars && pLine[i] == szQuote ){
				return i + 1;
			}
		}
		else if( m_sTypeConfig.m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && pLine[i] == szQuote ){
				if( i + 1 < nLineLen && pLine[i + 1] == szQuote ){
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

bool CLayoutMgr::CheckColorMODE( EColorIndexType &nCOMMENTMODE, int &nCOMMENTEND, int nPos, int nLineLen, const char* pLine )
{
	switch( nCOMMENTMODE ){
	case COLORIDX_TEXT: // 2002/03/13 novice
		// 2005.11.20 Mocaコメントの色分けがON/OFF関係なく行われていたバグを修正
		if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp && m_sTypeConfig.m_cLineComment.Match( nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice
		}else
		if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp && m_sTypeConfig.m_cBlockComments[0].Match_CommentFrom(nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_sTypeConfig.m_cBlockComments[0].Match_CommentTo(nPos + m_sTypeConfig.m_cBlockComments[0].getBlockFromLen(), nLineLen, pLine );
		}else
		if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&  m_sTypeConfig.m_cBlockComments[1].Match_CommentFrom(nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_sTypeConfig.m_cBlockComments[1].Match_CommentTo(nPos + m_sTypeConfig.m_cBlockComments[1].getBlockFromLen(), nLineLen, pLine );
		}else
		if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp && /* シングルクォーテーション文字列を表示する */
			pLine[nPos] == '\''
		){
			nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
			/* シングルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( '\'', nPos + 1, nLineLen, pLine );
		}else
		if( m_sTypeConfig.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp && /* ダブルクォーテーション文字列を表示する */
			pLine[nPos] == '"'
		){
			nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
			/* ダブルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( '"', nPos + 1, nLineLen, pLine );
		}
		break;
	case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
		break;
	case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_sTypeConfig.m_cBlockComments[0].Match_CommentTo(nPos, nLineLen, pLine );
		}else
		if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_sTypeConfig.m_cBlockComments[1].Match_CommentTo(nPos, nLineLen, pLine );
		}else
		if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* シングルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( '\'', nPos, nLineLen, pLine );
		}else
		if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* ダブルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( '"', nPos, nLineLen, pLine );
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
int CLayoutMgr::getIndentOffset_Normal( CLayout* )
{
	return 0;
}

/*!
	@brief インデント幅を計算する (Tx2x)
	
	前の行の最後のTABの位置をインデント位置として返す．
	ただし，残り幅が(m_nTabSpace + 2)文字未満の場合はインデントを行わない．
	
	@author Yazaki
	@return インデントすべき文字数
	
	@date 2002.10.01 
	@date 2002.10.07 YAZAKI 名称変更, 処理見直し
*/
int CLayoutMgr::getIndentOffset_Tx2x( CLayout* pLayoutPrev )
{
	//	前の行が無いときは、インデント不要。
	if ( pLayoutPrev == NULL ) return 0;

	int nIpos = pLayoutPrev->GetIndent();

	//	前の行が折り返し行ならばそれに合わせる
	if( pLayoutPrev->m_ptLogicPos.x > 0 )
		return nIpos;
	
	CMemoryIterator<CLayout> it( pLayoutPrev, m_sTypeConfig.m_nTabSpace );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && it.getCurrentChar() == TAB ){
			nIpos = it.getColumn() + it.getColumnDelta();
		}
		it.addDelta();
	}
	// 2010.07.06 Moca TAB=8などの場合に折り返すと無限ループする不具合の修正. 6固定を m_nTabSpace + 2に変更
	if ( m_sTypeConfig.m_nMaxLineKetas - nIpos < m_sTypeConfig.m_nTabSpace + 2 ){
		nIpos = pLayoutPrev->GetIndent();	//	あきらめる
	}
	return nIpos;	//	インデント
}

/*!
	@brief インデント幅を計算する (スペース字下げ版)
	
	論理行行頭のホワイトスペースの終わりインデント位置として返す．
	ただし，残り幅が(m_nTabSpace + 2)文字未満の場合はインデントを行わない．
	
	@author genta
	@return インデントすべき文字数
	
	@date 2002.10.01 
*/
int CLayoutMgr::getIndentOffset_LeftSpace( CLayout* pLayoutPrev )
{
	//	前の行が無いときは、インデント不要。
	if ( pLayoutPrev == NULL ) return 0;

	//	インデントの計算
	int nIpos = pLayoutPrev->GetIndent();
	
	//	Oct. 5, 2002 genta
	//	折り返しの3行目以降は1つ前の行のインデントに合わせる．
	if( pLayoutPrev->m_ptLogicPos.x > 0 )
		return nIpos;
	
	//	2002.10.07 YAZAKI インデントの計算
	CMemoryIterator<CLayout> it( pLayoutPrev, m_sTypeConfig.m_nTabSpace );

	//	Jul. 20, 2003 genta 自動インデントに準じた動作にする
	bool bZenSpace = m_pcEditDoc->GetDocumentAttribute().m_bAutoIndent_ZENSPACE != false ? 1 : 0;
	const char* szSpecialIndentChar = m_pcEditDoc->GetDocumentAttribute().m_szIndentChars;
	while( !it.end() ){
		it.scanNext();
		if (( it.getIndexDelta() == 1 && (it.getCurrentChar() == TAB || it.getCurrentChar() == ' ') ) ||
			//	Jul. 20, 2003 genta 全角スペース対応
			( bZenSpace && it.getIndexDelta() == 2 &&
				it.getCurrentChar() == (char)0x81 && it.getCurrentPos()[1] == (char)0x40 ))
		{
			//	インデントのカウントを継続する
		}
		//	Jul. 20, 2003 genta インデント対象文字
		else if( szSpecialIndentChar[0] != _T('\0') ){
			unsigned char buf[3]; // 文字の長さは1 or 2
			memcpy( buf, it.getCurrentPos(), it.getIndexDelta() );
			buf[ it.getIndexDelta() ] = _T('\0');
			if( NULL != _mbsstr( (const unsigned char*)szSpecialIndentChar, buf )){
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
	// 2010.07.06 Moca TAB=8などの場合に折り返すと無限ループする不具合の修正. 6固定を m_nTabSpace + 2に変更
	if ( m_sTypeConfig.m_nMaxLineKetas - nIpos < m_sTypeConfig.m_nTabSpace + 2 ){
		nIpos = pLayoutPrev->GetIndent();	//	あきらめる
	}
	return nIpos;	//	インデント
}

/*!
	@brief  テキスト最大幅を算出する

	指定されたラインを走査してテキストの最大幅を作成する。
	全て削除された時は未算出状態に戻す。

	@param bCalLineLen	[in] 各レイアウト行の長さの算出も行う
	@param nStart		[in] 算出開始レイアウト行
	@param nEnd			[in] 算出終了レイアウト行

	@retval TRUE 最大幅が変化した
	@retval FALSE 最大幅が変化しなかった

	@note nStart, nEndが両方とも-1の時、全ラインを走査する
		  範囲が指定されている場合は最大幅の拡大のみチェックする

	@date 2009.08.28 nasukoji	新規作成
*/
BOOL CLayoutMgr::CalculateTextWidth( BOOL bCalLineLen, int nStart, int nEnd )
{
	BOOL bRet = FALSE;
	BOOL bOnlyExpansion = TRUE;		// 最大幅の拡大のみをチェックする
	int nMaxLen = 0;
	int nMaxLineNum = 0;

	int nLines = GetLineCount();		// テキストのレイアウト行数

	// 開始・終了位置がどちらも指定されていない
	if( nStart < 0 && nEnd < 0 )
		bOnlyExpansion = FALSE;		// 最大幅の拡大・縮小をチェックする

	if( nStart < 0 )			// 算出開始行の指定なし
		nStart = 0;
	else if( nStart > nLines )	// 範囲オーバー
		nStart = nLines;
	
	if( nEnd < 0 || nEnd >= nLines )	// 算出終了行の指定なし または 文書行数以上
		nEnd = nLines;
	else
		nEnd++;					// 算出終了行の次行

	CLayout* pLayout;

	// 算出開始レイアウト行を探す
	if( nStart * 2 < nLines ){
		// 前方からサーチ
		int nCount = 0;
		pLayout = m_pLayoutTop;
		while( NULL != pLayout ){
			if( nStart == nCount ){
				break;
			}
			pLayout = pLayout->m_pNext;
			nCount++;
		}
	}else{
		// 後方からサーチ
		int nCount = m_nLines - 1;
		pLayout = m_pLayoutBot;
		while( NULL != pLayout ){
			if( nStart == nCount ){
				break;
			}
			pLayout = pLayout->m_pPrev;
			nCount--;
		}
	}

	// レイアウト行の最大幅を取り出す
	for( int i = nStart; i < nEnd; i++ ){
		if( !pLayout )
			break;

		// レイアウト行の長さを算出する
		if( bCalLineLen ){
			CMemoryIterator<CLayout> it( pLayout, GetTabSpace() );
			while( !it.end() ){
				it.scanNext();
				it.addDelta();
			}
			
			// 算出した長さを設定
			pLayout->m_nLayoutWidth = it.getColumn();
		}

		// 最大幅を更新
		if( nMaxLen < pLayout->m_nLayoutWidth ){
			nMaxLen = pLayout->m_nLayoutWidth;
			nMaxLineNum = i;		// 最大幅のレイアウト行

			// アプリケーションの最大幅となったら算出は停止
			if( nMaxLen >= MAXLINEKETAS && !bCalLineLen )
				break;
		}

		// 次のレイアウト行のデータ
		pLayout = pLayout->m_pNext;
	}

	// テキストの幅の変化をチェック
	if( nMaxLen ){
		// 最大幅が拡大した または 最大幅の拡大のみチェックでない
		if( m_nTextWidth < nMaxLen || !bOnlyExpansion ){
			m_nTextWidthMaxLine = nMaxLineNum;
			if( m_nTextWidth != nMaxLen ){	// 最大幅変化あり
				m_nTextWidth = nMaxLen;
				bRet = TRUE;
			}
		}
	}else if( m_nTextWidth && !nLines ){
		// 全削除されたら幅の記憶をクリア
		m_nTextWidthMaxLine = 0;
		m_nTextWidth = 0;
		bRet = TRUE;
	}
	
	return bRet;
}

/*!
	@brief  各行のレイアウト行長の記憶をクリアする
	
	@note 折り返し方法が「折り返さない」以外の時は、パフォーマンスの低下を
		  防止する目的で各行のレイアウト行長(m_nLayoutWidth)を計算していない。
		  後で設計する人が誤って使用してしまうかもしれないので「折り返さない」
		  以外の時はクリアしておく。
		  パフォーマンスの低下が気にならない程なら、全ての折り返し方法で計算
		  するようにしても良いと思う。

	@date 2009.08.28 nasukoji	新規作成
*/
void CLayoutMgr::ClearLayoutLineWidth( void )
{
	CLayout* pLayout = m_pLayoutTop;

	while( pLayout ){
		pLayout->m_nLayoutWidth = 0;	// レイアウト行長をクリア
		pLayout = pLayout->m_pNext;		// 次のレイアウト行のデータ
		
	}
}

/*[EOF]*/
