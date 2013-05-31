#include "StdAfx.h"
#include "types/CType.h"
#include "doc/CEditDoc.h"
#include "doc/CDocOutline.h"
#include "doc/logic/CDocLine.h"
#include "outline/CFuncInfoArr.h"

/* PL/SQL */
void CType_Sql::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("PL/SQL") );
	_tcscpy( pType->m_szTypeExts, _T("sql,plsql") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"--", -1 );					/* �s�R�����g�f���~�^ */
	pType->m_cBlockComments[0].SetBlockCommentRule( L"/*", L"*/" );	/* �u���b�N�R�����g�f���~�^ */
	pType->m_nStringType = 1;										/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"|��" );						/* ���̑��̃C���f���g�Ώە��� */
	pType->m_nKeyWordSetIdx[0] = 2;									/* �L�[���[�h�Z�b�g */
	pType->m_eDefaultOutline = OUTLINE_PLSQL;						/* �A�E�g���C����͕��@ */
}




/*! PL/SQL�֐����X�g�쐬 */
void CDocOutline::MakeFuncList_PLSQL( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int			i;
	int			nCharChars;
	wchar_t		szWordPrev[100];
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	wchar_t		szFuncName[100];
	CLogicInt	nFuncLine(0);
	int			nFuncId = 0;
	int			nFuncNum;
	int			nFuncOrProc = 0;
	int			nParseCnt = 0;

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	nFuncNum = 0;
	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL( &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
//			if( 1 < nCharChars ){
//				i += (nCharChars - 1);
//				continue;
//			}
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				if( L'\'' == pLine[i] ){
					if( i + 1 < nLineLen && L'\'' == pLine[i + 1] ){
						++i;
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* �R�����g�ǂݍ��ݒ� */
			if( 8 == nMode ){
				if( i + 1 < nLineLen && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 2013.05.08 ���{��Ή�
					) )
				 || 2 == nCharChars
				){
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						i += (nCharChars - 1);
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
				else{
					if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"FUNCTION" ) ){
						nFuncOrProc = 1;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"PROCEDURE" ) ){
						nFuncOrProc = 2;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"PACKAGE" ) ){
						nFuncOrProc = 3;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 1 == nParseCnt && 3 == nFuncOrProc && 0 == wcsicmp( szWord, L"BODY" ) ){
						nFuncOrProc = 4;
						nParseCnt = 1;
					}
					else if( 1 == nParseCnt ){
						if( 1 == nFuncOrProc ||
							2 == nFuncOrProc ||
							3 == nFuncOrProc ||
							4 == nFuncOrProc ){
							++nParseCnt;
							wcscpy( szFuncName, szWord );
//						}else
//						if( 3 == nFuncOrProc ){

						}
					}else
					if( 2 == nParseCnt ){
						if( 0 == wcsicmp( szWord, L"IS" ) ){
							if( 1 == nFuncOrProc ){
								nFuncId = 11;	/* �t�@���N�V�����{�� */
							}else
							if( 2 == nFuncOrProc ){
								nFuncId = 21;	/* �v���V�[�W���{�� */
							}else
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* �p�b�P�[�W�d�l�� */
							}else
							if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* �p�b�P�[�W�{�� */
							}
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							CLayoutPoint ptPos;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(0, nFuncLine - 1),
								&ptPos
							);
							pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1), szFuncName, nFuncId );
							nParseCnt = 0;
						}
						if( 0 == wcsicmp( szWord, L"AS" ) ){
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* �p�b�P�[�W�d�l�� */
								++nFuncNum;
								/*
								  �J�[�\���ʒu�ϊ�
								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
								  ��
								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
								*/
								CLayoutPoint ptPos;
								m_pcDocRef->m_cLayoutMgr.LogicToLayout(
									CLogicPoint(0, nFuncLine - 1),
									&ptPos
								);
								pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
								nParseCnt = 0;
							}
							else if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* �p�b�P�[�W�{�� */
								++nFuncNum;
								/*
								  �J�[�\���ʒu�ϊ�
								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
								  ��
								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
								*/
								CLayoutPoint ptPos;
								m_pcDocRef->m_cLayoutMgr.LogicToLayout(
									CLogicPoint(0, nFuncLine - 1),
									&ptPos
								);
								pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
								nParseCnt = 0;
							}
						}
					}
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( 2 == nMode ){
				if( L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i]))|| // 2013.05.08 ���{��Ή�
					L'\t' == pLine[i] ||
					 L' ' == pLine[i] ||
					 WCODE::IsLineDelimiter(pLine[i]) ||
					 L'{' == pLine[i] ||
					 L'}' == pLine[i] ||
					 L'(' == pLine[i] ||
					 L')' == pLine[i] ||
					 L';' == pLine[i] ||
					L'\'' == pLine[i] ||
					 L'/' == pLine[i] ||
					 L'-' == pLine[i] ||
					 2 == nCharChars
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}else{
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i])
				){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i])
				){
					continue;
				}else
				if( i < nLineLen - 1 && L'-' == pLine[i] &&  L'-' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( L'\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( L';' == pLine[i] ){
					if( 2 == nParseCnt ){
						if( 1 == nFuncOrProc ){
							nFuncId = 10;	/* �t�@���N�V�����錾 */
						}else{
							nFuncId = 20;	/* �v���V�[�W���錾 */
						}
						++nFuncNum;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						CLayoutPoint ptPos;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(
							CLogicPoint(0, nFuncLine - 1),
							&ptPos
						);
						pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
						nParseCnt = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( (1 == nCharChars && (
						L'_' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
						(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 2013.05.08 ���{��Ή�
						) )
					 || 2 == nCharChars
					){
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;

//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';

						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}




const wchar_t* g_ppszKeywordsPLSQL[] = {
	L"AND",
	L"AS",
	L"BEGIN",
	L"BINARY_INTEGER",
	L"BODY",
	L"BOOLEAN",
	L"BY",
	L"CHAR",
	L"CHR",
	L"COMMIT",
	L"COUNT",
	L"CREATE",
	L"CURSOR",
	L"DATE",
	L"DECLARE",
	L"DEFAULT",
	L"DELETE",
	L"ELSE",
	L"ELSIF",
	L"END",
	L"ERRORS",
	L"EXCEPTION",
	L"FALSE",
	L"FOR",
	L"FROM",
	L"FUNCTION",
	L"GOTO",
	L"HTP",
	L"IDENT_ARR",
	L"IF",
	L"IN",
	L"INDEX",
	L"INTEGER",
	L"IS",
	L"LOOP",
	L"NOT",
	L"NO_DATA_FOUND",
	L"NULL",
	L"NUMBER",
	L"OF",
	L"OR",
	L"ORDER",
	L"OUT",
	L"OWA_UTIL",
	L"PACKAGE",
	L"PRAGMA",
	L"PRN",
	L"PROCEDURE",
	L"REPLACE",
	L"RESTRICT_REFERENCES",
	L"RETURN",
	L"ROWTYPE",
	L"SELECT",
	L"SHOW",
	L"SUBSTR",
	L"TABLE",
	L"THEN",
	L"TRUE",
	L"TYPE",
	L"UPDATE",
	L"VARCHAR",
	L"VARCHAR2",
	L"WHEN",
	L"WHERE",
	L"WHILE",
	L"WNDS",
	L"WNPS",
	L"RAISE",
	L"INSERT",
	L"INTO",
	L"VALUES",
	L"SET",
	L"SYSDATE",
	L"RTRIM",
	L"LTRIM",
	L"TO_CHAR",
	L"DUP_VAL_ON_INDEX",
	L"ROLLBACK",
	L"OTHERS",
	L"SQLCODE"
};
int g_nKeywordsPLSQL = _countof(g_ppszKeywordsPLSQL);
