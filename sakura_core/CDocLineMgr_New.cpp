/*!	@file
	@brief 行データの管理

	@author Norio Nakatani
	@date 1999/12/17  新規作成
	@date 2001/12/03  hor Bookmark機能追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, hor, aroka, MIK, Moca, genta
	Copyright (C) 2003, Moca, genta
	Copyright (C) 2005, かろと

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "charcode.h"
#include "CDocLineMgr.h"
#include "debug.h"
#include "charcode.h"
#include <commctrl.h>
#include "global.h"
#include "CRunningTimer.h"
#include "CDlgCancel.h"
#include "CDocLine.h"// 2002/2/10 aroka
#include "CMemory.h"// 2002/2/10 aroka
#include "CBregexp.h"// 2002/2/10 aroka
#include "sakura_rc.h"// 2002/2/10 aroka
#include "my_icmp.h" // 2002/11/30 Moca 追加

/* 指定範囲のデータを置換(削除 & データを挿入)
  Fromを含む位置からToの直前を含むデータを削除する
  Fromの位置へテキストを挿入する
*/
void CDocLineMgr::ReplaceData( DocLineReplaceArg* pArg )
{
	MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::ReplaceData()" );

	/* 挿入によって増えた行の数 */
	pArg->nInsLineNum = CLogicInt(0);
	/* 削除した行の総数 */
	pArg->nDeletedLineNum = CLogicInt(0);
	/* 削除されたデータ */
	pArg->pcmemDeleted->SetString(L"");

	CDocLine* pCDocLine;
	CDocLine* pCDocLinePrev;
	CDocLine* pCDocLineNext;
	CDocLine* pCDocLineNew;
	int nWorkPos;
	int nWorkLen;
	const wchar_t* pLine;
	int nLineLen;
	int i;
	int			nBgn;
	int			nPos;
	CLogicInt	nAllLinesOld;
	int			nCount;
	int			nProgress;
	CNativeW	cmemPrevLine;
	CNativeW	cmemCurLine;
	CNativeW	cmemNextLine;
	//	May 15, 2000
	CEOL cEOLType;
	CEOL cEOLTypeNext;
	HWND		hwndCancel = NULL;	//	初期化
	HWND		hwndProgress = NULL;	//	初期化

	pArg->ptNewPos = pArg->sDelRange.GetFrom();

	/* 大量のデータを操作するとき */
	CDlgCancel*	pCDlgCancel = NULL;
	if( 3000 < pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y
	 || 1024000 < pArg->nInsDataLen
	){

		/* 進捗ダイアログの表示 */
		pCDlgCancel = new CDlgCancel;
		if( NULL != ( hwndCancel = pCDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
			::SendMessageAny( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
 			::SendMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
		}
	}


	// 削除データの取得のループ
	/* 前から処理していく */
	/* 現在行の情報を得る */
	pCDocLine = GetLineInfo( pArg->sDelRange.GetFrom().GetY2() );
	for( i = pArg->sDelRange.GetFrom().y; i <= pArg->sDelRange.GetTo().y && NULL != pCDocLine; i++ ){
		pLine = pCDocLine->m_cLine.GetStringPtr(); // 2002/2/10 aroka CMemory変更
		nLineLen = pCDocLine->m_cLine.GetStringLength(); // 2002/2/10 aroka CMemory変更
		pCDocLinePrev = pCDocLine->m_pPrev;
		pCDocLineNext = pCDocLine->m_pNext;
		/* 現在行の削除開始位置を調べる */
		if( i == pArg->sDelRange.GetFrom().y ){
			nWorkPos = pArg->sDelRange.GetFrom().x;
		}else{
			nWorkPos = 0;
		}
		/* 現在行の削除データ長を調べる */
		if( i == pArg->sDelRange.GetTo().y ){
			nWorkLen = pArg->sDelRange.GetTo().x - nWorkPos;
		}else{
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory変更
		}
		if( 0 == nWorkLen ){
			/* 前の行へ */
			goto next_line;
		}
		if( 0 > nWorkLen ){
			::MYMESSAGEBOX_A(	NULL, MB_OK | MB_ICONINFORMATION,
				"作者に教えて欲しいエラー",
				"CDocLineMgr::ReplaceData()\n"
				"\n"
				"0 > nWorkLen\nnWorkLen=%d\n"
				"i=%d\n"
				"pArg->sDelRange.GetTo().y=%d", nWorkLen, i, pArg->sDelRange.GetTo().y
			);
		}

		/* 改行も削除するんかぃのぉ・・・？ */
		if( EOL_NONE != pCDocLine->m_cEol &&
			nWorkPos + nWorkLen > nLineLen - pCDocLine->m_cEol.GetLen() // 2002/2/10 aroka CMemory変更
		){
			/* 削除する長さに改行も含める */
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory変更
		}
		/* 削除されたデータを保存 */
		// 2002/2/10 aroka from here CMemory変更 念のため。
		if( pLine != pCDocLine->m_cLine.GetStringPtr() ){
			::MYMESSAGEBOX_A(	NULL, MB_OK | MB_ICONINFORMATION, "作者に教えて欲しいエラー",
				"CDocLineMgr::ReplaceData()\n"
				"\n"
				"pLine != pCDocLine->m_cLine.GetPtr() =%d\n"
				"i=%d\n"
				"pArg->sDelRange.GetTo().y=%d",
				pLine, i, pArg->sDelRange.GetTo().y
			);
		}

		pArg->pcmemDeleted->AppendString( &pLine[nWorkPos], nWorkLen );

next_line:;
		/* 次の行のオブジェクトのポインタ */
		pCDocLine = pCDocLineNext;
		/* 最近参照した行番号と行データ */
		++m_nPrevReferLine;
		m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (i - pArg->sDelRange.GetFrom().y) && ( 0 == ((i - pArg->sDelRange.GetFrom().y) % 32)) ){
				nProgress = (i - pArg->sDelRange.GetFrom().y) * 100 / (pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y) / 2;
				::SendMessageAny( hwndProgress, PBM_SETPOS, nProgress, 0 );

			}
		}
	} // 削除データの取得のループ


	/* 現在行の情報を得る */
	pCDocLine = GetLineInfo( pArg->sDelRange.GetTo().GetY2() );
	i = pArg->sDelRange.GetTo().y;
	if( 0 < pArg->sDelRange.GetTo().y && NULL == pCDocLine ){
		pCDocLine = GetLineInfo( pArg->sDelRange.GetTo().GetY2() - CLogicInt(1) );
		i--;
	}
	/* 後ろから処理していく */
	for( ; i >= pArg->sDelRange.GetFrom().y && NULL != pCDocLine; i-- ){
		pLine = pCDocLine->m_cLine.GetStringPtr(); // 2002/2/10 aroka CMemory変更
		nLineLen = pCDocLine->m_cLine.GetStringLength(); // 2002/2/10 aroka CMemory変更
		pCDocLinePrev = pCDocLine->m_pPrev;
		pCDocLineNext = pCDocLine->m_pNext;
		/* 現在行の削除開始位置を調べる */
		if( i == pArg->sDelRange.GetFrom().y ){
			nWorkPos = pArg->sDelRange.GetFrom().x;
		}else{
			nWorkPos = 0;
		}
		/* 現在行の削除データ長を調べる */
		if( i == pArg->sDelRange.GetTo().y ){
			nWorkLen = pArg->sDelRange.GetTo().x - nWorkPos;
		}else{
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory変更
		}

		if( 0 == nWorkLen ){
			/* 前の行へ */
			goto prev_line;
		}
		/* 改行も削除するんかぃのぉ・・・？ */
		if( EOL_NONE != pCDocLine->m_cEol &&
			nWorkPos + nWorkLen > nLineLen - pCDocLine->m_cEol.GetLen() // 2002/2/10 aroka CMemory変更
		){
			/* 削除する長さに改行も含める */
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory変更
		}


		/* 行全体の削除 */
		if( nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory変更
			/* 削除した行の総数 */
			++(pArg->nDeletedLineNum);
			/* 行オブジェクトの削除、リスト変更、行数-- */
			DeleteNode( pCDocLine );
			pCDocLine = NULL;
		}
		/* 次の行と連結するような削除 */
		else if( nWorkPos + nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory変更

			/* 行内データ削除 */
			{// 20020119 aroka ブロック内に pWork を閉じ込めた
				wchar_t* pWork = new wchar_t[nWorkPos + 1];
				auto_memcpy( pWork, pLine, nWorkPos ); // 2002/2/10 aroka 何度も GetPtr を呼ばない
				pCDocLine->m_cLine.SetString( pWork, nWorkPos );
				delete [] pWork;
			}

			/* 次の行がある */
			if( pCDocLineNext ){
				/* 改行コードの情報を更新 (次の行からもらう) */
				pCDocLine->m_cEol = pCDocLineNext->m_cEol;	/* 改行コードの種類 */

				/* 次の行のデータを最後に追加 */
				// 2002/2/10 aroka 直接 CMemory を Append
				//pLine2 = pCDocLineNext->m_cLine.GetPtr();
				//nLineLen2 = pCDocLineNext->m_cLine.GetLength();
				pCDocLine->m_cLine.AppendNativeData( pCDocLineNext->m_cLine );
				/* 次の行 行オブジェクトの削除 */
				DeleteNode( pCDocLineNext );
				pCDocLineNext = NULL;
				/* 削除した行の総数 */
				++(pArg->nDeletedLineNum);
			}else{
				/* 改行コードの情報を更新 */
				pCDocLine->m_cEol.SetType( EOL_NONE );
			}
			pCDocLine->SetModifyFlg(true);	/* 変更フラグ */
		}else{
		/* 行内だけの削除 */
			{// 20020119 aroka ブロック内に pWork を閉じ込めた
				// 2002/2/10 aroka CMemory変更 何度も GetLength,GetPtr をよばない。
				int nLength = pCDocLine->m_cLine.GetStringLength();
				wchar_t* pWork = new wchar_t[nLength - nWorkLen + 1];
				wmemcpy( pWork, pLine, nWorkPos );

				wmemcpy( &pWork[nWorkPos], &pLine[nWorkPos + nWorkLen], nLength - ( nWorkPos + nWorkLen ) );

				pCDocLine->m_cLine.SetString( pWork, nLength - nWorkLen );
				delete [] pWork;
			}
			pCDocLine->SetModifyFlg(true);	/* 変更フラグ */
		}

prev_line:;
		/* 直前の行のオブジェクトのポインタ */
		pCDocLine = pCDocLinePrev;
		/* 最近参照した行番号と行データ */
		--m_nPrevReferLine;
		m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (pArg->sDelRange.GetTo().y - i) && ( 0 == ((pArg->sDelRange.GetTo().y - i) % 32) ) ){
				nProgress = (pArg->sDelRange.GetTo().y - i) * 100 / (pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y) / 2 + 50;
				::SendMessageAny( hwndProgress, PBM_SETPOS, nProgress, 0 );
			}
		}
	}


	/* データ挿入処理 */
	if( 0 == pArg->nInsDataLen ){
		goto end_of_func;
	}
	nAllLinesOld = m_nLines;
	pArg->ptNewPos.y = pArg->sDelRange.GetFrom().y;	/* 挿入された部分の次の位置の行 */
	pArg->ptNewPos.x = 0;	/* 挿入された部分の次の位置のデータ位置 */

	/* 挿入データを行終端で区切った行数カウンタ */
	nCount = 0;
	pArg->nInsLineNum = CLogicInt(0);
	pCDocLine = GetLineInfo( pArg->sDelRange.GetFrom().GetY2() );



	if( NULL == pCDocLine ){
		/* ここでNULLが帰ってくるということは、*/
		/* 全テキストの最後の次の行を追加しようとしていることを示す */
		cmemPrevLine.SetString(L"");
		cmemNextLine.SetString(L"");
		cEOLTypeNext.SetType( EOL_NONE );
		// ::MessageBoxA( NULL, "pDocLine==NULL","Warning",MB_OK);
	}else{
		pCDocLine->SetModifyFlg(true);	/* 変更フラグ */

		// 2002/2/10 aroka 何度も GetPtr を呼ばない
		pLine = pCDocLine->m_cLine.GetStringPtr( &nLineLen );
		cmemPrevLine.SetString( pLine, pArg->sDelRange.GetFrom().x );
		cmemNextLine.SetString( &pLine[pArg->sDelRange.GetFrom().x], nLineLen - pArg->sDelRange.GetFrom().x );

		cEOLTypeNext = pCDocLine->m_cEol;
	}
	nBgn = 0;
	for( nPos = 0; nPos < pArg->nInsDataLen; ){
		if( pArg->pInsData[nPos] == '\n' || pArg->pInsData[nPos] == '\r' ){
			/* 行終端子の種類を調べる */
			cEOLType.GetTypeFromString( &(pArg->pInsData[nPos]), pArg->nInsDataLen - nPos );
			/* 行終端子も含めてテキストをバッファに格納 */
			cmemCurLine.SetString( &(pArg->pInsData[nBgn]), nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + cEOLType.GetLen();
			nPos = nBgn;
			if( NULL == pCDocLine){
				pCDocLineNew = new CDocLine;

				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					if( NULL == m_pDocLineTop ){
						m_pDocLineTop = pCDocLineNew;
					}
					pCDocLineNew->m_pPrev = m_pDocLineBot;
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pCDocLineNew;
					}
					m_pDocLineBot = pCDocLineNew;
					pCDocLineNew->m_pNext = NULL;
					pCDocLineNew->m_cLine.SetNativeData( cmemPrevLine );
					pCDocLineNew->m_cLine += cmemCurLine;

					pCDocLineNew->m_cEol = cEOLType;	/* 改行コードの種類 */
				}else{
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pCDocLineNew;
					}
					pCDocLineNew->m_pPrev = m_pDocLineBot;
					m_pDocLineBot = pCDocLineNew;
					pCDocLineNew->m_pNext = NULL;
					pCDocLineNew->m_cLine.SetNativeData( cmemCurLine );

					pCDocLineNew->m_cEol = cEOLType;	/* 改行コードの種類 */
				}
				pCDocLine = NULL;
				++m_nLines;
			}else{
				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					pCDocLine->m_cLine.SetNativeData( cmemPrevLine );
					pCDocLine->m_cLine += cmemCurLine;

					pCDocLine->m_cEol = cEOLType;	/* 改行コードの種類 */
					pCDocLine = pCDocLine->m_pNext;
				}else{
					pCDocLineNew = new CDocLine;
					pCDocLineNew->m_pPrev = pCDocLine->m_pPrev;
					pCDocLineNew->m_pNext = pCDocLine;
					pCDocLine->m_pPrev->m_pNext = pCDocLineNew;
					pCDocLine->m_pPrev = pCDocLineNew;
					pCDocLineNew->m_cLine.SetNativeData( cmemCurLine );

					pCDocLineNew->m_cEol = cEOLType;	/* 改行コードの種類 */

					++m_nLines;
				}
			}

			/* 挿入データを行終端で区切った行数カウンタ */
			++nCount;
			++(pArg->ptNewPos.y);	/* 挿入された部分の次の位置の行 */
			if( NULL != hwndCancel ){
				if( 0 != pArg->nInsDataLen && ( 0 == (nPos % 1024) ) ){
					nProgress = nPos * 100 / pArg->nInsDataLen;
					::SendMessageAny( hwndProgress, PBM_SETPOS, nProgress, 0 );
				}
			}

		}else{
			++nPos;
		}
	}
	if( 0 < nPos - nBgn || 0 < cmemNextLine.GetStringLength() ){
		cmemCurLine.SetString( &(pArg->pInsData[nBgn]), nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pCDocLine){
			pCDocLineNew = new CDocLine;
			/* 挿入データを行終端で区切った行数カウンタ */
			if( 0 == nCount ){
				if( NULL == m_pDocLineTop ){
					m_pDocLineTop = pCDocLineNew;
				}
				pCDocLineNew->m_pPrev = m_pDocLineBot;
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pCDocLineNew;
				}
				m_pDocLineBot = pCDocLineNew;
				pCDocLineNew->m_pNext = NULL;
				pCDocLineNew->m_cLine.SetNativeData( cmemPrevLine );
				pCDocLineNew->m_cLine += cmemCurLine;

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* 改行コードの種類 */

			}else{
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pCDocLineNew;
				}
				pCDocLineNew->m_pPrev = m_pDocLineBot;
				m_pDocLineBot = pCDocLineNew;
				pCDocLineNew->m_pNext = NULL;
				pCDocLineNew->m_cLine.SetNativeData( cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* 改行コードの種類 */
			}
			pCDocLine = NULL;
			++m_nLines;
			pArg->ptNewPos.x = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
		}else{
			/* 挿入データを行終端で区切った行数カウンタ */
			if( 0 == nCount ){
				pCDocLine->m_cLine.SetNativeData( cmemPrevLine );
				pCDocLine->m_cLine += cmemCurLine;

				pCDocLine->m_cEol = cEOLTypeNext;	/* 改行コードの種類 */

				pCDocLine = pCDocLine->m_pNext;
				pArg->ptNewPos.x = cmemPrevLine.GetStringLength() + nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
			}else{
				pCDocLineNew = new CDocLine;
				pCDocLineNew->m_pPrev = pCDocLine->m_pPrev;
				pCDocLineNew->m_pNext = pCDocLine;
				pCDocLine->m_pPrev->m_pNext = pCDocLineNew;
				pCDocLine->m_pPrev = pCDocLineNew;
				pCDocLineNew->m_cLine.SetNativeData( cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* 改行コードの種類 */

				++m_nLines;
				pArg->ptNewPos.x = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
			}
		}
	}
	pArg->nInsLineNum = m_nLines - nAllLinesOld;
end_of_func:;
	if( NULL != pCDlgCancel ){
		delete pCDlgCancel;
	}
	return;
}


//!ブックマークの全解除
/*
	@date 2001.12.03 hor
*/
void CDocLineMgr::ResetAllBookMark( void )
{
	CDocLine* pDocLine;
	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine ){
		pDocLine->SetBookMark(false);
		pDocLine = pDocLine->m_pNext;
	}
	return;
}


//! ブックマーク検索
/*
	@date 2001.12.03 hor
*/
int CDocLineMgr::SearchBookMark(
	CLogicInt			nLineNum,		/* 検索開始行 */
	ESearchDirection	bPrevOrNext,	/* 0==前方検索 1==後方検索 */
	CLogicInt*			pnLineNum 		/* マッチ行 */
)
{
	CDocLine*	pDocLine;
	CLogicInt	nLinePos=nLineNum;

	//前方検索
	if( bPrevOrNext == SEARCH_BACKWARD ){
		nLinePos--;
		pDocLine = GetLineInfo( nLinePos );
		while( NULL != pDocLine ){
			if(pDocLine->IsBookMarked()){
				*pnLineNum = nLinePos;				/* マッチ行 */
				return TRUE;
			}
			nLinePos--;
			pDocLine = pDocLine->m_pPrev;
		}
	}
	//後方検索
	else{
		nLinePos++;
		pDocLine = GetLineInfo( nLinePos );
		while( NULL != pDocLine ){
			if(pDocLine->IsBookMarked()){
				*pnLineNum = nLinePos;				/* マッチ行 */
				return TRUE;
			}
			nLinePos++;
			pDocLine = pDocLine->m_pNext;
		}
	}
	return FALSE;
}


//! 検索条件に該当する行にブックマークをセットする
/*
	@date 2002.01.16 hor
*/
void CDocLineMgr::MarkSearchWord(
	const wchar_t*	pszPattern,			//!< 検索条件
	const SSearchOption& sSearchOption,	//!< 検索オプション
	/*
	bool			bRegularExp,		// 1==正規表現
	bool			bLoHiCase,			// 1==英大文字小文字の区別
	bool			bWordOnly,			// 1==単語のみ検索
	*/
	CBregexp*		pRegexp				//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
)
{
	CDocLine*	pDocLine;
	const wchar_t*	pLine;
	int			nLineLen;
	const wchar_t*	pszRes;
	int*		pnKey_CharCharsArr;
	//	Jun. 10, 2003 Moca
	//	wcslenを毎回呼ばずにnPatternLenを使うようにする
	const int	nPatternLen = wcslen( pszPattern );

	/* 1==正規表現 */
	if( sSearchOption.bRegularExp ){
		pDocLine = GetLineInfo( CLogicInt(0) );
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked()){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				// 2005.03.19 かろと 前方一致サポートのためのメソッド変更
				if( pRegexp->Match( pLine, nLineLen, 0 ) ){
					pDocLine->SetBookMark(true);
				}
			}
			pDocLine = pDocLine->m_pNext;
		}
	}
	/* 1==単語のみ検索 */
	else if( sSearchOption.bWordOnly ){
		pDocLine = GetLineInfo( CLogicInt(0) );
		CLogicInt nLinePos = CLogicInt(0);
		CLogicInt nNextWordFrom = CLogicInt(0);
		CLogicInt nNextWordFrom2;
		CLogicInt nNextWordTo2;
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked() &&
				WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL )) {
				const wchar_t* pData = pDocLine->m_cLine.GetStringPtr(); // 2002/2/10 aroka CMemory変更
				
				if(( nPatternLen == nNextWordTo2 - nNextWordFrom2 ) &&
				   (( !sSearchOption.bLoHiCase && 0 ==  _memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )) ||
					( sSearchOption.bLoHiCase && 0 == auto_memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )))){
					pDocLine->SetBookMark(true);
				}
				else if( PrevOrNextWord( nLinePos, nNextWordFrom, &nNextWordFrom, FALSE, FALSE) ){
					continue;
				}
			}
			/* 次の行を見に行く */
			nLinePos++;
			pDocLine = pDocLine->m_pNext;
			nNextWordFrom = CLogicInt(0);
		}
	}
	else{
		/* 検索条件の情報 */
		pnKey_CharCharsArr = NULL;
		CDocLineMgr::CreateCharCharsArr(
			pszPattern,
			nPatternLen,
			&pnKey_CharCharsArr
		);
		pDocLine = GetLineInfo( CLogicInt(0) );
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked()){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				pszRes = SearchString(
					pLine,
					nLineLen,
					0,
					pszPattern,
					nPatternLen,
					pnKey_CharCharsArr,
					sSearchOption.bLoHiCase
				);
				if( NULL != pszRes ){
					pDocLine->SetBookMark(true);
				}
			}
			pDocLine = pDocLine->m_pNext;
		}
		if( NULL != pnKey_CharCharsArr ){
			delete [] pnKey_CharCharsArr;
			pnKey_CharCharsArr = NULL;
		}
	}
	return;

}

//! 物理行番号のリストからまとめて行マーク
/*
	@date 2002.01.16 hor
*/
void CDocLineMgr::SetBookMarks( wchar_t* pMarkLines )
{
	CDocLine*	pCDocLine;
	wchar_t *p;
	wchar_t delim[] = L", ";
	p = pMarkLines;
	while(wcstok(p, delim) != NULL) {
		while(wcschr(delim, *p) != NULL)p++;
		pCDocLine=GetLineInfo( CLogicInt(_wtol(p)) );
		if(NULL!=pCDocLine)pCDocLine->SetBookMark(true);
		p += wcslen(p) + 1;
	}
	return;
}


//! 行マークされてる物理行番号のリストを作る
/*
	@date 2002.01.16 hor
*/
wchar_t* CDocLineMgr::GetBookMarks( void )
{
	CDocLine*	pCDocLine;
	static wchar_t szText[MAX_MARKLINES_LEN + 1];	//2002.01.17 // Feb. 17, 2003 genta staticに
	wchar_t szBuff[10];
	CLogicInt	nLinePos=CLogicInt(0);
	pCDocLine = GetLineInfo( nLinePos );
	wcscpy( szText, L"" );
	while( NULL != pCDocLine ){
		if(pCDocLine->IsBookMarked()){
			auto_sprintf( szBuff, L"%d,",nLinePos );
			if(wcslen(szBuff)+wcslen(szText)>MAX_MARKLINES_LEN)break;	//2002.01.17
			wcscat( szText, szBuff);
		}
		nLinePos++;
		pCDocLine = pCDocLine->m_pNext;
	}
	return szText; // Feb. 17, 2003 genta
}

/*!	差分表示の全解除
	@author	MIK
	@date	2002.05.25
*/
void CDocLineMgr::ResetAllDiffMark( void )
{
	CDocLine* pDocLine;

	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine )
	{
		pDocLine->SetDiffMark( 0 );
		pDocLine = pDocLine->m_pNext;
	}

	m_bIsDiffUse = false;

	return;
}

/*! 差分検索
	@author	MIK
	@date	2002.05.25
*/
int CDocLineMgr::SearchDiffMark(
	CLogicInt			nLineNum,		//!< 検索開始行
	ESearchDirection	bPrevOrNext,	//!< 0==前方検索 1==後方検索
	CLogicInt*			pnLineNum 		//!< マッチ行
)
{
	CDocLine*	pDocLine;
	CLogicInt	nLinePos = nLineNum;

	//前方検索
	if( bPrevOrNext == SEARCH_BACKWARD )
	{
		nLinePos--;
		pDocLine = GetLineInfo( nLinePos );
		while( NULL != pDocLine )
		{
			if( pDocLine->IsDiffMarked() )
			{
				*pnLineNum = nLinePos;				/* マッチ行 */
				return TRUE;
			}
			nLinePos--;
			pDocLine = pDocLine->m_pPrev;
		}
	}
	//後方検索
	else
	{
		nLinePos++;
		pDocLine = GetLineInfo( nLinePos );
		while( NULL != pDocLine )
		{
			if( pDocLine->IsDiffMarked() )
			{
				*pnLineNum = nLinePos;				/* マッチ行 */
				return TRUE;
			}
			nLinePos++;
			pDocLine = pDocLine->m_pNext;
		}
	}
	return FALSE;
}

/*!	差分情報を行範囲指定で登録する。
	@author	MIK
	@date	2002/05/25
*/
void CDocLineMgr::SetDiffMarkRange( int nMode, CLogicInt nStartLine, CLogicInt nEndLine )
{
	CDocLine	*pCDocLine;

	m_bIsDiffUse = true;

	if( nStartLine < CLogicInt(0) ) nStartLine = CLogicInt(0);

	//最終行より後に削除行あり
	CLogicInt	nLines = GetLineCount();
	if( nLines <= nEndLine )
	{
		nEndLine = nLines - CLogicInt(1);
		pCDocLine = GetLineInfo( nEndLine );
		if( NULL != pCDocLine ) pCDocLine->SetDiffMark( MARK_DIFF_DEL_EX );
	}

	//行範囲にマークをつける
	for( CLogicInt i = nStartLine; i <= nEndLine; i++ )
	{
		pCDocLine = GetLineInfo( i );
		if( NULL != pCDocLine ) pCDocLine->SetDiffMark( nMode );
	}

	return;
}

/*[EOF]*/
