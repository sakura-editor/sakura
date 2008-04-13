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
	Copyright (C) 2007, ryoji, maru

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "view/CEditView.h"
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
#include <string>///
#include <vector> /// 2002/2/3 aroka to here
#include "COsVersionInfo.h"   // 2002.04.09 minfu 
#include "CEditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理
#include "CEditWnd.h"
#include "CDlgCtrlCode.h"	//コントロールコードの入力(ダイアログ)
#include "CDlgFavorite.h"	//履歴の管理	//@@@ 2003.04.08 MIK

using namespace std; // 2002/2/3 aroka to here



/*!	現在位置にデータを挿入 Ver0

	@date 2002/03/24 YAZAKI bUndo削除
*/
void CEditView::InsertData_CEditView(
	CLayoutPoint	ptInsertPos,	// [in] 挿入位置
	const wchar_t*	pData,			// [in] 挿入テキスト
	int				nDataLen,		// [in] 挿入テキスト長。文字単位。
	CLayoutPoint*	pptNewPos,		// [out] 挿入された部分の次の位置のレイアウト位置
	bool			bRedraw
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::InsertData_CEditView" );
#endif

	//2007.10.18 kobake COpe処理をここにまとめる
	CInsertOpe* pcOpe = NULL;
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new CInsertOpe();
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			ptInsertPos,
			&pcOpe->m_ptCaretPos_PHY_Before
		);
	}


	pptNewPos->y = 0;			// 挿入された部分の次の位置のレイアウト行
	pptNewPos->x = 0;			// 挿入された部分の次の位置のレイアウト位置

	// テキストが選択されているか
	if( GetSelectionInfo().IsTextSelected() ){
		DeleteData( bRedraw );
		ptInsertPos = GetCaret().GetCaretLayoutPos();
	}

	//テキスト取得 -> pLine, nLineLen, pcLayout
	CLogicInt		nLineLen;
	const CLayout*	pcLayout;
	const wchar_t*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( ptInsertPos.GetY2(), &nLineLen, &pcLayout );

	CLayoutInt	nLineAllColLen;
	CLogicInt	nIdxFrom = CLogicInt(0);
	CNativeW	cMem;
	cMem.SetString(L"");
	if( pLine ){
		// 指定された桁に対応する行のデータ内の位置を調べる
		nIdxFrom = LineColmnToIndex2( pcLayout, ptInsertPos.GetX2(), &nLineAllColLen );

		// 行終端より右に挿入しようとした
		if( nLineAllColLen > 0 ){
			// 終端直前から挿入位置まで空白を埋める為の処理
			// 行終端が何らかの改行コードか?
			if( EOL_NONE != pcLayout->GetLayoutEol() ){
				nIdxFrom = nLineLen - CLogicInt(1);
				for( int i = 0; i < ptInsertPos.GetX2() - nLineAllColLen + 1; ++i ){
					cMem += L' ';
				}
				cMem.AppendString( pData, nDataLen );
			}
			else{
				nIdxFrom = nLineLen;
				for( int i = 0; i < ptInsertPos.GetX2() - nLineAllColLen; ++i ){
					cMem += L' ';
				}
				cMem.AppendString( pData, nDataLen );
			}
		}
		else{
			cMem.AppendString( pData, nDataLen );
		}
	}
	else{
		for( CLayoutInt i = CLayoutInt(0); i < ptInsertPos.GetX2(); ++i ){
			cMem += L' ';
		}
		cMem.AppendString( pData, nDataLen );
	}


	if( !m_bDoing_UndoRedo && pcOpe ){	// アンドゥ・リドゥの実行中か
		if( pLine ){
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				CLayoutPoint(LineIndexToColmn( pcLayout, nIdxFrom ), ptInsertPos.y),
				&pcOpe->m_ptCaretPos_PHY_Before
			);
		}
		else{
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				CLayoutPoint(CLayoutInt(0), ptInsertPos.y),
				&pcOpe->m_ptCaretPos_PHY_Before
			);
		}
	}


	// 文字列挿入
	CLayoutInt	nModifyLayoutLinesOld=CLayoutInt(0);
	CLayoutInt	nInsLineNum;		/* 挿入によって増えたレイアウト行の数 */
	m_pcEditDoc->m_cLayoutMgr.InsertData_CLayoutMgr(
		ptInsertPos.GetY2(),
		nIdxFrom,
		cMem.GetStringPtr(),
		CLogicInt(cMem.GetStringLength()),
		&nModifyLayoutLinesOld,
		&nInsLineNum,
		pptNewPos
	);

	// メモリが再確保されてアドレスが無効になるので、再度、行データを求める
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( ptInsertPos.GetY2(), &nLineLen );

	// 指定された行のデータ内の位置に対応する桁の位置を調べる
	const wchar_t*	pLine2;
	CLogicInt		nLineLen2;
	pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( pptNewPos->GetY2(), &nLineLen2, &pcLayout );
	if( pLine2 ){
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		pptNewPos->x = pptNewPos->GetX2(); //LineIndexToColmn( pcLayout, pptNewPos->GetX2() );
	}

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if( pptNewPos->x >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){
		if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuRet
		 || m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.16 MIK
		{
			if( m_pcEditDoc->m_cLayoutMgr.IsEndOfLine( *pptNewPos ) )	//@@@ 2002.04.18
			{
				pptNewPos->x = 0;
				pptNewPos->y++;
			}
		}
		else
		{
			// Oct. 7, 2002 YAZAKI
			pptNewPos->x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : CLayoutInt(0);
			pptNewPos->y++;
		}
	}

	// 状態遷移
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		m_pcEditDoc->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	// 再描画
	// 行番号表示に必要な幅を設定
	if( m_pcEditWnd->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
		// キャレットの表示・更新
		GetCaret().ShowEditCaret();
	}
	else{
		PAINTSTRUCT ps;

		if( bRedraw ){
			if( 0 < nInsLineNum ){
				// スクロールバーの状態を更新する
				AdjustScrollBars();
				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();
				ps.rcPaint.top = GetTextArea().GetAreaTop() + GetTextMetrics().GetHankakuDy() * (Int)(ptInsertPos.y - GetTextArea().GetViewTopLine());
				ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
			}
			else{
				if( nModifyLayoutLinesOld < 1 ){
					nModifyLayoutLinesOld = CLayoutInt(1);
				}
	//			ps.rcPaint.left = GetTextArea().GetAreaLeft();
				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();

				// 2002.02.25 Mod By KK 次行 (ptInsertPos.y - GetTextArea().GetViewTopLine() - 1); => (ptInsertPos.y - GetTextArea().GetViewTopLine());
				//ps.rcPaint.top = GetTextArea().GetAreaTop() + GetTextMetrics().GetHankakuDy() * (ptInsertPos.y - GetTextArea().GetViewTopLine() - 1);
				ps.rcPaint.top = GetTextArea().GetAreaTop() + GetTextMetrics().GetHankakuDy() * (Int)(ptInsertPos.y - GetTextArea().GetViewTopLine());

				//禁則がある場合は1行前から再描画を行う	@@@ 2002.04.19 MIK
				if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bWordWrap
				 || m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuHead	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuTail	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuRet	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.19 MIK
				{
					ps.rcPaint.top -= GetTextMetrics().GetHankakuDy();
				}
				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}

				ps.rcPaint.bottom = ps.rcPaint.top + GetTextMetrics().GetHankakuDy() * (Int)( nModifyLayoutLinesOld + 1);
				if( GetTextArea().GetAreaBottom() < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
				}
			}
			HDC hdc = this->GetDC();
			OnPaint( hdc, &ps, TRUE );	// メモリＤＣを使用してちらつきのない再描画
			this->ReleaseDC( hdc );
		}
	}

	if( !m_bDoing_UndoRedo && pcOpe ){	/* アンドゥ・リドゥの実行中か */
		pcOpe->m_pcmemData.Clear();					/* 操作に関連するデータ */
	}



	//2007.10.18 kobake ここでCOpe処理をまとめる
	if( !m_bDoing_UndoRedo ){
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			*pptNewPos,
			&pcOpe->m_ptCaretPos_PHY_After
		);

		// 操作の追加
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

#ifdef _DEBUG
	gm_ProfileOutput = 0;
#endif
}


/*!	指定位置の指定長データ削除

	@date 2002/03/24 YAZAKI bUndo削除
	@date 2002/05/12 YAZAKI bRedraw, bRedraw2削除（常にFALSEだから）
	@date 2007/10/17 kobake (重要)pcMemの所有者が条件によりCOpeに移ったり移らなかったりする振る舞いは
	                        非常にややこしく混乱の元になるため、常に、pcMemの所有者は移さないように仕様変更。
*/
void CEditView::DeleteData2(
	const CLayoutPoint& _ptCaretPos,
	CLogicInt			nDelLen,
	CNativeW*			pcMem		//!< [out]
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DeleteData(1)" );
#endif
	//_ptCaretPosをロジック位置に変換 -> ptDelPos
	CLogicPoint ptDelPos;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		_ptCaretPos,
		&ptDelPos
	);

	/* データ削除 */
	CLayoutInt	nModifyLayoutLinesOld;
	CLayoutInt	nModifyLayoutLinesNew;
	CLayoutInt	nDeleteLayoutLines;
	m_pcEditDoc->m_cLayoutMgr.DeleteData_CLayoutMgr(
		_ptCaretPos.GetY2(),
		ptDelPos.x,
		nDelLen,
		&nModifyLayoutLinesOld,
		&nModifyLayoutLinesNew,
		&nDeleteLayoutLines,
		pcMem
	);

	//2007.10.18 kobake COpeの生成をここにまとめる
	CDeleteOpe*	pcOpe = NULL;
	if( !m_bDoing_UndoRedo ){
		pcOpe = new CDeleteOpe();
		pcOpe->m_ptCaretPos_PHY_Before = ptDelPos;
		pcOpe->m_ptCaretPos_PHY_To = ptDelPos + CLogicPoint(nDelLen, CLogicInt(0));
		pcOpe->m_nDataLen = pcMem->GetStringLength();	/* 操作に関連するデータのサイズ */
		pcOpe->m_pcmemData = *pcMem;					/* 操作に関連するデータ */
	}

	/* 選択エリアの先頭へカーソルを移動 */
	GetCaret().MoveCursor( _ptCaretPos, false );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();

	//2007.10.18 kobake COpeの追加をここにまとめる
	if( pcOpe ){
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			_ptCaretPos,
			&pcOpe->m_ptCaretPos_PHY_After
		);
		// 操作の追加
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

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
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLayoutInt	nLineNum;
	CLogicInt	nCurIdx;
	CLogicInt	nNxtIdx;
	CLogicInt	nIdxFrom;
	CLogicInt	nIdxTo;
	CLogicInt	nDelPos;
	CLogicInt	nDelLen;
	CLogicInt	nDelPosNext;
	CLogicInt	nDelLenNext;
	CLayoutRect		rcSel;
	const CLayout*	pcLayout;

	CLayoutPoint ptSelectFrom_Old;

	CLayoutPoint ptCaretPosOld = GetCaret().GetCaretLayoutPos();

	/* テキストが選択されているか */
	if( GetSelectionInfo().IsTextSelected() ){
		CWaitCursor cWaitCursor( this->GetHwnd() );  // 2002.02.05 hor
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
					GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
				)
			);
		}

		/* 矩形範囲選択中か */
		if( GetSelectionInfo().IsBoxSelecting() ){
			m_pcEditDoc->m_cDocEditor.SetModified(true,bRedraw);	//	2002/06/04 YAZAKI 矩形選択を削除したときに変更マークがつかない。

			SetDrawSwitch(false);	// 2002.01.25 hor
			ptSelectFrom_Old = GetSelectionInfo().m_sSelect.GetFrom();

			/* 選択範囲のデータを取得 */
			/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
			/* ２点を対角とする矩形を求める */
			TwoPointToRect(
				&rcSel,
				GetSelectionInfo().m_sSelect.GetFrom(),	// 範囲選択開始
				GetSelectionInfo().m_sSelect.GetTo()		// 範囲選択終了
			);
			/* 現在の選択範囲を非選択状態に戻す */
			GetSelectionInfo().DisableSelectArea( bRedraw );

			nIdxFrom = CLogicInt(0);
			nIdxTo = CLogicInt(0);
			for( nLineNum = rcSel.bottom; nLineNum >= rcSel.top - 1; nLineNum-- ){
				nDelPosNext = nIdxFrom;
				nDelLenNext	= nIdxTo - nIdxFrom;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
				if( pLine ){
					using namespace WCODE;

					/* 指定された桁に対応する行のデータ内の位置を調べる */
					nIdxFrom = LineColmnToIndex( pcLayout, rcSel.left  );
					nIdxTo	 = LineColmnToIndex( pcLayout, rcSel.right );

					for( CLogicInt i = nIdxFrom; i <= nIdxTo; ++i ){
						if( pLine[i] == CR || pLine[i] == LF ){
							nIdxTo = i;
							break;
						}
					}
				}else{
					nIdxFrom = CLogicInt(0);
					nIdxTo	 = CLogicInt(0);
				}
				nDelPos = nDelPosNext;
				nDelLen	= nDelLenNext;
				if( nLineNum < rcSel.bottom && 0 < nDelLen ){
					CNativeW	pcMemDeleted;
					// 指定位置の指定長データ削除
					DeleteData2(
						CLayoutPoint(rcSel.left, nLineNum + 1),
						nDelLen,
						&pcMemDeleted
					);
				}
			}
			SetDrawSwitch(true);	// 2002.01.25 hor

			/* 行番号表示に必要な幅を設定 */
			if ( m_pcEditWnd->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
				/* キャレットの表示・更新 */
				GetCaret().ShowEditCaret();
			}
			if( bRedraw ){
				/* スクロールバーの状態を更新する */
				AdjustScrollBars();

				/* 再描画 */
				Call_OnPaint(PAINT_LINENUMBER | PAINT_BODY, TRUE); // メモリＤＣを使用してちらつきのない再描画
			}
			/* 選択エリアの先頭へカーソルを移動 */
			this->UpdateWindow();
			GetCaret().MoveCursor( ptSelectFrom_Old, bRedraw );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				CMoveCaretOpe*		pcOpe = new CMoveCaretOpe();
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
					ptCaretPosOld,
					&pcOpe->m_ptCaretPos_PHY_Before
				);

				pcOpe->m_ptCaretPos_PHY_After = GetCaret().GetCaretLogicPos();	// 操作後のキャレット位置
				/* 操作の追加 */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}
		}else{
			/* データ置換 削除&挿入にも使える */
			ReplaceData_CEditView(
				GetSelectionInfo().m_sSelect,
				NULL,					/* 削除されたデータのコピー(NULL可能) */
				L"",					/* 挿入するデータ */
				CLogicInt(0),			/* 挿入するデータの長さ */
				bRedraw,
				m_bDoing_UndoRedo?NULL:m_pcOpeBlk
			);
		}
	}else{
		/* 現在行のデータを取得 */
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );
		if( NULL == pLine ){
			goto end_of_func;
//			return;
		}
		/* 最後の行にカーソルがあるかどうか */
		bool bLastLine = ( GetCaret().GetCaretLayoutPos().GetY() == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 );

		/* 指定された桁に対応する行のデータ内の位置を調べる */
		nCurIdx = LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
//		MYTRACE_A("nLineLen=%d nCurIdx=%d \n", nLineLen, nCurIdx);
		if( nCurIdx == nLineLen && bLastLine ){	/* 全テキストの最後 */
			goto end_of_func;
//			return;
		}

		/* 指定された桁の文字のバイト数を調べる */
		CLayoutInt	nNxtPos;
		if( WCODE::isLineDelimiter(pLine[nCurIdx]) ){
			/* 改行 */
			nNxtIdx = nCurIdx + pcLayout->GetLayoutEol().GetLen();
			nNxtPos = GetCaret().GetCaretLayoutPos().GetX() + CLayoutInt((Int)pcLayout->GetLayoutEol().GetLen()); //※改行コードの文字数を文字幅と見なす
		}
		else{
			nNxtIdx = CLogicInt(CNativeW::GetCharNext( pLine, nLineLen, &pLine[nCurIdx] ) - pLine);
			// 指定された行のデータ内の位置に対応する桁の位置を調べる
			nNxtPos = LineIndexToColmn( pcLayout, nNxtIdx );
		}


		/* データ置換 削除&挿入にも使える */
		CLayoutRange sDelRange;
		sDelRange.SetFrom(GetCaret().GetCaretLayoutPos());
		sDelRange.SetTo(CLayoutPoint(nNxtPos,GetCaret().GetCaretLayoutPos().GetY()));
		ReplaceData_CEditView(
			sDelRange,
			NULL,				/* 削除されたデータのコピー(NULL可能) */
			L"",				/* 挿入するデータ */
			CLogicInt(0),		/* 挿入するデータの長さ */
			bRedraw,
			m_bDoing_UndoRedo?NULL:m_pcOpeBlk
		);
	}

	m_pcEditDoc->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() > 0 ){
		if( GetCaret().GetCaretLayoutPos().GetY() > m_pcEditDoc->m_cLayoutMgr.GetLineCount()	- 1	){
			/* 現在行のデータを取得 */
			const CLayout*	pcLayout;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_pcEditDoc->m_cLayoutMgr.GetLineCount() - CLayoutInt(1), &nLineLen, &pcLayout );
			if( NULL == pLine ){
				goto end_of_func;
			}
			/* 改行で終わっているか */
			if( ( EOL_NONE != pcLayout->GetLayoutEol() ) ){
				goto end_of_func;
			}
			/*ファイルの最後に移動 */
			GetCommander().Command_GOFILEEND( FALSE );
		}
	}
end_of_func:;

	return;
}






/* データ置換 削除&挿入にも使える */
// Jun 23, 2000 genta 変数名を書き換え忘れていたのを修正
// Jun. 1, 2000 genta DeleteDataから移動した
void CEditView::ReplaceData_CEditView(
	CLayoutRange	sDelRange,			//!< [in]  削除範囲レイアウト単位
	CNativeW*		pcmemCopyOfDeleted,	//!< [out] 削除されたデータのコピー(NULL可能)
	const wchar_t*	pInsData,			//!< [in]  挿入するデータ
	CLogicInt		nInsDataLen,		//!< [in]  挿入するデータの長さ
	bool			bRedraw,
	COpeBlk*		pcOpeBlk
)
{
	{
		//	May. 29, 2000 genta
		//	From Here
		//	行の後ろが選択されていたときの不具合を回避するため，
		//	選択領域から行末以降の部分を取り除く．

		//	先頭
		const CLayout*	pcLayout;
		CLogicInt		len;
		const wchar_t*	line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sDelRange.GetFrom().GetY2(), &len, &pcLayout );
		if( line ){
			CLogicInt pos = LineColmnToIndex( pcLayout, sDelRange.GetFrom().GetX2() );
			//	Jun. 1, 2000 genta
			//	同一行の行末以降のみが選択されている場合を考慮する

			//	Aug. 22, 2000 genta
			//	開始位置がEOFの後ろのときは次行に送る処理を行わない
			//	これをやってしまうと存在しない行をPointして落ちる．
			if( sDelRange.GetFrom().y < m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 && pos >= len){
				if( sDelRange.GetFrom().y == sDelRange.GetTo().y  ){
					//	GetSelectionInfo().m_sSelect.GetFrom().y <= GetSelectionInfo().m_sSelect.GetTo().y はチェックしない
					CLayoutPoint tmp = sDelRange.GetFrom();
					tmp.y++;
					sDelRange.Set(tmp);
				}
				else {
					sDelRange.GetFromPointer()->y++;
					sDelRange.SetFromX(0);
				}
			}
		}

		//	末尾
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sDelRange.GetTo().GetY2(), &len, &pcLayout );
		if( line ){
			CLayoutInt p = LineIndexToColmn( pcLayout, len );

			if( sDelRange.GetTo().x > p ){
				sDelRange.SetToX( p );
			}
		}
		//	To Here
	}

	//削除範囲ロジック単位 sDelRange -> sDelRangeLogic
	CLogicRange sDelRangeLogic;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		sDelRange,
		&sDelRangeLogic
	);


	CLogicPoint		ptCaretPos_PHY_Old;

	ptCaretPos_PHY_Old = GetCaret().GetCaretLogicPos();
	if( pcOpeBlk ){	/* アンドゥ・リドゥの実行中か */
		/* 操作の追加 */
		pcOpeBlk->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
				GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
			)
		);
	}

	CDeleteOpe* pcDeleteOpe = NULL;	// 編集操作要素 COpe
	if( pcOpeBlk ){
		pcDeleteOpe = new CDeleteOpe();
		pcDeleteOpe->m_ptCaretPos_PHY_Before = sDelRangeLogic.GetFrom();
		pcDeleteOpe->m_ptCaretPos_PHY_To = sDelRangeLogic.GetTo();
		pcDeleteOpe->m_ptCaretPos_PHY_After = pcDeleteOpe->m_ptCaretPos_PHY_Before;	// 操作後のキャレット位置
	}

	CNativeW pcMemDeleted;
	/*
	|| バッファサイズの調整
	*/
	if( 2000 < sDelRange.GetTo().y - sDelRange.GetFrom().y ){
		pcMemDeleted.AllocStringBuffer( 1024000 );
	}


	/* 文字列置換 */
	LayoutReplaceArg LRArg;
	LRArg.sDelRange    = sDelRange;		//!< 削除範囲レイアウト
	LRArg.pcmemDeleted = &pcMemDeleted;	//!< [out] 削除されたデータ
	LRArg.pInsData     = pInsData;		//!< 挿入するデータ
	LRArg.nInsDataLen  = nInsDataLen;	//!< 挿入するデータの長さ
	m_pcEditDoc->m_cLayoutMgr.ReplaceData_CLayoutMgr(
		&LRArg
	);

	//	Jan. 30, 2001 genta
	//	再描画の時点でファイル更新フラグが適切になっていないといけないので
	//	関数の末尾からここへ移動
	/* 状態遷移 */
	if( pcOpeBlk ){	/* アンドゥ・リドゥの実行中か */
		m_pcEditDoc->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	/* 現在の選択範囲を非選択状態に戻す */
	GetSelectionInfo().DisableSelectArea( bRedraw );

	/* 行番号表示に必要な幅を設定 */
	if( m_pcEditWnd->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
		/* キャレットの表示・更新 */
		GetCaret().ShowEditCaret();
	}
	else{
		/* 再描画 */
		if( bRedraw ){
			/* 再描画ヒント レイアウト行の増減 */
			//	Jan. 30, 2001 genta	貼り付けで行数が減る場合の考慮が抜けていた
			if( 0 != LRArg.nAddLineNum ){
				Call_OnPaint( PAINT_LINENUMBER | PAINT_BODY, true); // メモリＤＣを使用してちらつきのない再描画
			}
			else{
				PAINTSTRUCT ps;

				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();

				/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
				ps.rcPaint.top = GetTextArea().GetAreaTop() + (Int)(LRArg.nModLineFrom - GetTextArea().GetViewTopLine())* GetTextMetrics().GetHankakuDy();
				if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bWordWrap
				 || m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuHead	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuTail	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuRet	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.19 MIK
				{
					ps.rcPaint.top -= GetTextMetrics().GetHankakuDy();
				}
				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}
				ps.rcPaint.bottom = GetTextArea().GetAreaTop() + (Int)(LRArg.nModLineTo - GetTextArea().GetViewTopLine() + 1)* GetTextMetrics().GetHankakuDy();
				if( GetTextArea().GetAreaBottom() < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
				}

				HDC hdc = this->GetDC();
				OnPaint( hdc, &ps, TRUE );	/* メモリＤＣを使用してちらつきのない再描画 */
				this->ReleaseDC( hdc );
			}
		}
	}

	// 削除されたデータのコピー(NULL可能)
	if( pcmemCopyOfDeleted && 0 < pcMemDeleted.GetStringLength() ){
		*pcmemCopyOfDeleted = pcMemDeleted;
	}

	if( pcOpeBlk && 0 < pcMemDeleted.GetStringLength() ){
		pcDeleteOpe->m_nDataLen = pcMemDeleted.GetStringLength();	/* 操作に関連するデータのサイズ */
		pcDeleteOpe->m_pcmemData = pcMemDeleted;					/* 操作に関連するデータ */
		/* 操作の追加 */
		pcOpeBlk->AppendOpe( pcDeleteOpe );
	}
	pcMemDeleted.Clear();


	if( pcOpeBlk && 0 < nInsDataLen ){
		CInsertOpe* pcInsertOpe = new CInsertOpe();
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(sDelRange.GetFrom(), &pcInsertOpe->m_ptCaretPos_PHY_Before);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(LRArg.ptLayoutNew,   &pcInsertOpe->m_ptCaretPos_PHY_After);

		/* 操作の追加 */
		pcOpeBlk->AppendOpe( pcInsertOpe );
	}


	// 挿入直後位置へカーソルを移動
	GetCaret().MoveCursor(
		LRArg.ptLayoutNew,	// 挿入された部分の次の位置
		bRedraw
	);
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();


	if( pcOpeBlk ){	/* アンドゥ・リドゥの実行中か */
		/* 操作の追加 */
		pcOpeBlk->AppendOpe(
			new CMoveCaretOpe(
				ptCaretPos_PHY_Old,				// 操作後のキャレット位置
				GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
			)
		);
	}

	//	Jan. 30, 2001 genta
	//	ファイル全体の更新フラグが立っていないと各行の更新状態が表示されないので
	//	フラグ更新処理を再描画より前に移動する
}



/* C/C++スマートインデント処理 */
void CEditView::SmartIndent_CPP( wchar_t wcChar )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int			k;
	int			m;
	const wchar_t*	pLine2;
	CLogicInt	nLineLen2;
	int			nLevel;
	CLogicInt j;

	/* 調整によって置換される箇所 */
	CLogicRange sRangeA;
	sRangeA.Clear(-1);

	wchar_t*	pszData = NULL;
	CLogicInt	nDataLen;

	int			nWork;
	CDocLine*	pCDocLine = NULL;
	int	nCharChars;
	int			nSrcLen;
	wchar_t		pszSrc[1024];
	BOOL		bChange;

	int			nCaretPosX_PHY;

	CLogicPoint ptCP;

	if(wcChar==WCODE::CR || wcschr(L":{}()",wcChar)!=NULL){
		//次へ進む
	}else return;

	switch( wcChar ){
	case WCODE::CR:
	case L':':
	case L'}':
	case L')':
	case L'{':
	case L'(':

		nCaretPosX_PHY = GetCaret().GetCaretLogicPos().x;

		pLine = m_pcEditDoc->m_cDocLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2())->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			if( WCODE::CR != wcChar ){
				return;
			}
			/* 調整によって置換される箇所 */
			sRangeA.Set(CLogicPoint(0,GetCaret().GetCaretLogicPos().y));
		}else{
			pCDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() );


			//	nWorkに処理の基準桁位置を設定する
			if( WCODE::CR != wcChar ){
				nWork = nCaretPosX_PHY - 1;
			}else{
				/*
				|| CRが入力された時、カーソル直後の識別子をインデントする。
				|| カーソル直後の識別子が'}'や')'ならば
				|| '}'や')'が入力された時と同じ処理をする
				*/

				int i;
				for( i = nCaretPosX_PHY; i < nLineLen; i++ ){
					if( WCODE::TAB != pLine[i] && WCODE::SPACE != pLine[i] ){
						break;
					}
				}
				if( i < nLineLen ){
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
					if( 1 == nCharChars && ( pLine[i] == L')' || pLine[i] == L'}' ) ){
						wcChar = pLine[i];
					}
					nCaretPosX_PHY = i;
					nWork = nCaretPosX_PHY;
				}else{
					nWork = nCaretPosX_PHY;
				}
			}
			int i;
			for( i = 0; i < nWork; i++ ){
				if( WCODE::TAB != pLine[i] && WCODE::SPACE != pLine[i] ){
					break;
				}
			}
			if( i < nWork ){
				if( ( L':' == wcChar && WCODE::isHeadCppKeyword(&pLine[i]) )
					//	Sep. 18, 2002 かろと
					|| ( L'{' == wcChar && L'#' != pLine[i] )
					|| ( L'(' == wcChar && L'#' != pLine[i] )
				){

				}else{
					return;
				}
			}else{
				if( L':' == wcChar ){
					return;
				}
			}
			/* 調整によって置換される箇所 */
			sRangeA.SetFrom(CLogicPoint(0, GetCaret().GetCaretLogicPos().GetY2()));
			sRangeA.SetTo(CLogicPoint(i, GetCaret().GetCaretLogicPos().GetY2()));
		}


		/* 対応する括弧をさがす */
		nLevel = 0;	/* {}の入れ子レベル */


		nDataLen = CLogicInt(0);
		for( j = GetCaret().GetCaretLogicPos().GetY2(); j >= CLogicInt(0); --j ){
			pLine2 = m_pcEditDoc->m_cDocLineMgr.GetLine(j)->GetDocLineStrWithEOL(&nLineLen2);
			if( j == GetCaret().GetCaretLogicPos().y ){
				// 2005.10.11 ryoji EOF のみの行もスマートインデントの対象にする
				if( NULL == pLine2 ){
					if( GetCaret().GetCaretLogicPos().y == m_pcEditDoc->m_cDocLineMgr.GetLineCount() )
						continue;	// EOF のみの行
					break;
				}
				nCharChars = CLogicInt(&pLine2[nWork] - CNativeW::GetCharPrev( pLine2, nLineLen2, &pLine2[nWork] ));
				k = nWork - nCharChars;
			}else{
				if( NULL == pLine2 )
					break;
				nCharChars = CLogicInt(&pLine2[nLineLen2] - CNativeW::GetCharPrev( pLine2, nLineLen2, &pLine2[nLineLen2] ));
				k = nLineLen2 - nCharChars;
			}

			for( ; k >= 0; /*k--*/ ){
				if( 1 == nCharChars && ( L'}' == pLine2[k] || L')' == pLine2[k] )
				){
					if( 0 < k && L'\'' == pLine2[k - 1]
					 && nLineLen2 - 1 > k && L'\'' == pLine2[k + 1]
					){
//						MYTRACE_A( "▼[%ls]\n", pLine2 );
					}else{
						//同じ行の場合
						if( j == GetCaret().GetCaretLogicPos().y ){
							if( L'{' == wcChar && L'}' == pLine2[k] ){
								wcChar = L'}';
								nLevel--;	/* {}の入れ子レベル */
							}
							if( L'(' == wcChar && L')' == pLine2[k] ){
								wcChar = L')';
								nLevel--;	/* {}の入れ子レベル */
							}
						}

						nLevel++;	/* {}の入れ子レベル */
					}
				}
				if( 1 == nCharChars && ( L'{' == pLine2[k] || L'(' == pLine2[k] )
				){
					if( 0 < k && L'\'' == pLine2[k - 1]
					 && nLineLen2 - 1 > k && L'\'' == pLine2[k + 1]
					){
//						MYTRACE_A( "▼[%ls]\n", pLine2 );
					}else{
						//同じ行の場合
						if( j == GetCaret().GetCaretLogicPos().y ){
							if( L'{' == wcChar && L'{' == pLine2[k] ){
								return;
							}
							if( L'(' == wcChar && L'(' == pLine2[k] ){
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
				nCharChars = CLogicInt(&pLine2[k] - CNativeW::GetCharPrev( pLine2, nLineLen2, &pLine2[k] ));
				if( 0 == nCharChars ){
					nCharChars = CLogicInt(1);
				}
				k -= nCharChars;
			}
			if( k < 0 ){
				/* この行にはない */
				continue;
			}

			for( m = 0; m < nLineLen2; m++ ){
				if( WCODE::TAB != pLine2[m] && WCODE::SPACE != pLine2[m] ){
					break;
				}
			}


			nDataLen = CLogicInt(m);
			nCharChars = (m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bInsSpace)? (Int)m_pcEditDoc->m_cLayoutMgr.GetTabSpace(): 1;
			pszData = new wchar_t[nDataLen + nCharChars + 1];
			wmemcpy( pszData, pLine2, nDataLen );
			if( WCODE::CR  == wcChar || L'{' == wcChar || L'(' == wcChar ){
				// 2005.10.11 ryoji TABキーがSPACE挿入の設定なら追加インデントもSPACEにする
				//	既存文字列の右端の表示位置を求めた上で挿入するスペースの数を決定する
				if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bInsSpace ){	// SPACE挿入設定
					int i;
					i = m = 0;
					while( i < nDataLen ){
						nCharChars = CNativeW::GetSizeOfChar( pszData, nDataLen, i );
						if( nCharChars == 1 && WCODE::TAB == pszData[i] )
							m += (Int)m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace(CLayoutInt(m));
						else
							m += nCharChars;
						i += nCharChars;
					}
					nCharChars = (Int)m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace(CLayoutInt(m));
					for( int i = 0; i < nCharChars; i++ )
						pszData[nDataLen + i] = WCODE::SPACE;
					pszData[nDataLen + nCharChars] = L'\0';
					nDataLen += CLogicInt(nCharChars);
				}else{
					pszData[nDataLen] = WCODE::TAB;
					pszData[nDataLen + 1] = L'\0';
					++nDataLen;
				}
			}else{
				pszData[nDataLen] = L'\0';

			}
			break;
		}
		if( j < 0 ){
			/* 対応する括弧が見つからなかった */
			if( WCODE::CR == wcChar ){
				return;
			}else{
				nDataLen = CLogicInt(0);
				pszData = new wchar_t[nDataLen + 1];
				pszData[nDataLen] = L'\0';
			}
		}

		/* 調整後のカーソル位置を計算しておく */
		ptCP.x = nCaretPosX_PHY - sRangeA.GetTo().x + nDataLen;
		ptCP.y = GetCaret().GetCaretLogicPos().y;

		nSrcLen = sRangeA.GetTo().x - sRangeA.GetFrom().x;
		if( nSrcLen >= _countof( pszSrc ) - 1 ){
			//	Sep. 18, 2002 genta メモリリーク対策
			delete [] pszData;
			return;
		}
		if( NULL == pLine ){
			pszSrc[0] = L'\0';
		}else{
			wmemcpy( pszSrc, &pLine[sRangeA.GetFrom().x], nSrcLen );
			pszSrc[nSrcLen] = L'\0';
		}


		/* 調整によって置換される箇所 */
		CLayoutRange sRangeLayout;
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( sRangeA, &sRangeLayout );

		if( ( 0 == nDataLen && sRangeLayout.IsOne() )
		 || ( nDataLen == nSrcLen && 0 == wmemcmp( pszSrc, pszData, nDataLen ) )
		 ){
			bChange = FALSE;
		}else{
			bChange = TRUE;

			/* データ置換 削除&挿入にも使える */
			ReplaceData_CEditView(
				sRangeLayout,
				NULL,		/* 削除されたデータのコピー(NULL可能) */
				pszData,	/* 挿入するデータ */
				nDataLen,	/* 挿入するデータの長さ */
				true,
				m_bDoing_UndoRedo?NULL:m_pcOpeBlk
			);
		}


		/* カーソル位置調整 */
		CLayoutPoint ptCP_Layout;
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptCP, &ptCP_Layout );

		/* 選択エリアの先頭へカーソルを移動 */
		GetCaret().MoveCursor( ptCP_Layout, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();


		if( bChange && !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
					GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
				)
			);
		}
		break;
	}
	if( NULL != pszData ){
		delete [] pszData;
		pszData = NULL;
	}
	return;
}


// 2005.10.11 ryoji 前の行にある末尾の空白を削除
void CEditView::RTrimPrevLine( void )
{
	int			nCharChars;

	CLogicPoint ptCaretPos_PHY = GetCaret().GetCaretLogicPos();

	if( GetCaret().GetCaretLogicPos().y > 0 ){
		int				nLineLen;
		const wchar_t*	pLine = CDocReader(m_pcEditDoc->m_cDocLineMgr).GetLineStrWithoutEOL( GetCaret().GetCaretLogicPos().GetY2() - CLogicInt(1), &nLineLen );
		if( NULL != pLine && nLineLen > 0 ){
			int i=0;
			int j=0;
			while( i < nLineLen ){
				nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
				if( !WCODE::isBlank(pLine[i]) ){
					j = i + nCharChars;
				}
				i += nCharChars;
			}
			if( j < nLineLen ){
				CLayoutRange sRangeA;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( CLogicPoint(j, GetCaret().GetCaretLogicPos().y - 1), sRangeA.GetFromPointer() );
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( CLogicPoint(nLineLen, GetCaret().GetCaretLogicPos().y - 1), sRangeA.GetToPointer() );
				if( !( sRangeA.GetFrom().x >= sRangeA.GetTo().x && sRangeA.GetFrom().y == sRangeA.GetTo().y) ){
					ReplaceData_CEditView(
						sRangeA,
						NULL,		/* 削除されたデータのコピー(NULL可能) */
						NULL,		/* 挿入するデータ */
						CLogicInt(0),			/* 挿入するデータの長さ */
						true,
						m_bDoing_UndoRedo?NULL:m_pcOpeBlk
					);
					CLayoutPoint ptCP;
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptCaretPos_PHY, &ptCP );
					GetCaret().MoveCursor( ptCP, TRUE );

					if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
						/* 操作の追加 */
						m_pcOpeBlk->AppendOpe(
							new CMoveCaretOpe(
								GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
								GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
							)
						);
					}
				}
			}
		}
	}
}




