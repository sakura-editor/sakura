//	$Id$
/************************************************************************
	CDocLineMgr_New.cpp
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1999/12/17  新規作成
************************************************************************/
#include "charcode.h"
#include "CDocLineMgr.h"
#include "debug.h"
#include "charcode.h"
//#include <stdio.h>
#include <io.h>
//#include <string.h>
//#include <memory.h>
#include "CJre.h"
#include <commctrl.h>
#include "global.h"
#include "etc_uty.h"
#include "CRunningTimer.h"
#include "CDlgCancel.h"


/* 指定範囲のデータを置換(削除 & データを挿入)
  Fromを含む位置からToの直前を含むデータを削除する
  Fromの位置へテキストを挿入する
*/
void CDocLineMgr::ReplaceData(
		DocLineReplaceArg* pArg
#if 0
	int			nDelLineFrom,			/* 削除範囲行  From 改行単位の行番号 0開始) */
	int			nDelPosFrom,			/* 削除範囲位置From 改行単位の行頭からのバイト位置　0開始) */
	int			nDelLineTo,			/* 削除範囲行　To   改行単位の行番号 0開始) */
	int			nDelPosTo,				/* 削除範囲位置To   改行単位の行頭からのバイト位置　0開始) */
	CMemory*	pcmemDeleted,		/* 削除されたデータを保存 */
	int*		pnDeletedLineNum,	/* 削除した行の総数 */
	const char*	pInsData,			/* 挿入するデータ */      
	int			nInsDataLen,		/* 挿入するデータの長さ */
	int*		pnInsLineNum,		/* 挿入によって増えた行の数 */
	int*		pnNewLine,			/* 挿入された部分の次の位置の行 */
	int*		pnNewPos			/* 挿入された部分の次の位置のデータ位置 */
#endif
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::ReplaceData()" );
#endif
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
	char* pLine2;
	int nLineLen2;
	int i;
	char* pWork;
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
	// enumEOLType nEOLType;
	// enumEOLType nEOLTypeNext;
	CDlgCancel*	pCDlgCancel = NULL;
	HWND		hwndCancel;
	HWND		hwndProgress;
	

	pArg->nNewLine = pArg->nDelLineFrom;
	pArg->nNewPos =  pArg->nDelPosFrom;

	/* 大量のデータを操作するとき */
	if( 3000 < pArg->nDelLineTo - pArg->nDelLineFrom 
	 || 1024000 < pArg->nInsDataLen
	){
//		/* バッファサイズの調整 */
//		cmemWork.AllocBuffer( 1024000 );
		
		/* 進捗ダイアログの表示 */
		pCDlgCancel = new CDlgCancel;
		if( NULL != ( hwndCancel = pCDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
			::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
 			::SendMessage( hwndProgress, PBM_SETPOS, 0, 0 );

//			hwndStatic = ::GetDlgItem( hwndCancel, IDC_STATIC_KENSUU );
// 			::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)"" );
		}
	}
//	pcmemDeleted->cmemWork( 1024000 );
	

	
	
	
	

	// 削除データの取得のループ
	/* 前から処理していく */
	/* 現在行の情報を得る */
	pCDocLine = GetLineInfo( pArg->nDelLineFrom );
	for( i = pArg->nDelLineFrom; i <= pArg->nDelLineTo && NULL != pCDocLine; i++ ){
		pLine = pCDocLine->m_pLine->m_pData;
		nLineLen = pCDocLine->m_pLine->m_nDataLen;
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
			nWorkLen = pCDocLine->m_pLine->m_nDataLen - nWorkPos;
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
			nWorkPos + nWorkLen > pCDocLine->m_pLine->m_nDataLen - pCDocLine->m_cEol.GetLen() 
		){
			/* 削除する長さに改行も含める */
			nWorkLen = pCDocLine->m_pLine->m_nDataLen - nWorkPos;

		}
		/* 削除されたデータを保存 */
		if( NULL == pArg->pcmemDeleted->Append( &pCDocLine->m_pLine->m_pData[nWorkPos], nWorkLen ) ){
//		if( NULL == pArg->pcmemDeleted->m_pData ){
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

//				wsprintf( szWork, "%d/%d", (i - pArg->nDelLineFrom), (pArg->nDelLineTo - pArg->nDelLineFrom) );
//				::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szWork );
//				/* 処理中のユーザー操作を可能にする */
//				if( !::BlockingHook() ){
//					return;
//				}
//				/* 中断ボタン押下チェック */
//				if( pCDlgCancel->IsCanceled() ){
//					break;
//				}
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
		pLine = pCDocLine->m_pLine->m_pData;
		nLineLen = pCDocLine->m_pLine->m_nDataLen;
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
//			/* 削除開始位置が改行の位置なら */
//			if( EOL_NONE != pCDocLine->m_cEol &&
	//				nWorkPos >= pCDocLine->m_pLine->m_nDataLen - pCDocLine->m_cEol.GetLen()
//			){
//				nWorkLen = pCDocLine->m_pLine->m_nDataLen - nWorkPos;
//			}else{
				nWorkLen = pArg->nDelPosTo - nWorkPos;
//			}
		}else{
			nWorkLen = pCDocLine->m_pLine->m_nDataLen - nWorkPos;
		}

		if( 0 == nWorkLen ){
			/* 前の行へ */
			goto prev_line;
		}
		/* 改行も削除するんかぃのぉ・・・？ */
		if( EOL_NONE != pCDocLine->m_cEol &&
			nWorkPos + nWorkLen > pCDocLine->m_pLine->m_nDataLen - pCDocLine->m_cEol.GetLen() 
		){
			/* 削除する長さに改行も含める */
			nWorkLen = pCDocLine->m_pLine->m_nDataLen - nWorkPos;

		}

		/* 削除されたデータを保存 */
// 1999.12.23 我慢できなかった
//t		/* 遅いけど我慢する */
//t		cmemWork = (*(pArg->pcmemDeleted));
//t		(*(pArg->pcmemDeleted)).SetData( &pCDocLine->m_pLine->m_pData[nWorkPos], nWorkLen );
//t		(*(pArg->pcmemDeleted)) += cmemWork;

//		/* バッファの先頭にデータを挿入する */
//		pArg->pcmemDeleted->InsertTop( &pCDocLine->m_pLine->m_pData[nWorkPos], nWorkLen );


		/* 行全体の削除 */
		if( nWorkLen >= pCDocLine->m_pLine->m_nDataLen ){
			/* 削除した行の総数 */
			++(pArg->nDeletedLineNum);
			/* 行オブジェクトの削除、リスト変更、行数-- */
			DeleteNode( pCDocLine );
			pCDocLine = NULL;
		}else
		/* 次の行と連結するような削除 */
		if( nWorkPos + nWorkLen >= pCDocLine->m_pLine->m_nDataLen ){

			/* 行内データ削除 */
			pWork = new char[nWorkPos + 1];
			memcpy( pWork, pLine, nWorkPos );
			pCDocLine->m_pLine->SetData( pWork, nWorkPos );
			delete [] pWork;
			pWork = NULL;
			
			/* 次の行がある */
			if( NULL != pCDocLineNext ){
				/* 改行コードの情報を更新　(次の行からもらう) */
				pCDocLine->m_cEol = pCDocLineNext->m_cEol;	/* 改行コードの種類 */

				/* 次の行のデータを最後に追加 */
				pLine2 = pCDocLineNext->m_pLine->m_pData;
				nLineLen2 = pCDocLineNext->m_pLine->m_nDataLen;
				pCDocLine->m_pLine->Append( pLine2, nLineLen2 );
				/* 次の行 行オブジェクトの削除 */
				DeleteNode( pCDocLineNext );
				pCDocLineNext = NULL;
				/* 削除した行の総数 */
				++(pArg->nDeletedLineNum);
			}else{
				/* 改行コードの情報を更新 */
				pCDocLine->m_cEol.SetType( EOL_NONE );
			}
			pCDocLine->m_bModify = TRUE;	/* 変更フラグ */
		}else{
		/* 行内だけの削除 */
			pWork = new char[pCDocLine->m_pLine->m_nDataLen - nWorkLen + 1];
			memcpy( pWork, pLine, nWorkPos );
//			pWork[nWorkPos] = '\0';
//			MYTRACE( "pWork=[%s]\n", pWork );

			memcpy( &pWork[nWorkPos], &pLine[nWorkPos + nWorkLen], pCDocLine->m_pLine->m_nDataLen - ( nWorkPos + nWorkLen ) );
//			pWork[pCDocLine->m_pLine->m_nDataLen - nWorkLen] = '\0';
//			MYTRACE( "pWork=[%s]\n", pWork );

			pCDocLine->m_pLine->SetData( pWork, pCDocLine->m_pLine->m_nDataLen - nWorkLen );
			delete [] pWork;
			pWork = NULL;
			pCDocLine->m_bModify = TRUE;	/* 変更フラグ */
		}

prev_line:;		
		/* 直前の行のオブジェクトのポインタ */
		pCDocLine = pCDocLinePrev;
		/* 最近参照した行番号と行データ */
		--m_nPrevReferLine;
		m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (pArg->nDelLineTo - i) && ( 0 == ((pArg->nDelLineTo - i) % 32)) ){
				nProgress = (pArg->nDelLineTo - i) * 100 / (pArg->nDelLineTo - pArg->nDelLineFrom) / 2 + 50;
				::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );

//				wsprintf( szWork, "%d/%d", (pArg->nDelLineTo - i), (pArg->nDelLineTo - pArg->nDelLineFrom) );
 //				::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szWork );
//				/* 処理中のユーザー操作を可能にする */
//				if( !::BlockingHook() ){
//					return;
//				}
//				/* 中断ボタン押下チェック */
//				if( pCDlgCancel->IsCanceled() ){
//					break;
//				}
			}
		}
	}
//	MYTRACE( "\n\n■■■■■■■■■■■■■■\n" );
//	MYTRACE( "(pArg->nDeletedLineNum)=%d\n", (pArg->nDeletedLineNum) );

//	/* 難しいのでやめとく */
//	m_nPrevReferLine = 0;
//	m_pCodePrevRefer = NULL;

	
	
	
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
//		/* Undo操作かどうか */
//		if( bUndo ){
//			pCDocLine->m_nModifyCount--;	/* 変更回数 */
//			if( 0 == pCDocLine->m_nModifyCount ){	/* 変更回数 */
//				pCDocLine->m_bModify = FALSE;	/* 変更フラグ */
//			}
//			if( 0 > pCDocLine->m_nModifyCount ){	/* 変更回数 */
//				::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, "作者に教えて欲しいエラー",
//					"CDocLineMgr::InsertData()で変更カウンタが0以下になりました。バグじゃぁああ"
//				);
//			}
//		}else{
//			++pCDocLine->m_nModifyCount;	/* 変更回数 */
//			pCDocLine->m_bModify = TRUE;	/* 変更フラグ */
//		}
		pCDocLine->m_bModify = TRUE;	/* 変更フラグ */

		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );
		cmemPrevLine.SetData( pLine, pArg->nDelPosFrom );
		cmemNextLine.SetData( &pLine[pArg->nDelPosFrom], nLineLen - pArg->nDelPosFrom );

		cEOLTypeNext = pCDocLine->m_cEol;
	}
	nBgn = 0;
	nPos = 0;
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
					// pCDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLType];		/* 改行コードの長さ */ 		
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
					//pCDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLType];		/* 改行コードの長さ */ 		
					
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
					//pCDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLType];		/* 改行コードの長さ */ 		

					++m_nLines;
				}
			}

			/* 挿入データを行終端で区切った行数カウンタ */
			++nCount;
			++(pArg->nNewLine);	/* 挿入された部分の次の位置の行 */
			if( NULL != hwndCancel){
				if( 0 != pArg->nInsDataLen && ( 0 == (nPos % 1024)) ){
					nProgress = nPos * 100 / pArg->nInsDataLen;
					::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );
				}
			}

		}else{
			++nPos;
		}
		// ::MessageBox( NULL, cEOLType.GetName(),"cEOLTypeNext",MB_OK);
	}
//	nEOLType = EOL_NONE;
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
				// pCDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];		/* 改行コードの長さ */ 		

			}else{
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pCDocLineNew;
				}
				pCDocLineNew->m_pPrev = m_pDocLineBot;
				m_pDocLineBot = pCDocLineNew;
				pCDocLineNew->m_pNext = NULL;
				pCDocLineNew->m_pLine->SetData( &cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* 改行コードの種類 */
				//pCDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];		/* 改行コードの長さ */ 		

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
				//pCDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];		/* 改行コードの長さ */ 		

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
				//pCDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];		/* 改行コードの長さ */ 		


				++m_nLines;
				pArg->nNewPos = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
			}
		}
		// ::MessageBox( NULL, pCDocLine->m_cEol.GetName(),"cEOLTypeNext",MB_OK);
		// ::MessageBox( NULL, pCDocLine->m_pNext->m_cEol.GetName(),"cEOLTypeNext",MB_OK);
		// ::MessageBox( NULL, pCDocLine->m_pPrev->m_cEol.GetName(),"cEOLTypeNext",MB_OK);
	}
	pArg->nInsLineNum = m_nLines - nAllLinesOld;
end_of_func:;
//	pCDlgCancel->CloseDialog( 0 );	 
	if( NULL != pCDlgCancel ){
		delete pCDlgCancel;		
	}
	return;
}



