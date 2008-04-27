#include "stdafx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "CFuncInfoArr.h"

/* COBOL */
void CType_Cobol::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("COBOL") );
	_tcscpy( pType->m_szTypeExts, _T("cbl,cpy,pco,cob") );	//Jun. 04, 2001 JEPRO KENCH氏の助言に従い追加

	//設定
	pType->m_cLineComment.CopyTo( 0, L"*", 6 );			//Jun. 02, 2001 JEPRO 修正
	pType->m_cLineComment.CopyTo( 1, L"D", 6 );			//Jun. 04, 2001 JEPRO 追加
	pType->m_nStringType = 1;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"*" );				/* その他のインデント対象文字 */
	pType->m_nKeyWordSetIdx[0] = 3;						/* キーワードセット */		//Jul. 10, 2001 JEPRO
	pType->m_nDefaultOutline = OUTLINE_COBOL;			/* アウトライン解析方法 */
	// 指定桁縦線	//2005.11.08 Moca
	pType->m_ColorInfoArr[COLORIDX_VERTLINE].m_bDisp = true;
	pType->m_nVertLineIdx[0] = CLayoutInt(7);
	pType->m_nVertLineIdx[1] = CLayoutInt(8);
	pType->m_nVertLineIdx[2] = CLayoutInt(12);
	pType->m_nVertLineIdx[3] = CLayoutInt(73);
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
				 || pLine[i] == WCODE::CR
				 || pLine[i] == WCODE::LF
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
//			MYTRACE_A( "szLabel=[%ls]\n", szLabel );



			pszKeyWord = L"division";
			nKeyWordLen = wcslen( pszKeyWord );
			bDivision = FALSE;
			for( i = 0; i <= (int)wcslen( szLabel ) - nKeyWordLen; ++i ){
				if( 0 == auto_memicmp( &szLabel[i], pszKeyWord, nKeyWordLen ) ){
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

