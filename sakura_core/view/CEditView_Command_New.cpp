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
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "view/CEditView.h"
#include "CSearchAgent.h"
#include "uiparts/CWaitCursor.h"
#include "charset/charcode.h"
#include "COpe.h" ///	2002/2/3 aroka from here
#include "COpeBlk.h" ///
#include "doc/CEditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理
#include "doc/CDocReader.h"
#include "doc/layout/CLayout.h"
#include "doc/logic/CDocLine.h"
#include "cmd/CViewCommander_inline.h"
#include "window/CEditWnd.h"
#include "dlg/CDlgCtrlCode.h"	//コントロールコードの入力(ダイアログ)
#include "dlg/CDlgFavorite.h"	//履歴の管理	//@@@ 2003.04.08 MIK
#include "debug/CRunningTimer.h"

using namespace std; // 2002/2/3 aroka



static void StringToOpeLineData(const wchar_t* pLineData, int nLineDataLen, COpeLineData& lineData, int opeSeq)
{
	int nBegin = 0;
	int i;
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
	for(i = 0; i < nLineDataLen; i++){
		if( WCODE::IsLineDelimiter(pLineData[i], bExtEol) ){
			if( i + 1 < nLineDataLen && WCODE::CR == pLineData[i] && WCODE::LF == pLineData[i + 1] ){
				i++;
			}
			CLineData tmp;
			lineData.push_back(tmp);
			CLineData& insertLine = lineData[lineData.size()-1];
			insertLine.cmemLine.SetString(&pLineData[nBegin], i - nBegin + 1);
			insertLine.nSeq = opeSeq;
			nBegin = i + 1;
		}
	}
	if( nBegin < i ){
		CLineData tmp;
		lineData.push_back(tmp);
		CLineData& insertLine = lineData[lineData.size()-1];
		insertLine.cmemLine.SetString(&pLineData[nBegin], nLineDataLen - nBegin);
		insertLine.nSeq = opeSeq;
	}
}


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
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::InsertData_CEditView" );
#endif

	//2007.10.18 kobake COpe処理をここにまとめる
	CInsertOpe* pcOpe = NULL;
	int opeSeq;
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new CInsertOpe();
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			ptInsertPos,
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		opeSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetNextSeq();
	}else{
		opeSeq = 0;
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
	bool			bHintPrev = false;	// 更新が前行からになる可能性があることを示唆する
	bool			bHintNext = false;	// 更新が次行からになる可能性があることを示唆する
	bool			bKinsoku;			// 禁則の有無
	const wchar_t*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( ptInsertPos.GetY2(), &nLineLen, &pcLayout );

	//禁則がある場合は1行前から再描画を行う	@@@ 2002.04.19 MIK
	bKinsoku = ( m_pTypeData->m_bWordWrap
			 || m_pTypeData->m_bKinsokuHead	//@@@ 2002.04.19 MIK
			 || m_pTypeData->m_bKinsokuTail	//@@@ 2002.04.19 MIK
			 || m_pTypeData->m_bKinsokuRet	//@@@ 2002.04.19 MIK
			 || m_pTypeData->m_bKinsokuKuto );	//@@@ 2002.04.19 MIK

	CLayoutInt	nLineAllColLen;
	CLogicInt	nIdxFrom = CLogicInt(0);
	CLayoutInt	nColumnFrom = ptInsertPos.GetX2();
	CNativeW	cMem(L"");
	COpeLineData insData;
	if( pLine ){
		// 更新が前行からになる可能性を調べる	// 2009.02.17 ryoji
		// ※折り返し行頭への句読点入力で前の行だけが更新される場合もある
		// ※挿入位置は行途中でも句読点入力＋ワードラップで前の文字列から続けて前行に回り込む場合もある
		if( pcLayout->GetLogicOffset() && bKinsoku ){	// 折り返しレイアウト行か？
			bHintPrev = true;	// 更新が前行からになる可能性がある
		}

		// 更新が次行からになる可能性を調べる	// 2009.02.17 ryoji
		// ※折り返し行末への文字入力や文字列貼り付けで現在行は更新されず次行以後が更新される場合もある
		// 指定された桁に対応する行のデータ内の位置を調べる
		nIdxFrom = LineColumnToIndex2( pcLayout, ptInsertPos.GetX2(), &nLineAllColLen );

		// 行終端より右に挿入しようとした
		if( nLineAllColLen > 0 ){
			int nSpWidth = GetTextMetrics().CalcTextWidth3(L" ", 1);
			// 終端直前から挿入位置まで空白を埋める為の処理
			// 行終端が何らかの改行コードか?
			if( EOL_NONE != pcLayout->GetLayoutEol() ){
				nIdxFrom = nLineLen - CLogicInt(1);
				cMem.AllocStringBuffer( (Int)(ptInsertPos.GetX2() - nLineAllColLen + 1)/ nSpWidth + nDataLen );
				for( int i = 0; i < ptInsertPos.GetX2() - nLineAllColLen + 1; i += nSpWidth ){
					cMem += L' ';
				}
				cMem.AppendString( pData, nDataLen );
			}
			else{
				nIdxFrom = nLineLen;
				cMem.AllocStringBuffer( (Int)(ptInsertPos.GetX2() - nLineAllColLen) / nSpWidth + nDataLen );
				for( int i = 0; i < ptInsertPos.GetX2() - nLineAllColLen; i += nSpWidth ){
					cMem += L' ';
				}
				cMem.AppendString( pData, nDataLen );
				// 1行多く更新する必要がある可能性がある
				bHintNext = true;
			}
			StringToOpeLineData( cMem.GetStringPtr(), cMem.GetStringLength(), insData, opeSeq );
			cMem.Clear();
			nColumnFrom = LineIndexToColumn( pcLayout, nIdxFrom );
		}
		else{
			StringToOpeLineData( pData, nDataLen, insData, opeSeq );
		}
	}
	else{
		// 更新が前行からになる可能性を調べる	// 2009.02.17 ryoji
		const CLayout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.GetBottomLayout();
		if( pcLayoutWk && pcLayoutWk->GetLayoutEol() == EOL_NONE && bKinsoku ){	// 折り返しレイアウト行か？（前行の終端で調査）
			bHintPrev = true;	// 更新が前行からになる可能性がある
		}
		if( 0 < ptInsertPos.GetX2() ){
			int nSpWidth = GetTextMetrics().CalcTextWidth3(L" ", 1);
			cMem.AllocStringBuffer( (Int)ptInsertPos.GetX2() / nSpWidth + nDataLen );
			for( CLayoutInt i = CLayoutInt(0); i < ptInsertPos.GetX2(); i+=nSpWidth ){
				cMem += L' ';
			}
			cMem.AppendString( pData, nDataLen );
			StringToOpeLineData( cMem.GetStringPtr(), cMem.GetStringLength(), insData, opeSeq );
			cMem.Clear();
		}else{
			StringToOpeLineData( pData, nDataLen, insData, opeSeq );
		}
		nColumnFrom = 0;
	}


	if( !m_bDoing_UndoRedo && pcOpe ){	// アンドゥ・リドゥの実行中か
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint(nColumnFrom, ptInsertPos.y),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
	}


	// 文字列挿入
	CLayoutInt	nModifyLayoutLinesOld=CLayoutInt(0);
	CLayoutInt	nInsLineNum;		/* 挿入によって増えたレイアウト行の数 */
	int	nInsSeq;
	{
		LayoutReplaceArg arg;
		arg.sDelRange.Set(CLayoutPoint(nColumnFrom, ptInsertPos.y));
		arg.pcmemDeleted = NULL;
		arg.pInsData = &insData;
		arg.nDelSeq = opeSeq;
		m_pcEditDoc->m_cLayoutMgr.ReplaceData_CLayoutMgr( &arg );
		nInsLineNum = arg.nAddLineNum;
		nModifyLayoutLinesOld = arg.nModLineTo - arg.nModLineFrom + 1;
		*pptNewPos = arg.ptLayoutNew;
		nInsSeq = arg.nInsSeq;
	}

	// 指定された行のデータ内の位置に対応する桁の位置を調べる
	const wchar_t*	pLine2;
	CLogicInt		nLineLen2;
	pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( pptNewPos->GetY2(), &nLineLen2, &pcLayout );
	if( pLine2 ){
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		pptNewPos->x = pptNewPos->GetX2(); //LineIndexToColumn( pcLayout, pptNewPos->GetX2() );
	}

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if( pptNewPos->x >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineLayout() ){
		if( m_pTypeData->m_bKinsokuRet
		 || m_pTypeData->m_bKinsokuKuto )	//@@@ 2002.04.16 MIK
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
			CLayoutInt nStartLine(ptInsertPos.y);
			// 2013.05.08 折り返し行でEOF直前で改行したときEOFが再描画されないバグの修正
			if( nModifyLayoutLinesOld < 1 ){
				nModifyLayoutLinesOld = CLayoutInt(1);
			}
			// 2011.12.26 正規表現キーワード・検索文字列などは、ロジック行頭までさかのぼって更新する必要がある
			{
				const CLayout* pcLayoutLineFirst = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( ptInsertPos.GetY2() );
				while( pcLayoutLineFirst && 0 != pcLayoutLineFirst->GetLogicOffset() ){
					pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
					if( bHintPrev ){
						bHintPrev = false;
					}
					nStartLine--;
					nModifyLayoutLinesOld++;
				}
			}
			CLayoutYInt nLayoutTop;
			CLayoutYInt nLayoutBottom;
			if( 0 != nInsLineNum ){
				// スクロールバーの状態を更新する
				AdjustScrollBars();

				// 描画開始行位置を調整する	// 2009.02.17 ryoji
				if( bHintPrev ){	// 更新が前行からになる可能性がある
					nStartLine--;
				}

				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();
				ps.rcPaint.top = GetTextArea().GenerateYPx(nStartLine);
				ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
				nLayoutTop = nStartLine;
				nLayoutBottom = CLayoutYInt(-1);
			}
			else{
				// 描画開始行位置と描画行数を調整する	// 2009.02.17 ryoji
				if( bHintPrev ){	// 更新が前行からになる可能性がある
					nStartLine--;
					nModifyLayoutLinesOld++;
				}
				if( bHintNext ){	// 更新が次行からになる可能性がある
					nModifyLayoutLinesOld++;
				}

	//			ps.rcPaint.left = GetTextArea().GetAreaLeft();
				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();

				// 2002.02.25 Mod By KK 次行 (ptInsertPos.y - GetTextArea().GetViewTopLine() - 1); => (ptInsertPos.y - GetTextArea().GetViewTopLine());
				//ps.rcPaint.top = GetTextArea().GetAreaTop() + GetTextMetrics().GetHankakuDy() * (ptInsertPos.y - GetTextArea().GetViewTopLine() - 1);
				ps.rcPaint.top = GetTextArea().GenerateYPx(nStartLine);
				ps.rcPaint.bottom = GetTextArea().GenerateYPx(nStartLine + nModifyLayoutLinesOld);
				nLayoutTop = nStartLine;
				nLayoutBottom = nStartLine + nModifyLayoutLinesOld;
			}
			HDC hdc = this->GetDC();
			OnPaint( hdc, &ps, FALSE );
			this->ReleaseDC( hdc );
			// 2014.07.16 他のビュー(ミニマップ)の再描画を抑制する
			if( 0 == nInsLineNum ){
				for(int i = 0; i < m_pcEditWnd->GetAllViewCount(); i++ ){
					CEditView* pcView = &m_pcEditWnd->GetView(i);
					if( pcView == this ){
						continue;
					}
					pcView->RedrawLines(nLayoutTop, nLayoutBottom);
				}
				m_pcEditWnd->GetMiniMap().RedrawLines(nLayoutTop, nLayoutBottom);
				if( !m_bDoing_UndoRedo && pcOpe ){
					GetDocument()->m_cDocEditor.m_nOpeBlkRedawCount++;
				}
			}
		}
	}

	//2007.10.18 kobake ここでCOpe処理をまとめる
	if( !m_bDoing_UndoRedo ){
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			*pptNewPos,
			&pcOpe->m_ptCaretPos_PHY_After
		);
		pcOpe->m_nOrgSeq = nInsSeq;

		// 操作の追加
		m_cCommander.GetOpeBlk()->AppendOpe( pcOpe );
	}
}


/*!	指定位置の指定長データ削除

	@param _ptCaretPos [in]  削除データの位置
	@param nDelLen [out] 削除データのサイズ
	@param pcMem [out]  削除したデータ(NULL可能)

	@date 2002/03/24 YAZAKI bUndo削除
	@date 2002/05/12 YAZAKI bRedraw, bRedraw2削除（常にFALSEだから）
	@date 2007/10/17 kobake (重要)pcMemの所有者が条件によりCOpeに移ったり移らなかったりする振る舞いは
	                        非常にややこしく混乱の元になるため、常に、pcMemの所有者は移さないように仕様変更。
*/
void CEditView::DeleteData2(
	const CLayoutPoint& _ptCaretPos,
	CLogicInt			nDelLen,
	CNativeW*			pcMem
)
{
#ifdef _DEBUG
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DeleteData(1)" );
#endif
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nIdxFrom;

	const CLayout* pcLayout;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( _ptCaretPos.GetY2(), &nLineLen, &pcLayout );
	if( NULL == pLine ){
		return;
	}
	nIdxFrom = LineColumnToIndex( pcLayout, _ptCaretPos.GetX2() );

	//2007.10.18 kobake COpeの生成をここにまとめる
	CDeleteOpe*	pcOpe = NULL;
	CLayoutInt columnFrom = LineIndexToColumn( pcLayout, nIdxFrom );
	CLayoutInt columnTo = LineIndexToColumn( pcLayout, nIdxFrom + nDelLen );
	if( !m_bDoing_UndoRedo ){
		pcOpe = new CDeleteOpe();
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint( columnFrom, _ptCaretPos.GetY2() ),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint( columnTo, _ptCaretPos.GetY2() ),
			&pcOpe->m_ptCaretPos_PHY_To
		);
	}
	COpeLineData memDeleted;
	COpeLineData* pmemDeleted = NULL;
	if( pcMem || pcOpe ){
		pmemDeleted = &memDeleted;
	}

	/* データ削除 */
	{
		LayoutReplaceArg arg;
		arg.sDelRange.SetFrom(_ptCaretPos);
		arg.sDelRange.SetTo(CLayoutPoint(columnTo, _ptCaretPos.GetY2()));
		arg.pcmemDeleted = pmemDeleted;
		arg.pInsData = NULL;
		arg.nDelSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetNextSeq();
		m_pcEditDoc->m_cLayoutMgr.ReplaceData_CLayoutMgr( &arg );
	}

	/* 選択エリアの先頭へカーソルを移動 */
	GetCaret().MoveCursor( _ptCaretPos, false );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();

	if( pcMem ){
		int size = (int)memDeleted.size();
		size_t bufSize = 0;
		for(int i = 0; i < size; i++){
			bufSize += memDeleted[i].cmemLine.GetStringLength();
		}
		pcMem->SetString(L"");
		pcMem->AllocStringBuffer( bufSize );
		for(int i = 0; i < size; i++){
			pcMem->AppendNativeData(memDeleted[i].cmemLine);
		}
	}
	//2007.10.18 kobake COpeの追加をここにまとめる
	if( pcOpe ){
		pcOpe->m_cOpeLineData.swap(memDeleted);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			_ptCaretPos,
			&pcOpe->m_ptCaretPos_PHY_After
		);
		// 操作の追加
		m_cCommander.GetOpeBlk()->AppendOpe( pcOpe );
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
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DeleteData(2)" );
#endif
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLayoutInt	nLineNum;
	CLogicInt	nCurIdx;
	CLogicInt	nIdxFrom;
	CLogicInt	nIdxTo;
	CLogicInt	nDelLen;
	CLogicInt	nDelLenNext;
	CLayoutRect		rcSel;
	const CLayout*	pcLayout;

	// テキストの存在しないエリアの削除は、選択範囲のキャンセルとカーソル移動のみとする	// 2008.08.05 ryoji
	if( GetSelectionInfo().IsTextSelected() ){		// テキストが選択されているか
		if( IsEmptyArea( GetSelectionInfo().m_sSelect.GetFrom(), GetSelectionInfo().m_sSelect.GetTo(), true, GetSelectionInfo().IsBoxSelecting() ) ){
			// カーソルを選択範囲の左上に移動
			GetCaret().MoveCursor(
				CLayoutPoint(
					GetSelectionInfo().m_sSelect.GetFrom().GetX2() < GetSelectionInfo().m_sSelect.GetTo().GetX2() ? GetSelectionInfo().m_sSelect.GetFrom().GetX2() : GetSelectionInfo().m_sSelect.GetTo().GetX2(),
					GetSelectionInfo().m_sSelect.GetFrom().GetY2() < GetSelectionInfo().m_sSelect.GetTo().GetY2() ? GetSelectionInfo().m_sSelect.GetFrom().GetY2() : GetSelectionInfo().m_sSelect.GetTo().GetY2()
				), bRedraw
			);
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			GetSelectionInfo().DisableSelectArea( bRedraw );
			return;
		}
	}else{
		if( IsEmptyArea( GetCaret().GetCaretLayoutPos() ) ){
			return;
		}
	}

	CLayoutPoint ptCaretPosOld = GetCaret().GetCaretLayoutPos();

	/* テキストが選択されているか */
	if( GetSelectionInfo().IsTextSelected() ){
		CWaitCursor cWaitCursor( this->GetHwnd() );  // 2002.02.05 hor
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			/* 操作の追加 */
			m_cCommander.GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}

		/* 矩形範囲選択中か */
		if( GetSelectionInfo().IsBoxSelecting() ){
			m_pcEditDoc->m_cDocEditor.SetModified(true,bRedraw);	//	2002/06/04 YAZAKI 矩形選択を削除したときに変更マークがつかない。

			SetDrawSwitch(false);	// 2002.01.25 hor
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
				nDelLenNext	= nIdxTo - nIdxFrom;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
				if( pLine ){
					using namespace WCODE;

					/* 指定された桁に対応する行のデータ内の位置を調べる */
					nIdxFrom = LineColumnToIndex( pcLayout, rcSel.left  );
					nIdxTo	 = LineColumnToIndex( pcLayout, rcSel.right );

					bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
					for( CLogicInt i = nIdxFrom; i <= nIdxTo; ++i ){
						if( WCODE::IsLineDelimiter(pLine[i], bExtEol) ){
							nIdxTo = i;
							break;
						}
					}
				}else{
					nIdxFrom = CLogicInt(0);
					nIdxTo	 = CLogicInt(0);
				}
				nDelLen	= nDelLenNext;
				if( nLineNum < rcSel.bottom && 0 < nDelLen ){
					// 指定位置の指定長データ削除
					DeleteData2(
						CLayoutPoint(rcSel.left, nLineNum + 1),
						nDelLen,
						NULL
					);
				}
			}
			SetDrawSwitch(true);	// 2002.01.25 hor

			/* 行番号表示に必要な幅を設定 */
			if ( m_pcEditWnd->DetectWidthOfLineNumberAreaAllPane( true ) ){
				/* キャレットの表示・更新 */
				GetCaret().ShowEditCaret();
			}
			if( bRedraw ){
				/* スクロールバーの状態を更新する */
				AdjustScrollBars();

				/* 再描画 */
				Call_OnPaint(PAINT_LINENUMBER | PAINT_BODY, false);
			}
			/* 選択エリアの先頭へカーソルを移動 */
			this->UpdateWindow();
			
			CLayoutPoint caretOld = CLayoutPoint(rcSel.left, rcSel.top);
			m_pcEditDoc->m_cLayoutMgr.GetLineStr( rcSel.top, &nLineLen, &pcLayout );
			if( rcSel.left <= pcLayout->CalcLayoutWidth( m_pcEditDoc->m_cLayoutMgr ) ){
				// EOLより左なら文字の単位にそろえる
				CLogicInt nIdxCaret = LineColumnToIndex( pcLayout, rcSel.left );
				caretOld.SetX( LineIndexToColumn( pcLayout, nIdxCaret ) );
			}
			GetCaret().MoveCursor( caretOld, bRedraw );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				CMoveCaretOpe*		pcOpe = new CMoveCaretOpe();
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
					ptCaretPosOld,
					&pcOpe->m_ptCaretPos_PHY_Before
				);

				pcOpe->m_ptCaretPos_PHY_After = GetCaret().GetCaretLogicPos();	// 操作後のキャレット位置
				/* 操作の追加 */
				m_cCommander.GetOpeBlk()->AppendOpe( pcOpe );
			}
		}else{
			/* データ置換 削除&挿入にも使える */
			ReplaceData_CEditView(
				GetSelectionInfo().m_sSelect,
				L"",					/* 挿入するデータ */
				CLogicInt(0),			/* 挿入するデータの長さ */
				bRedraw,
				m_bDoing_UndoRedo?NULL:m_cCommander.GetOpeBlk()
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
		nCurIdx = LineColumnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
//		MYTRACE( _T("nLineLen=%d nCurIdx=%d \n"), nLineLen, nCurIdx);
		if( nCurIdx == nLineLen && bLastLine ){	/* 全テキストの最後 */
			goto end_of_func;
//			return;
		}

		/* 指定された桁の文字のバイト数を調べる */
		CLogicInt	nNxtIdx;
		CLayoutInt	nNxtPos;
		bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
		if( WCODE::IsLineDelimiter(pLine[nCurIdx], bExtEol) ){
			/* 改行 */
			nNxtIdx = pcLayout->GetLengthWithoutEOL() + pcLayout->GetLayoutEol().GetLen();
			nNxtPos = GetCaret().GetCaretLayoutPos().GetX() + CLayoutInt(pcLayout->GetLayoutEol().GetLen()>0?1+m_pcEditDoc->m_cLayoutMgr.GetCharSpacing():0);
		}
		else{
			nNxtIdx = CLogicInt(CNativeW::GetCharNext( pLine, nLineLen, &pLine[nCurIdx] ) - pLine);
			// 指定された行のデータ内の位置に対応する桁の位置を調べる
			nNxtPos = LineIndexToColumn( pcLayout, nNxtIdx );
		}


		/* データ置換 削除&挿入にも使える */
		CLayoutRange sDelRange;
		sDelRange.SetFrom(GetCaret().GetCaretLayoutPos());
		sDelRange.SetTo(CLayoutPoint(nNxtPos,GetCaret().GetCaretLayoutPos().GetY()));
		CLogicRange sDelRangeLogic;
		sDelRangeLogic.SetFrom(GetCaret().GetCaretLogicPos());
		sDelRangeLogic.SetTo(CLogicPoint(nNxtIdx + pcLayout->GetLogicOffset(), GetCaret().GetCaretLogicPos().GetY()));
		ReplaceData_CEditView(
			sDelRange,
			L"",				/* 挿入するデータ */
			CLogicInt(0),		/* 挿入するデータの長さ */
			bRedraw,
			m_bDoing_UndoRedo?NULL:m_cCommander.GetOpeBlk(),
			false,
			&sDelRangeLogic
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
			GetCommander().Command_GOFILEEND( false );
		}
	}
end_of_func:;

	return;
}


void CEditView::ReplaceData_CEditView(
	const CLayoutRange&	sDelRange,			//!< [in]  削除範囲レイアウト単位
	const wchar_t*		pInsData,			//!< [in]  挿入するデータ
	CLogicInt			nInsDataLen,		//!< [in]  挿入するデータの長さ
	bool				bRedraw,
	COpeBlk*			pcOpeBlk,
	bool				bFastMode,
	const CLogicRange*	psDelRangeLogicFast
)
{
	int opeSeq;
	if( !m_bDoing_UndoRedo ){
		opeSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetNextSeq();
	}else{
		opeSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetCurrentPointer();
	}
	if( 0 == nInsDataLen ){
		ReplaceData_CEditView3(sDelRange, NULL, NULL, bRedraw, pcOpeBlk, opeSeq, NULL, bFastMode, psDelRangeLogicFast);
	}else{
		COpeLineData insData;
		StringToOpeLineData(pInsData, nInsDataLen, insData, opeSeq);
		ReplaceData_CEditView3(sDelRange, NULL, &insData, bRedraw, pcOpeBlk, opeSeq, NULL, bFastMode, psDelRangeLogicFast);
	}
}

void CEditView::ReplaceData_CEditView2(
	const CLogicRange&	sDelRange,			// 削除範囲。ロジック単位。
	const wchar_t*		pInsData,			// 挿入するデータ
	CLogicInt			nInsDataLen,		// 挿入するデータの長さ
	bool				bRedraw,
	COpeBlk*			pcOpeBlk,
	bool				bFastMode
)
{
	CLayoutRange sDelRangeLayout;
	if( !bFastMode ){
		this->m_pcEditDoc->m_cLayoutMgr.LogicToLayout(sDelRange,&sDelRangeLayout);
	}
	ReplaceData_CEditView(sDelRangeLayout,pInsData,nInsDataLen,bRedraw,pcOpeBlk,bFastMode,&sDelRange);
}




/* データ置換 削除&挿入にも使える */
// Jun 23, 2000 genta 変数名を書き換え忘れていたのを修正
// Jun. 1, 2000 genta DeleteDataから移動した
bool CEditView::ReplaceData_CEditView3(
	CLayoutRange	sDelRange,			//!< [in]  削除範囲レイアウト単位
	COpeLineData*	pcmemCopyOfDeleted,	//!< [out] 削除されたデータのコピー(NULL可能)
	COpeLineData*	pInsData,			//!< [in]  挿入するデータ
	bool			bRedraw,
	COpeBlk*		pcOpeBlk,
	int				nDelSeq,
	int*			pnInsSeq,
	bool			bFastMode,			//!< [in] CDocLineMgrを更新しない,行末チェックを省略する。bRedraw==falseの必要あり
	const CLogicRange*	psDelRangeLogicFast
)
{
	assert( (bFastMode && bRedraw == false) || (!bFastMode) ); // bFastModeのときは bReadraw == false
	bool bUpdateAll = true;

	bool bDelRangeUpdate = false;
	{
		//	May. 29, 2000 genta
		//	From Here
		//	行の後ろが選択されていたときの不具合を回避するため，
		//	選択領域から行末以降の部分を取り除く．

		if( !bFastMode ){
			//	先頭
			const CLayout*	pcLayout;
			CLogicInt		len;
			const wchar_t*	line = NULL;
			line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sDelRange.GetFrom().GetY2(), &len, &pcLayout );
			if( line ){
				CLogicInt pos = LineColumnToIndex( pcLayout, sDelRange.GetFrom().GetX2() );
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
						tmp.x = CLayoutInt(0);
						sDelRange.Set(tmp);
					}
					else {
						sDelRange.GetFromPointer()->y++;
						sDelRange.SetFromX(CLayoutInt(0));
					}
					bDelRangeUpdate = true;
				}
			}

			//	末尾
			line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sDelRange.GetTo().GetY2(), &len, &pcLayout );
			if( line ){
				CLayoutInt p = LineIndexToColumn( pcLayout, len );

				if( sDelRange.GetTo().x > p ){
					sDelRange.SetToX( p );
					bDelRangeUpdate = true;
				}
			}
		}
		//	To Here
	}

	//削除範囲ロジック単位 sDelRange -> sDelRangeLogic
	CLogicRange sDelRangeLogic;
	if( !bDelRangeUpdate && psDelRangeLogicFast ){
		sDelRangeLogic = *psDelRangeLogicFast;
	}else{
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			sDelRange,
			&sDelRangeLogic
		);
	}


	CLogicPoint		ptCaretPos_PHY_Old;

	ptCaretPos_PHY_Old = GetCaret().GetCaretLogicPos();
	if( pcOpeBlk ){	/* アンドゥ・リドゥの実行中か */
		/* 操作の追加 */
		if( sDelRangeLogic.GetFrom() != GetCaret().GetCaretLogicPos() ){
			pcOpeBlk->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
	}

	CReplaceOpe* pcReplaceOpe = NULL;	// 編集操作要素 COpe
	if( pcOpeBlk ){
		pcReplaceOpe = new CReplaceOpe();
		pcReplaceOpe->m_ptCaretPos_PHY_Before = sDelRangeLogic.GetFrom();
		pcReplaceOpe->m_ptCaretPos_PHY_To = sDelRangeLogic.GetTo();
		pcReplaceOpe->m_ptCaretPos_PHY_After = pcReplaceOpe->m_ptCaretPos_PHY_Before;	// 操作後のキャレット位置
	}

	COpeLineData* pcMemDeleted = NULL;
	COpeLineData opeData;
	if( pcOpeBlk || pcmemCopyOfDeleted ){
		pcMemDeleted = &opeData;
	}


	/* 現在の選択範囲を非選択状態に戻す */
	// 2009.07.18 ryoji 置換後→置換前に位置を変更（置換後だと反転が不正になって汚い Wiki BugReport/43）
	GetSelectionInfo().DisableSelectArea( bRedraw );

	/* 文字列置換 */
	LayoutReplaceArg LRArg;
	DocLineReplaceArg DLRArg;
	if( bFastMode ){
		DLRArg.sDelRange = sDelRangeLogic;
		DLRArg.pcmemDeleted = pcMemDeleted;
		DLRArg.pInsData = pInsData;
		DLRArg.nDelSeq = nDelSeq;
		// DLRArg.ptNewPos;
		CSearchAgent(&GetDocument()->m_cDocLineMgr).ReplaceData( &DLRArg );
	}else{
		LRArg.sDelRange    = sDelRange;		//!< 削除範囲レイアウト
		LRArg.pcmemDeleted = pcMemDeleted;	//!< [out] 削除されたデータ
		LRArg.pInsData     = pInsData;		//!< 挿入するデータ
		LRArg.nDelSeq      = nDelSeq;	//!< 挿入するデータの長さ
		m_pcEditDoc->m_cLayoutMgr.ReplaceData_CLayoutMgr( &LRArg );
	}

	//	Jan. 30, 2001 genta
	//	再描画の時点でファイル更新フラグが適切になっていないといけないので
	//	関数の末尾からここへ移動
	/* 状態遷移 */
	if( pcOpeBlk ){	/* アンドゥ・リドゥの実行中か */
		m_pcEditDoc->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

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
				Call_OnPaint( PAINT_LINENUMBER | PAINT_BODY, false);
			}
			else{
				// 文書末が改行なし→ありに変化したら				// 2009.11.11 ryoji
				// EOFのみ行が追加になるので、1行余分に描画する。
				// （文書末が改行あり→なしに変化する場合の末尾EOF消去は描画関数側で行われる）
				int nAddLine = ( LRArg.ptLayoutNew.GetY2() > LRArg.sDelRange.GetTo().GetY2() )? 1: 0;

				PAINTSTRUCT ps;

				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();

				/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
				ps.rcPaint.top = GetTextArea().GenerateYPx(LRArg.nModLineFrom);
				// 2011.12.26 正規表現キーワード・検索文字列などは、ロジック行頭までさかのぼって更新する必要がある
				{
					const CLayout* pcLayoutLineFirst = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( LRArg.nModLineFrom );
					while( pcLayoutLineFirst && 0 != pcLayoutLineFirst->GetLogicOffset() ){
						pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
						ps.rcPaint.top -= GetTextMetrics().GetHankakuDy();
						if( ps.rcPaint.top < 0 ){
							break;
						}
					}
				}
				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}
				ps.rcPaint.bottom = GetTextArea().GenerateYPx(LRArg.nModLineTo + 1 + nAddLine);
				if( GetTextArea().GetAreaBottom() < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
				}

				HDC hdc = this->GetDC();
				OnPaint( hdc, &ps, FALSE );
				this->ReleaseDC( hdc );

				CLayoutYInt nLayoutTop = LRArg.nModLineFrom;
				CLayoutYInt nLayoutBottom = LRArg.nModLineTo + 1 + nAddLine;
				for(int i = 0; i < m_pcEditWnd->GetAllViewCount(); i++ ){
					CEditView* pcView = &m_pcEditWnd->GetView(i);
					if( pcView == this ){
						continue;
					}
					pcView->RedrawLines(nLayoutTop, nLayoutBottom);
				}
				m_pcEditWnd->GetMiniMap().RedrawLines(nLayoutTop, nLayoutBottom);
				if( !m_bDoing_UndoRedo && pcOpeBlk ){
					GetDocument()->m_cDocEditor.m_nOpeBlkRedawCount++;
				}
				bUpdateAll = false;
			}
		}
	}

	// 削除されたデータのコピー(NULL可能)
	if( pcMemDeleted && 0 < pcMemDeleted->size() ){
		if( pcmemCopyOfDeleted ){
			if( pcOpeBlk ){
				pcReplaceOpe->m_pcmemDataDel = *pcMemDeleted;
			}
			pcmemCopyOfDeleted->swap(*pcMemDeleted);
		}else if( pcOpeBlk ){
			pcReplaceOpe->m_pcmemDataDel.swap(*pcMemDeleted);
		}
	}

	if( pcOpeBlk ){
		if( bFastMode ){
			pcReplaceOpe->m_ptCaretPos_PHY_After = DLRArg.ptNewPos;
			pcReplaceOpe->m_nOrgInsSeq = DLRArg.nInsSeq;
		}else{
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(LRArg.ptLayoutNew,   &pcReplaceOpe->m_ptCaretPos_PHY_After);
			pcReplaceOpe->m_nOrgInsSeq = LRArg.nInsSeq;
		}
		/* 操作の追加 */
		pcOpeBlk->AppendOpe( pcReplaceOpe );
	}

	// 挿入直後位置へカーソルを移動
	if( bFastMode ){
		GetCaret().MoveCursorFastMode(DLRArg.ptNewPos);
	}else{
		GetCaret().MoveCursor(
			LRArg.ptLayoutNew,	// 挿入された部分の次の位置
			bRedraw
		);
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	}

// 2013.06.29 CMoveCaretOpeは不要。ReplaceOpeのみにする
	if( pnInsSeq ){
		if( bFastMode ){
			*pnInsSeq = DLRArg.nInsSeq;
		}else{
			*pnInsSeq = LRArg.nInsSeq;
		}
	}

	//	Jan. 30, 2001 genta
	//	ファイル全体の更新フラグが立っていないと各行の更新状態が表示されないので
	//	フラグ更新処理を再描画より前に移動する
	return  bUpdateAll;
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
				if( !WCODE::IsBlank(pLine[i]) ){
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
						NULL,		/* 挿入するデータ */
						CLogicInt(0),			/* 挿入するデータの長さ */
						true,
						m_bDoing_UndoRedo?NULL:m_cCommander.GetOpeBlk()
					);
					CLayoutPoint ptCP;
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptCaretPos_PHY, &ptCP );
					GetCaret().MoveCursor( ptCP, true );

					if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
						/* 操作の追加 */
						m_cCommander.GetOpeBlk()->AppendOpe(
							new CMoveCaretOpe(
								GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
							)
						);
					}
				}
			}
		}
	}
}





