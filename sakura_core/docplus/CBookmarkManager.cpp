#include "StdAfx.h"
#include "docplus/CBookmarkManager.h"
#include "doc/CDocLineMgr.h"
#include "CSearchAgent.h"
#include "CBregexp.h"


bool CBookmarkGetter::IsBookmarked() const{ return m_pcDocLine->m_sMark.m_cBookmarked; }
void CBookmarkSetter::SetBookmark(bool bFlag){ m_pcDocLine->m_sMark.m_cBookmarked = bFlag; }

//!�u�b�N�}�[�N�̑S����
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


//! �u�b�N�}�[�N����
/*
	@date 2001.12.03 hor
*/
bool CBookmarkManager::SearchBookMark(
	CLogicInt			nLineNum,		/* �����J�n�s */
	ESearchDirection	bPrevOrNext,	/* 0==�O������ 1==������� */
	CLogicInt*			pnLineNum 		/* �}�b�`�s */
)
{
	CDocLine*	pDocLine;
	CLogicInt	nLinePos=nLineNum;

	//�O������
	if( bPrevOrNext == SEARCH_BACKWARD ){
		nLinePos--;
		pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine ){
			if(CBookmarkGetter(pDocLine).IsBookmarked()){
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return true;
			}
			nLinePos--;
			pDocLine = pDocLine->GetPrevLine();
		}
	}
	//�������
	else{
		nLinePos++;
		pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while( NULL != pDocLine ){
			if(CBookmarkGetter(pDocLine).IsBookmarked()){
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return true;
			}
			nLinePos++;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

//! �����s�ԍ��̃��X�g����܂Ƃ߂čs�}�[�N
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


//! �s�}�[�N����Ă镨���s�ԍ��̃��X�g�����
/*
	@date 2002.01.16 hor
*/
LPCWSTR CBookmarkManager::GetBookMarks()
{
	CDocLine*	pCDocLine;
	static wchar_t szText[MAX_MARKLINES_LEN + 1];	//2002.01.17 // Feb. 17, 2003 genta static��
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




//! ���������ɊY������s�Ƀu�b�N�}�[�N���Z�b�g����
/*
	@date 2002.01.16 hor
*/
void CBookmarkManager::MarkSearchWord(
	const wchar_t*			pszPattern,		//!< ��������
	const SSearchOption&	sSearchOption,	//!< �����I�v�V����
	CBregexp*				pRegexp			//!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
)
{
	CDocLine*	pDocLine;
	const wchar_t*	pLine;
	int			nLineLen;
	//	Jun. 10, 2003 Moca
	//	wcslen�𖈉�Ă΂���nPatternLen���g���悤�ɂ���
	const int	nPatternLen = wcslen( pszPattern );

	/* 1==���K�\�� */
	if( sSearchOption.bRegularExp ){
		pDocLine = m_pcDocLineMgr->GetLine( CLogicInt(0) );
		while( pDocLine ){
			if(!CBookmarkGetter(pDocLine).IsBookmarked()){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				// 2005.03.19 ����� �O����v�T�|�[�g�̂��߂̃��\�b�h�ύX
				if( pRegexp->Match( pLine, nLineLen, 0 ) ){
					CBookmarkSetter(pDocLine).SetBookmark(true);
				}
			}
			pDocLine = pDocLine->GetNextLine();
		}
	}
	/* 1==�P��̂݌��� */
	else if( sSearchOption.bWordOnly ){
		pDocLine = m_pcDocLineMgr->GetLine( CLogicInt(0) );
		CLogicInt nLinePos = CLogicInt(0);
		CLogicInt nNextWordFrom = CLogicInt(0);
		CLogicInt nNextWordFrom2;
		CLogicInt nNextWordTo2;
		while( pDocLine ){
			if(!CBookmarkGetter(pDocLine).IsBookmarked() &&
				CSearchAgent(m_pcDocLineMgr).WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL )) {
				const wchar_t* pData = pDocLine->GetPtr(); // 2002/2/10 aroka CMemory�ύX
				
				if(( nPatternLen == nNextWordTo2 - nNextWordFrom2 ) &&
				   (( !sSearchOption.bLoHiCase && 0 == auto_memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )) ||
					( sSearchOption.bLoHiCase && 0 == auto_memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )))){
					CBookmarkSetter(pDocLine).SetBookmark(true);
				}
				else if( CSearchAgent(m_pcDocLineMgr).PrevOrNextWord( nLinePos, nNextWordFrom, &nNextWordFrom, FALSE, FALSE) ){
					continue;
				}
			}
			/* ���̍s�����ɍs�� */
			nLinePos++;
			pDocLine = pDocLine->GetNextLine();
			nNextWordFrom = CLogicInt(0);
		}
	}
	else{
		/* ���������̏�� */
		const CSearchStringPattern pattern(pszPattern, nPatternLen, sSearchOption.bLoHiCase);
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

