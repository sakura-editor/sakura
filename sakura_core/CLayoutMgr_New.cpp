//	$Id$
/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK

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

//	/*
//	|| 	新しい折り返し文字数に合わせて全データのレイアウト情報を再生成します
//	||
//	*/
//	void CLayoutMgr::DoLayout(
//			int		nMaxLineSize,
//			BOOL	bWordWrap,	/* 英文ワードラップをする */
//			HWND	hwndProgress,
//			BOOL	bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
//			BOOL	bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
//	)
//	{
//		m_nMaxLineSize = nMaxLineSize;
//		m_bWordWrap = bWordWrap;		/* 英文ワードラップをする */
//		DoLayout( hwndProgress, bDispSSTRING, bDispWSTRING );
//		return;
//	}


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
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::DoLayout" );
#endif
	int			nLineNum;
	int			nLineLen;
	CDocLine*	pCDocLine;
	const char* pLine;
	int			nBgn;
	int			nPos;
	int			nPosX;
	int			nCharChars;
	int			nCharChars2;
	int			nCharChars_2;
	int			nCOMMENTMODE;
	int			nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	int			nWordBgn;
	int			nWordLen;
	int			nAllLineNum;


	nCOMMENTMODE = 0;
	nCOMMENTMODE_Prev = 0;

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

//	pLine = m_pcDocLineMgr->GetFirstLinrStr( &nLineLen );
	pCDocLine = m_pcDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr変更

	if( nCOMMENTMODE_Prev == 1 ){	/* 行コメントである */
		nCOMMENTMODE_Prev = 0;
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

		int nEol_1 = pCDocLine->m_cEol.GetLen() - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		while( nPos < nLineLen - nEol_1 ){
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}

			SEARCH_START:;
			/* ワードラップ処理 */
			if( m_bWordWrap ){	/* 英文ワードラップをする */
				if( 0 == nWordLen ){
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
						if( nPosX + i - nPos >= m_nMaxLineSize
						 && nPos - nBgn > 0
						){
							AddLineBottom( pCDocLine, /*pLine,*/ nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							nPosX = 0;
//?							continue;
						}
					}
				}else{
					if( nPos == nWordBgn + nWordLen ){
						nWordLen = 0;
					}
				}
			}
			switch( nCOMMENTMODE ){
			case 0:
				if( ( NULL != m_pszLineComment &&	/* 行コメントデリミタ */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment ) &&	/* 行コメントデリミタ */
					  0 == memicmp( &pLine[nPos], m_pszLineComment, (int)strlen( m_pszLineComment ) )
					) ||
					( NULL != m_pszLineComment2 &&	/* 行コメントデリミタ2 */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment2 ) &&	/* 行コメントデリミタ2 */
					  0 == memicmp( &pLine[nPos], m_pszLineComment2, (int)strlen( m_pszLineComment2 ) )
					) ||	//Jun. 01, 2001 JEPRO 3つ目を追加
					( NULL != m_pszLineComment3 &&	/* 行コメントデリミタ3 */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment3 ) &&	/* 行コメントデリミタ3 */
					  0 == memicmp( &pLine[nPos], m_pszLineComment3, (int)strlen( m_pszLineComment3 ) )
					)
				){
					nCOMMENTMODE = 1;	/* 行コメントである */
				}else
				if( NULL != m_pszBlockCommentFrom &&	/* ブロックコメントデリミタ(From) */
					NULL != m_pszBlockCommentTo &&		/* ブロックコメントデリミタ(To) */
					nPos <= nLineLen - (int)strlen( m_pszBlockCommentFrom ) &&	/* ブロックコメントデリミタ(From) */
					0 == memicmp( &pLine[nPos], m_pszBlockCommentFrom, (int)strlen( m_pszBlockCommentFrom ) )
				){
					nCOMMENTMODE = 2;	/* ブロックコメントである */
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + (int)strlen( m_pszBlockCommentFrom ); i <= nLineLen - (int)strlen( m_pszBlockCommentTo ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo, (int)strlen( m_pszBlockCommentTo )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
				}else
				if( NULL != m_pszBlockCommentFrom2 &&	/* ブロックコメントデリミタ2(From) */
					NULL != m_pszBlockCommentTo2 &&		/* ブロックコメントデリミタ2(To) */
					nPos <= nLineLen - (int)strlen( m_pszBlockCommentFrom2 ) &&	/* ブロックコメントデリミタ2(From) */
					0 == memicmp( &pLine[nPos], m_pszBlockCommentFrom2, (int)strlen( m_pszBlockCommentFrom2 ) )
				){
					nCOMMENTMODE = 20;	/* ブロックコメントである */
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + (int)strlen( m_pszBlockCommentFrom2 ); i <= nLineLen - (int)strlen( m_pszBlockCommentTo2 ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo2, (int)strlen( m_pszBlockCommentTo2 )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo2 );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
//#endif
				}else
				if( pLine[nPos] == '\'' &&
					bDispSSTRING  /* シングルクォーテーション文字列を表示する */
				){
					nCOMMENTMODE = 3;	/* シングルクォーテーション文字列である */
					/* シングルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + 1; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( pLine[nPos] == '"' &&
					bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
				){
					nCOMMENTMODE = 4;	/* ダブルクォーテーション文字列である */
					/* ダブルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + 1; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}
				break;
			case 1:	/* 行コメントである */
				break;
			case 2:	/* ブロックコメントである */
				if( 0 == nCOMMENTEND ){
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + (int)strlen( m_pszBlockCommentFrom )*/; i <= nLineLen - (int)strlen( m_pszBlockCommentTo ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo, (int)strlen( m_pszBlockCommentTo )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = 0;
					goto SEARCH_START;
				}
				break;
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
			case 20:	/* ブロックコメントである */
				if( 0 == nCOMMENTEND ){
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + (int)strlen( m_pszBlockCommentFrom2 )*/; i <= nLineLen - (int)strlen( m_pszBlockCommentTo2 ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo2, (int)strlen( m_pszBlockCommentTo2 )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo2 );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = 0;
					goto SEARCH_START;
				}
				break;
//#endif
			case 3:	/* シングルクォーテーション文字列である */
				if( 0 == nCOMMENTEND ){
					/* シングルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = 0;
					goto SEARCH_START;
				}
				break;
			case 4:	/* ダブルクォーテーション文字列である */
				if( 0 == nCOMMENTEND ){
					/* ダブルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = 0;
					goto SEARCH_START;
				}
				break;
			}
			if( pLine[nPos] == TAB ){
				nCharChars = m_nTabSpace - ( nPosX % m_nTabSpace );
				if( nPosX + nCharChars > m_nMaxLineSize ){
					AddLineBottom( pCDocLine, /*pLine,*/nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
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
				}
				if( nPosX + nCharChars > m_nMaxLineSize ){
					AddLineBottom( pCDocLine, /*pLine,*/ nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
					nCOMMENTMODE_Prev = nCOMMENTMODE;
					nBgn = nPos;
					nPosX = 0;
					continue;
				}
				nPos+= nCharChars;
				nPosX += nCharChars;
			}
		}
		if( nPos - nBgn > 0 ){
			if( nCOMMENTMODE == 1 ){	/* 行コメントである */
				nCOMMENTMODE = 0;
			}
			AddLineBottom( pCDocLine, /*pLine,*/ nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
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
		if( nCOMMENTMODE_Prev == 1 ){	/* 行コメントである */
			nCOMMENTMODE_Prev = 0;
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


//	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
//	int CLayoutMgr::DoLayout3(
//				CLayout* pLayoutPrev,
//				int		nLineNum,
//				int		nDelLogicalLineFrom,
//				int		nDelLogicalColFrom
//	 )
//	{
//		int			nLineNumWork;
//		int			nLineLen;
//		int			nCurLine;
//		const char* pLine;
//		int			nBgn;
//		int			nPos;
//		int			nPosX;
//		int			nCharChars;
//		CLayout*	pLayout;
//		int			nModifyLayoutLinesNew;
//		int			nCOMMENTMODE;
//		int			nCOMMENTMODE_Prev;
//	//	int			nCOMMENTEND;
//		nLineNumWork = 0;
//
//		if( 0 == nLineNum ){
//			return 0;
//		}
//		pLayout = pLayoutPrev;
//		if( NULL == pLayout ){
//			nCurLine = 0;
//		}else{
//			nCurLine = pLayout->m_nLinePhysical + 1;
//		}
//		nCOMMENTMODE = 0;
//		nCOMMENTMODE_Prev = 0;
//
//		pLine = m_pcDocLineMgr->GetLineStr( nCurLine, &nLineLen );
//		nModifyLayoutLinesNew = 0;
//
//		while( NULL != pLine ){
//			nPosX = 0;
//			nCharChars = 0;
//			nBgn = 0;
//			nPos = 0;
//			while( nPos < nLineLen ){
//				if( pLine[nPos] == TAB ){
//					nCharChars = m_nTabSpace - ( nPosX % m_nTabSpace );
//					if( nPosX + nCharChars > m_nMaxLineSize ){
//						pLayout = InsertLineNext( pLayout, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
//						nCOMMENTMODE_Prev = nCOMMENTMODE;
//
//						nBgn = nPos;
//						nPosX = 0;
//						if( ( nDelLogicalLineFrom == nCurLine &&
//							  nDelLogicalColFrom < nPos ) ||
//							( nDelLogicalLineFrom < nCurLine )
//						){
//							(nModifyLayoutLinesNew)++;;
//						}
//						continue;
//					}
//					nPos++;
//				}else{
//					nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
//					if( 0 == nCharChars ){
//						nCharChars = 1;
//						break;
//					}
//					if( nPosX + nCharChars > m_nMaxLineSize ){
//						pLayout = InsertLineNext( pLayout, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
//						nCOMMENTMODE_Prev = nCOMMENTMODE;
//
//						nBgn = nPos;
//						nPosX = 0;
//						if( ( nDelLogicalLineFrom == nCurLine &&
//							  nDelLogicalColFrom < nPos ) ||
//							( nDelLogicalLineFrom < nCurLine )
//						){
//							(nModifyLayoutLinesNew)++;;
//						}
//						continue;
//					}
//					nPos+= nCharChars;
//				}
//
//				nPosX += nCharChars;
//			}
//			if( nPos - nBgn > 0 ){
//				if( nCOMMENTMODE == 1 ){	/* 行コメントである */
//					nCOMMENTMODE = 0;
//				}
//				pLayout = InsertLineNext( pLayout, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
//				nCOMMENTMODE_Prev = nCOMMENTMODE;
//
//				if( ( nDelLogicalLineFrom == nCurLine &&
//					  nDelLogicalColFrom < nPos ) ||
//					( nDelLogicalLineFrom < nCurLine )
//				){
//					(nModifyLayoutLinesNew)++;;
//				}
//			}
//
//			nLineNumWork++;
//			nCurLine++;
//			if( nLineNumWork >= nLineNum ){
//				break;
//			}
//			pLine = m_pcDocLineMgr->GetNextLinrStr( &nLineLen );
//		}
//		m_nPrevReferLine = 0;
//		m_pLayoutPrevRefer = NULL;
//		m_pLayoutCurrent = NULL;
//		return nModifyLayoutLinesNew;
//	}



/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
int CLayoutMgr::DoLayout3_New(
			CLayout* pLayoutPrev,
//			CLayout* pLayoutNext,
			int		nLineNum,
			int		nDelLogicalLineFrom,
			int		nDelLogicalColFrom,
			int		nCurrentLineType,
			int*	pnExtInsLineNum,
			BOOL	bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
			BOOL	bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::DoLayout3_New" );
//#endif
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
	int			nCharChars_2;
	CLayout*	pLayout;
	int			nModifyLayoutLinesNew;
	int			nCOMMENTMODE;
	int			nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	CLayout*	pLayoutNext;
	int			bAdd = FALSE;
	int			nWordBgn;
	int			nWordLen;
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

		int nEol_1 = pCDocLine->m_cEol.GetLen() - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		while( nPos < nLineLen - nEol_1 ){
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			SEARCH_START:;
			/* ワードラップ処理 */
			if( m_bWordWrap ){	/* 英文ワードラップをする */
				if( 0 == nWordLen ){
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
							if( nCharChars2 == 1 && ( pLine[i] == '#' || __iscsym( pLine[i] ) ) ){
							}else{
								break;
							}
							i += nCharChars2;
						}
						nWordBgn = nPos;
						nWordLen = i - nPos;

						if( nPosX + i - nPos >= m_nMaxLineSize
						 && nPos - nBgn > 0
						){
							pLayout = InsertLineNext( pLayout, pCDocLine, /*pLine,*/ nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							if( bAdd ){
								CLayout*	pLayoutWork;
								pLayoutWork = pLayoutNext;
								pLayoutNext = pLayoutNext->m_pNext;
								pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
								if( NULL != pLayoutNext ){
									pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
								}else{
									m_pLayoutBot = pLayoutWork->m_pPrev;
								}

#ifdef _DEBUG
								if( m_pLayoutPrevRefer == pLayoutWork ){
									MYTRACE( "バグバグ\n" );
								}
#endif
								delete pLayoutWork;
								m_nLines--;

								(*pnExtInsLineNum)++;
							}

							nBgn = nPos;
							nPosX = 0;
							if( ( nDelLogicalLineFrom == nCurLine &&
								  nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
//?							continue;
						}
					}
				}else{
					if( nPos == nWordBgn + nWordLen ){
						nWordLen = 0;
					}
				}
			}
			switch( nCOMMENTMODE ){
			case 0:
				if( ( NULL != m_pszLineComment &&	/* 行コメントデリミタ */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment ) &&	/* 行コメントデリミタ */
					  0 == memicmp( &pLine[nPos], m_pszLineComment, (int)strlen( m_pszLineComment ) )
					) ||
					( NULL != m_pszLineComment2 &&	/* 行コメントデリミタ2 */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment2 ) &&	/* 行コメントデリミタ2 */
					  0 == memicmp( &pLine[nPos], m_pszLineComment2, (int)strlen( m_pszLineComment2 ) )
					) ||	//Jun. 01, 2001 JEPRO 3つ目を追加
					( NULL != m_pszLineComment3 &&	/* 行コメントデリミタ3 */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment3 ) &&	/* 行コメントデリミタ3 */
					  0 == memicmp( &pLine[nPos], m_pszLineComment3, (int)strlen( m_pszLineComment3 ) )
					)
				){
					nCOMMENTMODE = 1;	/* 行コメントである */
				}else
				if( NULL != m_pszBlockCommentFrom &&	/* ブロックコメントデリミタ(From) */
					NULL != m_pszBlockCommentTo &&		/* ブロックコメントデリミタ(To) */
					nPos <= nLineLen - (int)strlen( m_pszBlockCommentFrom ) &&	/* ブロックコメントデリミタ(From) */
					0 == memicmp( &pLine[nPos], m_pszBlockCommentFrom, (int)strlen( m_pszBlockCommentFrom ) )
				){
					nCOMMENTMODE = 2;	/* ブロックコメントである */
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + (int)strlen( m_pszBlockCommentFrom ); i <= nLineLen - (int)strlen( m_pszBlockCommentTo ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo, (int)strlen( m_pszBlockCommentTo )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
//#ifdef	COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
				}else
				if( NULL != m_pszBlockCommentFrom2 &&	/* ブロックコメントデリミタ2(From) */
					NULL != m_pszBlockCommentTo2 &&		/* ブロックコメントデリミタ2(To) */
					nPos <= nLineLen - (int)strlen( m_pszBlockCommentFrom2 ) &&	/* ブロックコメントデリミタ2(From) */
					0 == memicmp( &pLine[nPos], m_pszBlockCommentFrom2, (int)strlen( m_pszBlockCommentFrom2 ) )
				){
					nCOMMENTMODE = 20;	/* ブロックコメントである */
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + (int)strlen( m_pszBlockCommentFrom2 ); i <= nLineLen - (int)strlen( m_pszBlockCommentTo2 ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo2, (int)strlen( m_pszBlockCommentTo2 )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo2 );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
//#endif
				}else
				if( pLine[nPos] == '\'' &&
					bDispSSTRING  /* シングルクォーテーション文字列を表示する */
				){
					nCOMMENTMODE = 3;	/* シングルクォーテーション文字列である */
					/* シングルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + 1; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( pLine[nPos] == '"' &&
					bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
				){
					nCOMMENTMODE = 4;	/* ダブルクォーテーション文字列である */
					/* ダブルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + 1; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}
				break;
			case 1:	/* 行コメントである */
				break;
			case 2:	/* ブロックコメントである */
				if( 0 == nCOMMENTEND ){
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + (int)strlen( m_pszBlockCommentFrom )*/; i <= nLineLen - (int)strlen( m_pszBlockCommentTo ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo, (int)strlen( m_pszBlockCommentTo )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = 0;
					goto SEARCH_START;
				}
				break;
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
			case 20:	/* ブロックコメントである */
				if( 0 == nCOMMENTEND ){
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + (int)strlen( m_pszBlockCommentFrom2 )*/; i <= nLineLen - (int)strlen( m_pszBlockCommentTo2 ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo2, (int)strlen( m_pszBlockCommentTo2 )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo2 );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = 0;
					goto SEARCH_START;
				}
				break;
//#endif
			case 3:	/* シングルクォーテーション文字列である */
				if( 0 == nCOMMENTEND ){
					/* シングルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = 0;
					goto SEARCH_START;
				}
				break;
			case 4:	/* ダブルクォーテーション文字列である */
				if( 0 == nCOMMENTEND ){
					/* ダブルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = 0;
					goto SEARCH_START;
				}
				break;
			}


			if( pLine[nPos] == TAB ){
				nCharChars = m_nTabSpace - ( nPosX % m_nTabSpace );
				if( nPosX + nCharChars > m_nMaxLineSize ){
					pLayout = InsertLineNext( pLayout, pCDocLine, /*pLine,*/ nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
					nCOMMENTMODE_Prev = nCOMMENTMODE;
					if( bAdd ){
						CLayout*	pLayoutWork;
						pLayoutWork = pLayoutNext;
						pLayoutNext = pLayoutNext->m_pNext;
						pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
						if( NULL != pLayoutNext ){
							pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
						}else{
							m_pLayoutBot = pLayoutWork->m_pPrev;
						}
#ifdef _DEBUG
						if( m_pLayoutPrevRefer == pLayoutWork ){
							MYTRACE( "バグバグ\n" );
						}
#endif
						delete pLayoutWork;
						m_nLines--;

						(*pnExtInsLineNum)++;
					}
					nBgn = nPos;
					nPosX = 0;
					if( ( nDelLogicalLineFrom == nCurLine &&
						  nDelLogicalColFrom < nPos ) ||
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
//					break;
				}
				if( nPosX + nCharChars > m_nMaxLineSize ){
					pLayout = InsertLineNext( pLayout, pCDocLine, /*pLine,*/ nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
					nCOMMENTMODE_Prev = nCOMMENTMODE;
					if( bAdd ){
						CLayout*	pLayoutWork;
						pLayoutWork = pLayoutNext;
						pLayoutNext = pLayoutNext->m_pNext;
						pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
						if( NULL != pLayoutNext ){
							pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
						}else{
							m_pLayoutBot = pLayoutWork->m_pPrev;
						}
#ifdef _DEBUG
						if( m_pLayoutPrevRefer == pLayoutWork ){
							MYTRACE( "バグバグ\n" );
						}
#endif
						delete pLayoutWork;
						m_nLines--;

						(*pnExtInsLineNum)++;
					}

					nBgn = nPos;
					nPosX = 0;
					if( ( nDelLogicalLineFrom == nCurLine &&
						  nDelLogicalColFrom < nPos ) ||
						( nDelLogicalLineFrom < nCurLine )
					){
						(nModifyLayoutLinesNew)++;;
					}
					continue;
				}
				nPos+= nCharChars;
			}

			nPosX += nCharChars;
		}
		if( nPos - nBgn > 0 ){
			if( nCOMMENTMODE == 1 ){	/* 行コメントである */
				nCOMMENTMODE = 0;
			}
			pLayout = InsertLineNext( pLayout, pCDocLine, /*pLine,*/ nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
			nCOMMENTMODE_Prev = nCOMMENTMODE;
			if( bAdd ){
				CLayout*	pLayoutWork;
				pLayoutWork = pLayoutNext;
				pLayoutNext = pLayoutNext->m_pNext;
				pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
				if( NULL != pLayoutNext ){
					pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
				}else{
					m_pLayoutBot = pLayoutWork->m_pPrev;
				}
#ifdef _DEBUG
				if( m_pLayoutPrevRefer == pLayoutWork ){
					MYTRACE( "バグバグ\n" );
				}
#endif
				delete pLayoutWork;
				m_nLines--;

				(*pnExtInsLineNum)++;
			}

			if( ( nDelLogicalLineFrom == nCurLine &&
				  nDelLogicalColFrom < nPos ) ||
				( nDelLogicalLineFrom < nCurLine )
			){
				(nModifyLayoutLinesNew)++;;
			}
		}

		nLineNumWork++;
		nCurLine++;
		if( nLineNumWork >= nLineNum ){
//			pLayoutNext = pLayout->m_pNext;
			if( NULL != pLayout
			 && NULL != ( pLayoutNext = pLayout->m_pNext )
			){
				if( nCOMMENTMODE_Prev == pLayoutNext->m_nTypePrev ){
					break;
				}else{
//					CLayout*	pLayoutWork;
//					pLayoutWork = pLayoutNext;
//					pLayoutNext = pLayoutNext->m_pNext;
//					pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
//					if( NULL != pLayoutNext ){
//						pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
//					}else{
//						m_pLayoutBot = pLayoutWork->m_pPrev;
//					}
//					delete pLayoutWork;
//					m_nLines--;

					bAdd = TRUE;



//					int i;
//					i = 1;
//					break;
				}
			}else{
				break;
			}
		}
//		pLine = m_pcDocLineMgr->GetNextLinrStr( &nLineLen );
		pCDocLine = pCDocLine->m_pNext;
		if( nCOMMENTMODE_Prev == 1 ){	/* 行コメントである */
			nCOMMENTMODE_Prev = 0;
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


/*[EOF]*/
