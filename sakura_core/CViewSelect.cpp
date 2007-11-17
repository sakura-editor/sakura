#include "stdafx.h"
#include "CViewSelect.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "CMemoryIterator.h"
#include "CLayout.h"
#include "CEditWnd.h"

CViewSelect::CViewSelect(CEditView* pcEditView)
: m_pcEditView(pcEditView)
{
	m_bSelectingLock   = false;	// 選択状態のロック
	m_bBeginSelect     = false;		// 範囲選択中
	m_bBeginBoxSelect  = false;	// 矩形範囲選択中
	m_bBeginLineSelect = false;	// 行単位選択中
	m_bBeginWordSelect = false;	// 単語単位選択中

	m_sSelectBgn.Clear(-1); // 範囲選択(原点)
	m_sSelect   .Clear(-1); // 範囲選択
	m_sSelectOld.Clear(0);  // 範囲選択(Old)
}

void CViewSelect::CopySelectStatus(CViewSelect* pSelect) const
{
	pSelect->m_bSelectingLock		= m_bSelectingLock;		/* 選択状態のロック */
	pSelect->m_bBeginSelect			= m_bBeginSelect;		/* 範囲選択中 */
	pSelect->m_bBeginBoxSelect		= m_bBeginBoxSelect;	/* 矩形範囲選択中 */

	pSelect->m_sSelectBgn			= m_sSelectBgn;			//範囲選択(原点)
	pSelect->m_sSelect				= m_sSelect;			//範囲選択
	pSelect->m_sSelectOld			= m_sSelectOld;			//範囲選択

	pSelect->m_ptMouseRollPosOld	= m_ptMouseRollPosOld;	// マウス範囲選択前回位置(XY座標)
}

//! 現在のカーソル位置から選択を開始する
void CViewSelect::BeginSelectArea()
{
	const CEditView* pView=GetEditView();

	m_sSelectBgn.Set(pView->GetCaret().GetCaretLayoutPos()); //範囲選択(原点)
	m_sSelect.   Set(pView->GetCaret().GetCaretLayoutPos()); //範囲選択
}


// 現在の選択範囲を非選択状態に戻す
void CViewSelect::DisableSelectArea( bool bDraw )
{
	const CEditView* pView=GetEditView();
	CEditView* pView2=GetEditView();

	m_sSelectOld = m_sSelect;		//範囲選択(Old)
	m_sSelect.Clear(-1);

	if( bDraw ){
		DrawSelectArea();
		m_bDrawSelectArea = false;	// 02/12/13 ai
	}

	m_bSelectingLock	 = false;	// 選択状態のロック
	m_sSelectOld.Clear(0);			// 範囲選択(Old)
	m_bBeginBoxSelect = false;		// 矩形範囲選択中
	m_bBeginLineSelect = false;		// 行単位選択中
	m_bBeginWordSelect = false;		// 単語単位選択中

	// 2002.02.16 hor 直前のカーソル位置をリセット
	pView2->GetCaret().m_nCaretPosX_Prev=pView->GetCaret().GetCaretLayoutPos().GetX();

	// カーソル行アンダーラインのON
	pView2->GetCaret().m_cUnderLine.CaretUnderLineON( bDraw );
}



// 現在のカーソル位置によって選択範囲を変更
void CViewSelect::ChangeSelectAreaByCurrentCursor( const CLayoutPoint& ptCaretPos )
{
	m_sSelectOld=m_sSelect; // 範囲選択(Old)

	//	2002/04/08 YAZAKI コードの重複を排除
	ChangeSelectAreaByCurrentCursorTEST(
		ptCaretPos,
		&m_sSelect
	);

	// 選択領域の描画
	DrawSelectArea();
}


// 現在のカーソル位置によって選択範囲を変更(テストのみ)
void CViewSelect::ChangeSelectAreaByCurrentCursorTEST(
	const CLayoutPoint& ptCaretPos,
	CLayoutRange* pSelect
)
{
	const CEditView* pView=GetEditView();
	CEditView* pView2=GetEditView();

	if(m_sSelectBgn.GetFrom()==m_sSelectBgn.GetTo()){
		if( ptCaretPos==m_sSelectBgn.GetFrom() ){
			// 選択解除
			pSelect->Clear(-1);
		}
		else if( PointCompare(ptCaretPos,m_sSelectBgn.GetFrom() ) < 0 ){ //キャレット位置がm_sSelectBgnのfromより小さかったら
			 pSelect->SetFrom(ptCaretPos);
			 pSelect->SetTo(m_sSelectBgn.GetFrom());
		}
		else{
			pSelect->SetFrom(m_sSelectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}else{
		// 常時選択範囲の範囲内
		// キャレット位置が m_sSelectBgn の from以上で、toより小さい場合
		if( PointCompare(ptCaretPos,m_sSelectBgn.GetFrom()) >= 0 && PointCompare(ptCaretPos,m_sSelectBgn.GetTo()) < 0 ){
			pSelect->SetFrom(m_sSelectBgn.GetFrom());
			if ( ptCaretPos==m_sSelectBgn.GetFrom() ){
				pSelect->SetTo(m_sSelectBgn.GetTo());
			}
			else {
				pSelect->SetTo(ptCaretPos);
			}
		}
		//キャレット位置がm_sSelectBgnのfromより小さかったら
		else if( PointCompare(ptCaretPos,m_sSelectBgn.GetFrom()) < 0 ){
			// 常時選択範囲の前方向
			pSelect->SetFrom(ptCaretPos);
			pSelect->SetTo(m_sSelectBgn.GetTo());
		}else{
			// 常時選択範囲の後ろ方向
			pSelect->SetFrom(m_sSelectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}
}



/*! 選択領域の描画

	@date 2006.10.01 Moca 重複コード削除．矩形作画改善．
*/
void CViewSelect::DrawSelectArea() const
{
	const CEditView* pView=GetEditView();

	if( !pView->GetDrawSwitch() ){
		return;
	}

	CLayoutRange sRangeA;

	CLayoutInt			nLineNum;

	m_bDrawSelectArea = true;

	// 2006.10.01 Moca 重複コード統合
	HDC         hdc = ::GetDC( pView->m_hWnd );
	HBRUSH      hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
	HBRUSH      hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
	int         nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );

//	MYTRACE_A( "DrawSelectArea()  m_bBeginBoxSelect=%ls\n", m_bBeginBoxSelect?"TRUE":"FALSE" );
	if( IsBoxSelecting() ){		// 矩形範囲選択中
		// 2001.12.21 hor 矩形エリアにEOFがある場合、RGN_XORで結合すると
		// EOF以降のエリアも反転してしまうので、この場合はRedrawを使う
		// 2002.02.16 hor ちらつきを抑止するためEOF以降のエリアが反転したらもう一度反転して元に戻すことにする
		//if((GetTextArea().GetViewTopLine()+m_nViewRowNum+1>=m_pcEditDoc->m_cLayoutMgr.GetLineCount()) &&
		//   (m_sSelect.GetTo().y+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ||
		//	m_sSelectOld.GetTo().y+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) ) {
		//	Redraw();
		//	return;
		//}

		const int nCharWidth = pView->GetTextMetrics().GetHankakuDx();
		const int nCharHeight = pView->GetTextMetrics().GetHankakuDy();


		// 2点を対角とする矩形を求める
		CLayoutRect  rcOld;
		TwoPointToRect(
			&rcOld,
			m_sSelectOld.GetFrom(),	// 範囲選択開始
			m_sSelectOld.GetTo()	// 範囲選択終了
		);
		rcOld.left   = max(rcOld.left  , pView->GetTextArea().GetViewLeftCol()  );
		rcOld.right  = max(rcOld.right , pView->GetTextArea().GetViewLeftCol()  );
		rcOld.right  = min(rcOld.right , pView->GetTextArea().GetRightCol() + 1 );
		rcOld.top    = max(rcOld.top   , pView->GetTextArea().GetViewTopLine()  );
		rcOld.bottom = min(rcOld.bottom, pView->GetTextArea().GetBottomLine()   );

		RECT rcOld2;
		rcOld2.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcOld.left  * nCharWidth;
		rcOld2.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcOld.right * nCharWidth;
		rcOld2.top		= (Int)( rcOld.top - pView->GetTextArea().GetViewTopLine() ) * nCharHeight + pView->GetTextArea().GetAreaTop();
		rcOld2.bottom	= (Int)( rcOld.bottom + 1 - pView->GetTextArea().GetViewTopLine() ) * nCharHeight + pView->GetTextArea().GetAreaTop();
		HRGN hrgnOld = ::CreateRectRgnIndirect( &rcOld2 );

		// 2点を対角とする矩形を求める
		CLayoutRect  rcNew;
		TwoPointToRect(
			&rcNew,
			m_sSelect.GetFrom(),	// 範囲選択開始
			m_sSelect.GetTo()		// 範囲選択終了
		);
		rcNew.left   = max(rcNew.left  , pView->GetTextArea().GetViewLeftCol() );
		rcNew.right  = max(rcNew.right , pView->GetTextArea().GetViewLeftCol() );
		rcNew.right  = min(rcNew.right , pView->GetTextArea().GetRightCol() + 1);
		rcNew.top    = max(rcNew.top   , pView->GetTextArea().GetViewTopLine() );
		rcNew.bottom = min(rcNew.bottom, pView->GetTextArea().GetBottomLine()  );

		RECT rcNew2;
		rcNew2.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcNew.left  * nCharWidth;
		rcNew2.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcNew.right * nCharWidth;
		rcNew2.top		= (Int)(rcNew.top - pView->GetTextArea().GetViewTopLine()) * nCharHeight + pView->GetTextArea().GetAreaTop();
		rcNew2.bottom	= (Int)(rcNew.bottom + 1 - pView->GetTextArea().GetViewTopLine()) * nCharHeight + pView->GetTextArea().GetAreaTop();

		HRGN hrgnNew = ::CreateRectRgnIndirect( &rcNew2 );

		// 矩形作画。
		// ::CombineRgn()の結果を受け取るために、適当なリージョンを作る
		HRGN hrgnDraw = ::CreateRectRgnIndirect( &rcNew2 );
		{
			// 旧選択矩形と新選択矩形のリージョンを結合し､ 重なりあう部分だけを除去します
			if( NULLREGION != ::CombineRgn( hrgnDraw, hrgnOld, hrgnNew, RGN_XOR ) ){

				// 2002.02.16 hor
				// 結合後のエリアにEOFが含まれる場合はEOF以降の部分を除去します
				// 2006.10.01 Moca リーソースリークを修正したら、チラつくようになったため、
				// 抑えるために EOF以降をリージョンから削除して1度の作画にする

				// 2006.10.01 Moca Start EOF位置計算をGetEndLayoutPosに書き換え。
				CLayoutPoint ptLast;
				pView->m_pcEditDoc->m_cLayoutMgr.GetEndLayoutPos( &ptLast );
				// 2006.10.01 Moca End
				if(m_sSelect.GetFrom().y>=ptLast.y || m_sSelect.GetTo().y>=ptLast.y ||
					m_sSelectOld.GetFrom().y>=ptLast.y || m_sSelectOld.GetTo().y>=ptLast.y){
					//	Jan. 24, 2004 genta nLastLenは物理桁なので変換必要
					//	最終行にTABが入っていると反転範囲が不足する．
					//	2006.10.01 Moca GetEndLayoutPosで処理するためColumnToIndexは不要に。
					RECT rcNew;
					rcNew.left   = pView->GetTextArea().GetAreaLeft() + (Int)(pView->GetTextArea().GetViewLeftCol() + ptLast.x) * nCharWidth;
					rcNew.right  = pView->GetTextArea().GetAreaRight();
					rcNew.top    = (Int)(ptLast.y - pView->GetTextArea().GetViewTopLine()) * nCharHeight + pView->GetTextArea().GetAreaTop();
					rcNew.bottom = rcNew.top + nCharHeight;
					
					// 2006.10.01 Moca GDI(リージョン)リソースリーク修正
					HRGN hrgnEOFNew = ::CreateRectRgnIndirect( &rcNew );
					::CombineRgn( hrgnDraw, hrgnDraw, hrgnEOFNew, RGN_DIFF );
					::DeleteObject( hrgnEOFNew );
				}
				::PaintRgn( hdc, hrgnDraw );
			}
		}

		//////////////////////////////////////////
		// デバッグ用 リージョン矩形のダンプ
//@@		TraceRgn( hrgnDraw );


		if( NULL != hrgnDraw ){
			::DeleteObject( hrgnDraw );
		}
		if( NULL != hrgnNew ){
			::DeleteObject( hrgnNew );
		}
		if( NULL != hrgnOld ){
			::DeleteObject( hrgnOld );
		}
	}else{

		// 現在描画されている範囲と始点が同じ
		if( m_sSelect == m_sSelectOld ){
			// 範囲が後方に拡大された
			if( PointCompare(m_sSelect.GetTo(),m_sSelectOld.GetTo()) > 0 ){
				sRangeA.SetFrom(m_sSelectOld.GetTo());
				sRangeA.SetTo  (m_sSelect.GetTo());
			}
			else{
				sRangeA.SetFrom(m_sSelect.GetTo());
				sRangeA.SetTo  (m_sSelectOld.GetTo());
			}
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine(	hdc, nLineNum, sRangeA);
				}
			}
		}
		else if( m_sSelect.GetTo() == m_sSelectOld.GetTo() ){
			// 範囲が前方に拡大された
			if(PointCompare(m_sSelect.GetFrom(),m_sSelectOld.GetFrom()) < 0){
				sRangeA.SetFrom(m_sSelect.GetFrom());
				sRangeA.SetTo  (m_sSelectOld.GetFrom());
			}
			else{
				sRangeA.SetFrom(m_sSelectOld.GetFrom());
				sRangeA.SetTo  (m_sSelect.GetFrom());
			}
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, sRangeA );
				}
			}
		}
		else{
			sRangeA = m_sSelectOld;
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, sRangeA );
				}
			}
			sRangeA = m_sSelect;
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, sRangeA );
				}
			}
		}
	}
	// 2006.10.01 Moca 重複コード統合
	::SetROP2( hdc, nROP_Old );
	::SelectObject( hdc, hBrushOld );
	::DeleteObject( hBrush );
	::ReleaseDC( pView->m_hWnd, hdc );
	//	Jul. 9, 2005 genta 選択領域の情報を表示
	PrintSelectionInfoMsg();
}




/*! 選択領域の中の指定行の描画

	複数行に渡る選択範囲のうち，nLineNumで指定された1行分だけを描画する．
	選択範囲は固定されたままnLineNumのみが必要行分変化しながら呼びだされる．

	@date 2006.03.29 Moca 3000桁制限を撤廃．
*/
void CViewSelect::DrawSelectAreaLine(
	HDC					hdc,		//!< [in] 描画領域のDevice Context Handle
	CLayoutInt			nLineNum,	//!< [in] 描画対象行(レイアウト行)
	const CLayoutRange&	sRange		//!< [in] 選択範囲(レイアウト単位)
) const
{
	const CEditView* pView=GetEditView();

	RECT			rcClip;
	CLayoutInt		nSelectFrom;	// 描画行の選択開始桁位置
	CLayoutInt		nSelectTo;		// 描画行の選択開始終了位置

	if( sRange.IsLineOne() ){
		nSelectFrom = sRange.GetFrom().x;
		nSelectTo	= sRange.GetTo().x;
	}
	else{
		// 2006.03.29 Moca 行末までの長さを求める位置を上からここに移動
		CLayoutInt nPosX = CLayoutInt(0);
		const CLayout* pcLayout = pView->m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
		CMemoryIterator it( pcLayout, pView->m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
				nPosX ++;
				break;
			}
			// 2006.03.28 Moca 画面外まで求めたら打ち切る
			if( it.getColumn() >pView->GetTextArea().GetRightCol() ){
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();
		
		if( nLineNum == sRange.GetFrom().y ){
			nSelectFrom = sRange.GetFrom().x;
			nSelectTo	= nPosX;
		}
		else if( nLineNum == sRange.GetTo().y ){
			nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
			nSelectTo	= sRange.GetTo().x;
		}
		else{
			nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
			nSelectTo	= nPosX;
		}
		// 2006.05.24 Mocaフリーカーソル選択(選択開始/終了行)で
		// To < From になることがある。必ず From < To になるように入れ替える。
		if( nSelectTo < nSelectFrom ){
			t_swap(nSelectFrom, nSelectTo);
		}
	}
	
	// 2006.03.28 Moca ウィンドウ幅が大きいと正しく反転しない問題を修正
	if( nSelectFrom < pView->GetTextArea().GetViewLeftCol() ){
		nSelectFrom = pView->GetTextArea().GetViewLeftCol();
	}
	int		nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int		nCharWidth = pView->GetTextMetrics().GetHankakuDx();
	rcClip.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)nSelectFrom * nCharWidth;
	rcClip.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)nSelectTo   * nCharWidth;
	rcClip.top		= (Int)(nLineNum - pView->GetTextArea().GetViewTopLine()) * nLineHeight + pView->GetTextArea().GetAreaTop();
	rcClip.bottom	= rcClip.top + nLineHeight;
	if( rcClip.right > pView->GetTextArea().GetAreaRight() ){
		rcClip.right = pView->GetTextArea().GetAreaRight();
	}
	//	必要なときだけ。
	if ( rcClip.right != rcClip.left ){
		pView->GetCaret().m_cUnderLine.CaretUnderLineOFF( TRUE );
		
		// 2006.03.28 Moca 表示域内のみ処理する
		if( nSelectFrom <=pView->GetTextArea().GetRightCol() && pView->GetTextArea().GetViewLeftCol() < nSelectTo ){
			HRGN hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
			::PaintRgn( hdc, hrgnDraw );
			::DeleteObject( hrgnDraw );
		}
	}
}



/*!	選択範囲情報メッセージの表示

	@author genta
	@date 2005.07.09 genta 新規作成
	@date 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策を追加
*/
void CViewSelect::PrintSelectionInfoMsg() const
{
	const CEditView* pView=GetEditView();

	//	出力されないなら計算を省略
	if( ! pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2IsEffective() )
		return;

	if( ! IsTextSelected() ){
		pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( _T("") );
		return;
	}

	TCHAR msg[128];
	//	From here 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策
	CLayoutInt nLineCount = pView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( m_sSelect.GetFrom().y >= nLineCount ){	// 先頭行が実在しない
		pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( _T("") );
		return;
	}

	CLayoutInt select_line;
	if( m_sSelect.GetTo().y >= nLineCount ){	// 最終行が実在しない
		select_line = nLineCount - m_sSelect.GetFrom().y + 1;
	}
	else {
		select_line = m_sSelect.GetTo().y - m_sSelect.GetFrom().y + 1;
	}
	
	//	To here 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策
	if( IsBoxSelecting() ){
		//	矩形の場合は幅と高さだけでごまかす
		CLayoutInt select_col = m_sSelect.GetFrom().x - m_sSelect.GetTo().x;
		if( select_col < 0 ){
			select_col = -select_col;
		}
		auto_sprintf( msg, _T("%d Columns * %d lines selected."),
			select_col, select_line );
			
	}
	else {
		//	通常の選択では選択範囲の中身を数える
		int select_sum = 0;	//	バイト数合計
		const wchar_t *pLine;	//	データを受け取る
		CLogicInt	nLineLen;		//	行の長さ
		const CLayout*	pcLayout;

		//	1行目
		pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.GetFrom().GetY2(), &nLineLen, &pcLayout );
		if( pLine ){
			//	1行だけ選択されている場合
			if( m_sSelect.IsLineOne() ){
				select_sum =
					pView->LineColmnToIndex( pcLayout, m_sSelect.GetTo().GetX2() )
					- pView->LineColmnToIndex( pcLayout, m_sSelect.GetFrom().GetX2() );
			}
			else {	//	2行以上選択されている場合
				select_sum =
					pcLayout->GetLengthWithoutEOL()
					+ pcLayout->m_cEol.GetLen()
					- pView->LineColmnToIndex( pcLayout, m_sSelect.GetFrom().GetX2() );

				//	GetSelectedDataと似ているが，先頭行と最終行は排除している
				//	Aug. 16, 2005 aroka nLineNumはfor以降でも使われるのでforの前で宣言する
				//	VC .NET以降でもMicrosoft拡張を有効にした標準動作はVC6と同じことに注意
				CLayoutInt nLineNum;
				for( nLineNum = m_sSelect.GetFrom().GetY2() + CLayoutInt(1);
					nLineNum < m_sSelect.GetTo().GetY2(); ++nLineNum ){
					pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
					//	2006.06.06 ryoji 指定行のデータが存在しない場合の対策
					if( NULL == pLine )
						break;
					select_sum += pcLayout->GetLengthWithoutEOL() + pcLayout->m_cEol.GetLen();
				}

				//	最終行の処理
				pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
				if( pLine ){
					int last_line_chars = pView->LineColmnToIndex( pcLayout, m_sSelect.GetTo().GetX2() );
					select_sum += last_line_chars;
					if( last_line_chars == 0 ){
						//	最終行の先頭にキャレットがある場合は
						//	その行を行数に含めない
						--select_line;
					}
				}
				else
				{
					//	最終行が空行なら
					//	その行を行数に含めない
					--select_line;
				}
			}
		}

#ifdef _DEBUG
		auto_sprintf( msg, _T("%d bytes (%d lines) selected. [%d:%d]-[%d:%d]"),
			select_sum, select_line,
			m_sSelect.GetFrom().x, m_sSelect.GetFrom().y,
			m_sSelect.GetTo().x, m_sSelect.GetTo().y );
#else
		auto_sprintf( msg, _T("%d bytes (%d lines) selected."), select_sum, select_line );
#endif
	}
	pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( msg );
}
