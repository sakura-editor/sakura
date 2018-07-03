#include "StdAfx.h"

#include <vector>
#include <utility>
#include "CSearchAgent.h"
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"
#include "COpe.h"
#include "dlg/CDlgCancel.h"
#include "util/string_ex.h"
#include <algorithm>
#include "sakura_rc.h"
#include "CEditApp.h"
#include "CGrepAgent.h"

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

CSearchStringPattern::CSearchStringPattern() : 
	m_pszKey(NULL),
	m_psSearchOption(NULL),
	m_pRegexp(NULL),
	m_pszCaseKeyRef(NULL),
	m_pszPatternCase(NULL),
#ifdef SEARCH_STRING_KMP
	m_pnNextPossArr(NULL),
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	m_pnUseCharSkipArr(NULL)
#endif
{
}

	CSearchStringPattern::CSearchStringPattern(HWND hwnd, const wchar_t* pszPattern, int nPatternLen, const SSearchOption& sSearchOption, CBregexp* pRegexp) :
	m_pszKey(NULL),
	m_psSearchOption(NULL),
	m_pRegexp(NULL),
	m_pszCaseKeyRef(NULL),
	m_pszPatternCase(NULL),
#ifdef SEARCH_STRING_KMP
	m_pnNextPossArr(NULL),
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	m_pnUseCharSkipArr(NULL)
#endif
{
	SetPattern(hwnd, pszPattern, nPatternLen, sSearchOption, pRegexp);
}

CSearchStringPattern::~CSearchStringPattern()
{
	Reset();
}

void CSearchStringPattern::Reset(){
	m_pszKey = NULL;
	m_pszCaseKeyRef = NULL;
	m_psSearchOption = NULL;
	m_pRegexp = NULL;

	delete [] m_pszPatternCase;
	m_pszPatternCase = NULL;
#ifdef SEARCH_STRING_KMP
	delete [] m_pnNextPossArr;
	m_pnNextPossArr = NULL;
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	delete [] m_pnUseCharSkipArr;
	m_pnUseCharSkipArr = NULL;
#endif
}

bool CSearchStringPattern::SetPattern(HWND hwnd, const wchar_t* pszPattern, int nPatternLen, const wchar_t* pszPattern2, const SSearchOption& sSearchOption, CBregexp* regexp, bool bGlobal)
{
	Reset();
	m_pszCaseKeyRef = m_pszKey = pszPattern;
	m_nPatternLen = nPatternLen;
	m_psSearchOption = &sSearchOption;
	m_pRegexp = regexp;
	if( m_psSearchOption->bRegularExp ){
		if( !m_pRegexp ){
			return false;
		}
		if( !InitRegexp( hwnd, *m_pRegexp, true ) ){
			return false;
		}
		int nFlag = (GetLoHiCase() ? CBregexp::optCaseSensitive : CBregexp::optNothing);
		if( bGlobal ){
			nFlag |= CBregexp::optGlobal;
		}
		/* 検索パターンのコンパイル */
		if( pszPattern2 ){
			if( !m_pRegexp->Compile( pszPattern, pszPattern2, nFlag ) ){
				return false;
			}
		}else{
			if( !m_pRegexp->Compile( pszPattern, nFlag ) ){
				return false;
			}
		}
	}else if( m_psSearchOption->bWordOnly ){
	}else{
		if( GetIgnoreCase() ){
			m_pszPatternCase = new wchar_t[nPatternLen + 1];
			m_pszCaseKeyRef = m_pszPatternCase;
			//note: 合成文字,サロゲートの「大文字小文字同一視」未対応
			for( int i = 0; i < m_nPatternLen; i++ ){
				m_pszPatternCase[i] = (wchar_t)skr_towlower(pszPattern[i]);
			}
			m_pszPatternCase[nPatternLen] = L'\0';
		}

#ifdef SEARCH_STRING_KMP
	// "ABCDE" => {-1, 0, 0, 0, 0}
	// "AAAAA" => {-1, 0, 1, 2, 3}
	// "AABAA" => {-1, 0, 0, 0, 0}
	// "ABABA" => {-1, 0, 0, 2, 0}
//	if( GetIgnoreCase() ){
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
		const int BM_MAPSIZE = 0x200;
		// 64KB も作らないで、ISO-8859-1 それ以外(包括) の2つの情報のみ記録する
		// 「あ」と「乂」　「ぅ」と「居」は値を共有している
		m_pnUseCharSkipArr = new int[BM_MAPSIZE];
		for( int n = 0; n < BM_MAPSIZE; ++n ){
			m_pnUseCharSkipArr[n] = nPatternLen + 1;
		}
		for( int n = 0; n < nPatternLen; ++n ){
			const int index = GetMapIndex(m_pszCaseKeyRef[n]);
			m_pnUseCharSkipArr[index] = nPatternLen - n;
		}
#endif
	}
	return true;
}



#define toLoHiLower(bLoHiCase, ch) (bLoHiCase? (ch) : skr_towlower(ch))

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
	const wchar_t* pszPattern  = pattern.GetCaseKey();
#ifdef SEARCH_STRING_SUNDAY_QUICK
	const int* const useSkipMap = pattern.GetUseCharSkipMap();
#endif
	bool bLoHiCase = pattern.GetLoHiCase();

	if( nLineLen < nPatternLen ){
		return NULL;
	}
	if( 0 >= nPatternLen || 0 >= nLineLen){
		return NULL;
	}

	// 線形探索
	const int nCompareTo = nLineLen - nPatternLen;	//	Mar. 4, 2001 genta

#if defined(SEARCH_STRING_SUNDAY_QUICK) && !defined(SEARCH_STRING_KMP)
	// SUNDAY_QUICKのみ版
	if( !bLoHiCase || nPatternLen > 5 ){
		for( int nPos = nIdxPos; nPos <= nCompareTo;){
			int i;
			for( i = 0; i < nPatternLen && toLoHiLower(bLoHiCase, pLine[nPos + i]) == pszPattern[i]; i++ ){
			}
			if( i >= nPatternLen ){
				return &pLine[nPos];
			}
			int index = CSearchStringPattern::GetMapIndex((wchar_t)toLoHiLower(bLoHiCase, pLine[nPos + nPatternLen]));
			nPos += useSkipMap[index];
		}
	} else {
		for( int nPos = nIdxPos; nPos <= nCompareTo; ){
			int n = wmemcmp( &pLine[nPos], pszPattern, nPatternLen );
			if( n == 0 ){
				return &pLine[nPos];
			}
			int index = CSearchStringPattern::GetMapIndex(pLine[nPos + nPatternLen]);
			nPos += useSkipMap[index];
		}
	}
#else
#ifdef SEARCH_STRING_KMP
	/* 大文字小文字を区別しない、かつ、検索語が5文字以下の場合は通常の検索を行う
	 * そうでない場合はKMP＋SUNDAY QUICKアルゴリズムを使った検索を行う */
	if ( !bLoHiCase || nPatternLen > 5 ) {
		const wchar_t pattern0 = pszPattern[0];
		const int* const nextTable = pattern.GetKMPNextTable();
		for( int nPos = nIdxPos; nPos <= nCompareTo; ){
			if( toLoHiLower(bLoHiCase, pLine[nPos]) != pattern0 ){
#ifdef SEARCH_STRING_SUNDAY_QUICK
				int index = CSearchStringPattern::GetMapIndex((wchar_t)toLoHiLower( bLoHiCase, pLine[nPos + nPatternLen]) );
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
				while( i < nPatternLen && toLoHiLower( bLoHiCase, pLine[nPos] ) == pszPattern[i] ){
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
#endif // defined(SEARCH_STRING_) && !defined(SEARCH_STRING_KMP)
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
		size_t nSize = searchWords.size();
		for( size_t iSW = 0; iSW < nSize; ++iSW ) {
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

	const CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLineNum );
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
	CLogicInt*	pnColumnNew,	//	見つかった位置
	BOOL		bLEFT,			//	TRUE:前方（左）へ向かう。FALSE:後方（右）へ向かう。
	BOOL		bStopsBothEnds	//	単語の両端で止まる
)
{
	using namespace WCODE;

	const CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLineNum );
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
	if( !bLEFT && WCODE::IsLineDelimiter(pLine[nIdx], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
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
		*pnColumnNew = nIdxNext;
	}else{
		CWordParse::SearchNextWordPosition(pLine, nLineLen, nIdx, pnColumnNew, bStopsBothEnds);
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
	ESearchDirection		eDirection,		//!< 検索方向
	CLogicRange*			pMatchRange,	//!< [out] マッチ範囲。ロジック単位。
	const CSearchStringPattern&	pattern		//!< 検索パターン
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
	const SSearchOption&	sSearchOption = pattern.GetSearchOption();
	CBregexp*	pRegexp = pattern.GetRegexp();
#ifdef MEASURE_SEARCH_TIME
	long clockStart, clockEnd;
	clockStart = clock();
#endif

	//正規表現
	if( sSearchOption.bRegularExp ){
		nLinePos = ptSerachBegin.GetY2();		// 検索行＝検索開始行
		pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		// 後方検索
		if( eDirection == SEARCH_BACKWARD ){
			//
			// 後方(↑)検索(正規表現)
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
		// 前方検索
		else {
			//
			// 前方検索(正規表現)
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
		const wchar_t* pszPattern = pattern.GetKey();
		const int	nPatternLen = pattern.GetLen();
		std::vector<std::pair<const wchar_t*, CLogicInt> > searchWords; // 単語の開始位置と長さの配列。
		CreateWordList( searchWords, pszPattern, nPatternLen );
		/*
			2001/06/23 Norio Nakatani
			単語単位の検索を試験的に実装。単語はWhereCurrentWord()で判別してますので、
			英単語やC/C++識別子などの検索条件ならヒットします。
		*/

		// 後方検索
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
						size_t nSize = searchWords.size();
						for( size_t iSW = 0; iSW < nSize; ++iSW ) {
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
		// 前方検索
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
		const int	nPatternLen = pattern.GetLen();
		// 後方検索
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
		// 前方検索
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
	if( pArg->pcmemDeleted ){
		pArg->pcmemDeleted->clear();
	}

	CDocLine* pCDocLine;
	CDocLine* pCDocLinePrev;
	CDocLine* pCDocLineNext;
	int nWorkPos;
	int nWorkLen;
	const wchar_t* pLine;
	int nLineLen;
	int i;
	CLogicInt	nAllLinesOld;
	int			nProgress;
	CDocLine::MarkType	markNext;
	//	May 15, 2000
	HWND		hwndCancel = NULL;	//	初期化
	HWND		hwndProgress = NULL;	//	初期化

	pArg->ptNewPos = pArg->sDelRange.GetFrom();

	/* 大量のデータを操作するとき */
	CDlgCancel*	pCDlgCancel = NULL;
	class CDLgCandelCloser{
		CDlgCancel*& m_pDlg;
	public:
		CDLgCandelCloser(CDlgCancel*& pDlg): m_pDlg(pDlg){}
		~CDLgCandelCloser(){
			if( NULL != m_pDlg ){
				// 進捗ダイアログを表示しない場合と同じ動きになるようにダイアログは遅延破棄する
				// ここで pCDlgCancel を delete すると delete から戻るまでの間に
				// ダイアログ破棄 -> 編集画面へフォーカス移動 -> キャレット位置調整
				// まで一気に動くので無効なレイアウト情報参照で異常終了することがある
				m_pDlg->DeleteAsync();	// 自動破棄を遅延実行する	// 2008.05.28 ryoji
			}
		}
	};
	CDLgCandelCloser closer(pCDlgCancel);
	const CLogicInt nDelLines = pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y;
	const CLogicInt nEditLines = std::max<CLogicInt>(CLogicInt(1), nDelLines + CLogicInt(pArg->pInsData ? pArg->pInsData->size(): 0));
	if( !CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning ){
		if( 3000 < nEditLines ){
			/* 進捗ダイアログの表示 */
			pCDlgCancel = new CDlgCancel;
			if( NULL != ( hwndCancel = pCDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
				hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
				Progress_SetRange( hwndProgress, 0, 101 );
 				Progress_SetPos( hwndProgress, 0 );
			}
		}
	}
	int nProgressOld = 0;

	// バッファを確保
	if( pArg->pcmemDeleted ){
		pArg->pcmemDeleted->reserve( pArg->sDelRange.GetTo().y + CLogicInt(1) - pArg->sDelRange.GetFrom().y );
	}

	// 2012.01.10 行内の削除&挿入のときの操作を1つにする
	bool bChangeOneLine = false;	// 行内の挿入
	bool bInsOneLine = false;
	bool bLastEOLReplace = false;	// 「最後改行」を「最後改行」で置換
	if( pArg->pInsData && 0 < pArg->pInsData->size() ){
		const CNativeW& cmemLine = pArg->pInsData->back().cmemLine;
		int nLen = cmemLine.GetStringLength();
		const wchar_t* pInsLine = cmemLine.GetStringPtr();
		if( 0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
			// 行挿入
			bLastEOLReplace = true; // 仮。後で修正
		}else{
			if( 1 == pArg->pInsData->size() ){
				bChangeOneLine = true; // 「abc\ndef」=>「123」のような置換もtrueなのに注意
			}
		}
	}
	const wchar_t* pInsData = L"";
	int nInsLen = 0;
	int nSetSeq = 0;
	if( bChangeOneLine ){
		nInsLen = pArg->pInsData->back().cmemLine.GetStringLength();
		pInsData = pArg->pInsData->back().cmemLine.GetStringPtr();
		nSetSeq = pArg->pInsData->back().nSeq;
	}

	/* 現在行の情報を得る */
	pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetTo().GetY2() );
	i = pArg->sDelRange.GetTo().y;
	if( 0 < pArg->sDelRange.GetTo().y && NULL == pCDocLine ){
		pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetTo().GetY2() - CLogicInt(1) );
		i--;
	}
	bool bFirstLine = true;
	bool bSetMark = false;
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
			if( pArg->pcmemDeleted ){
				CLineData tmp;
				pArg->pcmemDeleted->push_back(tmp);
				CLineData& delLine = pArg->pcmemDeleted->back();
				delLine.cmemLine.swap(pCDocLine->_GetDocLineData()); // CDocLine書き換え
				delLine.nSeq = CModifyVisitor().GetLineModifiedSeq(pCDocLine);
			}
			m_pcDocLineMgr->DeleteLine( pCDocLine );
			pCDocLine = NULL;
		}
		/* 次の行と連結するような削除 */
		else if( nWorkPos + nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory変更
			if( pArg->pcmemDeleted ){
				if( pCDocLineNext && 0 == pArg->pcmemDeleted->size() ){
					// 1行以内の行末削除のときだけ、次の行のseqが保存されないので必要
					// 2014.01.07 最後が改行の範囲を最後が改行のデータで置換した場合を変更
					if( !bLastEOLReplace ){
						CLineData tmp;
						pArg->pcmemDeleted->push_back(tmp);
						CLineData& delLine =  pArg->pcmemDeleted->back();
						delLine.cmemLine.SetString(L"");
						delLine.nSeq = CModifyVisitor().GetLineModifiedSeq(pCDocLineNext);
					}
				}
				CLineData tmp;
				pArg->pcmemDeleted->push_back(tmp);
				CLineData& delLine = pArg->pcmemDeleted->back();
				delLine.cmemLine.SetString(&pLine[nWorkPos], nWorkLen);
				delLine.nSeq = CModifyVisitor().GetLineModifiedSeq(pCDocLine);
			}

			/* 次の行がある */
			if( pCDocLineNext ){
				/* 次の行のデータを最後に追加 */
				// 改行を削除するような置換
				int nNewLen = nWorkPos + pCDocLineNext->GetLengthWithEOL() + nInsLen;
				if( nWorkLen <= nWorkPos && nLineLen <= nNewLen + 10 ){
					// 行を連結して1行にするような操作の高速化
					// 削除が元データの有効長以下で行の長さが伸びるか少し減る場合reallocを試みる
					static CDocLine* pDocLinePrevAccess = NULL;
					static int nAccessCount = 0;
					int nBufferReserve = nNewLen;
					if( pDocLinePrevAccess == pCDocLine ){
						if( 100 < nAccessCount ){
							if( 1000 < nNewLen ){
								int n = 1000;
								while( n < nNewLen ){
									n += n / 5; // 20%づつ伸ばす
								}
								nBufferReserve = n;
							}
						}else{
							nAccessCount++;
						}
					}else{
						pDocLinePrevAccess = pCDocLine;
						nAccessCount = 0;
					}
					CNativeW& ref = pCDocLine->_GetDocLineData();
					ref.AllocStringBuffer(nBufferReserve);
					ref._SetStringLength(nWorkPos);
					ref.AppendString(pInsData, nInsLen);
					ref.AppendNativeData(pCDocLineNext->_GetDocLineDataWithEOL());
					pCDocLine->SetEol();
				}else{
					CNativeW tmp;
					tmp.AllocStringBuffer(nNewLen);
					tmp.AppendString(pLine, nWorkPos);
					tmp.AppendString(pInsData, nInsLen);
					tmp.AppendNativeData(pCDocLineNext->_GetDocLineDataWithEOL());
					pCDocLine->SetDocLineStringMove(&tmp);
				}
				if( bChangeOneLine ){
					pArg->nInsSeq = CModifyVisitor().GetLineModifiedSeq(pCDocLine);
					CModifyVisitor().SetLineModified(pCDocLine, nSetSeq);
					if( !bInsOneLine ){
						pArg->ptNewPos.x = pArg->ptNewPos.x + nInsLen;
						bInsOneLine = true;
					}
				}else{
					CModifyVisitor().SetLineModified(pCDocLine, pArg->nDelSeq);
					// 削除される行のマーク類を保存
					markNext = pCDocLineNext->m_sMark;
					bSetMark = true;
				}

				/* 次の行 行オブジェクトの削除 */
				m_pcDocLineMgr->DeleteLine( pCDocLineNext );
				pCDocLineNext = NULL;

				/* 削除した行の総数 */
				++(pArg->nDeletedLineNum);
			}else{
				/* 行内データ削除 */
				CNativeW tmp;
				tmp.SetString(pLine, nWorkPos);
				pCDocLine->SetDocLineStringMove(&tmp);
				CModifyVisitor().SetLineModified(pCDocLine, pArg->nDelSeq);	/* 変更フラグ */
			}
		}
		else{
			/* 行内だけの削除 */
			if( pArg->pcmemDeleted ){
				CLineData tmp;
				pArg->pcmemDeleted->push_back(tmp);
				CLineData& delLine =  pArg->pcmemDeleted->back();
				delLine.cmemLine.SetString(&pLine[nWorkPos], nWorkLen);
				delLine.nSeq = CModifyVisitor().GetLineModifiedSeq(pCDocLine);
			}
			{// 20020119 aroka ブロック内に pWork を閉じ込めた
				// 2002/2/10 aroka CMemory変更 何度も GetLength,GetPtr をよばない。
				int nNewLen = nLineLen - nWorkLen + nInsLen;
				int nAfterLen = nLineLen - (nWorkPos + nWorkLen);
				if( pCDocLine->_GetDocLineData().capacity() * 9 / 10 < nNewLen
					&& nNewLen <= pCDocLine->_GetDocLineData().capacity() ){
					CNativeW& ref = pCDocLine->_GetDocLineData();
					WCHAR* pBuf = const_cast<WCHAR*>(ref.GetStringPtr());
					if( nWorkLen != nInsLen ){
						wmemmove(&pBuf[nWorkPos + nInsLen], &pLine[nWorkPos + nWorkLen], nAfterLen);
					}
					wmemcpy(&pBuf[nWorkPos], pInsData, nInsLen);
					ref._SetStringLength(nNewLen);
				}else{
					int nBufferSize = 16;
					if( 1000 < nNewLen ){
						nBufferSize = 1000;
						while( nBufferSize < nNewLen ){
							nBufferSize += nBufferSize / 20; // 5%づつ伸ばす
						}
					}
					CNativeW tmp;
					tmp.AllocStringBuffer(nBufferSize);
					tmp.AppendString(pLine, nWorkPos);
					tmp.AppendString(pInsData, nInsLen);
					tmp.AppendString(&pLine[nWorkPos + nWorkLen], nAfterLen);
					pCDocLine->SetDocLineStringMove(&tmp);
				}
			}
			if( bChangeOneLine ){
				pArg->nInsSeq = CModifyVisitor().GetLineModifiedSeq(pCDocLine);
				CModifyVisitor().SetLineModified(pCDocLine, nSetSeq);
				pArg->ptNewPos.x = pArg->ptNewPos.x + nInsLen;
				bInsOneLine = true;
				pInsData = L"";
				nInsLen = 0;
			}else{
				CModifyVisitor().SetLineModified(pCDocLine, pArg->nDelSeq);
			}
			if( bFirstLine ){
				bLastEOLReplace = false;
			}
		}
		bFirstLine = false;

prev_line:;
		/* 直前の行のオブジェクトのポインタ */
		pCDocLine = pCDocLinePrev;
		/* 最近参照した行番号と行データ */
		--m_pcDocLineMgr->m_nPrevReferLine;
		m_pcDocLineMgr->m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			int nLines = pArg->sDelRange.GetTo().y - i;
			if( 0 == (nLines % 32) ){
				nProgress = ::MulDiv(nLines, 100, nEditLines);
				if( nProgressOld != nProgress ){
					nProgressOld = nProgress;
					Progress_SetPos( hwndProgress, nProgress + 1 );
					Progress_SetPos( hwndProgress, nProgress );
				}
			}
		}
	}

	if( pArg->pcmemDeleted ){
		// 下から格納されているのでひっくり返す
		std::reverse(pArg->pcmemDeleted->begin(), pArg->pcmemDeleted->end());
	}
	if( bInsOneLine ){
		// 挿入済み
		return;
	}

	/* データ挿入処理 */
	if( NULL == pArg->pInsData || 0 == pArg->pInsData->size() ){
		pArg->nInsSeq = 0;
		return;
	}
	nAllLinesOld= m_pcDocLineMgr->GetLineCount();
	pArg->ptNewPos.y = pArg->sDelRange.GetFrom().y;	/* 挿入された部分の次の位置の行 */
	pArg->ptNewPos.x = 0;	/* 挿入された部分の次の位置のデータ位置 */

	/* 挿入データを行終端で区切った行数カウンタ */
	pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetFrom().GetY2() );



	int nInsSize = pArg->pInsData->size();
	bool bInsertLineMode = false;
	bool bLastInsert = false;
	{
		CNativeW& cmemLine = pArg->pInsData->back().cmemLine;
		int nLen = cmemLine.GetStringLength();
		const wchar_t* pInsLine = cmemLine.GetStringPtr();
		if( 0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
			if( 0 == pArg->sDelRange.GetFrom().x ){
				// 挿入データの最後が改行で行頭に挿入するとき、現在行を維持する
				bInsertLineMode = true;
				if( pCDocLine && m_pcDocLineMgr->m_pCodePrevRefer == pCDocLine ){
					m_pcDocLineMgr->m_pCodePrevRefer = pCDocLine->GetPrevLine();
					if( m_pcDocLineMgr->m_pCodePrevRefer ){
						m_pcDocLineMgr->m_nPrevReferLine--;
					}
				}
			}
		}else{
			bLastInsert = true;
			nInsSize--;
		}
	}
	CStringRef	cPrevLine;
	CStringRef	cNextLine;
	CNativeW	cmemCurLine;
	if( NULL == pCDocLine ){
		/* ここでNULLが帰ってくるということは、*/
		/* 全テキストの最後の次の行を追加しようとしていることを示す */
		pArg->nInsSeq = 0;
	}else{
		// 2002/2/10 aroka 何度も GetPtr を呼ばない
		if( !bInsertLineMode ){
			cmemCurLine.swap(pCDocLine->_GetDocLineData());
			pLine = cmemCurLine.GetStringPtr(&nLineLen);
			cPrevLine = CStringRef(pLine, pArg->sDelRange.GetFrom().x);
			cNextLine = CStringRef(&pLine[pArg->sDelRange.GetFrom().x], nLineLen - pArg->sDelRange.GetFrom().x);
			pArg->nInsSeq = CModifyVisitor().GetLineModifiedSeq(pCDocLine);
		}else{
			pArg->nInsSeq = 0;
		}
	}
	int nCount;
	for( nCount = 0; nCount < nInsSize; nCount++ ){
		CNativeW& cmemLine = (*pArg->pInsData)[nCount].cmemLine;
#ifdef _DEBUG
		int nLen = cmemLine.GetStringLength();
		const wchar_t* pInsLine = cmemLine.GetStringPtr();
		assert( 0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) );
#endif
		{
			if( NULL == pCDocLine ){
				CDocLine* pCDocLineNew = m_pcDocLineMgr->AddNewLine();

				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					CNativeW tmp;
					tmp.AllocStringBuffer(cPrevLine.GetLength() + cmemLine.GetStringLength());
					tmp.AppendString(cPrevLine.GetPtr(), cPrevLine.GetLength());
					tmp.AppendNativeData(cmemLine);
					pCDocLineNew->SetDocLineStringMove(&tmp);
				}
				else{
					pCDocLineNew->SetDocLineStringMove(&cmemLine);
				}
				CModifyVisitor().SetLineModified(pCDocLineNew, (*pArg->pInsData)[nCount].nSeq);
			}
			else{
				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount && !bInsertLineMode ){
					if( cmemCurLine.GetStringLength() - cPrevLine.GetLength() < cmemCurLine.GetStringLength() / 100
						&& cPrevLine.GetLength() + cmemLine.GetStringLength() <= cmemCurLine.GetStringLength()
						&& cmemCurLine.capacity() / 2 <= cPrevLine.GetLength() + cmemLine.GetStringLength() ){
						// 行のうちNextになるのが1%以下で行が短くなるなら再利用する(長い一行を分割する場合の最適化)
						CNativeW tmp; // Nextを退避
						tmp.SetString(cNextLine.GetPtr(), cNextLine.GetLength());
						cmemCurLine.swap(tmp);
						tmp._SetStringLength(cPrevLine.GetLength());
						tmp.AppendNativeData(cmemLine);
						pCDocLine->SetDocLineStringMove(&tmp);
						cNextLine = CStringRef(cmemCurLine.GetStringPtr(), cmemCurLine.GetStringLength());
					}else{
						CNativeW tmp;
						tmp.AllocStringBuffer(cPrevLine.GetLength() + cmemLine.GetStringLength());
						tmp.AppendString(cPrevLine.GetPtr(), cPrevLine.GetLength());
						tmp.AppendNativeData(cmemLine);
						pCDocLine->SetDocLineStringMove(&tmp);
					}
					CModifyVisitor().SetLineModified(pCDocLine, (*pArg->pInsData)[nCount].nSeq);
					pCDocLine = pCDocLine->GetNextLine();
				}
				else{
					CDocLine* pCDocLineNew = m_pcDocLineMgr->InsertNewLine(pCDocLine);	//pCDocLineの前に挿入
					pCDocLineNew->SetDocLineStringMove(&cmemLine);
					CModifyVisitor().SetLineModified(pCDocLineNew, (*pArg->pInsData)[nCount].nSeq);
				}
			}

			/* 挿入データを行終端で区切った行数カウンタ */
			++(pArg->ptNewPos.y);	/* 挿入された部分の次の位置の行 */
			if( NULL != hwndCancel ){
				if( 0 == (nCount % 32) ){
					nProgress = ::MulDiv(nCount + nDelLines, 100, nEditLines);
					if( nProgressOld != nProgress ){
						nProgressOld = nProgress;
						Progress_SetPos( hwndProgress, nProgress + 1 );
						Progress_SetPos( hwndProgress, nProgress );
					}
				}
			}
		}
	}
	if( bLastInsert || 0 < cNextLine.GetLength() ){
		CNativeW cNull;
		CStringRef cNullStr(L"", 0);
		CNativeW& cmemLine = bLastInsert ? pArg->pInsData->back().cmemLine : cNull;
		const CStringRef& cPrevLine2 = ((0 == nCount) ? cPrevLine: cNullStr);
		int nSeq = pArg->pInsData->back().nSeq;
		int nLen = cmemLine.GetStringLength();
		CNativeW tmp;
		tmp.AllocStringBuffer(cPrevLine2.GetLength() + cmemLine.GetStringLength() + cNextLine.GetLength());
		tmp.AppendString(cPrevLine2.GetPtr(), cPrevLine2.GetLength());
		tmp.AppendNativeData(cmemLine);
		tmp.AppendString(cNextLine.GetPtr(), cNextLine.GetLength());
		if( NULL == pCDocLine ){
			CDocLine* pCDocLineNew = m_pcDocLineMgr->AddNewLine();	//末尾に追加
			pCDocLineNew->SetDocLineStringMove(&tmp);
			pCDocLineNew->m_sMark = markNext;
			if( !bLastEOLReplace || !bSetMark ){
				CModifyVisitor().SetLineModified(pCDocLineNew, nSeq);
			}
			pArg->ptNewPos.x = nLen;	/* 挿入された部分の次の位置のデータ位置 */
		}else{
			if( 0 == nCount ){
				// 行の中間に挿入(削除データがなかった。1文字入力など)
			}else{
				// 複数行挿入の最後の行
				pCDocLine = m_pcDocLineMgr->InsertNewLine(pCDocLine);	//pCDocLineの前に挿入
				pCDocLine->m_sMark = markNext;
			}
			pCDocLine->SetDocLineStringMove(&tmp);
			if( !bLastEOLReplace || !bSetMark ){
				CModifyVisitor().SetLineModified(pCDocLine, nSeq);
			}
			pArg->ptNewPos.x = cPrevLine2.GetLength() + nLen;	/* 挿入された部分の次の位置のデータ位置 */
		}
	}
	pArg->nInsLineNum = m_pcDocLineMgr->GetLineCount() - nAllLinesOld;
	return;
}
