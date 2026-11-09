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
#include "view/colors/EColorIndexType.h"

/* COBOL */
void CType_Cobol::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, LTEXT("COBOL") );
	wcscpy( pType->m_szTypeExts, LTEXT("cbl,cpy,pco,cob") );	//Jun. 04, 2001 JEPRO KENCH氏の助言に従い追加

	//設定
	pType->m_cLineComment.CopyTo( 0, LTEXT("*"), 6 );			//Jun. 02, 2001 JEPRO 修正
	pType->m_cLineComment.CopyTo( 1, LTEXT("D"), 6 );			//Jun. 04, 2001 JEPRO 追加
	pType->m_nStringType = STRING_LITERAL_PLSQL;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, LTEXT("*") );				/* その他のインデント対象文字 */
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
	const WCHAR*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				k;
	WCHAR			szDivision[1024];
	WCHAR			szLabel[1024];
	const WCHAR*	pszKeyWord;
	int				nKeyWordLen;
	BOOL			bDivision;
	bool			bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	szDivision[0] = LCHAR('\0');
	szLabel[0] =  LCHAR('\0');

	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( nullptr == pLine ){
			break;
		}
		/* コメント行か */
		if( 7 <= nLineLen && pLine[6] == LCHAR('*') ){
			continue;
		}
		/* ラベル行か */
		if( 8 <= nLineLen && pLine[7] != LCHAR(' ') ){
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
				if( pLine[i - 1] == LCHAR(' ') ){
					for( ; i < nLineLen; ++i ){
						if( pLine[i] != LCHAR(' ') ){
							break;
						}
					}
				}
			}
			szLabel[k] = LCHAR('\0');
//			MYTRACE( L"szLabel=[%ls]\n", szLabel );

			pszKeyWord = LTEXT("division");
			nKeyWordLen = (int)wcslen( pszKeyWord );
			bDivision = FALSE;
			int nLen = (int)wcslen( szLabel ) - nKeyWordLen;
			for( i = 0; i <= nLen ; ++i ){
				if( 0 == wmemicmp( &szLabel[i], pszKeyWord, nKeyWordLen ) ){
					::wcsncpy_s( szDivision, szLabel, i + nKeyWordLen - 1);
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
			WCHAR	szWork[1024];
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			auto_sprintf( szWork, LTEXT("%ls::%ls"), szDivision, szLabel );
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , szWork, 0 );
		}
	}
	return;
}

//Jul. 10, 2001 JEPRO 追加
const WCHAR* g_ppszKeywordsCOBOL[] = {
	LTEXT("ACCEPT"),
	LTEXT("ADD"),
	LTEXT("ADVANCING"),
	LTEXT("AFTER"),
	LTEXT("ALL"),
	LTEXT("AND"),
	LTEXT("ARGUMENT"),
	LTEXT("ASSIGN"),
	LTEXT("AUTHOR"),
	LTEXT("BEFORE"),
	LTEXT("BLOCK"),
	LTEXT("BY"),
	LTEXT("CALL"),
	LTEXT("CHARACTERS"),
	LTEXT("CLOSE"),
	LTEXT("COMP"),
	LTEXT("COMPILED"),
	LTEXT("COMPUTE"),
	LTEXT("COMPUTER"),
	LTEXT("CONFIGURATION"),
	LTEXT("CONSOLE"),
	LTEXT("CONTAINS"),
	LTEXT("CONTINUE"),
	LTEXT("CONTROL"),
	LTEXT("COPY"),
	LTEXT("DATA"),
	LTEXT("DELETE"),
	LTEXT("DISPLAY"),
	LTEXT("DIVIDE"),
	LTEXT("DIVISION"),
	LTEXT("ELSE"),
	LTEXT("END"),
	LTEXT("ENVIRONMENT"),
	LTEXT("EVALUATE"),
	LTEXT("EXAMINE"),
	LTEXT("EXIT"),
	LTEXT("EXTERNAL"),
	LTEXT("FD"),
	LTEXT("FILE"),
	LTEXT("FILLER"),
	LTEXT("FROM"),
	LTEXT("GIVING"),
	LTEXT("GO"),
	LTEXT("GOBACK"),
	LTEXT("HIGH-VALUE"),
	LTEXT("IDENTIFICATION"),
	LTEXT("IF"),
	LTEXT("INITIALIZE"),
	LTEXT("INPUT"),
	LTEXT("INTO"),
	LTEXT("IS"),
	LTEXT("LABEL"),
	LTEXT("LINKAGE"),
	LTEXT("LOW-VALUE"),
	LTEXT("MODE"),
	LTEXT("MOVE"),
	LTEXT("NOT"),
	LTEXT("OBJECT"),
	LTEXT("OCCURS"),
	LTEXT("OF"),
	LTEXT("ON"),
	LTEXT("OPEN"),
	LTEXT("OR"),
	LTEXT("OTHER"),
	LTEXT("OUTPUT"),
	LTEXT("PERFORM"),
	LTEXT("PIC"),
	LTEXT("PROCEDURE"),
	LTEXT("PROGRAM"),
	LTEXT("READ"),
	LTEXT("RECORD"),
	LTEXT("RECORDING"),
	LTEXT("REDEFINES"),
	LTEXT("REMAINDER"),
	LTEXT("REMARKS"),
	LTEXT("REPLACING"),
	LTEXT("REWRITE"),
	LTEXT("ROLLBACK"),
	LTEXT("SECTION"),
	LTEXT("SELECT"),
	LTEXT("SOURCE"),
	LTEXT("SPACE"),
	LTEXT("STANDARD"),
	LTEXT("STOP"),
	LTEXT("STORAGE"),
	LTEXT("SYSOUT"),
	LTEXT("TEST"),
	LTEXT("THEN"),
	LTEXT("TO"),
	LTEXT("TODAY"),
	LTEXT("TRANSFORM"),
	LTEXT("UNTIL"),
	LTEXT("UPON"),
	LTEXT("USING"),
	LTEXT("VALUE"),
	LTEXT("VARYING"),
	LTEXT("WHEN"),
	LTEXT("WITH"),
	LTEXT("WORKING"),
	LTEXT("WRITE"),
	LTEXT("WRITTEN"),
	LTEXT("ZERO")
};
int g_nKeywordsCOBOL = int(std::size(g_ppszKeywordsCOBOL));
