/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSEARCHAGENT_9FAD6EE9_3D33_4D9F_9E9D_CAA9BE284140_H_
#define SAKURA_CSEARCHAGENT_9FAD6EE9_3D33_4D9F_9E9D_CAA9BE284140_H_
#pragma once

#include <optional>
#include <string>
#include <vector>
#include "_main/global.h"
#include "util/design_template.h"
#include "basis/SakuraBasis.h"

class CDocLineMgr;
struct DocLineReplaceArg;
class CBregexp;
class CNativeW;

// #define SEARCH_STRING_KMP
#define SEARCH_STRING_SUNDAY_QUICK

class CSearchStringPattern
{
public:
	CSearchStringPattern();
	void Reset();
	bool SetPattern(HWND hwnd, const wchar_t* pszPattern, size_t nPatternLen, const SSearchOption& sSearchOption, CBregexp* pRegexp){
		return SetPattern(hwnd, pszPattern, nPatternLen, nullptr, sSearchOption, pRegexp, false);
	}
	bool SetPattern(HWND, const wchar_t* pszPattern, size_t cchPattern, const wchar_t* pszPattern2, const SSearchOption& sSearchOption, CBregexp* pRegexp, bool bGlobal);
	const wchar_t* GetKey() const{ return m_strKey.c_str(); }
	//! 文字列検索で比べる検索語(大文字小文字を区別しないときは小文字にそろえたもの)
	const wchar_t* GetCaseKey() const{ return GetIgnoreCase() ? m_strCaseKey.c_str() : m_strKey.c_str(); }
	int GetLen() const{ return static_cast<int>(m_strKey.length()); }
	bool GetIgnoreCase() const{ return !m_sSearchOption.bLoHiCase; }
	bool GetLoHiCase() const{ return m_sSearchOption.bLoHiCase; }
	const SSearchOption& GetSearchOption() const{ return m_sSearchOption; }
	CBregexp* GetRegexp() const{ return m_pRegexp; }
	bool CompileRegexp( CBregexp& regexp ) const;
	//! 文字列検索の準備ができているか(正規表現・単語単位・設定の前は false)
	bool IsStringSearchReady() const{ return !m_skipMap.empty(); }
#ifdef SEARCH_STRING_KMP
	const int* GetKMPNextTable() const{ return m_nextPossArr.data(); }
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	const int* GetUseCharSkipMap() const{ return m_skipMap.data(); }

	static int GetMapIndex( wchar_t c );
#endif

private:
	int GetRegexpOption() const;
	const wchar_t* GetKey2() const;

	// SetPattern() に渡されたものの写し
	std::wstring	m_strKey;						//!< 検索語
	std::optional<std::wstring>	m_strKey2;			//!< 正規表現の置換後文字列(置換でなければ値なし)
	SSearchOption	m_sSearchOption;				//!< 検索オプション
	CBregexp* m_pRegexp = nullptr;					//!< 正規表現(持ち主は呼び出し側)
	bool	m_bGlobal = false;						//!< 正規表現の全域オプション(/g)

	// SetPattern() で作るもの
	std::wstring	m_strCaseKey;					//!< 小文字にそろえた検索語(大文字小文字を区別しない文字列検索のときだけ)
#ifdef SEARCH_STRING_KMP
	std::vector<int>	m_nextPossArr;				//!< KMP 法の表
#endif
	std::vector<int>	m_skipMap;					//!< 読み飛ばす幅の表(文字列検索のときだけ)

	DISALLOW_COPY_AND_ASSIGN(CSearchStringPattern);
};

class CSearchAgent{
public:
	// 文字列検索
	static const wchar_t* SearchString(
		const wchar_t*	pLine,
		int				nLineLen,
		int				nIdxPos,
		const CSearchStringPattern& pattern
	);
	// 単語単位で文字列検索
	static const wchar_t* SearchStringWord(
		const wchar_t*	pLine,
		int				nLineLen,
		int				nIdxPos,
		const std::vector<std::pair<const wchar_t*, CLogicInt> >& searchWords,
		bool	bLoHiCase,
		int*	pnMatchLen
	);

	// 検索条件の情報
	static void CreateCharCharsArr(
		const wchar_t*	pszPattern,
		int				nSrcLen,
		int**			ppnCharCharsArr
	);
	
	static void CreateWordList(
		std::vector<std::pair<const wchar_t*, CLogicInt> >&	searchWords,
		const wchar_t*	pszPattern,
		int	nPatternLen
	);

public:
	CSearchAgent(CDocLineMgr* pcDocLineMgr) : m_pcDocLineMgr(pcDocLineMgr) { }

	bool WhereCurrentWord( CLogicInt nLineNum, CLogicInt nIdx,
						   CLogicInt* pnIdxFrom, CLogicInt* pnIdxTo,
						   CNativeW* pcmcmWord, CNativeW* pcmcmWordLeft );	/* 現在位置の単語の範囲を調べる */

	bool PrevOrNextWord( CLogicInt nLineNum, CLogicInt nIdx, CLogicInt* pnColumnNew,
						 BOOL bLEFT, BOOL bStopsBothEnds );	/* 現在位置の左右の単語の先頭位置を調べる */
	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	int SearchWord( CLogicPoint ptSerachBegin, ESearchDirection eDirection, CLogicRange* pMatchRange, const CSearchStringPattern& pattern ); /* 単語検索 */

	void ReplaceData( DocLineReplaceArg* pArg, bool bEnableExtEol );
private:
	bool SearchWordBackwardByWord( CLogicPoint ptSerachBegin, CLogicRange* pMatchRange, const CSearchStringPattern& pattern );

	CDocLineMgr* m_pcDocLineMgr;
};

//! 1 行の中で見つかった一致
struct SLineMatch {
	int nIndex;		//!< 一致の始まり(行の先頭からの位置)
	int nLength;	//!< 一致の長さ(正規表現では 0 のこともある)
};

/*!
	@brief 1 行の中から、検索語に一致する部分を前から順に探す

	検索・ブックマークの一括設定・Grep・Grep 置換で使う。正規表現・単語単位・文字列のどれで探すかは、
	検索のオプションで決まる(正規表現・単語単位の順に優先する)。
	正規表現は CBregexp::Match() で探す。置換(CBregexp::Replace())はしない。
*/
class CLineMatchFinder {
public:
	/*!
		@param pattern [in] 検索の条件。設定済みであること。探し終わるまで残しておく
		@param pRegexp [in] 正規表現で探すときのコンパイル済みのデータ。探し終わるまで残しておく
	*/
	CLineMatchFinder( const CSearchStringPattern& pattern, CBregexp* pRegexp );

	//! nStart から後ろで最初の一致を探す。見つからなければ空
	std::optional<SLineMatch> Find( const wchar_t* pLine, int nLineLen, int nStart ) const;

	/*!
		Find() と同じ。ただし単語単位では、nStart を含む単語から調べる

		その単語が検索語の単語と同じなら、単語の先頭を一致として返す(nStart より前のこともある)。
		文書の中の検索で、カーソルが単語の途中にあるときに、その単語に一致させるため。
	*/
	std::optional<SLineMatch> FindIncludingWordAt( const wchar_t* pLine, int nLineLen, int nStart ) const;

	//! 一致の次に探し始める位置。長さ 0 の一致なら 1 文字進める
	static int GetNextStart( const wchar_t* pLine, int nLineLen, const SLineMatch& match );

private:
	const CSearchStringPattern* m_pPattern;
	CBregexp* m_pRegexp;
	std::vector<std::pair<const wchar_t*, CLogicInt> > m_searchWords;	//!< 単語単位で探すときの、検索語の単語
};
#endif /* SAKURA_CSEARCHAGENT_9FAD6EE9_3D33_4D9F_9E9D_CAA9BE284140_H_ */
