#include "stdafx.h"
#include "CEditView_Paint.h"
#include "view/CEditView.h"
#include <vector>
#include "CTypeSupport.h"
#include "doc/CLayout.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"

/*
	PAINT_LINENUMBER = (1<<0), //!< 行番号
	PAINT_RULER      = (1<<1), //!< ルーラー
	PAINT_BODY       = (1<<2), //!< 本文
*/

void CEditView_Paint::Call_OnPaint(
	int nPaintFlag,   //!< 描画する領域を選択する
	bool bUseMemoryDC //!< メモリDCを使用する
)
{
	CEditView* pView = GetEditView();

	//各要素
	CMyRect rcLineNumber(0,pView->GetTextArea().GetAreaTop(),pView->GetTextArea().GetAreaLeft(),pView->GetTextArea().GetAreaBottom());
	CMyRect rcRuler(pView->GetTextArea().GetAreaLeft(),0,pView->GetTextArea().GetAreaRight(),pView->GetTextArea().GetAreaTop());
	CMyRect rcBody(pView->GetTextArea().GetAreaLeft(),pView->GetTextArea().GetAreaTop(),pView->GetTextArea().GetAreaRight(),pView->GetTextArea().GetAreaBottom());

	//領域を作成 -> rc
	std::vector<CMyRect> rcs;
	if(nPaintFlag & PAINT_LINENUMBER)rcs.push_back(rcLineNumber);
	if(nPaintFlag & PAINT_RULER)rcs.push_back(rcRuler);
	if(nPaintFlag & PAINT_BODY)rcs.push_back(rcBody);
	if(rcs.size()==0)return;
	CMyRect rc=rcs[0];
	for(int i=1;i<(int)rcs.size();i++)
		rc=MergeRect(rc,rcs[i]);

	//描画
	PAINTSTRUCT	ps;
	ps.rcPaint = rc;
	HDC hdc = pView->GetDC();
	pView->OnPaint( hdc, &ps, bUseMemoryDC );
	pView->ReleaseDC( hdc );
}



/* フォーカス移動時の再描画

	@date 2001/06/21 asa-o 「スクロールバーの状態を更新する」「カーソル移動」削除
*/
void CEditView::RedrawAll()
{
	// ウィンドウ全体を再描画
	PAINTSTRUCT	ps;
	HDC hdc = ::GetDC( GetHwnd() );
	::GetClientRect( GetHwnd(), &ps.rcPaint );
	OnPaint( hdc, &ps, FALSE );	// メモリＤＣを使用してちらつきのない再描画
	::ReleaseDC( GetHwnd(), hdc );

	// キャレットの表示
	GetCaret().ShowEditCaret();

	// キャレットの行桁位置を表示する
	GetCaret().ShowCaretPosInfo();

	// 親ウィンドウのタイトルを更新
	m_pcEditWnd->UpdateCaption();

	//	Jul. 9, 2005 genta	選択範囲の情報をステータスバーへ表示
	GetSelectionInfo().PrintSelectionInfoMsg();

	// スクロールバーの状態を更新する
	AdjustScrollBars();
}

// 2001/06/21 Start by asa-o 再描画
void CEditView::Redraw()
{
	HDC			hdc;
	PAINTSTRUCT	ps;

	hdc = ::GetDC( GetHwnd() );

	::GetClientRect( GetHwnd(), &ps.rcPaint );

	OnPaint( hdc, &ps, FALSE );	/* メモリＤＣを使用してちらつきのない再描画 */

	::ReleaseDC( GetHwnd(), hdc );
}
// 2001/06/21 End



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           括弧                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@date 2003/02/18 ai
	@param flag [in] モード(true:登録, false:解除)
*/
void CEditView::SetBracketPairPos( bool flag )
{
	int	mode;

	// 03/03/06 ai すべて置換、すべて置換後のUndo&Redoがかなり遅い問題に対応
	if( m_bDoing_UndoRedo || !GetDrawSwitch() ){
		return;
	}

	if( !m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// 対括弧の検索&登録
	/*
	bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
	bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
	bit2(out) : 見つかった位置         0:後ろ      1:前
	*/
	mode = 2;

	CLayoutPoint ptColLine;

	if( flag && !GetSelectionInfo().IsTextSelected() && !GetSelectionInfo().m_bDrawSelectArea
		&& !GetSelectionInfo().IsBoxSelecting() && SearchBracket( GetCaret().GetCaretLayoutPos(), &ptColLine, &mode ) )
	{
		// 登録指定(flag=true)			&&
		// テキストが選択されていない	&&
		// 選択範囲を描画していない		&&
		// 矩形範囲選択中でない			&&
		// 対応する括弧が見つかった		場合
		if ( ( ptColLine.x >= GetTextArea().GetViewLeftCol() ) && ( ptColLine.x <= GetTextArea().GetRightCol() )
			&& ( ptColLine.y >= GetTextArea().GetViewTopLine() ) && ( ptColLine.y <= GetTextArea().GetBottomLine() ) )
		{
			// 表示領域内の場合

			// レイアウト位置から物理位置へ変換(強調表示位置を登録)
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptColLine, &m_ptBracketPairPos_PHY );
			m_ptBracketCaretPos_PHY.y = GetCaret().GetCaretLogicPos().y;
			if( 0 == ( mode & 4 ) ){
				// カーソルの後方文字位置
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x;
			}else{
				// カーソルの前方文字位置
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x - 1;
			}
			return;
		}
	}

	// 括弧の強調表示位置情報初期化
	m_ptBracketPairPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));
	m_ptBracketCaretPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));

	return;
}

/*!
	対括弧の強調表示
	@date 2002/09/18 ai
	@date 2003/02/18 ai 再描画対応の為大改造
*/
void CEditView::DrawBracketPair( bool bDraw )
{
	// 03/03/06 ai すべて置換、すべて置換後のUndo&Redoがかなり遅い問題に対応
	if( m_bDoing_UndoRedo || !GetDrawSwitch() ){
		return;
	}

	if( !m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// 括弧の強調表示位置が未登録の場合は終了
	if( m_ptBracketPairPos_PHY.HasNegative() || m_ptBracketCaretPos_PHY.HasNegative() ){
		return;
	}

	// 描画指定(bDraw=true)				かつ
	// ( テキストが選択されている		又は
	//   選択範囲を描画している			又は
	//   矩形範囲選択中					又は
	//   フォーカスを持っていない		又は
	//   アクティブなペインではない )	場合は終了
	if( bDraw
	 &&( GetSelectionInfo().IsTextSelected() || GetSelectionInfo().m_bDrawSelectArea || GetSelectionInfo().IsBoxSelecting() || !m_bDrawBracketPairFlag
	 || ( m_pcEditDoc->m_pcEditWnd->m_nActivePaneIndex != m_nMyIndex ) ) ){
		return;
	}

	HDC			hdc;
	hdc = ::GetDC( GetHwnd() );
	Types *TypeDataPtr = &( m_pcEditDoc->m_cDocType.GetDocumentAttribute() );

	for( int i = 0; i < 2; i++ )
	{
		// i=0:カーソル位置の括弧,i=1:対括弧

		CLayoutPoint	ptColLine;

		if( i == 0 ){
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketCaretPos_PHY, &ptColLine );
		}else{
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketPairPos_PHY,  &ptColLine );
		}

		if ( ( ptColLine.x >= GetTextArea().GetViewLeftCol() ) && ( ptColLine.x <= GetTextArea().GetRightCol() )
			&& ( ptColLine.y >= GetTextArea().GetViewTopLine() ) && ( ptColLine.y <= GetTextArea().GetBottomLine() ) )
		{	// 表示領域内の場合
			if( !bDraw && GetSelectionInfo().m_bDrawSelectArea && ( 0 == IsCurrentPositionSelected( ptColLine ) ) )
			{	// 選択範囲描画済みで消去対象の括弧が選択範囲内の場合
				continue;
			}
			const CLayout* pcLayout;
			CLogicInt		nLineLen;
			const wchar_t*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( ptColLine.GetY2(), &nLineLen, &pcLayout );
			if( pLine )
			{
				int		nColorIndex;
				CLogicInt	OutputX = LineColmnToIndex( pcLayout, ptColLine.GetX2() );
				if( bDraw )	{
					nColorIndex = COLORIDX_BRACKET_PAIR;
				}
				else{
					if( IsBracket( pLine, OutputX, CLogicInt(1) ) ){
						// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応
						if( i == 0 ){
							nColorIndex = GetColorIndex( hdc, pcLayout, m_ptBracketCaretPos_PHY.x );
						}else{
							nColorIndex = GetColorIndex( hdc, pcLayout, m_ptBracketPairPos_PHY.x );
						}
					}
					else{
						SetBracketPairPos( false );
						break;
					}
				}


				m_hFontOld = NULL;

				//色設定
				CTypeSupport cTextType(this,COLORIDX_TEXT);
				cTextType.SetFont(hdc);
				cTextType.SetColors(hdc);

				SetCurrentColor( hdc, nColorIndex );

				int nHeight = GetTextMetrics().GetHankakuDy();
				int nLeft = (GetTextArea().GetDocumentLeftClientPointX()) + (Int)ptColLine.x * GetTextMetrics().GetHankakuDx();
				int nTop  = (Int)( ptColLine.GetY2() - GetTextArea().GetViewTopLine() ) * nHeight + GetTextArea().GetAreaTop();

				// 03/03/03 ai カーソルの左に括弧があり括弧が強調表示されている状態でShift+←で選択開始すると
				//             選択範囲内に反転表示されない部分がある問題の修正
				if( ptColLine.x == GetCaret().GetCaretLayoutPos().GetX2() && GetCaret().GetCaretShowFlag() ){
					GetCaret().HideCaret_( GetHwnd() );	// キャレットが一瞬消えるのを防止
					GetTextDrawer().DispText( hdc, nLeft, nTop, &pLine[OutputX], 1 );
					// 2006.04.30 Moca 対括弧の縦線対応
					GetTextDrawer().DispVerticalLines( hdc, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + CLayoutInt(2) ); //※括弧が全角幅である場合を考慮
					GetCaret().ShowCaret_( GetHwnd() );	// キャレットが一瞬消えるのを防止
				}
				else{
					GetTextDrawer().DispText( hdc, nLeft, nTop, &pLine[OutputX], 1 );
					// 2006.04.30 Moca 対括弧の縦線対応
					GetTextDrawer().DispVerticalLines( hdc, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + CLayoutInt(2) ); //※括弧が全角幅である場合を考慮
				}

				if( NULL != m_hFontOld ){
					::SelectObject( hdc, m_hFontOld );
					m_hFontOld = NULL;
				}

				cTextType.RewindFont(hdc);
				cTextType.RewindColors(hdc);

				if( ( m_pcEditDoc->m_pcEditWnd->m_nActivePaneIndex == m_nMyIndex )
					&& ( ( ptColLine.y == GetCaret().GetCaretLayoutPos().GetY() ) || ( ptColLine.y - 1 == GetCaret().GetCaretLayoutPos().GetY() ) ) ){	// 03/02/27 ai 行の間隔が"0"の時にアンダーラインが欠ける事がある為修正
					GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
				}
			}
		}
	}

	::ReleaseDC( GetHwnd(), hdc );
}


//======================================================================
//!対括弧の対応表
//2007.10.16 kobake
struct KAKKO_T{
	wchar_t *sStr;
	wchar_t *eStr;
};
const KAKKO_T g_aKakkos[] = {
	//半角
	{ L"(", L")", },
	{ L"[", L"]", },
	{ L"{", L"}", },
	{ L"<", L">", },
	{ L"｢", L"｣", },
	//全角
	{ L"【", L"】", },
	{ L"『", L"』", },
	{ L"「", L"」", },
	{ L"＜", L"＞", },
	{ L"≪", L"≫", },
	{ L"《", L"》", },
	{ L"（", L"）", },
	{ L"〈", L"〉", },
	{ L"｛", L"｝", },
	{ L"〔", L"〕", },
	{ L"［", L"］", },
	{ L"“", L"”", },
	{ L"〝", L"〟", },
	//終端
	{ NULL, NULL, },
};



//	Jun. 16, 2000 genta
/*!
	@brief 対括弧の検索

	カーソル位置の括弧に対応する括弧を探す。カーソル位置が括弧でない場合は
	カーソルの後ろの文字が括弧かどうかを調べる。

	カーソルの前後いずれもが括弧でない場合は何もしない。

	括弧が半角か全角か、及び始まりか終わりかによってこれに続く4つの関数に
	制御を移す。

	@param LayoutX [in] 検索開始点の物理座標X
	@param LayoutY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param mode [in/out] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
						 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる (このbitを参照)
						 bit2(out) : 見つかった位置         0:後ろ      1:前     (このbitを更新)

	@retval true 成功
	@retval false 失敗

	@author genta
	@date Jun. 16, 2000 genta
	@date Feb. 03, 2001 MIK 全角括弧に対応
	@date Sep. 18, 2002 ai modeの追加
*/
bool CEditView::SearchBracket(
	const CLayoutPoint&	ptLayout,
	CLayoutPoint*		pptLayoutNew,
	int*				mode
)
{
	CLogicInt len;	//	行の長さ
	
	CLogicPoint ptPos;
	//int PosX, PosY;	//	物理位置

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptLayout, &ptPos );
	const wchar_t *cline = m_pcEditDoc->m_cDocLineMgr.GetLine(ptPos.GetY2())->GetDocLineStrWithEOL(&len);

	//	Jun. 19, 2000 genta
	if( cline == NULL )	//	最後の行に本文がない場合
		return false;

	// 括弧処理 2007.10.16 kobake
	{
		const KAKKO_T* p;
		for( p = g_aKakkos; p->sStr != NULL;  p++ )
		{
			if( wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketForward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
			else if( wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketBackward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
		}
	}

	// 02/09/18 ai Start
	if( 0 == ( *mode & 2 ) ){
		/* カーソルの前方を調べない場合 */
		return false;
	}
	*mode |= 4;
	// 02/09/18 ai End

	//	括弧が見つからなかったら，カーソルの直前の文字を調べる

	if( ptPos.x <= 0 ){
//		::MessageBoxA( NULL, "NO DATA", "Bracket", MB_OK );
		return false;	//	前の文字はない
	}

	const wchar_t *bPos = CNativeW::GetCharPrev( cline, ptPos.x, cline + ptPos.x );
	int nCharSize = cline + ptPos.x - bPos;
	// 括弧処理 2007.10.16 kobake
	if(nCharSize==1){
		const KAKKO_T* p;
		ptPos.x = bPos - cline;
		for( p = g_aKakkos; p->sStr != NULL; p++ )
		{
			if( wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketForward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
			else if( wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketBackward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
		}
	}
	return false;
}

/*!
	@brief 半角対括弧の検索:順方向

	@author genta

	@param PosX   [in] 検索開始点の物理座標X
	@param PosY   [in] 検索開始点の物理座標Y
	@param NewX   [out] 移動先のレイアウト座標X
	@param NewY   [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字
	@param dnChar [in] 括弧を閉じる文字列
	@param mode   [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
// 03/01/08 ai
bool CEditView::SearchBracketForward(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	upChar,
	const wchar_t*	dnChar,
	int*			mode
)
{
	CDocLine* ci;

	int			len;
	const wchar_t* cPos;
	const wchar_t* nPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int			level = 0;

	CLayoutPoint ptColLine;

	CLayoutInt	nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ( GetTextArea().GetBottomLine() ) - ptColLine.y;					// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos < lineend ){
			nPos = CNativeW::GetCharNext( cline, len, cPos );
			if( nPos - cPos > 1 ){
				//	skip
				cPos = nPos;
				continue;
			}
			// 03/01/08 ai Start
			if( wcsncmp(upChar, cPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, cPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	見つかった！
				ptPos.x = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
				//	Happy Ending
			}
			cPos = nPos;	//	次の文字へ
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// 表示領域外を調べないモードで表示領域の終端の場合
			//SendStatusMessage( "対括弧の検索を中断しました" );
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y++;
		ci = ci->GetNextLine();	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}

/*!
	@brief 半角対括弧の検索:逆方向

	@author genta

	@param PosX [in] 検索開始点の物理座標X
	@param PosY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字
	@param dnChar [in] 括弧を閉じる文字列
	@param mode [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchBracketBackward(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	dnChar,
	const wchar_t*	upChar,
	int*			mode
)
{
	CDocLine* ci;

	int			len;
	const wchar_t* cPos;
	const wchar_t* pPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int			level = 1;
	
	CLayoutPoint ptColLine;
	//int			nCol, nLine;

	CLayoutInt		nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ptColLine.y - GetTextArea().GetViewTopLine();										// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos > cline ){
			pPos = CNativeW::GetCharPrev( cline, len, cPos );
			if( cPos - pPos > 1 ){
				//	skip
				cPos = pPos;
				continue;
			}
			// 03/01/08 ai Start
			if( wcsncmp(upChar, pPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, pPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	見つかった！
				ptPos.x = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
				//	Happy Ending
			}
			cPos = pPos;	//	次の文字へ
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// 表示領域外を調べないモードで表示領域の先頭の場合
			//SendStatusMessage( "対括弧の検索を中断しました" );
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y--;
		ci = ci->GetPrevLine();	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}

//@@@ 2001.02.03 Start by MIK:
/*!
	@brief 全角対括弧の検索:順方向

	@author MIK

	@param PosX [in] 検索開始点の物理座標X
	@param PosY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字へのポインタ
	@param dnChar [in] 括弧を閉じる文字列へのポインタ
	@param mode [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchBracketForward2(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	upChar,
	const wchar_t*	dnChar,
	int*			mode
)
{
	CDocLine* ci;

	int len;
	const wchar_t* cPos;
	const wchar_t* nPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int level = 0;
	
	CLayoutPoint ptColLine;
//	int			nCol, nLine;

	CLayoutInt	nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ( GetTextArea().GetBottomLine() ) - ptColLine.y;					// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos < lineend ){
			nPos = CNativeW::GetCharNext( cline, len, cPos );
			if( nPos - cPos != 1 ){
				//	skip
				cPos = nPos;
				continue;
			}
			if( wcsncmp(upChar, cPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, cPos, 1) == 0 ){
				--level;
			}

			if( level == 0 ){	//	見つかった！
				ptPos.x = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
			}
			cPos = nPos;	//	次の文字へ
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// 表示領域外を調べないモードで表示領域の終端の場合
			//SendStatusMessage( "対括弧の検索を中断しました" );
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y++;
		ci = ci->GetNextLine();	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}
//@@@ 2001.02.03 End

//@@@ 2001.02.03 Start by MIK:
/*!
	@brief 全角対括弧の検索:逆方向

	@author MIK

	@param PosX [in] 検索開始点の物理座標X
	@param PosY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字へのポインタ
	@param dnChar [in] 括弧を閉じる文字列へのポインタ
	@param mode [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchBracketBackward2(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	dnChar,
	const wchar_t*	upChar,
	int*			mode
)
{
	CDocLine* ci;

	int len;
	const wchar_t* cPos;
	const wchar_t* pPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int level = 1;

	CLayoutPoint ptColLine;
	//int	nCol, nLine;

	CLayoutInt nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ptColLine.y - GetTextArea().GetViewTopLine();										// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos > cline ){
			pPos = CNativeW::GetCharPrev( cline, len, cPos );
			if( cPos - pPos != 1 ){
				//	skip
				cPos = pPos;
				continue;
			}
			if( wcsncmp(upChar, pPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, pPos, 1) == 0 ){
				--level;
			}

			if( level == 0 ){	//	見つかった！
				ptPos.x = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
			}
			cPos = pPos;	//	次の文字へ
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// 表示領域外を調べないモードで表示領域の先頭の場合
			//SendStatusMessage( "対括弧の検索を中断しました" );
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y--;
		ci = ci->GetPrevLine();	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}
//@@@ 2001.02.03 End

//@@@ 2003.01.09 Start by ai:
/*!
	@brief 括弧判定

	@author ai

	@param pLine [in] 
	@param x
	@param size

	@retval true 括弧
	@retval false 非括弧
*/
bool CEditView::IsBracket( const wchar_t *pLine, CLogicInt x, CLogicInt size )
{
	// 括弧処理 2007.10.16 kobake
	if( size == 1 ){
		const struct KAKKO_T *p;
		for( p = g_aKakkos; p->sStr != NULL; p++ )
		{
			if( wcsncmp( p->sStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
			else if( wcsncmp( p->eStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
		}
	}

	return false;
}
//@@@ 2003.01.09 End


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          色設定                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! 指定位置のColorIndexの取得
	CEditView::DispLineNewを元にしたためCEditView::DispLineNewに
	修正があった場合は、ここも修正が必要。

	@par nCOMMENTMODE
	関数内部で状態遷移のために使われる変数nCOMMENTMODEと状態の関係。
 - COLORIDX_TEXT     : テキスト
 - COLORIDX_COMMENT  : 行コメント
 - COLORIDX_BLOCK1   : ブロックコメント1
 - COLORIDX_SSTRING  : シングルコーテーション
 - COLORIDX_WSTRING  : ダブルコーテーション
 - COLORIDX_KEYWORD1 : 強調キーワード1
 - COLORIDX_CTRLCODE : コントロールコード
 - COLORIDX_DIGIT    : 半角数値
 - COLORIDX_BLOCK2   : ブロックコメント2
 - COLORIDX_KEYWORD2 : 強調キーワード2
 - COLORIDX_URL      : URL
 - COLORIDX_SEARCH   : 検索
 - 1000: 正規表現キーワード
 	色指定SetCurrentColorを呼ぶときにCOLORIDX_*値を加算するので、
 	1000～COLORIDX_LASTまでは正規表現で使用する。
*/
int CEditView::GetColorIndex(
		HDC						hdc,
		const CLayout*			pcLayout,
		int						nCol
)
{
	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->m_cDocType.GetDocumentAttribute());

	const wchar_t*			pLine;	//@@@ 2002.09.22 YAZAKI
	CLogicInt				nLineLen;
	int						nLineBgn;
	int						nCOMMENTMODE;
	int						nCOMMENTMODE_OLD;
	int						nCOMMENTEND;
	int						nCOMMENTEND_OLD;
	const CLayout*			pcLayout2;
	int						i, j;
	int						nIdx;
	int						nUrlLen;
	BOOL					bSearchStringMode;
	bool					bSearchFlg;			// 2002.02.08 hor
	bool					bKeyWordTop = true;	//	Keyword Top
	int						nColorIndex;

//@@@ 2001.11.17 add start MIK
	int		nMatchLen;
	int		nMatchColor;
//@@@ 2001.11.17 add end MIK

	bSearchStringMode = FALSE;
	bSearchFlg	= true;	// 2002.02.08 hor

	CLogicInt	nSearchStart = CLogicInt(-1);
	CLogicInt	nSearchEnd   = CLogicInt(-1);

	/* 論理行データの取得 */
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory変更
		nLineLen = pcLayout->GetDocLineRef()->GetLengthWithEOL()/* - pcLayout->GetLogicOffset()*/;	// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応
		pLine = pcLayout->GetPtr()/* + pcLayout->GetLogicOffset()*/;			// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応

		// 2005.11.20 Moca 色が正しくないことがある問題に対処
		const CLayout* pcLayoutLineFirst = pcLayout;
		// 論理行の最初のレイアウト情報を取得する
		while( 0 != pcLayoutLineFirst->GetLogicOffset() ){
			pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
		}
		nCOMMENTMODE = pcLayoutLineFirst->GetColorTypePrev();
		nCOMMENTEND = 0;
		pcLayout2 = pcLayout;

	}else{
		pLine = NULL;
		nLineLen = CLogicInt(0);
		nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		nCOMMENTEND = 0;
		pcLayout2 = NULL;
	}

	/* 現在の色を指定 */
	//@SetCurrentColor( hdc, nCOMMENTMODE );
	nColorIndex = nCOMMENTMODE;	// 02/12/18 ai

	int						nBgn;
	CLogicInt				nPos;
	nBgn = 0;
	nPos = CLogicInt(0);
	nLineBgn = 0;

	CLogicInt				nCharChars = CLogicInt(0);
	CLogicInt				nCharChars_2;

	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		while( nPos <= nCol ){	// 03/10/24 ai 行頭のColorIndexが取得できない問題に対応

			nBgn = nPos;
			nLineBgn = nBgn;

			while( nPos - nLineBgn <= nCol ){	// 02/12/18 ai
				/* 検索文字列の色分け */
				if( m_bCurSrchKeyMark	/* 検索文字列のマーク */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
				// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
					if(!bSearchStringMode && (!m_sCurSearchOption.bRegularExp || (bSearchFlg && nSearchStart < nPos))){
						bSearchFlg=IsSearchString( pLine, nLineLen, nPos, &nSearchStart, &nSearchEnd );
					}
					if( !bSearchStringMode && bSearchFlg && nSearchStart==nPos ){
						nBgn = nPos;
						bSearchStringMode = TRUE;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
						nColorIndex = COLORIDX_SEARCH;	// 02/12/18 ai
					}
					else if( bSearchStringMode && nSearchEnd == nPos ){
						nBgn = nPos;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, nCOMMENTMODE );
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						bSearchStringMode = FALSE;
						goto searchnext;
					}
				}

				if( nPos >= nLineLen - pcLayout2->GetLayoutEol().GetLen() ){
					goto end_of_line;
				}
				SEARCH_START:;
				switch( nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
//@@@ 2001.11.17 add start MIK
					//正規表現キーワード
					if( TypeDataPtr->m_bUseRegexKeyword
					 && m_cRegexKeyword->RegexIsKeyword( pLine, nPos, nLineLen, &nMatchLen, &nMatchColor )
					 /*&& TypeDataPtr->m_ColorInfoArr[nMatchColor].m_bDisp*/ )
					{
						/* 現在の色を指定 */
						nBgn = nPos;
						nCOMMENTMODE = MakeColorIndexType_RegularExpression(nMatchColor);	/* 色指定 */	//@@@ 2002.01.04 upd
						nCOMMENTEND = nPos + nMatchLen;  /* キーワード文字列の終端をセットする */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}
					else
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;

						nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}else
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI

//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
					}else
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
//#endif
					}else
					if( pLine[nPos] == L'\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* シングルクォーテーション文字列を表示する */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( pLine[nPos] == L'"' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* ダブルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URLを表示する */
					 && ( TRUE == IsURL( &pLine[nPos], nLineLen - nPos, &nUrlLen ) )	/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_URL;	/* URLモード */ // 2002/03/13 novice
						nCOMMENTEND = nPos + nUrlLen;
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 Start by MIK: 半角数値を強調表示
					}else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (i = IsNumber( pLine, nPos, nLineLen )) > 0 )		/* 半角数字を表示する */
					{
						/* キーワード文字列の終端をセットする */
						i = nPos + i;
						/* 現在の色を指定 */
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_DIGIT;	/* 半角数値である */ // 2002/03/13 novice
						nCOMMENTEND = i;
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 End by MIK: 半角数値を強調表示
					}else
					if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* キーワードセット */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* 強調キーワードを表示する */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						//	Mar 4, 2001 genta comment out
						/* キーワード文字列の終端を探す */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							if( IS_KEYWORD_CHAR( pLine[i] ) ){
							}else{
								break;
							}
						}
						/* キーワードが登録単語ならば、色を変える */
						j = i - nPos;
						/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
						nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
							TypeDataPtr->m_nKeyWordSetIdx[0],
							&pLine[nPos],
							j
						);
						if( nIdx != -1 ){
							/* 現在の色を指定 */
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_KEYWORD1;	/* 強調キーワード1 */ // 2002/03/13 novice
							nCOMMENTEND = i;
							if( !bSearchStringMode ){
								//@SetCurrentColor( hdc, nCOMMENTMODE );
								nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
							}
						}else{		//MIK START ADD 2000.12.01 second keyword & binary search
							// 2005.01.13 MIK 強調キーワード数追加に伴う配列化
							for( int my_i = 1; my_i < 10; my_i++ )
							{
								if(TypeDataPtr->m_nKeyWordSetIdx[my_i] != -1 && /* キーワードセット */							//MIK 2000.12.01 second keyword
									TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + my_i].m_bDisp)									//MIK
								{																							//MIK
									/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */						//MIK
									nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
										TypeDataPtr->m_nKeyWordSetIdx[my_i] ,													//MIK
										&pLine[nPos],																		//MIK
										j																					//MIK
									);																						//MIK
									if( nIdx != -1 ){																		//MIK
										/* 現在の色を指定 */																//MIK
										nBgn = nPos;																		//MIK
										nCOMMENTMODE = COLORIDX_KEYWORD1 + my_i;	/* 強調キーワード2 */ // 2002/03/13 novice		//MIK
										nCOMMENTEND = i;																	//MIK
										if( !bSearchStringMode ){															//MIK
											//@SetCurrentColor( hdc, nCOMMENTMODE );										//MIK
											nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
										}																					//MIK
										break;
									}																						//MIK
								}																							//MIK
								else
								{
									if(TypeDataPtr->m_nKeyWordSetIdx[my_i] == -1 )
										break;
								}
							}
						}			//MIK END
					}
					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pLine[nPos] ))	bKeyWordTop = false;
					else								bKeyWordTop = true;
					//	To Here
					break;
// 2002/03/13 novice
				case COLORIDX_URL:		/* URLモードである */
				case COLORIDX_KEYWORD1:	/* 強調キーワード1 */
				case COLORIDX_DIGIT:	/* 半角数値である */  //@@@ 2001.02.17 by MIK
				case COLORIDX_KEYWORD2:	/* 強調キーワード2 */	//MIK
				case COLORIDX_KEYWORD3:
				case COLORIDX_KEYWORD4:
				case COLORIDX_KEYWORD5:
				case COLORIDX_KEYWORD6:
				case COLORIDX_KEYWORD7:
				case COLORIDX_KEYWORD8:
				case COLORIDX_KEYWORD9:
				case COLORIDX_KEYWORD10:
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* コントロールコード */ // 2002/03/13 novice
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = nCOMMENTMODE_OLD;
						nCOMMENTEND = nCOMMENTEND_OLD;
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;

				case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ダブルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 ){	//正規表現キーワード1～10
						if( nPos == nCOMMENTEND ){
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* 現在の色を指定 */
							if( !bSearchStringMode ){
								//@SetCurrentColor( hdc, nCOMMENTMODE );
								nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}
				if( pLine[nPos] == WCODE::TAB ){
					nBgn = nPos + 1;
					nCharChars = CLogicInt(1);
				}
				else if( WCODE::isZenkakuSpace(pLine[nPos]) && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )	//@@@ 2002.01.04
				{
					nBgn = nPos + 1;
					nCharChars = CLogicInt(1);
				}
				//半角空白（半角スペース）を表示 2002.04.28 Add by KK 
				else if (pLine[nPos] == L' ' && CTypeSupport(this,COLORIDX_SPACE).IsDisp() && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{
					nBgn = nPos + 1;
					nCharChars = CLogicInt(1);
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = CLogicInt(1);
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* コントロールコードを色分け */
					 && WCODE::isControlCode(pLine[nPos])
					){
						nBgn = nPos;
						nCOMMENTMODE_OLD = nCOMMENTMODE;
						nCOMMENTEND_OLD = nCOMMENTEND;
						nCOMMENTMODE = COLORIDX_CTRLCODE;	/* コントロールコード モード */ // 2002/03/13 novice
						/* コントロールコード列の終端を探す */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if(!WCODE::isControlCode(pLine[i])){
								break;
							}
						}
						nCOMMENTEND = i;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, nCOMMENTMODE );
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
					}
				}
				nPos+= nCharChars;
			} //end of while( nPos - nLineBgn < pcLayout2->m_nLength ){
			if( nPos > nCol ){	// 03/10/24 ai 行頭のColorIndexが取得できない問題に対応
				break;
			}
		}

end_of_line:;

	}

//@end_of_func:;
	return nColorIndex;
}


/* 現在の色を指定 */
void CEditView::SetCurrentColor( HDC hdc, int nCOMMENTMODE )
{
	int				nColorIdx;
	COLORREF		colText;
	COLORREF		colBack;
//	if( NULL != m_hFontOld ){
//		::SelectObject( hdc, m_hFontOld );
//		m_hFontOld = NULL;
//	}
	nColorIdx = -1;
	switch( nCOMMENTMODE ){
// 2002/03/13 novice
	case COLORIDX_TEXT: /* テキスト */
	case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */
	case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */
	case COLORIDX_CTRLCODE:	/* コントロールコードである */
	case COLORIDX_DIGIT:	/* 半角数値である */
	case COLORIDX_KEYWORD1:	/* 強調キーワード1（登録単語）文字列である */
	case COLORIDX_KEYWORD2:	/* 強調キーワード2（登録単語）文字列である */
	case COLORIDX_KEYWORD3:	// 2005.01.13 MIK 強調キーワード3-10
	case COLORIDX_KEYWORD4:
	case COLORIDX_KEYWORD5:
	case COLORIDX_KEYWORD6:
	case COLORIDX_KEYWORD7:
	case COLORIDX_KEYWORD8:
	case COLORIDX_KEYWORD9:
	case COLORIDX_KEYWORD10:
	case COLORIDX_URL:	    /* URLである */
	case COLORIDX_SEARCH:	/* 検索文字列である */
	case COLORIDX_BRACKET_PAIR: /* 対括弧の強調表示である */	// 2002/12/18 ai
		nColorIdx = nCOMMENTMODE;
		break;
	case COLORIDX_COMMENT:	/* 行コメントである */
	case COLORIDX_BLOCK1:	/* ブロックコメント1である */
	case COLORIDX_BLOCK2:	/* ブロックコメント2である */	//@@@ 2001.03.10 by MIK
		nColorIdx = COLORIDX_COMMENT;
		break;

//@@@ 2001.11.17 add start MIK
	default:	/* 正規表現キーワード */
		if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 )
		{
			nColorIdx = nCOMMENTMODE - 1000;	//下駄を履かせているのをはずす
			colText = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* フォントを選ぶ */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				GetFontset().ChooseFontHandle(
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
			return;
		}
		break;
//@@@ 2001.11.17 add end MIK

	}

	if( -1 != nColorIdx ){
		if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bDisp ){
			colText = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* フォントを選ぶ */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				GetFontset().ChooseFontHandle(
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
		}
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           描画                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 通常の描画処理 new
void CEditView::OnPaint( HDC hdc, PAINTSTRUCT *pPs, BOOL bUseMemoryDC )
{
//	MY_RUNNINGTIMER( cRunningTimer, "CEditView::OnPaint" );

	// 2004.01.28 Moca デスクトップに作画しないように
	if( NULL == hdc )return;

	if( !GetDrawSwitch() )return;

	// キャレットを隠す
	GetCaret().HideCaret_( this->GetHwnd() ); // 2002/07/22 novice

	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->m_cDocType.GetDocumentAttribute());

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	RECT			rc;
	int				nLineHeight = GetTextMetrics().GetHankakuDy();
	int				nCharDx = GetTextMetrics().GetHankakuDx();
	const CLayout*	pcLayout;

	//サポート
	CTypeSupport cTextType(this,COLORIDX_TEXT);

//@@@ 2001.11.17 add start MIK
	//変更があればタイプ設定を行う。
	if( TypeDataPtr->m_bUseRegexKeyword || m_cRegexKeyword->m_bUseRegexKeyword ) //OFFなのに前回のデータが残ってる
	{
		//タイプ別設定をする。設定済みかどうかは呼び先でチェックする。
		m_cRegexKeyword->RegexKeySetTypes(TypeDataPtr);
	}
//@@@ 2001.11.17 add end MIK

	// メモリＤＣを利用した再描画の場合は描画先のＤＣを切り替える
	HDC hdcOld;
	bUseMemoryDC = FALSE;
	if( bUseMemoryDC ){
		hdcOld = hdc;
		hdc = m_hdcCompatDC;
	}

	/* 03/02/18 対括弧の強調表示(消去) ai */
	DrawBracketPair( false );

	/* ルーラーとテキストの間の余白 */
	//@@@ 2002.01.03 YAZAKI 余白が0のときは無駄でした。
	if ( GetTextArea().GetTopYohaku() ){
		rc.left   = 0;
		rc.top    = GetTextArea().GetRulerHeight();
		rc.right  = GetTextArea().GetAreaRight();
		rc.bottom = GetTextArea().GetAreaTop();

		cTextType.FillBack(hdc,rc);
	}
	
	/* 行番号の表示 */
	//	From Here Sep. 7, 2001 genta
	//	Sep. 23, 2002 genta 行番号非表示でも行番号色の帯があるので隙間を埋める
	if( GetTextArea().GetTopYohaku() ){ 
		rc.left   = 0;
		rc.top    = GetTextArea().GetRulerHeight();
		rc.right  = GetTextArea().GetAreaLeft() - m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace; //	Sep. 23 ,2002 genta 余白はテキスト色のまま残す
		rc.bottom = GetTextArea().GetAreaTop();
		HBRUSH hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_GYOU].m_colBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}
	//	To Here Sep. 7, 2001 genta

	::SetBkMode( hdc, TRANSPARENT );
	m_hFontOld = NULL;

	cTextType.SetFont(hdc);
	cTextType.SetColors(hdc);


	int nTop = pPs->rcPaint.top;
	BOOL bEOF = FALSE;



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//           描画開始行 -> nLayoutLine             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt nLayoutLine;
	if( 0 > nTop - GetTextArea().GetAreaTop() ){
		nLayoutLine = GetTextArea().GetViewTopLine(); //ビュー上部から描画
	}else{
		nLayoutLine = GetTextArea().GetViewTopLine() + CLayoutInt( ( nTop - GetTextArea().GetAreaTop() ) / nLineHeight ); //ビュー途中から描画
	}

	int nMaxRollBackLineNum = 260 / (Int)nWrapKeta + 1;
	int nRollBackLineNum = 0;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLayoutLine );
	while( nRollBackLineNum < nMaxRollBackLineNum ){
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLayoutLine );
		if( NULL == pcLayout ){
			break;
		}
		if( 0 == pcLayout->GetLogicOffset() ){	/* 対応する論理行の先頭からのオフセット */
			break;
		}
		nLayoutLine--;
		nRollBackLineNum++;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          描画終了行 -> nLogicLineTo            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt nLogicLineTo = GetTextArea().GetViewTopLine() + CLayoutInt( ( pPs->rcPaint.bottom - GetTextArea().GetAreaTop() ) / nLineHeight );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         描画座標                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	DispPos sPos(GetTextMetrics().GetHankakuDx(),GetTextMetrics().GetHankakuDy());
	sPos.InitDrawPos(CMyPoint(
		GetTextArea().GetAreaLeft() - (Int)GetTextArea().GetViewLeftCol() * nCharDx,
		GetTextArea().GetAreaTop() + (Int)( nLayoutLine - GetTextArea().GetViewTopLine() ) * nLineHeight
	));
	sPos.SetLayoutLineRef(nLayoutLine);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      全部の行を描画                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//必要な行を描画する
	bool bSelected = GetSelectionInfo().IsTextSelected();
	while(sPos.GetLayoutLineRef() <= nLogicLineTo)
	{
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( sPos.GetLayoutLineRef() );
	
		sPos.ResetDrawCol();

		bool bDispResult = DispLineNew(
			hdc,
			pcLayout,
			&sPos,
			nLogicLineTo,
			bSelected
		);

		if(bDispResult){
			pPs->rcPaint.bottom += nLineHeight;	// EOF再描画対応
			bEOF = TRUE;
			break;
		}

		if( NULL == pcLayout ){
			bEOF = TRUE;
			break;
		}
	}
	if( NULL != m_hFontOld ){
		::SelectObject( hdc, m_hFontOld );
		m_hFontOld = NULL;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              テキストの無い部分の塗りつぶし                 //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if( IsBkBitmap() ){
	}else{
		/* テキストのない部分を背景色で塗りつぶす */
		if( sPos.GetDrawPos().y < pPs->rcPaint.bottom ){
			RECT rcBack;
			rcBack.left   = pPs->rcPaint.left;
			rcBack.right  = pPs->rcPaint.right;
			rcBack.top    = sPos.GetDrawPos().y;
			rcBack.bottom = pPs->rcPaint.bottom;

			cTextType.FillBack(hdc,rcBack);

			// 2006.04.29 行部分は行ごとに作画し、ここでは縦線の残りを作画
			GetTextDrawer().DispVerticalLines( hdc, sPos.GetDrawPos().y, pPs->rcPaint.bottom, CLayoutInt(0), CLayoutInt(-1) );
		}
	}

	cTextType.RewindColors(hdc);
	cTextType.RewindFont(hdc);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                折り返し位置の表示 (縦線)                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CTypeSupport cWrapType(this,COLORIDX_WRAP);
	if( cWrapType.IsDisp() ){
		int nXPos = GetTextArea().GetAreaLeft() + (Int)( nWrapKeta - GetTextArea().GetViewLeftCol() ) * nCharDx;
		//	2005.11.08 Moca 作画条件変更
		if( GetTextArea().GetAreaLeft() < nXPos && nXPos < GetTextArea().GetAreaRight() ){
			/// 折り返し記号の色のペンを設定
			cWrapType.SetSolidPen(hdc,0);

			::MoveToEx( hdc, nXPos, GetTextArea().GetAreaTop(), NULL );
			::LineTo( hdc, nXPos, GetTextArea().GetAreaBottom() );

			cWrapType.RewindPen(hdc);
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ルーラー描画                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if ( pPs->rcPaint.top < GetTextArea().GetRulerHeight() ) { // ルーラーが再描画範囲にあるときのみ再描画する 2002.02.25 Add By KK
		GetRuler().SetRedrawFlag(); //2002.02.25 Add By KK ルーラー全体を描画。
		GetRuler().DispRuler( hdc );
	}

	if ( m_pcEditWnd->m_nActivePaneIndex == m_nMyIndex ){
		/* アクティブペインは、アンダーライン描画 */
		GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     その他後始末など                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	/* メモリＤＣを利用した再描画の場合はメモリＤＣに描画した内容を画面へコピーする */
	if( bUseMemoryDC ){
		::BitBlt(
			hdcOld,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			hdc,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
	}

	/* 03/02/18 対括弧の強調表示(描画) ai */
	DrawBracketPair( true );

	/* キャレットを現在位置に表示します */
	GetCaret().ShowCaret_( this->GetHwnd() ); // 2002/07/22 novice
	return;
}









/*
2002/03/13 novice
@par nCOMMENTMODE
関数内部で状態遷移のために使われる変数nCOMMENTMODEと状態の関係。
 - COLORIDX_TEXT     : テキスト
 - COLORIDX_COMMENT  : 行コメント
 - COLORIDX_BLOCK1   : ブロックコメント1
 - COLORIDX_SSTRING  : シングルコーテーション
 - COLORIDX_WSTRING  : ダブルコーテーション
 - COLORIDX_KEYWORD1 : 強調キーワード1
 - COLORIDX_CTRLCODE : コントロールコード
 - COLORIDX_DIGIT    : 半角数値
 - COLORIDX_BLOCK2   : ブロックコメント2
 - COLORIDX_KEYWORD2 : 強調キーワード2
 - COLORIDX_URL      : URL
 - COLORIDX_SEARCH   : 検索
 - 1000: 正規表現キーワード
 	色指定SetCurrentColorを呼ぶときにCOLORIDX_*値を加算するので、
 	1000～COLORIDX_LASTまでは正規表現で使用する。
*/

//@@@ 2001.02.17 MIK
//@@@ 2001.12.21 YAZAKI 改行記号の描きかたを変更
//@@@ 2007.08.31 kobake 引数 bDispBkBitmap を削除
/*!
	行のテキスト／選択状態の描画
	1回で1論理行分を作画する。

	@return EOFを作画したらtrue
*/
bool CEditView::DispLineNew(
	HDC				hdc,			//!< [in]     作画対象
	const CLayout*	pcLayout,		//!< [in]     表示を開始するレイアウト
	DispPos*		pDispPos,		//!< [in/out] 描画する箇所、描画元ソース
	CLayoutInt		nLineTo,		//!< [in]     作画終了するレイアウト行番号
	bool			bSelected		//!< [in]     選択中か
)
{
//	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DispLineNew" );

	//DispPosを保存しておく
	DispPos sDispPosBegin = *pDispPos;

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	int		nCharChars_2;
	EColorIndexType		nCOMMENTMODE;
	EColorIndexType		nCOMMENTMODE_OLD;
	int		nCOMMENTEND;
	int		nCOMMENTEND_OLD;

	bool	bEOF = false;
	int		i, j;
	int		nIdx;
	int		nUrlLen;
	int		nColorIdx;
	bool	bKeyWordTop = true;	//	Keyword Top

	//サイズ
	Types* TypeDataPtr = &m_pcEditDoc->m_cDocType.GetDocumentAttribute();
	int nLineHeight = GetTextMetrics().GetHankakuDy();  //行の縦幅？
	int nCharDx  = GetTextMetrics().GetHankakuDx();  //半角

	bool bSearchStringMode = false; //検索ヒットフラグ？これで色分けを判定する
	BOOL bSearchFlg        = TRUE;  //？ 2002.02.08 hor
	CLogicInt  nSearchStart      = CLogicInt(-1);    //？ 2002.02.08 hor
	CLogicInt  nSearchEnd        = CLogicInt(-1);    //？ 2002.02.08 hor

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          論理行データの取得 -> pLine, pLineLen              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	const CLayout*	pcLayout2; //ワーク用CLayoutポインタ
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory変更
		nLineLen = pcLayout->GetDocLineRef()->GetLengthWithEOL() - pcLayout->GetLogicOffset();
		pLine    = pcLayout->GetDocLineRef()->GetPtr() + pcLayout->GetLogicOffset();

		// タイプ
		// 0=通常
		// 1=行コメント
		// 2=ブロックコメント
		// 3=シングルクォーテーション文字列
		// 4=ダブルクォーテーション文字列
		nCOMMENTMODE = pcLayout->GetColorTypePrev();
		nCOMMENTEND = 0;
		pcLayout2 = pcLayout;
	}
	else{
		pLine = NULL;
		nLineLen = CLogicInt(0);
		nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		nCOMMENTEND = 0;

		pcLayout2 = NULL;
	}

	/* 現在の色を指定 */
	SetCurrentColor( hdc, nCOMMENTMODE );

	int nLineBgn   = 0; //？
//	int nX         = 0; //テキストX位置
	CLogicInt nCharChars = CLogicInt(0); //処理した文字数

	//処理する文字位置
	CLogicInt nPos = CLogicInt(0);
	#define SetNPos(N) nPos=(N)
	#define GetNPos() (nPos+CLogicInt(0))

	//通常テキスト開始位置 (ほぼ固定)
	int nBgn = 0;
	#define SetNBgn(N) nBgn=(N)
	#define GetNBgn() (nBgn+0)

	//サポート
	CTypeSupport cTextType(this,COLORIDX_TEXT);
	CTypeSupport cSearchType(this,COLORIDX_SEARCH);

	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		pDispPos->ForwardDrawLine(-1);
		pDispPos->ForwardLayoutLineRef(-1);

		while( GetNPos() < nLineLen ){
			pDispPos->ForwardDrawLine(1);
			pDispPos->ForwardLayoutLineRef(1);
			if( GetTextArea().GetBottomLine() < pDispPos->GetLayoutLineRef() ){
				pDispPos->SetLayoutLineRef(nLineTo + CLayoutInt(1));
				goto end_of_func;
			}
			if( nLineTo < pDispPos->GetLayoutLineRef() ){
				goto end_of_func;
			}

			pcLayout2 = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( pDispPos->GetLayoutLineRef() );

			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                        行番号描画                           //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
				// 行番号表示
				GetTextDrawer().DispLineNumber( hdc, pcLayout2, (Int)pDispPos->GetLayoutLineRef(), pDispPos->GetDrawPos().y );
			}


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                       本文描画開始                          //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			SetNBgn(GetNPos());
			nLineBgn = GetNBgn();
			pDispPos->ResetDrawCol();


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                 行頭(インデント)背景描画                    //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			if(pcLayout2 && pcLayout2->GetIndent()!=0)
			{
				RECT rcClip;
				if(GetTextArea().GenerateClipRect(&rcClip,*pDispPos,(Int)pcLayout2->GetIndent())){
					cTextType.FillBack(hdc,rcClip);
				}
				//描画位置進める
				pDispPos->ForwardDrawCol((Int)pcLayout2->GetIndent());
			}


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                 通常文字列以外描画ループ                    //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//行終端または折り返しに達するまでループ
			while( GetNPos() - nLineBgn < pcLayout2->GetLengthWithEOL() ){
				/* 検索文字列の色分け */
				/* 検索文字列のマーク */
				if( m_bCurSrchKeyMark && cSearchType.IsDisp() ){
searchnext:;
					// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
					//        検索ヒットフラグ設定 -> bSearchStringMode            //
					// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

					// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
					if(!bSearchStringMode && (!m_sCurSearchOption.bRegularExp || (bSearchFlg && nSearchStart < GetNPos()))){
						bSearchFlg=IsSearchString( pLine, nLineLen, GetNPos(), &nSearchStart, &nSearchEnd );
					}
					if( !bSearchStringMode && bSearchFlg && nSearchStart==GetNPos()){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						bSearchStringMode = true;

						// 現在の色を指定
						SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
					}
					else if( bSearchStringMode && nSearchEnd <= GetNPos() ){ //+ == では行頭文字の場合、nSearchEndも０であるために文字色の解除ができないバグを修正 2003.05.03 かろと
						// 検索した文字列の終わりまできたら、文字色を標準に戻す処理
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							// テキスト表示
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						/* 現在の色を指定 */
						SetCurrentColor( hdc, nCOMMENTMODE );
						bSearchStringMode = false;
						goto searchnext;
					}


				}

				if( GetNPos() >= nLineLen - pcLayout2->GetLayoutEol().GetLen() ){
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						/* テキスト表示 */
						GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						SetNBgn(GetNPos() + 1);

						// 行末背景描画
						RECT rcClip;
						if(GetTextArea().GenerateClipRectRight(&rcClip,*pDispPos)){
							cTextType.FillBack(hdc,rcClip);
						}

						// 改行記号の表示
						GetTextDrawer().DispEOL(hdc,pDispPos,pcLayout2->GetLayoutEol(),bSearchStringMode);

						// 2006.04.29 Moca 選択処理のため縦線処理を追加
						GetTextDrawer().DispVerticalLines( hdc, pDispPos->GetDrawPos().y, pDispPos->GetDrawPos().y + nLineHeight, CLayoutInt(0), CLayoutInt(-1) );
						if( bSelected ){
							/* テキスト反転 */
							DispTextSelected(
								hdc,
								pDispPos->GetLayoutLineRef(),
								CMyPoint(sDispPosBegin.GetDrawPos().x, pDispPos->GetDrawPos().y),
								pDispPos->GetDrawCol()
							);
						}
					}

					goto end_of_line;
				}
SEARCH_START:;
				int		nMatchLen;
				int		nMatchColor;
				switch( nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
//@@@ 2001.11.17 add start MIK
					//正規表現キーワード
					if( TypeDataPtr->m_bUseRegexKeyword
					 && m_cRegexKeyword->RegexIsKeyword( pLine, GetNPos(), nLineLen, &nMatchLen, &nMatchColor )
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() )
						{
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						/* 現在の色を指定 */
						SetNBgn(GetNPos());
						nCOMMENTMODE = MakeColorIndexType_RegularExpression(nMatchColor);	/* 色指定 */	//@@@ 2002.01.04 upd
						nCOMMENTEND = GetNPos() + nMatchLen;  /* キーワード文字列の終端をセットする */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
						}
					}
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					else if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( GetNPos(), nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());

						nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice

						/* コメントを表示する */
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
					}
					//	Mar. 15, 2000 genta
					else if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, GetNPos(), nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}

						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, GetNPos() + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI

					}
					else if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, GetNPos(), nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}

						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, GetNPos() + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}
					else if( pLine[GetNPos()] == L'\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* シングルクォーテーション文字列を表示する */
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice

//						if( TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp ){	/* シングルクォーテーション文字列を表示する */
							/* 現在の色を指定 */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}
					else if( pLine[GetNPos()] == L'"' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* ダブルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}
					else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URLを表示する */
					 && IsURL( &pLine[GetNPos()], nLineLen - GetNPos(), &nUrlLen )	/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_URL;	/* URLモード */ // 2002/03/13 novice
						nCOMMENTEND = GetNPos() + nUrlLen;
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//@@@ 2001.02.17 Start by MIK: 半角数値を強調表示
//#ifdef COMPILE_COLOR_DIGIT
					}
					else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (i = IsNumber( pLine, GetNPos(), nLineLen )) > 0 )		/* 半角数字を表示する */
					{
						/* キーワード文字列の終端をセットする */
						i = GetNPos() + i;
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() )
						{
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						/* 現在の色を指定 */
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_DIGIT;	/* 半角数値である */ // 2002/03/13 novice
						nCOMMENTEND = i;
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//#endif
//@@@ 2001.02.17 End by MIK: 半角数値を強調表示
					}
					else if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* キーワードセット */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* 強調キーワードを表示する */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[GetNPos()] )
					){
						//	Mar 4, 2001 genta comment out
						//	bKeyWordTop = false;
						/* キーワード文字列の終端を探す */
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							if( IS_KEYWORD_CHAR( pLine[i] ) ){
							}else{
								break;
							}
						}
						/* キーワードが登録単語ならば、色を変える */
						j = i - GetNPos();
						/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
						nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
							TypeDataPtr->m_nKeyWordSetIdx[0],
							&pLine[GetNPos()],
							j
						);
						if( nIdx != -1 ){
							if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
								/* テキスト表示 */
								GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
							}

							/* 現在の色を指定 */
							SetNBgn(GetNPos());
							nCOMMENTMODE = COLORIDX_KEYWORD1;	/* 強調キーワード1 */ // 2002/03/13 novice
							nCOMMENTEND = i;
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
						}else{		//MIK START ADD 2000.12.01 second keyword & binary search
							// 2005.01.13 MIK 強調キーワード数追加に伴う配列化
							for( int my_i = 1; my_i < 10; my_i++ )
							{
								if(TypeDataPtr->m_nKeyWordSetIdx[ my_i ] != -1 && /* キーワードセット */							//MIK 2000.12.01 second keyword
									TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + my_i].m_bDisp)									//MIK
								{																							//MIK
									/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */						//MIK
									nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
										TypeDataPtr->m_nKeyWordSetIdx[ my_i ] ,													//MIK
										&pLine[GetNPos()],																		//MIK
										j																					//MIK
									);																						//MIK
									if( nIdx != -1 ){																		//MIK
										if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){										//MIK
											/* テキスト表示 */																//MIK
											GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );	//MIK
										}																					//MIK
										/* 現在の色を指定 */																//MIK
										SetNBgn(GetNPos());																		//MIK
										nCOMMENTMODE = (EColorIndexType)(COLORIDX_KEYWORD1 + my_i);	/* 強調キーワード2 */ // 2002/03/13 novice		//MIK
										nCOMMENTEND = i;																	//MIK
										if( !bSearchStringMode ){															//MIK
											SetCurrentColor( hdc, nCOMMENTMODE );											//MIK
										}																					//MIK
										break;
									}																						//MIK
								}																							//MIK
								else
								{
									if(TypeDataPtr->m_nKeyWordSetIdx[my_i] == -1 )
										break;
								}
							}
						}			//MIK END
					}
					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pLine[GetNPos()] ))	bKeyWordTop = false;
					else								bKeyWordTop = true;
					//	To Here
					break;
// 2002/03/13 novice
				case COLORIDX_URL:		/* URLモードである */
				case COLORIDX_KEYWORD1:	/* 強調キーワード1 */
				case COLORIDX_DIGIT:	/* 半角数値である */  //@@@ 2001.02.17 by MIK
				case COLORIDX_KEYWORD2:	/* 強調キーワード2 */	//MIK
				case COLORIDX_KEYWORD3:	// 2005.01.13 MIK 強調キーワード3-10
				case COLORIDX_KEYWORD4:
				case COLORIDX_KEYWORD5:
				case COLORIDX_KEYWORD6:
				case COLORIDX_KEYWORD7:
				case COLORIDX_KEYWORD8:
				case COLORIDX_KEYWORD9:
				case COLORIDX_KEYWORD10:
				//case 1000:	//正規表現キーワード1～10	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04 del
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* コントロールコード */ // 2002/03/13 novice
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = nCOMMENTMODE_OLD;
						nCOMMENTEND = nCOMMENTEND_OLD;
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;

				case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, GetNPos(), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
//#ifdef	COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
				case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, GetNPos(), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
//#endif
				case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = GetNPos()/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ダブルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = GetNPos()/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 ){	//正規表現キーワード1～10
						if( GetNPos() == nCOMMENTEND ){
							if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
								/* テキスト表示 */
								GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
							}
							SetNBgn(GetNPos());
							nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* 現在の色を指定 */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}

				//タブ
				if( pLine[GetNPos()] == WCODE::TAB ){
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						// テキスト表示
						GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );

						// 色決定
						if( bSearchStringMode ){
							nColorIdx = COLORIDX_SEARCH;
						}else{
							nColorIdx = COLORIDX_TAB;
						}

						// タブ表示
						GetTextDrawer().DispTab( hdc, pDispPos, nColorIdx );
					}
					SetNBgn(GetNPos() + 1);
					nCharChars = CLogicInt(1);
				}
				//全角スペース
				else if( WCODE::isZenkakuSpace(pLine[GetNPos()]) && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						// 全角スペース以前のテキストを表示
						GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );

						// 全角空白を表示する
						GetTextDrawer().DispZenkakuSpace(hdc,pDispPos,bSearchStringMode);
					}
					//文字進める
					SetNBgn(GetNPos() + 1);
					nCharChars = CLogicInt(1);
				}
				//半角空白（半角スペース）を表示 2002.04.28 Add by KK 
				else if (pLine[GetNPos()] == ' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp 
					 && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{
					GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						GetTextDrawer().DispHankakuSpace(hdc,pDispPos,bSearchStringMode);
					}
					SetNBgn(GetNPos() + 1);
					nCharChars = CLogicInt(1);
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, GetNPos() );
					if( 0 == nCharChars ){
						nCharChars = CLogicInt(1);
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* コントロールコードを色分け */
					 &&	WCODE::isControlCode(pLine[GetNPos()])
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE_OLD = nCOMMENTMODE;
						nCOMMENTEND_OLD = nCOMMENTEND;
						nCOMMENTMODE = COLORIDX_CTRLCODE;	/* コントロールコード モード */ // 2002/03/13 novice
						/* コントロールコード列の終端を探す */
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if(!WCODE::isControlCode(pLine[i])){
								break;
							}
						}
						nCOMMENTEND = i;
						/* 現在の色を指定 */
						SetCurrentColor( hdc, nCOMMENTMODE );
					}
				}
				SetNPos( GetNPos() + nCharChars );
			}
			//end of while( GetNPos() - nLineBgn < pcLayout2->GetLength() )


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                         本文描画                            //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			if( GetNPos() >= nLineLen ){
				break;
			}
			if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
				/* テキスト表示 */
				GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
				SetNBgn(GetNPos());

				/* 行末背景描画 */
				RECT rcClip;
				if(GetTextArea().GenerateClipRectRight(&rcClip,*pDispPos)){
					cTextType.FillBack(hdc,rcClip);
				}

				/* 折り返し記号を表示する */
				GetTextDrawer().DispWrap(hdc,pDispPos);

				// 2006.04.29 Moca 選択処理のため縦線処理を追加
				GetTextDrawer().DispVerticalLines( hdc, pDispPos->GetDrawPos().y, pDispPos->GetDrawPos().y + nLineHeight,  CLayoutInt(0), CLayoutInt(-1) );
				if( bSelected ){
					/* テキスト反転 */
					DispTextSelected(
						hdc,
						pDispPos->GetLayoutLineRef(),
						CMyPoint(sDispPosBegin.GetDrawPos().x, pDispPos->GetDrawPos().y),
						pDispPos->GetDrawCol()
					);
				}
			}
		}
		// end of while( GetNPos() < nLineLen )

		if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){

			/* テキスト表示 */
			GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );

			/* EOF記号の表示 */
			if( pDispPos->GetLayoutLineRef() + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount()
				&& pDispPos->GetDrawCol() < nWrapKeta )
			{
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					//	May 29, 2004 genta (advised by MIK) 共通関数化
					GetTextDrawer().DispEOF(hdc,pDispPos);
				}
				bEOF = true;
			}

			if( IsBkBitmap() ){
			}else{
				// 行末背景描画
				RECT rcClip;
				if(GetTextArea().GenerateClipRectRight(&rcClip,*pDispPos)){
					cTextType.FillBack(hdc,rcClip);
				}
			}

			// 2006.04.29 Moca 選択処理のため縦線処理を追加
			GetTextDrawer().DispVerticalLines( hdc, pDispPos->GetDrawPos().y, pDispPos->GetDrawPos().y + nLineHeight,  CLayoutInt(0), CLayoutInt(-1) );

			if( bSelected ){
				/* テキスト反転 */
				DispTextSelected(
					hdc,
					pDispPos->GetLayoutLineRef(),
					CMyPoint(sDispPosBegin.GetDrawPos().x, pDispPos->GetDrawPos().y),
					pDispPos->GetDrawCol()
				);
			}

		}
end_of_line:;
		pDispPos->ForwardLayoutLineRef(1);
		pDispPos->ForwardDrawLine(1);
	}
	// NULL == pLineの場合
	else{
		if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
			if(GetTextDrawer().DispEmptyLine(hdc,pDispPos)){
				bEOF=true;
			}
		}
	}

end_of_func:;
//	2002/05/08 YAZAKI アンダーラインの再描画は不要でした
//	MYTRACE_A( "m_nOldUnderLineY=%d\n", m_nOldUnderLineY );
//	if( -1 != m_nOldUnderLineY ){
//		/* カーソル行アンダーラインのON */
//		CaretUnderLineON( TRUE );
//	}


	return bEOF;
}




/* テキスト反転

	@param hdc      
	@param nLineNum 
	@param x        
	@param y        
	@param nX       

	@note
	CCEditView::DispLineNew() での作画(WM_PAINT)時に、1レイアウト行をまとめて反転処理するための関数。
	範囲選択の随時更新は、CEditView::DrawSelectArea() が選択・反転解除を行う。
	
*/
void CEditView::DispTextSelected(
	HDC				hdc,		//!< 作画対象ビットマップを含むデバイス
	CLayoutInt		nLineNum,	//!< 反転処理対象レイアウト行番号(0開始)
	const CMyPoint&	ptXY,		//!< (相対レイアウト0桁目の左端座標, 対象行の上端座標)
	CLayoutInt		nX_Layout	//!< 対象行の終了桁位置。　[ABC\n]なら改行の後ろで4
)
{
	CLayoutInt	nSelectFrom;
	CLayoutInt	nSelectTo;
	RECT		rcClip;
	int			nLineHeight = GetTextMetrics().GetHankakuDy();
	int			nCharWidth = GetTextMetrics().GetHankakuDx();
	HRGN		hrgnDraw;
	const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );

	/* 選択範囲内の行かな */
//	if( IsTextSelected() ){
		if( nLineNum >= GetSelectionInfo().m_sSelect.GetFrom().y && nLineNum <= GetSelectionInfo().m_sSelect.GetTo().y ){
			if( GetSelectionInfo().IsBoxSelecting() ){		/* 矩形範囲選択中 */
				nSelectFrom = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
				nSelectTo   = GetSelectionInfo().m_sSelect.GetTo().GetX2();
				// 2006.09.30 Moca From 矩形選択時[EOF]とその右側は反転しないように修正。処理を追加
				if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 <= nLineNum ){
					CLayoutPoint ptEnd(0, 0);
					m_pcEditDoc->m_cLayoutMgr.GetEndLayoutPos( &ptEnd );
					if( ptEnd.y == nLineNum ){
						if( ptEnd.GetX2() < nSelectFrom ){
							nSelectFrom = ptEnd.GetX2();
						}
						if( ptEnd.GetX2() < nSelectTo ){
							nSelectTo = ptEnd.GetX2();
						}
					}
				}
				// 2006.09.30 Moca To
			}
			else{
				if( GetSelectionInfo().m_sSelect.IsLineOne() ){
					nSelectFrom = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
					nSelectTo   = GetSelectionInfo().m_sSelect.GetTo().GetX2();
				}
				else{
					if( nLineNum == GetSelectionInfo().m_sSelect.GetFrom().y ){
						nSelectFrom = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
						nSelectTo   = nX_Layout;
					}
					else if( nLineNum == GetSelectionInfo().m_sSelect.GetTo().GetY2() ){
						nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
						nSelectTo   = GetSelectionInfo().m_sSelect.GetTo().GetX2();
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
				CLayoutInt t = nSelectFrom;
				nSelectFrom = nSelectTo;
				nSelectTo = t;
			}

			// 2006.03.28 Moca 表示域外なら何もしない
			if( GetTextArea().GetRightCol() < nSelectFrom ){
				return;
			}
			if( nSelectTo <= GetTextArea().GetViewLeftCol() ){
				return;
			}

			if( nSelectFrom < GetTextArea().GetViewLeftCol() ){
				nSelectFrom = GetTextArea().GetViewLeftCol();
			}
			rcClip.left   = ptXY.x + (Int)nSelectFrom * ( nCharWidth );
			rcClip.right  = ptXY.x + (Int)nSelectTo   * ( nCharWidth );
			rcClip.top    = ptXY.y;
			rcClip.bottom = ptXY.y + nLineHeight;

			// 2005/04/02 かろと ０文字マッチだと反転幅が０となり反転されないので、1/3文字幅だけ反転させる
			// 2005/06/26 zenryaku 選択解除でキャレットの残骸が残る問題を修正
			// 2005/09/29 ryoji スクロール時にキャレットのようなゴミが表示される問題を修正
			if (GetSelectionInfo().IsTextSelected() && rcClip.right == rcClip.left &&
				GetSelectionInfo().m_sSelect.IsLineOne() &&
				GetSelectionInfo().m_sSelect.GetFrom().x >= GetTextArea().GetViewLeftCol())
			{
				rcClip.right = rcClip.left + (nCharWidth/3 == 0 ? 1 : nCharWidth/3);
			}

			// 2006.03.28 Moca ウィンドウ幅が大きいと正しく反転しない問題を修正
			if( rcClip.right > GetTextArea().GetAreaRight() ){
				rcClip.right = GetTextArea().GetAreaRight();
			}
			
			HBRUSH hBrush    = ::CreateSolidBrush( SELECTEDAREA_RGB );

			int    nROP_Old  = ::SetROP2( hdc, SELECTEDAREA_ROP2 );
			HBRUSH hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
			hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
			::PaintRgn( hdc, hrgnDraw );
			::DeleteObject( hrgnDraw );

			SetROP2( hdc, nROP_Old );
			SelectObject( hdc, hBrushOld );
			DeleteObject( hBrush );
		}
//	}
	return;
}
