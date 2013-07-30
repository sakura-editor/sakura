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
	Copyright (C) 2006, genta, Moca, fon, maru
	Copyright (C) 2007, ryoji, maru, genta
	Copyright (C) 2008, nasukoji, ryoji, genta
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <algorithm>		// 2001.12.11 hor    for VC++
#include <string>///
#include <vector> ///	2002/2/3 aroka
#include "CEditView.h"
#include "CWaitCursor.h"
#include "charcode.h"
#include "CRunningTimer.h"
#include "COpe.h" ///	2002/2/3 aroka from here
#include "COpeBlk.h" ///
#include "CLayout.h"///
#include "CDocLine.h"///
#include "mymessage.h"///
#include "Debug.h"///
#include "etc_uty.h"///
#include "COsVersionInfo.h"   // 2002.04.09 minfu 
#include "CEditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理
#include "CDlgCtrlCode.h"	//コントロールコードの入力(ダイアログ)
#include "CDlgFavorite.h"	//履歴の管理	//@@@ 2003.04.08 MIK
#include "CEditWnd.h"
#include "CFileLoad.h"	// 2006.12.09 maru
#include "CDlgCancel.h"	// 2006.12.09 maru
#include "CFileWrite.h"	// 2006.12.09 maru

using namespace std; // 2002/2/3 aroka

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
	bool		bRedraw
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
	int			nLineAllColLen;
	CMemory		cMem;
	int			i;
	const CLayout*	pcLayout;
	bool			bHintPrev = false;	// 更新が前行からになる可能性があることを示唆する
	bool			bHintNext = false;	// 更新が次行からになる可能性があることを示唆する
	bool			bKinsoku;			// 禁則の有無

	*pnNewLine = 0;			/* 挿入された部分の次の位置の行 */
	*pnNewPos = 0;			/* 挿入された部分の次の位置のデータ位置 */

	/* テキストが選択されているか */
	if( IsTextSelected() ){
		DeleteData( bRedraw );
		nX = m_ptCaretPos.x;
		nY = m_ptCaretPos.y;
	}

	//禁則がある場合は1行前から再描画を行う	@@@ 2002.04.19 MIK
	bKinsoku = ( m_pcEditDoc->GetDocumentAttribute().m_bWordWrap
			 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuHead	//@@@ 2002.04.19 MIK
			 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuTail	//@@@ 2002.04.19 MIK
			 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet	//@@@ 2002.04.19 MIK
			 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto );	//@@@ 2002.04.19 MIK

	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nY, &nLineLen, &pcLayout );
	bool bLineModifiedChange = (pLine)? !pcLayout->m_pCDocLine->IsModifyed(): true;

	nIdxFrom = 0;
	cMem.SetString( "" );
	if( pLine ){
		// 更新が前行からになる可能性を調べる	// 2009.02.17 ryoji
		// ※折り返し行頭への句読点入力で前の行だけが更新される場合もある
		// ※挿入位置は行途中でも句読点入力＋ワードラップで前の文字列から続けて前行に回り込む場合もある
		if (pcLayout->m_ptLogicPos.x && bKinsoku){	// 折り返しレイアウト行か？
			bHintPrev = true;	// 更新が前行からになる可能性がある
		}

		// 更新が次行からになる可能性を調べる	// 2009.02.17 ryoji
		// ※折り返し行末への文字入力や文字列貼り付けで現在行は更新されず次行以後が更新される場合もある
		// 指定された桁に対応する行のデータ内の位置を調べる
		nIdxFrom = LineColmnToIndex2( pcLayout, nX, nLineAllColLen );
		// 行終端より右に挿入しようとした
		if( nLineAllColLen > 0 ){
			// 終端直前から挿入位置まで空白を埋める為の処理
			// 行終端が何らかの改行コードか?
			if( EOL_NONE != pcLayout->m_cEol ){
				nIdxFrom = nLineLen - 1;
				for( i = 0; i < nX - nLineAllColLen + 1; ++i ){
					cMem += ' ';
				}
				cMem.AppendString( pData, nDataLen );
			}
			else{
				nIdxFrom = nLineLen;
				for( i = 0; i < nX - nLineAllColLen; ++i ){
					cMem += ' ';
				}
				cMem.AppendString( pData, nDataLen );
				bHintNext = true;	// 更新が次行からになる可能性がある
			}
		}
		else{
			cMem.AppendString( pData, nDataLen );
		}
	}
	else{
		// 更新が前行からになる可能性を調べる	// 2009.02.17 ryoji
		const CLayout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.GetBottomLayout();
		if( pcLayoutWk && pcLayoutWk->m_cEol == EOL_NONE && bKinsoku ){	// 折り返しレイアウト行か？（前行の終端で調査）
			bHintPrev = true;	// 更新が前行からになる可能性がある
		}

		nLineAllColLen = nX;
		for( i = 0; i < nX - nIdxFrom; ++i ){
			cMem += ' ';
		}
		cMem.AppendString( pData, nDataLen );
	}


	if( !m_bDoing_UndoRedo && pcOpe ){	// アンドゥ・リドゥの実行中か
		if( pLine ){
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				LineIndexToColmn( pcLayout, nIdxFrom ),
				nY,
				&pcOpe->m_ptCaretPos_PHY_Before.x,
				&pcOpe->m_ptCaretPos_PHY_Before.y
			);
		}
		else{
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				0,
				nY,
				&pcOpe->m_ptCaretPos_PHY_Before.x,
				&pcOpe->m_ptCaretPos_PHY_Before.y
			);
		}
	}


	// 文字列挿入
	m_pcEditDoc->m_cLayoutMgr.InsertData_CLayoutMgr(
		nY,
		nIdxFrom,
		cMem.GetStringPtr(),
		cMem.GetStringLength(),
		&nModifyLayoutLinesOld,
		&nInsLineNum,
		pnNewLine,			/* 挿入された部分の次の位置の行 */
		pnNewPos			/* 挿入された部分の次の位置のデータ位置 */
	);

	// メモリが再確保されてアドレスが無効になるので、再度、行データを求める
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nY, &nLineLen );

	// 指定された行のデータ内の位置に対応する桁の位置を調べる
	pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( *pnNewLine, &nLineLen2, &pcLayout );
	if( pLine2 ){
		*pnNewPos = LineIndexToColmn( pcLayout, *pnNewPos );
	}

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if( *pnNewPos >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){
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

	// 状態遷移
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	// 再描画
	// 行番号表示に必要な幅を設定
	if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
		// キャレットの表示・更新
		ShowEditCaret();
	}
	else{

		if( bRedraw ){
			int nStartLine = nY;
			if( 0 < nInsLineNum ){
				// スクロールバーの状態を更新する
				AdjustScrollBars();

				// 描画開始行位置を調整する	// 2009.02.17 ryoji
				if( bHintPrev ){	// 更新が前行からになる可能性がある
					nStartLine--;
				}

				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nStartLine - m_nViewTopLine);
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			}
			else{
				if( nModifyLayoutLinesOld < 1 ){
					nModifyLayoutLinesOld = 1;
				}

				// 描画開始行位置と描画行数を調整する	// 2009.02.17 ryoji
				if( bHintPrev ){	// 更新が前行からになる可能性がある
					nStartLine--;
					nModifyLayoutLinesOld++;
				}
				if( bHintNext ){	// 更新が次行からになる可能性がある
					nModifyLayoutLinesOld++;
				}

	//			ps.rcPaint.left = m_nViewAlignLeft;
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;

				// 2002.02.25 Mod By KK 次行 (nY - m_nViewTopLine - 1); => (nY - m_nViewTopLine);
				//ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nY - m_nViewTopLine - 1);
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nStartLine - m_nViewTopLine);
				ps.rcPaint.bottom = ps.rcPaint.top + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nModifyLayoutLinesOld);

				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}
				if( m_nViewAlignTop + m_nViewCy < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				}
			}
			HDC hdc = ::GetDC( m_hWnd );
			OnPaint( hdc, &ps, FALSE );
			::ReleaseDC( m_hWnd, hdc );

			// 行番号（変更行）表示は改行単位の行頭から更新する必要がある	// 2009.03.26 ryoji
			if( bLineModifiedChange ){	// 無変更だった行が変更された
				const CLayout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nStartLine );
				if( pcLayoutWk && pcLayoutWk->m_ptLogicPos.x ){	// 折り返しレイアウト行か？
					RedrawLineNumber();
				}
			}
		}
	}

	if( !m_bDoing_UndoRedo && pcOpe ){	/* アンドゥ・リドゥの実行中か */
		pcOpe->m_nOpe = OPE_INSERT;				/* 操作種別 */
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			*pnNewPos,
			*pnNewLine,
			&pcOpe->m_ptCaretPos_PHY_To.x,
			&pcOpe->m_ptCaretPos_PHY_To.y
		);

		pcOpe->m_nDataLen = cMem.GetStringLength();	/* 操作に関連するデータのサイズ */
		pcOpe->m_pcmemData = NULL;				/* 操作に関連するデータ */
	}

#ifdef _DEBUG
	gm_ProfileOutput = 0;
#endif
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
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			LineIndexToColmn( pcLayout, nIdxFrom + nDelLen ),
			nCaretY,
			&pcOpe->m_ptCaretPos_PHY_To.x,
			&pcOpe->m_ptCaretPos_PHY_To.y
		);
	}

	/* データ削除 */
	m_pcEditDoc->m_cLayoutMgr.DeleteData_CLayoutMgr(
		nCaretY, nIdxFrom, nDelLen,
		&nModifyLayoutLinesOld,
		&nModifyLayoutLinesNew,
		&nDeleteLayoutLines,
		pcMem
	);

	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* アンドゥ・リドゥの実行中か */
		pcOpe->m_nDataLen = pcMem->GetStringLength();	/* 操作に関連するデータのサイズ */
		pcOpe->m_pcmemData = pcMem;				/* 操作に関連するデータ */
	}

	/* 選択エリアの先頭へカーソルを移動 */
	MoveCursor( nCaretX, nCaretY, false );
	m_nCaretPosX_Prev = m_ptCaretPos.x;


end_of_func:;
#ifdef _DEBUG
	gm_ProfileOutput = 0;
#endif
}





/*!	カーソル位置または選択エリアを削除

	@date 2002/03/24 YAZAKI bUndo削除
*/
void CEditView::DeleteData(
	bool	bRedraw
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

	// テキストの存在しないエリアの削除は、選択範囲のキャンセルとカーソル移動のみとする	// 2008.08.05 ryoji
	if( IsTextSelected() ){		// テキストが選択されているか
		if( IsEmptyArea( m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y, m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y, TRUE, m_bBeginBoxSelect ) ){
			// カーソルを選択範囲の左上に移動
			MoveCursor( ( m_sSelect.m_ptFrom.x < m_sSelect.m_ptTo.x ) ? m_sSelect.m_ptFrom.x : m_sSelect.m_ptTo.x,
						( m_sSelect.m_ptFrom.y < m_sSelect.m_ptTo.y ) ? m_sSelect.m_ptFrom.y : m_sSelect.m_ptTo.y, bRedraw );
			m_nCaretPosX_Prev = m_ptCaretPos.x;
			DisableSelectArea( bRedraw );
			return;
		}
	}else{
		if( IsEmptyArea( m_ptCaretPos.x, m_ptCaretPos.y ) ){
			return;
		}
	}

	nCaretPosXOld = m_ptCaretPos.x;
	nCaretPosYOld = m_ptCaretPos.y;

	/* テキストが選択されているか */
	if( IsTextSelected() ){
		CWaitCursor cWaitCursor( m_hWnd );  // 2002.02.05 hor
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
			pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作前のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作前のキャレット位置Ｙ */

			pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}

		/* 矩形範囲選択中か */
		if( m_bBeginBoxSelect ){
			m_pcEditDoc->SetModified(true,bRedraw);	//	2002/06/04 YAZAKI 矩形選択を削除したときに変更マークがつかない。

			m_bDrawSWITCH = false;	// 2002.01.25 hor
			nSelectColmFrom_Old = m_sSelect.m_ptFrom.x;
			nSelectLineFrom_Old = m_sSelect.m_ptFrom.y;

			/* 選択範囲のデータを取得 */
			/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
			/* ２点を対角とする矩形を求める */
			TwoPointToRect(
				&rcSel,
				m_sSelect.m_ptFrom.y,		// 範囲選択開始行
				m_sSelect.m_ptFrom.x,		// 範囲選択開始桁
				m_sSelect.m_ptTo.y,		// 範囲選択終了行
				m_sSelect.m_ptTo.x			// 範囲選択終了桁
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
						m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
							LineIndexToColmn( pcLayout, nDelPos ),
							nLineNum + 1,
							&pcOpe->m_ptCaretPos_PHY_Before.x,
							&pcOpe->m_ptCaretPos_PHY_Before.y
						);

					}else{
						pcOpe = NULL;
					}
					pcMemDeleted = new CMemory;
					// 指定位置の指定長データ削除
					DeleteData2(
						rcSel.left,
						nLineNum + 1,
						nDelLen,
						pcMemDeleted,
						pcOpe				/* 編集操作要素 COpe */
					);

					if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
						m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
							rcSel.left,
							nLineNum + 1,
							&pcOpe->m_ptCaretPos_PHY_After.x,
							&pcOpe->m_ptCaretPos_PHY_After.y
						);
						/* 操作の追加 */
						m_pcOpeBlk->AppendOpe( pcOpe );
					}else{
						delete pcMemDeleted;
						pcMemDeleted = NULL;
					}
				}
			}
			m_bDrawSWITCH = true;	// 2002.01.25 hor

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
				ps.rcPaint.top = m_nViewAlignTop;
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				OnPaint( hdc, &ps, FALSE );
				::ReleaseDC( m_hWnd, hdc );
			}
			/* 選択エリアの先頭へカーソルを移動 */
			::UpdateWindow( m_hWnd );
			MoveCursor( nSelectColmFrom_Old, nSelectLineFrom_Old, bRedraw );
			m_nCaretPosX_Prev = m_ptCaretPos.x;
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe = new COpe;
				pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
					nCaretPosXOld,
					nCaretPosYOld,
					&pcOpe->m_ptCaretPos_PHY_Before.x,
					&pcOpe->m_ptCaretPos_PHY_Before.y
				);

				pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
				pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
				/* 操作の追加 */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}
		}else{
			/* データ置換 削除&挿入にも使える */
			ReplaceData_CEditView(
				m_sSelect.m_ptFrom.y,		// 範囲選択開始行
				m_sSelect.m_ptFrom.x,		// 範囲選択開始桁
				m_sSelect.m_ptTo.y,		// 範囲選択終了行
				m_sSelect.m_ptTo.x,		// 範囲選択終了桁
				NULL,					// 削除されたデータのコピー(NULL可能)
				"",						// 挿入するデータ
				0,						// 挿入するデータの長さ
				bRedraw
			);
		}
	}else{
		/* 現在行のデータを取得 */
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout );
		if( NULL == pLine ){
			goto end_of_func;
//			return;
		}
		/* 最後の行にカーソルがあるかどうか */
		if( m_ptCaretPos.y == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
			bLastLine = 1;
		}else{
			bLastLine = 0;
		}

		/* 指定された桁に対応する行のデータ内の位置を調べる */
		nCurIdx = LineColmnToIndex( pcLayout, m_ptCaretPos.x );
//		MYTRACE(_T("nLineLen=%d nCurIdx=%d \n"), nLineLen, nCurIdx);
		if( nCurIdx == nLineLen && bLastLine ){	/* 全テキストの最後 */
			goto end_of_func;
//			return;
		}

		/* 指定された桁の文字のバイト数を調べる */
		if( pLine[nCurIdx] == '\r' || pLine[nCurIdx] == '\n' ){
			/* 改行 */
			nNxtIdx = nCurIdx + pcLayout->m_cEol.GetLen();
			nNxtPos = m_ptCaretPos.x + pcLayout->m_cEol.GetLen();
		}
		else{
			nNxtIdx = CMemory::MemCharNext( pLine, nLineLen, &pLine[nCurIdx] ) - pLine;
			// 指定された行のデータ内の位置に対応する桁の位置を調べる
			nNxtPos = LineIndexToColmn( pcLayout, nNxtIdx );
		}


		/* データ置換 削除&挿入にも使える */
		ReplaceData_CEditView(
			m_ptCaretPos.y,		// 範囲選択開始行
			m_ptCaretPos.x,		// 範囲選択開始桁
			m_ptCaretPos.y,		// 範囲選択終了行
			nNxtPos,			// 範囲選択終了桁
			NULL,				// 削除されたデータのコピー(NULL可能)
			"",					// 挿入するデータ
			0,					// 挿入するデータの長さ
			bRedraw
		);
	}

	m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() > 0 ){
		if( m_ptCaretPos.y > m_pcEditDoc->m_cLayoutMgr.GetLineCount()	- 1	){
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
			Command_GOFILEEND( false );
		}
	}
end_of_func:;

	return;
}











/* Undo 元に戻す */
void CEditView::Command_UNDO( void )
{
	if( m_bBeginSelect ){	/* マウスによる範囲選択中 */
		ErrorBeep();
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
	bool		bIsModified;
//	int			nNewLine;	/* 挿入された部分の次の位置の行 */
//	int			nNewPos;	/* 挿入された部分の次の位置のデータ位置 */
	HDC			hdc;
	PAINTSTRUCT	ps;
	CWaitCursor cWaitCursor( m_hWnd );

	int			nCaretPosX_Before;
	int			nCaretPosY_Before;
	int			nCaretPosX_After;
	int			nCaretPosY_After;

	/* 各種モードの取り消し */
	Command_CANCEL_MODE();

	m_bDoing_UndoRedo = TRUE;	/* アンドゥ・リドゥの実行中か */

	/* 現在のUndo対象の操作ブロックを返す */
	if( NULL != ( pcOpeBlk = m_pcEditDoc->m_cOpeBuf.DoUndo( &bIsModified ) ) ){
		m_bDrawSWITCH = false;	//	hor
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = nOpeBlkNum - 1; i >= 0; i-- ){
			pcOpe = pcOpeBlk->GetOpe( i );
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_After.x,
				pcOpe->m_ptCaretPos_PHY_After.y,
				&nCaretPosX_After,
				&nCaretPosY_After
			);
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before.x,
				pcOpe->m_ptCaretPos_PHY_Before.y,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);


			/* カーソルを移動 */
			MoveCursor( nCaretPosX_After, nCaretPosY_After, false );

			switch( pcOpe->m_nOpe ){
			case OPE_INSERT:
				{
					CMemory* pcMem = new CMemory;

					/* 選択範囲の変更 */
					m_sSelectBgn.m_ptFrom.y = nCaretPosY_Before;	// 範囲選択開始行(原点)
					m_sSelectBgn.m_ptFrom.x = nCaretPosX_Before;	// 範囲選択開始桁(原点)
					m_sSelectBgn.m_ptTo.y = m_sSelectBgn.m_ptFrom.y;	// 範囲選択開始行(原点)
					m_sSelectBgn.m_ptTo.x = m_sSelectBgn.m_ptFrom.x;	// 範囲選択開始桁(原点)
					m_sSelect.m_ptFrom.y = nCaretPosY_Before;
					m_sSelect.m_ptFrom.x = nCaretPosX_Before;
					m_sSelect.m_ptTo.y = nCaretPosY_After;
					m_sSelect.m_ptTo.x = nCaretPosX_After;

					/* データ置換 削除&挿入にも使える */
					ReplaceData_CEditView(
						m_sSelect.m_ptFrom.y,		// 範囲選択開始行
						m_sSelect.m_ptFrom.x,		// 範囲選択開始桁
						m_sSelect.m_ptTo.y,		// 範囲選択終了行
						m_sSelect.m_ptTo.x,		// 範囲選択終了桁
						pcMem,					// 削除されたデータのコピー(NULL可能)
						"",						// 挿入するデータ
						0,						// 挿入するデータの長さ
						false					// 再描画するか否か
					);

					/* 選択範囲の変更 */
					m_sSelectBgn.m_ptFrom.y = -1;	// 範囲選択開始行(原点)
					m_sSelectBgn.m_ptFrom.x = -1;	// 範囲選択開始桁(原点)
					m_sSelectBgn.m_ptTo.y = -1;	// 範囲選択開始行(原点)
					m_sSelectBgn.m_ptTo.x = -1;	// 範囲選択開始桁(原点)
					m_sSelect.m_ptFrom.y = -1;
					m_sSelect.m_ptFrom.x = -1;
					m_sSelect.m_ptTo.y = -1;
					m_sSelect.m_ptTo.x = -1;

					pcOpe->m_pcmemData = pcMem;
				}
				break;
			case OPE_DELETE:
				// 2010.08.25 メモリーリーク修正
				if( 0 < pcOpe->m_pcmemData->GetStringLength() ){
					/* データ置換 削除&挿入にも使える */
					ReplaceData_CEditView(
						nCaretPosY_Before,					// 範囲選択開始行
						nCaretPosX_Before,					// 範囲選択開始桁
						nCaretPosY_Before,					// 範囲選択終了行
						nCaretPosX_Before,					// 範囲選択終了桁
						NULL,								// 削除されたデータのコピー(NULL可能)
						pcOpe->m_pcmemData->GetStringPtr(),	// 挿入するデータ
						pcOpe->m_nDataLen,					// 挿入するデータの長さ
						false								// 再描画するか否か
					);

				}
				delete pcOpe->m_pcmemData;
				pcOpe->m_pcmemData = NULL;
				break;
			case OPE_MOVECARET:
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, false/*true 2002.02.16 hor */ );
				break;
			}

			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before.x,
				pcOpe->m_ptCaretPos_PHY_Before.y,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);
			if( i == 0 ){
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, true );
			}else{
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, false );
			}
		}
		m_bDrawSWITCH = true;	//	hor
		AdjustScrollBars(); // 2007.07.22 ryoji

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
		OnPaint( hdc, &ps, FALSE );
		DispRuler( hdc );
		::ReleaseDC( m_hWnd, hdc );
		/* 行番号表示に必要な幅を設定 */
		if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
			/* キャレットの表示・更新 */
			ShowEditCaret();
		}
		DrawCaretPosInfo();	// キャレットの行桁位置を表示する	// 2007.10.19 ryoji

		if( !m_pcEditDoc->UpdateTextWrap() )	// 折り返し方法関連の更新	// 2008.06.10 ryoji
			m_pcEditDoc->RedrawAllViews( this );	//	他のペインの表示を更新

	}

	m_nCaretPosX_Prev = m_ptCaretPos.x;	// 2007.10.11 ryoji 追加
	m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */

	return;
}





/* Redo やり直し */
void CEditView::Command_REDO( void )
{
	if( m_bBeginSelect ){	/* マウスによる範囲選択中 */
		ErrorBeep();
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
//	int			nNewLine;	/* 挿入された部分の次の位置の行 */
//	int			nNewPos;	/* 挿入された部分の次の位置のデータ位置 */
	bool		bIsModified;
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
		m_bDrawSWITCH = false;	// 2007.07.22 ryoji
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = 0; i < nOpeBlkNum; ++i ){
			pcOpe = pcOpeBlk->GetOpe( i );
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before.x,
				pcOpe->m_ptCaretPos_PHY_Before.y,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);

			if( i == 0 ){
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, true );
			}else{
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, false );
			}
			switch( pcOpe->m_nOpe ){
			case OPE_INSERT:
				// 2010.08.25 メモリーリークの修正
				if( 0 < pcOpe->m_pcmemData->GetStringLength() ){
					/* データ置換 削除&挿入にも使える */
					ReplaceData_CEditView(
						nCaretPosY_Before,					// 範囲選択開始行
						nCaretPosX_Before,					// 範囲選択開始桁
						nCaretPosY_Before,					// 範囲選択終了行
						nCaretPosX_Before,					// 範囲選択終了桁
						NULL,								// 削除されたデータのコピー(NULL可能)
						pcOpe->m_pcmemData->GetStringPtr(),	// 挿入するデータ
						pcOpe->m_pcmemData->GetStringLength(),	// 挿入するデータの長さ
						false								//再描画するか否か
					);
				}
				delete pcOpe->m_pcmemData;
				pcOpe->m_pcmemData = NULL;
				break;
			case OPE_DELETE:
				{
				CMemory* pcMem = new CMemory;

				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
					pcOpe->m_ptCaretPos_PHY_To.x,
					pcOpe->m_ptCaretPos_PHY_To.y,
					&nCaretPosX_To,
					&nCaretPosY_To
				);

				/* データ置換 削除&挿入にも使える */
				ReplaceData_CEditView(
					nCaretPosY_Before,	// 範囲選択開始行
					nCaretPosX_Before,	// 範囲選択開始桁
					nCaretPosY_To,		// 範囲選択終了行
					nCaretPosX_To,		// 範囲選択終了桁
					pcMem,				// 削除されたデータのコピー(NULL可能)
					"",					// 挿入するデータ
					0,					// 挿入するデータの長さ
					false
				);

				pcOpe->m_pcmemData = pcMem;
				}
				break;
			case OPE_MOVECARET:
				break;
			}
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_After.x,
				pcOpe->m_ptCaretPos_PHY_After.y,
				&nCaretPosX_After,
				&nCaretPosY_After
			);

			if( i == nOpeBlkNum - 1	){
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, true );
			}else{
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, false );
			}
		}
		m_bDrawSWITCH = true; // 2007.07.22 ryoji
		AdjustScrollBars(); // 2007.07.22 ryoji

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
		OnPaint( hdc, &ps, FALSE );
		DispRuler( hdc );	// 2007.10.19 ryoji
		::ReleaseDC( m_hWnd, hdc );

		/* 行番号表示に必要な幅を設定 */
		if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
			/* キャレットの表示・更新 */
			ShowEditCaret();
		}
		DrawCaretPosInfo();	// キャレットの行桁位置を表示する	// 2007.10.19 ryoji

		if( !m_pcEditDoc->UpdateTextWrap() )	// 折り返し方法関連の更新	// 2008.06.10 ryoji
			m_pcEditDoc->RedrawAllViews( this );	//	他のペインの表示を更新
	}

	m_nCaretPosX_Prev = m_ptCaretPos.x;	// 2007.10.11 ryoji 追加
	m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */
}




/* データ置換 削除&挿入にも使える */
void CEditView::ReplaceData_CEditView(
	int			nDelLineFrom,			// 削除範囲行  From レイアウト行番号
	int			nDelColmFrom,			// 削除範囲位置From レイアウト行桁位置
	int			nDelLineTo,				// 削除範囲行  To   レイアウト行番号
	int			nDelColmTo,				// 削除範囲位置To   レイアウト行桁位置
	CMemory*	pcmemCopyOfDeleted,		// 削除されたデータのコピー(NULL可能)
	const char*	pInsData,				// 挿入するデータ
	int			nInsDataLen,			// 挿入するデータの長さ
	bool		bRedraw
)
{
	bool bLineModifiedChange;

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
		bLineModifiedChange = (line)? !pcLayout->m_pCDocLine->IsModifyed(): true;
		//	Jun. 1, 2000 genta
		//	ちゃんとNULLチェックしましょう
		if( line ){
			pos = LineColmnToIndex( pcLayout, nDelColmFrom );
			//	Jun. 1, 2000 genta
			//	同一行の行末以降のみが選択されている場合を考慮する

			//	Aug. 22, 2000 genta
			//	開始位置がEOFの後ろのときは次行に送る処理を行わない
			//	これをやってしまうと存在しない行をPointして落ちる．
			if( nDelLineFrom < m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 && pos >= len){
				if( nDelLineFrom == nDelLineTo  ){
					//	m_sSelect.m_ptFrom.y <= m_sSelect.m_ptTo.y はチェックしない
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
		if( line ){
			pos = LineIndexToColmn( pcLayout, len );

			if( nDelColmTo > pos ){
				nDelColmTo = pos;
			}
		}
		//	To Here
	}

	int nDelLineFrom_PHY;
	int nDelColmFrom_PHY;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(	// 2009.07.18 ryoji PHYで記憶する
		nDelColmFrom,
		nDelLineFrom,
		&nDelColmFrom_PHY,
		&nDelLineFrom_PHY
	);

	COpe* pcOpe = NULL;		/* 編集操作要素 COpe */
	CMemory* pcMemDeleted;
	int	nCaretPosXOld;
	int	nCaretPosYOld;
	int	nCaretPosX_PHY_Old;
	int	nCaretPosY_PHY_Old;

	nCaretPosXOld = m_ptCaretPos.x;
	nCaretPosYOld = m_ptCaretPos.y;
	nCaretPosX_PHY_Old = m_ptCaretPos_PHY.x;
	nCaretPosY_PHY_Old = m_ptCaretPos_PHY.y;
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;						/* 操作種別 */
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作前のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作前のキャレット位置Ｙ */


		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_DELETE;				/* 操作種別 */

		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			nDelColmFrom/*m_ptCaretPos.x*/,
			nDelLineFrom/*m_ptCaretPos.y*/,
			&pcOpe->m_ptCaretPos_PHY_Before.x,
			&pcOpe->m_ptCaretPos_PHY_Before.y
		);

		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			nDelColmTo,
			nDelLineTo,
			&pcOpe->m_ptCaretPos_PHY_To.x,
			&pcOpe->m_ptCaretPos_PHY_To.y
		);



		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
	}else{
		pcOpe = NULL;
	}
	pcMemDeleted = new CMemory;

	// Feb. 08, 2008 genta 削除バッファの確保はCDocLineMgr::ReplaceDataで行うので削除

	/* 現在の選択範囲を非選択状態に戻す */
	// 2009.07.18 ryoji 置換後→置換前に位置を変更（置換後だと反転が不正になって汚い Wiki BugReport/43）
	DisableSelectArea( bRedraw );

	/* 文字列置換 */
	LayoutReplaceArg	LRArg;
	LRArg.sDelRange.m_ptFrom.y = nDelLineFrom;	// 削除範囲行  From レイアウト行番号
	LRArg.sDelRange.m_ptFrom.x = nDelColmFrom;	// 削除範囲位置From レイアウト行桁位置
	LRArg.sDelRange.m_ptTo.y   = nDelLineTo;	// 削除範囲行  To   レイアウト行番号
	LRArg.sDelRange.m_ptTo.x   = nDelColmTo;	// 削除範囲位置To   レイアウト行桁位置
	LRArg.pcmemDeleted = pcMemDeleted;	// 削除されたデータ
	LRArg.pInsData = pInsData;			// 挿入するデータ
	LRArg.nInsDataLen = nInsDataLen;	// 挿入するデータの長さ
	m_pcEditDoc->m_cLayoutMgr.ReplaceData_CLayoutMgr(
		&LRArg
	);

	//	Jan. 30, 2001 genta
	//	再描画の時点でファイル更新フラグが適切になっていないといけないので
	//	関数の末尾からここへ移動
	/* 状態遷移 */
	if( FALSE == m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	/* 行番号表示に必要な幅を設定 */
	if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
		/* キャレットの表示・更新 */
		ShowEditCaret();
	}
	else{

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
				//ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (m_ptCaretPos.y - m_nViewTopLine); // 2002.02.25 Del By KK 次で上書きされているため未使用。
				//ps.rcPaint.top = 0/*m_nViewAlignTop*/;			// 2002.02.25 Del By KK
				ps.rcPaint.top = m_nViewAlignTop - m_nTopYohaku;	// ルーラーを範囲に含めない。2002.02.25 Add By KK
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			}
			else{
				// 文書末が改行なし→ありに変化したら				// 2009.11.11 ryoji
				// EOFのみ行が追加になるので、1行余分に描画する。
				// （文書末が改行あり→なしに変化する場合の末尾EOF消去は描画関数側で行われる）
				int nAddLine = ( LRArg.nNewLine > LRArg.sDelRange.m_ptTo.y )? 1: 0;

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
				ps.rcPaint.bottom = m_nViewAlignTop + (LRArg.nModLineTo - m_nViewTopLine + 1 + nAddLine)* (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				if( m_nViewAlignTop + m_nViewCy < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				}

			}
			OnPaint( hdc, &ps, FALSE );
			::ReleaseDC( m_hWnd, hdc );

			// 行番号（変更行）表示は改行単位の行頭から更新する必要がある	// 2009.03.26 ryoji
			if( bLineModifiedChange ){	// 無変更だった行が変更された
				const CLayout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( LRArg.nModLineFrom );
				if( pcLayoutWk && pcLayoutWk->m_ptLogicPos.x ){	// 折り返しレイアウト行か？
					RedrawLineNumber();
				}
			}
		}
	}

	// 削除されたデータのコピー(NULL可能)
	if( pcmemCopyOfDeleted && 0 < pcMemDeleted->GetStringLength() ){
		*pcmemCopyOfDeleted = *pcMemDeleted;
	}

	if( !m_bDoing_UndoRedo && 0 < pcMemDeleted->GetStringLength() ){
		pcOpe->m_nDataLen = pcMemDeleted->GetStringLength();	/* 操作に関連するデータのサイズ */
		pcOpe->m_pcmemData = pcMemDeleted;						/* 操作に関連するデータ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}else{
		delete pcMemDeleted;
		pcMemDeleted = NULL;
	}


	if( !m_bDoing_UndoRedo && 0 < nInsDataLen ){
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_INSERT;				/* 操作種別 */

		// 2009.07.18 ryoji レイアウトは変化するのに以前のnDelColmFrom,nDelLineFromからLayoutToLogicで計算していたバグを修正
		pcOpe->m_ptCaretPos_PHY_Before.x = nDelColmFrom_PHY;
		pcOpe->m_ptCaretPos_PHY_Before.y = nDelLineFrom_PHY;
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			LRArg.nNewPos,
			LRArg.nNewLine,
			&pcOpe->m_ptCaretPos_PHY_To.x,
			&pcOpe->m_ptCaretPos_PHY_To.y
		);
		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_To.x;
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_To.y;


		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}else{
		pcOpe = NULL;
	}


	// 挿入直後位置へカーソルを移動
	MoveCursor(
		LRArg.nNewPos,		// 挿入された部分の次の位置のデータ位置(レイアウト桁位置)
		LRArg.nNewLine,		// 挿入された部分の次の位置の行(レイアウト行)
		bRedraw
	);
	m_nCaretPosX_Prev = m_ptCaretPos.x;


	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
		pcOpe->m_ptCaretPos_PHY_Before.x = nCaretPosX_PHY_Old;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = nCaretPosY_PHY_Old;	/* 操作後のキャレット位置Ｙ */

		pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

	//	Jan. 30, 2001 genta
	//	ファイル全体の更新フラグが立っていないと各行の更新状態が表示されないので
	//	フラグ更新処理を再描画より前に移動する
}




/* C/C++スマートインデント処理 */
void CEditView::SmartIndent_CPP( char cChar )
{
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
	CDocLine*	pCDocLine = NULL;
	int			nCharChars;
	int			nSrcLen;
	char		pszSrc[1024];
	BOOL		bChange;

	int			nCaretPosX_PHY;


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

		nCaretPosX_PHY = m_ptCaretPos_PHY.x;

		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( m_ptCaretPos_PHY.y, &nLineLen );
		if( NULL == pLine ){
			if( CR != cChar ){
				return;
			}
			/* 調整によって置換される箇所 */
			nXFm = 0;
			nYFm = m_ptCaretPos_PHY.y;
			nXTo = 0;
			nYTo = m_ptCaretPos_PHY.y;
		}else{
			pCDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y );


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
			nYFm = m_ptCaretPos_PHY.y;
			nXTo = i;
			nYTo = m_ptCaretPos_PHY.y;
		}


		/* 対応する括弧をさがす */
		nLevel = 0;	/* {}の入れ子レベル */


		nDataLen = 0;
		for( j = m_ptCaretPos_PHY.y; j >= 0; --j ){
			pLine2 = m_pcEditDoc->m_cDocLineMgr.GetLineStr( j, &nLineLen2 );
			if( j == m_ptCaretPos_PHY.y ){
				// 2005.10.11 ryoji EOF のみの行もスマートインデントの対象にする
				if( NULL == pLine2 ){
					if( m_ptCaretPos_PHY.y == m_pcEditDoc->m_cDocLineMgr.GetLineCount() )
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
//						MYTRACE( _T("▼[%s]\n"), pLine2 );
					}else{
						//同じ行の場合
						if( j == m_ptCaretPos_PHY.y ){
							if( '{' == cChar && '}' == pLine2[k] ){
								cChar = '}';
								nLevel--;	/* {}の入れ子レベル */
							}
							if( '(' == cChar && ')' == pLine2[k] ){
								cChar = ')';
								nLevel--;	/* {}の入れ子レベル */
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
//						MYTRACE( _T("▼[%s]\n"), pLine2 );
					}else{
						//同じ行の場合
						if( j == m_ptCaretPos_PHY.y ){
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
			if( CR  == cChar || '{' == cChar || '(' == cChar ){
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
		nCPY = m_ptCaretPos_PHY.y;

		nSrcLen = nXTo - nXFm;
		if( nSrcLen >= _countof( pszSrc ) - 1 ){
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
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nXFm, nYFm, &nXFm, &nYFm );
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nXTo, nYTo, &nXTo, &nYTo );

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
				true
			);
		}


		/* カーソル位置調整 */
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nCPX, nCPY, &nCPX, &nCPY );
		/* 選択エリアの先頭へカーソルを移動 */
		MoveCursor( nCPX, nCPY, true );
		m_nCaretPosX_Prev = m_ptCaretPos.x;


		if( bChange && !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
			pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作前のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作前のキャレット位置Ｙ */
			pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		break;
	}
	if( NULL != pszData ){
		delete [] pszData;
		pszData = NULL;
	}
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

	nCaretPosX_PHY = m_ptCaretPos_PHY.x;
	nCaretPosY_PHY = m_ptCaretPos_PHY.y;

	if( m_ptCaretPos_PHY.y > 0 ){
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStrWithoutEOL( m_ptCaretPos_PHY.y - 1, &nLineLen );
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
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( j, m_ptCaretPos_PHY.y - 1, &nXFm, &nYFm );
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nLineLen, m_ptCaretPos_PHY.y - 1, &nXTo, &nYTo );
				if( !( nXFm >= nXTo && nYFm == nYTo) ){
					ReplaceData_CEditView(
						nYFm,		/* 削除範囲行  From レイアウト行番号 */
						nXFm,		/* 削除範囲位置From レイアウト行桁位置 */
						nYTo,		/* 削除範囲行  To   レイアウト行番号 */
						nXTo,		/* 削除範囲位置To   レイアウト行桁位置 */
						NULL,		/* 削除されたデータのコピー(NULL可能) */
						NULL,		/* 挿入するデータ */
						0,			/* 挿入するデータの長さ */
						true
					);
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nCaretPosX_PHY, nCaretPosY_PHY, &nCPX, &nCPY );
					MoveCursor( nCPX, nCPY, true );

					if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
						pcOpe = new COpe;
						pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
						pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作前のキャレット位置Ｘ */
						pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作前のキャレット位置Ｙ */
						pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
						pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
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

	if( m_ptCaretPos.y > m_nViewRowNum + m_nViewTopLine - (nCaretMarginY + 1) ){
		m_cUnderLine.CaretUnderLineOFF( true );
	}

	//	Sep. 11, 2004 genta 同期用に行数を記憶
	//	Sep. 11, 2004 genta 同期スクロールの関数化
	SyncScrollV( ScrollAtV(m_nViewTopLine - 1));

	// テキストが選択されていない
	if( !IsTextSelected() )
	{
		// カーソルが画面外に出た
		if( m_ptCaretPos.y > m_nViewRowNum + m_nViewTopLine - nCaretMarginY )
		{
			if( m_ptCaretPos.y > m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nCaretMarginY )
				Cursor_UPDOWN( (m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nCaretMarginY) - m_ptCaretPos.y, FALSE );
			else
				Cursor_UPDOWN( -1, FALSE);
			DrawCaretPosInfo();
		}
	}

	m_cUnderLine.CaretUnderLineON( true );
}

// テキストを１行上へスクロール
void CEditView::Command_WndScrollUp(void)
{
	int	nCaretMarginY;

	nCaretMarginY = m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	if( m_ptCaretPos.y < m_nViewTopLine + (nCaretMarginY + 1) ){
		m_cUnderLine.CaretUnderLineOFF( true );
	}

	//	Sep. 11, 2004 genta 同期用に行数を記憶
	//	Sep. 11, 2004 genta 同期スクロールの関数化
	SyncScrollV( ScrollAtV( m_nViewTopLine + 1 ));

	// テキストが選択されていない
	if( !IsTextSelected() )
	{
		// カーソルが画面外に出た
		if( m_ptCaretPos.y < m_nViewTopLine + nCaretMarginY )
		{
			if( m_nViewTopLine == 1 )
				Cursor_UPDOWN( nCaretMarginY + 1, FALSE );
			else
				Cursor_UPDOWN( 1, FALSE );
			DrawCaretPosInfo();
		}
	}

	m_cUnderLine.CaretUnderLineON( true );
}

// 2001/06/20 End



/* 次の段落へ進む
	2002/04/26 段落の両端で止まるオプションを追加
	2002/04/19 新規
*/
void CEditView::Command_GONEXTPARAGRAPH( bool bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = 0;
	
	bool nFirstLineIsEmptyLine = false;
	/* まずは、現在位置が空行（スペース、タブ、改行記号のみの行）かどうか判別 */
	if ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer++;
	}
	else {
		// EOF行でした。
		return;
	}

	/* 次に、nFirstLineIsEmptyLineと異なるところまで読み飛ばす */
	while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer++;
		}
		else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLineが空行だったら、今見ているところは非空行。すなわちおしまい。
		nFirstLineIsEmptyLineが非空行だったら、今見ているところは空行。
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	おしまい。
	}
	else {
		//	いま見ているところは空行の1行目
		if ( m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	段落の両端で止まる
		}
		else {
			/* 仕上げに、空行じゃないところまで進む */
			while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					nCaretPointer++;
				}
				else {
					break;
				}
			}
		}
	}

	//	EOFまで来たり、目的の場所まできたので移動終了。

	/* 移動距離を計算 */
	int nCaretPosX_Layo;
	int nCaretPosY_Layo;

	/* 移動前の物理位置 */
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		m_ptCaretPos_PHY.x, m_ptCaretPos_PHY.y,
		&nCaretPosX_Layo, &nCaretPosY_Layo
	);

	/* 移動後の物理位置 */
	int nCaretPosY_Layo_CaretPointer;
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		m_ptCaretPos_PHY.x, m_ptCaretPos_PHY.y + nCaretPointer,
		&nCaretPosX_Layo, &nCaretPosY_Layo_CaretPointer
	);

	Cursor_UPDOWN( nCaretPosY_Layo_CaretPointer - nCaretPosY_Layo, bSelect );
	return;
}

/* 前の段落へ進む
	2002/04/26 段落の両端で止まるオプションを追加
	2002/04/19 新規
*/
void CEditView::Command_GOPREVPARAGRAPH( bool bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = -1;

	bool nFirstLineIsEmptyLine = false;
	/* まずは、現在位置が空行（スペース、タブ、改行記号のみの行）かどうか判別 */
	if ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer--;
	}
	else {
		nFirstLineIsEmptyLine = true;
		nCaretPointer--;
	}

	/* 次に、nFirstLineIsEmptyLineと異なるところまで読み飛ばす */
	while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer--;
		}
		else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLineが空行だったら、今見ているところは非空行。すなわちおしまい。
		nFirstLineIsEmptyLineが非空行だったら、今見ているところは空行。
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	おしまい。
		if ( m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	段落の両端で止まる
			nCaretPointer++;	//	空行の最上行（段落の末端の次の行）で止まる。
		}
		else {
			/* 仕上げに、空行じゃないところまで進む */
			while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					break;
				}
				else {
					nCaretPointer--;
				}
			}
			nCaretPointer++;	//	空行の最上行（段落の末端の次の行）で止まる。
		}
	}
	else {
		//	いま見ているところは空行の1行目
		if ( m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	段落の両端で止まる
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
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		m_ptCaretPos_PHY.x, m_ptCaretPos_PHY.y,
		&nCaretPosX_Layo, &nCaretPosY_Layo
	);

	/* 移動後の物理位置 */
	int nCaretPosY_Layo_CaretPointer;
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		m_ptCaretPos_PHY.x, m_ptCaretPos_PHY.y + nCaretPointer,
		&nCaretPosX_Layo, &nCaretPosY_Layo_CaretPointer
	);

	Cursor_UPDOWN( nCaretPosY_Layo_CaretPointer - nCaretPosY_Layo, bSelect );
}

// From Here 2001.12.03 hor

//! ブックマークの設定・解除を行う(トグル動作)
void CEditView::Command_BOOKMARK_SET(void)
{
	CDocLine*	pCDocLine;
	int			nX=0;
	int			nY;
	int			nYfrom,nYto;
	if( IsTextSelected() && m_sSelect.m_ptFrom.y<m_sSelect.m_ptTo.y ){
		nYfrom=m_sSelect.m_ptFrom.y;
		nYto  =m_sSelect.m_ptTo.y;
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(nX,nYfrom,&nX,&nYfrom);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(nX,nYto,&nX,&nYto);
		for(nY=nYfrom;nY<=nYto;nY++){
			pCDocLine=m_pcEditDoc->m_cDocLineMgr.GetLine( nY );
			if(NULL!=pCDocLine)pCDocLine->SetBookMark(!pCDocLine->IsBookmarked());
		}
	}else{
		pCDocLine=m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y );
		if(NULL!=pCDocLine)pCDocLine->SetBookMark(!pCDocLine->IsBookmarked());
	}
	// 2002.01.16 hor 分割したビューも更新
	m_pcEditDoc->Views_Redraw();
	return;
}



//! 次のブックマークを探し，見つかったら移動する
void CEditView::Command_BOOKMARK_NEXT(void)
{
	int			nX=0;
	int			nY;
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor

	nY=m_ptCaretPos_PHY.y;
	nYOld=nY;						// hor

re_do:;								// hor
	if(m_pcEditDoc->m_cDocLineMgr.SearchBookMark(nY, SEARCH_FORWARD, &nY)){
		bFound = TRUE;				// hor
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(nX,nY,&nX,&nY);
		//	2006.07.09 genta 新規関数にまとめた
		MoveCursorSelecting( nX, nY, m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(m_pShareData->m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&		// 見つからなかった
			bRedo			// 最初の検索
		){
			nY=-1;	//	2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;		// 先頭から再検索
		}
	}
	if(bFound){
		if(nYOld >= nY)SendStatusMessage(_T("▼先頭から再検索しました"));
	}else{
		SendStatusMessage(_T("▽見つかりませんでした"));
		AlertNotFound( m_hWnd, _T("前方(↓) にブックマークが見つかりません。"));
	}
	return;
}



//! 前のブックマークを探し，見つかったら移動する．
void CEditView::Command_BOOKMARK_PREV(void)
{
	int			nX=0;
	int			nY;
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor

	nY=m_ptCaretPos_PHY.y;
	nYOld=nY;						// hor

re_do:;								// hor
	if(m_pcEditDoc->m_cDocLineMgr.SearchBookMark(nY, SEARCH_BACKWARD, &nY)){
		bFound = TRUE;				// hor
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(nX,nY,&nX,&nY);
		//	2006.07.09 genta 新規関数にまとめた
		MoveCursorSelecting( nX, nY, m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(m_pShareData->m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&	// 見つからなかった
			bRedo		// 最初の検索
		){
			nY=m_pcEditDoc->m_cLayoutMgr.GetLineCount();	// 2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;	// 末尾から再検索
		}
	}
	if(bFound){
		if(nYOld <= nY)SendStatusMessage(_T("▲末尾から再検索しました"));
	}else{
		SendStatusMessage(_T("△見つかりませんでした"));
		AlertNotFound( m_hWnd, _T("後方(↑) にブックマークが見つかりません。") );
	}
	return;
}



//! ブックマークをクリアする
void CEditView::Command_BOOKMARK_RESET(void)
{
	m_pcEditDoc->m_cDocLineMgr.ResetAllBookMark();
	// 2002.01.16 hor 分割したビューも更新
	m_pcEditDoc->Views_Redraw();
}


//指定パターンに一致する行をマーク 2002.01.16 hor
//キーマクロで記録できるように	2002.02.08 hor
void CEditView::Command_BOOKMARK_PATTERN( void )
{
	//検索or置換ダイアログから呼び出された
	if( !ChangeCurRegexp(false) ) return;

	m_pcEditDoc->m_cDocLineMgr.MarkSearchWord(
		m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0],		// 検索条件
		m_pShareData->m_Common.m_sSearch.m_sSearchOption,	// 検索条件
		&m_CurRegexp							// 正規表現コンパイルデータ
	);
	// 2002.01.16 hor 分割したビューも更新
	m_pcEditDoc->Views_Redraw();
	return;
}



/*! TRIM Step1
	非選択時はカレント行を選択して ConvSelectedArea → ConvMemory へ
	@author hor
	@date 2001.12.03 hor 新規作成
*/
void CEditView::Command_TRIM(
	BOOL bLeft	//!< [in] FALSE: 右TRIM / それ以外: 左TRIM
)
{
	bool bBeDisableSelectArea = false;
	if(!IsTextSelected()){	//	非選択時は行選択に変更
		m_sSelect.m_ptFrom.y = m_ptCaretPos.y;
		m_sSelect.m_ptFrom.x = 0;
		m_sSelect.m_ptTo.y   = m_ptCaretPos.y; 
		//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
		m_sSelect.m_ptTo.x   = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
		bBeDisableSelectArea = true;
	}

	if(bLeft){
		ConvSelectedArea( F_LTRIM );
	}
	else{
		ConvSelectedArea( F_RTRIM );
	}
	if (bBeDisableSelectArea) DisableSelectArea( true );
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
	CEol		cEol;
	int			nCharChars;

	nBgn = 0;
	nPosDes = 0;
	/* 変換後に必要なバイト数を調べる */
	while( NULL != ( pLine = GetNextLine( pCMemory->GetStringPtr(), pCMemory->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory変更
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
		while( NULL != ( pLine = GetNextLine( pCMemory->GetStringPtr(), pCMemory->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory変更
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
		while( NULL != ( pLine = GetNextLine( pCMemory->GetStringPtr(), pCMemory->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory変更
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

	pCMemory->SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
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
	bool		bBeginBoxSelectOld;
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
		nLFO = m_sSelect.m_ptFrom.y;
		nCFO = m_sSelect.m_ptFrom.x;
		nLTO = m_sSelect.m_ptTo.y;
		nCTO = m_sSelect.m_ptTo.x;
		if( m_sSelect.m_ptFrom.x==m_sSelect.m_ptTo.x ){
			//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
			m_sSelect.m_ptTo.x=m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
		}
		if(m_sSelect.m_ptFrom.x<m_sSelect.m_ptTo.x){
			nCF=m_sSelect.m_ptFrom.x;
			nCT=m_sSelect.m_ptTo.x;
		}else{
			nCF=m_sSelect.m_ptTo.x;
			nCT=m_sSelect.m_ptFrom.x;
		}
	}
	bBeginBoxSelectOld=m_bBeginBoxSelect;
	nCaretPosYOLD=m_ptCaretPos.y;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_sSelect.m_ptFrom.x,m_sSelect.m_ptFrom.y,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_sSelect.m_ptTo.x,m_sSelect.m_ptTo.y,
		&nSelectColToOld,&nSelectLineToOld
	);

	if( bBeginBoxSelectOld ){
		++nSelectLineToOld;
	}
	else{
		// カーソル位置が行頭じゃない ＆ 選択範囲の終端に改行コードがある場合は
		// その行も選択範囲に加える
		if ( nSelectColToOld > 0 ) {
			// 2006.03.31 Moca nSelectLineToOldは、物理行なのでLayout系からDocLine系に修正
			const CDocLine* pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( nSelectLineToOld );
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
		const CDocLine* pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( i );
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( i, &nLineLen );
		if( NULL == pLine ) continue;
		SORTTABLE pst = new SORTDATA;
		if( bBeginBoxSelectOld ){
			nColmFrom = LineColmnToIndex( pcDocLine, nCF );
			nColmTo   = LineColmnToIndex( pcDocLine, nCT );
			if(nColmTo<nLineLen){	// BOX選択範囲の右端が行内に収まっている場合
				// 2006.03.31 genta std::string::assignを使って一時変数削除
				pst->sKey1.assign( &pLine[nColmFrom], nColmTo-nColmFrom );
			}
			else if(nColmFrom<nLineLen){	// BOX選択範囲の右端が行末より右にはみ出している場合
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
	cmemBuf.SetString( "" );
	j=(int)sta.size();
	if( bBeginBoxSelectOld ){
		for (i=0; i<j; i++) cmemBuf.AppendString( sta[i]->sKey2.c_str() ); 
	}else{
		for (i=0; i<j; i++) cmemBuf.AppendString( sta[i]->sKey1.c_str() );
	}

	//sta.clear(); ←これじゃだめみたい
	for (i=0; i<j; i++) delete sta[i];

	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		nSelectColFromOld,nSelectLineFromOld,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		nSelectColToOld,nSelectLineToOld,
		&nSelectColToOld,&nSelectLineToOld
	);
	ReplaceData_CEditView(
		nSelectLineFromOld,
		nSelectColFromOld,
		nSelectLineToOld,
		nSelectColToOld,
		NULL,					/* 削除されたデータのコピー(NULL可能) */
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		false
	);

	//	選択エリアの復元
	if(bBeginBoxSelectOld){
		m_bBeginBoxSelect=bBeginBoxSelectOld;
		m_sSelect.m_ptFrom.y=nLFO;
		m_sSelect.m_ptFrom.x=nCFO;
		m_sSelect.m_ptTo.y  =nLTO;
		m_sSelect.m_ptTo.x  =nCTO;
	}else{
		m_sSelect.m_ptFrom.y=nSelectLineFromOld;
		m_sSelect.m_ptFrom.x=nSelectColFromOld;
		m_sSelect.m_ptTo.y  =nSelectLineToOld;
		m_sSelect.m_ptTo.x  =nSelectColToOld;
	}
	if(nCaretPosYOLD==m_sSelect.m_ptFrom.y || m_bBeginBoxSelect ) {
		MoveCursor( m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y, true );
	}else{
		MoveCursor( m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y, true );
	}
	m_nCaretPosX_Prev = m_ptCaretPos.x;
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* 操作前のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* 操作前のキャレット位置Ｙ */
		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
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

	nCaretPosYOLD=m_ptCaretPos.y;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_sSelect.m_ptFrom.x,m_sSelect.m_ptFrom.y,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_sSelect.m_ptTo.x,m_sSelect.m_ptTo.y,
		&nSelectColToOld,&nSelectLineToOld
	);

	// 2001.12.21 hor
	// カーソル位置が行頭じゃない ＆ 選択範囲の終端に改行コードがある場合は
	// その行も選択範囲に加える
	if ( nSelectColToOld > 0 ) {
		const CLayout* pcLayout=m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY(nSelectLineToOld);
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
	cmemBuf.SetString( "" );
	for( i = nSelectLineFromOld; i < nSelectLineToOld; i++ ){
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( i, &nLineLen );
		if( NULL == pLine ) continue;
		if( NULL == pLinew || strcmp(pLine,pLinew) ){
			cmemBuf.AppendString( pLine );
		}
		pLinew=pLine;
	}
	j=m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		nSelectColFromOld,nSelectLineFromOld,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		nSelectColToOld,nSelectLineToOld,
		&nSelectColToOld,&nSelectLineToOld
	);
	ReplaceData_CEditView(
		nSelectLineFromOld,
		nSelectColFromOld,
		nSelectLineToOld,
		nSelectColToOld,
		NULL,					/* 削除されたデータのコピー(NULL可能) */
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		false
	);
	j-=m_pcEditDoc->m_cDocLineMgr.GetLineCount();

	//	選択エリアの復元
	m_sSelect.m_ptFrom.y=nSelectLineFromOld;
	m_sSelect.m_ptFrom.x=nSelectColFromOld;
	m_sSelect.m_ptTo.y  =nSelectLineToOld-j;
	m_sSelect.m_ptTo.x  =nSelectColToOld;
	if(nCaretPosYOLD==m_sSelect.m_ptFrom.y){
		MoveCursor( m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y, true );
	}else{
		MoveCursor( m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y, true );
	}
	m_nCaretPosX_Prev = m_ptCaretPos.x;
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* 操作前のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* 操作前のキャレット位置Ｙ */
		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	RedrawAll();

	if(j){
		TopOkMessage( m_hWnd, _T("%d行をマージしました。"), j);
	}else{
		InfoMessage( m_hWnd, _T("マージ可能な行がみつかりませんでした。") );
	}
}



// To Here 2001.12.03 hor
	
/* メニューからの再変換対応 minfu 2002.04.09

	@date 2002.04.11 YAZAKI COsVersionInfoのカプセル化を守りましょう。
*/
void CEditView::Command_Reconvert(void)
{
	//サイズを取得
	int nSize = SetReconvertStruct(NULL,false);
	if( 0 == nSize )  // サイズ０の時は何もしない
		return ;
	
	bool bUseUnicodeATOK = false;
	//バージョンチェック
	if( !OsSupportReconvert() ){
		
		// MSIMEかどうか
		HWND hWnd = ImmGetDefaultIMEWnd(m_hWnd);
		if (SendMessage(hWnd, m_uWM_MSIME_RECONVERTREQUEST, FID_RECONVERT_VERSION, 0)){
			SendMessage(hWnd, m_uWM_MSIME_RECONVERTREQUEST, 0, (LPARAM)m_hWnd);
			return ;
		}

		// ATOKが使えるかどうか
		TCHAR sz[256];
		ImmGetDescription(GetKeyboardLayout(0),sz,_countof(sz));
		if ( (strncmp(sz,_T("ATOK"),4) == 0) && (NULL != m_AT_ImmSetReconvertString) ){
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
	HIMC hIMC = ::ImmGetContext( m_hWnd );
	
	//領域確保
	PRECONVERTSTRING pReconv = (PRECONVERTSTRING)::HeapAlloc(
		GetProcessHeap(),
		HEAP_GENERATE_EXCEPTIONS,
		nSize
	);
	
	//構造体設定
	SetReconvertStruct( pReconv, bUseUnicodeATOK);
	
	//変換範囲の調整
	if(bUseUnicodeATOK){
		(*m_AT_ImmSetReconvertString)(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize, NULL,0);
	}

	//調整した変換範囲を選択する
	SetSelectionFromReonvert(pReconv, bUseUnicodeATOK);
	
	//再変換実行
	if(bUseUnicodeATOK){
		(*m_AT_ImmSetReconvertString)(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize);
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
	if( cDlgCtrlCode.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc ) )
	{
		//コントロールコードを入力する
		Command_CHAR( cDlgCtrlCode.GetCharCode() );
	}
}

/*!	検索開始位置へ戻る
	@author	ai
	@date	02/06/26
*/
void CEditView::Command_JUMP_SRCHSTARTPOS(void)
{
	if( 0 <= m_ptSrchStartPos_PHY.x && 0 <= m_ptSrchStartPos_PHY.y )
	{
		int x, y;
		/* 範囲選択中か */
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			m_ptSrchStartPos_PHY.x,
			m_ptSrchStartPos_PHY.y,
			&x, &y );
		//	2006.07.09 genta 選択状態を保つ
		MoveCursorSelecting( x, y, m_bSelectingLock );
	}
	else
	{
		ErrorBeep();
	}
	return;
}

/*!	履歴の管理(ダイアログ)
	@author	MIK
	@date	2003/04/07
*/
void CEditView::Command_Favorite( void )
{
	CDlgFavorite	cDlgFavorite;

	//ダイアログを表示する
	if( !cDlgFavorite.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc ) )
	{
		return;
	}

	return;
}

/*! 入力する改行コードを設定

	@author moca
	@date 2003.06.23 新規作成
*/
void CEditView::Command_CHGMOD_EOL( EEolType e ){
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

	strncpy( m_pShareData->m_Common.m_sFormat.m_szInyouKigou, quotestr,
		_countof( m_pShareData->m_Common.m_sFormat.m_szInyouKigou ));
	
	m_pShareData->m_Common.m_sFormat.m_szInyouKigou[ _countof( m_pShareData->m_Common.m_sFormat.m_szInyouKigou ) - 1 ] = '\0';
}

/*!	@brief ウィンドウ一覧ポップアップ表示処理（ファイル名のみ）
	@date  2006.03.23 fon 新規作成
	@date  2006.05.19 genta コマンド実行要因を表す引数追加
	@date  2007.07.07 genta コマンド実行要因の値を変更
*/
void CEditView::Command_WINLIST( int nCommandFrom )
{
	CEditWnd	*pCEditWnd;
	pCEditWnd = m_pcEditDoc->m_pcEditWnd;

	//ウィンドウ一覧をポップアップ表示する
	pCEditWnd->PopupWinList(( nCommandFrom & FA_FROMKEYBOARD ) != FA_FROMKEYBOARD );
	// 2007.02.27 ryoji アクセラレータキーからでなければマウス位置に

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

/*!	@brief 編集中の内容を別名保存

	主に編集中の一時ファイル出力などの目的に使用する．
	現在開いているファイル(m_szFilePath)には影響しない．

	@param[in] filename 出力ファイル名
	@param[in] nCharCode 文字コード指定
		@li	CODE_xxxxxxxxxx:各種文字コード
		@li	CODE_AUTODETECT:現在の文字コードを維持
	@param[in] nFlgOpt 動作オプション
		@li	0x01:選択範囲を出力 (非選択状態でも空ファイルを出力する)

	@retval	TRUE 正常終了
	@retval	FALSE ファイル作成に失敗

	@author	maru
	@date	2006.12.10 maru 新規作成
*/
BOOL CEditView::Command_PUTFILE( const char* filename, const ECodeType nCharCode, int nFlgOpt )
{
	BOOL	bResult = TRUE;
	ECodeType	nSaveCharCode;
	nSaveCharCode = nCharCode;
	if(filename[0] == '\0') {
		return FALSE;
	}
	
	if(nSaveCharCode == CODE_AUTODETECT) nSaveCharCode = m_pcEditDoc->m_nCharCode;
	
	//	2007.09.08 genta CEditDoc::FileWrite()にならって砂時計カーソル
	CWaitCursor cWaitCursor( m_hWnd );
	
	if(nFlgOpt & 0x01)
	{	/* 選択範囲を出力 */
		try
		{
			CFileWrite cfw(filename);
			if ( m_pcEditDoc->m_bBomExist) {
				switch( nSaveCharCode ){
				case CODE_UNICODE:
					cfw.Write("\xff\xfe",sizeof(char)*2);
					break;
				case CODE_UNICODEBE:
					cfw.Write( "\xfe\xff", sizeof(char) * 2 );
					break;
				case CODE_UTF8: // 2003.05.04 Moca BOMの間違いを訂正
					cfw.Write( "\xef\xbb\xbf", sizeof(char) * 3 );
					break;
				default:
					//	genta ここに来るのはバグだ
					//	2007.09.08 genta 追加
					PleaseReportToAuthor( NULL, _T("CEditView::Command_PUTFILE/BOM Error\nSaveCharCode=%d"), nSaveCharCode );
				}
			}

			/* 選択範囲の取得 */
			CMemory cMem;
			GetSelectedData(&cMem, FALSE, NULL, FALSE, false);

			/* 書き込み時のコード変換 */
			switch( nSaveCharCode ){
				case CODE_UNICODE:	cMem.SJISToUnicode();break;	/* SJIS→Unicodeコード変換 */
				case CODE_UTF8:		cMem.SJISToUTF8();break;	/* SJIS→UTF-8コード変換 */
				case CODE_UTF7:		cMem.SJISToUTF7();break;	/* SJIS→UTF-7コード変換 */
				case CODE_EUC:		cMem.SJISToEUC();break;		/* SJIS→EUCコード変換 */
				case CODE_JIS:		cMem.SJIStoJIS();break;		/* SJIS→JISコード変換 */
				case CODE_UNICODEBE:	cMem.SJISToUnicodeBE();break;	/* SJIS→UnicodeBEコード変換 */
				case CODE_SJIS:		/* NO BREAK */
				default:			break;
			}
			if( 0 < cMem.GetStringLength() ) cfw.Write(cMem.GetStringPtr(),sizeof(char)*cMem.GetStringLength());
		}
		catch(CError_FileOpen)
		{
			WarningMessage( NULL,
				_T("\'%s\'\nファイルを保存できません。\nパスが存在しないか、他のアプリケーションで使用されている可能性があります。"),
				filename);
			bResult = FALSE;
		}
		catch(CError_FileWrite)
		{
			WarningMessage( NULL, _T("ファイルの書き込み中にエラーが発生しました。") );
			bResult = FALSE;
		}
	}
	else {	/* ファイル全体を出力 */
		CFileTime	cfiletime;
		HWND		hwndProgress;
		CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;
		
		if( NULL != pCEditWnd ){
			hwndProgress = pCEditWnd->m_hwndProgressBar;
		}else{
			hwndProgress = NULL;
		}
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}

		bResult = (BOOL)m_pcEditDoc->m_cDocLineMgr.WriteFile(	// 一時ファイル出力
					filename, m_pcEditDoc->m_hWnd, hwndProgress, nSaveCharCode,
					&cfiletime, EOL_NONE , m_pcEditDoc->m_bBomExist );

		if(hwndProgress) ::ShowWindow( hwndProgress, SW_HIDE );
	}
	return bResult;
}

/*!	@brief カーソル位置にファイルを挿入

	現在のカーソル位置に指定のファイルを読み込む．

	@param[in] filename 入力ファイル名
	@param[in] nCharCode 文字コード指定
		@li	CODE_xxxxxxxxxx:各種文字コード
		@li	CODE_AUTODETECT:前回文字コードもしくは自動判別の結果による
	@param[in] nFlgOpt 動作オプション（現在は未定義．0を指定のこと）

	@retval	TRUE 正常終了
	@retval	FALSE ファイルオープンに失敗

	@author	maru
	@date	2006.12.10 maru 新規作成
*/
BOOL CEditView::Command_INSFILE( const char* filename, ECodeType nCharCode, int nFlgOpt )
{
	CFileLoad	cfl;
	const char*	pLine;
	CEol cEol;
	int			nLineLen;
	int			nLineNum = 0;
	ECodeType	nSaveCharCode;

	CDlgCancel*	pcDlgCancel = NULL;
	HWND		hwndCancel = NULL;
	HWND		hwndProgress = NULL;
	BOOL		bResult = TRUE;

	if(filename[0] == '\0') {
		return FALSE;
	}

	//	2007.09.08 genta CEditDoc::FileRead()にならって砂時計カーソル
	CWaitCursor cWaitCursor( m_hWnd );

	// 範囲選択中なら挿入後も選択状態にするため	/* 2007.04.29 maru */
	int	nLineFrom, nColmFrom;
	BOOL	bBeforeTextSelected = IsTextSelected();
	if (bBeforeTextSelected){
		nLineFrom = m_sSelect.m_ptFrom.y;
		nColmFrom = m_sSelect.m_ptFrom.x;
	}


	nSaveCharCode = nCharCode;
	if(nSaveCharCode == CODE_AUTODETECT) {
		EditInfo		fi;
		const CMRUFile		cMRU;
		if ( cMRU.GetEditInfo( filename, &fi ) ){
				nSaveCharCode = fi.m_nCharCode;
		} else {
			nSaveCharCode = m_pcEditDoc->m_nCharCode;
		}
	}
	
	/* ここまできて文字コードが決定しないならどこかおかしい */
	if( !IsValidCodeType(nSaveCharCode) ) nSaveCharCode = CODE_SJIS;
	
	try{
		// ファイルを開く
		cfl.FileOpen( filename, nSaveCharCode, 0 );

		/* ファイルサイズが65KBを越えたら進捗ダイアログ表示 */
		if ( 0x10000 < cfl.GetFileSize() ) {
			pcDlgCancel = new CDlgCancel;
			if( NULL != ( hwndCancel = pcDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
				hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
				::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
				::SendMessage( hwndProgress, PBM_SETPOS, 0, 0 );
			}
		}

		// ReadLineはファイルから 文字コード変換された1行を読み出します
		// エラー時はthrow CError_FileRead を投げます
		while( NULL != ( pLine = cfl.ReadLine( &nLineLen, &cEol ) ) ){
			++nLineNum;
			Command_INSTEXT( false, pLine, nLineLen, TRUE);

			/* 進捗ダイアログ有無 */
			if( NULL == pcDlgCancel ){
				continue;
			}
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
				break;
			}
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				break;
			}
			if( 0 == ( nLineNum & 0xFF ) ){
				::PostMessage( hwndProgress, PBM_SETPOS, cfl.GetPercent(), 0 );
				Redraw();
			}
		}
		// ファイルを明示的に閉じるが、ここで閉じないときはデストラクタで閉じている
		cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		WarningMessage( NULL, _T("file open error [%s]"), filename );
		bResult = FALSE;
	}
	catch( CError_FileRead ){
		WarningMessage( NULL, _T("ファイルの読み込み中にエラーが発生しました。") );
		bResult = FALSE;
	} // 例外処理終わり

	delete pcDlgCancel;

	if (bBeforeTextSelected){	// 挿入された部分を選択状態に
		SetSelectArea( nLineFrom, nColmFrom, m_ptCaretPos.y, m_ptCaretPos.x );
		DrawSelectArea();
	}
	Redraw();
	return bResult;
}

/*!
	@brief テキストの折り返し方法を変更する
	
	@param[in] nWrapMethod 折り返し方法
		WRAP_NO_TEXT_WRAP  : 折り返さない
		WRAP_SETTING_WIDTH ; 指定桁で折り返す
		WRAP_WINDOW_WIDTH  ; 右端で折り返す
	
	@note ウィンドウが左右に分割されている場合、左側のウィンドウ幅を折り返し幅とする。
	
	@date 2008.05.31 nasukoji	新規作成
	@date 2009.08.28 nasukoji	テキストの最大幅を算出する
*/
void CEditView::Command_TEXTWRAPMETHOD( int nWrapMethod )
{
	// 現在の設定値と同じなら何もしない
	if( m_pcEditDoc->m_nTextWrapMethodCur == nWrapMethod )
		return;

	int nWidth;

	switch( nWrapMethod ){
	case WRAP_NO_TEXT_WRAP:		// 折り返さない
		nWidth = MAXLINEKETAS;	// アプリケーションの最大幅で折り返し
		break;

	case WRAP_SETTING_WIDTH:	// 指定桁で折り返す
		nWidth = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineKetas;
		break;

	case WRAP_WINDOW_WIDTH:		// 右端で折り返す
		// ウィンドウが左右に分割されている場合は左側のウィンドウ幅を使用する
		nWidth = ViewColNumToWrapColNum( m_pcEditDoc->m_pcEditViewArr[0]->m_nViewColNum );
		break;

	default:
		return;		// 不正な値の時は何もしない
	}

	m_pcEditDoc->m_nTextWrapMethodCur = nWrapMethod;	// 設定を記憶

	// 折り返し方法の一時設定適用／一時設定適用解除	// 2008.06.08 ryoji
	m_pcEditDoc->m_bTextWrapMethodCurTemp = !( m_pcEditDoc->GetDocumentAttribute().m_nTextWrapMethod == nWrapMethod );

	// 折り返し位置を変更
	m_pcEditDoc->ChangeLayoutParam( false, m_pcEditDoc->m_cLayoutMgr.GetTabSpace(), nWidth );

	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
		m_pcEditDoc->m_cLayoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
		m_pcEditDoc->RedrawAllViews( NULL );				// スクロールバーの更新が必要なので再表示を実行する
	}else{
		m_pcEditDoc->m_cLayoutMgr.ClearLayoutLineWidth();	// 各行のレイアウト行長の記憶をクリアする
	}
}

/*!
	@brief 指定位置または指定範囲がテキストの存在しないエリアかチェックする

	@param[in] nColmFrom  指定位置または指定範囲開始カラム
	@param[in] nLineFrom  指定位置または指定範囲開始ライン
	@param[in] nColmTo    指定範囲終了カラム
	@param[in] nLineTo    指定範囲終了ライン
	@param[in] bSelect    範囲指定
	@param[in] bBoxSelect 矩形選択
	
	@retval TRUE  指定位置または指定範囲内にテキストが存在しない
			FALSE 指定位置または指定範囲内にテキストが存在する

	@date 2008.08.03 nasukoji	新規作成
*/
BOOL CEditView::IsEmptyArea( int nColmFrom, int nLineFrom, int nColmTo, int nLineTo, BOOL bSelect, BOOL bBoxSelect )
{
	BOOL result;

	if( bSelect && !bBoxSelect && nLineFrom != nLineTo ){	// 複数行の範囲指定
		// 複数行通常選択した場合、必ずテキストを含む
		result = FALSE;
	}else{
		if( bSelect ){
			int nTemp;

			// 範囲の調整
			if( nLineFrom > nLineTo ){
				nTemp = nLineFrom;
				nLineFrom = nLineTo;
				nLineTo = nTemp;
			}

			if( nColmFrom > nColmTo ){
				nTemp = nColmFrom;
				nColmFrom = nColmTo;
				nColmTo = nTemp;
			}
		}else{
			nLineTo = nLineFrom;
		}

		const CLayout*	pcLayout;
		int nLineLen;

		result = TRUE;
		for( int nLineNum = nLineFrom; nLineNum <= nLineTo; nLineNum++ ){
			if( m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout ) ){
				// 指定位置に対応する行のデータ内の位置
				LineColmnToIndex2( pcLayout, nColmFrom, nLineLen );
				if( nLineLen == 0 ){	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					result = FALSE;		// 指定位置または指定範囲内にテキストがある
					break;
				}
			}
		}
	}

	return result;
}

/*[EOF]*/
