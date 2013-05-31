/*!	@file
	@brief アウトライン解析

	@author genta
	@date	2004.08.08 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta
	Copyright (C) 2002, frozen
	Copyright (C) 2003, zenryaku
	Copyright (C) 2005, genta, D.S.Koba, じゅうじ

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <string.h>
#include "doc/CDocOutline.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h"
#include "_main/global.h"
#include "outline/CFuncInfoArr.h"
#include "charset/charcode.h"
#include "util/other_util.h"



/*! ルールファイルを読み込み、ルール構造体の配列を作成する

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca 引数nMaxCountを追加。バッファ長チェックをするように変更
*/
int CDocOutline::ReadRuleFile( const TCHAR* pszFilename, SOneRule* pcOneRule, int nMaxCount )
{
	long	i;
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	FILE*	pFile = _tfopen_absini( pszFilename, _T("r") );
	if( NULL == pFile ){
		return 0;
	}
	wchar_t	szLine[LINEREADBUFSIZE];
	const wchar_t*	pszDelimit = L" /// ";
	const wchar_t*	pszKeySeps = L",\0";
	wchar_t*	pszWork;
	int nDelimitLen = wcslen( pszDelimit );
	int nCount = 0;
	szLine[LINEREADBUFSIZE-1] = '\0';
	while( NULL != fgetws( szLine, _countof(szLine), pFile ) && nCount < nMaxCount ){
		pszWork = wcsstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != L';' ){
			*pszWork = L'\0';
			pszWork += nDelimitLen;

			/* 最初のトークンを取得します。 */
			wchar_t* pszToken = wcstok( szLine, pszKeySeps );
			while( NULL != pszToken ){
//				nRes = wcsicmp( pszKey, pszToken );
				int nLen = (int)wcslen(pszWork);
				for( i = 0; i < nLen; ++i ){
					if( WCODE::IsLineDelimiter(pszWork[i]) ){
						pszWork[i] = L'\0';
						break;
					}
				}
				wcsncpy( pcOneRule[nCount].szMatch, pszToken, 255 );
				wcsncpy( pcOneRule[nCount].szGroupName, pszWork, 255 );
				pcOneRule[nCount].szMatch[255] = L'\0';
				pcOneRule[nCount].szGroupName[255] = L'\0';
				pcOneRule[nCount].nLength = wcslen(pcOneRule[nCount].szMatch);
				nCount++;
				pszToken = wcstok( NULL, pszKeySeps );
			}
		}
		assert_warning( '\0' != szLine[LINEREADBUFSIZE-1] && '\n' != szLine[LINEREADBUFSIZE-1] );
		// TODO:後ろのデータ読み飛ばし
	}
	fclose( pFile );
	return nCount;
}

/*! ルールファイルを元に、トピックリストを作成

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca ネストの深さが最大値を超えるとバッファオーバーランするのを修正
		最大値以上は追加せずに無視する
	@date 2007.11.29 kobake SOneRule test[1024] でスタックが溢れていたのを修正
*/
void CDocOutline::MakeFuncList_RuleFile( CFuncInfoArr* pcFuncInfoArr )
{
	wchar_t*		pszText;

	/* ルールファイルの内容をバッファに読み込む */
	auto_array_ptr<SOneRule> test(new SOneRule[1024]);	// 1024個許可。 2007.11.29 kobake スタック使いすぎなので、ヒープに確保するように修正。
	int nCount = ReadRuleFile(m_pcDocRef->m_cDocType.GetDocumentAttribute().m_szOutlineRuleFilename, test.get(), 1024 );
	if ( nCount < 1 ){
		return;
	}

	/*	ネストの深さは、32レベルまで、ひとつのヘッダは、最長256文字まで区別
		（256文字まで同じだったら同じものとして扱います）
	*/
	const int	nMaxStack = 32;	//	ネストの最深
	int			nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t		pszStack[nMaxStack][256];
	wchar_t		szTitle[256];			//	一時領域
	for( CLogicInt nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//行取得
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			break;
		}

		//行頭の空白飛ばし
		int		i;
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == L' ' || pLine[i] == L'\t' || pLine[i] == L'　'){
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}

		//先頭文字が見出し記号のいずれかであれば、次へ進む
		int		j;
		for( j = 0; j < nCount; j++ ){
			if ( 0 == wcsncmp( &pLine[i], test[j].szMatch, test[j].nLength ) ){
				wcscpy( szTitle, test[j].szGroupName );
				break;
			}
		}
		if( j >= nCount ){
			continue;
		}

		/*	ルールにマッチした行は、アウトライン結果に表示する。
		*/

		//行文字列から改行を取り除く pLine -> pszText
		pszText = new wchar_t[nLineLen + 1];
		wmemcpy( pszText, &pLine[i], nLineLen );
		pszText[nLineLen] = L'\0';
		int nTextLen = wcslen( pszText );
		for( i = 0; i < nTextLen; ++i ){
			if( WCODE::IsLineDelimiter(pszText[i]) ){
				pszText[i] = L'\0';
				break;
			}
		}

		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		CLayoutPoint ptPos;
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		/* nDepthを計算 */
		int k;
		BOOL bAppend;
		bAppend = TRUE;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = wcscmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	ループ途中でbreak;してきた。＝今までに同じ見出しが存在していた。
			//	ので、同じレベルに合わせてAppendData.
			nDepth = k;
		}
		else if( nMaxStack> k ){
			//	いままでに同じ見出しが存在しなかった。
			//	ので、pszStackにコピーしてAppendData.
			wcscpy(pszStack[nDepth], szTitle);
		}else{
			// 2002.11.03 Moca 最大値を超えるとバッファオーバーランするから規制する
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if( FALSE != bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}



// From Here 2001.12.03 hor
/*! ブックマークリスト作成（無理矢理！）

	@date 2002.01.19 aroka 空行をマーク対象にするフラグ bMarkUpBlankLineEnable を導入しました。
	@date 2005.10.11 ryoji "ａ@" の右２バイトが全角空白と判定される問題の対処
	@date 2005.11.03 genta 文字列長修正．右端のゴミを除去
*/
void CDocOutline::MakeFuncList_BookMark( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nLineCount;
	int		leftspace, pos_wo_space, k;
	wchar_t*	pszText;
	BOOL	bMarkUpBlankLineEnable = GetDllShareData().m_Common.m_sOutline.m_bMarkUpBlankLineEnable;	//! 空行をマーク対象にするフラグ 20020119 aroka
	int		nNewLineLen	= m_pcDocRef->m_cDocEditor.m_cNewLineCode.GetLen();
	CLogicInt	nLineLast	= m_pcDocRef->m_cDocLineMgr.GetLineCount();
	int		nCharChars;

	for( nLineCount = CLogicInt(0); nLineCount <  nLineLast; ++nLineCount ){
		if(!CBookmarkGetter(m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)).IsBookmarked())continue;
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			break;
		}
		// Jan, 16, 2002 hor
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if( nLineLen<=nNewLineLen && nLineCount< nLineLast ){
			  continue;
			}
		}// LTrim
		for( leftspace = 0; leftspace < nLineLen; ++leftspace ){
			if( WCODE::IsBlank(pLine[leftspace]) ){
				continue;
			}
			break;
		}
		
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if(( leftspace >= nLineLen-nNewLineLen && nLineCount< nLineLast )||
				( leftspace >= nLineLen )) {
				continue;
			}
		}// RTrim
		// 2005.10.11 ryoji 右から遡るのではなく左から探すように修正（"ａ@" の右２バイトが全角空白と判定される問題の対処）
		k = pos_wo_space = leftspace;
		while( k < nLineLen ){
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, k );
			if( 1 == nCharChars ){
				if( !(WCODE::IsLineDelimiter(pLine[k]) ||
						pLine[k] == WCODE::SPACE ||
						pLine[k] == WCODE::TAB ||
						WCODE::IsZenkakuSpace(pLine[k]) ||
						pLine[k] == L'\0') )
					pos_wo_space = k + nCharChars;
			}
			k += nCharChars;
		}
		//	Nov. 3, 2005 genta 文字列長計算式の修正
		{
			int nLen = pos_wo_space - leftspace;
			pszText = new wchar_t[nLen + 1];
			wmemcpy( pszText, &pLine[leftspace], nLen );
			pszText[nLen] = L'\0';
		}
		CLayoutPoint ptXY;
		//int nX,nY
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(	CLogicPoint(CLogicInt(0), nLineCount), &ptXY );
		pcFuncInfoArr->AppendData( nLineCount+CLogicInt(1), ptXY.GetY2()+CLayoutInt(1) , pszText, 0 );
		delete [] pszText;
	}
	return;
}
// To Here 2001.12.03 hor
