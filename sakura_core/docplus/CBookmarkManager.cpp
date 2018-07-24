#include "StdAfx.h"
#include "docplus/CBookmarkManager.h"
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"
#include "CSearchAgent.h"
#include "extmodule/CBregexp.h"


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
	CLogicInt			nLineNum,		//!< 検索開始行
	ESearchDirection	bPrevOrNext,	//!< 検索方向
	CLogicInt*			pnLineNum 		//!< マッチ行
)
{
	CDocLine*	pDocLine;
	CLogicInt	nLinePos=nLineNum;

	// 後方検索
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
	// 前方検索
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
	@date 2014.04.24 Moca ver2 差分32進数方式に変更
*/
void CBookmarkManager::SetBookMarks( wchar_t* pMarkLines )
{
	CDocLine*	pCDocLine;
	wchar_t *p;
	wchar_t delim[] = L", ";
	p = pMarkLines;
	if( p[0] == L':' ){
		if( p[1] == L'0' ){
			// ver2 形式 [0-9a-v] 0-31(終端バージョン) [w-zA-Z\+\-] 0-31
			// 2番目以降は、数値+1+ひとつ前の値
			// :00123x0 => 0,1,2,3,x0 => 0,(1+1),(2+2+1),(3+5+1),(32+9+1) => 0,2,5,9,42 => 1,3,6,10,43行目
			// :0a => a, => 10, 11
			p += 2;
			int nLineNum = 0;
			int nLineTemp = 0;
			while( *p != L'\0' ){
				bool bSeparete = false;
				if( L'0' <= *p && *p <= L'9' ){
					nLineTemp += (*p - L'0');
					bSeparete = true;
				}else if( L'a' <= *p && *p <= L'v' ){
					nLineTemp += (*p - L'a') + 10;
					bSeparete = true;
				}else if( L'w' <= *p && *p <= L'z' ){
					nLineTemp += (*p - L'w');
				}else if( L'A' <= *p && *p <= L'Z' ){
					nLineTemp += (*p - L'A') + 4;
				}else if( *p == L'+' ){
					nLineTemp += 30;
				}else if( *p == L'-' ){
					nLineTemp += 31;
				}else{
					break;
				}
				if( bSeparete ){
					nLineNum += nLineTemp;
					pCDocLine = m_pcDocLineMgr->GetLine( CLogicInt(nLineNum) );
					if( pCDocLine ){
						CBookmarkSetter(pCDocLine).SetBookmark(true);
					}
					nLineNum++;
					nLineTemp = 0;
				}else{
					nLineTemp *= 32;
				}
				p++;
			}
		}else{
			// 不明なバージョン
		}
	}else{
		// 旧形式 行番号,区切り
		while(wcstok(p, delim) != NULL) {
			while(wcschr(delim, *p) != NULL)p++;
			pCDocLine=m_pcDocLineMgr->GetLine( CLogicInt(_wtol(p)) );
			if(pCDocLine)CBookmarkSetter(pCDocLine).SetBookmark(true);
			p += wcslen(p) + 1;
		}
	}
}


//! 行マークされてる物理行番号のリストを作る
/*
	@date 2002.01.16 hor
	@date 2014.04.24 Moca ver2 差分32進数方式に変更
*/
LPCWSTR CBookmarkManager::GetBookMarks()
{
	const CDocLine*	pCDocLine;
	static wchar_t szText[MAX_MARKLINES_LEN + 1];	//2002.01.17 // Feb. 17, 2003 genta staticに
	wchar_t szBuff[10];
	wchar_t szBuff2[10];
	CLogicInt	nLinePos=CLogicInt(0);
	CLogicInt	nLinePosOld=CLogicInt(-1);
	int			nTextLen = 2;
	pCDocLine = m_pcDocLineMgr->GetLine( nLinePos );
	wcscpy( szText, L":0" );
	while( pCDocLine ){
		if(CBookmarkGetter(pCDocLine).IsBookmarked()){
			CLogicInt nDiff = nLinePos - nLinePosOld - CLogicInt(1);
			nLinePosOld = nLinePos;
			if( nDiff == CLogicInt(0) ){
				szBuff2[0] = L'0';
				szBuff2[1] = L'\0';
			}else{
				int nColumn = 0;
				while( nDiff ){
					CLogicInt nKeta = nDiff % 32;
					wchar_t c;
					if( nColumn == 0 ){
						if( nKeta <= 9 ){
							c = (wchar_t)((Int)nKeta + L'0');
						}else{
							c = (wchar_t)((Int)nKeta - 10 + L'a');
						}
					}else{
						if( nKeta <= 3 ){
							c = (wchar_t)((Int)nKeta + L'w');
						}else if( nKeta <= 29 ){
							c = (wchar_t)((Int)nKeta - 4 + L'A');
						}else if( nKeta == 30 ){
							c = L'+';
						}else{ // 31
							c = L'-';
						}
					}
					szBuff[nColumn] = c;
					nColumn++;
					nDiff /= 32;
				}
				int i = 0;
				for(; i < nColumn; i++ ){
					szBuff2[i] = szBuff[nColumn-1-i];
				}
				szBuff2[nColumn] = L'\0';
			}
			int nBuff2Len = wcslen(szBuff2);
			if( nBuff2Len + nTextLen > MAX_MARKLINES_LEN ) break;	//2002.01.17
			wcscpy( szText + nTextLen, szBuff2 );
			nTextLen += nBuff2Len;
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
	const CSearchStringPattern& pattern
)
{
	const SSearchOption&	sSearchOption = pattern.GetSearchOption();
	CDocLine*	pDocLine;
	const wchar_t*	pLine;
	int			nLineLen;

	/* 1==正規表現 */
	if( sSearchOption.bRegularExp ){
		CBregexp*	pRegexp = pattern.GetRegexp();
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
		const wchar_t*	pszPattern = pattern.GetKey();
		const int	nPatternLen = pattern.GetLen();
		// 検索語を単語に分割して searchWordsに格納する。
		std::vector<std::pair<const wchar_t*, CLogicInt> > searchWords; // 単語の開始位置と長さの配列。
		CSearchAgent::CreateWordList(searchWords, pszPattern, nPatternLen);

		pDocLine = m_pcDocLineMgr->GetLine( CLogicInt(0) );
		while( pDocLine ){
			if(!CBookmarkGetter(pDocLine).IsBookmarked()){
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				int nMatchLen;
				if( CSearchAgent::SearchStringWord(pLine, nLineLen, 0, searchWords, sSearchOption.bLoHiCase, &nMatchLen) ){
					CBookmarkSetter(pDocLine).SetBookmark(true);
				}
			}
			/* 次の行を見に行く */
			pDocLine = pDocLine->GetNextLine();
		}
	}
	else{
		/* 検索条件の情報 */
		pDocLine = m_pcDocLineMgr->GetLine( CLogicInt(0) );
		while( NULL != pDocLine ){
			if(!CBookmarkGetter(pDocLine).IsBookmarked()){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				if( CSearchAgent::SearchString(
					pLine,
					nLineLen,
					0,
					pattern
				) ){
					CBookmarkSetter(pDocLine).SetBookmark(true);
				}
			}
			pDocLine = pDocLine->GetNextLine();
		}
	}
}

