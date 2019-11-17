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
#include "view/Colors/EColorIndexType.h"

/* COBOL */
void CType_Cobol::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, L"COBOL" );
	wcscpy( pType->m_szTypeExts, L"cbl,cpy,pco,cob" );	//Jun. 04, 2001 JEPRO KENCH氏の助言に従い追加

	//設定
	pType->m_cLineComment.CopyTo( 0, L"*", 6 );			//Jun. 02, 2001 JEPRO 修正
	pType->m_cLineComment.CopyTo( 1, L"D", 6 );			//Jun. 04, 2001 JEPRO 追加
	pType->m_nStringType = STRING_LITERAL_PLSQL;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"*" );				/* その他のインデント対象文字 */
	pType->m_nKeyWordSetIdx[0] = 3;						/* キーワードセット */		//Jul. 10, 2001 JEPRO
	pType->m_eDefaultOutline = OUTLINE_COBOL;			/* アウトライン解析方法 */
	// 指定桁縦線	//2005.11.08 Moca
	pType->m_ColorInfoArr[COLORIDX_VERTLINE].m_bDisp = true;
	pType->m_nVertLineIdx[0] = CKetaXInt(7);
	pType->m_nVertLineIdx[1] = CKetaXInt(8);
	pType->m_nVertLineIdx[2] = CKetaXInt(12);
	pType->m_nVertLineIdx[3] = CKetaXInt(73);
}

/*! COBOL アウトライン解析 */
void CDocOutline::MakeTopicList_cobol( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				k;
	wchar_t			szDivision[1024];
	wchar_t			szLabel[1024];
	const wchar_t*	pszKeyWord;
	int				nKeyWordLen;
	BOOL			bDivision;
	bool			bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	szDivision[0] = L'\0';
	szLabel[0] =  L'\0';

	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			break;
		}
		/* コメント行か */
		if( 7 <= nLineLen && pLine[6] == L'*' ){
			continue;
		}
		/* ラベル行か */
		if( 8 <= nLineLen && pLine[7] != L' ' ){
			k = 0;
			for( i = 7; i < nLineLen; ){
				if( pLine[i] == '.'
				 || WCODE::IsLineDelimiter(pLine[i], bExtEol)
				){
					break;
				}
				szLabel[k] = pLine[i];
				++k;
				++i;
				if( pLine[i - 1] == L' ' ){
					for( ; i < nLineLen; ++i ){
						if( pLine[i] != L' ' ){
							break;
						}
					}
				}
			}
			szLabel[k] = L'\0';
//			MYTRACE( L"szLabel=[%ls]\n", szLabel );

			pszKeyWord = L"division";
			nKeyWordLen = wcslen( pszKeyWord );
			bDivision = FALSE;
			int nLen = (int)wcslen( szLabel ) - nKeyWordLen;
			for( i = 0; i <= nLen ; ++i ){
				if( 0 == wmemicmp( &szLabel[i], pszKeyWord, nKeyWordLen ) ){
					szLabel[i + nKeyWordLen] = L'\0';
					wcscpy( szDivision, szLabel );
					bDivision = TRUE;
					break;
				}
			}
			if( bDivision ){
				continue;
			}
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/

			CLayoutPoint ptPos;
			wchar_t	szWork[1024];
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			auto_sprintf( szWork, L"%ls::%ls", szDivision, szLabel );
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , szWork, 0 );
		}
	}
	return;
}

//Jul. 10, 2001 JEPRO 追加
const wchar_t* g_ppszKeywordsCOBOL[] = {
	L"ACCEPT",
	L"ACCESS",
	L"ADD",
	L"ADDRESS",
	L"ADVANCING",
	L"AFTER",
	L"ALL",
	L"ALPHABET",
	L"ALPHABETIC",
	L"ALPHABETIC-LOWER",
	L"ALPHABETIC-UPPER",
	L"ALPHANUMERIC",
	L"ALPHANUMERIC-EDITED",
	L"ALSO",
	L"ALTER",
	L"ALTERNATIVE",
	L"AND",
	L"ANY",
	L"APPLY",
	L"ARE",
	L"AREA",
	L"AREAS",
	L"ARGUMENT",
	L"ASCENDING",
	L"ASSIGN",
	L"AT",
	L"AUTHOR",
	L"BASIS",
	L"BEFORE",
	L"BEGINNING",
	L"BINARY",
	L"BLANK",
	L"BLOCK",
	L"BOTTOM",
	L"BY",
	L"CALL",
	L"CANCEL",
	L"CBL",
	L"CD",
	L"CF",
	L"CH",
	L"CHARACTER",
	L"CHARACTERS",
	L"CLASS",
	L"CLASS-ID",
	L"CLOCK-UNITS",
	L"CLOSE",
	L"COBOL",
	L"CODE",
	L"CODE-SET",
	L"COLLATING",
	L"COLUMN",
	L"COM-REG",
	L"COMMA",
	L"COMMON",
	L"COMMUNICATION",
	L"COMP",
	L"COMP-1",
	L"COMP-2",
	L"COMP-3",
	L"COMP-4",
	L"COMP-5",
	L"COMPUTATIONAL",
	L"COMPUTATIONAL-1",
	L"COMPUTATIONAL-2",
	L"COMPUTATIONAL-3",
	L"COMPUTATIONAL-4",
	L"COMPUTATIONAL-5",
	L"COMPILED",
	L"COMPUTE",
	L"COMPUTER",
	L"CONFIGURATION",
	L"CONSOLE",
	L"CONTAINS",
	L"CONTENT",
	L"CONTINUE",
	L"CONTROL",
	L"CONTROLS",
	L"CONVERTING",
	L"COPY",
	L"CORR",
	L"CORRESPONDING",
	L"COUNT",
	L"CURRENCY",
	L"DATA",
	L"DATE-COMPILED",
	L"DATE-WRITTEN",
	L"DAY",
	L"DAY-OF-WEEK",
	L"DBCS",
	L"DE",
	L"DEBUG-CONTENTS",
	L"DEBUG-ITEM",
	L"DEBUG-LINE",
	L"DEBUG-NAME",
	L"DEBUG-SUB-1",
	L"DEBUG-SUB-2",
	L"DEBUG-SUB-3",
	L"DEBUGGING",
	L"DECIMAL-POINT",
	L"DECLARATIVES",
	L"DELETE",
	L"DELIMITED",
	L"DELIMITER",
	L"DEPENDING",
	L"DESCENDING",
	L"DESTINATION",
	L"DETAIL",
	L"DISPLAY",
	L"DISPLAY-1",
	L"DIVIDE",
	L"DIVISION",
	L"DOWN",
	L"DUPLICATES",
	L"DYNAMIC",
	L"EGCS",
	L"EGI",
	L"EJECT",
	L"ELSE",
	L"EMI",
	L"ENABLE",
	L"END",
	L"END-ADD",
	L"END-CALL",
	L"END-COMPUTE",
	L"END-DELETE",
	L"END-DIVIDE",
	L"END-EVALUATE",
	L"END-IF",
	L"END-INVOKE",
	L"END-MULTIPLY",
	L"END-OF-PAGE",
	L"END-PERFORM",
	L"END-READ",
	L"END-RECEIVE",
	L"END-RETURN",
	L"END-REWRITE",
	L"END-SEARCH",
	L"END-START",
	L"END-STRING",
	L"END-SUBTRACT",
	L"END-UNSTRING",
	L"END-WRITE",
	L"ENDING",
	L"ENTER",
	L"ENTRY",
	L"ENVIRONMENT",
	L"EOP",
	L"EQUAL",
	L"ERROR",
	L"ESI",
	L"EVALUATE",
	L"EVERY",
	L"EXAMINE",
	L"EXCEPTION",
	L"EXIT",
	L"EXTEND",
	L"EXTERNAL",
	L"FALSE",
	L"FD",
	L"FILE",
	L"FILE-CONTROL",
	L"FILLER",
	L"FINAL",
	L"FIRST",
	L"FOOTING",
	L"FOR",
	L"FROM",
	L"FUNCTION",
	L"GENERATE",
	L"GIVING",
	L"GLOBAL",
	L"GO",
	L"GOBACK",
	L"GREATER",
	L"GROUP",
	L"HEADING",
	L"HIGH-VALUE",
	L"HIGH-VALUES",
	L"IDENTIFICATION",
	L"I-O",
	L"I-O-CONTROL",
	L"ID",
	L"IDENTIFICATION",
	L"IF",
	L"IN",
	L"INDEX",
	L"INDEXED",
	L"INDICATE",
	L"INHERITS",
	L"INITIAL",
	L"INITIALIZE",
	L"INITIATE",
	L"INPUT",
	L"INPUT-OUTPUT",
	L"INSERT",
	L"INSPECT",
	L"INSTALLATION",
	L"INTO",
	L"INVALID",
	L"INVOKE",
	L"IS",
	L"JUST",
	L"JUSTIFIED",
	L"KANJI",
	L"KEY",
	L"LABEL",
	L"LAST",
	L"LEADING",
	L"LEFT",
	L"LENGTH",
	L"LESS",
	L"LIMIT",
	L"LIMITS",
	L"LINKAGE",
	L"LINAGE-COUNTER",
	L"LINE",
	L"LINE-COUNTER",
	L"LINES",
	L"LINKAGE",
	L"LOCAL-STORAGE",
	L"LOCK",
	L"LOW-VALUE",
	L"LOW-VALUES",
	L"MEMORY",
	L"MERGE",
	L"MESSAGE",
	L"METACLASS",
	L"METHOD",
	L"METHOD-ID",
	L"MODE",
	L"MODULES",
	L"MORE-LABELS",
	L"MOVE",
	L"MULTIPLE",
	L"MULTIPLY",
	L"NATIVE",
	L"NATIVE_BINARY",
	L"NEGATIVE",
	L"NEXT",
	L"NO",
	L"NOT",
	L"NULL",
	L"NULLS",
	L"NUMBER",
	L"NUMERIC",
	L"NUMERIC-EDITED",
	L"OBJECT",
	L"OBJECT-COMPUTER",
	L"OCCURS",
	L"OF",
	L"OFF",
	L"OMITTED",
	L"ON",
	L"OPEN",
	L"OPTIONAL",
	L"OR",
	L"ORDER",
	L"ORGANIZATION",
	L"OTHER",
	L"OUTPUT",
	L"OVERFLOW",
	L"OVERRIDE",
	L"PACKED-DECIMAL",
	L"PADDING",
	L"PAGE",
	L"PAGE-COUNTER",
	L"PASSWORD",
	L"PERFORM",
	L"PF",
	L"PH",
	L"PIC",
	L"PICTURE",
	L"PLUS",
	L"POINTER",
	L"POSITION",
	L"POSITIVE",
	L"PRINTING",
	L"PROCEDURE",
	L"PROCEDURE-POINTER",
	L"PROCEDURES",
	L"PROCEED",
	L"PROCESSING",
	L"PROGRAM",
	L"PROGRAM-ID",
	L"PURGE",
	L"QUEUE",
	L"QUOTE",
	L"QUOTES",
	L"RANDAM",
	L"RD",
	L"READ",
	L"READY",
	L"RECEIVE",
	L"RECORD",
	L"RECORDING",
	L"RECORDS",
	L"RECURSIVE",
	L"REDEFINES",
	L"REEL",
	L"REFERENCE",
	L"REFERENCES",
	L"RELATIVE",
	L"RELEASE",
	L"RELOAD",
	L"REMAINDER",
	L"REMARKS",
	L"REMOVAL",
	L"RENAMES",
	L"REPLACE",
	L"REPLACING",
	L"REPORT",
	L"REPORTING",
	L"REPORTS",
	L"REPOSITORY",
	L"RERUN",
	L"RESERVE",
	L"RESET",
	L"RETURN",
	L"RETURN-CODE",
	L"RETURNING",
	L"REVERSED",
	L"REWIND",
	L"REWRITE",
	L"RF",
	L"RH",
	L"RIGHT",
	L"ROLLBACK",
	L"ROUNDED",
	L"RUN",
	L"SAME",
	L"SD",
	L"SEARCH",
	L"SECTION",
	L"SECURITY",
	L"SEGMENT",
	L"SEGMENT-LIMIT",
	L"SELECT",
	L"SELF",
	L"SEND",
	L"SENTENCE",
	L"SEPARATE",
	L"SEQUENCE",
	L"SEQUENTIAL",
	L"SERVICE",
	L"SET",
	L"SHIFT-IN",
	L"SHIFT-OUT",
	L"SIGN",
	L"SIZE",
	L"SKIP1",
	L"SKIP2",
	L"SKIP3",
	L"SORT",
	L"SORT-CONTROL",
	L"SORT-CORE-SIZE",
	L"SORT-FILE-SIZE",
	L"SORT-MERGE",
	L"SORT-MESSAGE",
	L"SORT-MODE-SIZE",
	L"SORT-RETURN",
	L"SOURCE",
	L"SOURCE-COMPUTER",
	L"SPACE",
	L"SPACES",
	L"SPECIAL-NAMES",
	L"STANDARD",
	L"STANDARD-1",
	L"STANDARD-2",
	L"START",
	L"STATUS",
	L"STOP",
	L"STORAGE",
	L"STRING",
	L"SUB-QUEUE-1",
	L"SUB-QUEUE-2",
	L"SUB-QUEUE-3",
	L"SUBTRACT",
	L"SUM",
	L"SUPER",
	L"SUPPRESS",
	L"SYMBOLIC",
	L"SYNC",
	L"SYNCHRONIZED",
	L"SYSOUT",
	L"TABLE",
	L"TALLY",
	L"TALLYING",
	L"TAPE",
	L"TERMINAL",
	L"TERMINATE",
	L"TEST",
	L"THAN",
	L"THEN",
	L"THROUGH",
	L"THRU",
	L"TIME",
	L"TIMES",
	L"TITLE",
	L"TO",
	L"TODAY",
	L"TOP",
	L"TRACE",
	L"TRAILING",
	L"TRUE",
	L"TYPE",
	L"TRANSFORM",
	L"UNIT",
	L"UNSTRING",
	L"UNTIL",
	L"UP",
	L"UPON",
	L"USAGE",
	L"USE",
	L"USING",
	L"VALUE",
	L"VALUES",
	L"VARYING",
	L"WHEN",
	L"WHEN-COMPILED",
	L"WITH",
	L"WORDS",
	L"WORKING",
	L"WORKING-STORAGE",
	L"WRITE",
	L"WRITE-ONLY",
	L"WRITTEN",
	L"ZERO"
	L"ZEROES"
	L"ZEROS"
};
int g_nKeywordsCOBOL = _countof(g_ppszKeywordsCOBOL);
