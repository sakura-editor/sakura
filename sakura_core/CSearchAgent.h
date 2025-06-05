/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSEARCHAGENT_9FAD6EE9_3D33_4D9F_9E9D_CAA9BE284140_H_
#define SAKURA_CSEARCHAGENT_9FAD6EE9_3D33_4D9F_9E9D_CAA9BE284140_H_
#pragma once

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
	~CSearchStringPattern();
	void Reset();
	bool SetPattern(HWND hwnd, const wchar_t* pszPattern, int nPatternLen, const SSearchOption& sSearchOption, CBregexp* pRegexp){
		return SetPattern(hwnd, pszPattern, nPatternLen, NULL, sSearchOption, pRegexp, false);
	}
	bool SetPattern(HWND, const wchar_t* pszPattern, int nPatternLen, const wchar_t* pszPattern2, const SSearchOption& sSearchOption, CBregexp* pRegexp, bool bGlobal);
	const wchar_t* GetKey() const{ return m_pszKey; }
	const wchar_t* GetCaseKey() const{ return m_pszCaseKeyRef; }
	int GetLen() const{ return m_nPatternLen; }
	bool GetIgnoreCase() const{ return !m_psSearchOption->bLoHiCase; }
	bool GetLoHiCase() const{ return m_psSearchOption->bLoHiCase; }
	const SSearchOption& GetSearchOption() const{ return *m_psSearchOption; }
	CBregexp* GetRegexp() const{ return m_pRegexp; }
#ifdef SEARCH_STRING_KMP
	const int* GetKMPNextTable() const{ return m_pnNextPossArr; }
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	const int* GetUseCharSkipMap() const{ return m_pnUseCharSkipArr; }

	static int GetMapIndex( wchar_t c );
#endif

private:
	// 外部依存
	const wchar_t*	m_pszKey = nullptr;
	const SSearchOption* m_psSearchOption = nullptr;
	mutable CBregexp* m_pRegexp = nullptr;

	const wchar_t* m_pszCaseKeyRef = nullptr;

	// 内部バッファ
	wchar_t* m_pszPatternCase = nullptr;
	int  m_nPatternLen;
#ifdef SEARCH_STRING_KMP
	int* m_pnNextPossArr = nullptr;
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	int* m_pnUseCharSkipArr = nullptr;
#endif

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
	CDocLineMgr* m_pcDocLineMgr;
};
#endif /* SAKURA_CSEARCHAGENT_9FAD6EE9_3D33_4D9F_9E9D_CAA9BE284140_H_ */
