/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"

#include <vector>
#include <utility>
#include "agent/CSearchAgent.h"
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"
#include "cmd/COpe.h"
#include "dlg/CDlgCancel.h"
#include "util/string_ex.h"
#include <algorithm>
#include <limits>
#include "sakura_rc.h"
#include "CEditApp.h"
#include "agent/CGrepAgent.h"
#include "apiwrap/CommonControl.h"
#include "env/DLLSHAREDATA.h"

// CSearchStringPattern
// @date 2010.06.22 Moca
inline int CSearchStringPattern::GetMapIndex( wchar_t c )
{
	// ASCII    => 0x000 - 0x0ff
	// それ以外 => 0x100 - 0x1ff
	return ((c & 0xff00) ? 0x100 : 0 ) | (c & 0xff);
}

CSearchStringPattern::CSearchStringPattern()
{
}

CSearchStringPattern::~CSearchStringPattern()
{
	Reset();
}

void CSearchStringPattern::Reset(){
	m_pszKey = nullptr;
	m_pszCaseKeyRef = nullptr;
	m_psSearchOption = nullptr;
	m_pRegexp = nullptr;

	delete [] m_pszPatternCase;
	m_pszPatternCase = nullptr;
#ifdef SEARCH_STRING_KMP
	delete [] m_pnNextPossArr;
	m_pnNextPossArr = nullptr;
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	delete [] m_pnUseCharSkipArr;
	m_pnUseCharSkipArr = nullptr;
#endif
}

bool CSearchStringPattern::SetPattern(
	HWND hwnd,
	const wchar_t* pszPattern,
	size_t cchPattern,
	const wchar_t* pszPattern2,
	const SSearchOption& sSearchOption,
	CBregexp* regexp,
	bool bGlobal
)
{
	auto nPatternLen = int(cchPattern);
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
		return nullptr;
	}
	if( 0 >= nPatternLen || 0 >= nLineLen){
		return nullptr;
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
	return nullptr;
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
		// 2026.06.23 CWE-787 fix: guard i+1 against nSrcLen. A pattern ending in a lone lead
		//            surrogate makes GetSizeOfChar return 2 at i==nSrcLen-1, so the write to
		//            pnCharCharsArr[i+1] (== [nSrcLen]) ran one int past the new int[nSrcLen].
		if( 2 == pnCharCharsArr[i] && i + 1 < nSrcLen ){
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
		if( CWordParse::WhereCurrentWord_2( pszPattern + pos, nPatternLen - pos, CLogicInt(0), GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol, &begin, &end, nullptr, nullptr )
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
	while( CWordParse::WhereCurrentWord_2( pLine, CLogicInt(nLineLen), nNextWordFrom, GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol, &nNextWordFrom2, &nNextWordTo2, nullptr, nullptr ) ){
		size_t nSize = searchWords.size();
		for( size_t iSW = 0; iSW < nSize; ++iSW ) {
			if( searchWords[iSW].second == nNextWordTo2 - nNextWordFrom2 ){
				/* 1==大文字小文字の区別 */
				if( (!bLoHiCase && 0 == wmemicmp( &(pLine[nNextWordFrom2]) , searchWords[iSW].first, searchWords[iSW].second ) ) ||
					(bLoHiCase && 0 == wmemcmp( &(pLine[nNextWordFrom2]) , searchWords[iSW].first, searchWords[iSW].second ) )
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
	return nullptr;
}

CLineMatchFinder::CLineMatchFinder( const CSearchStringPattern& pattern, CBregexp* pRegexp )
	: m_pPattern( &pattern )
	, m_pRegexp( pRegexp )
{
	if( pattern.GetSearchOption().bWordOnly ){
		CSearchAgent::CreateWordList( m_searchWords, pattern.GetKey(), pattern.GetLen() );
	}
}

std::optional<SLineMatch> CLineMatchFinder::Find( const wchar_t* pLine, int nLineLen, int nStart ) const
{
	auto match = FindIncludingWordAt( pLine, nLineLen, nStart );
	// 単語単位では、nStart を含む単語が nStart より前から始まることがある。その単語の後ろから探し直す
	while( match && match->nIndex < nStart ){
		match = FindIncludingWordAt( pLine, nLineLen, match->nIndex + match->nLength );
	}
	return match;
}

std::optional<SLineMatch> CLineMatchFinder::FindIncludingWordAt( const wchar_t* pLine, int nLineLen, int nStart ) const
{
	if( nLineLen < nStart ){
		return std::nullopt;
	}
	const SSearchOption& sSearchOption = m_pPattern->GetSearchOption();
	if( sSearchOption.bRegularExp ){
		// 2010.08.25 行頭以外で^にマッチする不具合の修正(行の先頭を渡し、探し始める位置を別に渡す)
		if( !m_pRegexp->Match( pLine, nLineLen, nStart ) ){
			return std::nullopt;
		}
		return SLineMatch{ m_pRegexp->GetIndex(), m_pRegexp->GetMatchLen() };
	}
	if( sSearchOption.bWordOnly ){
		int nMatchLen = 0;
		const wchar_t* pszRes = CSearchAgent::SearchStringWord( pLine, nLineLen, nStart, m_searchWords, sSearchOption.bLoHiCase, &nMatchLen );
		if( !pszRes ){
			return std::nullopt;
		}
		return SLineMatch{ static_cast<int>(pszRes - pLine), nMatchLen };
	}
	const wchar_t* pszRes = CSearchAgent::SearchString( pLine, nLineLen, nStart, *m_pPattern );
	if( !pszRes ){
		return std::nullopt;
	}
	return SLineMatch{ static_cast<int>(pszRes - pLine), m_pPattern->GetLen() };
}

int CLineMatchFinder::GetNextStart( const wchar_t* pLine, int nLineLen, const SLineMatch& match )
{
	if( 0 < match.nLength ){
		return match.nIndex + match.nLength;
	}
	// 2003.06.10 Moca 長さ 0 の一致は、同じ位置で何度も一致しないよう 1 文字進める
	const int nCharLen = CNativeW::GetSizeOfChar( pLine, nLineLen, match.nIndex );
	return match.nIndex + ( 0 < nCharLen ? nCharLen : 1 );
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
	if( nullptr == pDocLine ){
		return false;
	}

	CLogicInt		nLineLen;
	const wchar_t*	pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );

	/* 現在位置の単語の範囲を調べる */
	return CWordParse::WhereCurrentWord_2( pLine, nLineLen, nIdx, GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol, pnIdxFrom, pnIdxTo, pcmcmWord, pcmcmWordLeft );
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
	if( nullptr == pDocLine ){
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
		return CWordParse::SearchPrevWordPosition(pLine, nLineLen, nIdx, pnColumnNew, bStopsBothEnds);
	}
	CWordParse::SearchNextWordPosition(pLine, nLineLen, nIdx, pnColumnNew, bStopsBothEnds);
	return true;
}

namespace {

/*!
	1 行の中で、探し始めてよい最後の位置

	正規表現は、改行文字の中から探し始めない
*/
int GetLastSearchStart( const CDocLine& docLine, int nLineLen, bool bRegularExp )
{
	if( bRegularExp ){
		return docLine.GetLengthWithoutEOL();
	}
	return nLineLen;
}

//! 1 行の中で、nStart から後ろの最初の一致を探す
std::optional<SLineMatch> FindFirstInLine( const CLineMatchFinder& finder, const CDocLine& docLine, int nStart, bool bRegularExp )
{
	int nLineLen = 0;
	const wchar_t* pLine = docLine.GetDocLineStrWithEOL( &nLineLen );
	if( GetLastSearchStart( docLine, nLineLen, bRegularExp ) < nStart ){
		return std::nullopt;
	}
	// 単語単位では、カーソルが単語の途中にあれば、その単語にも一致させる
	return finder.FindIncludingWordAt( pLine, nLineLen, nStart );
}

//! 1 行の中で、始まりが nHitTo より前の一致のうち、最後のものを探す
std::optional<SLineMatch> FindLastInLineBefore( const CLineMatchFinder& finder, const CDocLine& docLine, int nHitTo, bool bRegularExp )
{
	int nLineLen = 0;
	const wchar_t* pLine = docLine.GetDocLineStrWithEOL( &nLineLen );
	const int nLastStart = GetLastSearchStart( docLine, nLineLen, bRegularExp );
	std::optional<SLineMatch> lastMatch;
	int nStart = 0;
	while( nStart <= nLastStart ){
		const auto match = finder.Find( pLine, nLineLen, nStart );
		if( !match || nHitTo <= match->nIndex ){
			break;
		}
		lastMatch = match;
		if( 0 == match->nLength ){
			// 長さ 0 の一致は、サロゲートペアなら 2、それ以外は 1 進める(異体字セレクタの前でも止まる)
			nStart = match->nIndex + ( CNativeW::GetSizeOfChar( pLine, nLineLen, match->nIndex ) == 2 ? 2 : 1 );
		}else{
			nStart = CLineMatchFinder::GetNextStart( pLine, nLineLen, *match );
		}
	}
	return lastMatch;
}

//! 単語が、検索語の単語のどれかと同じか
bool IsInWordList(
	const wchar_t*	pWord,
	int				nWordLen,
	const std::vector<std::pair<const wchar_t*, CLogicInt> >& searchWords,
	bool			bLoHiCase
)
{
	return std::any_of( searchWords.cbegin(), searchWords.cend(), [pWord, nWordLen, bLoHiCase]( const std::pair<const wchar_t*, CLogicInt>& searchWord ){
		if( searchWord.second != nWordLen ){
			return false;
		}
		/* 1==大文字小文字の区別 */
		if( bLoHiCase ){
			return 0 == wmemcmp( pWord, searchWord.first, nWordLen );
		}
		return 0 == wmemicmp( pWord, searchWord.first, nWordLen );
	} );
}

} // namespace

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
	const SSearchOption&	sSearchOption = pattern.GetSearchOption();
	const bool	bRegularExp = sSearchOption.bRegularExp;
	if( eDirection == SEARCH_BACKWARD && sSearchOption.bWordOnly && !bRegularExp ){
		return SearchWordBackwardByWord( ptSerachBegin, pMatchRange, pattern ) ? 1 : 0;
	}

	const CLineMatchFinder finder( pattern, pattern.GetRegexp() );
	CLogicInt	nLinePos = ptSerachBegin.GetY2();
	const CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
	std::optional<SLineMatch> match;
	if( eDirection == SEARCH_BACKWARD ){
		// 後方検索: 始まりがカーソルより前の一致のうち、最後のもの。無ければ前の行の最後の一致
		int nHitTo = ptSerachBegin.GetX2();
		while( nullptr != pDocLine ){
			match = FindLastInLineBefore( finder, *pDocLine, nHitTo, bRegularExp );
			if( match ){
				break;
			}
			--nLinePos;
			pDocLine = pDocLine->GetPrevLine();
			nHitTo = std::numeric_limits<int>::max();	// 前の行は、行末の長さ 0 の一致も含めて探す
		}
	}
	else{
		// 前方検索: 検索開始位置から後ろの最初の一致。無ければ次の行の最初の一致
		int nStart = ptSerachBegin.GetX2();
		while( nullptr != pDocLine ){
			match = FindFirstInLine( finder, *pDocLine, nStart, bRegularExp );
			if( match ){
				break;
			}
			++nLinePos;
			pDocLine = pDocLine->GetNextLine();
			nStart = 0;
		}
	}
	if( !match ){
		return 0;
	}

	pMatchRange->SetFromY( nLinePos );	// マッチ行
	pMatchRange->SetToY  ( nLinePos );	// マッチ行
	pMatchRange->SetFromX( CLogicInt(match->nIndex) );					// マッチ位置from
	pMatchRange->SetToX  ( CLogicInt(match->nIndex + match->nLength) );	// マッチ位置to
	// レイアウト行では改行文字内の位置を表現できないため、マッチ開始位置を補正
	if( bRegularExp && pDocLine->GetLengthWithoutEOL() < match->nIndex ){
		// \r\n改行時に\nにマッチすると置換できない不具合となるため
		// 改行文字内でマッチした場合、改行文字の始めからマッチしたことにする
		pMatchRange->SetFromX( pDocLine->GetLengthWithoutEOL() );
	}
	return 1;
}

/*!
	単語単位の後方検索

	カーソルの位置から単語を 1 つずつさかのぼり、検索語のどれかの単語と同じ単語を探す。
	行の先頭まで来たら、前の行の行末からさかのぼる。
	@retval true 見つかった。pMatchRange に一致の範囲を返す
*/
bool CSearchAgent::SearchWordBackwardByWord(
	CLogicPoint				ptSerachBegin,	//!< 検索開始位置
	CLogicRange*			pMatchRange,	//!< [out] マッチ範囲。ロジック単位。
	const CSearchStringPattern&	pattern		//!< 検索パターン
)
{
	// 検索語を単語に分割して searchWordsに格納する。
	std::vector<std::pair<const wchar_t*, CLogicInt> > searchWords; // 単語の開始位置と長さの配列。
	CreateWordList( searchWords, pattern.GetKey(), pattern.GetLen() );
	const bool bLoHiCase = pattern.GetSearchOption().bLoHiCase;

	CLogicInt	nLinePos = ptSerachBegin.GetY2();
	const CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
	CLogicInt	nNextWordFrom = ptSerachBegin.GetX2();
	CLogicInt	nWork(0);		// さかのぼった単語の先頭
	CLogicInt	nWordFrom(0);	// その単語の範囲の始まり
	CLogicInt	nWordTo(0);		// その単語の範囲の終わりの次
	while( nullptr != pDocLine ){
		if( PrevOrNextWord( nLinePos, nNextWordFrom, &nWork, TRUE, FALSE )
			&& WhereCurrentWord( nLinePos, nWork, &nWordFrom, &nWordTo, nullptr, nullptr )
		){
			nNextWordFrom = nWork;
			if( IsInWordList( &pDocLine->GetPtr()[nWordFrom], nWordTo - nWordFrom, searchWords, bLoHiCase ) ){
				pMatchRange->SetFromY( nLinePos );	// マッチ行
				pMatchRange->SetToY  ( nLinePos );	// マッチ行
				pMatchRange->SetFromX( nWordFrom );	// マッチ位置from
				pMatchRange->SetToX  ( nWordTo );	// マッチ位置to
				return true;
			}
			continue;
		}
		/* 前の行を見に行く */
		nLinePos--;
		pDocLine = pDocLine->GetPrevLine();
		if( nullptr != pDocLine ){
			nNextWordFrom = pDocLine->GetLengthWithoutEOL();
		}
	}
	return false;
}

/* 指定範囲のデータを置換(削除 & データを挿入)
  Fromを含む位置からToの直前を含むデータを削除する
  Fromの位置へテキストを挿入する
*/
void CSearchAgent::ReplaceData( DocLineReplaceArg* pArg, bool bEnableExtEol )
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
	HWND		hwndCancel = nullptr;	//	初期化
	HWND		hwndProgress = nullptr;	//	初期化

	pArg->ptNewPos = pArg->sDelRange.GetFrom();

	/* 大量のデータを操作するとき */
	CDlgCancel*	pCDlgCancel = nullptr;
	class CDLgCandelCloser{
		CDlgCancel*& m_pDlg;
	public:
		CDLgCandelCloser(CDlgCancel*& pDlg): m_pDlg(pDlg){}
		CDLgCandelCloser(const CDLgCandelCloser&) = delete;
		CDLgCandelCloser& operator = (const CDLgCandelCloser&) = delete;
		CDLgCandelCloser(CDLgCandelCloser&&) noexcept = delete;
		CDLgCandelCloser& operator = (CDLgCandelCloser&&) noexcept = delete;
		~CDLgCandelCloser(){
			if( nullptr != m_pDlg ){
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
	if( const CGrepAgent *pcGrepAgent = CEditApp::getInstance()->m_pcGrepAgent;
	    pcGrepAgent && !pcGrepAgent->m_bGrepRunning ){
		if( 3000 < nEditLines ){
			/* 進捗ダイアログの表示 */
			pCDlgCancel = new CDlgCancel;
			if( nullptr != ( hwndCancel = pCDlgCancel->DoModeless( ::GetModuleHandle( nullptr ), nullptr, IDD_OPERATIONRUNNING ) ) ){
				hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
				ApiWrap::Progress_SetRange( hwndProgress, 0, 101 );
				ApiWrap::Progress_SetPos( hwndProgress, 0 );
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
		if( 0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], bEnableExtEol) ){
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
	if( 0 < pArg->sDelRange.GetTo().y && nullptr == pCDocLine ){
		pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetTo().GetY2() - CLogicInt(1) );
		i--;
	}
	bool bFirstLine = true;
	bool bSetMark = false;
	/* 後ろから処理していく */
	for( ; i >= pArg->sDelRange.GetFrom().y && nullptr != pCDocLine; i-- ){
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
		if( pCDocLine->GetEol().IsValid() &&
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
				CLineData& delLine = pArg->pcmemDeleted->emplace_back(CLineData());
				delLine.cmemLine.swap(pCDocLine->_GetDocLineData()); // CDocLine書き換え
				delLine.nSeq = CModifyVisitor().GetLineModifiedSeq(pCDocLine);
			}
			m_pcDocLineMgr->DeleteLine( pCDocLine );
			pCDocLine = nullptr;
		}
		/* 次の行と連結するような削除 */
		else if( nWorkPos + nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory変更
			if( pArg->pcmemDeleted ){
				if( pCDocLineNext && 0 == pArg->pcmemDeleted->size() ){
					// 1行以内の行末削除のときだけ、次の行のseqが保存されないので必要
					// 2014.01.07 最後が改行の範囲を最後が改行のデータで置換した場合を変更
					if( !bLastEOLReplace ){
						CLineData& delLine = pArg->pcmemDeleted->emplace_back(CLineData());
						delLine.cmemLine.SetString(L"");
						delLine.nSeq = CModifyVisitor().GetLineModifiedSeq(pCDocLineNext);
					}
				}
				CLineData& delLine = pArg->pcmemDeleted->emplace_back(CLineData());
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
					static CDocLine* pDocLinePrevAccess = nullptr;
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
					pCDocLine->SetEol(bEnableExtEol);
				}else{
					CNativeW tmp;
					tmp.AllocStringBuffer(nNewLen);
					tmp.AppendString(pLine, nWorkPos);
					tmp.AppendString(pInsData, nInsLen);
					tmp.AppendNativeData(pCDocLineNext->_GetDocLineDataWithEOL());
					pCDocLine->SetDocLineStringMove(&tmp, bEnableExtEol);
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
				pCDocLineNext = nullptr;

				/* 削除した行の総数 */
				++(pArg->nDeletedLineNum);
			}else{
				/* 行内データ削除 */
				CNativeW tmp;
				tmp.SetString(pLine, nWorkPos);
				pCDocLine->SetDocLineStringMove(&tmp, bEnableExtEol);
				CModifyVisitor().SetLineModified(pCDocLine, pArg->nDelSeq);	/* 変更フラグ */
			}
		}
		else{
			/* 行内だけの削除 */
			if( pArg->pcmemDeleted ){
				CLineData& delLine = pArg->pcmemDeleted->emplace_back(CLineData());
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
					pCDocLine->SetDocLineStringMove(&tmp, bEnableExtEol);
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

		if( nullptr != hwndCancel){
			int nLines = pArg->sDelRange.GetTo().y - i;
			if( 0 == (nLines % 32) ){
				nProgress = ::MulDiv(nLines, 100, nEditLines);
				if( nProgressOld != nProgress ){
					nProgressOld = nProgress;
					ApiWrap::Progress_SetPos( hwndProgress, nProgress + 1 );
					ApiWrap::Progress_SetPos( hwndProgress, nProgress );
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
	if( nullptr == pArg->pInsData || 0 == pArg->pInsData->size() ){
		pArg->nInsSeq = 0;
		return;
	}
	nAllLinesOld= m_pcDocLineMgr->GetLineCount();
	pArg->ptNewPos.y = pArg->sDelRange.GetFrom().y;	/* 挿入された部分の次の位置の行 */
	pArg->ptNewPos.x = 0;	/* 挿入された部分の次の位置のデータ位置 */

	/* 挿入データを行終端で区切った行数カウンタ */
	pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetFrom().GetY2() );

	auto nInsSize = (int)pArg->pInsData->size();
	bool bInsertLineMode = false;
	bool bLastInsert = false;
	{
		CNativeW& cmemLine = pArg->pInsData->back().cmemLine;
		int nLen = cmemLine.GetStringLength();
		const wchar_t* pInsLine = cmemLine.GetStringPtr();
		if( 0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], bEnableExtEol) ){
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
	if( nullptr == pCDocLine ){
		/* ここでNULLが帰ってくるということは、*/
		/* 全テキストの最後の次の行を追加しようとしていることを示す */
		pArg->nInsSeq = 0;
	}else{
		// 2002/2/10 aroka 何度も GetPtr を呼ばない
		if( !bInsertLineMode ){
			cmemCurLine.swap(pCDocLine->_GetDocLineData());
			nLineLen = cmemCurLine.GetStringLength();
			pLine = cmemCurLine.GetStringPtr();
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
		assert( 0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], bEnableExtEol) );
#endif
		{
			if( nullptr == pCDocLine ){
				CDocLine* pCDocLineNew = m_pcDocLineMgr->AddNewLine();

				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					CNativeW tmp;
					tmp.AllocStringBuffer(cPrevLine.GetLength() + cmemLine.GetStringLength());
					tmp.AppendString(cPrevLine.GetPtr(), cPrevLine.GetLength());
					tmp.AppendNativeData(cmemLine);
					pCDocLineNew->SetDocLineStringMove(&tmp, bEnableExtEol);
				}
				else{
					pCDocLineNew->SetDocLineStringMove(&cmemLine, bEnableExtEol);
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
						pCDocLine->SetDocLineStringMove(&tmp, bEnableExtEol);
						cNextLine = CStringRef(cmemCurLine.GetStringPtr(), cmemCurLine.GetStringLength());
					}else{
						CNativeW tmp;
						tmp.AllocStringBuffer(cPrevLine.GetLength() + cmemLine.GetStringLength());
						tmp.AppendString(cPrevLine.GetPtr(), cPrevLine.GetLength());
						tmp.AppendNativeData(cmemLine);
						pCDocLine->SetDocLineStringMove(&tmp, bEnableExtEol);
					}
					CModifyVisitor().SetLineModified(pCDocLine, (*pArg->pInsData)[nCount].nSeq);
					pCDocLine = pCDocLine->GetNextLine();
				}
				else{
					CDocLine* pCDocLineNew = m_pcDocLineMgr->InsertNewLine(pCDocLine);	//pCDocLineの前に挿入
					pCDocLineNew->SetDocLineStringMove(&cmemLine, bEnableExtEol);
					CModifyVisitor().SetLineModified(pCDocLineNew, (*pArg->pInsData)[nCount].nSeq);
				}
			}

			/* 挿入データを行終端で区切った行数カウンタ */
			++(pArg->ptNewPos.y);	/* 挿入された部分の次の位置の行 */
			if( nullptr != hwndCancel ){
				if( 0 == (nCount % 32) ){
					nProgress = ::MulDiv(nCount + nDelLines, 100, nEditLines);
					if( nProgressOld != nProgress ){
						nProgressOld = nProgress;
						ApiWrap::Progress_SetPos( hwndProgress, nProgress + 1 );
						ApiWrap::Progress_SetPos( hwndProgress, nProgress );
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
		if( nullptr == pCDocLine ){
			CDocLine* pCDocLineNew = m_pcDocLineMgr->AddNewLine();	//末尾に追加
			pCDocLineNew->SetDocLineStringMove(&tmp, bEnableExtEol);
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
			pCDocLine->SetDocLineStringMove(&tmp, bEnableExtEol);
			if( !bLastEOLReplace || !bSetMark ){
				CModifyVisitor().SetLineModified(pCDocLine, nSeq);
			}
			pArg->ptNewPos.x = cPrevLine2.GetLength() + nLen;	/* 挿入された部分の次の位置のデータ位置 */
		}
	}
	pArg->nInsLineNum = m_pcDocLineMgr->GetLineCount() - nAllLinesOld;
	return;
}
