#include "StdAfx.h"

#include <vector>
#include <utility>
#include "CSearchAgent.h"
#include "doc/CDocLineMgr.h"
#include "doc/CDocLine.h"
#include "dlg/CDlgCancel.h"
#include "util/string_ex.h"
#include "sakura_rc.h"

//#define MEASURE_SEARCH_TIME
#ifdef MEASURE_SEARCH_TIME
#include <time.h>
#endif

// CSearchStringPattern
// @date 2010.06.22 Moca
inline int CSearchStringPattern::GetMapIndex( wchar_t c )
{
	// ASCII    => 0x000 - 0x0ff
	// それ以外 => 0x100 - 0x1ff
	return ((c & 0xff00) ? 0x100 : 0 ) | (c & 0xff);
}

CSearchStringPattern::CSearchStringPattern( const wchar_t* pszPattern, int nPatternLen, bool bLoHiCase ) : 
#ifdef SEARCH_STRING_KMP
	m_pnNextPossArr(NULL),
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	m_pnUseCharSkipArr(NULL),
#endif
	m_pszPatternCase(NULL)
{
	m_pszPatternCase = new wchar_t[nPatternLen + 1];
	m_nPatternLen = nPatternLen;
	m_bIgnoreCase = !bLoHiCase; // 注: フラグが反対
	if( m_bIgnoreCase ){
		//note: 合成文字,サロゲートの「大文字小文字同一視」未対応
		for( int i = 0; i < m_nPatternLen; i++ ){
			m_pszPatternCase[i] = (wchar_t)skr_towupper(pszPattern[i]);
		}
	}else{
		wmemcpy( m_pszPatternCase, pszPattern, m_nPatternLen );
	}
	m_pszPatternCase[nPatternLen] = L'\0';
	
#ifdef SEARCH_STRING_KMP
	// "ABCDE" => {-1, 0, 0, 0, 0}
	// "AAAAA" => {-1, 0, 1, 2, 3}
	// "AABAA" => {-1, 0, 0, 0, 0}
	// "ABABA" => {-1, 0, 0, 2, 0}
//	if( m_bIgnoreCase ){
		m_pnNextPossArr = new int[nPatternLen + 1];
		int* next = m_pnNextPossArr;
		const wchar_t* key = m_pszPatternCase;
		for( int i = 0, k = -1; i < nPatternLen; ++i, ++k ){
			next[i] = k;
			while( -1 < k && key[i] != key[k] ){
				k = next[k];
			}
		}
//	}
#endif

#ifdef SEARCH_STRING_SUNDAY_QUICK
	const int BM_MAPSIZE = 0x1ff + 1;
	// 64KB も作らないで、ASCII それ以外(包括) の2つの情報のみ記録する
	// 「あ」と「乂」　「ぅ」と「居」は値を共有している
	m_pnUseCharSkipArr = new int[BM_MAPSIZE];
	for( int n = 0; n < BM_MAPSIZE; ++n ){
		m_pnUseCharSkipArr[n] = nPatternLen + 1;
	}
	for( int n = 0; n < nPatternLen; ++n ){
		const int index = GetMapIndex(m_pszPatternCase[n]);
		m_pnUseCharSkipArr[index] = nPatternLen - n;
	}
#endif
}


CSearchStringPattern::~CSearchStringPattern()
{
	delete [] m_pszPatternCase;
#ifdef SEARCH_STRING_KMP
	delete [] m_pnNextPossArr;
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	delete [] m_pnUseCharSkipArr;
#endif
}

#define toLoHiUpper(bLoHiCase, ch) (bLoHiCase? (ch) : skr_towupper(ch))

/*!
	文字列検索
	@return 見つかった場所のポインタ。見つからなかったらNULL。
*/
const wchar_t* CSearchAgent::SearchString(
	const wchar_t*	pLine,
	int				nLineLen,
	int				nIdxPos,
	const CSearchStringPattern& pattern
)
{
	const int      nPatternLen = pattern.GetLen();
	const wchar_t* pszPattern  = pattern.GetString();
#ifdef SEARCH_STRING_SUNDAY_QUICK
	const int* const useSkipMap = pattern.GetUseCharSkipMap();
#endif
	bool bLoHiCase = ! pattern.GetIgnoreCase();

	if( nLineLen < nPatternLen ){
		return NULL;
	}
	if( 0 >= nPatternLen || 0 >= nLineLen){
		return NULL;
	}

	// 線形探索
	const int nCompareTo = nLineLen - nPatternLen;	//	Mar. 4, 2001 genta

#ifdef SEARCH_STRING_KMP
	/* 大文字小文字を区別しない、かつ、検索語が5文字以下の場合は通常の検索を行う
	 * そうでない場合はKMP＋SUNDAY QUICKアルゴリズムを使った検索を行う */
	if ( bLoHiCase || nPatternLen > 5 ) {
		const wchar_t pattern0 = pszPattern[0];
		const int* const nextTable = pattern.GetKMPNextTable();
		for( int nPos = nIdxPos; nPos <= nCompareTo; ){
			if( toLoHiUpper(bLoHiCase, pLine[nPos]) != pattern0 ){
#ifdef SEARCH_STRING_SUNDAY_QUICK
				int index = CSearchStringPattern::GetMapIndex((wchar_t)toLoHiUpper( bLoHiCase, pLine[nPos + nPatternLen]) );
				nPos += useSkipMap[index];
#else
				nPos++;
#endif
				continue;
			}
			// 途中まで一致ならずらして継続(KMP)
			int i = 1;
			nPos++;
			while ( 0 < i ){
				while( i < nPatternLen && toLoHiUpper( bLoHiCase, pLine[nPos] ) == pszPattern[i] ){
					i++;
					nPos++;
				}
				if( i >= nPatternLen ){
					return &pLine[nPos - nPatternLen];
				}
				i = nextTable[i];
			}
			assert( 0 == i ); // -1チェック
		}
	} else {
#endif
		// 通常版
		int	nPos;
		for( nPos = nIdxPos; nPos <= nCompareTo; nPos += CNativeW::GetSizeOfChar(pLine, nLineLen, nPos) ){
			int n = bLoHiCase?
						wmemcmp( &pLine[nPos], pszPattern, nPatternLen ):
						wmemicmp( &pLine[nPos], pszPattern, nPatternLen );
			if( n == 0 ){
				return &pLine[nPos];
			}
		}
#ifdef SEARCH_STRING_KMP
	}
#endif
	return NULL;
}

/* 検索条件の情報(キー文字列の全角か半角かの配列)作成 */
void CSearchAgent::CreateCharCharsArr(
	const wchar_t*	pszPattern,
	int				nSrcLen,
	int**			ppnCharCharsArr
)
{
	int		i;
	int*	pnCharCharsArr;
	pnCharCharsArr = new int[nSrcLen];
	for( i = 0; i < nSrcLen; /*i++*/ ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		pnCharCharsArr[i] = CNativeW::GetSizeOfChar( pszPattern, nSrcLen, i );
		if( 0 == pnCharCharsArr[i] ){
			pnCharCharsArr[i] = 1;
		}
		if( 2 == pnCharCharsArr[i] ){
			pnCharCharsArr[i + 1] = pnCharCharsArr[i];
		}
		i+= pnCharCharsArr[i];
	}
	*ppnCharCharsArr = pnCharCharsArr;
	return;
}

/*!	単語単位の単語リスト作成
*/
void CSearchAgent::CreateWordList(
	std::vector<std::pair<const wchar_t*, CLogicInt> >&	searchWords,
	const wchar_t*	pszPattern,
	int	nPatternLen
)
{
	for( CLogicInt pos = CLogicInt(0); pos < nPatternLen; ) {
		CLogicInt begin, end; // 検索語に含まれる単語?の posを基準とした相対位置。WhereCurrentWord_2()の仕様では空白文字列も単語に含まれる。
		if( CWordParse::WhereCurrentWord_2( pszPattern + pos, nPatternLen - pos, CLogicInt(0), &begin, &end, NULL, NULL )
			&& begin == 0 && begin < end
		) {
			if( ! WCODE::IsWordDelimiter( pszPattern[pos] ) ) {
				// pszPattern[pos]...pszPattern[pos + end] が検索語に含まれる単語。
				searchWords.push_back( std::make_pair( pszPattern + pos, end ) );
			}
			pos += end;
		} else {
			pos += t_max( CLogicInt(1), CNativeW::GetSizeOfChar( pszPattern, nPatternLen, pos ) );
		}
	}
}


/*!	単語単位検索
*/
const wchar_t* CSearchAgent::SearchStringWord(
	const wchar_t*	pLine,
	int				nLineLen,
	int				nIdxPos,
	const std::vector<std::pair<const wchar_t*, CLogicInt> >& searchWords,
	bool	bLoHiCase,
	int*	pnMatchLen
)
{
	CLogicInt nNextWordFrom = CLogicInt(nIdxPos);
	CLogicInt nNextWordFrom2;
	CLogicInt nNextWordTo2;
	while( CWordParse::WhereCurrentWord_2( pLine, CLogicInt(nLineLen), nNextWordFrom, &nNextWordFrom2, &nNextWordTo2, NULL, NULL ) ){
		for( unsigned iSW = 0; iSW < searchWords.size(); ++iSW ) {
			if( searchWords[iSW].second == nNextWordTo2 - nNextWordFrom2 ){
				/* 1==大文字小文字の区別 */
				if( (!bLoHiCase && 0 == auto_memicmp( &(pLine[nNextWordFrom2]) , searchWords[iSW].first, searchWords[iSW].second ) ) ||
					(bLoHiCase && 0 == auto_memcmp( &(pLine[nNextWordFrom2]) , searchWords[iSW].first, searchWords[iSW].second ) )
				){
					*pnMatchLen = searchWords[iSW].second;
					return &pLine[nNextWordFrom2];
				}
			}
		}
		if( !CWordParse::SearchNextWordPosition( pLine, CLogicInt(nLineLen), nNextWordFrom, &nNextWordFrom, FALSE ) ){
			break;	//	次の単語が無い。
		}
	}
	*pnMatchLen = 0;
	return NULL;
}




/* 現在位置の単語の範囲を調べる */
// 2001/06/23 N.Nakatani WhereCurrentWord()変更 WhereCurrentWord_2をコールするようにした
bool CSearchAgent::WhereCurrentWord(
	CLogicInt	nLineNum,
	CLogicInt	nIdx,
	CLogicInt*	pnIdxFrom,
	CLogicInt*	pnIdxTo,
	CNativeW*	pcmcmWord,
	CNativeW*	pcmcmWordLeft
)
{
	*pnIdxFrom = nIdx;
	*pnIdxTo = nIdx;

	CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLineNum );
	if( NULL == pDocLine ){
		return false;
	}

	CLogicInt		nLineLen;
	const wchar_t*	pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );

	/* 現在位置の単語の範囲を調べる */
	return CWordParse::WhereCurrentWord_2( pLine, nLineLen, nIdx, pnIdxFrom, pnIdxTo, pcmcmWord, pcmcmWordLeft );
}




// 現在位置の左右の単語の先頭位置を調べる
bool CSearchAgent::PrevOrNextWord(
	CLogicInt	nLineNum,		//	行数
	CLogicInt	nIdx,			//	桁数
	CLogicInt*	pnColmNew,		//	見つかった位置
	BOOL		bLEFT,			//	TRUE:前方（左）へ向かう。FALSE:後方（右）へ向かう。
	BOOL		bStopsBothEnds	//	単語の両端で止まる
)
{
	using namespace WCODE;

	CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLineNum );
	if( NULL == pDocLine ){
		return false;
	}

	CLogicInt		nLineLen;
	const wchar_t*	pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );

	// ABC D[EOF]となっていたときに、Dの後ろにカーソルを合わせ、単語の左端に移動すると、Aにカーソルがあうバグ修正。YAZAKI
	if( nIdx >= nLineLen ){
		if (bLEFT && nIdx == nLineLen){
		}
		else {
			// 2011.12.26 EOFより右へ行こうとするときもfalseを返すように
			// nIdx = nLineLen - CLogicInt(1);
			return false;
		}
	}
	/* 現在位置の文字の種類によっては選択不能 */
	if( !bLEFT && ( pLine[nIdx] == CR || pLine[nIdx] == LF ) ){
		return false;
	}
	/* 前の単語か？後ろの単語か？ */
	if( bLEFT ){
		/* 現在位置の文字の種類を調べる */
		ECharKind	nCharKind = CWordParse::WhatKindOfChar( pLine, nLineLen, nIdx );
		if( nIdx == 0 ){
			return false;
		}

		/* 文字種類が変わるまで前方へサーチ */
		/* 空白とタブは無視する */
		int		nCount = 0;
		CLogicInt	nIdxNext = nIdx;
		CLogicInt	nCharChars = CLogicInt(&pLine[nIdxNext] - CNativeW::GetCharPrev( pLine, nLineLen, &pLine[nIdxNext] ));
		while( nCharChars > 0 ){
			CLogicInt		nIdxNextPrev = nIdxNext;
			nIdxNext -= nCharChars;
			ECharKind nCharKindNext = CWordParse::WhatKindOfChar( pLine, nLineLen, nIdxNext );

			ECharKind nCharKindMerge = CWordParse::WhatKindOfTwoChars( nCharKindNext, nCharKind );
			if( nCharKindMerge == CK_NULL ){
				/* サーチ開始位置の文字が空白またはタブの場合 */
				if( nCharKind == CK_TAB	|| nCharKind == CK_SPACE ){
					if ( bStopsBothEnds && nCount ){
						nIdxNext = nIdxNextPrev;
						break;
					}
					nCharKindMerge = nCharKindNext;
				}else{
					if( nCount == 0){
						nCharKindMerge = nCharKindNext;
					}else{
						nIdxNext = nIdxNextPrev;
						break;
					}
				}
			}
			nCharKind = nCharKindMerge;
			nCharChars = CLogicInt(&pLine[nIdxNext] - CNativeW::GetCharPrev( pLine, nLineLen, &pLine[nIdxNext] ));
			++nCount;
		}
		*pnColmNew = nIdxNext;
	}else{
		CWordParse::SearchNextWordPosition(pLine, nLineLen, nIdx, pnColmNew, bStopsBothEnds);
	}
	return true;
}




/*! 単語検索

	@date 2003.05.22 かろと 行頭処理など見直し
	@date 2005.11.26 かろと \rや.が\r\nにヒットしないように
*/
/* 見つからない場合は０を返す */
int CSearchAgent::SearchWord(
	CLogicPoint				ptSerachBegin,	//!< 検索開始位置
	const wchar_t*			pszPattern,		//!< 検索条件
	ESearchDirection		eDirection,		//!< 検索方向
	const SSearchOption&	sSearchOption,	//!< 検索オプション
	CLogicRange*			pMatchRange,	//!< [out] マッチ範囲。ロジック単位。
	CBregexp*				pRegexp			//!< [in]  正規表現コンパイルデータ。既にコンパイルされている必要がある
)
{
	CDocLine*	pDocLine;
	CLogicInt	nLinePos;
	CLogicInt	nIdxPos;
	CLogicInt	nIdxPosOld;
	const wchar_t*	pLine;
	int			nLineLen;
	const wchar_t*	pszRes;
	CLogicInt	nHitTo;
	CLogicInt	nHitPos;
	CLogicInt	nHitPosOld;
	int			nRetVal = 0;
	//	Jun. 10, 2003 Moca
	//	lstrlenを毎回呼ばずにnPatternLenを使うようにする
	const int	nPatternLen = wcslen( pszPattern );	//2001/06/23 N.Nakatani
#ifdef MEASURE_SEARCH_TIME
	long clockStart, clockEnd;
	clockStart = clock();
#endif

	//正規表現
	if( sSearchOption.bRegularExp ){
		nLinePos = ptSerachBegin.GetY2();		// 検索行＝検索開始行
		pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		//前方検索
		if( eDirection == SEARCH_BACKWARD ){
			//
			// 前方(↑)検索(正規表現)
			//
			nHitTo = ptSerachBegin.x;				// 検索開始位置
			nIdxPos = 0;
			while( NULL != pDocLine ){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				nHitPos		= -1;	// -1:この行でマッチ位置なし
				for (;;) {
					nHitPosOld = nHitPos;
					nIdxPosOld = nIdxPos;
					// 長さ０でマッチしたので、この位置で再度マッチしないように、１文字進める
					if (nIdxPos == nHitPos) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxPos += (CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxPos ) == 2 ? 2 : 1);
					}
					if (	nIdxPos <= pDocLine->GetLengthWithoutEOL() 
						&&	pRegexp->Match( pLine, nLineLen, nIdxPos ) ){
						// 検索にマッチした！
						nHitPos = pRegexp->GetIndex();
						nIdxPos = pRegexp->GetLastIndex();
						if( nHitPos >= nHitTo ){
							// マッチしたのは、カーソル位置以降だった
							// すでにマッチした位置があれば、それを返し、なければ前の行へ
							break;
						}
					} else {
						// マッチしなかった
						// すでにマッチした位置があれば、それを返し、なければ前の行へ
						break;
					}
				}

				if ( -1 != nHitPosOld ) {
					// この行でマッチした位置が存在するので、この行で検索終了
					pMatchRange->SetFromX( nHitPosOld );	// マッチ位置from
					pMatchRange->SetToX  ( nIdxPosOld );	// マッチ位置to
					break;
				} else {
					// この行でマッチした位置が存在しないので、前の行を検索へ
					nLinePos--;
					pDocLine = pDocLine->GetPrevLine();
					nIdxPos = 0;
					if( NULL != pDocLine ){
						nHitTo = pDocLine->GetLengthWithEOL() + 1;		// 前の行のNULL文字(\0)にもマッチさせるために+1 2003.05.16 かろと 
					}
				}
			}
		}
		//後方検索
		else {
			//
			// 後方検索(正規表現)
			//
			nIdxPos = ptSerachBegin.x;
			while( NULL != pDocLine ){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				if(		nIdxPos <= pDocLine->GetLengthWithoutEOL() 
					&&	pRegexp->Match( pLine, nLineLen, nIdxPos ) ){
					// マッチした
					pMatchRange->SetFromX( pRegexp->GetIndex()     );	// マッチ位置from
					pMatchRange->SetToX  ( pRegexp->GetLastIndex() );	// マッチ位置to
					break;
				}
				++nLinePos;
				pDocLine = pDocLine->GetNextLine();
				nIdxPos = 0;
			}
		}
		//
		// 正規表現検索の後処理
		if ( pDocLine != NULL ) {
			// マッチした行がある
			pMatchRange->SetFromY(nLinePos); // マッチ行
			pMatchRange->SetToY  (nLinePos); // マッチ行
			nRetVal = 1;
			// レイアウト行では改行文字内の位置を表現できないため、マッチ開始位置を補正
			if (pMatchRange->GetFrom().x > pDocLine->GetLengthWithoutEOL()) {
				// \r\n改行時に\nにマッチすると置換できない不具合となるため
				// 改行文字内でマッチした場合、改行文字の始めからマッチしたことにする
				pMatchRange->SetFromX( pDocLine->GetLengthWithoutEOL() );
			}
		}
	}
	//単語のみ検索
	else if( sSearchOption.bWordOnly ){
		// 検索語を単語に分割して searchWordsに格納する。
		std::vector<std::pair<const wchar_t*, CLogicInt> > searchWords; // 単語の開始位置と長さの配列。
		CreateWordList( searchWords, pszPattern, nPatternLen );
		/*
			2001/06/23 Norio Nakatani
			単語単位の検索を試験的に実装。単語はWhereCurrentWord()で判別してますので、
			英単語やC/C++識別子などの検索条件ならヒットします。
		*/

		// 前方検索
		if( eDirection == SEARCH_BACKWARD ){
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
			CLogicInt nNextWordFrom;
			CLogicInt nNextWordFrom2;
			CLogicInt nNextWordTo2;
			CLogicInt nWork;
			nNextWordFrom = ptSerachBegin.GetX2();
			while( NULL != pDocLine ){
				if( PrevOrNextWord( nLinePos, nNextWordFrom, &nWork, TRUE, FALSE ) ){
					nNextWordFrom = nWork;
					if( WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL ) ){
						for( unsigned iSW = 0; iSW < searchWords.size(); ++iSW ) {
							if( searchWords[iSW].second == nNextWordTo2 - nNextWordFrom2 ){
								const wchar_t* pData = pDocLine->GetPtr();	// 2002/2/10 aroka CMemory変更
								/* 1==大文字小文字の区別 */
								if( (!sSearchOption.bLoHiCase && 0 == auto_memicmp( &(pData[nNextWordFrom2]) , searchWords[iSW].first, searchWords[iSW].second ) ) ||
									(sSearchOption.bLoHiCase && 0 ==	 auto_memcmp( &(pData[nNextWordFrom2]) , searchWords[iSW].first, searchWords[iSW].second ) )
								){
									pMatchRange->SetFromY(nLinePos);	// マッチ行
									pMatchRange->SetToY  (nLinePos);	// マッチ行
									pMatchRange->SetFromX(nNextWordFrom2);						// マッチ位置from
									pMatchRange->SetToX  (pMatchRange->GetFrom().x + searchWords[iSW].second);// マッチ位置to
									nRetVal = 1;
									goto end_of_func;
								}
							}
						}
						continue;
					}
				}
				/* 前の行を見に行く */
				nLinePos--;
				pDocLine = pDocLine->GetPrevLine();
				if( NULL != pDocLine ){
					nNextWordFrom = pDocLine->GetLengthWithEOL() - pDocLine->GetEol().GetLen();
					if( 0 > nNextWordFrom ){
						nNextWordFrom = CLogicInt(0);
					}
				}
			}
		}
		// 後方検索
		else{
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
			CLogicInt	nNextWordFrom = ptSerachBegin.GetX2();
			while( NULL != pDocLine ){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				int nMatchLen;
				pszRes = SearchStringWord(pLine, nLineLen, nNextWordFrom, searchWords, sSearchOption.bLoHiCase, &nMatchLen);
				if( NULL != pszRes ){
					pMatchRange->SetFromY(nLinePos);	// マッチ行
					pMatchRange->SetToY  (nLinePos);	// マッチ行
					pMatchRange->SetFromX(CLogicInt(pszRes - pLine));						// マッチ位置from
					pMatchRange->SetToX  (pMatchRange->GetFrom().x + nMatchLen);// マッチ位置to
					nRetVal = 1;
					goto end_of_func;
				}
				/* 次の行を見に行く */
				nLinePos++;
				pDocLine = pDocLine->GetNextLine();
				nNextWordFrom = CLogicInt(0);
			}
		}

		nRetVal = 0;
		goto end_of_func;
	}
	//普通の検索 (正規表現でも単語単位でもない)
	else{
		const CSearchStringPattern pattern(pszPattern, nPatternLen, sSearchOption.bLoHiCase);
		//前方検索
		if( eDirection == SEARCH_BACKWARD ){
			nLinePos = ptSerachBegin.GetY2();
			nHitTo = ptSerachBegin.x;

			nIdxPos = 0;
			pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				nHitPos = -1;
				for (;;) {
					nHitPosOld = nHitPos;
					nIdxPosOld = nIdxPos;
					pszRes = SearchString(
						pLine,
						nLineLen,
						nIdxPos,
						pattern
					);
					if( NULL != pszRes ){
						nHitPos = pszRes - pLine;
						nIdxPos = nHitPos + nPatternLen;	// マッチ文字列長進めるように変更 2005.10.28 Karoto
						if( nHitPos >= nHitTo ){
							if( -1 != nHitPosOld ){
								pMatchRange->SetFromY(nLinePos);	// マッチ行
								pMatchRange->SetToY  (nLinePos);	// マッチ行
								pMatchRange->SetFromX(nHitPosOld);	// マッチ位置from
 								pMatchRange->SetToX  (nIdxPosOld);	// マッチ位置to
								nRetVal = 1;
								goto end_of_func;
							}else{
								break;
							}
						}
					}else{
						if( -1 != nHitPosOld ){
							pMatchRange->SetFromY(nLinePos);	// マッチ行
							pMatchRange->SetToY  (nLinePos);	// マッチ行
							pMatchRange->SetFromX(nHitPosOld);	// マッチ位置from
							pMatchRange->SetToX  (nIdxPosOld);	// マッチ位置to
							nRetVal = 1;
							goto end_of_func;
						}else{
							break;
						}
					}
				}
				nLinePos--;
				pDocLine = pDocLine->GetPrevLine();
				nIdxPos = 0;
				if( NULL != pDocLine ){
					nHitTo = pDocLine->GetLengthWithEOL();
				}
			}
			nRetVal = 0;
			goto end_of_func;
		}
		//後方検索
		else{
			nIdxPos = ptSerachBegin.x;
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				pszRes = SearchString(
					pLine,
					nLineLen,
					nIdxPos,
					pattern
				);
				if( NULL != pszRes ){
					pMatchRange->SetFromY(nLinePos);	// マッチ行
					pMatchRange->SetToY  (nLinePos);	// マッチ行
					pMatchRange->SetFromX(CLogicInt(pszRes - pLine));							// マッチ位置from (文字単位)
					pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);	// マッチ位置to   (文字単位)
					nRetVal = 1;
					goto end_of_func;
				}
				++nLinePos;
				pDocLine = pDocLine->GetNextLine();
				nIdxPos = 0;
			}
			nRetVal = 0;
			goto end_of_func;
		}
	}
end_of_func:;
#ifdef MEASURE_SEARCH_TIME
	clockEnd = clock();
	TCHAR buf[100];
	memset(buf, 0x00, sizeof(buf));
	wsprintf( buf, _T("%d"), clockEnd - clockStart);
	::MessageBox( NULL, buf, GSTR_APPNAME, MB_OK );
#endif

	return nRetVal;
}




/* 指定範囲のデータを置換(削除 & データを挿入)
  Fromを含む位置からToの直前を含むデータを削除する
  Fromの位置へテキストを挿入する
*/
void CSearchAgent::ReplaceData( DocLineReplaceArg* pArg )
{
//	MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::ReplaceData()" );

	/* 挿入によって増えた行の数 */
	pArg->nInsLineNum = CLogicInt(0);
	/* 削除した行の総数 */
	pArg->nDeletedLineNum = CLogicInt(0);
	/* 削除されたデータ */
	pArg->pcmemDeleted->SetString(L"");

	CDocLine* pCDocLine;
	CDocLine* pCDocLinePrev;
	CDocLine* pCDocLineNext;
	int nWorkPos;
	int nWorkLen;
	const wchar_t* pLine;
	int nLineLen;
	int i;
	int			nBgn;
	int			nPos;
	CLogicInt	nAllLinesOld;
	int			nCount;
	int			nProgress;
	CNativeW	cmemPrevLine;
	CNativeW	cmemCurLine;
	CNativeW	cmemNextLine;
	//	May 15, 2000
	CEol cEOLType;
	CEol cEOLTypeNext;
	HWND		hwndCancel = NULL;	//	初期化
	HWND		hwndProgress = NULL;	//	初期化

	pArg->ptNewPos = pArg->sDelRange.GetFrom();

	/* 大量のデータを操作するとき */
	CDlgCancel*	pCDlgCancel = NULL;
	if( 3000 < pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y || 1024000 < pArg->nInsDataLen){
		/* 進捗ダイアログの表示 */
		pCDlgCancel = new CDlgCancel;
		if( NULL != ( hwndCancel = pCDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
			Progress_SetRange( hwndProgress, 0, 100 );
 			Progress_SetPos( hwndProgress, 0 );
		}
	}

	// バッファを確保
	nWorkLen = 0;
	i = pArg->sDelRange.GetFrom().y;
	pCDocLine = m_pcDocLineMgr->GetLine( CLogicInt(i) );
	while( i <= pArg->sDelRange.GetTo().y && NULL != pCDocLine ){
		nWorkLen += pCDocLine->GetLengthWithEOL();
		++i;
		pCDocLine = m_pcDocLineMgr->GetLine( CLogicInt(i) );
	}
	pArg->pcmemDeleted->AllocStringBuffer( nWorkLen );

	// 削除データの取得のループ
	/* 前から処理していく */
	/* 現在行の情報を得る */
	pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetFrom().GetY2() );
	for( i = pArg->sDelRange.GetFrom().y; i <= pArg->sDelRange.GetTo().y && NULL != pCDocLine; i++ ){
		pLine = pCDocLine->GetPtr(); // 2002/2/10 aroka CMemory変更
		nLineLen = pCDocLine->GetLengthWithEOL(); // 2002/2/10 aroka CMemory変更
		pCDocLinePrev = pCDocLine->GetPrevLine();
		pCDocLineNext = pCDocLine->GetNextLine();
		/* 現在行の削除開始位置を調べる */
		if( i == pArg->sDelRange.GetFrom().y ){
			nWorkPos = pArg->sDelRange.GetFrom().x;
		}else{
			nWorkPos = 0;
		}
		/* 現在行の削除データ長を調べる */
		if( i == pArg->sDelRange.GetTo().y ){
			nWorkLen = pArg->sDelRange.GetTo().x - nWorkPos;
		}else{
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory変更
		}
		if( 0 == nWorkLen ){
			/* 前の行へ */
			goto next_line;
		}
		if( 0 > nWorkLen ){
			PleaseReportToAuthor(
				NULL,
				_T("CDocLineMgr::ReplaceData()\n")
				_T("\n")
				_T("0 > nWorkLen\nnWorkLen=%d\n")
				_T("i=%d\n")
				_T("pArg->sDelRange.GetTo().y=%d"),
				nWorkLen, i, pArg->sDelRange.GetTo().y
			);
		}

		/* 改行も削除するんかぃのぉ・・・？ */
		if( EOL_NONE != pCDocLine->GetEol() &&
			nWorkPos + nWorkLen > nLineLen - pCDocLine->GetEol().GetLen() // 2002/2/10 aroka CMemory変更
		){
			/* 削除する長さに改行も含める */
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory変更
		}
		/* 削除されたデータを保存 */
		// 2002/2/10 aroka from here CMemory変更 念のため。
		if( pLine != pCDocLine->GetPtr() ){
			PleaseReportToAuthor(
				NULL,
				_T("CDocLineMgr::ReplaceData()\n")
				_T("\n")
				_T("pLine != pCDocLine->m_cLine.GetPtr() =%d\n")
				_T("i=%d\n")
				_T("pArg->sDelRange.GetTo().y=%d"),
				pLine, i, pArg->sDelRange.GetTo().y
			);
		}

		if( pArg->pcmemDeleted->GetStringPtr() != NULL ){
			pArg->pcmemDeleted->AppendString( &pLine[nWorkPos], nWorkLen );
		}

next_line:;
		/* 次の行のオブジェクトのポインタ */
		pCDocLine = pCDocLineNext;
		/* 最近参照した行番号と行データ */
		++m_pcDocLineMgr->m_nPrevReferLine;
		m_pcDocLineMgr->m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (i - pArg->sDelRange.GetFrom().y) && ( 0 == ((i - pArg->sDelRange.GetFrom().y) % 32)) ){
				nProgress = (i - pArg->sDelRange.GetFrom().y) * 100 / (pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y) / 2;
				Progress_SetPos( hwndProgress, nProgress );

			}
		}
	} // 削除データの取得のループ


	/* 現在行の情報を得る */
	pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetTo().GetY2() );
	i = pArg->sDelRange.GetTo().y;
	if( 0 < pArg->sDelRange.GetTo().y && NULL == pCDocLine ){
		pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetTo().GetY2() - CLogicInt(1) );
		i--;
	}
	/* 後ろから処理していく */
	for( ; i >= pArg->sDelRange.GetFrom().y && NULL != pCDocLine; i-- ){
		pLine = pCDocLine->GetPtr(); // 2002/2/10 aroka CMemory変更
		nLineLen = pCDocLine->GetLengthWithEOL(); // 2002/2/10 aroka CMemory変更
		pCDocLinePrev = pCDocLine->GetPrevLine();
		pCDocLineNext = pCDocLine->GetNextLine();
		/* 現在行の削除開始位置を調べる */
		if( i == pArg->sDelRange.GetFrom().y ){
			nWorkPos = pArg->sDelRange.GetFrom().x;
		}else{
			nWorkPos = 0;
		}
		/* 現在行の削除データ長を調べる */
		if( i == pArg->sDelRange.GetTo().y ){
			nWorkLen = pArg->sDelRange.GetTo().x - nWorkPos;
		}else{
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory変更
		}

		if( 0 == nWorkLen ){
			/* 前の行へ */
			goto prev_line;
		}
		/* 改行も削除するんかぃのぉ・・・？ */
		if( EOL_NONE != pCDocLine->GetEol() &&
			nWorkPos + nWorkLen > nLineLen - pCDocLine->GetEol().GetLen() // 2002/2/10 aroka CMemory変更
		){
			/* 削除する長さに改行も含める */
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory変更
		}


		/* 行全体の削除 */
		if( nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory変更
			/* 削除した行の総数 */
			++(pArg->nDeletedLineNum);
			/* 行オブジェクトの削除、リスト変更、行数-- */
			m_pcDocLineMgr->DeleteLine( pCDocLine );
			pCDocLine = NULL;
		}
		/* 次の行と連結するような削除 */
		else if( nWorkPos + nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory変更

			/* 行内データ削除 */
			{// 20020119 aroka ブロック内に pWork を閉じ込めた
				wchar_t* pWork = new wchar_t[nWorkPos + 1];
				auto_memcpy( pWork, pLine, nWorkPos ); // 2002/2/10 aroka 何度も GetPtr を呼ばない
				pCDocLine->SetDocLineString( pWork, nWorkPos );
				delete [] pWork;
			}

			/* 次の行がある */
			if( pCDocLineNext ){
				/* 次の行のデータを最後に追加 */
				{
					CNativeW tmp = pCDocLine->_GetDocLineDataWithEOL() + pCDocLineNext->_GetDocLineDataWithEOL();
					pCDocLine->SetDocLineString(tmp);
				}

				/* 次の行 行オブジェクトの削除 */
				m_pcDocLineMgr->DeleteLine( pCDocLineNext );
				pCDocLineNext = NULL;

				/* 削除した行の総数 */
				++(pArg->nDeletedLineNum);
			}
			CModifyVisitor().SetLineModified(pCDocLine,true);	/* 変更フラグ */
		}
		else{
		/* 行内だけの削除 */
			{// 20020119 aroka ブロック内に pWork を閉じ込めた
				// 2002/2/10 aroka CMemory変更 何度も GetLength,GetPtr をよばない。
				int nLength = pCDocLine->GetLengthWithEOL();
				wchar_t* pWork = new wchar_t[nLength - nWorkLen + 1];
				wmemcpy( pWork, pLine, nWorkPos );

				wmemcpy( &pWork[nWorkPos], &pLine[nWorkPos + nWorkLen], nLength - ( nWorkPos + nWorkLen ) );

				pCDocLine->SetDocLineString( pWork, nLength - nWorkLen );
				delete [] pWork;
			}
			CModifyVisitor().SetLineModified(pCDocLine,true);	/* 変更フラグ */
		}

prev_line:;
		/* 直前の行のオブジェクトのポインタ */
		pCDocLine = pCDocLinePrev;
		/* 最近参照した行番号と行データ */
		--m_pcDocLineMgr->m_nPrevReferLine;
		m_pcDocLineMgr->m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (pArg->sDelRange.GetTo().y - i) && ( 0 == ((pArg->sDelRange.GetTo().y - i) % 32) ) ){
				nProgress = (pArg->sDelRange.GetTo().y - i) * 100 / (pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y) / 2 + 50;
				Progress_SetPos( hwndProgress, nProgress );
			}
		}
	}


	/* データ挿入処理 */
	if( 0 == pArg->nInsDataLen ){
		goto end_of_func;
	}
	nAllLinesOld = m_pcDocLineMgr->GetLineCount();
	pArg->ptNewPos.y = pArg->sDelRange.GetFrom().y;	/* 挿入された部分の次の位置の行 */
	pArg->ptNewPos.x = 0;	/* 挿入された部分の次の位置のデータ位置 */

	/* 挿入データを行終端で区切った行数カウンタ */
	nCount = 0;
	pArg->nInsLineNum = CLogicInt(0);
	pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetFrom().GetY2() );



	if( NULL == pCDocLine ){
		/* ここでNULLが帰ってくるということは、*/
		/* 全テキストの最後の次の行を追加しようとしていることを示す */
		cmemPrevLine.SetString(L"");
		cmemNextLine.SetString(L"");
		cEOLTypeNext.SetType( EOL_NONE );
	}else{
		CModifyVisitor().SetLineModified(pCDocLine,true);	/* 変更フラグ */

		// 2002/2/10 aroka 何度も GetPtr を呼ばない
		pLine = pCDocLine->GetDocLineStrWithEOL( &nLineLen );
		cmemPrevLine.SetString( pLine, pArg->sDelRange.GetFrom().x );
		cmemNextLine.SetString( &pLine[pArg->sDelRange.GetFrom().x], nLineLen - pArg->sDelRange.GetFrom().x );

		cEOLTypeNext = pCDocLine->GetEol();
	}
	nBgn = 0;
	for( nPos = 0; nPos < pArg->nInsDataLen; ){
		if( pArg->pInsData[nPos] == '\n' || pArg->pInsData[nPos] == '\r' ){
			/* 行終端子の種類を調べる */
			cEOLType.SetTypeByString( &(pArg->pInsData[nPos]), pArg->nInsDataLen - nPos );
			/* 行終端子も含めてテキストをバッファに格納 */
			cmemCurLine.SetString( &(pArg->pInsData[nBgn]), nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + cEOLType.GetLen();
			nPos = nBgn;
			if( NULL == pCDocLine){
				CDocLine* pCDocLineNew = m_pcDocLineMgr->AddNewLine();

				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					pCDocLineNew->SetDocLineString( cmemPrevLine + cmemCurLine );
				}
				else{
					pCDocLineNew->SetDocLineString( cmemCurLine );
				}
				pCDocLine = NULL;
			}
			else{
				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					pCDocLine->SetDocLineString( cmemPrevLine + cmemCurLine );
					pCDocLine = pCDocLine->GetNextLine();
				}
				else{
					CDocLine* pCDocLineNew = m_pcDocLineMgr->InsertNewLine(pCDocLine);	//pCDocLineの前に挿入
					pCDocLineNew->SetDocLineString( cmemCurLine );
				}
			}

			/* 挿入データを行終端で区切った行数カウンタ */
			++nCount;
			++(pArg->ptNewPos.y);	/* 挿入された部分の次の位置の行 */
			if( NULL != hwndCancel ){
				if( 0 != pArg->nInsDataLen && ( 0 == (nPos % 1024) ) ){
					nProgress = nPos * 100 / pArg->nInsDataLen;
					Progress_SetPos( hwndProgress, nProgress );
				}
			}

		}else{
			++nPos;
		}
	}
	if( 0 < nPos - nBgn || 0 < cmemNextLine.GetStringLength() ){
		cmemCurLine.SetString( &(pArg->pInsData[nBgn]), nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pCDocLine){
			CDocLine* pCDocLineNew = m_pcDocLineMgr->AddNewLine();	//末尾に追加
			/* 挿入データを行終端で区切った行数カウンタ */
			if( 0 == nCount ){
				pCDocLineNew->SetDocLineString( cmemPrevLine + cmemCurLine );
			}
			else{
				pCDocLineNew->SetDocLineString( cmemCurLine );
			}
			pCDocLine = NULL;
			pArg->ptNewPos.x = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
		}
		else{
			/* 挿入データを行終端で区切った行数カウンタ */
			if( 0 == nCount ){
				pCDocLine->SetDocLineString( cmemPrevLine + cmemCurLine );

				pCDocLine = pCDocLine->GetNextLine();
				pArg->ptNewPos.x = cmemPrevLine.GetStringLength() + nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
			}
			else{
				CDocLine* pCDocLineNew = m_pcDocLineMgr->InsertNewLine(pCDocLine);	//pCDocLineの前に挿入

				pCDocLineNew->SetDocLineString( cmemCurLine );

				pArg->ptNewPos.x = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
			}
		}
	}
	pArg->nInsLineNum = m_pcDocLineMgr->GetLineCount() - nAllLinesOld;
end_of_func:;
	if( NULL != pCDlgCancel ){
		// 進捗ダイアログを表示しない場合と同じ動きになるようにダイアログは遅延破棄する
		// ここで pCDlgCancel を delete すると delete から戻るまでの間に
		// ダイアログ破棄 -> 編集画面へフォーカス移動 -> キャレット位置調整
		// まで一気に動くので無効なレイアウト情報参照で異常終了することがある
		pCDlgCancel->DeleteAsync();	// 自動破棄を遅延実行する	// 2008.05.28 ryoji
	}
	return;
}
