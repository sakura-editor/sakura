#include "stdafx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "CFuncInfoArr.h"

/* Perl */
//Jul. 08, 2001 JEPRO Perl ユーザに贈る
//Jul. 08, 2001 JEPRO 追加
void CType_Perl::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("Perl") );
	_tcscpy( pType->m_szTypeExts, _T("cgi,pl,pm") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"#", -1 );					/* 行コメントデリミタ */
	pType->m_nDefaultOutline = OUTLINE_PERL;						/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0]  = 11;								/* キーワードセット */
	pType->m_nKeyWordSetIdx[1] = 12;								/* キーワードセット2 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			/* 半角数値を色分け表示 */
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;	//対括弧の強調をデフォルトON	//Sep. 21, 2002 genta
}



//	From Here Sep 8, 2000 genta
//
//!	Perl用アウトライン解析機能（簡易版）
/*!
	単純に /^\\s*sub\\s+(\\w+)/ に一致したら $1を取り出す動作を行う。
	ネストとかは面倒くさいので考えない。
	package{ }を使わなければこれで十分．無いよりはまし。

	@par nModeの意味
	@li 0: はじめ
	@li 2: subを見つけた後
	@li 1: 単語読み出し中

	@date 2005.06.18 genta パッケージ区切りを表す ::と'を考慮するように
*/
void CDocOutline::MakeFuncList_Perl( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt			nLineLen;
	int			i;
	int			nCharChars;
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;

	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		nMode = 0;
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				break;
			}

			/* 単語読み込み中 */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}
				if( 's' != pLine[i] )
					break;
				//	sub の一文字目かもしれない
				if( nLineLen - i < 4 )
					break;
				if( wcsncmp_literal( pLine + i, L"sub" ) )
					break;
				int c = pLine[ i + 3 ];
				if( c == L' ' || c == L'\t' ){
					nMode = 2;	//	発見
					i += 3;
				}
				else
					break;
			}
			else if( 2 == nMode ){
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}
				if( L'_' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
				){
					//	関数名の始まり
					nWordIdx = 0;
					szWord[nWordIdx] = pLine[i];
					szWord[nWordIdx + 1] = L'\0';
					nMode = 1;
					continue;
				}
				else
					break;

			}
			else if( 1 == nMode ){
				if( L'_' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					//	Jun. 18, 2005 genta パッケージ修飾子を考慮
					//	コロンは2つ連続しないといけないのだが，そこは手抜き
					L':' == pLine[i] || L'\'' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						break;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
					}
				}else{
					//	関数名取得
					/*
					  カーソル位置変換
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					  →
					  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
					*/
					CLayoutPoint ptPosXY;
					m_pcDocRef->m_cLayoutMgr.LogicToLayout(
						CLogicPoint(CLogicInt(0), nLineCount),
						&ptPosXY
					);
					//	Mar. 9, 2001
					pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPosXY.GetY2() + CLayoutInt(1), szWord, 0 );

					break;
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}
//	To HERE Sep. 8, 2000 genta

