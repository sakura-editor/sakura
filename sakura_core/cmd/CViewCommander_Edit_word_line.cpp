/*!	@file
@brief CViewCommanderクラスのコマンド(編集系 単語/行単位)関数群

	2012/12/16	CViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, かろと
	Copyright (C) 2005, Moca
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"


//単語の左端まで削除
void CViewCommander::Command_WordDeleteToStart( void )
{
	/* 矩形選択状態では実行不能(★★もろ手抜き★★) */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 矩形範囲選択中か */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			ErrorBeep();
			return;
		}
	}

	// 単語の左端に移動
	CViewCommander::Command_WORDLEFT( true );
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		CMoveCaretOpe*	pcOpe = new CMoveCaretOpe();
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			GetSelect().GetTo(),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		pcOpe->m_ptCaretPos_PHY_After = pcOpe->m_ptCaretPos_PHY_Before;	// 操作後のキャレット位置

		// 操作の追加
		GetOpeBlk()->AppendOpe( pcOpe );
	}

	// 削除
	m_pCommanderView->DeleteData( true );
}



//単語の右端まで削除
void CViewCommander::Command_WordDeleteToEnd( void )
{

	/* 矩形選択状態では実行不能((★★もろ手抜き★★)) */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 矩形範囲選択中か */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			ErrorBeep();
			return;
		}
	}
	/* 単語の右端に移動 */
	CViewCommander::Command_WORDRIGHT( true );
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}
	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		CMoveCaretOpe*	pcOpe = new CMoveCaretOpe();
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			GetSelect().GetFrom(),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		pcOpe->m_ptCaretPos_PHY_After = pcOpe->m_ptCaretPos_PHY_Before;	// 操作後のキャレット位置
		/* 操作の追加 */
		GetOpeBlk()->AppendOpe( pcOpe );
	}
	/* 削除 */
	m_pCommanderView->DeleteData( true );
}



//単語切り取り
void CViewCommander::Command_WordCut( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 切り取り(選択範囲をクリップボードにコピーして削除) */
		Command_CUT();
		return;
	}
	//現在位置の単語選択
	Command_SELECTWORD();
	/* 切り取り(選択範囲をクリップボードにコピーして削除) */
	if ( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		//	単語選択で選択できなかったら、次の文字を選ぶことに挑戦。
		Command_RIGHT( true, false, false );
	}
	Command_CUT();
	return;
}



//単語削除
void CViewCommander::Command_WordDelete( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 削除 */
		m_pCommanderView->DeleteData( true );
		return;
	}
	//現在位置の単語選択
	Command_SELECTWORD();
	/* 削除 */
	m_pCommanderView->DeleteData( true );
	return;
}



//行頭まで切り取り(改行単位)
void CViewCommander::Command_LineCutToStart( void )
{
	const CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 切り取り(選択範囲をクリップボードにコピーして削除) */
		Command_CUT();
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;
	GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, pCLayout->GetLogicLineNo()), &ptPos );
	if( GetCaret().GetCaretLayoutPos() == ptPos ){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	CLayoutRange sRange(ptPos,GetCaret().GetCaretLayoutPos());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/*切り取り(選択範囲をクリップボードにコピーして削除) */
	Command_CUT();
}



//行末まで切り取り(改行単位)
void CViewCommander::Command_LineCutToEnd( void )
{
	const CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 切り取り(選択範囲をクリップボードにコピーして削除) */
		Command_CUT();
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	if( EOL_NONE == pCLayout->GetDocLineRef()->GetEol() ){	/* 改行コードの種類 */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}
	else{
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL() - pCLayout->GetDocLineRef()->GetEol().GetLen(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if( GetCaret().GetCaretLayoutPos().GetY2() == ptPos.y && GetCaret().GetCaretLayoutPos().GetX2() >= ptPos.x ){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	CLayoutRange sRange(GetCaret().GetCaretLayoutPos(),ptPos);
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/*切り取り(選択範囲をクリップボードにコピーして削除) */
	Command_CUT();
}



//行頭まで削除(改行単位)
void CViewCommander::Command_LineDeleteToStart( void )
{
	const CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		m_pCommanderView->DeleteData( true );
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, pCLayout->GetLogicLineNo()), &ptPos );
	if( GetCaret().GetCaretLayoutPos() == ptPos ){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	CLayoutRange sRange(ptPos,GetCaret().GetCaretLayoutPos());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* 選択領域削除 */
	m_pCommanderView->DeleteData( true );
}



//行末まで削除(改行単位)
void CViewCommander::Command_LineDeleteToEnd( void )
{
	const CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		m_pCommanderView->DeleteData( true );
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	if( EOL_NONE == pCLayout->GetDocLineRef()->GetEol() ){	/* 改行コードの種類 */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else{
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL() - pCLayout->GetDocLineRef()->GetEol().GetLen(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if( GetCaret().GetCaretLayoutPos().GetY2() == ptPos.y && GetCaret().GetCaretLayoutPos().GetX2() >= ptPos.x ){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	CLayoutRange sRange( GetCaret().GetCaretLayoutPos(), ptPos );
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* 選択領域削除 */
	m_pCommanderView->DeleteData( true );
}



//行切り取り(折り返し単位)
void CViewCommander::Command_CUT_LINE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		ErrorBeep();
		return;
	}

	const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().y );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}

	// 2007.10.04 ryoji 処理簡素化
	m_pCommanderView->CopyCurLine(
		GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy,
		EOL_UNKNOWN,
		GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste
	);
	Command_DELETE_LINE();
	return;
}



/* 行削除(折り返し単位) */
void CViewCommander::Command_DELETE_LINE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	const CLayout*	pcLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		ErrorBeep();
		return;
	}
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}
	GetSelect().SetFrom(CLayoutPoint(CLayoutInt(0),GetCaret().GetCaretLayoutPos().GetY2()    ));	//範囲選択開始位置
	GetSelect().SetTo  (CLayoutPoint(CLayoutInt(0),GetCaret().GetCaretLayoutPos().GetY2() + 1));	//範囲選択終了位置

	CLayoutPoint ptCaretPos_OLD = GetCaret().GetCaretLayoutPos();

	Command_DELETE();
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL != pcLayout ){
		// 2003-04-30 かろと
		// 行削除した後、フリーカーソルでないのにカーソル位置が行端より右になる不具合対応
		// フリーカーソルモードでない場合は、カーソル位置を調整する
		if( !GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode ) {
			CLogicInt nIndex;

			CLayoutInt tmp;
			nIndex = m_pCommanderView->LineColumnToIndex2( pcLayout, ptCaretPos_OLD.GetX2(), &tmp );
			ptCaretPos_OLD.x=tmp;

			if (ptCaretPos_OLD.x > 0) {
				ptCaretPos_OLD.x--;
			} else {
				ptCaretPos_OLD.x = m_pCommanderView->LineIndexToColumn( pcLayout, nIndex );
			}
		}
		/* 操作前の位置へカーソルを移動 */
		GetCaret().MoveCursor( ptCaretPos_OLD, true );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			/* 操作の追加 */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
	}
	return;
}



/* 行の二重化(折り返し単位) */
void CViewCommander::Command_DUPLICATELINE( void )
{
	int				bCRLF;
	int				bAddCRLF;
	CNativeW		cmemBuf;
	const CLayout*	pcLayout;

	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );
	}

	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		/* 操作の追加 */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}

	CLayoutPoint ptCaretPosOld = GetCaret().GetCaretLayoutPos() + CLayoutPoint(0,1);

	//行頭に移動(折り返し単位)
	Command_GOLINETOP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, 0x1 /* カーソル位置に関係なく行頭に移動 */ );

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		/* 操作の追加 */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}



	/* 二重化したい行を調べる
	||	・改行で終わっている
	||	・改行で終わっていない
	||	・最終行である
	||	→折り返しでない
	||	・最終行でない
	||	→折り返しである
	*/
	bCRLF = ( EOL_NONE == pcLayout->GetLayoutEol() ) ? FALSE : TRUE;

	bAddCRLF = FALSE;
	if( !bCRLF ){
		if( GetCaret().GetCaretLayoutPos().GetY2() == GetDocument()->m_cLayoutMgr.GetLineCount() - 1 ){
			bAddCRLF = TRUE;
		}
	}

	cmemBuf.SetString( pcLayout->GetPtr(), pcLayout->GetLengthWithoutEOL() + pcLayout->GetLayoutEol().GetLen() );	//	※pcLayout->GetLengthWithEOL()は、EOLの長さを必ず1にするので使えない。
	if( bAddCRLF ){
		/* 現在、Enterなどで挿入する改行コードの種類を取得 */
		CEol cWork = GetDocument()->m_cDocEditor.GetNewLineCode();
		cmemBuf.AppendString( cWork.GetValue2(), cWork.GetLen() );
	}

	/* 現在位置にデータを挿入 */
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		&ptLayoutNew,
		true
	);

	/* カーソルを移動 */
	GetCaret().MoveCursor( ptCaretPosOld, true );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();


	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		/* 操作の追加 */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}
	return;
}
