#include "stdafx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "CFuncInfoArr.h"

/* Java */
void CType_Java::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("Java") );
	_tcscpy( pType->m_szTypeExts, _T("java,jav") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );					/* 行コメントデリミタ */
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"/*", L"*/" );	/* ブロックコメントデリミタ */
	pType->m_nKeyWordSetIdx[0] = 4;									/* キーワードセット */
	pType->m_nDefaultOutline = OUTLINE_JAVA;						/* アウトライン解析方法 */
	pType->m_nSmartIndent = SMARTINDENT_CPP;						/* スマートインデント種別 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			//半角数値を色分け表示	//Mar. 10, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;	//対括弧の強調をデフォルトONに	//Sep. 21, 2002 genta
}



/* Java関数リスト作成 */
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
	int			nMode;
	wchar_t		szFuncName[80];
	CLogicInt	nFuncLine = CLogicInt(0);
	int			nFuncId;
	int			nFuncNum;
	wchar_t		szClass[1024];

	int			nClassNestArr[16];
	int			nClassNestArrNum;
	int			nNestLevel2Arr[16];

	nNestLevel = 0;
	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	nNestLevel2Arr[0] = 0;
	nFuncNum = 0;
	szClass[0] = L'\0';
	nClassNestArrNum = 0;
	CLogicInt		nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			//nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				i += (nCharChars - 1);
				continue;
			}

			/* エスケープシーケンスは常に取り除く */
			if( L'\\' == pLine[i] ){
				++i;
			}else
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				if( L'\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* ダブルクォーテーション文字列読み込み中 */
			if( 21 == nMode ){
				if( L'"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* コメント読み込み中 */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}
			/* 単語読み込み中 */
			else if( 1 == nMode ){
				if( L'_' == pLine[i] ||
					L':' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'.' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}
else{
					/* クラス宣言部分を見つけた */
					//	Oct. 10, 2002 genta interfaceも対象に
					if( 0 == wcscmp( L"class", szWordPrev ) ||
						0 == wcscmp( L"interface", szWordPrev )
					 ){
						nClassNestArr[nClassNestArrNum] = nNestLevel;
						++nClassNestArrNum;
						if( 0 < nNestLevel	){
							wcscat( szClass, L"\\" );
						}
						wcscat( szClass, szWord );

						nFuncId = 0;
						++nFuncNum;
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						CLogicPoint  ptPosXY_Logic = CLogicPoint(CLogicInt(0), nLineCount);
						CLayoutPoint ptPosXY_Layout;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(
							ptPosXY_Logic,
							&ptPosXY_Layout
						);
						wchar_t szWork[256];
						auto_sprintf( szWork, L"%ls::%ls", szClass, L"定義位置" );
						pcFuncInfoArr->AppendData( ptPosXY_Logic.GetY2() + CLogicInt(1), ptPosXY_Layout.GetY2() + CLayoutInt(1), szWork, nFuncId ); //2007.10.09 kobake レイアウト・ロジックの混在バグ修正
					}

					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( 2 == nMode ){
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
					nMode = 0;
					i--;
					continue;
				}else{
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
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
					nMode = 8;
					continue;
				}else
				if( L'\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( L'"' == pLine[i] ){
					nMode = 21;
					continue;
				}else
				if( L'{' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						//	メソッド中でさらにメソッドを定義することはないので
						//	ネストレベル判定追加 class/interfaceの直下の場合のみ判定する
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
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							CLayoutPoint ptPosXY;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(CLogicInt(0), nFuncLine - CLogicInt(1)),
								&ptPosXY
							);
							wchar_t szWork[256];
							auto_sprintf( szWork, L"%ls::%ls", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, ptPosXY.GetY2() + CLayoutInt(1), szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					++nNestLevel;
					nMode = 0;
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
					nMode = 0;
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
					nMode = 0;
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
						//	abscract にも対応
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
					nMode = 0;
					continue;
				}else
				if( L';' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						// 関数の中で別の関数の宣言部を使うことって，Javaであるの？
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
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							CLayoutPoint ptPosXY;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(CLogicInt(0), nFuncLine - CLogicInt(1)),
								&ptPosXY
							);
							wchar_t szWork[256];
							auto_sprintf( szWork, L"%ls::%ls", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, ptPosXY.GetY2() + CLayoutInt(1), szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( L'_' == pLine[i] ||
						L':' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
						L'.' == pLine[i]
					){
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
						nMode = 1;
					}else{
						nMode = 0;
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
