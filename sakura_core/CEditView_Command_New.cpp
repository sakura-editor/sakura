/*!	@file
	@brief CEditViewクラスのコマンド処理系関数群

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, asa-o, hor
	Copyright (C) 2002, YAZAKI, hor, genta. aroka, MIK, minfu, KK, かろと
	Copyright (C) 2003, MIK, Moca
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, ryoji, genta, D.S.Koba
	Copyright (C) 2006, genta, Moca, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CEditView.h"
#include "CWaitCursor.h"
#include "charcode.h"
#include "CRunningTimer.h"
#include <algorithm>		// 2001.12.11 hor    for VC++
#include "COpe.h" ///	2002/2/3 aroka from here
#include "COpeBlk.h" ///
#include "CLayout.h"///
#include "CDocLine.h"///
#include "mymessage.h"///
#include "debug.h"///
#include "etc_uty.h"///
#include <string>///
#include <vector> /// 2002/2/3 aroka to here
#include "COsVersionInfo.h"   // 2002.04.09 minfu 
#include "CEditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理
#include "CEditWnd.h"
#include "CDlgCtrlCode.h"	//コントロールコードの入力(ダイアログ)
#include "CDlgFavorite.h"	//お気に入り	//@@@ 2003.04.08 MIK

using namespace std; // 2002/2/3 aroka to here

#ifndef FID_RECONVERT_VERSION  // 2002.04.10 minfu 
#define FID_RECONVERT_VERSION 0x10000000
#endif
/*!	現在位置にデータを挿入 Ver0

	@date 2002/03/24 YAZAKI bUndo削除
*/
void CEditView::InsertData_CEditView(
	int			nX,
	int			nY,
	const char*	pData,
	int			nDataLen,
	int*		pnNewLine,			/* 挿入された部分の次の位置の行 */
	int*		pnNewPos,			/* 挿入された部分の次の位置のデータ位置 */
	COpe*		pcOpe,				/* 編集操作要素 COpe */
	BOOL		bRedraw
//	BOOL		bUndo			/* Undo操作かどうか */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::InsertData_CEditView" );
#endif
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			nIdxFrom;
	int			nModifyLayoutLinesOld;
	int			nInsLineNum;		/* 挿入によって増えたレイアウト行の数 */
	PAINTSTRUCT ps;
	HDC			hdc;
	int			nLineAllColLen;
	CMemory		cMem;
	int			i;
	const CLayout*	pcLayout;

	*pnNewLine = 0;			/* 挿入された部分の次の位置の行 */
	*pnNewPos = 0;			/* 挿入された部分の次の位置のデータ位置 */

	/* テキストが選択されているか */
	if( IsTextSelected() ){
		DeleteData( bRedraw );
		nX = m_nCaretPosX;
		nY = m_nCaretPosY;
	}

	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nY, &nLineLen, &pcLayout );

	nIdxFrom = 0;
//	cMem.SetData( "", lstrlen( "" ) );
	cMem.SetDataSz( "" );
	if( NULL != pLine ){
		/* 指定された桁に対応する行のデータ内の位置を調べる */
		nIdxFrom = LineColmnToIndex2( pcLayout, nX, nLineAllColLen );
		/* 行終端より右に挿入しようとした */
		if( nLineAllColLen > 0 ){
			/* 終端直前から挿入位置まで空白を埋める為の処理 */
			/* 行終端が何らかの改行コードか? */
			if( EOL_NONE != pcLayout->m_cEol ){
				nIdxFrom = nLineLen - 1;
				for( i = 0; i < nX - nLineAllColLen + 1; ++i ){
					cMem += ' ';
				}
				cMem.Append( pData, nDataLen );
			}else{
				nIdxFrom = nLineLen;
				for( i = 0; i < nX - nLineAllColLen; ++i ){
					cMem += ' ';
				}
				cMem.Append( pData, nDataLen );
			}
		}else{
			cMem.Append( pData, nDataLen );
		}
	}else{
		nLineAllColLen = nX;
		for( i = 0; i < nX - nIdxFrom; ++i ){
			cMem += ' ';
		}
		cMem.Append( pData, nDataLen );
	}
//	MYTRACE( "nY=%d nIdxFrom=%d nLineAllColLen=%d \n", nY, nIdxFrom, nLineAllColLen );


	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* アンドゥ・リドゥの実行中か */
		if( NULL != pLine ){
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
				LineIndexToColmn( pcLayout, nIdxFrom ),
				nY,
				&pcOpe->m_nCaretPosX_PHY_Before,
				&pcOpe->m_nCaretPosY_PHY_Before
			);
		}else{
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
				0,
				nY,
				&pcOpe->m_nCaretPosX_PHY_Before,
				&pcOpe->m_nCaretPosY_PHY_Before
			);
		}
	}


	/* 文字列挿入 */
	m_pcEditDoc->m_cLayoutMgr.InsertData_CLayoutMgr(
		nY,
		nIdxFrom,
		cMem.GetPtr(),
		cMem.GetLength(),
		&nModifyLayoutLinesOld,
		&nInsLineNum,
		pnNewLine,			/* 挿入された部分の次の位置の行 */
		pnNewPos			/* 挿入された部分の次の位置のデータ位置 */
	);


	/* メモリが再確保されてアドレスが無効になるので、再度、行データを求める */
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nY, &nLineLen );

	/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
	pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( *pnNewLine, &nLineLen2, &pcLayout );
	if( pLine2 != NULL ){
		*pnNewPos = LineIndexToColmn( pcLayout, *pnNewPos );
	}
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if( *pnNewPos >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() ){
		if( m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet
		 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.16 MIK
		{
			if( m_pcEditDoc->m_cLayoutMgr.IsEndOfLine( *pnNewLine, *pnNewPos ) )	//@@@ 2002.04.18
			{
				*pnNewPos = 0;
				(*pnNewLine)++;
			}
		}
		else
		{
			// Oct. 7, 2002 YAZAKI
			*pnNewPos = pcLayout->m_pNext ? pcLayout->m_pNext->GetIndent() : 0;
			(*pnNewLine)++;
		}
	}

//	MYTRACE( "nModifyLayoutLinesOld=%d nInsLineNum=%d *pnNewLine=%d *pnNewPos=%d\n", nModifyLayoutLinesOld, nInsLineNum, *pnNewLine, *pnNewPos );


	/* 状態遷移 */
	if( FALSE == m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	/* 再描画 */
	/* 行番号表示に必要な幅を設定 */
	if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
		/* キャレットの表示・更新 */
		ShowEditCaret();
	}else{

		if( bRedraw ){
			if( 0 < nInsLineNum ){
				/* スクロールバーの状態を更新する */
				AdjustScrollBars();
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nY - m_nViewTopLine);
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			}else{
				if( nModifyLayoutLinesOld < 1 ){
					nModifyLayoutLinesOld = 1;
				}
	//			ps.rcPaint.left = m_nViewAlignLeft;
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;

				// 2002.02.25 Mod By KK 次行 (nY - m_nViewTopLine - 1); => (nY - m_nViewTopLine);
				//ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nY - m_nViewTopLine - 1);
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nY - m_nViewTopLine);

				//禁則がある場合は1行前から再描画を行う	@@@ 2002.04.19 MIK
				if( m_pcEditDoc->GetDocumentAttribute().m_bWordWrap
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuHead	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuTail	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.19 MIK
				{
					ps.rcPaint.top -= (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				}
				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}

				ps.rcPaint.bottom = ps.rcPaint.top + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * ( nModifyLayoutLinesOld + 1);
				if( m_nViewAlignTop + m_nViewCy < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				}
			}
			hdc = ::GetDC( m_hWnd );
//			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
//			OnSetFocus();
			::ReleaseDC( m_hWnd, hdc );
		}
	}

	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* アンドゥ・リドゥの実行中か */
		pcOpe->m_nOpe = OPE_INSERT;				/* 操作種別 */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			*pnNewPos,
			*pnNewLine,
			&pcOpe->m_nCaretPosX_PHY_To,
			&pcOpe->m_nCaretPosY_PHY_To
		);

		pcOpe->m_nDataLen = cMem.GetLength();	/* 操作に関連するデータのサイズ */
		pcOpe->m_pcmemData = NULL;				/* 操作に関連するデータ */
	}
#ifdef _DEBUG
	gm_ProfileOutput = 0;
#endif
	return;
}


/*!	指定位置の指定長データ削除

	@date 2002/03/24 YAZAKI bUndo削除
	@date 2002/05/12 YAZAKI bRedraw, bRedraw2削除（常にFALSEだから）
*/
void CEditView::DeleteData2(
	int			nCaretX,
	int			nCaretY,
	int			nDelLen,
	CMemory*	pcMem,
	COpe*		pcOpe		/* 編集操作要素 COpe */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DeleteData(1)" );
#endif
	const char*	pLine;
	int			nLineLen;
	int			nIdxFrom;
	int			nModifyLayoutLinesOld;
	int			nModifyLayoutLinesNew;
	int			nDeleteLayoutLines;
	int			bLastLine;

	/* 最後の行にカーソルがあるかどうか */
	if( nCaretY == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
		bLastLine = 1;
	}else{
		bLastLine = 0;
	}

	const CLayout* pcLayout;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCaretY, &nLineLen, &pcLayout );
	if( NULL == pLine ){
		goto end_of_func;
	}
	nIdxFrom = LineColmnToIndex( pcLayout, nCaretX );
	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* アンドゥ・リドゥの実行中か */
		pcOpe->m_nOpe = OPE_DELETE;				/* 操作種別 */
//		pcOpe->m_nCaretPosX_To = LineIndexToColmn( pLine, nLineLen, nIdxFrom + nDelLen );/* 操作前のキャレット位置Ｘ */
//		pcOpe->m_nCaretPosY_To = nCaretY;	/* 操作前のキャレット位置Ｙ */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_To,
//			pcOpe->m_nCaretPosY_To,
//			&pcOpe->m_nCaretPosX_PHY_To,
//			&pcOpe->m_nCaretPosY_PHY_To
//		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			LineIndexToColmn( pcLayout, nIdxFrom + nDelLen ),
			nCaretY,
			&pcOpe->m_nCaretPosX_PHY_To,
			&pcOpe->m_nCaretPosY_PHY_To
		);
	}


	/* データ削除 */
	m_pcEditDoc->m_cLayoutMgr.DeleteData_CLayoutMgr(
		nCaretY, nIdxFrom, nDelLen,
		&nModifyLayoutLinesOld,
		&nModifyLayoutLinesNew,
		&nDeleteLayoutLines,
		*pcMem
	);

	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* アンドゥ・リドゥの実行中か */
		pcOpe->m_nDataLen = pcMem->GetLength();	/* 操作に関連するデータのサイズ */
		pcOpe->m_pcmemData = pcMem;				/* 操作に関連するデータ */
	}

	/* 選択エリアの先頭へカーソルを移動 */
	MoveCursor( nCaretX, nCaretY, FALSE );
	m_nCaretPosX_Prev = m_nCaretPosX;


end_of_func:;
#ifdef _DEBUG
	gm_ProfileOutput = 0;
#endif
	return;

}





/*!	カーソル位置または選択エリアを削除

	@date 2002/03/24 YAZAKI bUndo削除
*/
void CEditView::DeleteData(
	BOOL	bRedraw
//	BOOL	bUndo	/* Undo操作かどうか */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DeleteData(2)" );
#endif
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			nLineNum;
	int			nCurIdx;
	int			nNxtIdx;
	int			nNxtPos;
	PAINTSTRUCT ps;
	HDC			hdc;
	int			nIdxFrom;
	int			nIdxTo;
	int			nDelPos;
	int			nDelLen;
	int			nDelPosNext;
	int			nDelLenNext;
//	CMemory		cmemBuf;
	RECT		rcSel;
	int			bLastLine;
	CMemory*	pcMemDeleted;
	COpe*		pcOpe = NULL;
	int			nCaretPosXOld;
	int			nCaretPosYOld;
	int			i;
	const CLayout*	pcLayout;
	int			nSelectColmFrom_Old;
	int			nSelectLineFrom_Old;

// hor IsTextSelected内に移動
//	CWaitCursor cWaitCursor( m_hWnd );	// 2002.01.25 hor

	nCaretPosXOld = m_nCaretPosX;
	nCaretPosYOld = m_nCaretPosY;

	/* テキストが選択されているか */
	if( IsTextSelected() ){
		CWaitCursor cWaitCursor( m_hWnd );  // 2002.02.05 hor
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* 操作前のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* 操作前のキャレット位置Ｙ */

			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}

		/* 矩形範囲選択中か */
		if( m_bBeginBoxSelect ){
			m_pcEditDoc->SetModified(true,bRedraw);	//	2002/06/04 YAZAKI 矩形選択を削除したときに変更マークがつかない。

			m_bDrawSWITCH=FALSE;	// 2002.01.25 hor
//			bBoxSelected = TRUE;	// 2002/2/3 aroka
			nSelectColmFrom_Old = m_nSelectColmFrom;
			nSelectLineFrom_Old = m_nSelectLineFrom;

			/* 選択範囲のデータを取得 */
			/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
			/* ２点を対角とする矩形を求める */
			TwoPointToRect(
				&rcSel,
				m_nSelectLineFrom,		/* 範囲選択開始行 */
				m_nSelectColmFrom,		/* 範囲選択開始桁 */
				m_nSelectLineTo,		/* 範囲選択終了行 */
				m_nSelectColmTo			/* 範囲選択終了桁 */
			);
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( bRedraw );

			nIdxFrom = 0;
			nIdxTo = 0;
			for( nLineNum = rcSel.bottom; nLineNum >= rcSel.top - 1; nLineNum-- ){
				nDelPosNext = nIdxFrom;
				nDelLenNext	= nIdxTo - nIdxFrom;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					/* 指定された桁に対応する行のデータ内の位置を調べる */
					nIdxFrom = LineColmnToIndex( pcLayout, rcSel.left  );
					nIdxTo	 = LineColmnToIndex( pcLayout, rcSel.right );

					for( i = nIdxFrom; i <= nIdxTo; ++i ){
						if( pLine[i] == CR || pLine[i] == LF ){
							nIdxTo = i;
							break;
						}
					}
				}else{
					nIdxFrom = 0;
					nIdxTo	 = 0;
				}
				nDelPos = nDelPosNext;
				nDelLen	= nDelLenNext;
				if( nLineNum < rcSel.bottom && 0 < nDelLen ){
					if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
						pcOpe = new COpe;
						pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + 1, &nLineLen2, &pcLayout );
						m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
							LineIndexToColmn( pcLayout, nDelPos ),
							nLineNum + 1,
							&pcOpe->m_nCaretPosX_PHY_Before,
							&pcOpe->m_nCaretPosY_PHY_Before
						);

					}else{
						pcOpe = NULL;
					}
					pcMemDeleted = new CMemory;
					/* 指定位置の指定長データ削除 */
					DeleteData2(
						rcSel.left,
						nLineNum + 1,
						nDelLen,
						pcMemDeleted,
						pcOpe				/* 編集操作要素 COpe */
//						FALSE/*bRedraw	2002.01.25 hor*/,
//						FALSE/*bRedraw*	2002.01.25 hor*/
					);

					if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
						m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
							rcSel.left,
							nLineNum + 1,
							&pcOpe->m_nCaretPosX_PHY_After,
							&pcOpe->m_nCaretPosY_PHY_After
						);
						/* 操作の追加 */
						m_pcOpeBlk->AppendOpe( pcOpe );
					}else{
						delete pcMemDeleted;
						pcMemDeleted = NULL;
					}
				}
			}
			m_bDrawSWITCH=TRUE;	// 2002.01.25 hor

			/* 行番号表示に必要な幅を設定 */
			if ( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
				/* キャレットの表示・更新 */
				ShowEditCaret();
			}
			if( bRedraw ){
				/* スクロールバーの状態を更新する */
				AdjustScrollBars();

				/* 再描画 */
				hdc = ::GetDC( m_hWnd );
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
	//			ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (m_nCaretPosY - m_nViewTopLine);
				ps.rcPaint.top = m_nViewAlignTop;
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//				OnKillFocus();
				OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
//				OnSetFocus();
				::ReleaseDC( m_hWnd, hdc );
			}
			/* 選択エリアの先頭へカーソルを移動 */
			::UpdateWindow( m_hWnd );
			MoveCursor( nSelectColmFrom_Old, nSelectLineFrom_Old, bRedraw );
			m_nCaretPosX_Prev = m_nCaretPosX;
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe = new COpe;
				pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
					nCaretPosXOld,
					nCaretPosYOld,
					&pcOpe->m_nCaretPosX_PHY_Before,
					&pcOpe->m_nCaretPosY_PHY_Before
				);

				pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* 操作後のキャレット位置Ｘ */
				pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* 操作後のキャレット位置Ｙ */
				/* 操作の追加 */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}
		}else{
			/* データ置換 削除&挿入にも使える */
			ReplaceData_CEditView(
				m_nSelectLineFrom,		/* 範囲選択開始行 */
				m_nSelectColmFrom,		/* 範囲選択開始桁 */
				m_nSelectLineTo,		/* 範囲選択終了行 */
				m_nSelectColmTo,		/* 範囲選択終了桁 */
				NULL,					/* 削除されたデータのコピー(NULL可能) */
				"",						/* 挿入するデータ */
				0,						/* 挿入するデータの長さ */
				bRedraw
			);
		}
	}else{
		/* 現在行のデータを取得 */
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL == pLine ){
			goto end_of_func;
//			return;
		}
		/* 最後の行にカーソルがあるかどうか */
		if( m_nCaretPosY == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
			bLastLine = 1;
		}else{
			bLastLine = 0;
		}

		/* 指定された桁に対応する行のデータ内の位置を調べる */
		nCurIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );
//		MYTRACE("nLineLen=%d nCurIdx=%d \n", nLineLen, nCurIdx);
		if( nCurIdx == nLineLen && bLastLine ){	/* 全テキストの最後 */
			goto end_of_func;
//			return;
		}
		/* 指定された桁の文字のバイト数を調べる */
		if( pLine[nCurIdx] == '\r' || pLine[nCurIdx] == '\n' ){
			/* 改行 */
			nNxtIdx = nCurIdx + pcLayout->m_cEol.GetLen();
			nNxtPos = m_nCaretPosX + pcLayout->m_cEol.GetLen();
		}else{
			nNxtIdx = CMemory::MemCharNext( pLine, nLineLen, &pLine[nCurIdx] ) - pLine;
			/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
			nNxtPos = LineIndexToColmn( pcLayout, nNxtIdx );
		}


		/* データ置換 削除&挿入にも使える */
		ReplaceData_CEditView(
			m_nCaretPosY,		/* 範囲選択開始行 */
			m_nCaretPosX,		/* 範囲選択開始桁 */
			m_nCaretPosY,		/* 範囲選択終了行 */
			nNxtPos,			/* 範囲選択終了桁 */
			NULL,				/* 削除されたデータのコピー(NULL可能) */
			"",					/* 挿入するデータ */
			0,					/* 挿入するデータの長さ */
			bRedraw
		);
	}

	m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() > 0 ){
		if( m_nCaretPosY > m_pcEditDoc->m_cLayoutMgr.GetLineCount()	- 1	){
			/* 現在行のデータを取得 */
			const CLayout*	pcLayout;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				goto end_of_func;
			}
			/* 改行で終わっているか */
			if( ( EOL_NONE != pcLayout->m_cEol ) ){
				goto end_of_func;
			}
			/*ファイルの最後に移動 */
			Command_GOFILEEND( FALSE );
		}
	}
end_of_func:;

	return;
}











/* Undo 元に戻す */
void CEditView::Command_UNDO( void )
{
	if( m_bBeginSelect ){	/* マウスによる範囲選択中 */
		::MessageBeep( MB_ICONHAND );
		return;
	}

	if( !m_pcEditDoc->IsEnableUndo() ){	/* Undo(元に戻す)可能な状態か？ */
		return;
	}

	MY_RUNNINGTIMER( cRunningTimer, "CEditView::Command_UNDO()" );

	COpe*		pcOpe = NULL;
	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
	CMemory*	pcMem;
	int			bIsModified;
//	int			nNewLine;	/* 挿入された部分の次の位置の行 */
//	int			nNewPos;	/* 挿入された部分の次の位置のデータ位置 */
	HDC			hdc;
	PAINTSTRUCT	ps;
	CWaitCursor cWaitCursor( m_hWnd );
//@@@ 2002.01.03 YAZAKI 不使用のため
//	BOOL		bUndo;	/* Undo操作かどうか */
//	bUndo = TRUE;	/* Undo操作かどうか */

	int			nCaretPosX_Before;
	int			nCaretPosY_Before;
//	int			nCaretPosX_To;
//	int			nCaretPosY_To;
	int			nCaretPosX_After;
	int			nCaretPosY_After;

	/* 各種モードの取り消し */
	Command_CANCEL_MODE();

	m_bDoing_UndoRedo = TRUE;	/* アンドゥ・リドゥの実行中か */

	/* 現在のUndo対象の操作ブロックを返す */
	if( NULL != ( pcOpeBlk = m_pcEditDoc->m_cOpeBuf.DoUndo( &bIsModified ) ) ){
		m_bDrawSWITCH = FALSE;	//	hor
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = nOpeBlkNum - 1; i >= 0; i-- ){
			pcOpe = pcOpeBlk->GetOpe( i );
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				pcOpe->m_nCaretPosX_PHY_After,
				pcOpe->m_nCaretPosY_PHY_After,
				&nCaretPosX_After,
				&nCaretPosY_After
			);
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				pcOpe->m_nCaretPosX_PHY_Before,
				pcOpe->m_nCaretPosY_PHY_Before,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);


		// 2002.02.16 hor
		//	if( i == nOpeBlkNum - 1	){
		//		/* カーソルを移動 */
		//		MoveCursor( nCaretPosX_After, nCaretPosY_After, TRUE );
		//	}else{
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, FALSE );
		//	}
			switch( pcOpe->m_nOpe ){
			case OPE_INSERT:
				pcMem = new CMemory;

				/* 選択範囲の変更 */
				m_nSelectLineBgnFrom = nCaretPosY_Before;	/* 範囲選択開始行(原点) */
				m_nSelectColmBgnFrom = nCaretPosX_Before;	/* 範囲選択開始桁(原点) */
				m_nSelectLineBgnTo = m_nSelectLineBgnFrom;		/* 範囲選択開始行(原点) */
				m_nSelectColmBgnTo = m_nSelectColmBgnFrom;		/* 範囲選択開始桁(原点) */
				m_nSelectLineFrom = nCaretPosY_Before;
				m_nSelectColmFrom = nCaretPosX_Before;
				m_nSelectLineTo = nCaretPosY_After;
				m_nSelectColmTo = nCaretPosX_After;

//				GetSelectedData( *pcMem, FALSE, NULL, FALSE );
//				DeleteData(
//					FALSE,
//					bUndo	/* Undo操作かどうか */
//				);


				/* データ置換 削除&挿入にも使える */
				ReplaceData_CEditView(
					m_nSelectLineFrom,		/* 範囲選択開始行 */
					m_nSelectColmFrom,		/* 範囲選択開始桁 */
					m_nSelectLineTo,		/* 範囲選択終了行 */
					m_nSelectColmTo,		/* 範囲選択終了桁 */
					pcMem,					/* 削除されたデータのコピー(NULL可能) */
					"",						/* 挿入するデータ */
					0,						/* 挿入するデータの長さ */
					FALSE					/*再描画するか否か*/
				);




				/* 選択範囲の変更 */
				m_nSelectLineBgnFrom = -1;	/* 範囲選択開始行(原点) */
				m_nSelectColmBgnFrom = -1;	/* 範囲選択開始桁(原点) */
				m_nSelectLineBgnTo = -1;	/* 範囲選択開始行(原点) */
				m_nSelectColmBgnTo = -1;	/* 範囲選択開始桁(原点) */
				m_nSelectLineFrom = -1;
				m_nSelectColmFrom = -1;
				m_nSelectLineTo = -1;
				m_nSelectColmTo = -1;

				pcOpe->m_pcmemData = pcMem;
//				if( 0 == pcMem->GetLength() ){
//					MYTRACE( "?? ERROR\n" );
//				}

				break;
			case OPE_DELETE:
				pcMem = new CMemory;
				if( 0 < pcOpe->m_pcmemData->GetLength() ){
					/* データ置換 削除&挿入にも使える */
					ReplaceData_CEditView(
						nCaretPosY_Before,					/* 範囲選択開始行 */
						nCaretPosX_Before,					/* 範囲選択開始桁 */
						nCaretPosY_Before,					/* 範囲選択終了行 */
						nCaretPosX_Before,					/* 範囲選択終了桁 */
						pcMem,								/* 削除されたデータのコピー(NULL可能) */
						pcOpe->m_pcmemData->GetPtr(),	/* 挿入するデータ */
						pcOpe->m_nDataLen,					/* 挿入するデータの長さ */
						FALSE								/*再描画するか否か*/
					);

//					InsertData_CEditView(
//						nCaretPosX_Before,
//						nCaretPosY_Before,
//						pcOpe->m_pcmemData->GetPtr(),
//						pcOpe->m_nDataLen,
//						&nNewLine,
//						&nNewPos,
//						NULL,
//						FALSE,
//						bUndo	/* Undo操作かどうか */
//					);
				}
				delete pcOpe->m_pcmemData;
				pcOpe->m_pcmemData = NULL;
				break;
			case OPE_MOVECARET:
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, FALSE/*TRUE 2002.02.16 hor */ );
				break;
			}

			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				pcOpe->m_nCaretPosX_PHY_Before,
				pcOpe->m_nCaretPosY_PHY_Before,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);
			if( i == 0 ){
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, TRUE );
			}else{
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, FALSE );
			}
		}
		m_bDrawSWITCH = TRUE;	//	hor

		/* Undo後の変更フラグ */
		m_pcEditDoc->SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */

		SetBracketPairPos( true );	// 03/03/07 ai

		/* 再描画 */
		hdc = ::GetDC( m_hWnd );
		ps.rcPaint.left = 0;
		ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
		ps.rcPaint.top = m_nViewAlignTop;
		ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//		OnKillFocus();
		OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
//		OnSetFocus();
		DispRuler( hdc );
		::ReleaseDC( m_hWnd, hdc );
		/* 行番号表示に必要な幅を設定 */
		if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
			/* キャレットの表示・更新 */
			ShowEditCaret();
		}

		m_pcEditDoc->RedrawInactivePane();/* 他のペインの表示状態を更新 */
#if 0
	//	2001/06/21 Start by asa-o: 他のペインの表示状態を更新
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^1].Redraw();
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^2].Redraw();
		m_pcEditDoc->m_cEditViewArr[(m_nMyIndex^1)^2].Redraw();
	//	2001/06/21 End
#endif

	}
	m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */
	return;
}





/* Redo やり直し */
void CEditView::Command_REDO( void )
{
	if( m_bBeginSelect ){	/* マウスによる範囲選択中 */
		::MessageBeep( MB_ICONHAND );
		return;
	}


	if( !m_pcEditDoc->IsEnableRedo() ){	/* Redo(やり直し)可能な状態か？ */
		return;
	}
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::Command_REDO()" );

	COpe*		pcOpe = NULL;
	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
	CMemory*	pcMem;
//	int			nNewLine;	/* 挿入された部分の次の位置の行 */
//	int			nNewPos;	/* 挿入された部分の次の位置のデータ位置 */
	int			bIsModified;
	HDC			hdc;
	PAINTSTRUCT	ps;
	CWaitCursor cWaitCursor( m_hWnd );

	int			nCaretPosX_Before;
	int			nCaretPosY_Before;
	int			nCaretPosX_To;
	int			nCaretPosY_To;
	int			nCaretPosX_After;
	int			nCaretPosY_After;


	/* 各種モードの取り消し */
	Command_CANCEL_MODE();

	m_bDoing_UndoRedo = TRUE;	/* アンドゥ・リドゥの実行中か */

	/* 現在のRedo対象の操作ブロックを返す */
	if( NULL != ( pcOpeBlk = m_pcEditDoc->m_cOpeBuf.DoRedo( &bIsModified ) ) ){
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = 0; i < nOpeBlkNum; ++i ){
			pcOpe = pcOpeBlk->GetOpe( i );
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
//			int		nPosX2;
//			int		nPosY2;
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				pcOpe->m_nCaretPosX_PHY_Before,
				pcOpe->m_nCaretPosY_PHY_Before,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);

			if( i == 0 ){
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, TRUE );
			}else{
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, FALSE );
			}
			switch( pcOpe->m_nOpe ){
			case OPE_INSERT:
				pcMem = new CMemory;
				if( 0 < pcOpe->m_pcmemData->GetLength() ){
					/* データ置換 削除&挿入にも使える */
					ReplaceData_CEditView(
						nCaretPosY_Before,					/* 範囲選択開始行 */
						nCaretPosX_Before,					/* 範囲選択開始桁 */
						nCaretPosY_Before,					/* 範囲選択終了行 */
						nCaretPosX_Before,					/* 範囲選択終了桁 */
						NULL,								/* 削除されたデータのコピー(NULL可能) */
						pcOpe->m_pcmemData->GetPtr(),	/* 挿入するデータ */
						pcOpe->m_pcmemData->GetLength(),	/* 挿入するデータの長さ */
						FALSE								/*再描画するか否か*/
					);

//					InsertData_CEditView(
//						nCaretPosX_Before,
//						nCaretPosY_Before,
//						pcOpe->m_pcmemData->GetPtr(),
//						pcOpe->m_pcmemData->GetLength(),//*pcOpe->m_nDataLen,
//						&nNewLine,
//						&nNewPos,
//						NULL,
//						FALSE
//					);
				}
				delete pcOpe->m_pcmemData;
				pcOpe->m_pcmemData = NULL;
				break;
			case OPE_DELETE:
				pcMem = new CMemory;

				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
					pcOpe->m_nCaretPosX_PHY_To,
					pcOpe->m_nCaretPosY_PHY_To,
					&nCaretPosX_To,
					&nCaretPosY_To
				);

				/* データ置換 削除&挿入にも使える */
				ReplaceData_CEditView(
					nCaretPosY_Before,	/* 範囲選択開始行 */
					nCaretPosX_Before,	/* 範囲選択開始桁 */
					nCaretPosY_To,		/* 範囲選択終了行 */
					nCaretPosX_To,		/* 範囲選択終了桁 */
					pcMem,				/* 削除されたデータのコピー(NULL可能) */
					"",					/* 挿入するデータ */
					0,					/* 挿入するデータの長さ */
					FALSE
				);

// 199.12.20
//				DeleteData2(
//					pcOpe->m_nCaretPosX_Before,
//					pcOpe->m_nCaretPosY_Before,
//					pcOpe->m_nDataLen,
//					pcMem,
//					NULL,
//					FALSE,
//					FALSE
//				);
				pcOpe->m_pcmemData = pcMem;
				break;
			case OPE_MOVECARET:
				break;
			}
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				pcOpe->m_nCaretPosX_PHY_After,
				pcOpe->m_nCaretPosY_PHY_After,
				&nCaretPosX_After,
				&nCaretPosY_After
			);

			if( i == nOpeBlkNum - 1	){
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, TRUE );
			}else{
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, FALSE );
			}
		}
		/* Redo後の変更フラグ */
		m_pcEditDoc->SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */

		SetBracketPairPos( true );	// 03/03/07 ai

		/* 再描画 */
		hdc = ::GetDC( m_hWnd );
		ps.rcPaint.left = 0;
		ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
		ps.rcPaint.top = m_nViewAlignTop;
		ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//		OnKillFocus();
		OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
//		OnSetFocus();
		::ReleaseDC( m_hWnd, hdc );

		/* 行番号表示に必要な幅を設定 */
		if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
			/* キャレットの表示・更新 */
			ShowEditCaret();
		}

		m_pcEditDoc->RedrawInactivePane();/* 他のペインの表示状態を更新 */
#if 0
	//	2001/06/21 Start by asa-o: 他のペインの表示状態を更新
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^1].Redraw();
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^2].Redraw();
		m_pcEditDoc->m_cEditViewArr[(m_nMyIndex^1)^2].Redraw();
	//	2001/06/21 End
#endif
	}
	m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */

	return;
}












/* データ置換 削除&挿入にも使える */
void CEditView::ReplaceData_CEditView(
	int			nDelLineFrom,			/* 削除範囲行  From レイアウト行番号 */
	int			nDelColmFrom,			/* 削除範囲位置From レイアウト行桁位置 */
	int			nDelLineTo,				/* 削除範囲行  To   レイアウト行番号 */
	int			nDelColmTo,				/* 削除範囲位置To   レイアウト行桁位置 */
	CMemory*	pcmemCopyOfDeleted,		/* 削除されたデータのコピー(NULL可能) */
	const char*	pInsData,				/* 挿入するデータ */
	int			nInsDataLen,			/* 挿入するデータの長さ */
	BOOL		bRedraw
//	BOOL		bUndo					/* Undo操作かどうか */
)
{
	{
		//	Jun 23, 2000 genta
		//	変数名を書き換え忘れていたのを修正

		//	Jun. 1, 2000 genta
		//	DeleteDataから移動した

		//	May. 29, 2000 genta
		//	From Here
		//	行の後ろが選択されていたときの不具合を回避するため，
		//	選択領域から行末以降の部分を取り除く．

		//	先頭
		int len, pos;
		const char *line;
		const CLayout* pcLayout;
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nDelLineFrom, &len, &pcLayout );
		//	Jun. 1, 2000 genta
		//	ちゃんとNULLチェックしましょう
		if( line != NULL ){
			pos = LineColmnToIndex( pcLayout, nDelColmFrom );
			//	Jun. 1, 2000 genta
			//	同一行の行末以降のみが選択されている場合を考慮する

			//	Aug. 22, 2000 genta
			//	開始位置がEOFの後ろのときは次行に送る処理を行わない
			//	これをやってしまうと存在しない行をPointして落ちる．
			if( nDelLineFrom < m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 && pos >= len){
				if( nDelLineFrom == nDelLineTo  ){
					//	m_nSelectLineFrom <= m_nSelectLineTo はチェックしない
					++nDelLineFrom;
					nDelLineTo = nDelLineFrom;
					nDelColmTo = nDelColmFrom = 0;
				}
				else {
					nDelLineFrom++;
					nDelColmFrom = 0;
				}
			}
		}

		//	末尾
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nDelLineTo, &len, &pcLayout );
		if( line != NULL ){
			pos = LineIndexToColmn( pcLayout, len );

			if( nDelColmTo > pos ){
				nDelColmTo = pos;
			}
		}
		//	To Here
	}

	COpe* pcOpe = NULL;		/* 編集操作要素 COpe */
	CMemory* pcMemDeleted;
	int	nCaretPosXOld;
	int	nCaretPosYOld;
	int	nCaretPosX_PHY_Old;
	int	nCaretPosY_PHY_Old;

	nCaretPosXOld = m_nCaretPosX;
	nCaretPosYOld = m_nCaretPosY;
	nCaretPosX_PHY_Old = m_nCaretPosX_PHY;
	nCaretPosY_PHY_Old = m_nCaretPosY_PHY;
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;						/* 操作種別 */
//		pcOpe->m_nCaretPosX_Before = m_nCaretPosX;			/* 操作前のキャレット位置Ｘ */
//		pcOpe->m_nCaretPosY_Before = m_nCaretPosY;			/* 操作前のキャレット位置Ｙ */
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* 操作前のキャレット位置Ｘ */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* 操作前のキャレット位置Ｙ */


//		pcOpe->m_nCaretPosX_After = m_nCaretPosX;			/* 操作後のキャレット位置Ｘ */
//		pcOpe->m_nCaretPosY_After = m_nCaretPosY;			/* 操作後のキャレット位置Ｙ */
//		pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* 操作後のキャレット位置Ｘ */
//		pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* 操作後のキャレット位置Ｙ */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* 操作後のキャレット位置Ｙ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_DELETE;				/* 操作種別 */

//		pcOpe->m_nCaretPosX_Before = nDelColmFrom/*m_nCaretPosX*/;	/* 操作前のキャレット位置Ｘ */
//		pcOpe->m_nCaretPosY_Before = nDelLineFrom/*m_nCaretPosY*/;	/* 操作前のキャレット位置Ｙ */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_Before,
//			pcOpe->m_nCaretPosY_Before,
//			&pcOpe->m_nCaretPosX_PHY_Before,
//			&pcOpe->m_nCaretPosY_PHY_Before
//		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			nDelColmFrom/*m_nCaretPosX*/,
			nDelLineFrom/*m_nCaretPosY*/,
			&pcOpe->m_nCaretPosX_PHY_Before,
			&pcOpe->m_nCaretPosY_PHY_Before
		);

//		pcOpe->m_nCaretPosX_To = nDelColmTo;	/* 操作前のキャレット位置Ｘ */
//		pcOpe->m_nCaretPosY_To = nDelLineTo;	/* 操作前のキャレット位置Ｙ */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_To,
//			pcOpe->m_nCaretPosY_To,
//			&pcOpe->m_nCaretPosX_PHY_To,
//			&pcOpe->m_nCaretPosY_PHY_To
//		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			nDelColmTo,
			nDelLineTo,
			&pcOpe->m_nCaretPosX_PHY_To,
			&pcOpe->m_nCaretPosY_PHY_To
		);



//		pcOpe->m_nCaretPosX_After = pcOpe->m_nCaretPosX_Before;	/* 操作後のキャレット位置Ｘ */
//		pcOpe->m_nCaretPosY_After = pcOpe->m_nCaretPosY_Before;	/* 操作後のキャレット位置Ｙ */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* 操作後のキャレット位置Ｙ */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_After,
//			pcOpe->m_nCaretPosY_After,
//			&pcOpe->m_nCaretPosX_PHY_After,
//			&pcOpe->m_nCaretPosY_PHY_After
//		);
	}else{
		pcOpe = NULL;
	}
	pcMemDeleted = new CMemory;
	/*
	|| バッファサイズの調整
	*/
	if( 2000 < nDelLineTo - nDelLineFrom ){
		pcMemDeleted->AllocBuffer( 1024000 );
	}


	/* 文字列置換 */
//	int		nAddLineNum;	/* 再描画ヒント レイアウト行の増減 */
//	int		nModLineFrom;	/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
//	int		nModLineTo;		/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
//	int		nNewLine;		/* 挿入された部分の次の位置の行(レイアウト行) */
//	int		nNewPos;		/* 挿入された部分の次の位置のデータ位置(レイアウト桁位置) */
	LayoutReplaceArg	LRArg;
	LRArg.nDelLineFrom = nDelLineFrom;	/* 削除範囲行  From レイアウト行番号 */
	LRArg.nDelColmFrom = nDelColmFrom;	/* 削除範囲位置From レイアウト行桁位置 */
	LRArg.nDelLineTo = nDelLineTo;		/* 削除範囲行  To   レイアウト行番号 */
	LRArg.nDelColmTo = nDelColmTo;		/* 削除範囲位置To   レイアウト行桁位置 */
	LRArg.pcmemDeleted = pcMemDeleted;	/* 削除されたデータ */
	LRArg.pInsData = pInsData;			/* 挿入するデータ */
	LRArg.nInsDataLen = nInsDataLen;	/* 挿入するデータの長さ */
//	LRArg.nAddLineNum = 0;			/* 再描画ヒント レイアウト行の増減 */
//	LRArg.nModLineFrom = 0;			/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
//	LRArg.nModLineTo = 0;			/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
//	LRArg.nNewLine = 0;				/* 挿入された部分の次の位置の行(レイアウト行) */
//	LRArg.nNewPos = 0;				/* 挿入された部分の次の位置のデータ位置(レイアウト桁位置) */
//	LRArg.bUndo = m_bDoing_UndoRedo;					/* Undo操作かどうか */
	m_pcEditDoc->m_cLayoutMgr.ReplaceData_CLayoutMgr(
		&LRArg
#if 0
		nDelLineFrom,			/* 削除範囲行  From レイアウト行番号 */
		nDelColmFrom,			/* 削除範囲位置From レイアウト行桁位置 */
		nDelLineTo,				/* 削除範囲行  To   レイアウト行番号 */
		nDelColmTo,				/* 削除範囲位置To   レイアウト行桁位置 */
		pcMemDeleted,			/* 削除されたデータ */
		pInsData,				/* 挿入するデータ */
		nInsDataLen,			/* 挿入するデータの長さ */
		&nAddLineNum,			/* 再描画ヒント レイアウト行の増減 */
		&nModLineFrom,			/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
		&nModLineTo,			/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
		&nNewLine,				/* 挿入された部分の次の位置の行(レイアウト行) */
		&nNewPos,				/* 挿入された部分の次の位置のデータ位置(レイアウト桁位置) */

		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* シングルクォーテーション文字列を表示する */
		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp,	/* ダブルクォーテーション文字列を表示する */
		m_bDoing_UndoRedo	/* Undo操作かどうか */
#endif
	);

	//	Jan. 30, 2001 genta
	//	再描画の時点でファイル更新フラグが適切になっていないといけないので
	//	関数の末尾からここへ移動
	/* 状態遷移 */
	if( FALSE == m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	/* 現在の選択範囲を非選択状態に戻す */
	DisableSelectArea( bRedraw );

	/* 行番号表示に必要な幅を設定 */
	if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
		/* キャレットの表示・更新 */
		ShowEditCaret();
	}else{

		if( bRedraw ){
		/* 再描画 */
			HDC	hdc;
			PAINTSTRUCT ps;
			hdc = ::GetDC( m_hWnd );
			/* 再描画ヒント レイアウト行の増減 */
			//	Jan. 30, 2001 genta	貼り付けで行数が減る場合の考慮が抜けていた
			if( 0 != LRArg.nAddLineNum ){
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
				//ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (m_nCaretPosY - m_nViewTopLine); // 2002.02.25 Del By KK 次で上書きされているため未使用。
				//ps.rcPaint.top = 0/*m_nViewAlignTop*/;			// 2002.02.25 Del By KK
				ps.rcPaint.top = m_nViewAlignTop - m_nTopYohaku;	// ルーラーを範囲に含めない。2002.02.25 Add By KK
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			}else{
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;

				/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
				ps.rcPaint.top = m_nViewAlignTop + (LRArg.nModLineFrom - m_nViewTopLine)* (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				if( m_pcEditDoc->GetDocumentAttribute().m_bWordWrap
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuHead	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuTail	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.19 MIK
				{
					ps.rcPaint.top -= (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				}
				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}
				ps.rcPaint.bottom = m_nViewAlignTop + (LRArg.nModLineTo - m_nViewTopLine + 1)* (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				if( m_nViewAlignTop + m_nViewCy < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				}

			}
//			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
//			OnSetFocus();
			::ReleaseDC( m_hWnd, hdc );
		}
	}

	/* 削除されたデータのコピー(NULL可能) */
	if( NULL != pcmemCopyOfDeleted 		/* 削除されたデータのコピー(NULL可能) */
	 && 0 < pcMemDeleted->GetLength()	/* 削除したデータの長さ */
	){
		*pcmemCopyOfDeleted = *pcMemDeleted;
	}

	if( !m_bDoing_UndoRedo /* アンドゥ・リドゥの実行中か */
	 && 0 < pcMemDeleted->GetLength()	/* 削除したデータの長さ */
	){
		pcOpe->m_nDataLen = pcMemDeleted->GetLength();	/* 操作に関連するデータのサイズ */
		pcOpe->m_pcmemData = pcMemDeleted;				/* 操作に関連するデータ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}else{
		delete pcMemDeleted;
		pcMemDeleted = NULL;
	}


	if( !m_bDoing_UndoRedo		/* アンドゥ・リドゥの実行中か */
	 && 0 < nInsDataLen			/* 挿入するデータの長さ */
	){
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_INSERT;				/* 操作種別 */

//		pcOpe->m_nCaretPosX_Before = nDelColmFrom/*m_nCaretPosX*/;	/* 操作前のキャレット位置Ｘ */
//		pcOpe->m_nCaretPosY_Before = nDelLineFrom/*m_nCaretPosY*/;	/* 操作前のキャレット位置Ｙ */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			nDelColmFrom,
			nDelLineFrom,
			&pcOpe->m_nCaretPosX_PHY_Before,
			&pcOpe->m_nCaretPosY_PHY_Before
		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			LRArg.nNewPos,
			LRArg.nNewLine,
			&pcOpe->m_nCaretPosX_PHY_To,
			&pcOpe->m_nCaretPosY_PHY_To
		);
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_To;
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_To;


		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}else{
		pcOpe = NULL;
	}


	/* 挿入直後位置へカーソルを移動 */
	MoveCursor(
		LRArg.nNewPos,		/* 挿入された部分の次の位置のデータ位置(レイアウト桁位置) */
		LRArg.nNewLine,		/* 挿入された部分の次の位置の行(レイアウト行) */
		bRedraw
	);
	m_nCaretPosX_Prev = m_nCaretPosX;


	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
//		pcOpe->m_nCaretPosX_Before = nCaretPosXOld;			/* 操作前のキャレット位置Ｘ */
//		pcOpe->m_nCaretPosY_Before = nCaretPosYOld;			/* 操作前のキャレット位置Ｙ */
		pcOpe->m_nCaretPosX_PHY_Before = nCaretPosX_PHY_Old;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_nCaretPosY_PHY_Before = nCaretPosY_PHY_Old;	/* 操作後のキャレット位置Ｙ */

//		pcOpe->m_nCaretPosX_After = m_nCaretPosX;	/* 操作後のキャレット位置Ｘ */
//		pcOpe->m_nCaretPosY_After = m_nCaretPosY;	/* 操作後のキャレット位置Ｙ */
		pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* 操作後のキャレット位置Ｙ */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_After,
//			pcOpe->m_nCaretPosY_After,
//			&pcOpe->m_nCaretPosX_PHY_After,
//			&pcOpe->m_nCaretPosY_PHY_After
//		);
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	//	Jan. 30, 2001 genta
	//	ファイル全体の更新フラグが立っていないと各行の更新状態が表示されないので
	//	フラグ更新処理を再描画より前に移動する
	return;

}











/* C/C++スマートインデント処理 */
void CEditView::SmartIndent_CPP( char cChar )
{

//! 2000.1.12 編集動作とスマートインデントのUndo/Redoバッファを分ける場合
//!
//!	/* アンドゥバッファの処理 */
//!	if( NULL != m_pcOpeBlk ){
//!		if( 0 < m_pcOpeBlk->GetNum() ){	/* 操作の数を返す */
//!			/* 操作の追加 */
//!			m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
//!		}else{
//!			delete m_pcOpeBlk;
//!		}
//!		m_pcOpeBlk = NULL;
//!	}
//!	/* アンドゥバッファの処理 */
//!	m_pcOpeBlk = new COpeBlk;


	const char*	pLine;
	int			nLineLen;
	int			i;
	int			j;
	int			k;
	int			m;
	const char*	pLine2;
	int			nLineLen2;
	int			nLevel;

	/* 調整によって置換される箇所 */
	int			nXFm = -1;
	int			nYFm = -1;
	int			nXTo = -1;
	int			nYTo = -1;

	char*		pszData = NULL;
	int			nDataLen;

	int			nCPX;
	int			nCPY;
	COpe*		pcOpe = NULL;
	int			nWork;
	int			nCaretPosX_PHY;
	CDocLine*	pCDocLine = NULL;
	int			nCharChars;
	int			nSrcLen;
	char		pszSrc[1024];
	BOOL		bChange;


	switch( cChar ){
	case CR:
	case ':':
	case '}':
	case ')':
	case '{':
	case '(':
		break;
	default:
		return;
	}
	switch( cChar ){
	case CR:
	case ':':
	case '}':
	case ')':
	case '{':
	case '(':
		/* インデント調整可能か */
//		if( 0 >= m_nCaretPosY_PHY ){
//			return;
//		}
		nCaretPosX_PHY = m_nCaretPosX_PHY;

		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( m_nCaretPosY_PHY, &nLineLen );
		if( NULL == pLine ){
			if( CR != cChar ){
				return;
			}
			/* 調整によって置換される箇所 */
			nXFm = 0;
			nYFm = m_nCaretPosY_PHY;
			nXTo = 0;
			nYTo = m_nCaretPosY_PHY;
		}else{
			pCDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY );


			//	nWorkに処理の基準桁位置を設定する
			if( CR != cChar ){
				nWork = nCaretPosX_PHY - 1;
			}else{
				/*
				|| CRが入力された時、カーソル直後の識別子をインデントする。
				|| カーソル直後の識別子が'}'や')'ならば
				|| '}'や')'が入力された時と同じ処理をする
				*/

				for( i = nCaretPosX_PHY; i < nLineLen; i++ ){
					if( TAB != pLine[i] && SPACE != pLine[i] ){
						break;
					}
				}
				if( i < nLineLen ){
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
					if( 1 == nCharChars && ( pLine[i] == ')' || pLine[i] == '}' ) ){
						cChar = pLine[i];
					}
					nCaretPosX_PHY = i;
					nWork = nCaretPosX_PHY;
				}else{
					nWork = nCaretPosX_PHY;
				}
			}
			for( i = 0; i < nWork; i++ ){
				if( TAB != pLine[i] && SPACE != pLine[i] ){
					break;
				}
			}
			if( i < nWork ){
				if( ( ':' == cChar
					 && (
							0 == strncmp( &pLine[i], "case", 4 )
						 || 0 == strncmp( &pLine[i], "default:", 8 )
						 || 0 == strncmp( &pLine[i], "public:", 7 )
						 || 0 == strncmp( &pLine[i], "private:", 8 )
						 || 0 == strncmp( &pLine[i], "protected:", 10 )
						)
					)
					//	Sep. 18, 2002 かろと
					|| (( '{' == cChar ) && ( '#' != pLine[i] ))
					|| (( '(' == cChar ) && ( '#' != pLine[i] ))
				){

				}else{
					return;
				}
			}else{
				if( ':' == cChar ){
					return;
				}
			}
			/* 調整によって置換される箇所 */
			nXFm = 0;
			nYFm = m_nCaretPosY_PHY;
			nXTo = i;
			nYTo = m_nCaretPosY_PHY;
		}


		/* 対応する括弧をさがす */
		nLevel = 0;	/* {}の入れ子レベル */
//		bString = FALSE;


		nDataLen = 0;
		for( j = m_nCaretPosY_PHY; j >= 0; --j ){
			pLine2 = m_pcEditDoc->m_cDocLineMgr.GetLineStr( j, &nLineLen2 );
			if( j == m_nCaretPosY_PHY ){
				// 2005.10.11 ryoji EOF のみの行もスマートインデントの対象にする
				if( NULL == pLine2 ){
					if( m_nCaretPosY_PHY == m_pcEditDoc->m_cDocLineMgr.GetLineCount() )
						continue;	// EOF のみの行
					break;
				}
				nCharChars = &pLine2[nWork] - CMemory::MemCharPrev( pLine2, nLineLen2, &pLine2[nWork] );
				k = nWork - nCharChars;
			}else{
				if( NULL == pLine2 )
					break;
				nCharChars = &pLine2[nLineLen2] - CMemory::MemCharPrev( pLine2, nLineLen2, &pLine2[nLineLen2] );
				k = nLineLen2 - nCharChars;
			}

			for( ; k >= 0; /*k--*/ ){
				if( 1 == nCharChars && ( '}' == pLine2[k] || ')' == pLine2[k] )
				){
					if( 0 < k && '\'' == pLine2[k - 1]
					 && nLineLen2 - 1 > k && '\'' == pLine2[k + 1]
					){
//						MYTRACE( "▼[%s]\n", pLine2 );
					}else{
						//同じ行の場合
						if( j == m_nCaretPosY_PHY ){
							if( '{' == cChar && '}' == pLine2[k] ){
								cChar = '}';
								nLevel--;	/* {}の入れ子レベル */
//								return;
							}
							if( '(' == cChar && ')' == pLine2[k] ){
								cChar = ')';
								nLevel--;	/* {}の入れ子レベル */
//								return;
							}
						}

						nLevel++;	/* {}の入れ子レベル */
					}
				}
				if( 1 == nCharChars && ( '{' == pLine2[k] || '(' == pLine2[k] )
				){
					if( 0 < k && '\'' == pLine2[k - 1]
					 && nLineLen2 - 1 > k && '\'' == pLine2[k + 1]
					){
//						MYTRACE( "▼[%s]\n", pLine2 );
					}else{
						//同じ行の場合
						if( j == m_nCaretPosY_PHY ){
							if( '{' == cChar && '{' == pLine2[k] ){
								return;
							}
							if( '(' == cChar && '(' == pLine2[k] ){
								return;
							}
						}
						if( 0 == nLevel ){
							break;
						}else{
							nLevel--;	/* {}の入れ子レベル */
						}

					}
				}
				nCharChars = &pLine2[k] - CMemory::MemCharPrev( pLine2, nLineLen2, &pLine2[k] );
				if( 0 == nCharChars ){
					nCharChars = 1;
				}
				k -= nCharChars;
			}
			if( k < 0 ){
				/* この行にはない */
				continue;
			}

			for( m = 0; m < nLineLen2; m++ ){
				if( TAB != pLine2[m] && SPACE != pLine2[m] ){
					break;
				}
			}


			nDataLen = m;
			nCharChars = (m_pcEditDoc->GetDocumentAttribute().m_bInsSpace)? m_pcEditDoc->m_cLayoutMgr.GetTabSpace(): 1;
			pszData = new char[nDataLen + nCharChars + 1];
			memcpy( pszData, pLine2, nDataLen );
			if( CR  == cChar
			 || '{' == cChar
			 || '(' == cChar
			){
				// 2005.10.11 ryoji TABキーがSPACE挿入の設定なら追加インデントもSPACEにする
				//	既存文字列の右端の表示位置を求めた上で挿入するスペースの数を決定する
				if( m_pcEditDoc->GetDocumentAttribute().m_bInsSpace ){	// SPACE挿入設定
					i = m = 0;
					while( i < nDataLen ){
						nCharChars = CMemory::GetSizeOfChar( pszData, nDataLen, i );
						if( nCharChars == 1 && TAB == pszData[i] )
							m += m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace(m);
						else
							m += nCharChars;
						i += nCharChars;
					}
					nCharChars = m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace(m);
					for( i = 0; i < nCharChars; i++ )
						pszData[nDataLen + i] = SPACE;
					pszData[nDataLen + nCharChars] = '\0';
					nDataLen += nCharChars;
				}else{
					pszData[nDataLen] = TAB;
					pszData[nDataLen + 1] = '\0';
					++nDataLen;
				}
			}else{
				pszData[nDataLen] = '\0';

			}
			break;
		}
		if( j < 0 ){
			/* 対応する括弧が見つからなかった */
			if( CR == cChar ){
				return;
			}else{
				nDataLen = 0;
				pszData = new char[nDataLen + 1];
				pszData[nDataLen] = '\0';
			}
		}

		/* 調整後のカーソル位置を計算しておく */
		nCPX = nCaretPosX_PHY - nXTo + nDataLen;
		nCPY = m_nCaretPosY_PHY;

		nSrcLen = nXTo - nXFm;
		if( nSrcLen >= sizeof( pszSrc ) - 1 ){
			//	Sep. 18, 2002 genta メモリリーク対策
			delete [] pszData;
			return;
		}
		if( NULL == pLine ){
			pszSrc[0] = '\0';
		}else{
			memcpy( pszSrc, &pLine[nXFm], nSrcLen );
			pszSrc[nSrcLen] = '\0';
		}


		/* 調整によって置換される箇所 */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nXFm, nYFm, &nXFm, &nYFm );
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nXTo, nYTo, &nXTo, &nYTo );

		if( ( 0 == nDataLen && nYFm == nYTo && nXFm == nXTo )
		 || ( nDataLen == nSrcLen && 0 == memcmp( pszSrc, pszData, nDataLen ) )
		 ){
			bChange = FALSE;
		}else{
			bChange = TRUE;

			/* データ置換 削除&挿入にも使える */
			ReplaceData_CEditView(
				nYFm,		/* 削除範囲行  From レイアウト行番号 */
				nXFm,		/* 削除範囲位置From レイアウト行桁位置 */
				nYTo,		/* 削除範囲行  To   レイアウト行番号 */
				nXTo,		/* 削除範囲位置To   レイアウト行桁位置 */
				NULL,		/* 削除されたデータのコピー(NULL可能) */
				pszData,	/* 挿入するデータ */
				nDataLen,	/* 挿入するデータの長さ */
				TRUE
			//	BOOL		bUndo			/* Undo操作かどうか */
			);
		}


		/* カーソル位置調整 */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nCPX, nCPY, &nCPX, &nCPY );
		/* 選択エリアの先頭へカーソルを移動 */
		MoveCursor( nCPX, nCPY, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;


		if( bChange && !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* 操作前のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* 操作前のキャレット位置Ｙ */
			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		break;
	}
	if( NULL != pszData ){
		delete [] pszData;
		pszData = NULL;
	}
	return;
}


/* 2005.10.11 ryoji 前の行にある末尾の空白を削除 */
void CEditView::RTrimPrevLine( void )
{
	const char*	pLine;
	int			nLineLen;
	int			i;
	int			j;
	int			nXFm;
	int			nYFm;
	int			nXTo;
	int			nYTo;
	int			nCPX;
	int			nCPY;
	int			nCharChars;
	int			nCaretPosX_PHY;
	int			nCaretPosY_PHY;
	COpe*		pcOpe = NULL;

	nCaretPosX_PHY = m_nCaretPosX_PHY;
	nCaretPosY_PHY = m_nCaretPosY_PHY;

	if( m_nCaretPosY_PHY > 0 ){
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStrWithoutEOL( m_nCaretPosY_PHY - 1, &nLineLen );
		if( NULL != pLine && nLineLen > 0 ){
			i = j = 0;
			while( i < nLineLen ){
				nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
				if( 1 == nCharChars ){
					if( TAB != pLine[i] && SPACE != pLine[i])
						j = i + nCharChars;
				}
				else if( 2 == nCharChars ){
					if( !((unsigned char)pLine[i] == (unsigned char)0x81 && (unsigned char)pLine[i + 1] == (unsigned char)0x40) )
						j = i + nCharChars;
				}
				i += nCharChars;
			}
			if( j < nLineLen ){
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( j, m_nCaretPosY_PHY - 1, &nXFm, &nYFm );
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nLineLen, m_nCaretPosY_PHY - 1, &nXTo, &nYTo );
				if( !( nXFm >= nXTo && nYFm == nYTo) ){
					ReplaceData_CEditView(
						nYFm,		/* 削除範囲行  From レイアウト行番号 */
						nXFm,		/* 削除範囲位置From レイアウト行桁位置 */
						nYTo,		/* 削除範囲行  To   レイアウト行番号 */
						nXTo,		/* 削除範囲位置To   レイアウト行桁位置 */
						NULL,		/* 削除されたデータのコピー(NULL可能) */
						NULL,		/* 挿入するデータ */
						0,			/* 挿入するデータの長さ */
						TRUE
					);
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nCaretPosX_PHY, nCaretPosY_PHY, &nCPX, &nCPY );
					MoveCursor( nCPX, nCPY, TRUE );

					if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
						pcOpe = new COpe;
						pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
						pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* 操作前のキャレット位置Ｘ */
						pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* 操作前のキャレット位置Ｙ */
						pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* 操作後のキャレット位置Ｘ */
						pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* 操作後のキャレット位置Ｙ */
						/* 操作の追加 */
						m_pcOpeBlk->AppendOpe( pcOpe );
					}
				}
			}
		}
	}
}


// 2001/06/20 Start by asa-o

// テキストを１行下へスクロール
void CEditView::Command_WndScrollDown( void )
{
	int	nCaretMarginY;

	nCaretMarginY = m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	nCaretMarginY += 2;

	if( m_nCaretPosY > m_nViewRowNum + m_nViewTopLine - (nCaretMarginY + 1) ){
		m_cUnderLine.CaretUnderLineOFF( TRUE );
	}

	//	Sep. 11, 2004 genta 同期用に行数を記憶
	//	Sep. 11, 2004 genta 同期スクロールの関数化
	SyncScrollV( ScrollAtV(m_nViewTopLine - 1));

	// テキストが選択されていない
	if( !IsTextSelected() )
	{
		// カーソルが画面外に出た
		if( m_nCaretPosY > m_nViewRowNum + m_nViewTopLine - nCaretMarginY )
		{
// From Here 2001.12.03 hor
//			if( m_nCaretPosY > m_pcEditDoc->m_cDocLineMgr.GetLineCount() - nCaretMarginY )
//				Cursor_UPDOWN( (m_pcEditDoc->m_cDocLineMgr.GetLineCount() - nCaretMarginY) - m_nCaretPosY, FALSE );
// To Here 2001.12.03 hor
			if( m_nCaretPosY > m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nCaretMarginY )
				Cursor_UPDOWN( (m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nCaretMarginY) - m_nCaretPosY, FALSE );
			else
				Cursor_UPDOWN( -1, FALSE);
			DrawCaretPosInfo();
		}
	}

	m_cUnderLine.CaretUnderLineON( TRUE );
}

// テキストを１行上へスクロール
void CEditView::Command_WndScrollUp(void)
{
	int	nCaretMarginY;

	nCaretMarginY = m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	if( m_nCaretPosY < m_nViewTopLine + (nCaretMarginY + 1) ){
		m_cUnderLine.CaretUnderLineOFF( TRUE );
	}

	//	Sep. 11, 2004 genta 同期用に行数を記憶
	//	Sep. 11, 2004 genta 同期スクロールの関数化
	SyncScrollV( ScrollAtV( m_nViewTopLine + 1 ));

	// テキストが選択されていない
	if( !IsTextSelected() )
	{
		// カーソルが画面外に出た
		if( m_nCaretPosY < m_nViewTopLine + nCaretMarginY )
		{
			if( m_nViewTopLine == 1 )
				Cursor_UPDOWN( nCaretMarginY + 1, FALSE );
			else
				Cursor_UPDOWN( 1, FALSE );
			DrawCaretPosInfo();
		}
	}

	m_cUnderLine.CaretUnderLineON( TRUE );
}

// 2001/06/20 End



/* 次の段落へ進む
	2002/04/26 段落の両端で止まるオプションを追加
	2002/04/19 新規
*/
void CEditView::Command_GONEXTPARAGRAPH( int bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = 0;
	
	bool nFirstLineIsEmptyLine = false;
	/* まずは、現在位置が空行（スペース、タブ、改行記号のみの行）かどうか判別 */
	if ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer++;
	}
	else {
		// EOF行でした。
		return;
	}

	/* 次に、nFirstLineIsEmptyLineと異なるところまで読み飛ばす */
	while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer++;
		}
		else {
			break;
		}
	};

	/*	nFirstLineIsEmptyLineが空行だったら、今見ているところは非空行。すなわちおしまい。
		nFirstLineIsEmptyLineが非空行だったら、今見ているところは空行。
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	おしまい。
	}
	else {
		//	いま見ているところは空行の1行目
		if ( m_pShareData->m_Common.m_bStopsBothEndsWhenSearchParagraph ){	//	段落の両端で止まる
		}
		else {
			/* 仕上げに、空行じゃないところまで進む */
			while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					nCaretPointer++;
				}
				else {
					break;
				}
			};
		}
	}

	//	EOFまで来たり、目的の場所まできたので移動終了。

	/* 移動距離を計算 */
	int nCaretPosX_Layo;
	int nCaretPosY_Layo;

	/* 移動前の物理位置 */
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		m_nCaretPosX_PHY, m_nCaretPosY_PHY,
		&nCaretPosX_Layo, &nCaretPosY_Layo
	);

	/* 移動後の物理位置 */
	int nCaretPosY_Layo_CaretPointer;
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		m_nCaretPosX_PHY, m_nCaretPosY_PHY + nCaretPointer,
		&nCaretPosX_Layo, &nCaretPosY_Layo_CaretPointer
	);

	Cursor_UPDOWN( nCaretPosY_Layo_CaretPointer - nCaretPosY_Layo, bSelect );
	return;
}

/* 前の段落へ進む
	2002/04/26 段落の両端で止まるオプションを追加
	2002/04/19 新規
*/
void CEditView::Command_GOPREVPARAGRAPH( int bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = -1;

	bool nFirstLineIsEmptyLine = false;
	/* まずは、現在位置が空行（スペース、タブ、改行記号のみの行）かどうか判別 */
	if ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer--;
	}
	else {
		nFirstLineIsEmptyLine = true;
		nCaretPointer--;
	}

	/* 次に、nFirstLineIsEmptyLineと異なるところまで読み飛ばす */
	while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer--;
		}
		else {
			break;
		}
	};

	/*	nFirstLineIsEmptyLineが空行だったら、今見ているところは非空行。すなわちおしまい。
		nFirstLineIsEmptyLineが非空行だったら、今見ているところは空行。
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	おしまい。
		if ( m_pShareData->m_Common.m_bStopsBothEndsWhenSearchParagraph ){	//	段落の両端で止まる
			nCaretPointer++;	//	空行の最上行（段落の末端の次の行）で止まる。
		}
		else {
			/* 仕上げに、空行じゃないところまで進む */
			while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					break;
				}
				else {
					nCaretPointer--;
				}
			};
			nCaretPointer++;	//	空行の最上行（段落の末端の次の行）で止まる。
		}
	}
	else {
		//	いま見ているところは空行の1行目
		if ( m_pShareData->m_Common.m_bStopsBothEndsWhenSearchParagraph ){	//	段落の両端で止まる
			nCaretPointer++;
		}
		else {
			nCaretPointer++;
		}
	}

	//	EOFまで来たり、目的の場所まできたので移動終了。

	/* 移動距離を計算 */
	int nCaretPosX_Layo;
	int nCaretPosY_Layo;

	/* 移動前の物理位置 */
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		m_nCaretPosX_PHY, m_nCaretPosY_PHY,
		&nCaretPosX_Layo, &nCaretPosY_Layo
	);

	/* 移動後の物理位置 */
	int nCaretPosY_Layo_CaretPointer;
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		m_nCaretPosX_PHY, m_nCaretPosY_PHY + nCaretPointer,
		&nCaretPosX_Layo, &nCaretPosY_Layo_CaretPointer
	);

	Cursor_UPDOWN( nCaretPosY_Layo_CaretPointer - nCaretPosY_Layo, bSelect );
	return;
}

// From Here 2001.12.03 hor

//! ブックマークの設定・解除を行う(トグル動作)
void CEditView::Command_BOOKMARK_SET(void)
{
	CDocLine*	pCDocLine;
	int			nX=0;
	int			nY;
	int			nYfrom,nYto;
	if( IsTextSelected() && m_nSelectLineFrom<m_nSelectLineTo ){
		nYfrom=m_nSelectLineFrom;
		nYto  =m_nSelectLineTo;
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(nX,nYfrom,&nX,&nYfrom);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(nX,nYto,&nX,&nYto);
		for(nY=nYfrom;nY<=nYto;nY++){
			pCDocLine=m_pcEditDoc->m_cDocLineMgr.GetLineInfo( nY );
			if(NULL!=pCDocLine)pCDocLine->SetBookMark(!pCDocLine->IsBookMarked());
		}
	}else{
		pCDocLine=m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY );
		if(NULL!=pCDocLine)pCDocLine->SetBookMark(!pCDocLine->IsBookMarked());
	}
	// 2002.01.16 hor 分割したビューも更新
	for( int v = 0; v < 4; ++v ) if( m_pcEditDoc->m_nActivePaneIndex != v )m_pcEditDoc->m_cEditViewArr[v].Redraw();
	Redraw();
	return;
}



//! 次のブックマークを探し，見つかったら移動する
void CEditView::Command_BOOKMARK_NEXT(void)
{
//	CDocLine*	pCDocLine;
	int			nX=0;
	int			nY;
//	int			nRet;
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor
	nY=m_nCaretPosY_PHY;
	nYOld=nY;						// hor
re_do:;								// hor
	if(m_pcEditDoc->m_cDocLineMgr.SearchBookMark(nY, 1 /* 後方検索 */, &nY)){
		bFound = TRUE;				// hor
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(nX,nY,&nX,&nY);
		//	2006.07.09 genta 新規関数にまとめた
		MoveCursorSelecting( nX, nY, m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(m_pShareData->m_Common.m_bSearchAll){
		if(!bFound	&&		// 見つからなかった
			bRedo			// 最初の検索
		){
			nY=-1;	//	2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;		// 先頭から再検索
		}
	}
	if(bFound){
		if(nYOld >= nY)SendStatusMessage("▼先頭から再検索しました");
	}else{
		SendStatusMessage("▽見つかりませんでした");
		if(m_pShareData->m_Common.m_bNOTIFYNOTFOUND)	/* 検索／置換  見つからないときメッセージを表示 */
			::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
				"後方(↓) にブックマークが見つかりません。" );
	}
	return;
}



//! 前のブックマークを探し，見つかったら移動する．
void CEditView::Command_BOOKMARK_PREV(void)
{
//	CDocLine*	pCDocLine;
	int			nX=0;
	int			nY;
//	int			nRet;
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor
	nY=m_nCaretPosY_PHY;
	nYOld=nY;						// hor
re_do:;								// hor
	if(m_pcEditDoc->m_cDocLineMgr.SearchBookMark(nY, 0 /* 前方検索 */, &nY)){
		bFound = TRUE;				// hor
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(nX,nY,&nX,&nY);
		//	2006.07.09 genta 新規関数にまとめた
		MoveCursorSelecting( nX, nY, m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(m_pShareData->m_Common.m_bSearchAll){
		if(!bFound	&&	// 見つからなかった
			bRedo		// 最初の検索
		){
			nY=m_pcEditDoc->m_cLayoutMgr.GetLineCount();	//	2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;	// 末尾から再検索
		}
	}
	if(bFound){
		if(nYOld <= nY)SendStatusMessage("▲末尾から再検索しました");
	}else{
		SendStatusMessage("△見つかりませんでした");
		if(m_pShareData->m_Common.m_bNOTIFYNOTFOUND)	/* 検索／置換  見つからないときメッセージを表示 */
			::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
				"前方(↑) にブックマークが見つかりません。" );
	}
	return;
}



//! ブックマークをクリアする
void CEditView::Command_BOOKMARK_RESET(void)
{
	m_pcEditDoc->m_cDocLineMgr.ResetAllBookMark();
	// 2002.01.16 hor 分割したビューも更新
	for( int v = 0; v < 4; ++v ) if( m_pcEditDoc->m_nActivePaneIndex != v )m_pcEditDoc->m_cEditViewArr[v].Redraw();
	Redraw();
	return;
}


//指定パターンに一致する行をマーク 2002.01.16 hor
//キーマクロで記録できるように	2002.02.08 hor
void CEditView::Command_BOOKMARK_PATTERN( void )
{
	//検索or置換ダイアログから呼び出された
	if(!ChangeCurRegexp())return;
	m_pcEditDoc->m_cDocLineMgr.MarkSearchWord(
		m_pShareData->m_szSEARCHKEYArr[0],		/* 検索条件 */
		m_pShareData->m_Common.m_bRegularExp,	/* 1==正規表現 */
		m_pShareData->m_Common.m_bLoHiCase,		/* 1==英大文字小文字の区別 */
		m_pShareData->m_Common.m_bWordOnly,		/* 1==単語のみ検索 */
		&m_CurRegexp							/* 正規表現コンパイルデータ */
	);
	// 2002.01.16 hor 分割したビューも更新
	for( int v = 0; v < 4; ++v ) if( m_pcEditDoc->m_nActivePaneIndex != v )m_pcEditDoc->m_cEditViewArr[v].Redraw();
	Redraw();
	return;
}



/*! TRIM Step1

	非選択時はカレント行を選択して ConvSelectedArea → ConvMemory へ
	
	@param bLeft [in] FALSE: 右TRIM / それ以外: 左TRIM
	@author hor
	@date 2001.12.03 hor 新規作成
*/
void CEditView::Command_TRIM( BOOL bLeft )
{
	bool bBeDisableSelectArea = false;
	if(!IsTextSelected()){	//	非選択時は行選択に変更
		m_nSelectLineFrom = m_nCaretPosY;
		m_nSelectColmFrom = 0;
		m_nSelectLineTo   = m_nCaretPosY; 
		//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
		m_nSelectColmTo   = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();
		bBeDisableSelectArea = true;
	}
	if(bLeft){
		ConvSelectedArea( F_LTRIM );
	}else{
		ConvSelectedArea( F_RTRIM );
	}
	if (bBeDisableSelectArea) DisableSelectArea( TRUE );
	return;
}

/*! TRIM Step2
	ConvMemory から 戻ってきた後の処理．
	CMemory.cppのなかに置かないほうが良いかなと思ってこちらに置きました．
	
	@author hor
	@date 2001.12.03 hor 新規作成
*/
void CEditView::Command_TRIM2( CMemory* pCMemory , BOOL bLeft )
{
	const char*	pLine;
	int			nLineLen;
	char*		pDes;
	int			nBgn;
	int			i,j;
	int			nPosDes;
	CEOL		cEol;
	int			nCharChars;

	nBgn = 0;
	nPosDes = 0;
	/* 変換後に必要なバイト数を調べる */
	while( NULL != ( pLine = GetNextLine( pCMemory->GetPtr(), pCMemory->GetLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory変更
		if( 0 < nLineLen ){
			nPosDes += nLineLen;
		}
		nPosDes += cEol.GetLen();
	}
	if( 0 >= nPosDes ){
		return;
	}
	pDes = new char[nPosDes + 1];
	nBgn = 0;
	nPosDes = 0;
	if( bLeft ){
	// LTRIM
		while( NULL != ( pLine = GetNextLine( pCMemory->GetPtr(), pCMemory->GetLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory変更
			if( 0 < nLineLen ){
				for( i = 0; i <= nLineLen; ++i ){
					if( pLine[i] ==' ' ||
						pLine[i] =='\t'){
						continue;
					}else if( (unsigned char)pLine[i] == (unsigned char)0x81 && (unsigned char)pLine[i + 1] == (unsigned char)0x40 ){
						++i;
						continue;
					}else{
						break;
					}
				}
				if(nLineLen-i>0){
					memcpy( &pDes[nPosDes], (const char *)&pLine[i], nLineLen );
					nPosDes+=nLineLen-i;
				}
			}
			memcpy( &pDes[nPosDes], cEol.GetValue(), cEol.GetLen() );
			nPosDes += cEol.GetLen();
		}
	}else{
	// RTRIM
		while( NULL != ( pLine = GetNextLine( pCMemory->GetPtr(), pCMemory->GetLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory変更
			if( 0 < nLineLen ){
				// 2005.10.11 ryoji 右から遡るのではなく左から探すように修正（"ａ@" の右２バイトが全角空白と判定される問題の対処）
				i = j = 0;
				while( i < nLineLen ){
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
					if( 1 == nCharChars ){
						if( TAB != pLine[i] && SPACE != pLine[i])
							j = i + nCharChars;
					}
					else if( 2 == nCharChars ){
						if( !((unsigned char)pLine[i] == (unsigned char)0x81 && (unsigned char)pLine[i + 1] == (unsigned char)0x40) )
							j = i + nCharChars;
					}
					i += nCharChars;
				}
				if(j>0){
					memcpy( &pDes[nPosDes], (const char *)&pLine[0], j );
					nPosDes+=j;
				}
			}
			memcpy( &pDes[nPosDes], cEol.GetValue(), cEol.GetLen() );
			nPosDes += cEol.GetLen();
		}
	}
	pDes[nPosDes] = '\0';

	pCMemory->SetData( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return;
}

/*!	物理行のソートに使う構造体*/
typedef struct _SORTTABLE {
	string sKey1;
	string sKey2;
} SORTDATA, *SORTTABLE;
/*!	物理行のソートに使う関数(昇順) */
bool SortByKeyAsc (SORTTABLE pst1, SORTTABLE pst2) {return (pst1->sKey1<pst2->sKey1);}
/*!	物理行のソートに使う関数(降順) */
bool SortByKeyDesc(SORTTABLE pst1, SORTTABLE pst2) {return (pst1->sKey1>pst2->sKey1);}

/*!	@brief 物理行のソート

	非選択時は何も実行しない．矩形選択時は、その範囲をキーにして物理行をソート．
	
	@note とりあえず改行コードを含むデータをソートしているので、
	ファイルの最終行はソート対象外にしています
	@author hor
	@date 2001.12.03 hor 新規作成
	@date 2001.12.21 hor 選択範囲の調整ロジックを訂正
*/
void CEditView::Command_SORT(BOOL bAsc)	//bAsc:TRUE=昇順,FALSE=降順
{
	int			nLFO , nSelectLineFromOld;	/* 範囲選択開始行 */
	int			nCFO , nSelectColFromOld ; 	/* 範囲選択開始桁 */
	int			nLTO , nSelectLineToOld  ;	/* 範囲選択終了行 */
	int			nCTO , nSelectColToOld   ;	/* 範囲選択終了桁 */
	int			nColmFrom,nColmTo;
	int			nCF,nCT;
	int			nCaretPosYOLD;
	BOOL		bBeginBoxSelectOld;
	const char*	pLine;
	int			nLineLen;
	int			i,j;
	CMemory		cmemBuf;
	std::vector<SORTTABLE> sta;
	COpe*		pcOpe = NULL;

	if( !IsTextSelected() ){			/* テキストが選択されているか */
		return;
	}

	if( m_bBeginBoxSelect ){
	// 2001.12.21 hor 間違い
	//	if( m_nSelectLineTo >= m_pcEditDoc->m_cLayoutMgr.GetLineCount()-1 ) {
	//		--m_nSelectLineTo;
	//	}
		nLFO = m_nSelectLineFrom;
		nCFO = m_nSelectColmFrom;
		nLTO = m_nSelectLineTo;
		nCTO = m_nSelectColmTo;
		if( m_nSelectColmFrom==m_nSelectColmTo ){
			//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
			m_nSelectColmTo=m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();
		}
		if(m_nSelectColmFrom<m_nSelectColmTo){
			nCF=m_nSelectColmFrom;
			nCT=m_nSelectColmTo;
		}else{
			nCF=m_nSelectColmTo;
			nCT=m_nSelectColmFrom;
		}
	}
	bBeginBoxSelectOld=m_bBeginBoxSelect;
	nCaretPosYOLD=m_nCaretPosY;
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nSelectColmFrom,m_nSelectLineFrom,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nSelectColmTo,m_nSelectLineTo,
		&nSelectColToOld,&nSelectLineToOld
	);
	if( bBeginBoxSelectOld ){
		++nSelectLineToOld;
	}else{
		// カーソル位置が行頭じゃない ＆ 選択範囲の終端に改行コードがある場合は
		// その行も選択範囲に加える
		if ( nSelectColToOld > 0 ) {
			// 2006.03.31 Moca nSelectLineToOldは、物理行なのでLayout系からDocLine系に修正
			const CDocLine* pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( nSelectLineToOld );
			if( NULL != pcDocLine && EOL_NONE != pcDocLine->m_cEol ){
				++nSelectLineToOld;
			}
		}
	}
	nSelectColFromOld = 0;
	nSelectColToOld = 0;

	//行選択されてない
	if(nSelectLineFromOld==nSelectLineToOld){
		return;
	}

	for( i = nSelectLineFromOld; i < nSelectLineToOld; i++ ){
		const CDocLine* pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( i );
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( i, &nLineLen );
		if( NULL == pLine ) continue;
		SORTTABLE pst = new SORTDATA;
		if( bBeginBoxSelectOld ){
			nColmFrom = LineColmnToIndex( pcDocLine, nCF );
			nColmTo   = LineColmnToIndex( pcDocLine, nCT );
			if(nColmTo<nLineLen){	// BOX選択範囲の右端が行内に収まっている場合
				// 2006.03.31 genta std::string::assignを使って一時変数削除
				pst->sKey1.assign( &pLine[nColmFrom], nColmTo-nColmFrom );
			}else
			if(nColmFrom<nLineLen){	// BOX選択範囲の右端が行末より右にはみ出している場合
				pst->sKey1=&pLine[nColmFrom];
			}
			pst->sKey2=pLine;
		}else{
			pst->sKey1=pLine;
		}
		sta.push_back(pst);
	}
	if(bAsc){
		std::stable_sort(sta.begin(), sta.end(), SortByKeyAsc);
	}else{
		std::stable_sort(sta.begin(), sta.end(), SortByKeyDesc);
	}
	cmemBuf.SetDataSz( "" );
	j=(int)sta.size();
	if( bBeginBoxSelectOld ){
		for (i=0; i<j; i++) cmemBuf.AppendSz( sta[i]->sKey2.c_str() ); 
	}else{
		for (i=0; i<j; i++) cmemBuf.AppendSz( sta[i]->sKey1.c_str() );
	}
	//sta.clear(); ←これじゃだめみたい
	for (i=0; i<j; i++) delete sta[i];
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		nSelectColFromOld,nSelectLineFromOld,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		nSelectColToOld,nSelectLineToOld,
		&nSelectColToOld,&nSelectLineToOld
	);
	ReplaceData_CEditView(
		nSelectLineFromOld,
		nSelectColFromOld,
		nSelectLineToOld,
		nSelectColToOld,
		NULL,					/* 削除されたデータのコピー(NULL可能) */
		cmemBuf.GetPtr(),
		cmemBuf.GetLength(),
		FALSE
	);
	//	選択エリアの復元
	if(bBeginBoxSelectOld){
		m_bBeginBoxSelect=bBeginBoxSelectOld;
		m_nSelectLineFrom=nLFO;
		m_nSelectColmFrom=nCFO;
		m_nSelectLineTo  =nLTO;
		m_nSelectColmTo  =nCTO;
	}else{
		m_nSelectLineFrom=nSelectLineFromOld;
		m_nSelectColmFrom=nSelectColFromOld;
		m_nSelectLineTo  =nSelectLineToOld;
		m_nSelectColmTo  =nSelectColToOld;
	}
	if(nCaretPosYOLD==m_nSelectLineFrom || m_bBeginBoxSelect ) {
		MoveCursor( m_nSelectColmFrom, m_nSelectLineFrom, TRUE );
	}else{
		MoveCursor( m_nSelectColmTo, m_nSelectLineTo, TRUE );
	}
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* 操作前のキャレット位置Ｘ */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* 操作前のキャレット位置Ｙ */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* 操作後のキャレット位置Ｙ */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	RedrawAll();
}


/*! @brief 物理行のマージ

	連続する物理行で内容が同一の物を1行にまとめます．
	
	矩形選択時はなにも実行しません．
	
	@note 改行コードを含むデータを比較しているので、
	ファイルの最終行はソート対象外にしています
	
	@author hor
	@date 2001.12.03 hor 新規作成
	@date 2001.12.21 hor 選択範囲の調整ロジックを訂正
*/
void CEditView::Command_MERGE(void)
{
	int			nSelectLineFromOld;	/* 範囲選択開始行 */
	int			nSelectColFromOld ; /* 範囲選択開始桁 */
	int			nSelectLineToOld  ;	/* 範囲選択終了行 */
	int			nSelectColToOld   ;	/* 範囲選択終了桁 */
	int			nCaretPosYOLD;
	const char*	pLine;
	const char*	pLinew;
	int			nLineLen;
	int			i,j;
	CMemory		cmemBuf;
	COpe*		pcOpe = NULL;

	if( !IsTextSelected() ){			/* テキストが選択されているか */
		return;
	}
	if( m_bBeginBoxSelect ){
		return;
	}

	nCaretPosYOLD=m_nCaretPosY;
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nSelectColmFrom,m_nSelectLineFrom,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nSelectColmTo,m_nSelectLineTo,
		&nSelectColToOld,&nSelectLineToOld
	);

	// 2001.12.21 hor
	// カーソル位置が行頭じゃない ＆ 選択範囲の終端に改行コードがある場合は
	// その行も選択範囲に加える
	if ( nSelectColToOld > 0 ) {
		const CLayout* pcLayout=m_pcEditDoc->m_cLayoutMgr.Search(nSelectLineToOld);
		if( NULL != pcLayout && EOL_NONE != pcLayout->m_cEol ){
			++nSelectLineToOld;
		}
	}

	nSelectColFromOld = 0;
	nSelectColToOld = 0;

	//行選択されてない
	if(nSelectLineFromOld==nSelectLineToOld){
		return;
	}

	pLinew=NULL;
	cmemBuf.SetDataSz( "" );
	for( i = nSelectLineFromOld; i < nSelectLineToOld; i++ ){
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( i, &nLineLen );
		if( NULL == pLine ) continue;
		if( NULL == pLinew || strcmp(pLine,pLinew) ){
			cmemBuf.AppendSz( pLine );
		}
		pLinew=pLine;
	}
	j=m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		nSelectColFromOld,nSelectLineFromOld,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		nSelectColToOld,nSelectLineToOld,
		&nSelectColToOld,&nSelectLineToOld
	);
	ReplaceData_CEditView(
		nSelectLineFromOld,
		nSelectColFromOld,
		nSelectLineToOld,
		nSelectColToOld,
		NULL,					/* 削除されたデータのコピー(NULL可能) */
		cmemBuf.GetPtr(),
		cmemBuf.GetLength(),
		FALSE
	);
	j-=m_pcEditDoc->m_cDocLineMgr.GetLineCount();

	//	選択エリアの復元
	m_nSelectLineFrom=nSelectLineFromOld;
	m_nSelectColmFrom=nSelectColFromOld;
	m_nSelectLineTo  =nSelectLineToOld-j;
	m_nSelectColmTo  =nSelectColToOld;
	if(nCaretPosYOLD==m_nSelectLineFrom){
		MoveCursor( m_nSelectColmFrom, m_nSelectLineFrom, TRUE );
	}else{
		MoveCursor( m_nSelectColmTo, m_nSelectLineTo, TRUE );
	}
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* 操作前のキャレット位置Ｘ */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* 操作前のキャレット位置Ｙ */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* 操作後のキャレット位置Ｙ */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	RedrawAll();

	if(j){
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_TOPMOST, GSTR_APPNAME,
			"%d行をマージしました。", j
		);
	}else{
		::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
			"マージ可能な行がみつかりませんでした。" );
	}
}



// To Here 2001.12.03 hor
	
/* メニューからの再変換対応 minfu 2002.04.09

	@date 2002.04.11 YAZAKI COsVersionInfoのカプセル化を守りましょう。
*/
void CEditView::Command_Reconvert(void)
{
	int		nSize;
	PRECONVERTSTRING	pReconv;
	COsVersionInfo cOs;
//	POSVERSIONINFO	pOsVer;
	bool	bUseUnicodeATOK;
	HIMC hIMC ;
	
	
	//サイズを取得
	nSize = SetReconvertStruct(NULL,false);
	
	if( 0 == nSize )  // サイズ０の時は何もしない
		return ;
	
	bUseUnicodeATOK = false;
	//バージョンチェック
//	pOsVer  = cOs.GetOsVersionInfo();
	if( cOs.OsDoesNOTSupportReconvert() ){
		
		// MSIMEかどうか
		HWND hWnd = ImmGetDefaultIMEWnd(m_hWnd);
		if (SendMessage(hWnd, m_uWM_MSIME_RECONVERTREQUEST, FID_RECONVERT_VERSION, 0)){
			SendMessage(hWnd, m_uWM_MSIME_RECONVERTREQUEST, 0, (LPARAM)m_hWnd);
			return ;
		}
		// ATOKが使えるかどうか
		
		//説明の取得
		char sz[256];
		
		ImmGetDescription(GetKeyboardLayout(0),sz,256);
		if ( (strncmp(sz,"ATOK",4) == 0) && (NULL != AT_ImmSetReconvertString) ){
			bUseUnicodeATOK = true;
		}else{
			//対応IMEなし
			return;
		}
	}else{
		//現在のIMEが対応しているかどうか
		//IMEのプロパティ
		if ( !(ImmGetProperty(GetKeyboardLayout(0),IGP_SETCOMPSTR) & SCS_CAP_SETRECONVERTSTRING) ){
			//対応IMEなし			
			return ;
		}
	}
	
	//IMEのコンテキスト取得
	hIMC = ::ImmGetContext( m_hWnd );
	
	//領域確保
	pReconv = (PRECONVERTSTRING)::HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, nSize);
	
	//構造体設定
	SetReconvertStruct( pReconv, bUseUnicodeATOK);
	
	//変換範囲の調整
	if(bUseUnicodeATOK){
		(*AT_ImmSetReconvertString)(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize, NULL,0);
	}
	//調整した変換範囲を選択する
	SetSelectionFromReonvert(pReconv, bUseUnicodeATOK);
	
	//再変換実行
	if(bUseUnicodeATOK){
		(*AT_ImmSetReconvertString)(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize, NULL,0);
	}
	//領域解放
	::HeapFree(GetProcessHeap(),0,(LPVOID)pReconv);
	::ImmReleaseContext( m_hWnd, hIMC);
}

/*!	コントロールコードの入力(ダイアログ)
	@author	MIK
	@date	2002/06/02
*/
void CEditView::Command_CtrlCode_Dialog( void )
{
	CDlgCtrlCode	cDlgCtrlCode;

	//コントロールコード入力ダイアログを表示する
	if( FALSE == cDlgCtrlCode.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc ) )
	{
		return;
	}

	//コントロールコードを入力する
	Command_CHAR( cDlgCtrlCode.m_nCode );

	return;
}

/*!	検索開始位置へ戻る
	@author	ai
	@date	02/06/26
*/
void CEditView::Command_JUMP_SRCHSTARTPOS(void)
{
	if( 0 <= m_nSrchStartPosX_PHY && 0 <= m_nSrchStartPosY_PHY )
	{
		int x, y;
		/* 範囲選択中か */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
			m_nSrchStartPosX_PHY,
			m_nSrchStartPosY_PHY,
			&x, &y );
		//	2006.07.09 genta 選択状態を保つ
		MoveCursorSelecting( x, y, m_bSelectingLock );
	}
	else
	{
//		SendStatusMessage( "検索開始位置がありません" );
		::MessageBeep( MB_ICONHAND );
	}
	return;
}

/*!	お気に入りの設定(ダイアログ)
	@author	MIK
	@date	2003/04/07
*/
void CEditView::Command_Favorite( void )
{
	CDlgFavorite	cDlgFavorite;

	//ダイアログを表示する
	if( FALSE == cDlgFavorite.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc ) )
	{
		return;
	}

	return;
}

/*! 入力する改行コードを設定

	@author moca
	@date 2003.06.23 新規作成
*/
void CEditView::Command_CHGMOD_EOL( enumEOLType e ){
	if( EOL_NONE < e && e < EOL_CODEMAX  ){
		m_pcEditDoc->SetNewLineCode( e );
		// ステータスバーを更新するため
		// キャレットの行桁位置を表示する関数を呼び出す
		DrawCaretPosInfo();
	}
}

/*! 常に手前に表示
	@date 2004.09.21 Moca
*/
void CEditView::Command_WINTOPMOST( LPARAM lparam )
{
	m_pcEditDoc->m_pcEditWnd->WindowTopMost( int(lparam) );
}

/*!	@brief 引用符の設定
	@date Jan. 29, 2005 genta 新規作成
*/
void CEditView::Command_SET_QUOTESTRING( const char* quotestr )
{
	if( quotestr == NULL )
		return;

	strncpy( m_pShareData->m_Common.m_szInyouKigou, quotestr,
		sizeof( m_pShareData->m_Common.m_szInyouKigou ));
	
	m_pShareData->m_Common.m_szInyouKigou[ sizeof( m_pShareData->m_Common.m_szInyouKigou ) - 1 ] = '\0';
}

/*!	@brief ウィンドウ一覧ポップアップ表示処理（ファイル名のみ）
	@date  2006.03.23 fon 新規作成
	@date  2006.05.19 genta コマンド実行要因を表す引数追加
*/
void CEditView::Command_WINLIST( int nCommandFrom )
{
	CEditWnd	*pCEditWnd;
	pCEditWnd = m_pcEditDoc->m_pcEditWnd;

	//ウィンドウ一覧をポップアップ表示する
	pCEditWnd->PopupWinList( nCommandFrom != 1 );	// 2007.02.27 ryoji アクセラレータキーからでなければマウス位置に

}


/*!	@brief マクロ用アウトプットウインドウに表示
	@date 2006.04.26 maru 新規作成
*/
void CEditView::Command_TRACEOUT( const char* outputstr, int nFlgOpt )
{
	if( outputstr == NULL )
		return;

	// 0x01 ExpandParameterによる文字列展開有無
	if (nFlgOpt & 0x01) {
		char Buffer[2048];
		m_pcEditDoc->ExpandParameter(outputstr, Buffer, 2047);
		CShareData::getInstance()->TraceOut( "%s", Buffer );
	} else {
		CShareData::getInstance()->TraceOut( "%s", outputstr );
	}

	// 0x02 改行コードの有無
	if ((nFlgOpt & 0x02) == 0) CShareData::getInstance()->TraceOut( "\r\n" );

}

/*[EOF]*/
