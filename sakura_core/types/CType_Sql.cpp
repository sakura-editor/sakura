/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "types/CType.h"
#include "doc/CEditDoc.h"
#include "doc/CDocOutline.h"
#include "doc/logic/CDocLine.h"
#include "outline/CFuncInfoArr.h"

/* PL/SQL */
void CType_Sql::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, LTEXT("PL/SQL") );
	wcscpy( pType->m_szTypeExts, LTEXT("sql,plsql") );

	//設定
	pType->m_cLineComment.CopyTo( 0, LTEXT("--"), -1 );					/* 行コメントデリミタ */
	pType->m_cBlockComments[0].SetBlockCommentRule( LTEXT("/*"), LTEXT("*/") );	/* ブロックコメントデリミタ */
	pType->m_nStringType = STRING_LITERAL_PLSQL;					/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, LTEXT("|★") );						/* その他のインデント対象文字 */
	pType->m_nKeyWordSetIdx[0] = 2;									/* キーワードセット */
	pType->m_eDefaultOutline = OUTLINE_PLSQL;						/* アウトライン解析方法 */
}

/*! PL/SQL関数リスト作成 */
void CDocOutline::MakeFuncList_PLSQL( CFuncInfoArr* pcFuncInfoArr )
{
	const WCHAR*	pLine;
	CLogicInt		nLineLen;
	int			i;
	int			nCharChars;
	WCHAR		szWordPrev[100];
	WCHAR		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	WCHAR		szFuncName[100];
	CLogicInt	nFuncLine(0);
	int			nFuncId = 0;
	int			nFuncNum;
	int			nFuncOrProc = 0;
	int			nParseCnt = 0;
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	szWordPrev[0] = LCHAR('\0');
	szWord[nWordIdx] = LCHAR('\0');
	nMode = 0;
	nFuncNum = 0;
	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL( &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
//			if( 1 < nCharChars ){
//				i += (nCharChars - 1);
//				continue;
//			}
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				if( LCHAR('\'') == pLine[i] ){
					if( i + 1 < nLineLen && LCHAR('\'') == pLine[i + 1] ){
						++i;
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* コメント読み込み中 */
			if( 8 == nMode ){
				if( i + 1 < nLineLen && LCHAR('*') == pLine[i] &&  LCHAR('/') == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* 単語読み込み中 */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					LCHAR('_') == pLine[i] ||
					LCHAR('~') == pLine[i] ||
					(LCHAR('a') <= pLine[i] &&	pLine[i] <= LCHAR('z') )||
					(LCHAR('A') <= pLine[i] &&	pLine[i] <= LCHAR('Z') )||
					(LCHAR('0') <= pLine[i] &&	pLine[i] <= LCHAR('9') )||
					(LCHAR('\u00a1') <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 2013.05.08 日本語対応
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
						szWord[nWordIdx + nCharChars] = LCHAR('\0');
						nWordIdx += (nCharChars);
					}
				}
				else{
					if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("FUNCTION") ) ){
						nFuncOrProc = 1;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("PROCEDURE") ) ){
						nFuncOrProc = 2;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("PACKAGE") ) ){
						nFuncOrProc = 3;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 1 == nParseCnt && 3 == nFuncOrProc && 0 == _wcsicmp( szWord, LTEXT("BODY") ) ){
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
						if( 0 == _wcsicmp( szWord, LTEXT("IS") ) ){
							if( 1 == nFuncOrProc ){
								nFuncId = 11;	/* ファンクション本体 */
							}else
							if( 2 == nFuncOrProc ){
								nFuncId = 21;	/* プロシージャ本体 */
							}else
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* パッケージ仕様部 */
							}else
							if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* パッケージ本体 */
							}
							++nFuncNum;
							/*
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							CLayoutPoint ptPos;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(0, nFuncLine - 1),
								&ptPos
							);
							pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1), szFuncName, nFuncId );
							nParseCnt = 0;
						}
						if( 0 == _wcsicmp( szWord, LTEXT("AS") ) ){
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* パッケージ仕様部 */
								++nFuncNum;
								/*
								  カーソル位置変換
								  物理位置(行頭からのバイト数、折り返し無し行位置)
								  →
								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
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
								nFuncId = 41;	/* パッケージ本体 */
								++nFuncNum;
								/*
								  カーソル位置変換
								  物理位置(行頭からのバイト数、折り返し無し行位置)
								  →
								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
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
					szWord[0] = LCHAR('\0');
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( 2 == nMode ){
				if( LCHAR('_') == pLine[i] ||
					LCHAR('~') == pLine[i] ||
					(LCHAR('a') <= pLine[i] &&	pLine[i] <= LCHAR('z') )||
					(LCHAR('A') <= pLine[i] &&	pLine[i] <= LCHAR('Z') )||
					(LCHAR('0') <= pLine[i] &&	pLine[i] <= LCHAR('9') )||
					(LCHAR('\u00a1') <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i]))|| // 2013.05.08 日本語対応
					LCHAR('\t') == pLine[i] ||
					 LCHAR(' ') == pLine[i] ||
					 WCODE::IsLineDelimiter(pLine[i], bExtEol) ||
					 LCHAR('{') == pLine[i] ||
					 LCHAR('}') == pLine[i] ||
					 LCHAR('(') == pLine[i] ||
					 LCHAR(')') == pLine[i] ||
					 LCHAR(';') == pLine[i] ||
					LCHAR('\'') == pLine[i] ||
					 LCHAR('/') == pLine[i] ||
					 LCHAR('-') == pLine[i] ||
					 2 == nCharChars
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = LCHAR('\0');
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
						szWord[nWordIdx + nCharChars] = LCHAR('\0');
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( LCHAR('\t') == pLine[i] ||
					LCHAR(' ') == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( LCHAR('\t') == pLine[i] ||
					LCHAR(' ') == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				){
					continue;
				}else
				if( i < nLineLen - 1 && LCHAR('-') == pLine[i] &&  LCHAR('-') == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && LCHAR('/') == pLine[i] &&  LCHAR('*') == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( LCHAR('\'') == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( LCHAR(';') == pLine[i] ){
					if( 2 == nParseCnt ){
						if( 1 == nFuncOrProc ){
							nFuncId = 10;	/* ファンクション宣言 */
						}else{
							nFuncId = 20;	/* プロシージャ宣言 */
						}
						++nFuncNum;
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
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
						LCHAR('_') == pLine[i] ||
						LCHAR('~') == pLine[i] ||
						(LCHAR('a') <= pLine[i] &&	pLine[i] <= LCHAR('z') )||
						(LCHAR('A') <= pLine[i] &&	pLine[i] <= LCHAR('Z') )||
						(LCHAR('0') <= pLine[i] &&	pLine[i] <= LCHAR('9') )||
						(LCHAR('\u00a1') <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 2013.05.08 日本語対応
						) )
					 || 2 == nCharChars
					){
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;

//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = LCHAR('\0');
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';

						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = LCHAR('\0');
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

const WCHAR* g_ppszKeywordsPLSQL[] = {
	LTEXT("AND"),
	LTEXT("AS"),
	LTEXT("BEGIN"),
	LTEXT("BINARY_INTEGER"),
	LTEXT("BODY"),
	LTEXT("BOOLEAN"),
	LTEXT("BY"),
	LTEXT("CHAR"),
	LTEXT("CHR"),
	LTEXT("COMMIT"),
	LTEXT("COUNT"),
	LTEXT("CREATE"),
	LTEXT("CURSOR"),
	LTEXT("DATE"),
	LTEXT("DECLARE"),
	LTEXT("DEFAULT"),
	LTEXT("DELETE"),
	LTEXT("ELSE"),
	LTEXT("ELSIF"),
	LTEXT("END"),
	LTEXT("ERRORS"),
	LTEXT("EXCEPTION"),
	LTEXT("FALSE"),
	LTEXT("FOR"),
	LTEXT("FROM"),
	LTEXT("FUNCTION"),
	LTEXT("GOTO"),
	LTEXT("HTP"),
	LTEXT("IDENT_ARR"),
	LTEXT("IF"),
	LTEXT("IN"),
	LTEXT("INDEX"),
	LTEXT("INTEGER"),
	LTEXT("IS"),
	LTEXT("LOOP"),
	LTEXT("NOT"),
	LTEXT("NO_DATA_FOUND"),
	LTEXT("NULL"),
	LTEXT("NUMBER"),
	LTEXT("OF"),
	LTEXT("OR"),
	LTEXT("ORDER"),
	LTEXT("OUT"),
	LTEXT("OWA_UTIL"),
	LTEXT("PACKAGE"),
	LTEXT("PRAGMA"),
	LTEXT("PRN"),
	LTEXT("PROCEDURE"),
	LTEXT("REPLACE"),
	LTEXT("RESTRICT_REFERENCES"),
	LTEXT("RETURN"),
	LTEXT("ROWTYPE"),
	LTEXT("SELECT"),
	LTEXT("SHOW"),
	LTEXT("SUBSTR"),
	LTEXT("TABLE"),
	LTEXT("THEN"),
	LTEXT("TRUE"),
	LTEXT("TYPE"),
	LTEXT("UPDATE"),
	LTEXT("VARCHAR"),
	LTEXT("VARCHAR2"),
	LTEXT("WHEN"),
	LTEXT("WHERE"),
	LTEXT("WHILE"),
	LTEXT("WNDS"),
	LTEXT("WNPS"),
	LTEXT("RAISE"),
	LTEXT("INSERT"),
	LTEXT("INTO"),
	LTEXT("VALUES"),
	LTEXT("SET"),
	LTEXT("SYSDATE"),
	LTEXT("RTRIM"),
	LTEXT("LTRIM"),
	LTEXT("TO_CHAR"),
	LTEXT("DUP_VAL_ON_INDEX"),
	LTEXT("ROLLBACK"),
	LTEXT("OTHERS"),
	LTEXT("SQLCODE")
};
int g_nKeywordsPLSQL = int(std::size(g_ppszKeywordsPLSQL));
