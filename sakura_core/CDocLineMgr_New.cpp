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
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "charcode.h"
#include "CDocLineMgr.h"
#include "debug.h"
#include "charcode.h"
#include <io.h>
#include <commctrl.h>
#include "global.h"
#include "etc_uty.h"
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
	pArg->nInsLineNum = 0;
	/* 削除した行の総数 */
	pArg->nDeletedLineNum = 0;
	/* 削除されたデータ */
	pArg->pcmemDeleted->SetDataSz( "" );

	CDocLine* pCDocLine;
	CDocLine* pCDocLinePrev;
	CDocLine* pCDocLineNext;
	CDocLine* pCDocLineNew;
	int nWorkPos;
	int nWorkLen;
	char* pLine;
	int nLineLen;
// 2002/2/10 aroka 未使用
//	char* pLine2;
//	int nLineLen2;
	int i;
	int			nBgn;
	int			nPos;
	int			nAllLinesOld;
	int			nCount;
	int			nProgress;
	CMemory		cmemPrevLine;
	CMemory		cmemCurLine;
	CMemory		cmemNextLine;
	//	May 15, 2000
	CEOL cEOLType;
	CEOL cEOLTypeNext;
	CDlgCancel*	pCDlgCancel = NULL;
	HWND		hwndCancel = NULL;	//	初期化
	HWND		hwndProgress = NULL;	//	初期化

	pArg->nNewLine = pArg->nDelLineFrom;
	pArg->nNewPos =  pArg->nDelPosFrom;

	/* 大量のデータを操作するとき */
	if( 3000 < pArg->nDelLineTo - pArg->nDelLineFrom
	 || 1024000 < pArg->nInsDataLen
	){

		/* 進捗ダイアログの表示 */
		pCDlgCancel = new CDlgCancel;
		if( NULL != ( hwndCancel = pCDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
			::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
 			::SendMessage( hwndProgress, PBM_SETPOS, 0, 0 );
		}
	}

	// From Here Feb. 08, 2008 genta 削除バッファをあらかじめ確保する
	pCDocLine = GetLineInfo( pArg->nDelLineFrom );
	nWorkLen = 0;
	for( i = pArg->nDelLineFrom; i <= pArg->nDelLineTo && NULL != pCDocLine; i++ ){
		nWorkLen += pCDocLine->m_pLine->GetLength();
		pCDocLine = pCDocLine->m_pNext;
	}
	pArg->pcmemDeleted->AllocBuffer( nWorkLen );
	// To Here Feb. 08, 2008 genta

	// 削除データの取得のループ
	/* 前から処理していく */
	/* 現在行の情報を得る */
	pCDocLine = GetLineInfo( pArg->nDelLineFrom );
	for( i = pArg->nDelLineFrom; i <= pArg->nDelLineTo && NULL != pCDocLine; i++ ){
		pLine = pCDocLine->m_pLine->GetPtr(); // 2002/2/10 aroka CMemory変更
		nLineLen = pCDocLine->m_pLine->GetLength(); // 2002/2/10 aroka CMemory変更
		pCDocLinePrev = pCDocLine->m_pPrev;
		pCDocLineNext = pCDocLine->m_pNext;
		/* 現在行の削除開始位置を調べる */
		if( i == pArg->nDelLineFrom ){
			nWorkPos = pArg->nDelPosFrom;
		}else{
			nWorkPos = 0;
		}
		/* 現在行の削除データ長を調べる */
		if( i == pArg->nDelLineTo ){
			nWorkLen = pArg->nDelPosTo - nWorkPos;
		}else{
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory変更
		}
		if( 0 == nWorkLen ){
			/* 前の行へ */
			goto next_line;
		}
		if( 0 > nWorkLen ){
			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, "作者に教えて欲しいエラー",
				"CDocLineMgr::ReplaceData()\n\n0 > nWorkLen\nnWorkLen=%d\ni=%d\npArg->nDelLineTo=%d", nWorkLen, i, pArg->nDelLineTo
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
		if( pLine != pCDocLine->m_pLine->GetPtr() ){
			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, "作者に教えて欲しいエラー",
				"CDocLineMgr::ReplaceData()\n\npLine != pCDocLine->m_pLine->GetPtr() =%d\ni=%d\npArg->nDelLineTo=%d", pLine, i, pArg->nDelLineTo
			);
		}
		// 2002/2/10 aroka to here CMemory変更
		if( NULL == pArg->pcmemDeleted->Append( &pLine[nWorkPos], nWorkLen ) ){
			/* メモリ確保に失敗した */
			pArg->pcmemDeleted->SetDataSz( "" );
			break;
		}
next_line:;
		/* 次の行のオブジェクトのポインタ */
		pCDocLine = pCDocLineNext;
		/* 最近参照した行番号と行データ */
		++m_nPrevReferLine;
		m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (i - pArg->nDelLineFrom) && ( 0 == ((i - pArg->nDelLineFrom) % 32)) ){
				nProgress = (i - pArg->nDelLineFrom) * 100 / (pArg->nDelLineTo - pArg->nDelLineFrom) / 2;
				::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );

			}
		}
	} // 削除データの取得のループ


	/* 現在行の情報を得る */
	pCDocLine = GetLineInfo( pArg->nDelLineTo );
	i = pArg->nDelLineTo;
	if( 0 < pArg->nDelLineTo && NULL == pCDocLine ){
		pCDocLine = GetLineInfo( pArg->nDelLineTo - 1 );
		i--;
	}
	/* 後ろから処理していく */
	for( ; i >= pArg->nDelLineFrom && NULL != pCDocLine; i-- ){
		pLine = pCDocLine->m_pLine->GetPtr(); // 2002/2/10 aroka CMemory変更
		nLineLen = pCDocLine->m_pLine->GetLength(); // 2002/2/10 aroka CMemory変更
		pCDocLinePrev = pCDocLine->m_pPrev;
		pCDocLineNext = pCDocLine->m_pNext;
		/* 現在行の削除開始位置を調べる */
		if( i == pArg->nDelLineFrom ){
			nWorkPos = pArg->nDelPosFrom;
		}else{
			nWorkPos = 0;
		}
		/* 現在行の削除データ長を調べる */
		if( i == pArg->nDelLineTo ){
			nWorkLen = pArg->nDelPosTo - nWorkPos;
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
		}else
		/* 次の行と連結するような削除 */
		if( nWorkPos + nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory変更

			/* 行内データ削除 */
			{// 20020119 aroka ブロック内に pWork を閉じ込めた
				char* pWork = new char[nWorkPos + 1];
				memcpy( pWork, pLine, nWorkPos ); // 2002/2/10 aroka 何度も GetPtr を呼ばない
				pCDocLine->m_pLine->SetData( pWork, nWorkPos );
				delete [] pWork;
			}

			/* 次の行がある */
			if( NULL != pCDocLineNext ){
				/* 改行コードの情報を更新 (次の行からもらう) */
				pCDocLine->m_cEol = pCDocLineNext->m_cEol;	/* 改行コードの種類 */

				/* 次の行のデータを最後に追加 */
				// 2002/2/10 aroka 直接 CMemory を Append
				//pLine2 = pCDocLineNext->m_pLine->GetPtr();
				//nLineLen2 = pCDocLineNext->m_pLine->GetLength();
				pCDocLine->m_pLine->Append( pCDocLineNext->m_pLine );
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
				int nLength = pCDocLine->m_pLine->GetLength();
				char* pWork = new char[nLength - nWorkLen + 1];
				memcpy( pWork, pLine, nWorkPos );

				memcpy( &pWork[nWorkPos], &pLine[nWorkPos + nWorkLen], nLength - ( nWorkPos + nWorkLen ) );

				pCDocLine->m_pLine->SetData( pWork, nLength - nWorkLen );
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
			if( 0 != (pArg->nDelLineTo - i) && ( 0 == ((pArg->nDelLineTo - i) % 32) ) ){
				nProgress = (pArg->nDelLineTo - i) * 100 / (pArg->nDelLineTo - pArg->nDelLineFrom) / 2 + 50;
				::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );
			}
		}
	}


	/* データ挿入処理 */
	if( 0 == pArg->nInsDataLen ){
		goto end_of_func;
	}
	nAllLinesOld = m_nLines;
	pArg->nNewLine = pArg->nDelLineFrom;	/* 挿入された部分の次の位置の行 */
	pArg->nNewPos  = 0;	/* 挿入された部分の次の位置のデータ位置 */

	/* 挿入データを行終端で区切った行数カウンタ */
	nCount = 0;
	pArg->nInsLineNum = 0;
	pCDocLine = GetLineInfo( pArg->nDelLineFrom );



	if( NULL == pCDocLine ){
		/* ここでNULLが帰ってくるということは、*/
		/* 全テキストの最後の次の行を追加しようとしていることを示す */
		cmemPrevLine.SetDataSz( "" );
		cmemNextLine.SetDataSz( "" );
		cEOLTypeNext.SetType( EOL_NONE );
		// ::MessageBox( NULL, "pDocLine==NULL","Warning",MB_OK);
	}else{
		pCDocLine->SetModifyFlg(true);	/* 変更フラグ */

		// 2002/2/10 aroka 何度も GetPtr を呼ばない
		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );
		cmemPrevLine.SetData( pLine, pArg->nDelPosFrom );
		cmemNextLine.SetData( &pLine[pArg->nDelPosFrom], nLineLen - pArg->nDelPosFrom );

		cEOLTypeNext = pCDocLine->m_cEol;
	}
	nBgn = 0;
	for( nPos = 0; nPos < pArg->nInsDataLen; ){
		if( pArg->pInsData[nPos] == '\n' || pArg->pInsData[nPos] == '\r' ){
			/* 行終端子の種類を調べる */
			cEOLType.GetTypeFromString( &(pArg->pInsData[nPos]), pArg->nInsDataLen - nPos );
			/* 行終端子も含めてテキストをバッファに格納 */
			cmemCurLine.SetData( &(pArg->pInsData[nBgn]), nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + cEOLType.GetLen();
			nPos = nBgn;
			if( NULL == pCDocLine){
				pCDocLineNew = new CDocLine;

				pCDocLineNew->m_pLine = new CMemory;
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
					pCDocLineNew->m_pLine->SetData( &cmemPrevLine );
					*(pCDocLineNew->m_pLine) += cmemCurLine;

					pCDocLineNew->m_cEol = cEOLType;	/* 改行コードの種類 */
				}else{
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pCDocLineNew;
					}
					pCDocLineNew->m_pPrev = m_pDocLineBot;
					m_pDocLineBot = pCDocLineNew;
					pCDocLineNew->m_pNext = NULL;
					pCDocLineNew->m_pLine->SetData( &cmemCurLine );

					pCDocLineNew->m_cEol = cEOLType;	/* 改行コードの種類 */
				}
				pCDocLine = NULL;
				++m_nLines;
			}else{
				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					pCDocLine->m_pLine->SetData( &cmemPrevLine );
					*(pCDocLine->m_pLine) += cmemCurLine;

					pCDocLine->m_cEol = cEOLType;	/* 改行コードの種類 */
					pCDocLine = pCDocLine->m_pNext;
				}else{
					pCDocLineNew = new CDocLine;
					pCDocLineNew->m_pLine = new CMemory;
					pCDocLineNew->m_pPrev = pCDocLine->m_pPrev;
					pCDocLineNew->m_pNext = pCDocLine;
					pCDocLine->m_pPrev->m_pNext = pCDocLineNew;
					pCDocLine->m_pPrev = pCDocLineNew;
					pCDocLineNew->m_pLine->SetData( &cmemCurLine );

					pCDocLineNew->m_cEol = cEOLType;	/* 改行コードの種類 */

					++m_nLines;
				}
			}

			/* 挿入データを行終端で区切った行数カウンタ */
			++nCount;
			++(pArg->nNewLine);	/* 挿入された部分の次の位置の行 */
			if( NULL != hwndCancel ){
				if( 0 != pArg->nInsDataLen && ( 0 == (nPos % 1024) ) ){
					nProgress = nPos * 100 / pArg->nInsDataLen;
					::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );
				}
			}

		}else{
			++nPos;
		}
	}
	if( 0 < nPos - nBgn || 0 < cmemNextLine.GetLength() ){
		cmemCurLine.SetData( &(pArg->pInsData[nBgn]), nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pCDocLine){
			pCDocLineNew = new CDocLine;
			pCDocLineNew->m_pLine = new CMemory;
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
				pCDocLineNew->m_pLine->SetData( &cmemPrevLine );
				*(pCDocLineNew->m_pLine) += cmemCurLine;

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* 改行コードの種類 */

			}else{
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pCDocLineNew;
				}
				pCDocLineNew->m_pPrev = m_pDocLineBot;
				m_pDocLineBot = pCDocLineNew;
				pCDocLineNew->m_pNext = NULL;
				pCDocLineNew->m_pLine->SetData( &cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* 改行コードの種類 */
			}
			pCDocLine = NULL;
			++m_nLines;
			pArg->nNewPos = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
		}else{
			/* 挿入データを行終端で区切った行数カウンタ */
			if( 0 == nCount ){
				pCDocLine->m_pLine->SetData( &cmemPrevLine );
				*(pCDocLine->m_pLine) += cmemCurLine;

				pCDocLine->m_cEol = cEOLTypeNext;	/* 改行コードの種類 */

				pCDocLine = pCDocLine->m_pNext;
				pArg->nNewPos = cmemPrevLine.GetLength() + nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
			}else{
				pCDocLineNew = new CDocLine;
				pCDocLineNew->m_pLine = new CMemory;
				pCDocLineNew->m_pPrev = pCDocLine->m_pPrev;
				pCDocLineNew->m_pNext = pCDocLine;
				pCDocLine->m_pPrev->m_pNext = pCDocLineNew;
				pCDocLine->m_pPrev = pCDocLineNew;
				pCDocLineNew->m_pLine->SetData( &cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* 改行コードの種類 */

				++m_nLines;
				pArg->nNewPos = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
			}
		}
	}
	pArg->nInsLineNum = m_nLines - nAllLinesOld;
end_of_func:;
	if( NULL != pCDlgCancel ){
		// 進捗ダイアログを表示しない場合と同じ動きになるようにダイアログは遅延破棄する
		// ここで pCDlgCancel を delete すると delete から戻るまでの間に
		// ダイアログ破棄 -> 編集画面へフォーカス移動 -> キャレット位置調整
		// まで一気に動くので無効なレイアウト情報参照で異常終了することがある
		pCDlgCancel->DeleteAsync();	// 自動破棄を遅延実行する	// 2008.05.28 ryoji
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
	int			nLineNum,		/* 検索開始行 */
	int			bPrevOrNext,	/* 0==前方検索 1==後方検索 */
	int*		pnLineNum 		/* マッチ行 */
)
{
	CDocLine*	pDocLine;
	int			nLinePos=nLineNum;

	/* 0==前方検索 1==後方検索 */
	if( 0 == bPrevOrNext ){
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
	}else{
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
	const char*	pszPattern,		/* 検索条件 */
	int			bRegularExp,	/* 1==正規表現 */
	int			bLoHiCase,		/* 1==英大文字小文字の区別 */
	int			bWordOnly,		/* 1==単語のみ検索 */
	CBregexp*	pRegexp			/*!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある */
)
{
	CDocLine*	pDocLine;
	const char*	pLine;
	int			nLineLen;
	char*		pszRes;
	int*		pnKey_CharCharsArr;
	//	Jun. 10, 2003 Moca
	//	lstrlenを毎回呼ばずにnPatternLenを使うようにする
	const int	nPatternLen = lstrlen( pszPattern );

	/* 1==正規表現 */
	if( bRegularExp ){
		pDocLine = GetLineInfo( 0 );
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked()){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				// 2005.03.19 かろと 前方一致サポートのためのメソッド変更
				if( pRegexp->Match( pLine, nLineLen, 0 ) ){
					pDocLine->SetBookMark(true);
				}
			}
			pDocLine = pDocLine->m_pNext;
		}
	}else
	/* 1==単語のみ検索 */
	if( bWordOnly ){
		pDocLine = GetLineInfo( 0 );
		int nLinePos = 0;
		int nNextWordFrom = 0;
		int nNextWordFrom2;
		int nNextWordTo2;
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked() &&
				WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL )) {
				const char* pData = pDocLine->m_pLine->GetPtr(); // 2002/2/10 aroka CMemory変更
				
				if(( nPatternLen == nNextWordTo2 - nNextWordFrom2 ) &&
				   (( FALSE == bLoHiCase && 0 == _memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )) ||
					( TRUE  == bLoHiCase && 0 ==   memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )))){
					pDocLine->SetBookMark(true);
				}else
				if( PrevOrNextWord( nLinePos, nNextWordFrom, &nNextWordFrom, FALSE, FALSE) ){
					continue;
				}
			}
			/* 次の行を見に行く */
			nLinePos++;
			pDocLine = pDocLine->m_pNext;
			nNextWordFrom = 0;
		}
	}else{
		/* 検索条件の情報 */
		pnKey_CharCharsArr = NULL;
		CDocLineMgr::CreateCharCharsArr(
			(const unsigned char *)pszPattern,
			nPatternLen,
			&pnKey_CharCharsArr
		);
		pDocLine = GetLineInfo( 0 );
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked()){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				pszRes = SearchString(
					(const unsigned char *)pLine,
					nLineLen,
					0,
					(const unsigned char *)pszPattern,
					nPatternLen,
					pnKey_CharCharsArr,
					bLoHiCase
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
void CDocLineMgr::SetBookMarks( char* pMarkLines )
{
	CDocLine*	pCDocLine;
	char *p;
	char delim[] = ", ";
	p = pMarkLines;
	while(strtok(p, delim) != NULL) {
		while(strchr(delim, *p) != NULL)p++;
		pCDocLine=GetLineInfo( atol(p) );
		if(NULL!=pCDocLine)pCDocLine->SetBookMark(true);
		p += strlen(p) + 1;
	}
	return;
}


//! 行マークされてる物理行番号のリストを作る
/*
	@date 2002.01.16 hor
*/
char* CDocLineMgr::GetBookMarks( void )
{
	CDocLine*	pCDocLine;
	static char szText[MAX_MARKLINES_LEN + 1];	//2002.01.17 // Feb. 17, 2003 genta staticに
	char szBuff[10];
	int	nLinePos=0;
	pCDocLine = GetLineInfo( nLinePos );
	strcpy( szText, "" );
	while( NULL != pCDocLine ){
		if(pCDocLine->IsBookMarked()){
			wsprintf( szBuff, "%d,",nLinePos );
			if(lstrlen(szBuff)+lstrlen(szText)>MAX_MARKLINES_LEN)break;	//2002.01.17
			strcat( szText, szBuff);
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
	int			nLineNum,		/* 検索開始行 */
	int			bPrevOrNext,	/* 0==前方検索 1==後方検索 */
	int*		pnLineNum 		/* マッチ行 */
)
{
	CDocLine*	pDocLine;
	int			nLinePos = nLineNum;

	/* 0==前方検索 1==後方検索 */
	if( 0 == bPrevOrNext )
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
void CDocLineMgr::SetDiffMarkRange( int nMode, int nStartLine, int nEndLine )
{
	int	i;
	int	nLines;
	CDocLine	*pCDocLine;

	m_bIsDiffUse = true;

	if( nStartLine < 0 ) nStartLine = 0;

	//最終行より後に削除行あり
	nLines = GetLineCount();
	if( nLines <= nEndLine )
	{
		nEndLine = nLines - 1;
		pCDocLine = GetLineInfo( nEndLine );
		if( NULL != pCDocLine ) pCDocLine->SetDiffMark( MARK_DIFF_DEL_EX );
	}

	//行範囲にマークをつける
	for( i = nStartLine; i <= nEndLine; i++ )
	{
		pCDocLine = GetLineInfo( i );
		if( NULL != pCDocLine ) pCDocLine->SetDiffMark( nMode );
	}

	return;
}

/*[EOF]*/
