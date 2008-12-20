#include "stdafx.h"
#include "docplus/CBookmarkManager.h"
#include "CSearchAgent.h"

bool CBookmarkGetter::IsBookmarked() const{ return m_pcDocLine->m_sMark.m_cBookmarked; }
void CBookmarkSetter::SetBookmark(bool bFlag){ m_pcDocLine->m_sMark.m_cBookmarked = bFlag; }

//!ブックマークの全解除
/*
	@date 2001.12.03 hor
*/
void CBookmarkManager::ResetAllBookMark( void )
{
	CDocLine* pDocLine = m_pcDocLineMgr->GetDocLineTop();
	while( pDocLine ){
		CBookmarkSetter(pDocLine).SetBookmark(false);
		pDocLine = pDocLine->GetNextLine();
	}
}


//! ブックマーク検索
/*
	@date 2001.12.03 hor
*/
bool CBookmarkManager::SearchBookMark(
	CLogicInt			nLineNum,		/* 検索開始行 */
	ESearchDirection	bPrevOrNext,	/* 0==前方検索 1==後方検索 */
	CLogicInt*			pnLineNum 		/* マッチ行 */
)
{
	CDocLine*	pDocLine;
	CLogicInt	nLinePos=nLineNum;

	//前方検索
	if( bPrevOrNext == SEARCH_BACKWARD ){
		nLinePos--;
		pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine ){
			if(CBookmarkGetter(pDocLine).IsBookmarked()){
				*pnLineNum = nLinePos;				/* マッチ行 */
				return true;
			}
			nLinePos--;
			pDocLine = pDocLine->GetPrevLine();
		}
	}
	//後方検索
	else{
		nLinePos++;
		pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while( NULL != pDocLine ){
			if(CBookmarkGetter(pDocLine).IsBookmarked()){
				*pnLineNum = nLinePos;				/* マッチ行 */
				return true;
			}
			nLinePos++;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

//! 物理行番号のリストからまとめて行マーク
/*
	@date 2002.01.16 hor
*/
void CBookmarkManager::SetBookMarks( wchar_t* pMarkLines )
{
	CDocLine*	pCDocLine;
	wchar_t *p;
	wchar_t delim[] = L", ";
	p = pMarkLines;
	while(wcstok(p, delim) != NULL) {
		while(wcschr(delim, *p) != NULL)p++;
		pCDocLine=m_pcDocLineMgr->GetLine( CLogicInt(_wtol(p)) );
		if(pCDocLine)CBookmarkSetter(pCDocLine).SetBookmark(true);
		p += wcslen(p) + 1;
	}
}


//! 行マークされてる物理行番号のリストを作る
/*
	@date 2002.01.16 hor
*/
LPCWSTR CBookmarkManager::GetBookMarks()
{
	CDocLine*	pCDocLine;
	static wchar_t szText[MAX_MARKLINES_LEN + 1];	//2002.01.17 // Feb. 17, 2003 genta staticに
	wchar_t szBuff[10];
	CLogicInt	nLinePos=CLogicInt(0);
	pCDocLine = m_pcDocLineMgr->GetLine( nLinePos );
	wcscpy( szText, L"" );
	while( pCDocLine ){
		if(CBookmarkGetter(pCDocLine).IsBookmarked()){
			auto_sprintf( szBuff, L"%d,",nLinePos );
			if(wcslen(szBuff)+wcslen(szText)>MAX_MARKLINES_LEN)break;	//2002.01.17
			wcscat( szText, szBuff);
		}
		nLinePos++;
		pCDocLine = pCDocLine->GetNextLine();
	}
	return szText; // Feb. 17, 2003 genta
}




//! 検索条件に該当する行にブックマークをセットする
/*
	@date 2002.01.16 hor
*/
void CBookmarkManager::MarkSearchWord(
	const wchar_t*			pszPattern,		//!< 検索条件
	const SSearchOption&	sSearchOption,	//!< 検索オプション
	CBregexp*				pRegexp			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
)
{
	CDocLine*	pDocLine;
	const wchar_t*	pLine;
	int			nLineLen;
	const wchar_t*	pszRes;
	int*		pnKey_CharCharsArr;
	//	Jun. 10, 2003 Moca
	//	wcslenを毎回呼ばずにnPatternLenを使うようにする
	const int	nPatternLen = wcslen( pszPattern );

	/* 1==正規表現 */
	if( sSearchOption.bRegularExp ){
		pDocLine = m_pcDocLineMgr->GetLine( CLogicInt(0) );
		while( pDocLine ){
			if(!CBookmarkGetter(pDocLine).IsBookmarked()){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				// 2005.03.19 かろと 前方一致サポートのためのメソッド変更
				if( pRegexp->Match( pLine, nLineLen, 0 ) ){
					CBookmarkSetter(pDocLine).SetBookmark(true);
				}
			}
			pDocLine = pDocLine->GetNextLine();
		}
	}
	/* 1==単語のみ検索 */
	else if( sSearchOption.bWordOnly ){
		pDocLine = m_pcDocLineMgr->GetLine( CLogicInt(0) );
		CLogicInt nLinePos = CLogicInt(0);
		CLogicInt nNextWordFrom = CLogicInt(0);
		CLogicInt nNextWordFrom2;
		CLogicInt nNextWordTo2;
		while( pDocLine ){
			if(!CBookmarkGetter(pDocLine).IsBookmarked() &&
				CSearchAgent(m_pcDocLineMgr).WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL )) {
				const wchar_t* pData = pDocLine->GetPtr(); // 2002/2/10 aroka CMemory変更
				
				if(( nPatternLen == nNextWordTo2 - nNextWordFrom2 ) &&
				   (( !sSearchOption.bLoHiCase && 0 == auto_memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )) ||
					( sSearchOption.bLoHiCase && 0 == auto_memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )))){
					CBookmarkSetter(pDocLine).SetBookmark(true);
				}
				else if( CSearchAgent(m_pcDocLineMgr).PrevOrNextWord( nLinePos, nNextWordFrom, &nNextWordFrom, FALSE, FALSE) ){
					continue;
				}
			}
			/* 次の行を見に行く */
			nLinePos++;
			pDocLine = pDocLine->GetNextLine();
			nNextWordFrom = CLogicInt(0);
		}
	}
	else{
		/* 検索条件の情報 */
		pnKey_CharCharsArr = NULL;
		CSearchAgent::CreateCharCharsArr(
			pszPattern,
			nPatternLen,
			&pnKey_CharCharsArr
		);
		pDocLine = m_pcDocLineMgr->GetLine( CLogicInt(0) );
		while( NULL != pDocLine ){
			if(!CBookmarkGetter(pDocLine).IsBookmarked()){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				pszRes = CSearchAgent::SearchString(
					pLine,
					nLineLen,
					0,
					pszPattern,
					nPatternLen,
					pnKey_CharCharsArr,
					sSearchOption.bLoHiCase
				);
				if( NULL != pszRes ){
					CBookmarkSetter(pDocLine).SetBookmark(true);
				}
			}
			pDocLine = pDocLine->GetNextLine();
		}
		if( NULL != pnKey_CharCharsArr ){
			delete [] pnKey_CharCharsArr;
			pnKey_CharCharsArr = NULL;
		}
	}
}

