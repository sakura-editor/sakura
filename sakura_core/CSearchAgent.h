#pragma once

#include "global.h"

class CDocLineMgr;
struct DocLineReplaceArg;

class CSearchAgent{
public:
	// �����񌟍�
	static const wchar_t* SearchString(
		const wchar_t*	pLine,
		int				nLineLen,
		int				nIdxPos,
		const wchar_t*	pszPattern,
		int				nPatternLen,
		int*			pnCharCharsArr,
		bool			bLoHiCase
	);
	
	// ���������̏��
	static void CreateCharCharsArr(
		const wchar_t*	pszPattern,
		int				nSrcLen,
		int**			ppnCharCharsArr
	);

public:
	CSearchAgent(CDocLineMgr* pcDocLineMgr) : m_pcDocLineMgr(pcDocLineMgr) { }

	bool WhereCurrentWord( CLogicInt , CLogicInt , CLogicInt* , CLogicInt*, CNativeW*, CNativeW* );	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */

	bool PrevOrNextWord( CLogicInt , CLogicInt , CLogicInt* , BOOL bLEFT, BOOL bStopsBothEnds );	/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	int SearchWord( CLogicPoint ptSerachBegin, const wchar_t* , ESearchDirection eDirection, const SSearchOption& sSearchOption , CLogicRange* pMatchRange, CBregexp* ); /* �P�ꌟ�� */

	void ReplaceData( DocLineReplaceArg* );
private:
	CDocLineMgr* m_pcDocLineMgr;
};
