//	$Id$
/*!	@file
	CEditViewクラスのコマンド処理系関数群

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "CEditView.h"
#include "CWaitCursor.h"
#include "charcode.h"
#include "CRunningTimer.h"




/* 現在位置にデータを挿入 Ver0  */
void CEditView::InsertData_CEditView(
	int			nX,
	int			nY,
	const char*	pData,
	int			nDataLen,
	int*		pnNewLine,			/* 挿入された部分の次の位置の行 */
	int*		pnNewPos,			/* 挿入された部分の次の位置のデータ位置 */
	COpe*		pcOpe,				/* 編集操作要素 COpe */
	BOOL		bRedraw,
	BOOL		bUndo			/* Undo操作かどうか */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	CRunningTimer*  pCRunningTimer = new CRunningTimer( (const char*)"CEditView::InsertData_CEditView()" );
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

	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nY, &nLineLen, &pcLayout );

	nIdxFrom = 0;
//	cMem.SetData( "", lstrlen( "" ) );
	cMem.SetDataSz( "" );
	if( NULL != pLine ){
		/* 指定された桁に対応する行のデータ内の位置を調べる */
		nIdxFrom = LineColmnToIndex2(
			pLine,
			nLineLen/* - pcLayout->m_cEol.GetLen() + (pcLayout->m_cEol.GetLen()?1:0)*/,
			nX, &nLineAllColLen
		);
		/* 行終端より右に挿入しようとした */
		if( nLineAllColLen > 0 ){
			/* 終端直前から挿入位置まで空白を埋める為の処理 */
			/* 行終端が何らかの改行コードか? */
			if( EOL_NONE != pcLayout->m_cEol ){
//			if( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ){
				nIdxFrom = nLineLen - 1;
//				nIdxFrom = nLineLen - pcLayout->m_cEol.GetLen();
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
				LineIndexToColmn( pLine, nLineLen, nIdxFrom ),
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
		cMem.GetPtr( NULL ),
		cMem.GetLength(),
		&nModifyLayoutLinesOld,
		&nInsLineNum,
		pnNewLine,			/* 挿入された部分の次の位置の行 */
		pnNewPos,			/* 挿入された部分の次の位置のデータ位置 */
		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* シングルクォーテーション文字列を表示する */
		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp,	/* ダブルクォーテーション文字列を表示する */
		bUndo	/* Undo操作かどうか */
	);


	/* メモリが再確保されてアドレスが無効になるので、再度、行データを求める */
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nY, &nLineLen );

	/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
	pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( *pnNewLine, &nLineLen2 );
	if( pLine2 != NULL ){
		*pnNewPos = LineIndexToColmn( pLine2, nLineLen2, *pnNewPos );
	}
	if( *pnNewPos >= m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize ){
		*pnNewPos = 0;
		(*pnNewLine)++;
	}

//	MYTRACE( "nModifyLayoutLinesOld=%d nInsLineNum=%d *pnNewLine=%d *pnNewPos=%d\n", nModifyLayoutLinesOld, nInsLineNum, *pnNewLine, *pnNewPos );


	/* 状態遷移 */
	if( FALSE == m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		m_pcEditDoc->m_bIsModified = TRUE;	/* 変更フラグ */
		if( bRedraw ){
			SetParentCaption();	/* 親ウィンドウのタイトルを更新 */
		}
	}

	/* 再描画 */
	/* 行番号表示に必要な幅を設定 */
	if( DetectWidthOfLineNumberArea( bRedraw ) ){
		::DestroyCaret();
		m_nCaretWidth = 0;
		for( i = 0; i < 4; ++i ){
			if( m_nMyIndex != i ){
				m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
			}
		}
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
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nY - m_nViewTopLine - 1);
				ps.rcPaint.bottom = ps.rcPaint.top + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * ( nModifyLayoutLinesOld + 1);
				if( m_nViewAlignTop + m_nViewCy < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				}
			}
			hdc = ::GetDC( m_hWnd );
			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
			OnSetFocus();
			::ReleaseDC( m_hWnd, hdc );
		}
	}

	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* アンドゥ・リドゥの実行中か */
		pcOpe->m_nOpe = OPE_INSERT;				/* 操作種別 */
//		pcOpe->m_nCaretPosX_To = *pnNewPos;		/* 操作前のキャレット位置Ｘ To */
//		pcOpe->m_nCaretPosY_To = *pnNewLine;	/* 操作前のキャレット位置Ｙ To */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_To,
//			pcOpe->m_nCaretPosY_To,
//			&pcOpe->m_nCaretPosX_PHY_To,
//			&pcOpe->m_nCaretPosY_PHY_To
//		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			*pnNewPos,
			*pnNewLine,
			&pcOpe->m_nCaretPosX_PHY_To,
			&pcOpe->m_nCaretPosY_PHY_To
		);

//?	2000.1.24 ワードラップ時の不具合の根源
//?
//?		if( NULL != pLine ){
//?			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//?				LineIndexToColmn( pLine, nLineLen, nIdxFrom ),
//?				nY,
//?				&pcOpe->m_nCaretPosX_PHY_Before,
//?				&pcOpe->m_nCaretPosY_PHY_Before
//?			);
//?		}else{
//?			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//?				0,
//?				nY,
//?				&pcOpe->m_nCaretPosX_PHY_Before,
//?				&pcOpe->m_nCaretPosY_PHY_Before
//?			);
//?		}




		pcOpe->m_nDataLen = cMem.GetLength();	/* 操作に関連するデータのサイズ */
		pcOpe->m_pcmemData = NULL;				/* 操作に関連するデータ */
	}
#ifdef _DEBUG
	delete pCRunningTimer;
	pCRunningTimer = NULL;

	gm_ProfileOutput = 0;
#endif
	return;
}


/* 指定位置の指定長データ削除 */
void CEditView::DeleteData2(
	int			nCaretX,
	int			nCaretY,
	int			nDelLen,
	CMemory*	pcMem,
	COpe*		pcOpe,		/* 編集操作要素 COpe */
	BOOL		bRedraw,
	BOOL		bRedraw2,
	BOOL		bUndo			/* Undo操作かどうか */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	CRunningTimer* pCRunningTimer = new CRunningTimer( (const char*)"CEditView::DeleteData(1)" );
#endif
	const char*	pLine;
	int			nLineLen;
	int			nIdxFrom;
	int			nModifyLayoutLinesOld;
	int			nModifyLayoutLinesNew;
	int			nDeleteLayoutLines;
	PAINTSTRUCT ps;
	HDC			hdc;
	int			bLastLine;

	/* 最後の行にカーソルがあるかどうか */
	if( nCaretY == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
		bLastLine = 1;
	}else{
		bLastLine = 0;
	}

	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCaretY, &nLineLen );
	if( NULL == pLine ){
		goto end_of_func;
	}
	nIdxFrom = LineColmnToIndex( pLine, nLineLen, nCaretX );
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
			LineIndexToColmn( pLine, nLineLen, nIdxFrom + nDelLen ),
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
		*pcMem,
		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* シングルクォーテーション文字列を表示する */
		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp,	/* ダブルクォーテーション文字列を表示する */
		bUndo	/* Undo操作かどうか */
	);

	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* アンドゥ・リドゥの実行中か */
		pcOpe->m_nDataLen = pcMem->GetLength();	/* 操作に関連するデータのサイズ */
		pcOpe->m_pcmemData = pcMem;				/* 操作に関連するデータ */
	}

	if( bRedraw2 ){
		/* 再描画 */
		if( 0 < nDeleteLayoutLines ){
			ps.rcPaint.left = 0;
			ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
			ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nCaretY - m_nViewTopLine - bLastLine );
			ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			hdc = ::GetDC( m_hWnd );
			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
			OnSetFocus();
			::ReleaseDC( m_hWnd, hdc );

			/* 行番号表示に必要な幅を設定 */
			if( DetectWidthOfLineNumberArea( TRUE ) ){
				int i;
				::DestroyCaret();
				m_nCaretWidth = 0;
				for( i = 0; i < 4; ++i ){
					if( m_nMyIndex != i ){
						m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
					}
				}
				/* キャレットの表示・更新 */
				ShowEditCaret();
			}

			/* スクロールバーの状態を更新する */
			AdjustScrollBars();
		}else{
			if( bRedraw ){
//				ps.rcPaint.left = m_nViewAlignLeft;
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nCaretY - m_nViewTopLine - bLastLine);
				ps.rcPaint.bottom = ps.rcPaint.top + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nModifyLayoutLinesOld + bLastLine );
				hdc = ::GetDC( m_hWnd );
				OnKillFocus();
				OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
				OnSetFocus();
				::ReleaseDC( m_hWnd, hdc );
			}
		}
	}
	/* 選択エリアの先頭へカーソルを移動 */
	MoveCursor( nCaretX, nCaretY, FALSE );
	m_nCaretPosX_Prev = m_nCaretPosX;


end_of_func:;
#ifdef _DEBUG
	delete pCRunningTimer;
	pCRunningTimer = NULL;

	gm_ProfileOutput = 0;
#endif
	return;

}





//カーソル位置または選択エリアを削除
void CEditView::DeleteData(
				BOOL	bRedraw,
				BOOL	bUndo	/* Undo操作かどうか */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	CRunningTimer*  pCRunningTimer = new CRunningTimer( (const char*)"CEditView::DeleteData(2)" );
#endif
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			nLineNum;
	int			nCurIdx;
	int			nNxtIdx;
	int			nNxtPos;
//	int			nModifyLayoutLinesOld;
//	int			nModifyLayoutLinesNew;
//	int			nDeleteLayoutLines;
	PAINTSTRUCT ps;
	HDC			hdc;
	int			nIdxFrom;
	int			nIdxTo;
	int			nDelPos;
	int			nDelLen;
	int			nDelPosNext;
	int			nDelLenNext;
	CMemory		cmemBuf;
	RECT		rcSel;
	int			bLastLine;
	CMemory*	pcMemDeleted;
	COpe*		pcOpe = NULL;
//	int			nPosX;
//	int			nPosY;
//	int			nPosXNext;
//	int			nPosYNext;
	int			nCaretPosXOld;
	int			nCaretPosYOld;
	BOOL		bBoxSelected;
	int			i;
	const CLayout*	pcLayout;
	int			nSelectColmFrom_Old;
	int			nSelectLineFrom_Old;


	nCaretPosXOld = m_nCaretPosX;
	nCaretPosYOld = m_nCaretPosY;
	bBoxSelected = FALSE;

	/* テキストが選択されているか */
	if( IsTextSelected() ){
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
//			pcOpe->m_nCaretPosX_Before = m_nCaretPosX;			/* 操作前のキャレット位置Ｘ */
//			pcOpe->m_nCaretPosY_Before = m_nCaretPosY;			/* 操作前のキャレット位置Ｙ */
//			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//				pcOpe->m_nCaretPosX_Before,
//				pcOpe->m_nCaretPosY_Before,
//				&pcOpe->m_nCaretPosX_PHY_Before,
//				&pcOpe->m_nCaretPosY_PHY_Before
//			);
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* 操作前のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* 操作前のキャレット位置Ｙ */

//			pcOpe->m_nCaretPosX_After = m_nCaretPosX;	/* 操作後のキャレット位置Ｘ */
//			pcOpe->m_nCaretPosY_After = m_nCaretPosY;	/* 操作後のキャレット位置Ｙ */
//			pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* 操作後のキャレット位置Ｘ */
//			pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* 操作後のキャレット位置Ｙ */
			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}

		/* 矩形範囲選択中か */
		if( m_bBeginBoxSelect ){
			bBoxSelected = TRUE;
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
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
				if( NULL != pLine ){
					/* 指定された桁に対応する行のデータ内の位置を調べる */
					nIdxFrom = LineColmnToIndex( pLine, nLineLen, rcSel.left  );
					nIdxTo	 = LineColmnToIndex( pLine, nLineLen, rcSel.right );

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
						pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + 1, &nLineLen2 );
//						pcOpe->m_nCaretPosX_Before = LineIndexToColmn( pLine2, nLineLen2, nDelPos );	/* 操作前のキャレット位置Ｘ */
//						pcOpe->m_nCaretPosY_Before = nLineNum + 1;	/* 操作前のキャレット位置Ｙ */
//						m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//							pcOpe->m_nCaretPosX_Before,
//							pcOpe->m_nCaretPosY_Before,
//							&pcOpe->m_nCaretPosX_PHY_Before,
//							&pcOpe->m_nCaretPosY_PHY_Before
//						);
						m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
							LineIndexToColmn( pLine2, nLineLen2, nDelPos ),
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
						pcOpe,				/* 編集操作要素 COpe */
						bRedraw,
						bRedraw,
						bUndo	/* Undo操作かどうか */
					);

					if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
//						pcOpe->m_nCaretPosX_After = rcSel.left;	/* 操作後のキャレット位置Ｘ */
//						pcOpe->m_nCaretPosY_After = nLineNum + 1;	/* 操作後のキャレット位置Ｙ */
//						m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//							pcOpe->m_nCaretPosX_After,
//							pcOpe->m_nCaretPosY_After,
//							&pcOpe->m_nCaretPosX_PHY_After,
//							&pcOpe->m_nCaretPosY_PHY_After
//						);
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

			/* 行番号表示に必要な幅を設定 */
			if( DetectWidthOfLineNumberArea( TRUE ) ){
				for( i = 0; i < 4; ++i ){
					::DestroyCaret();
					m_nCaretWidth = 0;
					if( m_nMyIndex != i ){
						m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
					}
				}
				/* キャレットの表示・更新 */
				ShowEditCaret();
			}
			if( bRedraw ){
				/* スクロールバーの状態を更新する */
				AdjustScrollBars();
			}
			if( bRedraw  ){
			/* 再描画 */
				hdc = ::GetDC( m_hWnd );
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
	//			ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (m_nCaretPosY - m_nViewTopLine);
				ps.rcPaint.top = m_nViewAlignTop;
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				OnKillFocus();
				OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
				OnSetFocus();
				::ReleaseDC( m_hWnd, hdc );
			}
			/* 選択エリアの先頭へカーソルを移動 */
			::UpdateWindow( m_hWnd );
			MoveCursor( nSelectColmFrom_Old, nSelectLineFrom_Old, bRedraw );
			m_nCaretPosX_Prev = m_nCaretPosX;
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe = new COpe;
				pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
//				pcOpe->m_nCaretPosX_Before = nCaretPosXOld;			/* 操作前のキャレット位置Ｘ */
//				pcOpe->m_nCaretPosY_Before = nCaretPosYOld;			/* 操作前のキャレット位置Ｙ */
//				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//					pcOpe->m_nCaretPosX_Before,
//					pcOpe->m_nCaretPosY_Before,
//					&pcOpe->m_nCaretPosX_PHY_Before,
//					&pcOpe->m_nCaretPosY_PHY_Before
//				);
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
					nCaretPosXOld,
					nCaretPosYOld,
					&pcOpe->m_nCaretPosX_PHY_Before,
					&pcOpe->m_nCaretPosY_PHY_Before
				);

//				pcOpe->m_nCaretPosX_After = m_nCaretPosX;	/* 操作後のキャレット位置Ｘ */
//				pcOpe->m_nCaretPosY_After = m_nCaretPosY;	/* 操作後のキャレット位置Ｙ */
//				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//					pcOpe->m_nCaretPosX_After,
//					pcOpe->m_nCaretPosY_After,
//					&pcOpe->m_nCaretPosX_PHY_After,
//					&pcOpe->m_nCaretPosY_PHY_After
//				);
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
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL == pLine ){
			goto end_of_func;
			return;
		}
		/* 最後の行にカーソルがあるかどうか */
		if( m_nCaretPosY == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
			bLastLine = 1;
		}else{
			bLastLine = 0;
		}

		/* 指定された桁に対応する行のデータ内の位置を調べる */
		nCurIdx = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );
//		MYTRACE("nLineLen=%d nCurIdx=%d \n", nLineLen, nCurIdx);
		if( nCurIdx == nLineLen && bLastLine ){	/* 全テキストの最後 */
			goto end_of_func;
			return;
		}
		/* 指定された桁の文字のバイト数を調べる */
		if( pLine[nCurIdx] == '\r' || pLine[nCurIdx] == '\n' ){
			/* 改行 */
			nNxtIdx = nCurIdx + pcLayout->m_cEol.GetLen();
			nNxtPos = m_nCaretPosX + pcLayout->m_cEol.GetLen();
		}else{
			nNxtIdx = CMemory::MemCharNext( pLine, nLineLen, &pLine[nCurIdx] ) - pLine;
			/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
			nNxtPos = LineIndexToColmn( pLine, nLineLen, nNxtIdx );
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

	m_pcEditDoc->m_bIsModified = TRUE;	/* 変更フラグ */
	if( bRedraw ){
		SetParentCaption();	/* 親ウィンドウのタイトルを更新 */
	}

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() > 0 ){
		if( m_nCaretPosY > m_pcEditDoc->m_cLayoutMgr.GetLineCount()	- 1	){
			/* 現在行のデータを取得 */
			const CLayout*	pcLayout;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				goto end_of_func;
				return;
			}
			/* 改行で終わっているか */
			if( ( EOL_NONE != pcLayout->m_cEol.GetLen() ) ){
				goto end_of_func;
				return;
			}
			/*ファイルの最後に移動 */
			Command_GOFILEEND( FALSE );
		}
	}
end_of_func:;
//#ifdef _DEBUG
//	delete pCRunningTimer;
//	pCRunningTimer = NULL;
//
//	gm_ProfileOutput = 0;
//#endif

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

#ifdef _DEBUG
//	MYTRACE( "\n\n======================================\n" );
	CRunningTimer cRunningTimer( (const char*)"CEditView::Command_UNDO()" );
#endif
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
	BOOL		bUndo;	/* Undo操作かどうか */
	bUndo = TRUE;	/* Undo操作かどうか */

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


			if( i == nOpeBlkNum - 1	){
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, TRUE );
			}else{
				/* カーソルを移動 */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, FALSE );
			}
			switch( pcOpe->m_nOpe ){
			case OPE_INSERT:
				pcMem = new CMemory;

				/* 選択範囲の変更 */
//				m_nSelectLineBgn = pcOpe->m_nCaretPosY_Before;	/* 範囲選択開始行(原点) */
//				m_nSelectColmBgn = pcOpe->m_nCaretPosX_Before;	/* 範囲選択開始桁(原点) */

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
//				m_nSelectLineBgn = -1;	/* 範囲選択開始行(原点) */
//				m_nSelectColmBgn = -1;	/* 範囲選択開始桁(原点) */
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
						pcOpe->m_pcmemData->GetPtr( NULL ),	/* 挿入するデータ */
						pcOpe->m_nDataLen,					/* 挿入するデータの長さ */
						FALSE								/*再描画するか否か*/
					);

//					InsertData_CEditView(
//						nCaretPosX_Before,
//						nCaretPosY_Before,
//						pcOpe->m_pcmemData->GetPtr( NULL ),
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
				MoveCursor( nCaretPosX_After, nCaretPosY_After, TRUE );
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
		m_pcEditDoc->m_bIsModified = bIsModified;	/* Undo後の変更フラグ */
		SetParentCaption();	/* 親ウィンドウのタイトルを更新 */

		m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */

		/* 再描画 */
		hdc = ::GetDC( m_hWnd );
		ps.rcPaint.left = 0;
		ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
		ps.rcPaint.top = m_nViewAlignTop;
		ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
		OnKillFocus();
		OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
		OnSetFocus();
		::ReleaseDC( m_hWnd, hdc );
		/* 行番号表示に必要な幅を設定 */
		if( DetectWidthOfLineNumberArea( TRUE ) ){
			::DestroyCaret();
			m_nCaretWidth = 0;
			for( i = 0; i < 4; ++i ){
				if( m_nMyIndex != i ){
					m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
				}
			}
			/* キャレットの表示・更新 */
			ShowEditCaret();
		}

	//	2001/06/21 Start by asa-o: 他のペインの表示状態を更新
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^1].Redraw();
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^2].Redraw();
		m_pcEditDoc->m_cEditViewArr[(m_nMyIndex^1)^2].Redraw();
	//	2001/06/21 End

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
#ifdef _DEBUG
//	MYTRACE( "\n\n======================================\n" );
	CRunningTimer cRunningTimer( (const char*)"CEditView::Command_REDO()" );
#endif
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
						pcOpe->m_pcmemData->GetPtr( NULL ),	/* 挿入するデータ */
						pcOpe->m_pcmemData->GetLength(),	/* 挿入するデータの長さ */
						FALSE								/*再描画するか否か*/
					);

//					InsertData_CEditView(
//						nCaretPosX_Before,
//						nCaretPosY_Before,
//						pcOpe->m_pcmemData->GetPtr( NULL ),
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
		m_pcEditDoc->m_bIsModified = bIsModified;	/* Redo後の変更フラグ */
		SetParentCaption();	/* 親ウィンドウのタイトルを更新 */

		m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */

		/* 再描画 */
		hdc = ::GetDC( m_hWnd );
		ps.rcPaint.left = 0;
		ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
		ps.rcPaint.top = m_nViewAlignTop;
		ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
		OnKillFocus();
		OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
		OnSetFocus();
		::ReleaseDC( m_hWnd, hdc );

		/* 行番号表示に必要な幅を設定 */
		if( DetectWidthOfLineNumberArea( TRUE ) ){
			::DestroyCaret();
			m_nCaretWidth = 0;
			for( i = 0; i < 4; ++i ){
				if( m_nMyIndex != i ){
					m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
				}
			}
			/* キャレットの表示・更新 */
			ShowEditCaret();
		}

	//	2001/06/21 Start by asa-o: 他のペインの表示状態を更新
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^1].Redraw();
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^2].Redraw();
		m_pcEditDoc->m_cEditViewArr[(m_nMyIndex^1)^2].Redraw();
	//	2001/06/21 End

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
//#ifdef _DEBUG
//	gm_ProfileOutput = 1;
//	CRunningTimer*  pCRunningTimer = new CRunningTimer( (const char*)"CEditView::ReplaceData_CEditView()" );
//#endif
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
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nDelLineFrom, &len );
		//	Jun. 1, 2000 genta
		//	ちゃんとNULLチェックしましょう
		if( line != NULL ){
			pos = LineColmnToIndex( line, len, nDelColmFrom );
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
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nDelLineTo, &len );
		if( line != NULL ){
			pos = LineIndexToColmn( line, len, len );

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
	LRArg.bDispSSTRING = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp;			/* シングルクォーテーション文字列を表示する */
	LRArg.bDispWSTRING = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp;			/* ダブルクォーテーション文字列を表示する */
	LRArg.bUndo = m_bDoing_UndoRedo;					/* Undo操作かどうか */
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
		m_pcEditDoc->m_bIsModified = TRUE;	/* 変更フラグ */
		if( bRedraw ){
			SetParentCaption();	/* 親ウィンドウのタイトルを更新 */
		}
	}

	/* 現在の選択範囲を非選択状態に戻す */
	DisableSelectArea( bRedraw );

	/* 行番号表示に必要な幅を設定 */
	if( DetectWidthOfLineNumberArea( bRedraw ) ){
		::DestroyCaret();
		m_nCaretWidth = 0;
		int i;
		for( i = 0; i < 4; ++i ){
			if( m_nMyIndex != i ){
				m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
			}
		}
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
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (m_nCaretPosY - m_nViewTopLine);
				ps.rcPaint.top = 0/*m_nViewAlignTop*/;
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			}else{
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;

				/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
				ps.rcPaint.top = m_nViewAlignTop + (LRArg.nModLineFrom - m_nViewTopLine)* (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				if( m_pcEditDoc->GetDocumentAttribute().m_bWordWrap ){
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
			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
			OnSetFocus();
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
#if 0
	/* 行番号表示に必要な幅を設定 */
	if( DetectWidthOfLineNumberArea( TRUE ) ){
		for( int i = 0; i < 4; ++i ){
			::DestroyCaret();
			m_nCaretWidth = 0;
			if( m_nMyIndex != i ){
				m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
			}
		}
		/* キャレットの表示・更新 */
		ShowEditCaret();
	}
#endif
	//	Jan. 30, 2001 genta
	//	ファイル全体の更新フラグが立っていないと各行の更新状態が表示されないので
	//	フラグ更新処理を再描画より前に移動する

//#ifdef _DEBUG
//	gm_ProfileOutput = 1;
//	delete pCRunningTimer;
//	pCRunningTimer = NULL;
//
//	gm_ProfileOutput = 0;
//#endif
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
					nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
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
				 || ( '{' == cChar )
				 || ( '(' == cChar )
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
		for( j = m_nCaretPosY_PHY; j >= 0 && NULL != ( pLine2 = m_pcEditDoc->m_cDocLineMgr.GetLineStr( j, &nLineLen2 ) ); --j ){
			if( j == m_nCaretPosY_PHY ){
				nCharChars = &pLine2[nWork] - CMemory::MemCharPrev( pLine2, nLineLen2, &pLine2[nWork] );
				k = nWork - nCharChars;
			}else{
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
			pszData = new char[nDataLen + 2];
			memcpy( pszData, pLine2, nDataLen );
			if( CR  == cChar
			 || '{' == cChar
			 || '(' == cChar
			){
				pszData[nDataLen] = '\t';
				pszData[nDataLen + 1] = '\0';
				++nDataLen;
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
		delete pszData;
		pszData = NULL;
	}
	return;
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
		CaretUnderLineOFF(TRUE);
	}

	ScrollAtV(m_nViewTopLine - 1);

	// テキストが選択されていない
	if( !IsTextSelected() )
	{
		// カーソルが画面外に出た
		if( m_nCaretPosY > m_nViewRowNum + m_nViewTopLine - nCaretMarginY )
		{
			if( m_nCaretPosY > m_pcEditDoc->m_cDocLineMgr.GetLineCount() - nCaretMarginY )
				Cursor_UPDOWN( (m_pcEditDoc->m_cDocLineMgr.GetLineCount() - nCaretMarginY) - m_nCaretPosY, FALSE );
			else
				Cursor_UPDOWN( -1, FALSE);
			DrawCaretPosInfo();
		}
	}
	if( m_pShareData->m_Common.m_bSplitterWndVScroll )	// 垂直スクロールの同期をとる
	{
		CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x01];
		pcEditView -> ScrollAtV( m_nViewTopLine );
	}

	CaretUnderLineON(TRUE);
}

// テキストを１行上へスクロール
void CEditView::Command_WndScrollUp(void)
{
	int	nCaretMarginY;

	nCaretMarginY = m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	if( m_nCaretPosY < m_nViewTopLine + (nCaretMarginY + 1) ){
		CaretUnderLineOFF( TRUE );
	}

	ScrollAtV( m_nViewTopLine + 1 );

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
	if( m_pShareData->m_Common.m_bSplitterWndVScroll )	// 垂直スクロールの同期をとる
	{
		CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x01];
		pcEditView -> ScrollAtV( m_nViewTopLine );
	}

	CaretUnderLineON( TRUE );
}

// 2001/06/20 End


/*[EOF]*/
