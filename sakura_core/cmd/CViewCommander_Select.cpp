/*!	@file
@brief CViewCommanderクラスのコマンド(選択系/矩形選択系)関数群

	2012/12/20	CViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2005, Moca
	Copyright (C) 2007, kobake, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"


/* 現在位置の単語選択 */
bool CViewCommander::Command_SELECTWORD( CLayoutPoint* pptCaretPos )
{
	CLayoutRange sRange;
	CLogicInt	nIdx;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );
	}
	CLayoutPoint ptCaretPos = (NULL == pptCaretPos ? GetCaret().GetCaretLayoutPos() : *pptCaretPos);
	const CLayout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptCaretPos.GetY2() );
	if( NULL == pcLayout ){
		return false;	//	単語選択に失敗
	}
	/* 指定された桁に対応する行のデータ内の位置を調べる */
	nIdx = m_pCommanderView->LineColumnToIndex( pcLayout, ptCaretPos.GetX2() );

	/* 現在位置の単語の範囲を調べる */
	if( GetDocument()->m_cLayoutMgr.WhereCurrentWord(	ptCaretPos.GetY2(), nIdx, &sRange, NULL, NULL ) ){

		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		/*
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( sRange.GetFrom().GetY2() );
		sRange.SetFromX( m_pCommanderView->LineIndexToColumn( pcLayout, sRange.GetFrom().x ) );
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( sRange.GetTo().GetY2() );
		sRange.SetToX( m_pCommanderView->LineIndexToColumn( pcLayout, sRange.GetTo().x ) );
		*/

		/* 選択範囲の変更 */
		//	2005.06.24 Moca
		m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );
		/* 選択領域描画 */
		m_pCommanderView->GetSelectionInfo().DrawSelectArea();

		/* 単語の先頭にカーソルを移動 */
		GetCaret().MoveCursor( sRange.GetTo(), true );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		return true;	//	単語選択に成功。
	}
	else {
		return false;	//	単語選択に失敗
	}
}



/* すべて選択 */
void CViewCommander::Command_SELECTALL( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );
	}

	/* 先頭へカーソルを移動 */
	//	Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	//	Jul. 29, 2006 genta 選択位置の末尾を正確に取得する
	//	マクロから取得した場合に正しい範囲が取得できないため
	//int nX, nY;
	CLayoutRange sRange;
	sRange.SetFrom(CLayoutPoint(0,0));
	GetDocument()->m_cLayoutMgr.GetEndLayoutPos(sRange.GetToPointer());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* 選択領域描画 */
	m_pCommanderView->GetSelectionInfo().DrawSelectArea(false);
}



/*!	1行選択
	@brief カーソル位置を1行選択する
	@param lparam [in] マクロから使用する拡張フラグ（拡張用に予約）

	note 改行単位で選択を行う。

	@date 2007.11.15 nasukoji	新規作成
*/
void CViewCommander::Command_SELECTLINE( int lparam )
{
	// 改行単位で1行選択する
	Command_GOLINETOP( false, 0x9 );	// 物理行頭に移動

	m_pCommanderView->GetSelectionInfo().m_bBeginLineSelect = true;		// 行単位選択中

	CLayoutPoint ptCaret;

	// 最下行（物理行）でない
	if(GetCaret().GetCaretLogicPos().y < GetDocument()->m_cDocLineMgr.GetLineCount() ){
		// 1行先の物理行からレイアウト行を求める
		GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, GetCaret().GetCaretLogicPos().y + 1), &ptCaret );

		// カーソルを次の物理行頭へ移動する
		m_pCommanderView->MoveCursorSelecting( ptCaret, TRUE );

		// 移動後のカーソル位置を取得する
		ptCaret = GetCaret().GetCaretLayoutPos().Get();
	}else{
		// カーソルを最下行（レイアウト行）へ移動する
		m_pCommanderView->MoveCursorSelecting( CLayoutPoint(CLayoutInt(0), GetDocument()->m_cLayoutMgr.GetLineCount()), TRUE );
		Command_GOLINEEND( true, 0, 0 );	// 行末に移動

		// 選択するものが無い（[EOF]のみの行）時は選択状態としない
		if(( ! m_pCommanderView->GetSelectionInfo().IsTextSelected() )&&
		   ( GetCaret().GetCaretLogicPos().y >= GetDocument()->m_cDocLineMgr.GetLineCount() ))
		{
			// 現在の選択範囲を非選択状態に戻す
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );
		}
	}

	if( m_pCommanderView->GetSelectionInfo().m_bBeginLineSelect ){
		// 範囲選択開始行・カラムを記憶
		m_pCommanderView->GetSelectionInfo().m_sSelect.SetTo( ptCaret );
		m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetTo( ptCaret );
	}

	return;
}



/* 範囲選択開始 */
void CViewCommander::Command_BEGIN_SELECT( void )
{
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在のカーソル位置から選択を開始する */
		m_pCommanderView->GetSelectionInfo().BeginSelectArea();
	}

	//	ロックの解除切り替え
	if ( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ) {
		m_pCommanderView->GetSelectionInfo().m_bSelectingLock = false;	/* 選択状態のロック解除 */
	}
	else {
		m_pCommanderView->GetSelectionInfo().m_bSelectingLock = true;	/* 選択状態のロック */
	}
	if( GetSelect().IsOne() ){
		GetCaret().m_cUnderLine.CaretUnderLineOFF(true);
	}
	m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
	return;
}



/* 矩形範囲選択開始 */
void CViewCommander::Command_BEGIN_BOXSELECT( bool bSelectingLock )
{
	if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
		return;
	}

//@@@ 2002.01.03 YAZAKI 範囲選択中にShift+F6を実行すると選択範囲がクリアされない問題に対処
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );
	}

	/* 現在のカーソル位置から選択を開始する */
	m_pCommanderView->GetSelectionInfo().BeginSelectArea();

	m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock;	/* 選択状態のロック */
	m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);	/* 矩形範囲選択中 */

	m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
	GetCaret().m_cUnderLine.CaretUnderLineOFF(true);
	return;
}
