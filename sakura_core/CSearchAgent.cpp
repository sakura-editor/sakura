#include "StdAfx.h"
#include "CBregexp.h" // �w�b�_�ˑ� CSearchAgent

#include "CSearchAgent.h"
#include "doc/CDocLineMgr.h"
#include "dlg/CDlgCancel.h"
#include "util/string_ex.h"
#include "sakura_rc.h"

/*!
	�����񌟍�
	@return ���������ꏊ�̃|�C���^�B������Ȃ�������NULL�B
*/
const wchar_t* CSearchAgent::SearchString(
	const wchar_t*	pLine,
	int				nLineLen,
	int				nIdxPos,
	const wchar_t*	pszPattern,
	int				nPatternLen,
	int*			pnCharCharsArr,
	bool			bLoHiCase
)
{
	if( nLineLen < nPatternLen ){
		return NULL;
	}
	if( 0 >= nPatternLen || 0 >= nLineLen){
		return NULL;
	}

	int	nPos;
	int	nCompareTo;

	// ���`�T��
	nCompareTo = nLineLen - nPatternLen;	//	Mar. 4, 2001 genta

	for( nPos = nIdxPos; nPos <= nCompareTo; nPos += CNativeW::GetSizeOfChar(pLine, nLineLen, nPos) ){
		int n = bLoHiCase?
					wmemcmp( &pLine[nPos], pszPattern, nPatternLen ):
					wmemicmp( &pLine[nPos], pszPattern, nPatternLen );
		if( n == 0 ){
			return &pLine[nPos];
		}
	}
	return NULL;
}

/* ���������̏��(�L�[������̑S�p�����p���̔z��)�쐬 */
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





/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
// 2001/06/23 N.Nakatani WhereCurrentWord()�ύX WhereCurrentWord_2���R�[������悤�ɂ���
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

	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	return CWordParse::WhereCurrentWord_2( pLine, nLineLen, nIdx, pnIdxFrom, pnIdxTo, pcmcmWord, pcmcmWordLeft );
}




// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
bool CSearchAgent::PrevOrNextWord(
	CLogicInt	nLineNum,		//	�s��
	CLogicInt	nIdx,			//	����
	CLogicInt*	pnColmNew,		//	���������ʒu
	BOOL		bLEFT,			//	TRUE:�O���i���j�֌������BFALSE:����i�E�j�֌������B
	BOOL		bStopsBothEnds	//	�P��̗��[�Ŏ~�܂�
)
{
	using namespace WCODE;

	CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLineNum );
	if( NULL == pDocLine ){
		return false;
	}

	CLogicInt		nLineLen;
	const wchar_t*	pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );

	// ABC D[EOF]�ƂȂ��Ă����Ƃ��ɁAD�̌��ɃJ�[�\�������킹�A�P��̍��[�Ɉړ�����ƁAA�ɃJ�[�\���������o�O�C���BYAZAKI
	if( nIdx >= nLineLen ){
		if (bLEFT && nIdx == nLineLen){
		}
		else {
			nIdx = nLineLen - CLogicInt(1);
		}
	}
	/* ���݈ʒu�̕����̎�ނɂ���Ă͑I��s�\ */
	if( !bLEFT && ( pLine[nIdx] == CR || pLine[nIdx] == LF ) ){
		return false;
	}
	/* �O�̒P�ꂩ�H���̒P�ꂩ�H */
	if( bLEFT ){
		/* ���݈ʒu�̕����̎�ނ𒲂ׂ� */
		ECharKind	nCharKind = CWordParse::WhatKindOfChar( pLine, nLineLen, nIdx );
		if( nIdx == 0 ){
			return false;
		}

		/* ������ނ��ς��܂őO���փT�[�` */
		/* �󔒂ƃ^�u�͖������� */
		int		nCount = 0;
		CLogicInt	nIdxNext = nIdx;
		CLogicInt	nCharChars = CLogicInt(&pLine[nIdxNext] - CNativeW::GetCharPrev( pLine, nLineLen, &pLine[nIdxNext] ));
		while( nCharChars > 0 ){
			CLogicInt		nIdxNextPrev = nIdxNext;
			nIdxNext -= nCharChars;
			ECharKind nCharKindNext = CWordParse::WhatKindOfChar( pLine, nLineLen, nIdxNext );

			ECharKind nCharKindMerge = CWordParse::WhatKindOfTwoChars( nCharKindNext, nCharKind );
			if( nCharKindMerge == CK_NULL ){
				/* �T�[�`�J�n�ʒu�̕������󔒂܂��̓^�u�̏ꍇ */
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




/*! �P�ꌟ��

	@date 2003.05.22 ����� �s�������Ȃǌ�����
	@date 2005.11.26 ����� \r��.��\r\n�Ƀq�b�g���Ȃ��悤��
*/
/* ������Ȃ��ꍇ�͂O��Ԃ� */
int CSearchAgent::SearchWord(
	CLogicPoint				ptSerachBegin,	//!< �����J�n�ʒu
	const wchar_t*			pszPattern,		//!< ��������
	ESearchDirection		eDirection,		//!< ��������
	const SSearchOption&	sSearchOption,	//!< �����I�v�V����
	CLogicRange*			pMatchRange,	//!< [out] �}�b�`�͈́B���W�b�N�P�ʁB
	CBregexp*				pRegexp			//!< [in]  ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������ */
)
{
	CDocLine*	pDocLine;
	CLogicInt	nLinePos;
	CLogicInt	nIdxPos;
	CLogicInt	nIdxPosOld;
	const wchar_t*	pLine;
	int			nLineLen;
	const wchar_t*	pszRes;
	int			nHitTo;
	int			nHitPos;
	int			nHitPosOld;
	int			nRetVal = 0;
	//	Jun. 10, 2003 Moca
	//	lstrlen�𖈉�Ă΂���nPatternLen���g���悤�ɂ���
	const int	nPatternLen = wcslen( pszPattern );	//2001/06/23 N.Nakatani

	// ���������̏�� -> pnKey_CharCharsArr
	int* pnKey_CharCharsArr = NULL;
	CSearchAgent::CreateCharCharsArr(
		pszPattern,
		nPatternLen,
		&pnKey_CharCharsArr
	);

	//���K�\��
	if( sSearchOption.bRegularExp ){
		nLinePos = ptSerachBegin.GetY2();		// �����s�������J�n�s
		pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		//�O������
		if( eDirection == SEARCH_BACKWARD ){
			//
			// �O��(��)����(���K�\��)
			//
			nHitTo = ptSerachBegin.x;				// �����J�n�ʒu
			nIdxPos = 0;
			while( NULL != pDocLine ){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				nHitPos		= -1;	// -1:���̍s�Ń}�b�`�ʒu�Ȃ�
				while( 1 ){
					nHitPosOld = nHitPos;
					nIdxPosOld = nIdxPos;
					// �����O�Ń}�b�`�����̂ŁA���̈ʒu�ōēx�}�b�`���Ȃ��悤�ɁA�P�����i�߂�
					if (nIdxPos == nHitPos) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxPos += (CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxPos ) == 2 ? 2 : 1);
					}
					if (	nIdxPos <= pDocLine->GetLengthWithoutEOL() 
						&&	pRegexp->Match( pLine, nLineLen, nIdxPos ) ){
						// �����Ƀ}�b�`�����I
						nHitPos = pRegexp->GetIndex();
						nIdxPos = pRegexp->GetLastIndex();
						if( nHitPos >= nHitTo ){
							// �}�b�`�����̂́A�J�[�\���ʒu�ȍ~������
							// ���łɃ}�b�`�����ʒu������΁A�����Ԃ��A�Ȃ���ΑO�̍s��
							break;
						}
					} else {
						// �}�b�`���Ȃ�����
						// ���łɃ}�b�`�����ʒu������΁A�����Ԃ��A�Ȃ���ΑO�̍s��
						break;
					}
				}

				if ( -1 != nHitPosOld ) {
					// ���̍s�Ń}�b�`�����ʒu�����݂���̂ŁA���̍s�Ō����I��
					pMatchRange->SetFromX( nHitPosOld );	// �}�b�`�ʒufrom
					pMatchRange->SetToX  ( nIdxPosOld );	// �}�b�`�ʒuto
					break;
				} else {
					// ���̍s�Ń}�b�`�����ʒu�����݂��Ȃ��̂ŁA�O�̍s��������
					nLinePos--;
					pDocLine = pDocLine->GetPrevLine();
					nIdxPos = 0;
					if( NULL != pDocLine ){
						nHitTo = pDocLine->GetLengthWithEOL() + 1;		// �O�̍s��NULL����(\0)�ɂ��}�b�`�����邽�߂�+1 2003.05.16 ����� 
					}
				}
			}
		}
		//�������
		else {
			//
			// �������(���K�\��)
			//
			nIdxPos = ptSerachBegin.x;
			while( NULL != pDocLine ){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				if(		nIdxPos <= pDocLine->GetLengthWithoutEOL() 
					&&	pRegexp->Match( pLine, nLineLen, nIdxPos ) ){
					// �}�b�`����
					pMatchRange->SetFromX( pRegexp->GetIndex()     );	// �}�b�`�ʒufrom
					pMatchRange->SetToX  ( pRegexp->GetLastIndex() );	// �}�b�`�ʒuto
					break;
				}
				++nLinePos;
				pDocLine = pDocLine->GetNextLine();
				nIdxPos = 0;
			}
		}
		//
		// ���K�\�������̌㏈��
		if ( pDocLine != NULL ) {
			// �}�b�`�����s������
			pMatchRange->SetFromY(nLinePos); // �}�b�`�s
			pMatchRange->SetToY  (nLinePos); // �}�b�`�s
			nRetVal = 1;
			// ���C�A�E�g�s�ł͉��s�������̈ʒu��\���ł��Ȃ����߁A�}�b�`�J�n�ʒu��␳
			if (pMatchRange->GetFrom().x > pDocLine->GetLengthWithoutEOL()) {
				// \r\n���s����\n�Ƀ}�b�`����ƒu���ł��Ȃ��s��ƂȂ邽��
				// ���s�������Ń}�b�`�����ꍇ�A���s�����̎n�߂���}�b�`�������Ƃɂ���
				pMatchRange->SetFromX( pDocLine->GetLengthWithoutEOL() );
			}
		}
	}
	//�P��̂݌���
	else if( sSearchOption.bWordOnly ){
		/*
			2001/06/23 Norio Nakatani
			�P��P�ʂ̌����������I�Ɏ����B�P���WhereCurrentWord()�Ŕ��ʂ��Ă܂��̂ŁA
			�p�P���C/C++���ʎq�Ȃǂ̌��������Ȃ�q�b�g���܂��B
		*/

		// �O������
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
						if( nPatternLen == nNextWordTo2 - nNextWordFrom2 ){
							const wchar_t* pData = pDocLine->GetPtr();	// 2002/2/10 aroka CMemory�ύX
							/* 1==�啶���������̋�� */
							if( (!sSearchOption.bLoHiCase && 0 == auto_memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) ) ||
								(sSearchOption.bLoHiCase && 0 ==	 auto_memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) )
							){
								pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
								pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
								pMatchRange->SetFromX(nNextWordFrom2);						// �}�b�`�ʒufrom
								pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);// �}�b�`�ʒuto
								nRetVal = 1;
								goto end_of_func;
							}
						}
						continue;
					}
				}
				/* �O�̍s�����ɍs�� */
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
		// �������
		else{
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
			CLogicInt nNextWordFrom;

			CLogicInt nNextWordFrom2;
			CLogicInt nNextWordTo2;
			nNextWordFrom = ptSerachBegin.GetX2();
			while( NULL != pDocLine ){
				if( WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL ) ){
					if( nPatternLen == nNextWordTo2 - nNextWordFrom2 ){
						const wchar_t* pData = pDocLine->GetPtr();	// 2002/2/10 aroka CMemory�ύX
						/* 1==�啶���������̋�� */
						if( (!sSearchOption.bLoHiCase && 0 ==  auto_memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) ) ||
							(sSearchOption.bLoHiCase && 0 == auto_memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) )
						){
							pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
							pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
							pMatchRange->SetFromX(nNextWordFrom2);						// �}�b�`�ʒufrom
							pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);// �}�b�`�ʒuto
							nRetVal = 1;
							goto end_of_func;
						}
					}
					/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
					if( PrevOrNextWord( nLinePos, nNextWordFrom, &nNextWordFrom, FALSE, FALSE ) ){
						continue;
					}
				}
				/* ���̍s�����ɍs�� */
				nLinePos++;
				pDocLine = pDocLine->GetNextLine();
				nNextWordFrom = CLogicInt(0);
			}
		}

		nRetVal = 0;
		goto end_of_func;
	}
	//���ʂ̌��� (���K�\���ł��P��P�ʂł��Ȃ�)
	else{
		//�O������
		if( eDirection == SEARCH_BACKWARD ){
			nLinePos = ptSerachBegin.GetY2();
			nHitTo = ptSerachBegin.x;

			nIdxPos = 0;
			pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				nHitPos = -1;
				while( 1 ){
					nHitPosOld = nHitPos;
					nIdxPosOld = nIdxPos;
					pszRes = SearchString(
						pLine,
						nLineLen,
						nIdxPos,
						pszPattern,
						nPatternLen,
						pnKey_CharCharsArr,
						sSearchOption.bLoHiCase
					);
					if( NULL != pszRes ){
						nHitPos = pszRes - pLine;
						nIdxPos = nHitPos + nPatternLen;	// �}�b�`�����񒷐i�߂�悤�ɕύX 2005.10.28 Karoto
						if( nHitPos >= nHitTo ){
							if( -1 != nHitPosOld ){
								pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
								pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
								pMatchRange->SetFromX(nHitPosOld);	// �}�b�`�ʒufrom
 								pMatchRange->SetToX  (nIdxPosOld);	// �}�b�`�ʒuto
								nRetVal = 1;
								goto end_of_func;
							}else{
								break;
							}
						}
					}else{
						if( -1 != nHitPosOld ){
							pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
							pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
							pMatchRange->SetFromX(nHitPosOld);	// �}�b�`�ʒufrom
							pMatchRange->SetToX  (nIdxPosOld);	// �}�b�`�ʒuto
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
		//�������
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
					pszPattern,
					nPatternLen,
					pnKey_CharCharsArr,
					sSearchOption.bLoHiCase
				);
				if( NULL != pszRes ){
					pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
					pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
					pMatchRange->SetFromX(pszRes - pLine);							// �}�b�`�ʒufrom (�����P��)
					pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);	// �}�b�`�ʒuto   (�����P��)
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
	if( NULL != pnKey_CharCharsArr ){
		delete [] pnKey_CharCharsArr;
		pnKey_CharCharsArr = NULL;
	}

	return nRetVal;
}




/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
  From�̈ʒu�փe�L�X�g��}������
*/
void CSearchAgent::ReplaceData( DocLineReplaceArg* pArg )
{
//	MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::ReplaceData()" );

	/* �}���ɂ���đ������s�̐� */
	pArg->nInsLineNum = CLogicInt(0);
	/* �폜�����s�̑��� */
	pArg->nDeletedLineNum = CLogicInt(0);
	/* �폜���ꂽ�f�[�^ */
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
	HWND		hwndCancel = NULL;	//	������
	HWND		hwndProgress = NULL;	//	������

	pArg->ptNewPos = pArg->sDelRange.GetFrom();

	/* ��ʂ̃f�[�^�𑀍삷��Ƃ� */
	CDlgCancel*	pCDlgCancel = NULL;
	if( 3000 < pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y || 1024000 < pArg->nInsDataLen){
		/* �i���_�C�A���O�̕\�� */
		pCDlgCancel = new CDlgCancel;
		if( NULL != ( hwndCancel = pCDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
			Progress_SetRange( hwndProgress, 0, 100 );
 			Progress_SetPos( hwndProgress, 0 );
		}
	}

	// �o�b�t�@���m��
	nWorkLen = 0;
	i = pArg->sDelRange.GetFrom().y;
	pCDocLine = m_pcDocLineMgr->GetLine( CLogicInt(i) );
	while( i <= pArg->sDelRange.GetTo().y && NULL != pCDocLine ){
		nWorkLen += pCDocLine->GetLengthWithEOL();
		++i;
		pCDocLine = m_pcDocLineMgr->GetLine( CLogicInt(i) );
	}
	pArg->pcmemDeleted->AllocStringBuffer( nWorkLen );

	// �폜�f�[�^�̎擾�̃��[�v
	/* �O���珈�����Ă��� */
	/* ���ݍs�̏��𓾂� */
	pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetFrom().GetY2() );
	for( i = pArg->sDelRange.GetFrom().y; i <= pArg->sDelRange.GetTo().y && NULL != pCDocLine; i++ ){
		pLine = pCDocLine->GetPtr(); // 2002/2/10 aroka CMemory�ύX
		nLineLen = pCDocLine->GetLengthWithEOL(); // 2002/2/10 aroka CMemory�ύX
		pCDocLinePrev = pCDocLine->GetPrevLine();
		pCDocLineNext = pCDocLine->GetNextLine();
		/* ���ݍs�̍폜�J�n�ʒu�𒲂ׂ� */
		if( i == pArg->sDelRange.GetFrom().y ){
			nWorkPos = pArg->sDelRange.GetFrom().x;
		}else{
			nWorkPos = 0;
		}
		/* ���ݍs�̍폜�f�[�^���𒲂ׂ� */
		if( i == pArg->sDelRange.GetTo().y ){
			nWorkLen = pArg->sDelRange.GetTo().x - nWorkPos;
		}else{
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}
		if( 0 == nWorkLen ){
			/* �O�̍s�� */
			goto next_line;
		}
		if( 0 > nWorkLen ){
			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION,
				_T("��҂ɋ����ė~�����G���["),
				_T("CDocLineMgr::ReplaceData()\n")
				_T("\n")
				_T("0 > nWorkLen\nnWorkLen=%d\n")
				_T("i=%d\n")
				_T("pArg->sDelRange.GetTo().y=%d"),
				nWorkLen, i, pArg->sDelRange.GetTo().y
			);
		}

		/* ���s���폜����񂩂��̂��E�E�E�H */
		if( EOL_NONE != pCDocLine->GetEol() &&
			nWorkPos + nWorkLen > nLineLen - pCDocLine->GetEol().GetLen() // 2002/2/10 aroka CMemory�ύX
		){
			/* �폜���钷���ɉ��s���܂߂� */
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}
		/* �폜���ꂽ�f�[�^��ۑ� */
		// 2002/2/10 aroka from here CMemory�ύX �O�̂��߁B
		if( pLine != pCDocLine->GetPtr() ){
			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION,
				_T("��҂ɋ����ė~�����G���["),
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
		/* ���̍s�̃I�u�W�F�N�g�̃|�C���^ */
		pCDocLine = pCDocLineNext;
		/* �ŋߎQ�Ƃ����s�ԍ��ƍs�f�[�^ */
		++m_pcDocLineMgr->m_nPrevReferLine;
		m_pcDocLineMgr->m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (i - pArg->sDelRange.GetFrom().y) && ( 0 == ((i - pArg->sDelRange.GetFrom().y) % 32)) ){
				nProgress = (i - pArg->sDelRange.GetFrom().y) * 100 / (pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y) / 2;
				Progress_SetPos( hwndProgress, nProgress );

			}
		}
	} // �폜�f�[�^�̎擾�̃��[�v


	/* ���ݍs�̏��𓾂� */
	pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetTo().GetY2() );
	i = pArg->sDelRange.GetTo().y;
	if( 0 < pArg->sDelRange.GetTo().y && NULL == pCDocLine ){
		pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetTo().GetY2() - CLogicInt(1) );
		i--;
	}
	/* ��납�珈�����Ă��� */
	for( ; i >= pArg->sDelRange.GetFrom().y && NULL != pCDocLine; i-- ){
		pLine = pCDocLine->GetPtr(); // 2002/2/10 aroka CMemory�ύX
		nLineLen = pCDocLine->GetLengthWithEOL(); // 2002/2/10 aroka CMemory�ύX
		pCDocLinePrev = pCDocLine->GetPrevLine();
		pCDocLineNext = pCDocLine->GetNextLine();
		/* ���ݍs�̍폜�J�n�ʒu�𒲂ׂ� */
		if( i == pArg->sDelRange.GetFrom().y ){
			nWorkPos = pArg->sDelRange.GetFrom().x;
		}else{
			nWorkPos = 0;
		}
		/* ���ݍs�̍폜�f�[�^���𒲂ׂ� */
		if( i == pArg->sDelRange.GetTo().y ){
			nWorkLen = pArg->sDelRange.GetTo().x - nWorkPos;
		}else{
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}

		if( 0 == nWorkLen ){
			/* �O�̍s�� */
			goto prev_line;
		}
		/* ���s���폜����񂩂��̂��E�E�E�H */
		if( EOL_NONE != pCDocLine->GetEol() &&
			nWorkPos + nWorkLen > nLineLen - pCDocLine->GetEol().GetLen() // 2002/2/10 aroka CMemory�ύX
		){
			/* �폜���钷���ɉ��s���܂߂� */
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}


		/* �s�S�̂̍폜 */
		if( nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory�ύX
			/* �폜�����s�̑��� */
			++(pArg->nDeletedLineNum);
			/* �s�I�u�W�F�N�g�̍폜�A���X�g�ύX�A�s��-- */
			m_pcDocLineMgr->DeleteLine( pCDocLine );
			pCDocLine = NULL;
		}
		/* ���̍s�ƘA������悤�ȍ폜 */
		else if( nWorkPos + nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory�ύX

			/* �s���f�[�^�폜 */
			{// 20020119 aroka �u���b�N���� pWork ������߂�
				wchar_t* pWork = new wchar_t[nWorkPos + 1];
				auto_memcpy( pWork, pLine, nWorkPos ); // 2002/2/10 aroka ���x�� GetPtr ���Ă΂Ȃ�
				pCDocLine->SetDocLineString( pWork, nWorkPos );
				delete [] pWork;
			}

			/* ���̍s������ */
			if( pCDocLineNext ){
				/* ���̍s�̃f�[�^���Ō�ɒǉ� */
				{
					CNativeW tmp = pCDocLine->_GetDocLineDataWithEOL() + pCDocLineNext->_GetDocLineDataWithEOL();
					pCDocLine->SetDocLineString(tmp);
				}

				/* ���̍s �s�I�u�W�F�N�g�̍폜 */
				m_pcDocLineMgr->DeleteLine( pCDocLineNext );
				pCDocLineNext = NULL;

				/* �폜�����s�̑��� */
				++(pArg->nDeletedLineNum);
			}
			CModifyVisitor().SetLineModified(pCDocLine,true);	/* �ύX�t���O */
		}
		else{
		/* �s�������̍폜 */
			{// 20020119 aroka �u���b�N���� pWork ������߂�
				// 2002/2/10 aroka CMemory�ύX ���x�� GetLength,GetPtr ����΂Ȃ��B
				int nLength = pCDocLine->GetLengthWithEOL();
				wchar_t* pWork = new wchar_t[nLength - nWorkLen + 1];
				wmemcpy( pWork, pLine, nWorkPos );

				wmemcpy( &pWork[nWorkPos], &pLine[nWorkPos + nWorkLen], nLength - ( nWorkPos + nWorkLen ) );

				pCDocLine->SetDocLineString( pWork, nLength - nWorkLen );
				delete [] pWork;
			}
			CModifyVisitor().SetLineModified(pCDocLine,true);	/* �ύX�t���O */
		}

prev_line:;
		/* ���O�̍s�̃I�u�W�F�N�g�̃|�C���^ */
		pCDocLine = pCDocLinePrev;
		/* �ŋߎQ�Ƃ����s�ԍ��ƍs�f�[�^ */
		--m_pcDocLineMgr->m_nPrevReferLine;
		m_pcDocLineMgr->m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (pArg->sDelRange.GetTo().y - i) && ( 0 == ((pArg->sDelRange.GetTo().y - i) % 32) ) ){
				nProgress = (pArg->sDelRange.GetTo().y - i) * 100 / (pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y) / 2 + 50;
				Progress_SetPos( hwndProgress, nProgress );
			}
		}
	}


	/* �f�[�^�}������ */
	if( 0 == pArg->nInsDataLen ){
		goto end_of_func;
	}
	nAllLinesOld = m_pcDocLineMgr->GetLineCount();
	pArg->ptNewPos.y = pArg->sDelRange.GetFrom().y;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
	pArg->ptNewPos.x = 0;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */

	/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
	nCount = 0;
	pArg->nInsLineNum = CLogicInt(0);
	pCDocLine = m_pcDocLineMgr->GetLine( pArg->sDelRange.GetFrom().GetY2() );



	if( NULL == pCDocLine ){
		/* ������NULL���A���Ă���Ƃ������Ƃ́A*/
		/* �S�e�L�X�g�̍Ō�̎��̍s��ǉ����悤�Ƃ��Ă��邱�Ƃ����� */
		cmemPrevLine.SetString(L"");
		cmemNextLine.SetString(L"");
		cEOLTypeNext.SetType( EOL_NONE );
	}else{
		CModifyVisitor().SetLineModified(pCDocLine,true);	/* �ύX�t���O */

		// 2002/2/10 aroka ���x�� GetPtr ���Ă΂Ȃ�
		pLine = pCDocLine->GetDocLineStrWithEOL( &nLineLen );
		cmemPrevLine.SetString( pLine, pArg->sDelRange.GetFrom().x );
		cmemNextLine.SetString( &pLine[pArg->sDelRange.GetFrom().x], nLineLen - pArg->sDelRange.GetFrom().x );

		cEOLTypeNext = pCDocLine->GetEol();
	}
	nBgn = 0;
	for( nPos = 0; nPos < pArg->nInsDataLen; ){
		if( pArg->pInsData[nPos] == '\n' || pArg->pInsData[nPos] == '\r' ){
			/* �s�I�[�q�̎�ނ𒲂ׂ� */
			cEOLType.SetTypeByString( &(pArg->pInsData[nPos]), pArg->nInsDataLen - nPos );
			/* �s�I�[�q���܂߂ăe�L�X�g���o�b�t�@�Ɋi�[ */
			cmemCurLine.SetString( &(pArg->pInsData[nBgn]), nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + cEOLType.GetLen();
			nPos = nBgn;
			if( NULL == pCDocLine){
				CDocLine* pCDocLineNew = m_pcDocLineMgr->AddNewLine();

				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					pCDocLineNew->SetDocLineString( cmemPrevLine + cmemCurLine );
				}
				else{
					pCDocLineNew->SetDocLineString( cmemCurLine );
				}
				pCDocLine = NULL;
			}
			else{
				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					pCDocLine->SetDocLineString( cmemPrevLine + cmemCurLine );
					pCDocLine = pCDocLine->GetNextLine();
				}
				else{
					CDocLine* pCDocLineNew = m_pcDocLineMgr->InsertNewLine(pCDocLine);	//pCDocLine�̑O�ɑ}��
					pCDocLineNew->SetDocLineString( cmemCurLine );
				}
			}

			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			++nCount;
			++(pArg->ptNewPos.y);	/* �}�����ꂽ�����̎��̈ʒu�̍s */
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
			CDocLine* pCDocLineNew = m_pcDocLineMgr->AddNewLine();	//�����ɒǉ�
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				pCDocLineNew->SetDocLineString( cmemPrevLine + cmemCurLine );
			}
			else{
				pCDocLineNew->SetDocLineString( cmemCurLine );
			}
			pCDocLine = NULL;
			pArg->ptNewPos.x = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
		}
		else{
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				pCDocLine->SetDocLineString( cmemPrevLine + cmemCurLine );

				pCDocLine = pCDocLine->GetNextLine();
				pArg->ptNewPos.x = cmemPrevLine.GetStringLength() + nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}
			else{
				CDocLine* pCDocLineNew = m_pcDocLineMgr->InsertNewLine(pCDocLine);	//pCDocLine�̑O�ɑ}��

				pCDocLineNew->SetDocLineString( cmemCurLine );

				pArg->ptNewPos.x = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}
		}
	}
	pArg->nInsLineNum = m_pcDocLineMgr->GetLineCount() - nAllLinesOld;
end_of_func:;
	if( NULL != pCDlgCancel ){
		// �i���_�C�A���O��\�����Ȃ��ꍇ�Ɠ��������ɂȂ�悤�Ƀ_�C�A���O�͒x���j������
		// ������ pCDlgCancel �� delete ����� delete ����߂�܂ł̊Ԃ�
		// �_�C�A���O�j�� -> �ҏW��ʂփt�H�[�J�X�ړ� -> �L�����b�g�ʒu����
		// �܂ň�C�ɓ����̂Ŗ����ȃ��C�A�E�g���Q�Ƃňُ�I�����邱�Ƃ�����
		pCDlgCancel->DeleteAsync();	// �����j����x�����s����	// 2008.05.28 ryoji
	}
	return;
}
