#include "StdAfx.h"
#include <limits.h>
#include "CViewSelect.h"
#include "CEditView.h"
#include "doc/CEditDoc.h"
#include "doc/layout/CLayout.h"
#include "mem/CMemoryIterator.h"
#include "window/CEditWnd.h"
#include "charset/CCodeBase.h"
#include "charset/CCodeFactory.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "types/CTypeSupport.h"

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
	m_bSelectAreaChanging = false;	// 選択範囲変更中
	m_nLastSelectedByteLen = 0;	// 前回選択時の選択バイト数
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
void CViewSelect::BeginSelectArea( const CLayoutPoint* po )
{
	const CEditView* pView=GetEditView();
	CLayoutPoint temp;
	if( NULL == po ){
		temp = pView->GetCaret().GetCaretLayoutPos();
		po = &temp;
	}
	m_sSelectBgn.Set(*po); //範囲選択(原点)
	m_sSelect.   Set(*po); //範囲選択
}


// 現在の選択範囲を非選択状態に戻す
void CViewSelect::DisableSelectArea( bool bDraw, bool bDrawBracketCursorLine )
{
	const CEditView* pView=GetEditView();
	CEditView* pView2=GetEditView();

	m_sSelectOld = m_sSelect;		//範囲選択(Old)
	m_sSelect.Clear(-1);
	m_bSelectingLock	 = false;	// 選択状態のロック

	if( bDraw ){
		DrawSelectArea( bDrawBracketCursorLine );
	}
	m_bDrawSelectArea = false;	// 02/12/13 ai // 2011.12.24 bDraw括弧内から移動

	m_sSelectOld.Clear(0);			// 範囲選択(Old)
	m_bBeginBoxSelect = false;		// 矩形範囲選択中
	m_bBeginLineSelect = false;		// 行単位選択中
	m_bBeginWordSelect = false;		// 単語単位選択中
	m_nLastSelectedByteLen = 0;		// 前回選択時の選択バイト数

	// 2002.02.16 hor 直前のカーソル位置をリセット
	pView2->GetCaret().m_nCaretPosX_Prev=pView->GetCaret().GetCaretLayoutPos().GetX();

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
	m_bSelectAreaChanging = true;
	DrawSelectArea(true);
	m_bSelectAreaChanging = false;
}


// 現在のカーソル位置によって選択範囲を変更(テストのみ)
void CViewSelect::ChangeSelectAreaByCurrentCursorTEST(
	const CLayoutPoint& ptCaretPos,
	CLayoutRange* pSelect
)
{
	if(m_sSelectBgn.GetFrom()==m_sSelectBgn.GetTo()){
		if( ptCaretPos==m_sSelectBgn.GetFrom() ){
			// 選択解除
			pSelect->Clear(-1);
			m_nLastSelectedByteLen = 0;		// 前回選択時の選択バイト数
		}
		else if( PointCompare(ptCaretPos,m_sSelectBgn.GetFrom() ) < 0 ){ //キャレット位置がm_sSelectBgnのfromより小さかったら
			 pSelect->SetFrom(ptCaretPos);
			 pSelect->SetTo(m_sSelectBgn.GetFrom());
		}
		else{
			pSelect->SetFrom(m_sSelectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}
	else{
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
		}
		else{
			// 常時選択範囲の後ろ方向
			pSelect->SetFrom(m_sSelectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}
}



/*! 選択領域の描画

	@date 2006.10.01 Moca 重複コード削除．矩形作画改善．
	@date 2007.09.09 Moca 互換BMPによる画面バッファ
		画面バッファが有効時、画面と互換BMPの両方の反転処理を行う。
*/
void CViewSelect::DrawSelectArea(bool bDrawBracketCursorLine)
{
	CEditView* pView=GetEditView();

	if( !pView->GetDrawSwitch() ){
		return;
	}
	m_bDrawSelectArea = true;
	
	bool bDispText = CTypeSupport(pView,COLORIDX_SELECT).IsDisp();
	if( bDispText ){
		if( m_sSelect != m_sSelectOld ){
			// 選択色表示の時は、WM_PAINT経由で作画
			const int nCharWidth = pView->GetTextMetrics().GetCharPxWidth();
			const CTextArea& area =  pView->GetTextArea();
			CLayoutRect rcOld; // CLayoutRect
			TwoPointToRect( &rcOld, m_sSelectOld.GetFrom(), m_sSelectOld.GetTo() );
			CLayoutRect rcNew; // CLayoutRect
			TwoPointToRect( &rcNew, m_sSelect.GetFrom(), m_sSelect.GetTo() );
			CLayoutRect rc; // CLayoutRect ただしtop,bottomだけ使う
			CLayoutInt drawLeft = CLayoutInt(0);
			CLayoutInt drawRight = CLayoutInt(-1);
			if( !m_sSelect.IsValid() ){
				rc.top    = rcOld.top;
				rc.bottom = rcOld.bottom;
			}else if( !m_sSelectOld.IsValid() ){
				rc.top    = rcNew.top;
				rc.bottom = rcNew.bottom;
			}else if(IsBoxSelecting() && 
				(m_sSelect.GetTo().x != m_sSelectOld.GetTo().x || m_sSelect.GetFrom().x != m_sSelectOld.GetFrom().x)){
				rc.UnionStrictRect(rcOld, rcNew);
			}else if(!IsBoxSelecting() && rcOld.top == rcNew.top && rcOld.bottom == rcNew.bottom){
				if(m_sSelect.GetFrom() == m_sSelectOld.GetFrom() && m_sSelect.GetTo().x != m_sSelectOld.GetTo().x){
					// GetToの行が対象
					rc.top = rc.bottom = m_sSelect.GetTo().GetY2();
					drawLeft  = t_min(m_sSelect.GetTo().x, m_sSelectOld.GetTo().x);
					drawRight = t_max(m_sSelect.GetTo().x, m_sSelectOld.GetTo().x)
						+ pView->GetTextMetrics().GetLayoutXDefault() + 4; // 改行コード幅分余分に取る
				}else if(m_sSelect.GetTo() == m_sSelectOld.GetTo() && m_sSelect.GetFrom().x != m_sSelectOld.GetFrom().x){
					// GetFromの行が対象
					rc.top = rc.bottom = m_sSelect.GetFrom().GetY2();
					drawLeft  = t_min(m_sSelectOld.GetFrom().x, m_sSelect.GetFrom().x);
					drawRight = t_max(m_sSelectOld.GetFrom().x, m_sSelect.GetFrom().x)
						+ pView->GetTextMetrics().GetLayoutXDefault() + 4; // 改行コード幅分余分に取る
				}else{
					rc.UnionStrictRect(rcOld, rcNew);
				}
			}else if(rcOld.top == rcNew.top){
				rc.top    = t_min(rcOld.bottom, rcNew.bottom);
				rc.bottom = t_max(rcOld.bottom, rcNew.bottom);
			}else if(rcOld.bottom == rcNew.bottom){
				rc.top    = t_min(rcOld.top, rcNew.top);
				rc.bottom = t_max(rcOld.top, rcNew.top);
			}else{
				rc.UnionStrictRect(rcOld, rcNew);
			}
			CMyRect rcPx;
			if( pView->IsBkBitmap() ||  drawRight == -1){
				// 背景表示のクリッピングが甘いので左右を指定しない
				rcPx.left   =  0;
				rcPx.right  = SHRT_MAX; 
			}else{
				rcPx.left   =  area.GetAreaLeft() + nCharWidth * (Int)(drawLeft - area.GetViewLeftCol());
				rcPx.right  = area.GetAreaLeft() + nCharWidth * (Int)(drawRight- area.GetViewLeftCol());
			}
			rcPx.top    = area.GenerateYPx(rc.top);
			rcPx.bottom = area.GenerateYPx(rc.bottom + 1);

			CMyRect rcArea;
			pView->GetTextArea().GenerateTextAreaRect(&rcArea);
			RECT rcUpdate;
			CEditView& view = *pView;
			if( ::IntersectRect(&rcUpdate, &rcPx, &rcArea) ){
				HDC hdc = view.GetDC();
				PAINTSTRUCT ps;
				ps.rcPaint = rcUpdate;
				// DrawSelectAreaLineでの下線OFFの代わり
				view.GetCaret().m_cUnderLine.CaretUnderLineOFF(true, false);
				view.GetCaret().m_cUnderLine.Lock();
				view.OnPaint(hdc, &ps, false);
				view.GetCaret().m_cUnderLine.UnLock();
				view.ReleaseDC( hdc );
			}
			// 2010.10.10 0幅選択(解除)状態での、カーソル位置ライン復帰(リージョン外)
			if( bDrawBracketCursorLine ){
				view.GetCaret().m_cUnderLine.CaretUnderLineON(true, false);
			}
		}
	}else{
		if( IsTextSelecting() && (!m_sSelectOld.IsValid() || m_sSelectOld.IsOne()) ){
			m_bDrawSelectArea = false;
			pView->DrawBracketPair( false );
			m_bDrawSelectArea = true;
		}
		HDC hdc = pView->GetDC();
		DrawSelectArea2( hdc );
		// 2011.12.02 選択解除状態での、カーソル位置ライン復帰
		if( bDrawBracketCursorLine ){
			pView->GetCaret().m_cUnderLine.CaretUnderLineON(true, false);
		}
		pView->ReleaseDC( hdc );
	}

	// 2011.12.02 選択解除状態になると対括弧強調ができなくなるバグ対策
	if( !IsTextSelecting() ){
		// ただし選択ロック中はここでは強調表示されない
		m_bDrawSelectArea = false;
		if( bDrawBracketCursorLine ){
			pView->SetBracketPairPos( true );
			pView->DrawBracketPair( true );
		}
	}

	//	Jul. 9, 2005 genta 選択領域の情報を表示
	PrintSelectionInfoMsg();
}

/*!
	反転用再作画処理本体
*/
void CViewSelect::DrawSelectArea2( HDC hdc ) const
{
	CEditView const * const pView = GetEditView();

	// 2006.10.01 Moca 重複コード統合
	HBRUSH      hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
	HBRUSH      hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
	int         nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	HBRUSH		hBrushCompatOld = 0;
	int			nROPCompatOld = 0;
	bool bCompatBMP = pView->m_hbmpCompatBMP && hdc != pView->m_hdcCompatDC;
	if( bCompatBMP ){
		hBrushCompatOld = (HBRUSH)::SelectObject( pView->m_hdcCompatDC, hBrush );
		nROPCompatOld = ::SetROP2( pView->m_hdcCompatDC, SELECTEDAREA_ROP2 );
	}
	// To Here 2007.09.09 Moca

//	MYTRACE( _T("DrawSelectArea()  m_bBeginBoxSelect=%hs\n", m_bBeginBoxSelect?"true":"false") );
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

		const int nCharWidth = pView->GetTextMetrics().GetCharPxWidth();
		const int nCharHeight = pView->GetTextMetrics().GetHankakuDy();


		// 2点を対角とする矩形を求める
		CLayoutRect  rcOld;
		TwoPointToRect(
			&rcOld,
			m_sSelectOld.GetFrom(),	// 範囲選択開始
			m_sSelectOld.GetTo()	// 範囲選択終了
		);
		rcOld.left   = t_max(rcOld.left  , pView->GetTextArea().GetViewLeftCol()  );
		rcOld.right  = t_max(rcOld.right , pView->GetTextArea().GetViewLeftCol()  );
		rcOld.right  = t_min(rcOld.right , pView->GetTextArea().GetRightCol() + 1 );
		rcOld.top    = t_max(rcOld.top   , pView->GetTextArea().GetViewTopLine()  );
		rcOld.bottom = t_max(rcOld.bottom, pView->GetTextArea().GetViewTopLine() - 1);	// 2010.11.02 ryoji 追加（画面上端よりも上にある矩形選択を解除するとルーラーが反転表示になる問題の修正）
		rcOld.bottom = t_min(rcOld.bottom, pView->GetTextArea().GetBottomLine()   );

		RECT rcOld2;
		rcOld2.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcOld.left  * nCharWidth;
		rcOld2.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcOld.right * nCharWidth;
		rcOld2.top		= pView->GetTextArea().GenerateYPx( rcOld.top );
		rcOld2.bottom	= pView->GetTextArea().GenerateYPx( rcOld.bottom + 1 );
		HRGN hrgnOld = ::CreateRectRgnIndirect( &rcOld2 );

		// 2点を対角とする矩形を求める
		CLayoutRect  rcNew;
		TwoPointToRect(
			&rcNew,
			m_sSelect.GetFrom(),	// 範囲選択開始
			m_sSelect.GetTo()		// 範囲選択終了
		);
		rcNew.left   = t_max(rcNew.left  , pView->GetTextArea().GetViewLeftCol() );
		rcNew.right  = t_max(rcNew.right , pView->GetTextArea().GetViewLeftCol() );
		rcNew.right  = t_min(rcNew.right , pView->GetTextArea().GetRightCol() + 1);
		rcNew.top    = t_max(rcNew.top   , pView->GetTextArea().GetViewTopLine() );
		rcNew.bottom = t_max(rcNew.bottom, pView->GetTextArea().GetViewTopLine() - 1);	// 2010.11.02 ryoji 追加（画面上端よりも上にある矩形選択を解除するとルーラーが反転表示になる問題の修正）
		rcNew.bottom = t_min(rcNew.bottom, pView->GetTextArea().GetBottomLine()  );

		RECT rcNew2;
		rcNew2.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcNew.left  * nCharWidth;
		rcNew2.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcNew.right * nCharWidth;
		rcNew2.top		= pView->GetTextArea().GenerateYPx(rcNew.top);
		rcNew2.bottom	= pView->GetTextArea().GenerateYPx(rcNew.bottom + 1);

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
				// 2011.12.26 EOFのぶら下がり行は反転し、EOFのみの行は反転しない
				const CLayout* pBottom = pView->m_pcEditDoc->m_cLayoutMgr.GetBottomLayout();
				if( pBottom && pBottom->GetLayoutEol() == EOL_NONE ){
					ptLast.x = 0;
					ptLast.y++;
				}
				if(m_sSelect.GetFrom().y>=ptLast.y || m_sSelect.GetTo().y>=ptLast.y ||
					m_sSelectOld.GetFrom().y>=ptLast.y || m_sSelectOld.GetTo().y>=ptLast.y){
					//	Jan. 24, 2004 genta nLastLenは物理桁なので変換必要
					//	最終行にTABが入っていると反転範囲が不足する．
					//	2006.10.01 Moca GetEndLayoutPosで処理するためColumnToIndexは不要に。
					RECT rcNew;
					rcNew.left   = pView->GetTextArea().GetAreaLeft() + (Int)(pView->GetTextArea().GetViewLeftCol() + ptLast.x) * nCharWidth;
					rcNew.right  = pView->GetTextArea().GetAreaRight();
					rcNew.top    = pView->GetTextArea().GenerateYPx( ptLast.y );
					rcNew.bottom = rcNew.top + nCharHeight;
					
					// 2006.10.01 Moca GDI(リージョン)リソースリーク修正
					HRGN hrgnEOFNew = ::CreateRectRgnIndirect( &rcNew );
					::CombineRgn( hrgnDraw, hrgnDraw, hrgnEOFNew, RGN_DIFF );
					::DeleteObject( hrgnEOFNew );
				}
				::PaintRgn( hdc, hrgnDraw );
				// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
				if( bCompatBMP ){
					::PaintRgn( pView->m_hdcCompatDC, hrgnDraw );
				}
				// To Here 2007.09.09 Moca
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
		CLayoutRange sRangeA;
		CLayoutInt nLineNum;

		// 現在描画されている範囲と始点が同じ
		if( m_sSelect.GetFrom() == m_sSelectOld.GetFrom() ){
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

	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	if( bCompatBMP ){
		::SetROP2( pView->m_hdcCompatDC, nROPCompatOld );
		::SelectObject( pView->m_hdcCompatDC, hBrushCompatOld );
	}
	// To Here 2007.09.09 Moca

	// 2006.10.01 Moca 重複コード統合
	::SetROP2( hdc, nROP_Old );
	::SelectObject( hdc, hBrushOld );
	::DeleteObject( hBrush );
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
	CEditView const * const pView = m_pcEditView;
	bool bCompatBMP = pView->m_hbmpCompatBMP && hdc != pView->m_hdcCompatDC;

	const CLayoutMgr& layoutMgr = pView->m_pcEditDoc->m_cLayoutMgr;
	const CLayout* pcLayout = layoutMgr.SearchLineByLayoutY( nLineNum );
	CLayoutRange lineArea;
	GetSelectAreaLineFromRange(lineArea, nLineNum, pcLayout, sRange);
	CLayoutInt nSelectFrom = lineArea.GetFrom().GetX2();
	CLayoutInt nSelectTo = lineArea.GetTo().GetX2();
	if( nSelectFrom == INT_MAX || nSelectTo == INT_MAX ){
		CLayoutInt nPosX = CLayoutInt(0);
		CMemoryIterator it = layoutMgr.CreateCMemoryIterator(pcLayout);
		
		while( !it.end() ){
			it.scanNext();
			if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
				// HACK:改行コードは選択だけ1桁幅
				if( CTypeSupport(pView, COLORIDX_EOL).IsDisp() ){
					nPosX += pView->GetTextMetrics().GetLayoutXDefault();
					if( pcLayout->GetLayoutEol().GetLen() != 0 ){
						nPosX += 4; // 4pxはCRLFのはみ出てる分
					}
				}else{
					nPosX += 2; // 非表示なら()2px
				}
				break;
			}
			// 2006.03.28 Moca 画面外まで求めたら打ち切る
			if( it.getColumn() > pView->GetTextArea().GetRightCol() ){
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();

		if( nSelectFrom == INT_MAX ){
			nSelectFrom = nPosX;
		}
		if( nSelectTo == INT_MAX ){
			nSelectTo = nPosX;
		}
	}
	
	// 2006.03.28 Moca ウィンドウ幅が大きいと正しく反転しない問題を修正
	if( nSelectFrom < pView->GetTextArea().GetViewLeftCol() ){
		nSelectFrom = pView->GetTextArea().GetViewLeftCol();
	}
	int		nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int		nCharWidth = pView->GetTextMetrics().GetCharPxWidth();
	CMyRect	rcClip; // px
	rcClip.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)nSelectFrom * nCharWidth;
	rcClip.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)nSelectTo   * nCharWidth;
	rcClip.top		= pView->GetTextArea().GenerateYPx( nLineNum );
	rcClip.bottom	= rcClip.top + nLineHeight;
	if( rcClip.right > pView->GetTextArea().GetAreaRight() ){
		rcClip.right = pView->GetTextArea().GetAreaRight();
	}
	//	必要なときだけ。
	if ( rcClip.right != rcClip.left ){
		CLayoutRange selectOld = m_sSelect;
		const_cast<CLayoutRange*>(&m_sSelect)->Clear(-1);
		pView->GetCaret().m_cUnderLine.CaretUnderLineOFF(true, false, true);
		*(const_cast<CLayoutRange*>(&m_sSelect)) = selectOld;
		
		// 2006.03.28 Moca 表示域内のみ処理する
		if( nSelectFrom <=pView->GetTextArea().GetRightCol() && pView->GetTextArea().GetViewLeftCol() < nSelectTo ){
			HRGN hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
			::PaintRgn( hdc, hrgnDraw );
			// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
			if( bCompatBMP ){
				::PaintRgn( pView->m_hdcCompatDC, hrgnDraw );
			}
			// To Here 2007.09.09 Moca
			::DeleteObject( hrgnDraw );
		}
	}
}

void CViewSelect::GetSelectAreaLineFromRange(
	CLayoutRange& ret,
	CLayoutInt nLineNum,
	const CLayout* pcLayout,
	const CLayoutRange&	sRange
) const
{
	const CEditView& view = *GetEditView();
	if( nLineNum >= sRange.GetFrom().y && nLineNum <= sRange.GetTo().y ||
		nLineNum >= sRange.GetTo().y && nLineNum <= sRange.GetFrom().y ){
		CLayoutInt	nSelectFrom = sRange.GetFrom().GetX2();
		CLayoutInt	nSelectTo   = sRange.GetTo().GetX2();
		if( IsBoxSelecting() ){		/* 矩形範囲選択中 */
			nSelectFrom = sRange.GetFrom().GetX2();
			nSelectTo   = sRange.GetTo().GetX2();
			// 2006.09.30 Moca From 矩形選択時[EOF]とその右側は反転しないように修正。処理を追加
			// 2011.12.26 [EOF]単独行以外なら反転する
			if( view.m_pcEditDoc->m_cLayoutMgr.GetLineCount() <= nLineNum ){
				nSelectFrom = -1;
				nSelectTo = -1;
			}
			// 2006.09.30 Moca To
		}
		else{
			if( sRange.IsLineOne() ){
				nSelectFrom = sRange.GetFrom().GetX2();
				nSelectTo   = sRange.GetTo().GetX2();
			}
			else{
				CLayoutInt nX_Layout = CLayoutInt(INT_MAX);
				if( nLineNum == sRange.GetFrom().y ){
					nSelectFrom = sRange.GetFrom().GetX2();
					nSelectTo   = nX_Layout;
				}
				else if( nLineNum == sRange.GetTo().GetY2() ){
					nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
					nSelectTo   = sRange.GetTo().GetX2();
				}
				else{
					nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
					nSelectTo   = nX_Layout;
				}
			}
		}
		// 2006.05.24 Moca 矩形選択/フリーカーソル選択(選択開始/終了行)で
		// To < From になることがある。必ず From < To になるように入れ替える。
		if( nSelectTo < nSelectFrom ){
			std::swap(nSelectFrom, nSelectTo);
		}
		ret.SetFrom(CLayoutPoint(nSelectFrom, nLineNum));
		ret.SetTo(CLayoutPoint(nSelectTo, nLineNum));
	}else{
		ret.SetFrom(CLayoutPoint(-1, -1));
		ret.SetTo(CLayoutPoint(-1, -1));
	}
}

/*!	選択範囲情報メッセージの表示

	@author genta
	@date 2005.07.09 genta 新規作成
	@date 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策を追加
	@date 2006.06.28 syat バイト数カウントを追加
*/
void CViewSelect::PrintSelectionInfoMsg() const
{
	const CEditView* pView=GetEditView();

	//	出力されないなら計算を省略
	if( ! pView->m_pcEditWnd->m_cStatusBar.SendStatusMessage2IsEffective() )
		return;

	CLayoutInt nLineCount = pView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( ! IsTextSelected() || m_sSelect.GetFrom().y >= nLineCount ){ // 先頭行が実在しない
		const_cast<CEditView*>(pView)->GetCaret().m_bClearStatus = false;
		if( IsBoxSelecting() ){
			pView->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( _T("box selecting") );
		}else if( m_bSelectingLock ){
			pView->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( _T("selecting") );
		}else{
			pView->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( _T("") );
		}
		return;
	}

	TCHAR msg[128];
	//	From here 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策

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
		int select_col_keta = (Int)select_col / (Int)pView->GetTextMetrics().GetLayoutXDefault();
		auto_sprintf( msg, _T("%d col (%dpx) * %d lines selected."),
			select_col_keta, select_col, select_line );
	}
	else {
		//	通常の選択では選択範囲の中身を数える
		int select_sum = 0;	//	バイト数合計
		const wchar_t *pLine;	//	データを受け取る
		CLogicInt	nLineLen;		//	行の長さ
		const CLayout*	pcLayout;
		CViewSelect* thiz = const_cast<CViewSelect*>( this );	// const外しthis

		// 共通設定・選択文字数を文字単位ではなくバイト単位で表示する
		BOOL bCountByByteCommon = GetDllShareData().m_Common.m_sStatusbar.m_bDispSelCountByByte;
		BOOL bCountByByte = ( pView->m_pcEditWnd->m_nSelectCountMode == SELECT_COUNT_TOGGLE ?
								bCountByByteCommon :
								pView->m_pcEditWnd->m_nSelectCountMode == SELECT_COUNT_BY_BYTE );

		//	1行目
		pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.GetFrom().GetY2(), &nLineLen, &pcLayout );
		if( pLine ){
			if( bCountByByte ){
				//  バイト数でカウント
				//  内部文字コードから現在の文字コードに変換し、バイト数を取得する。
				//  コード変換は負荷がかかるため、選択範囲の増減分のみを対象とする。

				CNativeW cmemW;
				CMemory cmemCode;

				// 増減分文字列の取得にCEditView::GetSelectedDataを使いたいが、m_sSelect限定のため、
				// 呼び出し前にm_sSelectを書き換える。呼出し後に元に戻すので、constと言えないこともない。
				CLayoutRange rngSelect = m_sSelect;		// 選択領域の退避
				bool bSelExtend;						// 選択領域拡大フラグ

				// 最終行の処理
				pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.GetTo().y, &nLineLen, &pcLayout );
				if( pLine ){
					if( pView->LineColumnToIndex( pcLayout, m_sSelect.GetTo().GetX2() ) == 0 ){
						//	最終行の先頭にキャレットがある場合は
						//	その行を行数に含めない
						--select_line;
					}
				}else{
					//	最終行が空行なら
					//	その行を行数に含めない
					--select_line;
				}

				//2009.07.07 syat m_nLastSelectedByteLenが0の場合は、差分ではなく全体を変換する（モード切替時にキャッシュクリアするため）

				if( m_bSelectAreaChanging && m_nLastSelectedByteLen && m_sSelect.GetFrom() == m_sSelectOld.GetFrom() ){
					// 範囲が後方に拡大された
					if( PointCompare( m_sSelect.GetTo(), m_sSelectOld.GetTo() ) < 0 ){
						bSelExtend = false;				// 縮小
						thiz->m_sSelect = CLayoutRange( m_sSelect.GetTo(), m_sSelectOld.GetTo() );
					}else{
						bSelExtend = true;				// 拡大
						thiz->m_sSelect = CLayoutRange( m_sSelectOld.GetTo(), m_sSelect.GetTo() );
					}

					const_cast<CEditView*>( pView )->GetSelectedDataSimple(cmemW);
					thiz->m_sSelect = rngSelect;		// m_sSelectを元に戻す
				}
				else if( m_bSelectAreaChanging && m_nLastSelectedByteLen && m_sSelect.GetTo() == m_sSelectOld.GetTo() ){
					// 範囲が前方に拡大された
					if( PointCompare( m_sSelect.GetFrom(), m_sSelectOld.GetFrom() ) < 0 ){
						bSelExtend = true;				// 拡大
						thiz->m_sSelect = CLayoutRange( m_sSelect.GetFrom(), m_sSelectOld.GetFrom() );
					}else{
						bSelExtend = false;				// 縮小
						thiz->m_sSelect = CLayoutRange( m_sSelectOld.GetFrom(), m_sSelect.GetFrom() );
					}

					const_cast<CEditView*>( pView )->GetSelectedDataSimple(cmemW);
					thiz->m_sSelect = rngSelect;		// m_sSelectを元に戻す
				}
				else{
					// 選択領域全体をコード変換対象にする
					const_cast<CEditView*>( pView )->GetSelectedDataSimple(cmemW);
					bSelExtend = true;
					thiz->m_nLastSelectedByteLen = 0;
				}
				//  現在の文字コードに変換し、バイト長を取得する
				CCodeBase* pCode = CCodeFactory::CreateCodeBase(pView->m_pcEditDoc->GetDocumentEncoding(), false);
				pCode->UnicodeToCode( cmemW, &cmemCode );
				delete pCode;

				if( bSelExtend ){
					select_sum = m_nLastSelectedByteLen + cmemCode.GetRawLength();
				}else{
					select_sum = m_nLastSelectedByteLen - cmemCode.GetRawLength();
				}
				thiz->m_nLastSelectedByteLen = select_sum;

			}
			else{
				//  文字数でカウント

				//2009.07.07 syat カウント方法を切り替えながら選択範囲を拡大・縮小すると整合性が
				//                とれなくなるため、モード切替時にキャッシュをクリアする。
				thiz->m_nLastSelectedByteLen = 0;

				//	1行だけ選択されている場合
				if( m_sSelect.IsLineOne() ){
					select_sum =
						pView->LineColumnToIndex( pcLayout, m_sSelect.GetTo().GetX2() )
						- pView->LineColumnToIndex( pcLayout, m_sSelect.GetFrom().GetX2() );
				} else {	//	2行以上選択されている場合
					select_sum =
						pcLayout->GetLengthWithoutEOL()
						+ pcLayout->GetLayoutEol().GetLen()
						- pView->LineColumnToIndex( pcLayout, m_sSelect.GetFrom().GetX2() );

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
						select_sum += pcLayout->GetLengthWithoutEOL() + pcLayout->GetLayoutEol().GetLen();
					}

					//	最終行の処理
					pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
					if( pLine ){
						int last_line_chars = pView->LineColumnToIndex( pcLayout, m_sSelect.GetTo().GetX2() );
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
		}

#ifdef _DEBUG
		auto_sprintf( msg, _T("%d %ts (%d lines) selected. [%d:%d]-[%d:%d]"),
			select_sum,
			( bCountByByte ? _T("bytes") : _T("chars") ),
			select_line,
			m_sSelect.GetFrom().x, m_sSelect.GetFrom().y,
			m_sSelect.GetTo().x, m_sSelect.GetTo().y );
#else
		auto_sprintf( msg, _T("%d %ts (%d lines) selected."), select_sum, ( bCountByByte ? _T("bytes") : _T("chars") ), select_line );
#endif
	}
	const_cast<CEditView*>(pView)->GetCaret().m_bClearStatus = false;
	pView->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( msg );
}
