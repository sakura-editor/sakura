/*! @file */
/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
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
	wcscpy( pType->m_szTypeName, L"PL/SQL" );
	wcscpy( pType->m_szTypeExts, L"sql,plsql" );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"--", -1 );					/* 行コメントデリミタ */
	pType->m_cBlockComments[0].SetBlockCommentRule( L"/*", L"*/" );	/* ブロックコメントデリミタ */
	pType->m_nStringType = STRING_LITERAL_PLSQL;					/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"|★" );						/* その他のインデント対象文字 */
	pType->m_nKeyWordSetIdx[0] = 2;									/* キーワードセット */
	pType->m_eDefaultOutline = OUTLINE_PLSQL;						/* アウトライン解析方法 */
}

/*! PL/SQL関数リスト作成 */
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
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
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
			/* コメント読み込み中 */
			if( 8 == nMode ){
				if( i + 1 < nLineLen && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* 単語読み込み中 */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 2013.05.08 日本語対応
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
					if( 0 == nParseCnt && 0 == _wcsicmp( szWord, L"FUNCTION" ) ){
						nFuncOrProc = 1;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, L"PROCEDURE" ) ){
						nFuncOrProc = 2;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, L"PACKAGE" ) ){
						nFuncOrProc = 3;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 1 == nParseCnt && 3 == nFuncOrProc && 0 == _wcsicmp( szWord, L"BODY" ) ){
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
						if( 0 == _wcsicmp( szWord, L"IS" ) ){
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
						if( 0 == _wcsicmp( szWord, L"AS" ) ){
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
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( 2 == nMode ){
				if( L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i]))|| // 2013.05.08 日本語対応
					L'\t' == pLine[i] ||
					 L' ' == pLine[i] ||
					 WCODE::IsLineDelimiter(pLine[i], bExtEol) ||
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
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
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
						L'_' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
						(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 2013.05.08 日本語対応
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
	L"ABS",
	L"ABSOLUTE",
	L"ACCESS",
	L"ACCESSIBLE",
	L"ACCOUNT",
	L"ACTION",
	L"ACTIVATE",
	L"ADA",
	L"ADD",
	L"ADMIN",
	L"ADVISE",
	L"AFTER",
	L"ALL",
	L"ALL_ROWS",
	L"ALLOCATE",
	L"ALTER",
	L"ANALYSE",
	L"ANALYZE",
	L"AND",
	L"ANY",
	L"ARCHIVE",
	L"ARCHIVELOG",
	L"ARE",
	L"ARRAY",
	L"ARRAY_AGG",
	L"ARRAY_MAX_CARDINALITY",
	L"AS",
	L"ASC",
	L"ASENSITIVE",
	L"ASSERTION",
	L"ASYMMETRIC",
	L"AT",
	L"ATOMIC",
	L"AUDIT",
	L"AUTHENTICATED",
	L"AUTHORIZATION",
	L"AUTOEXTEND",
	L"AUTOMATIC",
	L"AVG",
	L"BACKUP",
	L"BECOME",
	L"BEFORE",
	L"BEGIN",
	L"BEGIN_FRAME",
	L"BEGIN_PARTITION",
	L"BETWEEN",
	L"BFILE",
	L"BIGINT",
	L"BINARY",
	L"BIT",
	L"BIT_LENGTH",
	L"BITMAP",
	L"BLOB",
	L"BLOCK",
	L"BODY",
	L"BOOLEAN",
	L"BOTH",
	L"BREAK",
	L"BROWSE",
	L"BULK",
	L"BY",
	L"CACHE",
	L"CACHE_INSTANCES",
	L"CALL",
	L"CALLED",
	L"CANCEL",
	L"CARDINALITY",
	L"CASCADE",
	L"CASCADED",
	L"CASE",
	L"CAST",
	L"CATALOG",
	L"CEIL",
	L"CEILING",
	L"CFILE",
	L"CHAINED",
	L"CHANGE",
	L"CHAR",
	L"CHAR_CS",
	L"CHAR_LENGTH",
	L"CHARACTER",
	L"CHARACTER_LENGTH",
	L"CHECK",
	L"CHECKPOINT",
	L"CHOOSE",
	L"CHUNK",
	L"CLEAR",
	L"CLOB",
	L"CLONE",
	L"CLOSE",
	L"CLOSE_CACHED_OPEN_CURSORS",
	L"CLUSTER",
	L"CLUSTERED",
	L"COALESCE",
	L"COLLATE",
	L"COLLATION",
	L"COLLECT",
	L"COLUMN",
	L"COLUMNS",
	L"COMMENT",
	L"COMMIT",
	L"COMMITTED",
	L"COMPATIBILITY",
	L"COMPILE",
	L"COMPLETE",
	L"COMPOSITE_LIMIT",
	L"COMPRESS",
	L"COMPUTE",
	L"CONCURRENTLY",
	L"CONDITION",
	L"CONNECT",
	L"CONNECT_TIME",
	L"CONNECTION",
	L"CONSTRAINT",
	L"CONSTRAINTS",
	L"CONTAINS",
	L"CONTAINSTABLE",
	L"CONTENTS",
	L"CONTINUE",
	L"CONTROLFILE",
	L"CONVERT",
	L"CORR",
	L"CORRESPONDING",
	L"COST",
	L"COUNT",
	L"COVAR_POP",
	L"COVAR_SAMP",
	L"CPU_PER_CALL",
	L"CPU_PER_SESSION",
	L"CREATE",
	L"CROSS",
	L"CUBE",
	L"CUME_DIST",
	L"CURREN_USER",
	L"CURRENT",
	L"CURRENT_CATALOG",
	L"CURRENT_DATE",
	L"CURRENT_DEFAULT_TRANSFORM_GROUP",
	L"CURRENT_PATH",
	L"CURRENT_ROLE",
	L"CURRENT_ROW",
	L"CURRENT_SCHEMA",
	L"CURRENT_TIME",
	L"CURRENT_TIMESTAMP",
	L"CURRENT_TRANSFORM_GROUP_FOR_TYPE",
	L"CURRENT_USER",
	L"CURSOR",
	L"CYCLE",
	L"DANGLING",
	L"DATABASE",
	L"DATABASES",
	L"DATAFILE",
	L"DATAFILES",
	L"DATALINK",
	L"DATAOBJNO",
	L"DATE",
	L"DAY",
	L"DAY_HOUR",
	L"DAY_MICROSECOND",
	L"DAY_MINUTE",
	L"DAY_SECOND",
	L"DBA",
	L"DBCC",
	L"DBHIGH",
	L"DBLOW",
	L"DBMAC",
	L"DEALLOCATE",
	L"DEBUG",
	L"DEC",
	L"DECIMAL",
	L"DECLARE",
	L"DEFAULT",
	L"DEFERRABLE",
	L"DEFERRED",
	L"DEGREE",
	L"DELAYED",
	L"DELETE",
	L"DENSE_RANK",
	L"DENY",
	L"DEREF",
	L"DESC",
	L"DESCRIBE",
	L"DESCRIPTOR",
	L"DETERMINISTIC",
	L"DIAGNOSTICS",
	L"DIRECTORY",
	L"DISABLE",
	L"DISCONNECT",
	L"DISK",
	L"DISMOUNT",
	L"DISTINCT",
	L"DISTINCTROW",
	L"DISTRIBUTED",
	L"DIV",
	L"DLNEWCOPY",
	L"DLPREVIOUSCOPY",
	L"DLURLCOMPLETE",
	L"DLURLCOMPLETEONLY",
	L"DLURLCOMPLETEWRITE",
	L"DLURLPATH",
	L"DLURLPATHONLY",
	L"DLURLPATHWRITE",
	L"DLURLSCHEME",
	L"DLURLSERVER",
	L"DLVALUE",
	L"DML",
	L"DO",
	L"DOMAIN",
	L"DOUBLE",
	L"DROP",
	L"DUAL",
	L"DUMP",
	L"DYNAMIC",
	L"EACH",
	L"ELEMENT",
	L"ELSE",
	L"ELSEIF",
	L"EMPTY",
	L"ENABLE",
	L"ENCLOSED",
	L"END",
	L"END_FRAME",
	L"END_PARTITION",
	L"END-EXEC",
	L"ENFORCE",
	L"ENTRY",
	L"EQUALS",
	L"ERRLVL",
	L"ESCAPE",
	L"ESCAPED",
	L"EVERY",
	L"EXCEPT",
	L"EXCEPTION",
	L"EXCEPTIONS",
	L"EXCHANGE",
	L"EXCLUDING",
	L"EXCLUSIVE",
	L"EXEC",
	L"EXECUTE",
	L"EXISTS",
	L"EXIT",
	L"EXP",
	L"EXPIRE",
	L"EXPLAIN",
	L"EXTENT",
	L"EXTENTS",
	L"EXTERNAL",
	L"EXTERNALLY",
	L"EXTRACT",
	L"FAILED_LOGIN_ATTEMPTS",
	L"FALSE",
	L"FAST",
	L"FETCH",
	L"FILE",
	L"FILLFACTOR",
	L"FILTER",
	L"FIRST",
	L"FIRST_ROWS",
	L"FIRST_VALUE",
	L"FLAGGER",
	L"FLOAT",
	L"FLOAT4",
	L"FLOAT8",
	L"FLOB",
	L"FLOOR",
	L"FLUSH",
	L"FOR",
	L"FORCE",
	L"FOREIGN",
	L"FORTRAN",
	L"FOUND",
	L"FRAME_ROW",
	L"FREE",
	L"FREELIST",
	L"FREELISTS",
	L"FREETEXT",
	L"FREETEXTTABLE",
	L"FREEZE",
	L"FROM",
	L"FULL",
	L"FULLTEXT",
	L"FUNCTION",
	L"FUSION",
	L"GENERATED",
	L"GET",
	L"GLOBAL",
	L"GLOBAL_NAME",
	L"GLOBALLY",
	L"GO",
	L"GOTO",
	L"GRANT",
	L"GROUP",
	L"GROUPING",
	L"GROUPS",
	L"HASH",
	L"HASHKEYS",
	L"HAVING",
	L"HEADER",
	L"HEAP",
	L"HIGH_PRIORITY",
	L"HOLD",
	L"HOLDLOCK",
	L"HOUR",
	L"HOUR_MICROSECOND",
	L"HOUR_MINUTE",
	L"HOUR_SECOND",
	L"IDENTIFIED",
	L"IDENTITY",
	L"IDENTITY_INSERT",
	L"IDENTITYCOL",
	L"IDGENERATORS",
	L"IDLE_TIME",
	L"IF",
	L"IGNORE",
	L"ILIKE",
	L"IMMEDIATE",
	L"IMPORT",
	L"IN",
	L"INCLUDE",
	L"INCLUDING",
	L"INCREMENT",
	L"IND_PARTITION",
	L"INDEX",
	L"INDEXED",
	L"INDEXES",
	L"INDICATOR",
	L"INFILE",
	L"INITIAL",
	L"INITIALLY",
	L"INITRANS",
	L"INNER",
	L"INOUT",
	L"INPUT",
	L"INSENSITIVE",
	L"INSERT",
	L"INSTANCE",
	L"INSTANCES",
	L"INSTEAD",
	L"INT",
	L"INT1",
	L"INT2",
	L"INT3",
	L"INT4",
	L"INT8",
	L"INTEGER",
	L"INTERMEDIATE",
	L"INTERSECT",
	L"INTERSECTION",
	L"INTERVAL",
	L"INTO",
	L"IO_AFTER_GTIDS",
	L"IO_BEFORE_GTIDS",
	L"IS",
	L"ISNULL",
	L"ISOLATION",
	L"ISOLATION_LEVEL",
	L"ITERATE",
	L"JOIN",
	L"JSON_TABLE",
	L"KEEP",
	L"KEY",
	L"KEYS",
	L"KILL",
	L"LABEL",
	L"LAG",
	L"LANGUAGE",
	L"LARGE",
	L"LAST",
	L"LAST_VALUE",
	L"LATERAL",
	L"LAYER",
	L"LEAD",
	L"LEADING",
	L"LEAVE",
	L"LEFT",
	L"LESS",
	L"LEVEL",
	L"LIBRARY",
	L"LIKE",
	L"LIKE_REGEX",
	L"LIMIT",
	L"LINEAR",
	L"LINENO",
	L"LINES",
	L"LINK",
	L"LIST",
	L"LN",
	L"LOAD",
	L"LOB",
	L"LOCAL",
	L"LOCALTIME",
	L"LOCALTIMESTAMP",
	L"LOCK",
	L"LOCKED",
	L"LOG",
	L"LOGFILE",
	L"LOGGING",
	L"LOGICAL_READS_PER_CALL",
	L"LOGICAL_READS_PER_SESSION",
	L"LONG",
	L"LONGBLOB",
	L"LONGTEXT",
	L"LOOP",
	L"LOW_PRIORITY",
	L"LOWER",
	L"MANAGE",
	L"MASTER",
	L"MASTER_BIND",
	L"MASTER_SSL_VERIFY_SERVER_CERT",
	L"MATCH",
	L"MAX",
	L"MAX_CARDINALITY",
	L"MAXARCHLOGS",
	L"MAXDATAFILES",
	L"MAXEXTENTS",
	L"MAXINSTANCES",
	L"MAXLOGFILES",
	L"MAXLOGHISTORY",
	L"MAXLOGMEMBERS",
	L"MAXSIZE",
	L"MAXTRANS",
	L"MAXVALUE",
	L"MEDIUMBLOB",
	L"MEDIUMINT",
	L"MEDIUMTEXT",
	L"MEMBER",
	L"MERGE",
	L"METHOD",
	L"MIDDLEINT",
	L"MIN",
	L"MINEXTENTS",
	L"MINIMUM",
	L"MINUS",
	L"MINUTE",
	L"MINUTE_MICROSECOND",
	L"MINUTE_SECOND",
	L"MINVALUE",
	L"MLS_LABEL_FORMAT",
	L"MLSLABEL",
	L"MOD",
	L"MODE",
	L"MODIFIES",
	L"MODIFY",
	L"MODULE",
	L"MONTH",
	L"MOUNT",
	L"MOVE",
	L"MTS_DISPATCHERS",
	L"MULTISET",
	L"NAMES",
	L"NATIONAL",
	L"NATURAL",
	L"NCHAR",
	L"NCHAR_CS",
	L"NCLOB",
	L"NEEDED",
	L"NESTED",
	L"NETWORK",
	L"NEW",
	L"NEXT",
	L"NO",
	L"NO_WRITE_TO_BINLOG",
	L"NOARCHIVELOG",
	L"NOAUDIT",
	L"NOCACHE",
	L"NOCHECK",
	L"NOCOMPRESS",
	L"NOCYCLE",
	L"NOFORCE",
	L"NOLOGGING",
	L"NOMAXVALUE",
	L"NOMINVALUE",
	L"NONCLUSTERED",
	L"NONE",
	L"NOORDER",
	L"NOOVERRIDE",
	L"NOPARALLEL",
	L"NOREVERSE",
	L"NORMAL",
	L"NORMALIZE",
	L"NOSORT",
	L"NOT",
	L"NOTHING",
	L"NOTNULL",
	L"NOWAIT",
	L"NTH_VALUE",
	L"NTILE",
	L"NULL",
	L"NULLIF",
	L"NUMBER",
	L"NUMERIC",
	L"NVARCHAR2",
	L"OBJECT",
	L"OBJNO",
	L"OBJNO_REUSE",
	L"OCCURRENCES_REGEX",
	L"OCTET_LENGTH",
	L"OF",
	L"OFF",
	L"OFFLINE",
	L"OFFSET",
	L"OFFSETS",
	L"OID",
	L"OIDINDEX",
	L"OLD",
	L"ON",
	L"ONLINE",
	L"ONLY",
	L"OPCODE",
	L"OPEN",
	L"OPENDATASOURCE",
	L"OPENQUERY",
	L"OPENROWSET",
	L"OPENXML",
	L"OPTIMAL",
	L"OPTIMIZE",
	L"OPTIMIZER_COSTS",
	L"OPTIMIZER_GOAL",
	L"OPTION",
	L"OPTIONALLY",
	L"OR",
	L"ORDER",
	L"ORGANIZATION",
	L"OSLABEL",
	L"OUT",
	L"OUTER",
	L"OUTFILE",
	L"OUTPUT",
	L"OVER",
	L"OVERFLOW",
	L"OVERLAPS",
	L"OVERLAY",
	L"OWN",
	L"PACKAGE",
	L"PAD",
	L"PARALLEL",
	L"PARAMETER",
	L"PARTIAL",
	L"PARTITION",
	L"PASCAL",
	L"PASSWORD",
	L"PASSWORD_GRACE_TIME",
	L"PASSWORD_LIFE_TIME",
	L"PASSWORD_LOCK_TIME",
	L"PASSWORD_REUSE_MAX",
	L"PASSWORD_REUSE_TIME",
	L"PASSWORD_VERIFY_FUNCTION",
	L"PCTFREE",
	L"PCTINCREASE",
	L"PCTTHRESHOLD",
	L"PCTUSED",
	L"PCTVERSION",
	L"PERCENT",
	L"PERCENT_RANK",
	L"PERCENTILE_CONT",
	L"PERCENTILE_DISC",
	L"PERIOD",
	L"PERMANENT",
	L"PERSIST",
	L"PERSIST_ONLY",
	L"PIVOT",
	L"PLACING",
	L"PLAN",
	L"PLSQL_DEBUG",
	L"PORTION",
	L"POSITION",
	L"POSITION_REGEX",
	L"POST_TRANSACTION",
	L"POWER",
	L"PRECEDES",
	L"PRECISION",
	L"PREPARE",
	L"PRESERVE",
	L"PRIMARY",
	L"PRINT",
	L"PRIOR",
	L"PRIVATE",
	L"PRIVATE_SGA",
	L"PRIVILEGE",
	L"PRIVILEGES",
	L"PROC",
	L"PROCEDURE",
	L"PROFILE",
	L"PUBLIC",
	L"PURGE",
	L"QUEUE",
	L"QUOTA",
	L"RAISERROR",
	L"RANGE",
	L"RANK",
	L"RAW",
	L"RBA",
	L"READ",
	L"READ_WRITE",
	L"READS",
	L"READTEXT",
	L"READUP",
	L"REAL",
	L"REBUILD",
	L"RECONFIGURE",
	L"RECOVER",
	L"RECOVERABLE",
	L"RECOVERY",
	L"RECURSIVE",
	L"REF",
	L"REFERENCES",
	L"REFERENCING",
	L"REFRESH",
	L"REGEXP",
	L"REGR_AVGX",
	L"REGR_AVGY",
	L"REGR_COUNT",
	L"REGR_INTERCEPT",
	L"REGR_R2",
	L"REGR_SLOPE",
	L"REGR_SXX",
	L"REGR_SXY",
	L"REGR_SYY",
	L"RELATIVE",
	L"RELEASE",
	L"RENAME",
	L"REPEAT",
	L"REPLACE",
	L"REPLICATION",
	L"REQUIRE",
	L"RESET",
	L"RESETLOGS",
	L"RESIGNAL",
	L"RESIZE",
	L"RESOURCE",
	L"RESTORE",
	L"RESTRICT",
	L"RESTRICTED",
	L"RESULT",
	L"RETURN",
	L"RETURNING",
	L"RETURNS",
	L"REUSE",
	L"REVERSE",
	L"REVERT",
	L"REVOKE",
	L"RIGHT",
	L"RLIKE",
	L"ROLE",
	L"ROLES",
	L"ROLLBACK",
	L"ROLLUP",
	L"ROW",
	L"ROW_NUMBER",
	L"ROWCOUNT",
	L"ROWGUIDCOL",
	L"ROWID",
	L"ROWNUM",
	L"ROWS",
	L"RULE",
	L"SAMPLE",
	L"SAVE",
	L"SAVEPOINT",
	L"SB4",
	L"SCAN_INSTANCES",
	L"SCHEMA",
	L"SCHEMAS",
	L"SCN",
	L"SCOPE",
	L"SCROLL",
	L"SD_ALL",
	L"SD_INHIBIT",
	L"SD_SHOW",
	L"SEARCH",
	L"SECOND",
	L"SECOND_MICROSECOND",
	L"SECTION",
	L"SECURITYAUDIT",
	L"SEG_BLOCK",
	L"SEG_FILE",
	L"SEGMENT",
	L"SELECT",
	L"SEMANTICKEYPHRASETABLE",
	L"SEMANTICSIMILARITYDETAILSTABLE",
	L"SEMANTICSIMILARITYTABLE",
	L"SENSITIVE",
	L"SEPARATOR",
	L"SEQUENCE",
	L"SERIALIZABLE",
	L"SESSION",
	L"SESSION_CACHED_CURSORS",
	L"SESSION_USER",
	L"SESSIONS_PER_USER",
	L"SET",
	L"SETUSER",
	L"SHARE",
	L"SHARED",
	L"SHARED_POOL",
	L"SHOW",
	L"SHRINK",
	L"SHUTDOWN",
	L"SIGNAL",
	L"SIMILAR",
	L"SIZE",
	L"SKIP",
	L"SKIP_UNUSABLE_INDEXES",
	L"SMALLINT",
	L"SNAPSHOT",
	L"SOME",
	L"SORT",
	L"SPACE",
	L"SPATIAL",
	L"SPECIFIC",
	L"SPECIFICATION",
	L"SPECIFICTYPE",
	L"SPLIT",
	L"SQL",
	L"SQL_BIG_RESULT",
	L"SQL_CALC_FOUND_ROWS",
	L"SQL_SMALL_RESULT",
	L"SQL_TRACE",
	L"SQLCA",
	L"SQLCODE",
	L"SQLERROR",
	L"SQLEXCEPTION",
	L"SQLSTATE",
	L"SQLWARNING",
	L"SQRT",
	L"SSL",
	L"STANDBY",
	L"START",
	L"STARTING",
	L"STATEMENT_ID",
	L"STATIC",
	L"STATISTICS",
	L"STDDEV_POP",
	L"STDDEV_SAMP",
	L"STOP",
	L"STORAGE",
	L"STORE",
	L"STORED",
	L"STRAIGHT_JOIN",
	L"STRUCTURE",
	L"SUBMULTISET",
	L"SUBSTRING",
	L"SUBSTRING_REGEX",
	L"SUCCEEDS",
	L"SUCCESSFUL",
	L"SUM",
	L"SWITCH",
	L"SYMMETRIC",
	L"SYNONYM",
	L"SYS_OP_ENFORCE_NOT_NULL$",
	L"SYS_OP_NTCIMG$",
	L"SYSDATE",
	L"SYSDBA",
	L"SYSOPER",
	L"SYSTEM",
	L"SYSTEM_TIME",
	L"SYSTEM_USER",
	L"TABLE",
	L"TABLES",
	L"TABLESAMPLE",
	L"TABLESPACE",
	L"TABLESPACE_NO",
	L"TABNO",
	L"TEMPORARY",
	L"TERMINATED",
	L"TEXTSIZE",
	L"THAN",
	L"THE",
	L"THEN",
	L"THREAD",
	L"TIME",
	L"TIMESTAMP",
	L"TIMEZONE_HOUR",
	L"TIMEZONE_MINUTE",
	L"TINYBLOB",
	L"TINYINT",
	L"TINYTEXT",
	L"TO",
	L"TOP",
	L"TOPLEVEL",
	L"TRACE",
	L"TRACING",
	L"TRAILING",
	L"TRAN",
	L"TRANSACTION",
	L"TRANSITIONAL",
	L"TRANSLATE",
	L"TRANSLATE_REGEX",
	L"TRANSLATION",
	L"TREAT",
	L"TRIGGER",
	L"TRIGGERS",
	L"TRIM",
	L"TRIM_ARRAY",
	L"TRUE",
	L"TRUNCATE",
	L"TRY_CONVERT",
	L"TSEQUAL",
	L"TX",
	L"TYPE",
	L"UB2",
	L"UBA",
	L"UESCAPE",
	L"UID",
	L"UNARCHIVED",
	L"UNDO",
	L"UNION",
	L"UNIQUE",
	L"UNKNOWN",
	L"UNLIMITED",
	L"UNLOCK",
	L"UNNEST",
	L"UNPIVOT",
	L"UNRECOVERABLE",
	L"UNSIGNED",
	L"UNTIL",
	L"UNUSABLE",
	L"UNUSED",
	L"UPDATABLE",
	L"UPDATE",
	L"UPDATETEXT",
	L"UPPER",
	L"USAGE",
	L"USE",
	L"USER",
	L"USING",
	L"UTC_DATE",
	L"UTC_TIME",
	L"UTC_TIMESTAMP",
	L"VALIDATE",
	L"VALIDATION",
	L"VALUE",
	L"VALUE_OF",
	L"VALUES",
	L"VAR_POP",
	L"VAR_SAMP",
	L"VARBINARY",
	L"VARCHAR",
	L"VARCHAR2",
	L"VARCHARACTER",
	L"VARIADIC",
	L"VARYING",
	L"VERBOSE",
	L"VERSIONING",
	L"VIEW",
	L"VIRTUAL",
	L"WAITFOR",
	L"WHEN",
	L"WHENEVER",
	L"WHERE",
	L"WHILE",
	L"WIDTH_BUCKET",
	L"WINDOW",
	L"WITH",
	L"WITHIN GROUP",
	L"WITHIN",
	L"WITHOUT",
	L"WORK",
	L"WRITE",
	L"WRITEDOWN",
	L"WRITETEXT",
	L"WRITEUP",
	L"XID",
	L"XML",
	L"XMLAGG",
	L"XMLATTRIBUTES",
	L"XMLBINARY",
	L"XMLCAST",
	L"XMLCOMMENT",
	L"XMLCONCAT",
	L"XMLDOCUMENT",
	L"XMLELEMENT",
	L"XMLEXISTS",
	L"XMLFOREST",
	L"XMLITERATE",
	L"XMLNAMESPACES",
	L"XMLPARSE",
	L"XMLPI",
	L"XMLQUERY",
	L"XMLSERIALIZE",
	L"XMLTABLE",
	L"XMLTEXT",
	L"XMLVALIDATE",
	L"XOR",
	L"YEAR",
	L"YEAR_MONTH",
	L"ZEROFILL",
	L"ZONE",
};
int g_nKeywordsPLSQL = _countof(g_ppszKeywordsPLSQL);
