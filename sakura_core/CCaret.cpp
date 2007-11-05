#include "stdafx.h"
#include "CCaret.h"
#include "CTextArea.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "CTextMetrics.h"
#include "CMemoryIterator.h"
#include "CLayout.h"
#include "charcode.h"
#include "CEditWnd.h"
#include <vector>
using namespace std;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         外部依存                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


int CCaret::GetHankakuDx() const
{
	return m_pEditView->GetTextMetrics().GetHankakuDx();
}

int CCaret::GetHankakuHeight() const
{
	return m_pEditView->GetTextMetrics().GetHankakuHeight();
}

int CCaret::GetHankakuDy() const
{
	return m_pEditView->GetTextMetrics().GetHankakuDy();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CCaretUnderLine                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* カーソル行アンダーラインのON */
void CCaretUnderLine::CaretUnderLineON( bool bDraw )
{
	if( m_nLockCounter ) return;	//	ロックされていたら何もできない。
	m_pcEditView->CaretUnderLineON( bDraw );
}

/* カーソル行アンダーラインのOFF */
void CCaretUnderLine::CaretUnderLineOFF( bool bDraw )
{
	if( m_nLockCounter ) return;	//	ロックされていたら何もできない。
	m_pcEditView->CaretUnderLineOFF( bDraw );
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CCaret::CCaret(CEditView* pEditView, const CEditDoc* pEditDoc)
: m_pEditView(pEditView)
, m_pEditDoc(pEditDoc)
, m_ptCaretPos_Layout(0,0)
, m_sizeCaret(0,0)				// キャレットのサイズ
, m_ptCaretPos_Logic(0,0)			// カーソル位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
, m_cUnderLine(pEditView)
{
	m_nCaretPosX_Prev = CLayoutInt(0);		/* ビュー左端からのカーソル桁直前の位置(０オリジン) */

	m_crCaret = -1;				/* キャレットの色 */			// 2006.12.16 ryoji
	m_hbmpCaret = NULL;			/* キャレット用ビットマップ */	// 2006.11.28 ryoji
}

CCaret::~CCaret()
{
	// キャレット用ビットマップ	// 2006.11.28 ryoji
	if( m_hbmpCaret != NULL )
		DeleteObject( m_hbmpCaret );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief 行桁指定によるカーソル移動

	必要に応じて縦/横スクロールもする．
	垂直スクロールをした場合はその行数を返す（正／負）．
	
	@param nWk_CaretPosX	[in] 移動先桁位置(0～)
	@param nWk_CaretPosY	[in] 移動先行位置(0～)
	@param bScroll			[in] TRUE: 画面位置調整有り/ FALSE: 画面位置調整有り無し
	@param nCaretMarginRate	[in] 縦スクロール開始位置を決める値
	@return 縦スクロール行数(負:上スクロール/正:下スクロール)

	@note 不正な位置が指定された場合には適切な座標値に
		移動するため，引数で与えた座標と移動後の座標は
		必ずしも一致しない．
	
	@note bScrollがfalseの場合にはカーソル位置のみ移動する．
		trueの場合にはスクロール位置があわせて変更される

	@date 2001/10/20 deleted by novice AdjustScrollBar()を呼ぶ位置を変更
	@date 2004.04.02 Moca 行だけ有効な座標に修正するのを厳密に処理する
	@date 2004.09.11 genta bDrawスイッチは動作と名称が一致していないので
		再描画スイッチ→画面位置調整スイッチと名称変更
*/
CLayoutInt CCaret::MoveCursor(
	CLayoutPoint	ptWk_CaretPos,	//!< 移動先 (桁,行)
	bool			bScroll,
	int				nCaretMarginRate
)
{
	//必要なインターフェース
	const CLayoutMgr* pLayoutMgr=&m_pEditDoc->m_cLayoutMgr;
	const Types* pTypes=&m_pEditDoc->GetDocumentAttribute();

	// スクロール処理
	CLayoutInt	nScrollRowNum = CLayoutInt(0);
	CLayoutInt	nScrollColNum = CLayoutInt(0);
	RECT	rcClip;
	RECT	rcClip2;
	int		nCaretMarginY;
	CLayoutInt		nScrollMarginRight;
	CLayoutInt		nScrollMarginLeft;

	if( 0 >= m_pEditView->GetTextArea().m_nViewColNum ){
		return CLayoutInt(0);
	}

	// カーソル行アンダーラインのOFF
	m_cUnderLine.CaretUnderLineOFF( bScroll );	//	YAZAKI

	if( m_pEditView->GetSelectionInfo().IsMouseSelecting() ){	// 範囲選択中
		nCaretMarginY = 0;
	}
	else{
		//	2001/10/20 novice
		nCaretMarginY = (Int)m_pEditView->GetTextArea().m_nViewRowNum / nCaretMarginRate;
		if( 1 > nCaretMarginY ){
			nCaretMarginY = 1;
		}
	}
	// 2004.04.02 Moca 行だけ有効な座標に修正するのを厳密に処理する
	GetAdjustCursorPos( &ptWk_CaretPos );
	
	
	// 水平スクロール量（文字数）の算出
	nScrollColNum = CLayoutInt(0);
	nScrollMarginRight = CLayoutInt(4);
	nScrollMarginLeft = CLayoutInt(4);

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if( m_pEditDoc->m_cLayoutMgr.GetMaxLineKetas() > m_pEditView->GetTextArea().m_nViewColNum &&
		ptWk_CaretPos.GetX() > m_pEditView->GetTextArea().GetViewLeftCol() + m_pEditView->GetTextArea().m_nViewColNum - nScrollMarginRight ){
		nScrollColNum =
			( m_pEditView->GetTextArea().GetViewLeftCol() + m_pEditView->GetTextArea().m_nViewColNum - nScrollMarginRight ) - ptWk_CaretPos.GetX2();
	}
	else if( 0 < m_pEditView->GetTextArea().GetViewLeftCol() &&
		ptWk_CaretPos.GetX() < m_pEditView->GetTextArea().GetViewLeftCol() + nScrollMarginLeft
	){
		nScrollColNum = m_pEditView->GetTextArea().GetViewLeftCol() + nScrollMarginLeft - ptWk_CaretPos.GetX2();
		if( 0 > m_pEditView->GetTextArea().GetViewLeftCol() - nScrollColNum ){
			nScrollColNum = m_pEditView->GetTextArea().GetViewLeftCol();
		}

	}

	m_pEditView->GetTextArea().SetViewLeftCol(m_pEditView->GetTextArea().GetViewLeftCol() - nScrollColNum);

	//	From Here 2007.07.28 じゅうじ : 表示行数が3行以下の場合の動作改善
	/* 垂直スクロール量（行数）の算出 */
										// 画面が３行以下
	if( m_pEditView->GetTextArea().m_nViewRowNum <= 3 ){
							// 移動先は、画面のスクロールラインより上か？（up キー）
		if( ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine() < nCaretMarginY ){
			if( ptWk_CaretPos.y < nCaretMarginY ){	//１行目に移動
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine();
			}
			else if( m_pEditView->GetTextArea().m_nViewRowNum <= 1 ){	// 画面が１行
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine() - ptWk_CaretPos.y;
			}
#if !(0)	// COMMENTにすると、上下の空きを死守しない為、縦移動はgoodだが、横移動の場合上下にぶれる
			else if( m_pEditView->GetTextArea().m_nViewRowNum <= 2 ){	// 画面が２行
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine() - ptWk_CaretPos.y;
			}
#endif
			else
			{						// 画面が３行
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine() - ptWk_CaretPos.y + 1;
			}
		}else
							// 移動先は、画面の最大行数－２より下か？（down キー）
		if( ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine() >= (m_pEditView->GetTextArea().m_nViewRowNum - nCaretMarginY - 2) ){
			CLayoutInt ii = m_pEditDoc->m_cLayoutMgr.GetLineCount();
			if( ii - ptWk_CaretPos.y < nCaretMarginY + 1 &&
				ii - m_pEditView->GetTextArea().GetViewTopLine() < m_pEditView->GetTextArea().m_nViewRowNum ) {
			}
			else if( m_pEditView->GetTextArea().m_nViewRowNum <= 2 ){	// 画面が２行、１行
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine() - ptWk_CaretPos.y;
			}else{						// 画面が３行
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine() - ptWk_CaretPos.y + 1;
			}
		}
	}
	// 移動先は、画面のスクロールラインより上か？（up キー）
	else if( ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine() < nCaretMarginY ){
		if( ptWk_CaretPos.y < nCaretMarginY ){	//１行目に移動
			nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine();
		}else{
			nScrollRowNum = -(ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine()) + nCaretMarginY;
		}
	}
	// 移動先は、画面の最大行数－２より下か？（down キー）
	else if( ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine() >= m_pEditView->GetTextArea().m_nViewRowNum - nCaretMarginY - 2 ){
		CLayoutInt ii = m_pEditDoc->m_cLayoutMgr.GetLineCount();
		if( ii - ptWk_CaretPos.y < nCaretMarginY + 1 &&
			ii - m_pEditView->GetTextArea().GetViewTopLine() < m_pEditView->GetTextArea().m_nViewRowNum ) {
		}
		else{
			nScrollRowNum =
				-(ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine()) + (m_pEditView->GetTextArea().m_nViewRowNum - nCaretMarginY - 2);
		}
	}
	//	To Here 2007.07.28 じゅうじ
	if( bScroll ){
		/* スクロール */
		if( t_abs( nScrollColNum ) >= m_pEditView->GetTextArea().m_nViewColNum ||
			t_abs( nScrollRowNum ) >= m_pEditView->GetTextArea().m_nViewRowNum ){
			m_pEditView->GetTextArea().OffsetViewTopLine(-nScrollRowNum);
			::InvalidateRect( m_pEditView->m_hWnd, NULL, TRUE );
		}
		else if( nScrollRowNum != 0 || nScrollColNum != 0 ){
			RECT	rcScroll;

			m_pEditView->GetTextArea().GenerateTextAreaRect(&rcScroll);
			if( nScrollRowNum > 0 ){
				rcScroll.bottom = m_pEditView->GetTextArea().GetAreaBottom() - (Int)nScrollRowNum * m_pEditView->GetTextMetrics().GetHankakuDy();
				m_pEditView->GetTextArea().OffsetViewTopLine(-nScrollRowNum);
				m_pEditView->GetTextArea().GenerateTopRect(&rcClip,nScrollRowNum);
			}
			else if( nScrollRowNum < 0 ){
				rcScroll.top = m_pEditView->GetTextArea().GetAreaTop() - (Int)nScrollRowNum * m_pEditView->GetTextMetrics().GetHankakuDy();
				m_pEditView->GetTextArea().OffsetViewTopLine(-nScrollRowNum);
				m_pEditView->GetTextArea().GenerateBottomRect(&rcClip,-nScrollRowNum);
			}

			if( nScrollColNum > 0 ){
				rcScroll.left = m_pEditView->GetTextArea().GetAreaLeft();
				rcScroll.right = m_pEditView->GetTextArea().GetAreaRight() - (Int)nScrollColNum * GetHankakuDx();
				m_pEditView->GetTextArea().GenerateLeftRect(&rcClip2, nScrollColNum);
			}
			else if( nScrollColNum < 0 ){
				rcScroll.left = m_pEditView->GetTextArea().GetAreaLeft() - (Int)nScrollColNum * GetHankakuDx();
				m_pEditView->GetTextArea().GenerateRightRect(&rcClip2, -nScrollColNum);
			}

			if( m_pEditView->GetDrawSwitch() ){
				::ScrollWindowEx(
					m_pEditView->m_hWnd,
					(Int)nScrollColNum * GetHankakuDx(),	/* 水平スクロール量 */
					(Int)nScrollRowNum * m_pEditView->GetTextMetrics().GetHankakuDy(),	/* 垂直スクロール量 */
					&rcScroll,	/* スクロール長方形の構造体のアドレス */
					NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
				);

				if( nScrollRowNum != 0 ){
					::InvalidateRect( m_pEditView->m_hWnd, &rcClip, TRUE );
					if( nScrollColNum != 0 ){
						m_pEditView->GetTextArea().GenerateLineNumberRect(&rcClip);
						::InvalidateRect( m_pEditView->m_hWnd, &rcClip, TRUE );
					}
				}
				if( nScrollColNum != 0 ){
					::InvalidateRect( m_pEditView->m_hWnd, &rcClip2, TRUE );
				}
			}
		}

		/* スクロールバーの状態を更新する */
		m_pEditView->AdjustScrollBars(); // 2001/10/20 novice
	}

	/* キャレット移動 */
	SetCaretLayoutPos(ptWk_CaretPos);

	/* カーソル位置変換
	||  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	||  →物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	m_pEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_ptCaretPos_Layout,
		&m_ptCaretPos_Logic	//カーソル位置。ロジック単位。
	);

	// 横スクロールが発生したら、ルーラー全体を再描画 2002.02.25 Add By KK
	if (nScrollColNum != 0 ){
		//次回DispRuler呼び出し時に再描画。（bDraw=falseのケースを考慮した。）
		m_pEditView->GetRuler().SetRedrawFlag();
	}

	/* カーソル行アンダーラインのON */
	//CaretUnderLineON( bDraw ); //2002.02.27 Del By KK アンダーラインのちらつきを低減
	HDC		hdc = ::GetDC( m_pEditView->m_hWnd );
	if( bScroll ){
		/* キャレットの表示・更新 */
		ShowEditCaret();

		/* ルーラの再描画 */
		m_pEditView->GetRuler().DispRuler( hdc );

		/* アンダーラインの再描画 */
		m_cUnderLine.CaretUnderLineON(TRUE);

		/* キャレットの行桁位置を表示する */
		DrawCaretPosInfo();

		//	Sep. 11, 2004 genta 同期スクロールの関数化
		//	bScroll == FALSEの時にはスクロールしないので，実行しない
		m_pEditView->SyncScrollV( -nScrollRowNum );	//	方向が逆なので符号反転が必要
		m_pEditView->SyncScrollH( -nScrollColNum );	//	方向が逆なので符号反転が必要

	}
	::ReleaseDC( m_pEditView->m_hWnd, hdc );

// 02/09/18 対括弧の強調表示 ai Start	03/02/18 ai mod S
	m_pEditView->DrawBracketPair( false );
	m_pEditView->SetBracketPairPos( true );
	m_pEditView->DrawBracketPair( true );
// 02/09/18 対括弧の強調表示 ai End		03/02/18 ai mod E

	return nScrollRowNum;

}

/* マウス等による座標指定によるカーソル移動
|| 必要に応じて縦/横スクロールもする
|| 垂直スクロールをした場合はその行数を返す(正／負)
*/
//2007.09.11 kobake 関数名変更: MoveCursorToPoint→MoveCursorToClientPoint
CLayoutInt CCaret::MoveCursorToClientPoint( const POINT& ptClientPos )
{
	CLayoutInt		nScrollRowNum;
	CLayoutPoint	ptLayoutPos;
	m_pEditView->GetTextArea().ClientToLayout(ptClientPos, &ptLayoutPos);

	int	dx = (ptClientPos.x - m_pEditView->GetTextArea().GetAreaLeft()) % ( m_pEditView->GetTextMetrics().GetHankakuDx() );

	nScrollRowNum = MoveCursorProperly( ptLayoutPos, TRUE, 1000, dx );
	m_nCaretPosX_Prev = GetCaretLayoutPos().GetX2();
	return nScrollRowNum;
}
//_CARETMARGINRATE_CARETMARGINRATE_CARETMARGINRATE



/*! 正しいカーソル位置を算出する(EOF以降のみ)
	@param pnPosX [in/out] カーソルのレイアウト座標X
	@param pnPosY [in/out] カーソルのレイアウト座標Y
	@retval	TRUE 座標を修正した
	@retval	FALSE 座標は修正されなかった
	@note	EOFの直前が改行でない場合は、その行に限りEOF以降にも移動可能
			EOFだけの行は、先頭位置のみ正しい。
	@date 2004.04.02 Moca 関数化
*/
BOOL CCaret::GetAdjustCursorPos(
	CLayoutPoint* pptPosXY
)
{
	//必要なインターフェース
	const CLayoutMgr* pLayoutMgr=&m_pEditDoc->m_cLayoutMgr;

	// 2004.03.28 Moca EOFのみのレイアウト行は、0桁目のみ有効.EOFより下の行のある場合は、EOF位置にする
	CLayoutInt nLayoutLineCount = m_pEditDoc->m_cLayoutMgr.GetLineCount();

	CLayoutPoint ptPosXY2 = *pptPosXY;
	/*
	int nPosX2 = *pnPosX;
	int nPosY2 = *pnPosY;
	*/
	BOOL ret = FALSE;
	if( ptPosXY2.y >= nLayoutLineCount ){
		if( 0 < nLayoutLineCount ){
			ptPosXY2.y = nLayoutLineCount - 1;
			const CLayout* pcLayout = m_pEditDoc->m_cLayoutMgr.SearchLineByLayoutY( ptPosXY2.GetY2() );
			if( pcLayout->m_cEol == EOL_NONE ){
				ptPosXY2.x = m_pEditView->LineIndexToColmn( pcLayout, pcLayout->GetLength() );
				// EOFだけ折り返されているか
				//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
				if( ptPosXY2.x >= m_pEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){
					ptPosXY2.y++;
					ptPosXY2.x = CLayoutInt(0);
				}
			}
			else{
				// EOFだけの行
				ptPosXY2.y++;
				ptPosXY2.x = CLayoutInt(0);
			}
		}else{
			// 空のファイル
			ptPosXY2.Set(CLayoutInt(0), CLayoutInt(0));
		}
		if( *pptPosXY != ptPosXY2 ){//*pnPosX != nPosX2 || *pnPosY != nPosY2 ){
			*pptPosXY = ptPosXY2;
			//*pnPosX = nPosX2;
			//*pnPosY = nPosY2;
			ret = TRUE;
		}
	}
	return ret;
}

/* キャレットの表示・更新 */
void CCaret::ShowEditCaret()
{
	//必要なインターフェース
	const CLayoutMgr* pLayoutMgr=&m_pEditDoc->m_cLayoutMgr;
	CommonSetting* pCommon=&m_pEditView->m_pShareData->m_Common;
	const Types* pTypes=&m_pEditDoc->GetDocumentAttribute();


	using namespace WCODE;

	int				nIdxFrom;


/*
		なんかフレームウィンドウがアクティブでないときに内部的にカーソル移動すると
		カーソルがないのに、カーソルがあるということになってしまう
		のでアクティブにしてもカーソルが出てこないときがある
		フレームウィンドウがアクティブでないときは、カーソルがないことにする
*/

	//207.08.26 kobake GetParent → GetAncestor に変更
	//アクティブでないのにカーソル作っちゃったから消しちゃう
	if( ::GetActiveWindow() != ::GetAncestor( m_pEditView->m_hWnd, GA_ROOT ) ){
		m_sizeCaret.cx = 0;
		return;
	}

	/* アクティブなペインを取得 */
	if( m_pEditView->m_nMyIndex != m_pEditDoc->m_pcEditWnd->GetActivePane() ){
		m_sizeCaret.cx = 0;
		return;
	}
	/* キャレットの幅、高さを決定 */
	int				nCaretWidth = 0;
	int				nCaretHeight = 0;
	// カーソルのタイプ = win
	if( 0 == pCommon->m_sGeneral.GetCaretType() ){
		nCaretHeight = GetHankakuHeight();					/* キャレットの高さ */
		if( m_pEditView->IsInsMode() /* Oct. 2, 2005 genta */ ){
			nCaretWidth = 2;
		}
		else{
			nCaretWidth = GetHankakuDx();

			const wchar_t*	pLine;
			CLogicInt		nLineLen;
			const CLayout*	pcLayout;
			pLine = pLayoutMgr->GetLineStr( GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );

			if( NULL != pLine ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom = m_pEditView->LineColmnToIndex( pcLayout, GetCaretLayoutPos().GetX2() );
				if( nIdxFrom >= nLineLen ||
					pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
					pLine[nIdxFrom] == TAB ){
					nCaretWidth = GetHankakuDx();
				}
				else{
					CLayoutInt nKeta = CNativeW2::GetKetaOfChar( pLine, nLineLen, nIdxFrom );
					if( 0 < nKeta ){
						nCaretWidth = GetHankakuDx() * (Int)nKeta;
					}
				}
			}
		}
	}
	// カーソルのタイプ = dos
	else if( 1 == pCommon->m_sGeneral.GetCaretType() ){
		if( m_pEditView->IsInsMode() /* Oct. 2, 2005 genta */ ){
			nCaretHeight = GetHankakuHeight() / 2;			/* キャレットの高さ */
		}
		else{
			nCaretHeight = GetHankakuHeight();				/* キャレットの高さ */
		}
		nCaretWidth = GetHankakuDx();

		const wchar_t*	pLine;
		CLogicInt		nLineLen;
		const CLayout*	pcLayout;
		pLine= pLayoutMgr->GetLineStr( GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );

		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdxFrom = m_pEditView->LineColmnToIndex( pcLayout, GetCaretLayoutPos().GetX2() );
			if( nIdxFrom >= nLineLen ||
				pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
				pLine[nIdxFrom] == TAB ){
				nCaretWidth = GetHankakuDx();
			}else{
				CLayoutInt nKeta = CNativeW2::GetKetaOfChar( pLine, nLineLen, nIdxFrom );
				if( 0 < nKeta ){
					nCaretWidth = GetHankakuDx() * (Int)nKeta;
				}
			}
		}
	}

	//	キャレット色の取得
	const ColorInfo* ColorInfoArr = pTypes->m_ColorInfoArr;
	int nCaretColor = ( ColorInfoArr[COLORIDX_CARET_IME].m_bDisp && m_pEditView->IsImeON() )? COLORIDX_CARET_IME: COLORIDX_CARET;
	COLORREF crCaret = ColorInfoArr[nCaretColor].m_colTEXT;
	COLORREF crBack = ColorInfoArr[COLORIDX_TEXT].m_colBACK;

	if( !ExistCaretFocus() ){
		/* キャレットがなかった場合 */
		/* キャレットの作成 */
		CreateEditCaret( crCaret, crBack, nCaretWidth, nCaretHeight );	// 2006.12.07 ryoji
		m_bCaretShowFlag = false; // 2002/07/22 novice
	}else{
		if( GetCaretSize() != CMySize(nCaretWidth,nCaretHeight) ||
			m_crCaret != crCaret || m_pEditView->m_crBack != crBack ){
			/* キャレットはあるが、大きさや色が変わった場合 */
			/* 現在のキャレットを削除 */
			::DestroyCaret();

			/* キャレットの作成 */
			CreateEditCaret( crCaret, crBack, nCaretWidth, nCaretHeight );	// 2006.12.07 ryoji
			m_bCaretShowFlag = false; // 2002/07/22 novice
		}else{
			/* キャレットはあるし、大きさも変わっていない場合 */
			/* キャレットを隠す */
			HideCaret_( m_pEditView->m_hWnd ); // 2002/07/22 novice
		}
	}

	// キャレットサイズ
	SetCaretSize(nCaretWidth,nCaretHeight);

	/* キャレットの位置を調整 */
	//2007.08.26 kobake キャレットX座標の計算をUNICODE仕様にした。
	POINT ptDrawPos=CalcCaretDrawPos(GetCaretLayoutPos());
	::SetCaretPos( ptDrawPos.x, ptDrawPos.y );
	if ( m_pEditView->GetTextArea().GetAreaLeft() <= ptDrawPos.x && m_pEditView->GetTextArea().GetAreaTop() <= ptDrawPos.y ){
		/* キャレットの表示 */
		ShowCaret_( m_pEditView->m_hWnd ); // 2002/07/22 novice
	}

	m_crCaret = crCaret;	//	2006.12.07 ryoji
	m_pEditView->m_crBack = crBack;		//	2006.12.07 ryoji
	m_pEditView->SetIMECompFormPos();


	return;
}




/*! キャレットの行桁位置およびステータスバーの状態表示の更新

	@note ステータスバーの状態の並び方の変更はメッセージを受信する
		CEditWnd::DispatchEvent()のWM_NOTIFYにも影響があることに注意
	
	@note ステータスバーの出力内容の変更はCEditWnd::OnSize()の
		カラム幅計算に影響があることに注意
*/
//2007.10.17 kobake 重複するコードを整理
void CCaret::DrawCaretPosInfo()
{
	//必要なインターフェース
	const CLayoutMgr* pLayoutMgr=&m_pEditDoc->m_cLayoutMgr;
	const Types* pTypes=&m_pEditDoc->GetDocumentAttribute();


	if( !m_pEditView->GetDrawSwitch() ){
		return;
	}

	// ステータスバーハンドルを取得
	HWND hwndStatusBar = m_pEditDoc->m_pcEditWnd->m_cStatusBar.GetStatusHwnd();


	// カーソル位置の文字列を取得
	const CLayout*	pcLayout;
	CLogicInt		nLineLen;
	const wchar_t*	pLine = pLayoutMgr->GetLineStr( GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );


	// -- -- -- -- 文字コード情報 -> pszCodeName -- -- -- -- //
	const TCHAR* pszCodeName;
	if(!hwndStatusBar){
		pszCodeName = gm_pszCodeNameArr_Short[m_pEditDoc->m_nCharCode];
	}
	else{
		pszCodeName = gm_pszCodeNameArr_Normal[m_pEditDoc->m_nCharCode];
	}


	// -- -- -- -- 改行モード -> szEolMode -- -- -- -- //
	//	May 12, 2000 genta
	//	改行コードの表示を追加
	CEOL cNlType = m_pEditDoc->GetNewLineCode();
	const TCHAR* szEolMode = cNlType.GetName();


	// -- -- -- -- キャレット位置 -> ptCaret -- -- -- -- //
	//
	CMyPoint ptCaret;
	//行番号をロジック単位で表示
	if(pTypes->m_bLineNumIsCRLF){
		ptCaret.x = (Int)GetCaretLayoutPos().GetX();
		ptCaret.y = (Int)GetCaretLogicPos().y;
		if(pcLayout)ptCaret.x += (Int)pcLayout->CalcLayoutOffset(*pLayoutMgr);
	}
	//行番号をレイアウト単位で表示
	else {
		ptCaret.x = (Int)GetCaretLayoutPos().GetX();
		ptCaret.y = (Int)GetCaretLayoutPos().GetY();
	}
	//表示値が1から始まるように補正
	ptCaret.x++;
	ptCaret.y++;


	// -- -- -- -- キャレット位置の文字情報 -> szCaretChar -- -- -- -- //
	//
	TCHAR szCaretChar[32]=_T("");
	if( pLine ){
		// 指定された桁に対応する行のデータ内の位置を調べる
		CLogicInt nIdx = m_pEditView->LineColmnToIndex( pcLayout, GetCaretLayoutPos().GetX2() );
		if( nIdx < nLineLen ){
			if( nIdx < nLineLen - (pcLayout->m_cEol.GetLen()?1:0) ){
				auto_sprintf( szCaretChar, _T("%04x"), pLine[nIdx]);
			}
			else{
				_tcscpy_s(szCaretChar, _countof(szCaretChar), pcLayout->m_cEol.GetName());
			}
		}
	}


	// -- -- -- --  ステータス情報を書き出す -- -- -- -- //
	//
	// ウィンドウ右上に書き出す
	if( !hwndStatusBar ){
		TCHAR	szText[64];
		auto_sprintf(
			szText,
			_T("%ts(%ts)   [%ls]%6d：%d"),
			pszCodeName,
			szEolMode,
			szCaretChar,
			ptCaret.y,
			ptCaret.x
		);
		m_pEditDoc->m_pcEditWnd->PrintMenubarMessage( szText );
	}
	// ステータスバーに状態を書き出す
	else{
		TCHAR	szText_1[64];
		auto_sprintf( szText_1, _T("%5d 行 %4d 桁"), ptCaret.y, ptCaret.x );	//Oct. 30, 2000 JEPRO 千万行も要らん

		TCHAR	szText_6[16];
		if( m_pEditView->IsInsMode() /* Oct. 2, 2005 genta */ ){
			_tcscpy( szText_6, _T("挿入") );
		}else{
			_tcscpy( szText_6, _T("上書") );
		}
		::StatusBar_SetText( hwndStatusBar, 0 | SBT_NOBORDERS, _T("") );
		::StatusBar_SetText( hwndStatusBar, 1 | 0,             szText_1 );
		//	May 12, 2000 genta
		//	改行コードの表示を追加．後ろの番号を1つずつずらす
		//	From Here
		::StatusBar_SetText( hwndStatusBar, 2 | 0,             szEolMode );
		//	To Here
		::StatusBar_SetText( hwndStatusBar, 3 | 0,             szCaretChar );
		::StatusBar_SetText( hwndStatusBar, 4 | 0,             pszCodeName );
		::StatusBar_SetText( hwndStatusBar, 5 | SBT_OWNERDRAW, _T("") );
		::StatusBar_SetText( hwndStatusBar, 6 | 0,             szText_6 );
	}

}

/* カーソル上下移動処理 */
CLayoutInt CCaret::Cursor_UPDOWN( CLayoutInt nMoveLines, bool bSelect )
{
	//必要なインターフェース
	const CLayoutMgr* pLayoutMgr=&m_pEditDoc->m_cLayoutMgr;
//	const Types* pTypes=&m_pEditDoc->GetDocumentAttribute();
	CommonSetting* pCommon=&m_pEditView->m_pShareData->m_Common;


	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	CLayoutInt		nLineCols;
	CLayoutInt		nScrollLines;
	const CLayout*	pcLayout;
	nScrollLines = CLayoutInt(0);

	CLayoutPoint		ptPosXY(CLayoutInt(0), GetCaretLayoutPos().GetY());

	if( nMoveLines > 0 ){
		/* カーソルがテキスト最下端行にあるか */
		if( GetCaretLayoutPos().GetY() + nMoveLines >= pLayoutMgr->GetLineCount() ){
			nMoveLines = pLayoutMgr->GetLineCount() - GetCaretLayoutPos().GetY()  - 1;
		}
		if( nMoveLines <= 0 ){
			pLine = pLayoutMgr->GetLineStr( GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );
			if( NULL != pLine ){
				nLineCols = m_pEditView->LineIndexToColmn( pcLayout, nLineLen );
				/* 改行で終わっているか */
				//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
				if( ( EOL_NONE != pcLayout->m_cEol.GetLen() )
//				if( ( pLine[ nLineLen - 1 ] == L'\n' || pLine[ nLineLen - 1 ] == L'\r' )
				 || nLineCols >= pLayoutMgr->GetMaxLineKetas()
				){
					if( bSelect ){
						if( !m_pEditView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
							/* 現在のカーソル位置から選択を開始する */
							m_pEditView->GetSelectionInfo().BeginSelectArea();
						}
					}else{
						if( m_pEditView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
							/* 現在の選択範囲を非選択状態に戻す */
							m_pEditView->GetSelectionInfo().DisableSelectArea( TRUE );
						}
					}
					ptPosXY.x = CLayoutInt(0);
					++ptPosXY.y;
					if( bSelect ){
						/* 現在のカーソル位置によって選択範囲を変更 */
						m_pEditView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptPosXY );
					}
					nScrollLines = MoveCursor( ptPosXY, m_pEditView->GetDrawSwitch() /* TRUE */ ); // YAZAKI.
				}
			}
			//	Sep. 11, 2004 genta 同期スクロールの関数化
			//	MoveCursorでスクロール位置調整済み
			//SyncScrollV( nScrollLines );
			return nScrollLines;
		}
	}else{
		/* カーソルがテキスト最上端行にあるか */
		if( GetCaretLayoutPos().GetY() + nMoveLines < 0 ){
			nMoveLines = - GetCaretLayoutPos().GetY();
		}
		if( nMoveLines >= 0 ){
			//	Sep. 11, 2004 genta 同期スクロールの関数化
			m_pEditView->SyncScrollV( nScrollLines );
			return nScrollLines;
		}
	}
	if( bSelect ){
		if( !m_pEditView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在のカーソル位置から選択を開始する */
			m_pEditView->GetSelectionInfo().BeginSelectArea();
		}
	}else{
		if( m_pEditView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			m_pEditView->GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}
	/* 次の行のデータを取得 */
	pLine = pLayoutMgr->GetLineStr( GetCaretLayoutPos().GetY2() + CLayoutInt(nMoveLines), &nLineLen, &pcLayout );
	CMemoryIterator it( pcLayout, pLayoutMgr->GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
			i = nLineLen;
			break;
		}
		if( it.getColumn() + it.getColumnDelta() > m_nCaretPosX_Prev ){
			i = it.getIndex();
			break;
		}
		it.addDelta();
	}
	ptPosXY.x += it.getColumn();
	if ( it.end() ){
		i = it.getIndex();
	}

	if( i >= nLineLen ){
		/* フリーカーソルモードか */
		if( pCommon->m_sGeneral.m_bIsFreeCursorMode
		 || m_pEditView->GetSelectionInfo().IsTextSelected() && m_pEditView->GetSelectionInfo().IsBoxSelecting()	/* 矩形範囲選択中 */
		){
			if( GetCaretLayoutPos().GetY() + nMoveLines + 1 == pLayoutMgr->GetLineCount()  ){
				if( NULL != pLine ){
					if( pLine[nLineLen - 1] == WCODE::CR || pLine[nLineLen - 1] == WCODE::LF ){
						ptPosXY.x = m_nCaretPosX_Prev;
					}
				}
			}else{
				ptPosXY.x = m_nCaretPosX_Prev;
			}
		}
	}
	nScrollLines = MoveCursor( CLayoutPoint(ptPosXY.x, GetCaretLayoutPos().GetY() + nMoveLines), m_pEditView->GetDrawSwitch() /* TRUE */ ); // YAZAKI.
	if( bSelect ){
		/* 現在のカーソル位置によって選択範囲を変更 */
		m_pEditView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( CLayoutPoint(ptPosXY.x, GetCaretLayoutPos().GetY()) );
	}

	return nScrollLines;
}


/*!	キャレットの作成

	@param nCaretColor [in]	キャレットの色種別 (0:通常, 1:IME ON)
	@param nWidth [in]		キャレット幅
	@param nHeight [in]		キャレット高

	@date 2006.12.07 ryoji 新規作成
*/
void CCaret::CreateEditCaret( COLORREF crCaret, COLORREF crBack, int nWidth, int nHeight )
{
	//
	// キャレット用のビットマップを作成する
	//
	// Note: ウィンドウ互換のメモリ DC 上で PatBlt を用いてキャレット色と背景色を XOR 結合
	//       することで，目的のビットマップを得る．
	//       ※ 256 色環境では RGB 値を単純に直接演算してもキャレット色を出すための正しい
	//          ビットマップ色は得られない．
	//       参考: [HOWTO] キャレットの色を制御する方法
	//             http://support.microsoft.com/kb/84054/ja
	//

	HBITMAP hbmpCaret;	// キャレット用のビットマップ

	HDC hdc = ::GetDC( m_pEditView->m_hWnd );

	hbmpCaret = ::CreateCompatibleBitmap( hdc, nWidth, nHeight );
	HDC hdcMem = ::CreateCompatibleDC( hdc );
	HBITMAP hbmpOld = (HBITMAP)::SelectObject( hdcMem, hbmpCaret );
	HBRUSH hbrCaret = ::CreateSolidBrush( crCaret );
	HBRUSH hbrBack = ::CreateSolidBrush( crBack );
	HBRUSH hbrOld = (HBRUSH)::SelectObject( hdcMem, hbrCaret );
	::PatBlt( hdcMem, 0, 0, nWidth, nHeight, PATCOPY );
	::SelectObject( hdcMem, hbrBack );
	::PatBlt( hdcMem, 0, 0, nWidth, nHeight, PATINVERT );
	::SelectObject( hdcMem, hbrOld );
	::SelectObject( hdcMem, hbmpOld );
	::DeleteObject( hbrCaret );
	::DeleteObject( hbrBack );
	::DeleteDC( hdcMem );

	::ReleaseDC( m_pEditView->m_hWnd, hdc );

	// 以前のビットマップを破棄する
	if( m_hbmpCaret != NULL )
		::DeleteObject( m_hbmpCaret );
	m_hbmpCaret = hbmpCaret;

	// キャレットを作成する
	::CreateCaret( m_pEditView->m_hWnd, hbmpCaret, nWidth, nHeight );
	return;
}


// 2002/07/22 novice
/*!
	キャレットの表示
*/
void CCaret::ShowCaret_( HWND hwnd )
{
	if ( m_bCaretShowFlag == false ){
		::ShowCaret( hwnd );
		m_bCaretShowFlag = true;
	}
}


/*!
	キャレットの非表示
*/
void CCaret::HideCaret_( HWND hwnd )
{
	if ( m_bCaretShowFlag == true ){
		::HideCaret( hwnd );
		m_bCaretShowFlag = false;
	}
}

//! 自分の状態を他のCCaretにコピー
void CCaret::CopyCaretStatus(CCaret* pCaret) const
{
	pCaret->SetCaretLayoutPos(GetCaretLayoutPos());
	pCaret->m_nCaretPosX_Prev = m_nCaretPosX_Prev;	/* ビュー左端からのカーソル桁位置（０オリジン）*/

	//※ キャレットのサイズはコピーしない。2002/05/12 YAZAKI
}


POINT CCaret::CalcCaretDrawPos(const CLayoutPoint& ptCaretPos) const
{
	int nPosX = m_pEditView->GetTextArea().GetAreaLeft() + (Int)(ptCaretPos.x - m_pEditView->GetTextArea().GetViewLeftCol()) * GetHankakuDx();
	int nPosY = m_pEditView->GetTextArea().GetAreaTop()  + (Int)(ptCaretPos.y - m_pEditView->GetTextArea().GetViewTopLine()) * m_pEditView->GetTextMetrics().GetHankakuDy()
		+ m_pEditView->GetTextMetrics().GetHankakuDy() - GetCaretSize().cy; //下寄せ

	return CMyPoint(nPosX,nPosY);
}




/*!
	行桁指定によるカーソル移動（座標調整付き）

	@return 縦スクロール行数(負:上スクロール/正:下スクロール)

	@note マウス等による移動で不適切な位置に行かないよう座標調整してカーソル移動する

	@date 2007.08.23 ryoji 関数化（MoveCursorToPoint()から処理を抜き出し）
	@date 2007.09.26 ryoji 半角文字でも中央で左右にカーソルを振り分ける
	@date 2007.10.23 kobake 引数説明の誤りを修正 ([in/out]→[in])
*/
CLayoutInt CCaret::MoveCursorProperly(
	CLayoutPoint	ptNewXY,			//!< [in] カーソルのレイアウト座標X
	bool			bScroll,			//!< [in] TRUE: 画面位置調整有り/ FALSE: 画面位置調整有り無し
	int				nCaretMarginRate,	//!< [in] 縦スクロール開始位置を決める値
	int				dx					//!< [in] nNewXとマウスカーソル位置との誤差(カラム幅未満のドット数)
)
{
	const wchar_t*		pLine;
	CLogicInt		nLineLen;
	const CLayout*	pcLayout;

	if( 0 > ptNewXY.y ){
		ptNewXY.y = CLayoutInt(0);
	}
	
	/* カーソルがテキスト最下端行にあるか */
	if( ptNewXY.y >= m_pEditDoc->m_cLayoutMgr.GetLineCount() ){
		// 2004.04.03 Moca EOFより後ろの座標調整は、MoveCursor内でやってもらうので、削除
	}
	/* カーソルがテキスト最上端行にあるか */
	else if( ptNewXY.y < 0 ){
		ptNewXY.Set(CLayoutInt(0), CLayoutInt(0));
	}
	else{
		/* 移動先の行のデータを取得 */
		pLine = m_pEditDoc->m_cLayoutMgr.GetLineStr( ptNewXY.GetY2(), &nLineLen, &pcLayout );

		int nColWidth = m_pEditView->GetTextMetrics().GetHankakuDx();
		CLayoutInt nPosX = CLayoutInt(0);
		int i = 0;
		CMemoryIterator it( pcLayout, m_pEditDoc->m_cLayoutMgr.GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			if ( it.getIndex() + it.getIndexDelta() > CLogicInt(pcLayout->GetLengthWithoutEOL()) ){
				i = nLineLen;
				break;
			}
			if( it.getColumn() + it.getColumnDelta() > ptNewXY.GetX2() ){
				if (ptNewXY.GetX2() >= (pcLayout ? pcLayout->GetIndent() : CLayoutInt(0)) && ((ptNewXY.GetX2() - it.getColumn()) * nColWidth + dx) * 2 >= it.getColumnDelta() * nColWidth){
				//if (ptNewXY.GetX2() >= (pcLayout ? pcLayout->GetIndent() : CLayoutInt(0)) && (it.getColumnDelta() > CLayoutInt(1)) && ((it.getColumn() + it.getColumnDelta() - ptNewXY.GetX2()) <= it.getColumnDelta() / 2)){
					nPosX += it.getColumnDelta();
				}
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();
		if ( it.end() ){
			i = it.getIndex();
			nPosX -= it.getColumnDelta();
		}

		if( i >= nLineLen ){
			if( ptNewXY.y +1 == m_pEditDoc->m_cLayoutMgr.GetLineCount() &&
				EOL_NONE == pcLayout->m_cEol.GetLen() ){
				nPosX = m_pEditView->LineIndexToColmn( pcLayout, nLineLen );
			}
			/* フリーカーソルモードか */
			else if( m_pEditView->m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode
			  || ( m_pEditView->GetSelectionInfo().IsMouseSelecting() && m_pEditView->GetSelectionInfo().IsBoxSelecting() )	/* マウス範囲選択中 && 矩形範囲選択中 */
			  || ( m_pEditView->m_bDragMode && m_pEditView->GetSelectionInfo().IsBoxSelecting() ) /* OLE DropTarget && 矩形範囲選択中 */
			){
				nPosX = ptNewXY.GetX2();
				//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
				if( nPosX < CLayoutInt(0) ){
					nPosX = CLayoutInt(0);
				}
				else if( nPosX > m_pEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){	/* 折り返し桁数 */
					nPosX = m_pEditDoc->m_cLayoutMgr.GetMaxLineKetas();
				}
			}
		}
		ptNewXY.SetX( nPosX );
	}

	return MoveCursor( ptNewXY, bScroll, nCaretMarginRate );
}
