/*!	@file
@brief CViewCommanderクラスのコマンド(モード切り替え系)関数群

	2012/12/15	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2007, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"


/*! 挿入／上書きモード切り替え

	@date 2005.10.02 genta InsMode関数化
*/
void CViewCommander::Command_CHGMOD_INS( void )
{
	/* 挿入モードか？ */
	if( m_pCommanderView->IsInsMode() ){
		m_pCommanderView->SetInsMode( false );
	}else{
		m_pCommanderView->SetInsMode( true );
	}
	/* キャレットの表示・更新 */
	GetCaret().ShowEditCaret();
	/* キャレットの行桁位置を表示する */
	GetCaret().ShowCaretPosInfo();
}



//	from CViewCommander_New.cpp
/*! 入力する改行コードを設定

	@author moca
	@date 2003.06.23 新規作成
*/
void CViewCommander::Command_CHGMOD_EOL( EEolType e ){
	if( EOL_NONE < e && e < EOL_CODEMAX  ){
		GetDocument()->m_cDocEditor.SetNewLineCode( e );
		// ステータスバーを更新するため
		// キャレットの行桁位置を表示する関数を呼び出す
		GetCaret().ShowCaretPosInfo();
	}
}



//! 文字コードセット指定
void CViewCommander::Command_CHG_CHARSET(
	ECodeType	eCharSet,	// [in] 設定する文字コードセット
	bool		bBom		// [in] 設定するBOM(Unicode系以外は無視)
)
{
	if (eCharSet == CODE_NONE || eCharSet ==  CODE_AUTODETECT) {
		// 文字コードが指定されていないならば
		// 文字コードの確認
		eCharSet = GetDocument()->GetDocumentEncoding();	// 設定する文字コードセット
		bBom     = GetDocument()->m_cDocFile.IsBomExist();	// 設定するBOM
		int nRet = GetEditWindow()->m_cDlgSetCharSet.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 
						&eCharSet, &bBom );
		if (!nRet) {
			return;
		}
	}

	// 文字コードの設定
	GetDocument()->SetDocumentEncoding( eCharSet );
	GetDocument()->m_cDocFile.SetBomMode( CCodeTypeName( eCharSet ).UseBom() ? bBom : false );

	// ステータス表示
	GetCaret().ShowCaretPosInfo();
}



/** 各種モードの取り消し
	@param whereCursorIs 選択をキャンセルした後、キャレットをどこに置くか。0=動かさない。1=左上。2=右下。
*/
void CViewCommander::Command_CANCEL_MODE( int whereCursorIs )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ) {
		// 選択解除後のカーソル位置を決める。
		CLayoutPoint ptTo ;
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ) { // 矩形選択ではキャレットが改行の後ろに取り残されないように、左上。
			/* 2点を対角とする矩形を求める */
			CLayoutRange rcSel;
			TwoPointToRange(
				&rcSel,
				GetSelect().GetFrom(),	// 範囲選択開始
				GetSelect().GetTo()		// 範囲選択終了
			);
			ptTo = rcSel.GetFrom();
		} else if( 1 == whereCursorIs ) { // 左上
			ptTo = GetSelect().GetFrom();
		} else if( 2 == whereCursorIs ) { // 右下
			ptTo = GetSelect().GetTo();
		} else {
			ptTo = GetCaret().GetCaretLayoutPos();
		}

		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );

		/* カーソルを移動 */
		if( ptTo.y >= GetDocument()->m_cLayoutMgr.GetLineCount() ){
			/* ファイルの最後に移動 */
			Command_GOFILEEND(false);
		} else {
			GetCaret().MoveCursor( ptTo, true );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
	}else{
		// 2011.12.05 Moca 選択中の未選択状態でもLockの解除と描画が必要
		if( m_pCommanderView->GetSelectionInfo().IsTextSelecting()
				|| m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			m_pCommanderView->GetSelectionInfo().DisableSelectArea(true);
			GetCaret().m_cUnderLine.CaretUnderLineON(true,false);
			m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
		}
	}
}
