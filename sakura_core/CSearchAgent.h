#pragma once

#include "global.h"

class CDocLineMgr;
struct DocLineReplaceArg;

class CSearchAgent{
public:
	// 文字列検索
	static const wchar_t* SearchString(
		const wchar_t*	pLine,
		int				nLineLen,
		int				nIdxPos,
		const wchar_t*	pszPattern,
		int				nPatternLen,
		int*			pnCharCharsArr,
		bool			bLoHiCase
	);
	
	// 検索条件の情報
	static void CreateCharCharsArr(
		const wchar_t*	pszPattern,
		int				nSrcLen,
		int**			ppnCharCharsArr
	);

public:
	CSearchAgent(CDocLineMgr* pcDocLineMgr) : m_pcDocLineMgr(pcDocLineMgr) { }

	bool WhereCurrentWord( CLogicInt , CLogicInt , CLogicInt* , CLogicInt*, CNativeW*, CNativeW* );	/* 現在位置の単語の範囲を調べる */

	bool PrevOrNextWord( CLogicInt , CLogicInt , CLogicInt* , BOOL bLEFT, BOOL bStopsBothEnds );	/* 現在位置の左右の単語の先頭位置を調べる */
	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	int SearchWord( CLogicPoint ptSerachBegin, const wchar_t* , ESearchDirection eDirection, const SSearchOption& sSearchOption , CLogicRange* pMatchRange, CBregexp* ); /* 単語検索 */

	void ReplaceData( DocLineReplaceArg* );
private:
	CDocLineMgr* m_pcDocLineMgr;
};
