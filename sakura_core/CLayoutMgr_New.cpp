//	$Id$
/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CLayoutMgr.h"
#include "charcode.h"
#include "etc_uty.h"
#include "debug.h"
#include <commctrl.h>
#include "CRunningTimer.h"
#include "CLayout.h"/// 2002/2/10 aroka
#include "CDocLine.h"/// 2002/2/10 aroka
#include "CDocLineMgr.h"// 2002/2/10 aroka
#include "CMemory.h"/// 2002/2/10 aroka
//#include "CShareData.h" // 2002/03/13 novice	@@@ 2002.09.29 YAZAKI
#include "CMemoryIterator.h"

//レイアウト中の禁則タイプ	//@@@ 2002.04.20 MIK
#define	KINSOKU_TYPE_NONE			0	//なし
#define	KINSOKU_TYPE_WORDWRAP		1	//ワードラップ中
#define	KINSOKU_TYPE_KINSOKU_HEAD	2	//行頭禁則中
#define	KINSOKU_TYPE_KINSOKU_TAIL	3	//行末禁則中
#define	KINSOKU_TYPE_KINSOKU_KUTO	4	//句読点ぶら下げ中



/*
|| 	現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します
||
*/
void CLayoutMgr::DoLayout(
		HWND	hwndProgress,
		BOOL	bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
		BOOL	bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::DoLayout" );

	int			nLineNum;
	int			nLineLen;
	CDocLine*	pCDocLine;
	const char* pLine;
	int			nBgn;
	int			nPos;
	int			nPosX;
	int			nCharChars;
	int			nCharChars2;
	int			nCOMMENTMODE;
	int			nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	int			nWordBgn;
	int			nWordLen;
	int			nAllLineNum;
	bool		bKinsokuFlag;	//@@@ 2002.04.08 MIK
	int			nCharChars3;	//@@@ 2002.04.17 MIK
	int			nKinsokuType;	//@@@ 2002.04.20 MIK

	int			nIndent;			//	インデント幅
	CLayout*	pLayoutCalculated;	//	インデント幅計算済みのCLayout.
	int			nMaxLineSize;

// 2002/03/13 novice
	nCOMMENTMODE = COLORIDX_TEXT;
	nCOMMENTMODE_Prev = COLORIDX_TEXT;

	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( NULL ) ){
			return;
		}
	}

	Empty();
	Init();
	nLineNum = 0;
	
	//	Nov. 16, 2002 genta
	//	折り返し幅 <= TAB幅のとき無限ループするのを避けるため，
	//	TABが折り返し幅以上の時はTAB=4としてしまう
	//	折り返し幅の最小値=10なのでこの値は問題ない
	if( m_nTabSpace >= m_nMaxLineSize ){
		m_nTabSpace = 4;
	}

//	pLine = m_pcDocLineMgr->GetFirstLinrStr( &nLineLen );
	pCDocLine = m_pcDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr変更

// 2002/03/13 novice
	if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* 行コメントである */
		nCOMMENTMODE_Prev = COLORIDX_TEXT;
	}
	nCOMMENTMODE = nCOMMENTMODE_Prev;
	nCOMMENTEND = 0;
	nAllLineNum = m_pcDocLineMgr->GetLineCount();
//	while( NULL != pLine ){
	while( NULL != pCDocLine ){
		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );
		nPosX = 0;
		nCharChars = 0;
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
		nMaxLineSize = m_nMaxLineSize;

		while( nPos < nLineLen - nEol_1 ){
			//	インデント幅の計算コストを下げるための方策
			if ( m_pLayoutBot && 
				 m_pLayoutBot != pLayoutCalculated &&
				 nBgn ){
				//	計算
				//	Oct, 1, 2002 genta Indentサイズを取得するように変更
				nIndent = (this->*getIndentOffset)( m_pLayoutBot );
				nMaxLineSize = m_nMaxLineSize - nIndent;
				//nMaxLineSize = getMaxLineSize( m_pLayoutBot );
				//nIndent = m_nMaxLineSize - nMaxLineSize;
				
				//	計算済み
				pLayoutCalculated = m_pLayoutBot;
			}

			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
			if( 0 == nCharChars ){
				nCharChars = 1;
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
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol ) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{
							AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							nPosX = 0;
						}
					}
					
					nWordLen = 0;
					nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
				}
			}
			else
			{
			
				/* ワードラップ処理 */
				if( m_bWordWrap	/* 英文ワードラップをする */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
//				if( 0 == nWordLen ){
					/* 英単語の先頭か */
					if( nPos >= nBgn &&
						nCharChars == 1 &&
//						( pLine[nPos] == '#' || __iscsym( pLine[nPos] ) )
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						/* キーワード文字列の終端を探す */
						int	i;
						for( i = nPos + 1; i <= nLineLen - 1; ){
							nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
							if( 0 == nCharChars2 ){
								nCharChars2 = 1;
							}
							if( nCharChars2 == 1 &&
//								( pLine[i] == '#' || __iscsym( pLine[i] ) )
								IS_KEYWORD_CHAR( pLine[i] )
							){
							}else{
								break;
							}
							i += nCharChars2;
						}
						nWordBgn = nPos;
						nWordLen = i - nPos;
						nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK
						if( nPosX + i - nPos >= nMaxLineSize
						 && nPos - nBgn > 0
						){
							AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							nPosX = 0;
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
				if( m_bKinsokuKuto
				 && (nMaxLineSize - nPosX < 2)
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					switch( nMaxLineSize - nPosX )
					{
					case 1:	// 1文字前
						if( nCharChars2 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					case 0:	// 
						if( nCharChars2 == 1 || nCharChars2 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag && IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
					}
				}

				/* 行頭禁則 */
				if( m_bKinsokuHead
				 && (nMaxLineSize - nPosX < 4)
				 && nPosX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					nCharChars3 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos+nCharChars2] ) - &pLine[nPos+nCharChars2];
					switch( nMaxLineSize - nPosX )
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
						if( nCharChars2 == 2 && nCharChars3 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					case 2:	// 2文字前
						if( 2 == nCharChars2 /*&& nCharChars3 > 0*/ )
						{
							bKinsokuFlag = true;
						}
						else if( nCharChars2 == 1 )
						{
							if( nCharChars3 == 2 )
							{
								bKinsokuFlag = true;
							}
						}
						break;
					case 1:	// 1文字前
						if( nCharChars2 == 1 /*&& nCharChars3 > 0*/ )
						{
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag
					 && IsKinsokuHead( &pLine[nPos+nCharChars2], nCharChars3 )
					 && ! IsKinsokuHead( &pLine[nPos], nCharChars2 )	//1文字前が行頭禁則でない
					 && ! IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )	//句読点でない
					{
						//nPos += nCharChars2 + nCharChars3; nPosX += nCharChars2 + nCharChars3;
						nWordBgn = nPos;
						nWordLen = nCharChars2 + nCharChars3;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;
						AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						m_nLineTypeBot = nCOMMENTMODE;
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						nPosX = 0;
					}
				}

				/* 行末禁則 */
				if( m_bKinsokuTail
				 && (nMaxLineSize - nPosX < 4)
				 && nPosX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう) */
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					switch( nMaxLineSize - nPosX )
					{
					case 3:	// 3文字前
						if( nCharChars2 == 2 )
						{
							if( CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos+nCharChars2] ) - &pLine[nPos+nCharChars2] == 2 )
							{
								// "（あ": "あ"の2バイト目で折り返しのとき
								bKinsokuFlag = true;
							}
						}
						break;
					case 2:	// 2文字前
						if( nCharChars2 == 2 )
						{
							// "（あ": "あ"で折り返しのとき
							bKinsokuFlag = true;
						}
						else if( nCharChars2 == 1 )
						{
							if( CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos+nCharChars2] ) - &pLine[nPos+nCharChars2] == 2 )
							{
								// "(あ": "あ"の2バイト目で折り返しのとき
								bKinsokuFlag = true;
							}
						}
						break;
					case 1:	// 1文字前
						if( nCharChars2 == 1 )
						{
							// "(あ": "あ"で折り返しのとき
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag && IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
						AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						m_nLineTypeBot = nCOMMENTMODE;
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						nPosX = 0;
					}
				}
				//@@@ 2002.04.08 MIK end
			}	//if( KINSOKU_TYPE_NONE != nKinsokuTyoe ) 禁則処理中

			//@@@ 2002.09.22 YAZAKI
			bool bGotoSEARCH_START = CheckColorMODE( nCOMMENTMODE, nCOMMENTEND, nPos, nLineLen, pLine, bDispSSTRING, bDispWSTRING );
			if ( bGotoSEARCH_START )
				goto SEARCH_START;
			
			if( pLine[nPos] == TAB ){
				//	Sep. 23, 2002 genta せっかく作ったので関数を使う
				nCharChars = GetActualTabSpace( nPosX );
				if( nPosX + nCharChars > nMaxLineSize ){
					AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
					m_nLineTypeBot = nCOMMENTMODE;
					nCOMMENTMODE_Prev = nCOMMENTMODE;
					nBgn = nPos;
					nPosX = 0;
					continue;
				}
				nPosX += nCharChars;
				nCharChars = 1;
				nPos+= nCharChars;
			}else{
				nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
				if( 0 == nCharChars ){
					nCharChars = 1;
					break;	//@@@ 2002.04.16 MIK
				}
				if( nPosX + nCharChars > nMaxLineSize ){
					if( nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
					{
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{	//@@@ 2002.04.14 MIK
							AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							nPosX = 0;
							continue;
						}	//@@@ 2002.04.14 MIK
					}
				}
				nPos+= nCharChars;
				nPosX += nCharChars;
			}
		}
		if( nPos - nBgn > 0 ){
// 2002/03/13 novice
			if( nCOMMENTMODE == COLORIDX_COMMENT ){	/* 行コメントである */
				nCOMMENTMODE = COLORIDX_TEXT;
			}
			AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
			m_nLineTypeBot = nCOMMENTMODE;
			nCOMMENTMODE_Prev = nCOMMENTMODE;
		}
		nLineNum++;
		if( NULL != hwndProgress && 0 < nAllLineNum && 0 == ( nLineNum % 1024 ) ){
			::PostMessage( hwndProgress, PBM_SETPOS, nLineNum * 100 / nAllLineNum , 0 );
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
	m_nPrevReferLine = 0;
	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
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
*/
int CLayoutMgr::DoLayout_Range(
			CLayout* pLayoutPrev,
			int		nLineNum,
			int		nDelLogicalLineFrom,
			int		nDelLogicalColFrom,
			int		nCurrentLineType,
			int*	pnExtInsLineNum,
			BOOL	bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
			BOOL	bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
)
{
	int			nLineNumWork;
	int			nLineLen;
	int			nCurLine;
	CDocLine*	pCDocLine;
	const char* pLine;
	int			nBgn;
	int			nPos;
	int			nPosX;
	int			nCharChars;
	int			nCharChars2;
	CLayout*	pLayout;
	int			nModifyLayoutLinesNew;
	int			nCOMMENTMODE;
	int			nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	bool		bNeedChangeCOMMENTMODE = false;	//@@@ 2002.09.23 YAZAKI bAddを名称変更
	int			nWordBgn;
	int			nWordLen;
	bool		bKinsokuFlag;	//@@@ 2002.04.08 MIK
	int			nCharChars3;	//@@@ 2002.04.17 MIK
	int			nKinsokuType;	//@@@ 2002.04.20 MIK

	int			nIndent;			//	インデント幅
	CLayout*	pLayoutCalculated;	//	インデント幅計算済みのCLayout.
	int			nMaxLineSize;

	nLineNumWork = 0;
	*pnExtInsLineNum = 0;
	if( 0 == nLineNum ){
		return 0;
	}
	pLayout = pLayoutPrev;
	if( NULL == pLayout ){
		nCurLine = 0;
	}else{
		nCurLine = pLayout->m_nLinePhysical + 1;
	}
	nCOMMENTMODE = nCurrentLineType;
	nCOMMENTMODE_Prev = nCOMMENTMODE;

//	pLine = m_pcDocLineMgr->GetLineStr( nCurLine, &nLineLen );
	pCDocLine = m_pcDocLineMgr->GetLineInfo( nCurLine );




//	if( nCOMMENTMODE_Prev == 1 ){	/* 行コメントである */
//		nCOMMENTMODE_Prev = 0;
//	}
//	nCOMMENTMODE = nCOMMENTMODE_Prev;
	nCOMMENTEND = 0;

	nModifyLayoutLinesNew = 0;

//	while( NULL != pLine ){
	while( NULL != pCDocLine ){
		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );
		nPosX = 0;
		nCharChars = 0;
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
		nMaxLineSize = m_nMaxLineSize;

		while( nPos < nLineLen - nEol_1 ){
			//	インデント幅の計算コストを下げるための方策
			if ( pLayout && 
				 pLayout != pLayoutCalculated ){
				//	計算
				//	Oct, 1, 2002 genta Indentサイズを取得するように変更
				nIndent = (this->*getIndentOffset)( pLayout );
				nMaxLineSize = m_nMaxLineSize - nIndent;
				//nMaxLineSize = getMaxLineSize( pLayout );
				//nIndent = m_nMaxLineSize - nMaxLineSize;

				//	計算済み
				pLayoutCalculated = pLayout;
			}

			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
			if( 0 == nCharChars ){
				nCharChars = 1;
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
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol ) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{
							//@@@ 2002.09.23 YAZAKI 最適化
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->m_pNext;
								pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
								(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							nPosX = 0;
							if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
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
				if( m_bWordWrap	/* 英文ワードラップをする */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
//				if( 0 == nWordLen ){
					/* 英単語の先頭か */
					if( nPos >= nBgn &&
						nCharChars == 1 &&
//						( pLine[nPos] == '#' || __iscsym( pLine[nPos] ) )
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						/* キーワード文字列の終端を探す */
						int	i;
						for( i = nPos + 1; i <= nLineLen - 1; ){
							nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
							if( 0 == nCharChars2 ){
								nCharChars2 = 1;
							}
							if( nCharChars2 == 1 &&
								IS_KEYWORD_CHAR( pLine[i] ) ){	//@@@ 2002.09.23 YAZAKI
							}else{
								break;
							}
							i += nCharChars2;
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
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							nPosX = 0;
							if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
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
				if( m_bKinsokuKuto
				 && (nMaxLineSize - nPosX < 2)
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					switch( nMaxLineSize - nPosX )
					{
					case 1:	// 1文字前
						if( nCharChars2 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					case 0:	// 
						if( nCharChars2 == 1 || nCharChars2 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag && IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
					}
				}

				/* 行頭禁則 */
				if( m_bKinsokuHead
				 && (nMaxLineSize - nPosX < 4)
				 && nPosX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					nCharChars3 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos+nCharChars2] ) - &pLine[nPos+nCharChars2];
					switch( nMaxLineSize - nPosX )
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
						if( nCharChars2 == 2 && nCharChars3 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					case 2:	// 2文字前
						if( 2 == nCharChars2 /*&& nCharChars3 > 0*/ )
						{
							bKinsokuFlag = true;
						}
						else if( nCharChars2 == 1 )
						{
							if( nCharChars3 == 2 )
							{
								bKinsokuFlag = true;
							}
						}
						break;
					case 1:	// 1文字前
						if( nCharChars2 == 1 /*&& nCharChars3 > 0*/ )
						{
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag
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
							pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						}
						m_nLineTypeBot = nCOMMENTMODE;
						nCOMMENTMODE_Prev = nCOMMENTMODE;

						nBgn = nPos;
						nPosX = 0;
						if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
							( nDelLogicalLineFrom < nCurLine )
						){
							(nModifyLayoutLinesNew)++;;
						}
					}
				}

				/* 行末禁則 */
				if( m_bKinsokuTail
				 && (nMaxLineSize - nPosX < 4)
				 && nPosX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう) */
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					switch( nMaxLineSize - nPosX )
					{
					case 3:	// 3文字前
						if( nCharChars2 == 2 )
						{
							if( CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos+nCharChars2] ) - &pLine[nPos+nCharChars2] == 2 )
							{
								// "（あ": "あ"の2バイト目で折り返しのとき
								bKinsokuFlag = true;
							}
						}
						break;
					case 2:	// 2文字前
						if( nCharChars2 == 2 )
						{
							// "（あ": "あ"で折り返しのとき
							bKinsokuFlag = true;
						}
						else if( nCharChars2 == 1 )
						{
							if( CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos+nCharChars2] ) - &pLine[nPos+nCharChars2] == 2 )
							{
								// "(あ": "あ"の2バイト目で折り返しのとき
								bKinsokuFlag = true;
							}
						}
						break;
					case 1:	// 1文字前
						if( nCharChars2 == 1 )
						{
							// "(あ": "あ"で折り返しのとき
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag && IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
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
							pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						}
						m_nLineTypeBot = nCOMMENTMODE;
						nCOMMENTMODE_Prev = nCOMMENTMODE;

						nBgn = nPos;
						nPosX = 0;
						if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
							( nDelLogicalLineFrom < nCurLine )
						){
							(nModifyLayoutLinesNew)++;;
						}
					}
				}
				//@@@ 2002.04.08 MIK end
			}	// if( nKinsokuType != KINSOKU_TYPE_NONE )

			//@@@ 2002.09.22 YAZAKI
			bool bGotoSEARCH_START = CheckColorMODE( nCOMMENTMODE, nCOMMENTEND, nPos, nLineLen, pLine, bDispSSTRING, bDispWSTRING );
			if ( bGotoSEARCH_START )
				goto SEARCH_START;

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
						pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
					}
					m_nLineTypeBot = nCOMMENTMODE;
					nCOMMENTMODE_Prev = nCOMMENTMODE;

					nBgn = nPos;
					nPosX = 0;
					if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
						( nDelLogicalLineFrom < nCurLine )
					){
						(nModifyLayoutLinesNew)++;;
					}
					continue;
				}
				nPos++;
			}else{
				nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
				if( 0 == nCharChars ){
					nCharChars = 1;
					break;	//@@@ 2002.04.16 MIK
				}
				if( nPosX + nCharChars > nMaxLineSize ){
					if( nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
					{
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{	//@@@ 2002.04.14 MIK
							//@@@ 2002.09.23 YAZAKI 最適化
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->m_pNext;
								pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
								(*pnExtInsLineNum)++;								//	再描画してほしい行数+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							nPosX = 0;
							if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
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
				pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
			}
			m_nLineTypeBot = nCOMMENTMODE;
			nCOMMENTMODE_Prev = nCOMMENTMODE;

			if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
				( nDelLogicalLineFrom < nCurLine )
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

// 1999.12.22 レイアウト情報がなくなる訳ではないので
//	m_nPrevReferLine = 0;
//	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	return nModifyLayoutLinesNew;
}

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

bool CLayoutMgr::IsKutoTen( unsigned char c1, unsigned char c2 )
{
	static const char	*KUTOTEN_1 = "｡､,.";
	static const char	*KUTOTEN_2 = "。、，．";
	unsigned const char	*p;

	if( c2 )	//全角
	{
		for( p = (const unsigned char *)KUTOTEN_2; *p; p += 2 )
		{
			if( *p == c1 && *(p + 1) == c2 ) return true;
		}
	}
	else		//半角
	{
		for( p = (const unsigned char *)KUTOTEN_1; *p; p++ )
		{
			if( *p == c1 ) return true;
		}
	}

	return false;
}

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

int CLayoutMgr::Match_Quote( char szQuote, int nPos, int nLineLen, const char* pLine )
{
	int nCharChars;
	int i;
	for( i = nPos; i < nLineLen; ++i ){
		nCharChars = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && pLine[i] == '\\' ){
				++i;
			}else
			if( 1 == nCharChars && pLine[i] == szQuote ){
				return i + 1;
			}
		}else
		if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
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

bool CLayoutMgr::CheckColorMODE( int &nCOMMENTMODE, int &nCOMMENTEND, int nPos, int nLineLen, const char* pLine, BOOL bDispSSTRING, BOOL bDispWSTRING )
{
	switch( nCOMMENTMODE ){
	case COLORIDX_TEXT: // 2002/03/13 novice
		if( m_cLineComment.Match( nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice
		}else
		if( m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_cBlockComment.Match_CommentTo( 0, nPos + (int)lstrlen( m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		}else
		if( m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
			/* この物理行にブロックコメントの終端があるか */
			nCOMMENTEND = m_cBlockComment.Match_CommentTo( 1, nPos + (int)lstrlen( m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );
//#endif
		}else
		if( bDispSSTRING && /* シングルクォーテーション文字列を表示する */
			pLine[nPos] == '\''
		){
			nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
			/* シングルクォーテーション文字列の終端があるか */
			nCOMMENTEND = Match_Quote( '\'', nPos + 1, nLineLen, pLine );
		}else
		if( pLine[nPos] == '"' &&
			bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
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
	ただし，残り幅が6文字未満の場合はインデントを行わない．
	
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
	if( pLayoutPrev->m_nOffset > 0 )
		return nIpos;
	
	CMemoryIterator<CLayout> it( pLayoutPrev, m_nTabSpace );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && it.getCurrentChar() == TAB ){
			nIpos = it.getColumn() + it.getColumnDelta();
		}
		it.addDelta();
	}
	if ( m_nMaxLineSize - nIpos < 6 ){
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
int CLayoutMgr::getIndentOffset_LeftSpace( CLayout* pLayoutPrev )
{
	//	前の行が無いときは、インデント不要。
	if ( pLayoutPrev == NULL ) return 0;

	//	インデントの計算
	int nIpos = pLayoutPrev->GetIndent();
	
	//	Oct. 5, 2002 genta
	//	折り返しの3行目以降は1つ前の行のインデントに合わせる．
	if( pLayoutPrev->m_nOffset > 0 )
		return nIpos;
	
	//	2002.10.07 YAZAKI インデントの計算
	CMemoryIterator<CLayout> it( pLayoutPrev, m_nTabSpace );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && (it.getCurrentChar() == TAB || it.getCurrentChar() == ' ') ){
			//	インデントのカウントを継続する
		}
		else {
			nIpos = it.getColumn();	//	終了
			break;
		}
		it.addDelta();
	}
	if ( m_nMaxLineSize - nIpos < 6 ){
		nIpos = pLayoutPrev->GetIndent();	//	あきらめる
	}
	return nIpos;	//	インデント
}

/*[EOF]*/
