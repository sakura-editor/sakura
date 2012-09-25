#include "StdAfx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "outline/CFuncInfoArr.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"

/* Java */
void CType_Java::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("Java") );
	_tcscpy( pType->m_szTypeExts, _T("java,jav") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );					/* �s�R�����g�f���~�^ */
	pType->m_cBlockComments[0].SetBlockCommentRule( L"/*", L"*/" );	/* �u���b�N�R�����g�f���~�^ */
	pType->m_nKeyWordSetIdx[0] = 4;									/* �L�[���[�h�Z�b�g */
	pType->m_eDefaultOutline = OUTLINE_JAVA;						/* �A�E�g���C����͕��@ */
	pType->m_eSmartIndent = SMARTINDENT_CPP;						/* �X�}�[�g�C���f���g��� */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			//���p���l��F�����\��	//Mar. 10, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;	//�Ί��ʂ̋������f�t�H���gON��	//Sep. 21, 2002 genta
}


/* Java��̓��[�h */
enum EFuncListJavaMode {
	FL_JAVA_MODE_NORMAL = 0,
	FL_JAVA_MODE_WORD = 1,
	FL_JAVA_MODE_SYMBOL = 2,
	FL_JAVA_MODE_COMMENT = 8,
	FL_JAVA_MODE_SINGLE_QUOTE = 20,
	FL_JAVA_MODE_DOUBLE_QUOTE = 21,
	FL_JAVA_MODE_TOO_LONG_WORD = 999
};

/* Java�֐����X�g�쐬 */
void CDocOutline::MakeFuncList_Java( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt	nLineLen;
	int			i;
	int			nNestLevel;
	int			nCharChars;
	wchar_t		szWordPrev[100];
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	EFuncListJavaMode	nMode;
	wchar_t		szFuncName[100];
	CLogicInt	nFuncLine = CLogicInt(0);
	int			nFuncId;
	int			nFuncNum;
	wchar_t		szClass[1024];

	int			nClassNestArrNum;
	std::vector<int>	nClassNestArr(0);
	std::vector<int>	nNestLevel2Arr(0);

	nNestLevel = 0;
	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = FL_JAVA_MODE_NORMAL;
	//nNestLevel2Arr[0] = 0;
	nFuncNum = 0;
	szClass[0] = L'\0';
	nClassNestArrNum = 0;
	CLogicInt		nLineCount;
	wchar_t*	szJavaKigou = L"!\"#%&'()=-^|\\`@[{+;*}]<,>?/";	//���ʎq�Ɏg�p�ł��Ȃ����p�L���B_:~.$�͋���

	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		for( i = 0; i < nLineLen; i += nCharChars ){
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );

			/* �G�X�P�[�v�V�[�P���X�͏�Ɏ�菜�� */
			if( L'\\' == pLine[i] ){
				++i;
			}else
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( FL_JAVA_MODE_SINGLE_QUOTE == nMode ){
				if( L'\'' == pLine[i] ){
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
				}
			}else
			/* �_�u���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( FL_JAVA_MODE_DOUBLE_QUOTE == nMode ){
				if( L'"' == pLine[i] ){
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
				}
			}else
			/* �R�����g�ǂݍ��ݒ� */
			if( FL_JAVA_MODE_COMMENT == nMode ){
				if( i < nLineLen - 1 && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					++i;
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
				}
			}
			/* �P��ǂݍ��ݒ� */
			else if( FL_JAVA_MODE_WORD == nMode ){
				// 2011.09.16 syat �A�E�g���C����͂œ��{�ꂪ�܂܂�Ă��镔�����\������Ȃ�
				if( ! WCODE::IsBlank(pLine[i]) &&
					! WCODE::IsLineDelimiter(pLine[i]) &&
					! WCODE::IsControlCode(pLine[i]) &&
					wcschr( szJavaKigou, pLine[i] ) == NULL
					){
					if( nWordIdx + nCharChars >= nMaxWordLeng ){
						nMode = FL_JAVA_MODE_TOO_LONG_WORD;
						continue;
					}else{
						memcpy(&szWord[nWordIdx], &pLine[i], sizeof(wchar_t)*nCharChars);
						szWord[nWordIdx + nCharChars] = '\0';
					}
					nWordIdx += nCharChars;
				}else{
					/* �N���X�錾������������ */
					//	Oct. 10, 2002 genta interface���Ώۂ�
					if( 0 == wcscmp( L"class", szWordPrev ) ||
						0 == wcscmp( L"interface", szWordPrev )
					 ){
						nClassNestArr.push_back( nNestLevel );
						nNestLevel2Arr.push_back( 0 );
						++nClassNestArrNum;
						if( 0 < nNestLevel	){
							wcscat( szClass, L"\\" );
						}
						wcscat( szClass, szWord );

						nFuncId = 0;
						++nFuncNum;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						CLogicPoint  ptPosXY_Logic = CLogicPoint(CLogicInt(0), nLineCount);
						CLayoutPoint ptPosXY_Layout;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(
							ptPosXY_Logic,
							&ptPosXY_Layout
						);
						wchar_t szWork[256];
						if( 0 < auto_snprintf_s( szWork, _countof(szWork), L"%ls::%ls", szClass, L"��`�ʒu" ) ){
							pcFuncInfoArr->AppendData( ptPosXY_Logic.GetY2() + CLogicInt(1), ptPosXY_Layout.GetY2() + CLayoutInt(1), szWork, nFuncId ); //2007.10.09 kobake ���C�A�E�g�E���W�b�N�̍��݃o�O�C��
						}
					}

					nMode = FL_JAVA_MODE_NORMAL;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( FL_JAVA_MODE_SYMBOL == nMode ){
				if( L'_' == pLine[i] ||
					L':' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i] ||
					L'{' == pLine[i] ||
					L'}' == pLine[i] ||
					L'(' == pLine[i] ||
					L')' == pLine[i] ||
					L';' == pLine[i]	||
					L'\'' == pLine[i] ||
					L'"' == pLine[i] ||
					L'/' == pLine[i] ||
					L'.' == pLine[i]
				){
					nMode = FL_JAVA_MODE_NORMAL;
					i--;
					continue;
				}else{
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( FL_JAVA_MODE_TOO_LONG_WORD == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( FL_JAVA_MODE_NORMAL == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'/' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'*' == pLine[i + 1] ){
					++i;
					nMode = FL_JAVA_MODE_COMMENT;
					continue;
				}else
				if( L'\'' == pLine[i] ){
					nMode = FL_JAVA_MODE_SINGLE_QUOTE;
					continue;
				}else
				if( L'"' == pLine[i] ){
					nMode = FL_JAVA_MODE_DOUBLE_QUOTE;
					continue;
				}else
				if( L'{' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						//	���\�b�h���ł���Ƀ��\�b�h���`���邱�Ƃ͂Ȃ��̂�
						//	�l�X�g���x������ǉ� class/interface�̒����̏ꍇ�̂ݔ��肷��
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != wcscmp( L"sizeof", szFuncName )
						 && 0 != wcscmp( L"if", szFuncName )
						 && 0 != wcscmp( L"for", szFuncName )
						 && 0 != wcscmp( L"do", szFuncName )
						 && 0 != wcscmp( L"while", szFuncName )
						 && 0 != wcscmp( L"catch", szFuncName )
						 && 0 != wcscmp( L"switch", szFuncName )
						 && 0 != wcscmp( L"return", szFuncName )
						){
							nFuncId = 2;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							CLayoutPoint ptPosXY;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(CLogicInt(0), nFuncLine - CLogicInt(1)),
								&ptPosXY
							);
							wchar_t szWork[256];
							if( 0 < auto_snprintf_s( szWork, _countof(szWork), L"%ls::%ls", szClass, szFuncName ) ){
								pcFuncInfoArr->AppendData( nFuncLine, ptPosXY.GetY2() + CLayoutInt(1), szWork, nFuncId );
							}
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					++nNestLevel;
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( L'}' == pLine[i] ){
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}

					nNestLevel--;
					if( 0 < nClassNestArrNum &&
						nClassNestArr[nClassNestArrNum - 1] == nNestLevel
					){
						nClassNestArr.pop_back();
						nNestLevel2Arr.pop_back();
						nClassNestArrNum--;
						int k;
						for( k = wcslen( szClass ) - 1; k >= 0; k-- ){
							if( L'\\' == szClass[k] ){
								break;
							}
						}
						if( 0 > k ){
							k = 0;
						}
						szClass[k] = L'\0';
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( L'(' == pLine[i] ){
					if( 0 < nClassNestArrNum /*nNestLevel == 1*/ &&
						0 != wcscmp( L"new", szWordPrev )
					){
						wcscpy( szFuncName, szWord );
						nFuncLine = nLineCount + CLogicInt(1);
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 1;
						}
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( L')' == pLine[i] ){
					int			k;
					const wchar_t*	pLine2;
					CLogicInt		nLineLen2;
					CLogicInt	nLineCount2;
					nLineCount2 = nLineCount;
					pLine2 = pLine;
					nLineLen2 = nLineLen;
					k = i + 1;
					BOOL		bCommentLoop;
					bCommentLoop = FALSE;
				loop_is_func:;
					for( ; k < nLineLen2; ++k ){
						if( !bCommentLoop ){
							if( pLine2[k] != L' ' && pLine2[k] != WCODE::TAB && pLine2[k] != WCODE::CR && pLine2[k] != WCODE::LF ){
								if( k + 1 < nLineLen2 && pLine2[k] == L'/' && pLine2[k + 1] == L'*' ){
									bCommentLoop = TRUE;
									++k;
								}else
								if( k + 1 < nLineLen2 && pLine2[k] == L'/' && pLine2[k + 1] == L'/' ){
									k = nLineLen2 + 1;
									break;
								}else{
									break;
								}
							}
						}else{
							if( k + 1 < nLineLen2 && pLine2[k] == L'*' && pLine2[k + 1] == L'/' ){
								bCommentLoop = FALSE;
								++k;
							}
						}
					}
					if( k >= nLineLen2 ){
						k = 0;
						++nLineCount2;
						pLine2 = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount2)->GetDocLineStrWithEOL(&nLineLen2);
						if( NULL != pLine2 ){
							goto loop_is_func;
						}
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 0;
						}
					}else{
						//	Oct. 10, 2002 genta
						//	abstract �ɂ��Ή�
						if( pLine2[k] == L'{' || pLine2[k] == L';' ||
							__iscsym( pLine2[k] ) ){
							if( 0 < nClassNestArrNum ){
								if( 1 == nNestLevel2Arr[nClassNestArrNum - 1] ){
									nNestLevel2Arr[nClassNestArrNum - 1] = 2;
								}
							}
						}else{
							if( 0 < nClassNestArrNum ){
								nNestLevel2Arr[nClassNestArrNum - 1] = 0;
							}
						}
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( L';' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						// �֐��̒��ŕʂ̊֐��̐錾�����g�����Ƃ��āCJava�ł���́H
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != wcscmp( L"sizeof", szFuncName )
						 && 0 != wcscmp( L"if", szFuncName )
						 && 0 != wcscmp( L"for", szFuncName )
						 && 0 != wcscmp( L"do", szFuncName )
						 && 0 != wcscmp( L"while", szFuncName )
						 && 0 != wcscmp( L"catch", szFuncName )
						 && 0 != wcscmp( L"switch", szFuncName )
						 && 0 != wcscmp( L"return", szFuncName )
						){
							nFuncId = 1;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							CLayoutPoint ptPosXY;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(CLogicInt(0), nFuncLine - CLogicInt(1)),
								&ptPosXY
							);
							wchar_t szWork[256];
							if( 0 < auto_snprintf_s( szWork, _countof(szWork), L"%ls::%ls", szClass, szFuncName ) ){
								pcFuncInfoArr->AppendData( nFuncLine, ptPosXY.GetY2() + CLayoutInt(1), szWork, nFuncId );
							}
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
					if( ! WCODE::IsBlank(pLine[i]) &&
						! WCODE::IsLineDelimiter(pLine[i]) &&
						! WCODE::IsControlCode(pLine[i]) &&
						wcschr( szJavaKigou, pLine[i] ) == NULL
						){
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
						memcpy(&szWord[nWordIdx], &pLine[i], sizeof(wchar_t)*nCharChars);
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += nCharChars;
						nMode = FL_JAVA_MODE_WORD;
					}else{
						nMode = FL_JAVA_MODE_NORMAL;
					}
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}



const wchar_t* g_ppszKeywordsJAVA[] = {
	L"abstract",
	L"assert",	// Mar. 8, 2003 genta
	L"boolean",
	L"break",
	L"byte",
	L"case",
	L"catch",
	L"char",
	L"class",
	L"const",
	L"continue",
	L"default",
	L"do",
	L"double",
	L"else",
	L"extends",
	L"final",
	L"finally",
	L"float",
	L"for",
	L"goto",
	L"if",
	L"implements",
	L"import",
	L"instanceof",
	L"int",
	L"interface",
	L"long",
	L"native",
	L"new",
	L"package",
	L"private",
	L"protected",
	L"public",
	L"return",
	L"short",
	L"static",
	L"strictfp",	// Mar. 8, 2003 genta
	L"super",
	L"switch",
	L"synchronized",
	L"this",
	L"throw",
	L"throws",
	L"transient",
	L"try",
	L"void",
	L"volatile",
	L"while"
};
int g_nKeywordsJAVA = _countof(g_ppszKeywordsJAVA);
