//	$Id$
/************************************************************************

	CLayoutMgr.cpp

    テキストのレイアウト情報管理
	Copyright (C) 1998-2000, Norio Nakatani


************************************************************************/
#include "CLayoutMgr.h"

/*** for TRACE()****
#include <afx.h>
***/
#include "charcode.h"
#include "debug.h"
#include <commctrl.h>
#include "CRunningTimer.h"






/*
|| コンストラクタ
*/
CLayoutMgr::CLayoutMgr()
{
	m_pcDocLineMgr = NULL;
	m_nMaxLineSize = 0;
	m_bWordWrap = TRUE;		/* 英文ワードラップをする */
	m_nTabSpace = 8;		/* TAB文字スペース */
	m_nStringType = 0;		/* 文字列区切り記号エスケープ方法　0=[\"][\'] 1=[""][''] */
	m_pszLineComment = NULL;			/* 行コメントデリミタ */
	m_pszLineComment2 = NULL;			/* 行コメントデリミタ2 */
	m_pszBlockCommentFrom = NULL;		/* ブロックコメントデリミタ(From) */
	m_pszBlockCommentTo = NULL;			/* ブロックコメントデリミタ(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	m_pszBlockCommentFrom2 = NULL;		/* ブロックコメントデリミタ(From) */
	m_pszBlockCommentTo2 = NULL;		/* ブロックコメントデリミタ(To) */
//#endif
	Init();
	return;
}


/*
|| デストラクタ
*/
CLayoutMgr::~CLayoutMgr()
{
	Empty();

	if( NULL != m_pszLineComment ){	/* 行コメントデリミタ */
		delete [] m_pszLineComment;
		m_pszLineComment = NULL;
	}
	if( NULL != m_pszLineComment2 ){	/* 行コメントデリミタ2 */
		delete [] m_pszLineComment2;
		m_pszLineComment2 = NULL;
	}
	if( NULL != m_pszBlockCommentFrom ){	/* ブロックコメントデリミタ(From) */
		delete [] m_pszBlockCommentFrom;
		m_pszBlockCommentFrom = NULL;
	}
	if( NULL != m_pszBlockCommentTo ){	/* ブロックコメントデリミタ(To) */
		delete [] m_pszBlockCommentTo;
		m_pszBlockCommentTo = NULL;
	}
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	if( NULL != m_pszBlockCommentFrom2 ){	/* ブロックコメントデリミタ(From) */
		delete [] m_pszBlockCommentFrom2;
		m_pszBlockCommentFrom2 = NULL;
	}
	if( NULL != m_pszBlockCommentTo2 ){	/* ブロックコメントデリミタ(To) */
		delete [] m_pszBlockCommentTo2;
		m_pszBlockCommentTo2 = NULL;
	}
//#endif
	return;
}


/*
|| レイアウト情報の変更
*/
void CLayoutMgr::SetLayoutInfo( 
	int		nMaxLineSize,			/* 折り返し文字数 */
	BOOL	bWordWrap,				/* 英文ワードラップをする */
	int		nTabSpace,				/* TAB文字スペース */
	char*	pszLineComment,			/* 行コメントデリミタ */
	char*	pszLineComment2,		/* 行コメントデリミタ2 */
	char*	pszBlockCommentFrom,	/* ブロックコメントデリミタ(From) */
	char*	pszBlockCommentTo,		/* ブロックコメントデリミタ(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	char*	pszBlockCommentFrom2,	/* ブロックコメントデリミタ(From) */
	char*	pszBlockCommentTo2,		/* ブロックコメントデリミタ(To) */
//#endif
	int		nStringType,			/* 文字列区切り記号エスケープ方法　0=[\"][\'] 1=[""][''] */
	int		bDoRayout, 
	HWND	hwndProgress,
	BOOL	bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
	BOOL	bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
)
{
	int		nStrLen;
	m_nMaxLineSize = nMaxLineSize;
	m_bWordWrap = bWordWrap;		/* 英文ワードラップをする */
	m_nTabSpace    = nTabSpace;
	m_nStringType = nStringType;		/* 文字列区切り記号エスケープ方法　0=[\"][\'] 1=[""][''] */

	if( NULL != m_pszLineComment ){	/* 行コメントデリミタ */
		delete [] m_pszLineComment;
		m_pszLineComment = NULL;
	}
	nStrLen = strlen( pszLineComment );
	if( 0 < nStrLen ){
		m_pszLineComment = new char[nStrLen + 1];
		strcpy( m_pszLineComment, pszLineComment );
	}

	if( NULL != m_pszLineComment2 ){	/* 行コメントデリミタ2 */
		delete [] m_pszLineComment2;
		m_pszLineComment2 = NULL;
	}
	nStrLen = strlen( pszLineComment2 );
	if( 0 < nStrLen ){
		m_pszLineComment2 = new char[nStrLen + 1];
		strcpy( m_pszLineComment2, pszLineComment2 );
	}
	

	if( NULL != m_pszBlockCommentFrom ){	/* ブロックコメントデリミタ(From) */
		delete [] m_pszBlockCommentFrom;
		m_pszBlockCommentFrom = NULL;
	}
	nStrLen = strlen( pszBlockCommentFrom );
	if( 0 < nStrLen ){
		m_pszBlockCommentFrom = new char[nStrLen + 1];
		strcpy( m_pszBlockCommentFrom, pszBlockCommentFrom );
	}

	if( NULL != m_pszBlockCommentTo ){	/* ブロックコメントデリミタ(To) */
		delete [] m_pszBlockCommentTo;
		m_pszBlockCommentTo = NULL;
	}
	nStrLen = strlen( pszBlockCommentTo );
	if( 0 < nStrLen ){
		m_pszBlockCommentTo = new char[nStrLen + 1];
		strcpy( m_pszBlockCommentTo, pszBlockCommentTo );
	}

//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	if( NULL != m_pszBlockCommentFrom2 ){	/* ブロックコメントデリミタ(From) */
		delete [] m_pszBlockCommentFrom2;
		m_pszBlockCommentFrom2 = NULL;
	}
	nStrLen = strlen( pszBlockCommentFrom2 );
	if( 0 < nStrLen ){
		m_pszBlockCommentFrom2 = new char[nStrLen + 1];
		strcpy( m_pszBlockCommentFrom2, pszBlockCommentFrom2 );
	}

	if( NULL != m_pszBlockCommentTo2 ){	/* ブロックコメントデリミタ(To) */
		delete [] m_pszBlockCommentTo2;
		m_pszBlockCommentTo2 = NULL;
	}
	nStrLen = strlen( pszBlockCommentTo2 );
	if( 0 < nStrLen ){
		m_pszBlockCommentTo2 = new char[nStrLen + 1];
		strcpy( m_pszBlockCommentTo2, pszBlockCommentTo2 );
	}
//#endif

	if( bDoRayout ){
		DoLayout( hwndProgress, bDispSSTRING, bDispWSTRING );
	}
	return;
}


/*
||
|| 行データ管理クラスのポインタを初期化します
||
*/
void CLayoutMgr::Create( CDocLineMgr* pcDocLineMgr )
{
	Init();
	m_pcDocLineMgr = pcDocLineMgr;
	return;
}



void CLayoutMgr::Init()
{
	m_pLayoutTop = NULL;
	m_pLayoutBot = NULL;
	m_nPrevReferLine = 0;
	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;
	m_nLines = 0;			/* 全物理行数 */
//	m_pszLineComment = NULL;			/* 行コメントデリミタ */
//	m_pszBlockCommentFrom = NULL;		/* ブロックコメントデリミタ(From) */
//	m_pszBlockCommentTo = NULL;			/* ブロックコメントデリミタ(To) */
	return;
}



void CLayoutMgr::Empty()
{
	CLayout* pLayout;
	CLayout* pLayoutNext;
	pLayout = m_pLayoutTop;
	while( NULL != pLayout ){
		pLayoutNext = pLayout->m_pNext;
		delete pLayout;
		pLayout = pLayoutNext;
	}
	return;
}


/*
|| 指定された物理行のレイアウト情報を取得
|| 物理行は０オリジン
*/
CLayout* CLayoutMgr::Search( int nLineNum )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::Search()" );
//#endif
	CLayout*	pLayout;
	int			nCount;
	if( 0 == m_nLines ){
		return NULL;
	}
	if( nLineNum >= m_nLines ){
		return NULL;
	}

//	/*+++++++ 低速版 +++++++++*/
//	if( nLineNum < (m_nLines / 2) ){
//		nCount = 0;
//		pLayout = m_pLayoutTop;
//		while( NULL != pLayout ){
//			if( nLineNum == nCount ){
//				m_pLayoutPrevRefer = pLayout;
//				m_nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->m_pNext;
//			nCount++;
//		}
//	}else{
//		nCount = m_nLines - 1;
//		pLayout = m_pLayoutBot;
//		while( NULL != pLayout ){
//			if( nLineNum == nCount ){
//				m_pLayoutPrevRefer = pLayout;
//				m_nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->m_pPrev;
//			nCount--;
//		}
//	}

	/*+++++++わずかに高速版+++++++*/
	if( m_pLayoutPrevRefer == NULL ){
		if( nLineNum < (m_nLines / 2) ){
			nCount = 0;
			pLayout = m_pLayoutTop;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pNext;
				nCount++;
			}
		}else{
			nCount = m_nLines - 1;
			pLayout = m_pLayoutBot;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pPrev;
				nCount--;
			}
		}
	}else{
		if( nLineNum == m_nPrevReferLine ){
			return m_pLayoutPrevRefer;
		}else
		if( nLineNum > m_nPrevReferLine ){
			nCount = m_nPrevReferLine + 1;
			pLayout = m_pLayoutPrevRefer->m_pNext;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pNext;
				nCount++;
			}
		}else{
			nCount = m_nPrevReferLine - 1;
			pLayout = m_pLayoutPrevRefer->m_pPrev;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pPrev;
				nCount--;
			}
		}
	}
	return NULL;
}



void CLayoutMgr::AddLineBottom( CDocLine* pCDocLine, /*const char* pLine,*/ int nLine, int nOffset, int nLength, int nTypePrev, int nTypeNext )
{
#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::AddLineBottom" );
#endif
//	if( pLine != pCDocLine->m_pLine->GetPtr2() ){
//	if( pLine != pCDocLine->m_pLine->m_pData ){
//		int		i;
//		i = 1234;
//	
//	}
#ifdef _DEBUG
	{
		if( NULL != pCDocLine ){
			int nLineLen = pCDocLine->m_pLine->m_nDataLen;
			const char * pLine = (const char *)pCDocLine->m_pLine->m_pData;
		}
	}
#endif	
	
	CLayout* pLayout;
	if(	0 == m_nLines ){
		m_pLayoutBot = m_pLayoutTop = new CLayout;
		m_pLayoutTop->m_pPrev = NULL;
		pLayout = m_pLayoutTop;
	}else{
		pLayout = new CLayout;
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		m_pLayoutBot = pLayout;
	}
	pLayout->m_pNext = NULL;
//	pLayout->m_pLine = pLine;
//	pLayout->m_pLine = pCDocLine->m_pLine->m_pData;
	pLayout->m_pCDocLine = pCDocLine;

	pLayout->m_nLinePhysical = nLine;
	pLayout->m_nOffset = nOffset;
	pLayout->m_nLength = nLength;
	pLayout->m_nTypePrev = nTypePrev;

	if( EOL_NONE == pCDocLine->m_cEol ){
		pLayout->m_cEol.SetType( EOL_NONE );/* 改行コードの種類 */
	}else{
//		if( pLayout->m_nOffset + pLayout->m_nLength >= 
		if( pLayout->m_nOffset + pLayout->m_nLength > 
			pCDocLine->m_pLine->m_nDataLen - pCDocLine->m_cEol.GetLen()
		){
			pLayout->m_cEol = pCDocLine->m_cEol;/* 改行コードの種類 */
		}else{
			pLayout->m_cEol.SetType( EOL_NONE );/* 改行コードの種類 */
		}
	}
	// pLayout->m_nEOLLen = gm_pnEolLenArr[pLayout->m_enumEOLType];/* 改行コードの長さ */ 		


	pLayout->m_nTypeNext = nTypeNext;
//#ifdef _DEBUG
//	if( 1 == pLayout->m_nTypeNext ){
//		MYTRACE( "★★★CLayoutMgr::AddLineBottom::InsertLineNext()   1 == pLayout->m_nTypeNext\n" );
//	}
//#endif	
/***
	MYTRACE( "m_nLines=%d, pLayout->nLine=%d, pLayout->nOffset=%d, pLayout->nLength=%d \n", m_nLines, pLayout->nLine, pLayout->nOffset, pLayout->nLength );
***/
	m_nLines++;
	return;
}


CLayout* CLayoutMgr::InsertLineNext( CLayout* pLayoutPrev, CDocLine* pCDocLine, /*const char* pLine,*/ int nLine, int nOffset, int nLength, int nTypePrev, int nTypeNext )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::InsertLineNext" );
//#endif
	CLayout* pLayoutNext;
	CLayout* pLayout;
	pLayout = new CLayout;
#ifdef _DEBUG
	{
		if( NULL != pCDocLine ){
			int nLineLen = pCDocLine->m_pLine->m_nDataLen;
			const char * pLine = (const char *)pCDocLine->m_pLine->m_pData;
		}
	}
#endif


	if(	0 == m_nLines ){
		m_pLayoutBot = m_pLayoutTop = pLayout;
		m_pLayoutTop->m_pPrev = NULL;
		m_pLayoutTop->m_pNext = NULL;
	}else
	if( NULL == pLayoutPrev ){
		/* 先頭に挿入 */
		m_pLayoutTop->m_pPrev = pLayout;
		pLayout->m_pPrev = NULL;
		pLayout->m_pNext = m_pLayoutTop;
		m_pLayoutTop = pLayout;
	}else
	if( NULL == pLayoutPrev->m_pNext ){
		/* 最後に挿入 */
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		pLayout->m_pNext = NULL;
		m_pLayoutBot = pLayout;
	}else{
		pLayoutNext = pLayoutPrev->m_pNext;
		pLayoutPrev->m_pNext = pLayout;
		pLayoutNext->m_pPrev = pLayout;
		pLayout->m_pPrev = pLayoutPrev;
		pLayout->m_pNext = pLayoutNext;
	}
//	pLayout->m_pLine = pLine;
//	pLayout->m_pLine = pCDocLine->m_pLine->m_pData;
	pLayout->m_pCDocLine = pCDocLine;

	pLayout->m_nLinePhysical = nLine;
	pLayout->m_nOffset = nOffset;
	pLayout->m_nLength = nLength;
	pLayout->m_nTypePrev = nTypePrev;

	if( EOL_NONE == pCDocLine->m_cEol ){
		pLayout->m_cEol.SetType( EOL_NONE );/* 改行コードの種類 */
	}else{
//		if( pLayout->m_nOffset + pLayout->m_nLength >= 
		if( pLayout->m_nOffset + pLayout->m_nLength > 
			pCDocLine->m_pLine->m_nDataLen - pCDocLine->m_cEol.GetLen()
		){
			pLayout->m_cEol = pCDocLine->m_cEol;/* 改行コードの種類 */
		}else{
			pLayout->m_cEol = EOL_NONE;/* 改行コードの種類 */
		}
	}
	// pLayout->m_nEOLLen = gm_pnEolLenArr[pLayout->m_enumEOLType];/* 改行コードの長さ */ 		
	
	
	pLayout->m_nTypeNext = nTypeNext;
#ifdef _DEBUG
	if( 1 == pLayout->m_nTypeNext ){
		MYTRACE( "★★★CLayoutMgr::InsertLineNext()   1 == pLayout->m_nTypeNext\n" );
	}
#endif	
	m_nLines++;
	return pLayout;
}



//	/*
//	|| 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す
//	*/
//	const CLayout* CLayoutMgr::GetLineData( int nLine )
//	{
//		return Search( nLine );
//	}




/*
|| 指定された物理行のデータへのポインタとその長さを返す Ver0
*/
const char* CLayoutMgr::GetLineStr( int nLine, int* pnLineLen )
{
	CLayout* pLayout;
//	const char* pData;
//	int nDataLen;
	if( NULL == ( pLayout = Search( nLine )	) ){
		return NULL;
	}
//	pData = m_pcDocLineMgr->GetLineStr( pLayout->m_nLinePhysical, &nDataLen );
	*pnLineLen = pLayout->m_nLength;
//	return pData + pLayout->m_nOffset;
//	return pLayout->m_pLine + pLayout->m_nOffset;
	return pLayout->m_pCDocLine->m_pLine->m_pData + pLayout->m_nOffset;
}

/*
|| 指定された物理行のデータへのポインタとその長さを返す Ver1
*/
const char* CLayoutMgr::GetLineStr2( int nLine, int* pnLineLen, const CLayout** ppcLayoutDes )
{
	const CLayout* pLayout;
	const char* pData;
	int nDataLen;
	if( NULL == ( pLayout = Search( nLine )	) ){
		*ppcLayoutDes = pLayout;
		return NULL;
	}
	pData = m_pcDocLineMgr->GetLineStr( pLayout->m_nLinePhysical, &nDataLen );
	*pnLineLen = pLayout->m_nLength;
	*ppcLayoutDes = pLayout;
	return pData + pLayout->m_nOffset;
}



/* 行内文字削除 */
void CLayoutMgr::DeleteData_CLayoutMgr(
		int			nLineNum,
		int			nDelPos,
		int			nDelLen,
		int			*pnModifyLayoutLinesOld,
		int			*pnModifyLayoutLinesNew,
		int			*pnDeleteLayoutLines,
		CMemory&	cmemDeleted,			/* 削除されたデータ */	
		BOOL		bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
		BOOL		bDispWSTRING,	/* ダブルクォーテーション文字列を表示する */
		BOOL		bUndo			/* Undo操作かどうか */
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::DeleteData_CLayoutMgr" );
#endif
	const char*	pLine;
	int			nLineLen;
	CLayout*	pLayout;
	CLayout*	pLayoutPrev;
//	CLayout*	pLayoutNext;
	CLayout*	pLayoutWork;
	int			nModLineOldFrom;	/* 影響のあった変更前の行(from) */
	int			nModLineOldTo;		/* 影響のあった変更前の行(to) */
	int			nDelLineOldFrom;	/* 削除された変更前論理行(from) */
	int			nDelLineOldNum;		/* 削除された行数 */
	int			nRowNum;
	int			nAllLinesOld;
	int			nDelStartLogicalLine;
	int			nDelStartLogicalPos;
	int			nCurrentLineType;
	int			nAddInsLineNum;
	int			nLineWork;

	/* 現在行のデータを取得 */
	pLine = GetLineStr( nLineNum, &nLineLen );
	if( NULL == pLine ){
		return;
	}
	pLayout = m_pLayoutPrevRefer;
	nDelStartLogicalLine = pLayout->m_nLinePhysical;
	nDelStartLogicalPos  = nDelPos + pLayout->m_nOffset;
	
//	pLayoutWork = pLayout;
//	do{
//		pLayoutWork = pLayoutWork->m_pNext;		
//	}while( NULL != pLayoutWork && 0 != pLayoutWork->m_nOffset );
//	pLayoutNext = pLayoutWork;
	
	
	pLayoutWork = pLayout;
	nLineWork = nLineNum;
	while( 0 != pLayoutWork->m_nOffset ){
		pLayoutWork = pLayoutWork->m_pPrev;
		--nLineWork;
	}
	nCurrentLineType = pLayoutWork->m_nTypePrev;

	/* テキストのデータを削除 */
	m_pcDocLineMgr->DeleteData(
		nDelStartLogicalLine, nDelStartLogicalPos,
		nDelLen, &nModLineOldFrom, &nModLineOldTo,
		&nDelLineOldFrom, &nDelLineOldNum, cmemDeleted, bUndo
	);

//	DUMP();

	/*--- 変更された行のレイアウト情報を再生成 ---*/
	/* 論理行の指定範囲に該当するレイアウト情報を削除して */
	/* 削除した範囲の直前のレイアウト情報のポインタを返す */
	nAllLinesOld = m_nLines;
	pLayoutPrev = DeleteLayoutAsLogical(
		pLayoutWork,
		nLineWork,
		
		nModLineOldFrom, nModLineOldTo,
		nDelStartLogicalLine, nDelStartLogicalPos,
		pnModifyLayoutLinesOld
	);

	/* 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする */
	/* 論理行が削除された場合は０より小さい行数 */
	/* 論理行が挿入された場合は０より大きい行数 */
	ShiftLogicalLineNum( pLayoutPrev, -1 * nDelLineOldNum );

	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->m_nLinePhysical;
		}
	}else{
		if( NULL == pLayoutPrev->m_pNext ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->m_nLinePhysical - 1;
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->m_nLinePhysical -
				pLayoutPrev->m_nLinePhysical - 1;
		}
	}
	
//	if( NULL != pLayoutPrev ){
//		pLayoutNext	= pLayoutPrev->m_pNext;
//	}else{
//		pLayoutNext	= m_pLayoutBot;
//	}

	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
//	*pnModifyLayoutLinesNew = DoLayout3(
//		pLayoutPrev, nRowNum,
//		nDelStartLogicalLine, nDelStartLogicalPos
//	);

//	MYTRACE( "pLayoutPrev=[%xh]\n", pLayoutPrev );
//	MYTRACE( "pLayoutNext=[%xh]\n", pLayoutNext );
	*pnModifyLayoutLinesNew = DoLayout3_New(
		pLayoutPrev, 
//		pLayoutNext, 
		nRowNum,
		nDelStartLogicalLine, nDelStartLogicalPos,
		nCurrentLineType, 
		&nAddInsLineNum,
		bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
		bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
);

//	DUMP();

	*pnDeleteLayoutLines = nAllLinesOld - m_nLines + nAddInsLineNum;
	return;
}






/* 文字列挿入 */
void CLayoutMgr::InsertData_CLayoutMgr(
		int			nLineNum,
		int			nInsPos,
		const char*	pInsData,
		int			nInsDataLen,
		int*		pnModifyLayoutLinesOld,
		int*		pnInsLineNum,		/* 挿入によって増えたレイアウト行の数 */
		int*		pnNewLine,			/* 挿入された部分の次の位置の行 */
		int*		pnNewPos,			/* 挿入された部分の次の位置のデータ位置 */
		BOOL		bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
		BOOL		bDispWSTRING,	/* ダブルクォーテーション文字列を表示する */
		BOOL		bUndo			/* Undo操作かどうか */

)
{
	const char*	pLine;
	int			nLineLen;
	CLayout*	pLayout;
	CLayout*	pLayoutPrev;
//	CLayout*	pLayoutNext;
	CLayout*	pLayoutLast;
	CLayout*	pLayoutWork;
	int			nInsStartLogicalLine;
	int			nInsStartLogicalPos;
	int			nInsLineNum;
	int			nAllLinesOld;
	int			nRowNum;
	int			nNewLine;			/* 挿入された部分の次の位置の行 */
	int			nNewPos;			/* 挿入された部分の次の位置のデータ位置 */
	int			nCurrentLineType;
	int			nAddInsLineNum;
	int			nLineWork;

//	MYTRACE( "CLayoutMgr::InsertData()\n" );

	/* 現在行のデータを取得 */
	pLine = GetLineStr( nLineNum, &nLineLen );
	if( NULL == pLine ){
		if( 0 == nLineNum ){
			/* 空のテキストの先頭に行を作る場合 */
			pLayout = NULL;
			nLineWork = 0;
			nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
			nInsStartLogicalPos  = 0;
			nCurrentLineType = 0;
		}else{
			pLine = GetLineStr( m_nLines - 1, &nLineLen );
			pLayoutLast = m_pLayoutPrevRefer;
			if( ( nLineLen > 0 && ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF )) ||
				( nLineLen > 1 && ( pLine[nLineLen - 2] == CR || pLine[nLineLen - 2] == LF )) ){
				/* 空でないテキストの最後に行を作る場合 */
				pLayout = NULL;
				nLineWork = 0;
				nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
				nInsStartLogicalPos  = 0;
				nCurrentLineType = pLayoutLast->m_nTypeNext;
			}else{
				/* 空でないテキストの最後の行を変更する場合 */
				nLineNum = m_nLines	- 1;
				nInsPos = nLineLen;
				pLayout = m_pLayoutPrevRefer;
				nLineWork = m_nPrevReferLine;


				nInsStartLogicalLine = pLayout->m_nLinePhysical;
				nInsStartLogicalPos  = nInsPos + pLayout->m_nOffset;
				nCurrentLineType = pLayout->m_nTypePrev;
			} 
		}
	}else{
		pLayout = m_pLayoutPrevRefer;
		nLineWork = m_nPrevReferLine;


		nInsStartLogicalLine = pLayout->m_nLinePhysical;
		nInsStartLogicalPos  = nInsPos + pLayout->m_nOffset;
		nCurrentLineType = pLayout->m_nTypePrev;
	}
	
	if( NULL != pLayout ){
		pLayoutWork = pLayout;
		while( pLayoutWork != NULL && 0 != pLayoutWork->m_nOffset ){
			pLayoutWork = pLayoutWork->m_pPrev;		
			nLineWork--;
		}
		if( NULL != pLayoutWork ){
			nCurrentLineType = pLayoutWork->m_nTypePrev;
		}else{
			nCurrentLineType = 0;
		}
	}


	/* データの挿入 */
	m_pcDocLineMgr->InsertData_CDocLineMgr(
		nInsStartLogicalLine,
		nInsStartLogicalPos,
		pInsData, 
		nInsDataLen,
		&nInsLineNum,
		&nNewLine, 
		&nNewPos, 
		bUndo
	);
//	MYTRACE( "nNewLine=%d nNewPos=%d \n", nNewLine, nNewPos );


	/*--- 変更された行のレイアウト情報を再生成 ---*/
	/* 論理行の指定範囲に該当するレイアウト情報を削除して */
	/* 削除した範囲の直前のレイアウト情報のポインタを返す */
	nAllLinesOld = m_nLines;
	if( NULL != pLayout ){
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork, 
			nLineWork,			
			
			nInsStartLogicalLine, nInsStartLogicalLine,
			nInsStartLogicalLine, nInsStartLogicalPos,
			pnModifyLayoutLinesOld
		);
	}else{
		pLayoutPrev = m_pLayoutBot;
	}

	/* 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする */
	/* 論理行が削除された場合は０より小さい行数 */
	/* 論理行が挿入された場合は０より大きい行数 */
	if( NULL != pLine ){
		ShiftLogicalLineNum( pLayoutPrev, nInsLineNum );
	}

	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->m_nLinePhysical;
		}
	}else{
		if( NULL == pLayoutPrev->m_pNext ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->m_nLinePhysical - 1;
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->m_nLinePhysical -
				pLayoutPrev->m_nLinePhysical - 1;
		}
	}


//	if( NULL != pLayoutPrev ){
//		pLayoutNext	= pLayoutPrev->m_pNext;
//	}else{
//		pLayoutNext	= m_pLayoutBot;
//	}

	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
//	DoLayout3(
//		pLayoutPrev, nRowNum,
//		nInsStartLogicalLine, nInsStartLogicalPos
//	);
	DoLayout3_New(
		pLayoutPrev,  
//		pLayoutNext, 
		nRowNum,
		nInsStartLogicalLine, nInsStartLogicalPos,
		nCurrentLineType,
		&nAddInsLineNum,
		bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
		bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
	);

	*pnInsLineNum = m_nLines - nAllLinesOld + nAddInsLineNum;

	/* 論理位置→レイアウト位置変換 */
	pLayout = (CLayout*)Search( nNewLine );
	XYLogicalToLayout( pLayout, nNewLine, nNewLine, nNewPos, pnNewLine, pnNewPos );

//	MYTRACE( "nNewLine=%d nNewPos=%d \n", nNewLine, nNewPos );
//	MYTRACE( "*pnNewLine=%d *pnNewPos=%d \n", *pnNewLine, *pnNewPos );

	return;
}






/* 論理行の指定範囲に該当するレイアウト情報を削除して */
/* 削除した範囲の直前のレイアウト情報のポインタを返す */
CLayout* CLayoutMgr::DeleteLayoutAsLogical(
			CLayout* pLayoutInThisArea,
			int		nLineOf_pLayoutInThisArea,

			int nLineFrom,
			int nLineTo,
			int nDelLogicalLineFrom,
			int nDelLogicalColFrom,
			int* pnDeleteLines
)
{

	
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::DeleteLayoutAsLogical" );
//#endif
	CLayout* pLayout;
	CLayout* pLayoutWork;
	CLayout* pLayoutNext;

	*pnDeleteLines = 0;
	if( 0 == m_nLines){	/* 全物理行数 */
		return NULL;
	}
	if( NULL == pLayoutInThisArea ){
		return NULL;
	}

	// 1999.11.22 
	m_pLayoutPrevRefer = pLayoutInThisArea->m_pPrev;
	m_nPrevReferLine = nLineOf_pLayoutInThisArea - 1;
//	m_pLayoutPrevRefer = NULL;
//	m_nPrevReferLine = 0;


	/* 範囲内先頭に該当するレイアウト情報をサーチ */
	pLayoutWork = pLayoutInThisArea->m_pPrev;
	while( NULL != pLayoutWork && nLineFrom <= pLayoutWork->m_nLinePhysical){
		pLayoutWork = pLayoutWork->m_pPrev;
	}
//			m_pLayoutPrevRefer = pLayout->m_pPrev;
//			--m_nPrevReferLine;



	if( NULL == pLayoutWork ){
		pLayout	= m_pLayoutTop;
	}else{
		pLayout = pLayoutWork->m_pNext;
	}
	while( NULL != pLayout ){
		if( pLayout->m_nLinePhysical > nLineTo ){
			break;
		}
		pLayoutNext = pLayout->m_pNext;
		if( NULL == pLayoutWork ){
			/* 先頭行の処理 */
			m_pLayoutTop = pLayout->m_pNext;
			if( NULL != pLayout->m_pNext ){
				pLayout->m_pNext->m_pPrev = NULL;
			}
		}else{
			pLayoutWork->m_pNext = pLayout->m_pNext;
			if( NULL != pLayout->m_pNext ){
				pLayout->m_pNext->m_pPrev = pLayoutWork;
			}
		}
//		if( m_pLayoutPrevRefer == pLayout ){
//			// 1999.12.22 前にずらすだけでよいのでは
//			m_pLayoutPrevRefer = pLayout->m_pPrev;
//			--m_nPrevReferLine;
//		}

		if( ( nDelLogicalLineFrom == pLayout->m_nLinePhysical &&
			  nDelLogicalColFrom < pLayout->m_nOffset + pLayout->m_nLength ) ||
		    ( nDelLogicalLineFrom < pLayout->m_nLinePhysical )
		){
			(*pnDeleteLines)++;;
		}

#ifdef _DEBUG
		if( m_pLayoutPrevRefer == pLayout ){
			MYTRACE( "バグバグ\n" );
		}
#endif
		delete pLayout;
		
		m_nLines--;	/* 全物理行数 */
		if( NULL == pLayoutNext ){
			m_pLayoutBot = pLayoutWork;
		}
		pLayout = pLayoutNext;
	}
//	MYTRACE( "(*pnDeleteLines)=%d\n", (*pnDeleteLines) );

	return pLayoutWork;
}




/* 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする */
/* 論理行が削除された場合は０より小さい行数 */
/* 論理行が挿入された場合は０より大きい行数 */
void CLayoutMgr::ShiftLogicalLineNum( CLayout* pLayoutPrev, int nShiftLines )
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::ShiftLogicalLineNum" );
#endif
	CLayout* pLayout;
//	CLayout* pLayoutNext;
	if( 0 == nShiftLines ){
		return;
	}
	if( NULL == pLayoutPrev ){
		pLayout = m_pLayoutTop;
	}else{
		pLayout = pLayoutPrev->m_pNext;
	}
	/* レイアウト情報全体を更新する(なな、なんと!!!) */
	while( NULL != pLayout ){
//		pLayoutNext = pLayout->m_pNext;
		pLayout->m_nLinePhysical += nShiftLines;	/* 対応する論理行番号 */
//		pLayout = pLayoutNext;
		pLayout = pLayout->m_pNext;
	}
	return;
}





/* 現在位置の単語の範囲を調べる */
int CLayoutMgr::WhereCurrentWord(
	int			nLineNum,
	int			nIdx,
	int*		pnLineFrom,
	int*		pnIdxFrom,
	int*		pnLineTo,
	int*		pnIdxTo,
	CMemory*	pcmcmWord,
	CMemory*	pcmcmWordLeft
)
{
	int nRetCode;
	CLayout* pLayout;
	pLayout = Search( nLineNum );
	if( NULL == pLayout ){
		return FALSE;
	}
	/* 現在位置の単語の範囲を調べる */
	nRetCode = m_pcDocLineMgr->WhereCurrentWord(
		pLayout->m_nLinePhysical,
		pLayout->m_nOffset + nIdx,
		pnIdxFrom,
		pnIdxTo,
		pcmcmWord,
		pcmcmWordLeft
	);
	if( nRetCode ){
		/* 論理位置→レイアウト位置変換 */
		XYLogicalToLayout( pLayout, nLineNum, pLayout->m_nLinePhysical, *pnIdxFrom, pnLineFrom, pnIdxFrom );
		XYLogicalToLayout( pLayout, nLineNum, pLayout->m_nLinePhysical, *pnIdxTo,   pnLineTo, pnIdxTo );
	}
	return nRetCode;

}





/* 現在位置の左右の単語の先頭位置を調べる */
int CLayoutMgr::PrevOrNextWord(
		int		nLineNum,
		int		nIdx,
		int*	pnLineNew,
		int*	pnColmNew,
		int		bLEFT
)
{
	int			nRetCode;
	CLayout*	pLayout;
	pLayout = Search( nLineNum );
	if( NULL == pLayout ){
		return FALSE;
	}
	/* 現在位置の左右の単語の先頭位置を調べる */
	nRetCode = m_pcDocLineMgr->PrevOrNextWord(
		pLayout->m_nLinePhysical,
		pLayout->m_nOffset + nIdx,
		pnColmNew,
		bLEFT
	);

	if( nRetCode ){
		/* 論理位置→レイアウト位置変換 */
		XYLogicalToLayout( pLayout, nLineNum, pLayout->m_nLinePhysical, *pnColmNew, pnLineNew, pnColmNew );
	}
	return nRetCode;
}





/* 単語検索 */
/* 見つからない場合は０を返す */
int CLayoutMgr::SearchWord(
	int			nLineNum, 		/* 検索開始行 */
	int			nIdx, 			/* 検索開始位置 */
	const char*	pszPattern,		/* 検索条件 */
	int			bPrevOrNext,	/* 0==前方検索 1==後方検索 */
	int			bRegularExp,	/* 1==正規表現 */
	int			bLoHiCase,		/* 1==大文字小文字の区別 */
	int			bWordOnly,		/* 1==単語のみ検索 */
	int*		pnLineFrom, 	/* マッチレイアウト行from */
	int*		pnIdxFrom, 		/* マッチレイアウト位置from */
	int*		pnLineTo, 		/* マッチレイアウト行to */
	int*		pnIdxTo,  		/* マッチレイアウト位置to */
	CJre*		pCJre			/* 正規表現コンパイルデータ */

)
{
	int			nRetCode;
	int			nLogLine;
	CLayout*	pLayout;
	pLayout = Search( nLineNum );
	if( NULL == pLayout ){
		return FALSE;
	}
	/* 単語検索 */
	nRetCode = m_pcDocLineMgr->SearchWord(
		pLayout->m_nLinePhysical,
		pLayout->m_nOffset + nIdx,
		pszPattern,
		bPrevOrNext,
		bRegularExp,
		bLoHiCase,
		bWordOnly,
		pnLineFrom,
		pnIdxFrom,
		pnIdxTo,
		pCJre			/* 正規表現コンパイルデータ */
	);

	if( nRetCode ){
		/* 論理位置→レイアウト位置変換 */
		nLogLine = *pnLineFrom;
//		XYLogicalToLayout( pLayout, nLineNum, nLogLine, *pnIdxFrom, pnLineFrom, pnIdxFrom );
//		XYLogicalToLayout( pLayout, nLineNum, nLogLine, *pnIdxTo,   pnLineTo,   pnIdxTo );
		CaretPos_Phys2Log(
			*pnIdxFrom,	
			nLogLine,
			pnIdxFrom,	
			pnLineFrom
		);
		CaretPos_Phys2Log(
			*pnIdxTo,	
			nLogLine,
			pnIdxTo,	
			pnLineTo
		);
	}
	return nRetCode;
}





/* 論理位置→レイアウト位置変換 */
void CLayoutMgr::XYLogicalToLayout(
		CLayout*	pLayoutInThisArea,
		int			nLayoutLineOfThisArea,
		int			nLogLine,
		int			nLogIdx,
		int*		pnLayLine,
		int*		pnLayIdx
)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::XYLogicalToLayout" );
//#endif
	CLayout*	pLayout;
	int			nCurLayLine;
	*pnLayLine = 0;
	*pnLayIdx = 0;

	if( pLayoutInThisArea == NULL ){
		pLayoutInThisArea = m_pLayoutBot;
		nLayoutLineOfThisArea = m_nLines - 1;
	}
	if( pLayoutInThisArea == NULL ){
		*pnLayLine = m_nLines;
		*pnLayIdx = 0;
		return;
	}
	/* 範囲内先頭に該当するレイアウト情報をサーチ */
	if( (pLayoutInThisArea->m_nLinePhysical > nLogLine) ||
		(pLayoutInThisArea->m_nLinePhysical == nLogLine &&
		 pLayoutInThisArea->m_nOffset >	nLogIdx)
	){
		/* 現在位置より前方に向かってサーチ */
		pLayout =  pLayoutInThisArea->m_pPrev;
		nCurLayLine = nLayoutLineOfThisArea - 1;
		while( pLayout != NULL ){
			if( pLayout->m_nLinePhysical == nLogLine &&
				pLayout->m_nOffset <= nLogIdx &&
				nLogIdx <= pLayout->m_nOffset + pLayout->m_nLength
			){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_nOffset;
				return;
			}
			if( NULL == pLayout->m_pPrev ){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = 0;
				return;
			}
			pLayout = pLayout->m_pPrev;
			nCurLayLine--;
		}
	}else{
		/* 現在位置を含む後方に向かってサーチ */
		pLayout =  pLayoutInThisArea;
		nCurLayLine = nLayoutLineOfThisArea;
		while( pLayout != NULL ){
			if( pLayout->m_nLinePhysical   == nLogLine &&
				pLayout->m_nOffset <= nLogIdx  &&
				nLogIdx <= pLayout->m_nOffset + pLayout->m_nLength
			 ){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_nOffset;
				return;
			}
			if( NULL == pLayout->m_pNext ){
//			    if( nCurLayLine == nLogLine ){
//					*pnLayLine = nCurLayLine;
//					*pnLayIdx = pLayout->m_nLength;
//				}else{
					*pnLayLine = nCurLayLine + 1;
					*pnLayIdx = 0;
//				}
				return;
			}else
			if( pLayout->m_pNext->m_nLinePhysical > nLogLine ){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_nOffset;
				return;
			}
			pLayout = pLayout->m_pNext;
			nCurLayLine++;
		}
	}
	return;
}


/*
  カーソル位置変換
  物理位置(行頭からのバイト数、折り返し無し行位置)
  →
  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
*/
void CLayoutMgr::CaretPos_Phys2Log(
		int		nX,
		int		nY,
		int*	pnCaretPosX,
		int*	pnCaretPosY
//		BOOL	bFreeCaret
)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::CaretPos_Phys2Log" );
//#endif
	const CLayout*	pLayout;
//	CLayout*		pLayoutNext;
	int				nCaretPosX;
	int				nCaretPosY;
	const char*		pData;
	int				nDataLen;
	int				i;
	int				nCharChars;

	*pnCaretPosX = 0;
	*pnCaretPosY = 0;
//1999.12.11
//	サーチ開始地点は先頭から
//	pLayout = m_pLayoutTop;
//	nCaretPosY = 0;

	/* 改行単位行 <= 折り返し単位行　が成り立つから、
	サーチ開始地点をできるだけ目的地へ近づける */
//	pLayout = GetLineData( nY );
	pLayout = Search( nY );
	if( NULL == pLayout ){
		if( 0 < m_nLines ){
			*pnCaretPosY = m_nLines;
		}
		return;
	}
	nCaretPosY = nY;

	
	nCaretPosX = 0;
	do{
		if( nY == pLayout->m_nLinePhysical ){
			nCaretPosX = 0;
//			pData = GetLineStr( nCaretPosY, &nDataLen );
//			pData = pLayout->m_pLine + pLayout->m_nOffset;
			pData = pLayout->m_pCDocLine->m_pLine->m_pData + pLayout->m_nOffset;
			nDataLen = pLayout->m_nLength;

			for( i = 0; i < nDataLen; ++i ){
				if( pLayout->m_nOffset + i >= nX ){
					break;
				}
				if( pData[i] ==	TAB ){
					nCharChars = m_nTabSpace - ( nCaretPosX % m_nTabSpace );
				}else{
					nCharChars = CMemory::MemCharNext( pData, nDataLen, &pData[i] ) - &pData[i];
				}
				if( nCharChars == 0 ){
					nCharChars = 1;
				}
				nCaretPosX += nCharChars;
				if( pData[i] ==	TAB ){
					nCharChars = 1;
				}
				i += nCharChars - 1;
			}
			if( i < nDataLen ){
				break;
			}
			if( NULL == pLayout->m_pNext ){
				nCaretPosX += ( nDataLen - i );
//				nCaretPosX = 0;
				break;
			}
			if( nY < pLayout->m_pNext->m_nLinePhysical ){
				nCaretPosX += ( nDataLen - i );
				break;
			}
//		}else{
//			nCaretPosX = 0;
		}
		if( nY < pLayout->m_nLinePhysical ){
			break;
		}else{
//			nCaretPosX = 0;
			nCaretPosY++;
		}
		pLayout = pLayout->m_pNext;
	}while( NULL != pLayout );
	*pnCaretPosX = nCaretPosX;
	*pnCaretPosY = nCaretPosY;
//#ifdef _DEBUG
//	MYTRACE( "\t\tnCaretPosY - nY = %d\n", nCaretPosY - nY );
//#endif
	return;
}

/*
  カーソル位置変換
  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
  →
  物理位置(行頭からのバイト数、折り返し無し行位置)
*/
void CLayoutMgr::CaretPos_Log2Phys(
		int		nCaretPosX,
		int		nCaretPosY,
		int*	pnX,
		int*	pnY
)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::CaretPos_Log2Phys" );
//#endif
	int				nX;
//	int				nY;
	const CLayout*	pLayout;
	int				i;
	const char*		pData;
	int				nDataLen;
	int				nCharChars;
	BOOL			bEOF;
	bEOF = FALSE;

	*pnX = 0;
	*pnY = 0;
	if( nCaretPosY > m_nLines ){
		*pnX = 0;
		*pnY = m_nLines;
		return;
	}
//	pLayout = GetLineData( nCaretPosY );
	pLayout = Search( nCaretPosY );
	if( NULL == pLayout ){
		if( 0 < nCaretPosY ){
//			pLayout = GetLineData( nCaretPosY - 1 );
			pLayout = Search( nCaretPosY - 1 );
			if( NULL == pLayout ){
				*pnX = 0;
				*pnY = m_pcDocLineMgr->m_nLines;
				return;
			}else{
				pData = GetLineStr( nCaretPosY - 1, &nDataLen );
				if( pData[nDataLen - 1] == '\r' || pData[nDataLen - 1] == '\n' ){
					*pnX = 0;
					*pnY = m_pcDocLineMgr->m_nLines;
					return;
				}else{
					*pnY = m_pcDocLineMgr->m_nLines - 1;
					bEOF = TRUE;
					nX = 999999;
					goto checkloop;
				
				}
			}
		}
		*pnX = 0;
		*pnY = m_nLines;
		return;
	}else{
		*pnY = pLayout->m_nLinePhysical;
	}
	
	pData = GetLineStr( nCaretPosY, &nDataLen );
	nX = 0;
checkloop:;	
//	enumEOLType nEOLType;
	for( i = 0; i < nDataLen; ++i ){
		if( pData[i] ==	TAB ){
			nCharChars = m_nTabSpace - ( nX % m_nTabSpace );
//		}else
//		if( pData[i] == '\r' || pData[i] == '\n' ){
//			/* 行終端子の種類を調べる */
//			nEOLType = GetEOLType( &pData[i], nDataLen - i );
//			nCharChars = 1;
		}else{
			nCharChars = CMemory::MemCharNext( pData, nDataLen, &pData[i] ) - &pData[i];
		}

		if( nCharChars == 0 ){
			nCharChars = 1;
		}
		nX += nCharChars;
		if( nX > nCaretPosX && !bEOF ){
			break;
		}
		if( pData[i] ==	TAB ){
			nCharChars = 1;
		}
//		if( pData[i] == '\r' || pData[i] == '\n' ){
//			nCharChars = gm_pnEolLenArr[nEOLType];
//		}
		i += nCharChars - 1;
	}
	i += pLayout->m_nOffset;
	*pnX = i;
	return;
}



/* テスト用にレイアウト情報をダンプ */
void CLayoutMgr::DUMP( void )
{
#ifdef _DEBUG
	const char* pData;
	int nDataLen;
	MYTRACE( "------------------------\n" );
	MYTRACE( "m_nLines=%d\n", m_nLines );
	MYTRACE( "m_pLayoutTop=%08lxh\n", m_pLayoutTop );
	MYTRACE( "m_pLayoutBot=%08lxh\n", m_pLayoutBot );
	MYTRACE( "m_nMaxLineSize=%d\n", m_nMaxLineSize );

	MYTRACE( "m_nTabSpace=%d\n", m_nTabSpace );
	CLayout* pLayout;
	CLayout* pLayoutNext;
	pLayout = m_pLayoutTop;
	while( NULL != pLayout ){
		pLayoutNext = pLayout->m_pNext;
		MYTRACE( "\t-------\n" );
		MYTRACE( "\tthis=%08lxh\n", pLayout );
		MYTRACE( "\tm_pPrev =%08lxh\n", pLayout->m_pPrev );
		MYTRACE( "\tm_pNext =%08lxh\n", pLayout->m_pNext );
		MYTRACE( "\tm_nLinePhysical=%d\n",      pLayout->m_nLinePhysical );
		MYTRACE( "\tm_nOffset=%d\n",     pLayout->m_nOffset );
		MYTRACE( "\tm_nLength=%d\n",     pLayout->m_nLength );
		MYTRACE( "\tm_enumEOLType =%s\n", pLayout->m_cEol.GetName() );
		MYTRACE( "\tm_nEOLLen =%d\n", pLayout->m_cEol.GetLen() );
		MYTRACE( "\tm_nTypePrev=%d\n",   pLayout->m_nTypePrev );
		MYTRACE( "\tm_nTypeNext=%d\n",   pLayout->m_nTypeNext );
		pData = m_pcDocLineMgr->GetLineStr( pLayout->m_nLinePhysical, &nDataLen );
		MYTRACE( "\t[%s]\n",   pData );
		pLayout = pLayoutNext;
	}
	MYTRACE( "------------------------\n" );
#endif
	return;
}


/*[EOF]*/
