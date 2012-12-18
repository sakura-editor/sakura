#include "StdAfx.h"
#include <algorithm>		// 2001.12.11 hor    for VC++
#include <string>///	2002/2/3 aroka 
#include <vector> ///	2002/2/3 aroka
#include "CViewCommander.h"
#include "view/CEditView.h"
#include "CWaitCursor.h"
#include "charset/charcode.h"
#include "debug/CRunningTimer.h"
#include "COpe.h" ///	2002/2/3 aroka from here
#include "COpeBlk.h" ///	2002/2/3 aroka 
#include "doc/CLayout.h"///	2002/2/3 aroka 
#include "doc/CDocLine.h"///	2002/2/3 aroka 
#include "doc/CEditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理
#include "debug/Debug.h"///	2002/2/3 aroka 
#include "_os/COsVersionInfo.h"   // 2002.04.09 minfu 
#include "dlg/CDlgCtrlCode.h"	//コントロールコードの入力(ダイアログ)
#include "dlg/CDlgFavorite.h"	//履歴の管理	//@@@ 2003.04.08 MIK
#include "dlg/CDlgCancel.h"	// 2006.12.09 maru
#include "window/CEditWnd.h"
#include "io/CFileLoad.h"	// 2006.12.09 maru
#include "env/CSakuraEnvironment.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

using namespace std; // 2002/2/3 aroka to here



#ifndef FID_RECONVERT_VERSION  // 2002.04.10 minfu 
#define FID_RECONVERT_VERSION 0x10000000
#endif


/* Undo 元に戻す */
void CViewCommander::Command_UNDO( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	if( !GetDocument()->m_cDocEditor.IsEnableUndo() ){	/* Undo(元に戻す)可能な状態か？ */
		return;
	}

	MY_RUNNINGTIMER( cRunningTimer, "CViewCommander::Command_UNDO()" );

	COpe*		pcOpe = NULL;

	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
	bool		bIsModified;
//	int			nNewLine;	/* 挿入された部分の次の位置の行 */
//	int			nNewPos;	/* 挿入された部分の次の位置のデータ位置 */
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	CLayoutPoint ptCaretPos_Before;

	CLayoutPoint ptCaretPos_After;

	/* 各種モードの取り消し */
	Command_CANCEL_MODE();

	m_pCommanderView->m_bDoing_UndoRedo = TRUE;	/* アンドゥ・リドゥの実行中か */

	/* 現在のUndo対象の操作ブロックを返す */
	if( NULL != ( pcOpeBlk = GetDocument()->m_cDocEditor.m_cOpeBuf.DoUndo( &bIsModified ) ) ){
		m_pCommanderView->SetDrawSwitch(false);	//	hor
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = nOpeBlkNum - 1; i >= 0; i-- ){
			pcOpe = pcOpeBlk->GetOpe( i );
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_After,
				&ptCaretPos_After
			);
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before,
				&ptCaretPos_Before
			);


			/* カーソルを移動 */
			GetCaret().MoveCursor( ptCaretPos_After, false );

			switch( pcOpe->GetCode() ){
			case OPE_INSERT:
				{
					CInsertOpe* pcInsertOpe = static_cast<CInsertOpe*>(pcOpe);

					/* 選択範囲の変更 */
					m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetFrom(ptCaretPos_Before);
					m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetTo(m_pCommanderView->GetSelectionInfo().m_sSelectBgn.GetFrom());
					m_pCommanderView->GetSelectionInfo().m_sSelect.SetFrom(ptCaretPos_Before);
					m_pCommanderView->GetSelectionInfo().m_sSelect.SetTo(ptCaretPos_After);

					/* データ置換 削除&挿入にも使える */
					m_pCommanderView->ReplaceData_CEditView(
						m_pCommanderView->GetSelectionInfo().m_sSelect,				// 削除範囲
						&pcInsertOpe->m_pcmemData,	// 削除されたデータのコピー(NULL可能)
						L"",						// 挿入するデータ
						CLogicInt(0),				// 挿入するデータの長さ
						false,						// 再描画するか否か
						m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
					);

					/* 選択範囲の変更 */
					m_pCommanderView->GetSelectionInfo().m_sSelectBgn.Clear(-1); //範囲選択(原点)
					m_pCommanderView->GetSelectionInfo().m_sSelect.Clear(-1);
				}
				break;
			case OPE_DELETE:
				{
					CDeleteOpe* pcDeleteOpe = static_cast<CDeleteOpe*>(pcOpe);

					//2007.10.17 kobake メモリリークしてました。修正。
					if( 0 < pcDeleteOpe->m_pcmemData.GetStringLength() ){
						/* データ置換 削除&挿入にも使える */
						CLayoutRange sRange;
						sRange.Set(ptCaretPos_Before);
						m_pCommanderView->ReplaceData_CEditView(
							sRange,
							NULL,										/* 削除されたデータのコピー(NULL可能) */
							pcDeleteOpe->m_pcmemData.GetStringPtr(),	/* 挿入するデータ */
							pcDeleteOpe->m_nDataLen,					/* 挿入するデータの長さ */
							false,										/*再描画するか否か*/
							m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
						);
					}
					pcDeleteOpe->m_pcmemData.Clear();
				}
				break;
			case OPE_MOVECARET:
				/* カーソルを移動 */
				GetCaret().MoveCursor( ptCaretPos_After, false );
				break;
			}

			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before,
				&ptCaretPos_Before
			);
			if( i == 0 ){
				/* カーソルを移動 */
				GetCaret().MoveCursor( ptCaretPos_Before, true );
			}else{
				/* カーソルを移動 */
				GetCaret().MoveCursor( ptCaretPos_Before, false );
			}
		}
		m_pCommanderView->SetDrawSwitch(true);	//	hor
		m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji

		/* Undo後の変更フラグ */
		GetDocument()->m_cDocEditor.SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */

		m_pCommanderView->SetBracketPairPos( true );	// 03/03/07 ai

		/* 再描画 */
		// ルーラー再描画の必要があるときは DispRuler() ではなく他の部分と同時に Call_OnPaint() で描画する	// 2010.08.20 ryoji
		// ・DispRuler() はルーラーとテキストの隙間（左側は行番号の幅に合わせた帯）を描画してくれない
		// ・行番号表示に必要な幅は OPE_INSERT/OPE_DELETE 処理内で更新されており変更があればルーラー再描画フラグに反映されている
		// ・水平スクロールもルーラー再描画フラグに反映されている
		const bool bRedrawRuler = m_pCommanderView->GetRuler().GetRedrawFlag();
		m_pCommanderView->Call_OnPaint( PAINT_LINENUMBER | PAINT_BODY | (bRedrawRuler? PAINT_RULER: 0), false );
		if( !bRedrawRuler ){
			// ルーラーのキャレットのみを再描画
			HDC hdc = m_pCommanderView->GetDC();
			m_pCommanderView->GetRuler().DispRuler( hdc );
			m_pCommanderView->ReleaseDC( hdc );
		}

		GetCaret().ShowCaretPosInfo();	// キャレットの行桁位置を表示する	// 2007.10.19 ryoji

		if( !GetEditWindow()->UpdateTextWrap() )	// 折り返し方法関連の更新	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews( m_pCommanderView );	//	他のペインの表示を更新

	}

	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji 追加
	m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */

	return;
}





/* Redo やり直し */
void CViewCommander::Command_REDO( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}


	if( !GetDocument()->m_cDocEditor.IsEnableRedo() ){	/* Redo(やり直し)可能な状態か？ */
		return;
	}
	MY_RUNNINGTIMER( cRunningTimer, "CViewCommander::Command_REDO()" );

	COpe*		pcOpe = NULL;
	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
//	int			nNewLine;	/* 挿入された部分の次の位置の行 */
//	int			nNewPos;	/* 挿入された部分の次の位置のデータ位置 */
	bool		bIsModified;
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	CLayoutPoint ptCaretPos_Before;
	CLayoutPoint ptCaretPos_To;
	CLayoutPoint ptCaretPos_After;


	/* 各種モードの取り消し */
	Command_CANCEL_MODE();

	m_pCommanderView->m_bDoing_UndoRedo = TRUE;	/* アンドゥ・リドゥの実行中か */

	/* 現在のRedo対象の操作ブロックを返す */
	if( NULL != ( pcOpeBlk = GetDocument()->m_cDocEditor.m_cOpeBuf.DoRedo( &bIsModified ) ) ){
		m_pCommanderView->SetDrawSwitch(false);	// 2007.07.22 ryoji
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = 0; i < nOpeBlkNum; ++i ){
			pcOpe = pcOpeBlk->GetOpe( i );
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before,
				&ptCaretPos_Before
			);

			if( i == 0 ){
				/* カーソルを移動 */
				GetCaret().MoveCursor( ptCaretPos_Before, true );
			}else{
				/* カーソルを移動 */
				GetCaret().MoveCursor( ptCaretPos_Before, false );
			}
			switch( pcOpe->GetCode() ){
			case OPE_INSERT:
				{
					CInsertOpe* pcInsertOpe = static_cast<CInsertOpe*>(pcOpe);

					//2007.10.17 kobake メモリリークしてました。修正。
					if( 0 < pcInsertOpe->m_pcmemData.GetStringLength() ){
						/* データ置換 削除&挿入にも使える */
						CLayoutRange sRange;
						sRange.Set(ptCaretPos_Before);
						m_pCommanderView->ReplaceData_CEditView(
							sRange,
							NULL,										/* 削除されたデータのコピー(NULL可能) */
							pcInsertOpe->m_pcmemData.GetStringPtr(),	/* 挿入するデータ */
							pcInsertOpe->m_pcmemData.GetStringLength(),	/* 挿入するデータの長さ */
							false,										/*再描画するか否か*/
							m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
						);

					}
					pcInsertOpe->m_pcmemData.Clear();
				}
				break;
			case OPE_DELETE:
				{
					CDeleteOpe* pcDeleteOpe = static_cast<CDeleteOpe*>(pcOpe);

					GetDocument()->m_cLayoutMgr.LogicToLayout(
						pcDeleteOpe->m_ptCaretPos_PHY_To,
						&ptCaretPos_To
					);

					/* データ置換 削除&挿入にも使える */
					m_pCommanderView->ReplaceData_CEditView(
						CLayoutRange(ptCaretPos_Before,ptCaretPos_To),
						&pcDeleteOpe->m_pcmemData,	/* 削除されたデータのコピー(NULL可能) */
						L"",						/* 挿入するデータ */
						CLogicInt(0),				/* 挿入するデータの長さ */
						false,
						m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
					);
				}
				break;
			case OPE_MOVECARET:
				break;
			}
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_After,
				&ptCaretPos_After
			);

			if( i == nOpeBlkNum - 1	){
				/* カーソルを移動 */
				GetCaret().MoveCursor( ptCaretPos_After, true );
			}else{
				/* カーソルを移動 */
				GetCaret().MoveCursor( ptCaretPos_After, false );
			}
		}
		m_pCommanderView->SetDrawSwitch(true); // 2007.07.22 ryoji
		m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji

		/* Redo後の変更フラグ */
		GetDocument()->m_cDocEditor.SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */

		m_pCommanderView->SetBracketPairPos( true );	// 03/03/07 ai

		/* 再描画 */
		// ルーラー再描画の必要があるときは DispRuler() ではなく他の部分と同時に Call_OnPaint() で描画する	// 2010.08.20 ryoji
		// ・DispRuler() はルーラーとテキストの隙間（左側は行番号の幅に合わせた帯）を描画してくれない
		// ・行番号表示に必要な幅は OPE_INSERT/OPE_DELETE 処理内で更新されており変更があればルーラー再描画フラグに反映されている
		// ・水平スクロールもルーラー再描画フラグに反映されている
		const bool bRedrawRuler = m_pCommanderView->GetRuler().GetRedrawFlag();
		m_pCommanderView->Call_OnPaint( PAINT_LINENUMBER | PAINT_BODY | (bRedrawRuler? PAINT_RULER: 0), false );
		if( !bRedrawRuler ){
			// ルーラーのキャレットのみを再描画
			HDC hdc = m_pCommanderView->GetDC();
			m_pCommanderView->GetRuler().DispRuler( hdc );
			m_pCommanderView->ReleaseDC( hdc );
		}

		GetCaret().ShowCaretPosInfo();	// キャレットの行桁位置を表示する	// 2007.10.19 ryoji

		if( !GetEditWindow()->UpdateTextWrap() )	// 折り返し方法関連の更新	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews( m_pCommanderView );	//	他のペインの表示を更新
	}

	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji 追加
	m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */

	return;
}







// 2001/06/20 Start by asa-o

// テキストを１行下へスクロール
void CViewCommander::Command_WndScrollDown( void )
{
	CLayoutInt	nCaretMarginY;

	nCaretMarginY = m_pCommanderView->GetTextArea().m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = CLayoutInt(1);

	nCaretMarginY += 2;

	if( GetCaret().GetCaretLayoutPos().GetY() > m_pCommanderView->GetTextArea().m_nViewRowNum + m_pCommanderView->GetTextArea().GetViewTopLine() - (nCaretMarginY + 1) ){
		GetCaret().m_cUnderLine.CaretUnderLineOFF( TRUE );
	}

	//	Sep. 11, 2004 genta 同期用に行数を記憶
	//	Sep. 11, 2004 genta 同期スクロールの関数化
	m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV(m_pCommanderView->GetTextArea().GetViewTopLine() - CLayoutInt(1)));

	// テキストが選択されていない
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() )
	{
		// カーソルが画面外に出た
		if( GetCaret().GetCaretLayoutPos().GetY() > m_pCommanderView->GetTextArea().m_nViewRowNum + m_pCommanderView->GetTextArea().GetViewTopLine() - nCaretMarginY )
		{
			if( GetCaret().GetCaretLayoutPos().GetY() > GetDocument()->m_cLayoutMgr.GetLineCount() - nCaretMarginY )
				GetCaret().Cursor_UPDOWN( (GetDocument()->m_cLayoutMgr.GetLineCount() - nCaretMarginY) - GetCaret().GetCaretLayoutPos().GetY2(), FALSE );
			else
				GetCaret().Cursor_UPDOWN( CLayoutInt(-1), FALSE);
			GetCaret().ShowCaretPosInfo();
		}
	}

	GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
}

// テキストを１行上へスクロール
void CViewCommander::Command_WndScrollUp(void)
{
	CLayoutInt	nCaretMarginY;

	nCaretMarginY = m_pCommanderView->GetTextArea().m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	if( GetCaret().GetCaretLayoutPos().GetY2() < m_pCommanderView->GetTextArea().GetViewTopLine() + (nCaretMarginY + 1) ){
		GetCaret().m_cUnderLine.CaretUnderLineOFF( TRUE );
	}

	//	Sep. 11, 2004 genta 同期用に行数を記憶
	//	Sep. 11, 2004 genta 同期スクロールの関数化
	m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV( m_pCommanderView->GetTextArea().GetViewTopLine() + CLayoutInt(1) ));

	// テキストが選択されていない
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() )
	{
		// カーソルが画面外に出た
		if( GetCaret().GetCaretLayoutPos().GetY() < m_pCommanderView->GetTextArea().GetViewTopLine() + nCaretMarginY )
		{
			if( m_pCommanderView->GetTextArea().GetViewTopLine() == 1 )
				GetCaret().Cursor_UPDOWN( nCaretMarginY + 1, FALSE );
			else
				GetCaret().Cursor_UPDOWN( CLayoutInt(1), FALSE );
			GetCaret().ShowCaretPosInfo();
		}
	}

	GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
}

// 2001/06/20 End



/* 次の段落へ進む
	2002/04/26 段落の両端で止まるオプションを追加
	2002/04/19 新規
*/
void CViewCommander::Command_GONEXTPARAGRAPH( bool bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = 0;
	
	bool nFirstLineIsEmptyLine = false;
	/* まずは、現在位置が空行（スペース、タブ、改行記号のみの行）かどうか判別 */
	if ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer++;
	}
	else {
		// EOF行でした。
		return;
	}

	/* 次に、nFirstLineIsEmptyLineと異なるところまで読み飛ばす */
	while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer++;
		}
		else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLineが空行だったら、今見ているところは非空行。すなわちおしまい。
		nFirstLineIsEmptyLineが非空行だったら、今見ているところは空行。
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	おしまい。
	}
	else {
		//	いま見ているところは空行の1行目
		if ( GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	段落の両端で止まる
		}
		else {
			/* 仕上げに、空行じゃないところまで進む */
			while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					nCaretPointer++;
				}
				else {
					break;
				}
			}
		}
	}

	//	EOFまで来たり、目的の場所まできたので移動終了。

	/* 移動距離を計算 */
	CLayoutPoint ptCaretPos_Layo;

	/* 移動前の物理位置 */
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos(),
		&ptCaretPos_Layo
	);

	/* 移動後の物理位置 */
	CLayoutPoint ptCaretPos_Layo_CaretPointer;
	//int nCaretPosY_Layo_CaretPointer;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos() + CLogicPoint(0,nCaretPointer),
		&ptCaretPos_Layo_CaretPointer
	);

	GetCaret().Cursor_UPDOWN( ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect );
}

/* 前の段落へ進む
	2002/04/26 段落の両端で止まるオプションを追加
	2002/04/19 新規
*/
void CViewCommander::Command_GOPREVPARAGRAPH( bool bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = -1;

	bool nFirstLineIsEmptyLine = false;
	/* まずは、現在位置が空行（スペース、タブ、改行記号のみの行）かどうか判別 */
	if ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer--;
	}
	else {
		nFirstLineIsEmptyLine = true;
		nCaretPointer--;
	}

	/* 次に、nFirstLineIsEmptyLineと異なるところまで読み飛ばす */
	while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer--;
		}
		else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLineが空行だったら、今見ているところは非空行。すなわちおしまい。
		nFirstLineIsEmptyLineが非空行だったら、今見ているところは空行。
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	おしまい。
		if ( GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	段落の両端で止まる
			nCaretPointer++;	//	空行の最上行（段落の末端の次の行）で止まる。
		}
		else {
			/* 仕上げに、空行じゃないところまで進む */
			while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					break;
				}
				else {
					nCaretPointer--;
				}
			}
			nCaretPointer++;	//	空行の最上行（段落の末端の次の行）で止まる。
		}
	}
	else {
		//	いま見ているところは空行の1行目
		if ( GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	段落の両端で止まる
			nCaretPointer++;
		}
		else {
			nCaretPointer++;
		}
	}

	//	EOFまで来たり、目的の場所まできたので移動終了。

	/* 移動距離を計算 */
	CLayoutPoint ptCaretPos_Layo;

	/* 移動前の物理位置 */
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos(),
		&ptCaretPos_Layo
	);

	/* 移動後の物理位置 */
	CLayoutPoint ptCaretPos_Layo_CaretPointer;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos() + CLogicPoint(0, nCaretPointer),
		&ptCaretPos_Layo_CaretPointer
	);

	GetCaret().Cursor_UPDOWN( ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect );
}

// From Here 2001.12.03 hor

//! ブックマークの設定・解除を行う(トグル動作)
void CViewCommander::Command_BOOKMARK_SET(void)
{
	CDocLine*	pCDocLine;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() && m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y<m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().y ){
		CLogicPoint ptFrom;
		CLogicPoint ptTo;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint(CLayoutInt(0), m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y),
			&ptFrom
		);
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint(CLayoutInt(0), m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().y  ),
			&ptTo
		);
		for(CLogicInt nY=ptFrom.GetY2();nY<=ptTo.y;nY++){
			pCDocLine=GetDocument()->m_cDocLineMgr.GetLine( nY );
			CBookmarkSetter cBookmark(pCDocLine);
			if(pCDocLine)cBookmark.SetBookmark(!cBookmark.IsBookmarked());
		}
	}
	else{
		pCDocLine=GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() );
		CBookmarkSetter cBookmark(pCDocLine);
		if(pCDocLine)cBookmark.SetBookmark(!cBookmark.IsBookmarked());
	}

	// 2002.01.16 hor 分割したビューも更新
	GetEditWindow()->Views_Redraw();
}



//! 次のブックマークを探し，見つかったら移動する
void CViewCommander::Command_BOOKMARK_NEXT(void)
{
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor

	CLogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	CLogicInt tmp_y;

	nYOld=ptXY.y;					// hor

re_do:;								// hor
	if(CBookmarkManager(&GetDocument()->m_cDocLineMgr).SearchBookMark(ptXY.GetY2(), SEARCH_FORWARD, &tmp_y)){
		ptXY.y = tmp_y;
		bFound = TRUE;
		CLayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
		//	2006.07.09 genta 新規関数にまとめた
		m_pCommanderView->MoveCursorSelecting( ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&		// 見つからなかった
			bRedo			// 最初の検索
		){
			ptXY.y=-1;	//	2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;		// 先頭から再検索
		}
	}
	if(bFound){
		if(nYOld >= ptXY.y)m_pCommanderView->SendStatusMessage(_T("▼先頭から再検索しました"));
	}else{
		m_pCommanderView->SendStatusMessage(_T("▽見つかりませんでした"));
		AlertNotFound( m_pCommanderView->GetHwnd(), _T("前方(↓) にブックマークが見つかりません。"));
	}
	return;
}



//! 前のブックマークを探し，見つかったら移動する．
void CViewCommander::Command_BOOKMARK_PREV(void)
{
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor

	CLogicPoint	ptXY(0,GetCaret().GetCaretLogicPos().y);
	CLogicInt tmp_y;

	nYOld=ptXY.y;						// hor

re_do:;								// hor
	if(CBookmarkManager(&GetDocument()->m_cDocLineMgr).SearchBookMark(ptXY.GetY2(), SEARCH_BACKWARD, &tmp_y)){
		ptXY.y = tmp_y;
		bFound = TRUE;				// hor
		CLayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
		//	2006.07.09 genta 新規関数にまとめた
		m_pCommanderView->MoveCursorSelecting( ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&	// 見つからなかった
			bRedo		// 最初の検索
		){
			// 2011.02.02 m_cLayoutMgr→m_cDocLineMgr
			ptXY.y= GetDocument()->m_cDocLineMgr.GetLineCount();	// 2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;	// 末尾から再検索
		}
	}
	if(bFound){
		if(nYOld <= ptXY.y)m_pCommanderView->SendStatusMessage(_T("▲末尾から再検索しました"));
	}else{
		m_pCommanderView->SendStatusMessage(_T("△見つかりませんでした"));
		AlertNotFound( m_pCommanderView->GetHwnd(), _T("後方(↑) にブックマークが見つかりません。") );
	}
	return;
}



//! ブックマークをクリアする
void CViewCommander::Command_BOOKMARK_RESET(void)
{
	CBookmarkManager(&GetDocument()->m_cDocLineMgr).ResetAllBookMark();
	// 2002.01.16 hor 分割したビューも更新
	GetEditWindow()->Views_Redraw();
}


//指定パターンに一致する行をマーク 2002.01.16 hor
//キーマクロで記録できるように	2002.02.08 hor
void CViewCommander::Command_BOOKMARK_PATTERN( void )
{
	//検索or置換ダイアログから呼び出された
	if( !m_pCommanderView->ChangeCurRegexp(false) ) return;
	
	CBookmarkManager(&GetDocument()->m_cDocLineMgr).MarkSearchWord(
		m_pCommanderView->m_strCurSearchKey.c_str(),		// 検索条件
		m_pCommanderView->m_sCurSearchOption,	// 検索条件
		&m_pCommanderView->m_CurRegexp							// 正規表現コンパイルデータ
	);
	// 2002.01.16 hor 分割したビューも更新
	GetEditWindow()->Views_Redraw();
}



/*! TRIM Step1
	非選択時はカレント行を選択して m_pCommanderView->ConvSelectedArea → ConvMemory へ
	@author hor
	@date 2001.12.03 hor 新規作成
*/
void CViewCommander::Command_TRIM(
	BOOL bLeft	//!<  [in] FALSE: 右TRIM / それ以外: 左TRIM
)
{
	bool bBeDisableSelectArea = false;
	CViewSelect& cViewSelect = m_pCommanderView->GetSelectionInfo();

	if(!cViewSelect.IsTextSelected()){	//	非選択時は行選択に変更
		cViewSelect.m_sSelect.SetFrom(
			CLayoutPoint(
				CLayoutInt(0),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		cViewSelect.m_sSelect.SetTo  (
			CLayoutPoint(
				GetDocument()->m_cLayoutMgr.GetMaxLineKetas(),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		bBeDisableSelectArea = true;
	}

	if(bLeft){
		m_pCommanderView->ConvSelectedArea( F_LTRIM );
	}
	else{
		m_pCommanderView->ConvSelectedArea( F_RTRIM );
	}

	if(bBeDisableSelectArea)
		cViewSelect.DisableSelectArea( TRUE );
}

/*!	物理行のソートに使う構造体*/
typedef struct _SORTTABLE {
	wstring sKey1;
	wstring sKey2;
} SORTDATA, *SORTTABLE;

/*!	物理行のソートに使う関数(昇順) */
bool SortByKeyAsc (SORTTABLE pst1, SORTTABLE pst2) {return (pst1->sKey1<pst2->sKey1);}

/*!	物理行のソートに使う関数(降順) */
bool SortByKeyDesc(SORTTABLE pst1, SORTTABLE pst2) {return (pst1->sKey1>pst2->sKey1);}

/*!	@brief 物理行のソート

	非選択時は何も実行しない．矩形選択時は、その範囲をキーにして物理行をソート．
	
	@note とりあえず改行コードを含むデータをソートしているので、
	ファイルの最終行はソート対象外にしています
	@author hor
	@date 2001.12.03 hor 新規作成
	@date 2001.12.21 hor 選択範囲の調整ロジックを訂正
*/
void CViewCommander::Command_SORT(BOOL bAsc)	//bAsc:TRUE=昇順,FALSE=降順
{
	CLayoutRange sRangeA;
	CLogicRange sSelectOld;

	int			nColmFrom,nColmTo;
	CLayoutInt	nCF,nCT;
	CLayoutInt	nCaretPosYOLD;
	bool		bBeginBoxSelectOld;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int			j;
	CNativeW	cmemBuf;
	std::vector<SORTTABLE> sta;

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){			/* テキストが選択されているか */
		return;
	}

	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		sRangeA=m_pCommanderView->GetSelectionInfo().m_sSelect;
		if( m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().x==m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().x ){
			//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
			m_pCommanderView->GetSelectionInfo().m_sSelect.SetToX( GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
		}
		if(m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().x<m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().x){
			nCF=m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().GetX2();
			nCT=m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().GetX2();
		}else{
			nCF=m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().GetX2();
			nCT=m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().GetX2();
		}
	}
	bBeginBoxSelectOld=m_pCommanderView->GetSelectionInfo().IsBoxSelecting();
	nCaretPosYOLD=GetCaret().GetCaretLayoutPos().GetY();
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		m_pCommanderView->GetSelectionInfo().m_sSelect,
		&sSelectOld
	);

	if( bBeginBoxSelectOld ){
		sSelectOld.GetToPointer()->y++;
	}
	else{
		// カーソル位置が行頭じゃない ＆ 選択範囲の終端に改行コードがある場合は
		// その行も選択範囲に加える
		if ( sSelectOld.GetTo().x > 0 ) {
			// 2006.03.31 Moca nSelectLineToOldは、物理行なのでLayout系からDocLine系に修正
			const CDocLine* pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( sSelectOld.GetTo().GetY2() );
			if( NULL != pcDocLine && EOL_NONE != pcDocLine->GetEol() ){
				sSelectOld.GetToPointer()->y++;
			}
		}
	}
	sSelectOld.SetFromX(CLogicInt(0));
	sSelectOld.SetToX(CLogicInt(0));

	//行選択されてない
	if(sSelectOld.IsLineOne()){
		return;
	}

	for( CLogicInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().y; i++ ){
		const CDocLine* pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( i );
		pLine = GetDocument()->m_cDocLineMgr.GetLine(i)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ) continue;
		SORTTABLE pst = new SORTDATA;
		if( bBeginBoxSelectOld ){
			nColmFrom = m_pCommanderView->LineColmnToIndex( pcDocLine, nCF );
			nColmTo   = m_pCommanderView->LineColmnToIndex( pcDocLine, nCT );
			if(nColmTo<nLineLen){	// BOX選択範囲の右端が行内に収まっている場合
				// 2006.03.31 genta std::string::assignを使って一時変数削除
				pst->sKey1.assign( &pLine[nColmFrom], nColmTo-nColmFrom );
			}
			else if(nColmFrom<nLineLen){	// BOX選択範囲の右端が行末より右にはみ出している場合
				pst->sKey1=&pLine[nColmFrom];
			}
			pst->sKey2=pLine;
		}else{
			pst->sKey1=pLine;
		}
		sta.push_back(pst);
	}
	if(bAsc){
		std::stable_sort(sta.begin(), sta.end(), SortByKeyAsc);
	}else{
		std::stable_sort(sta.begin(), sta.end(), SortByKeyDesc);
	}
	cmemBuf.SetString(L"");
	j=(int)sta.size();
	if( bBeginBoxSelectOld ){
		for (int i=0; i<j; i++) cmemBuf.AppendString( sta[i]->sKey2.c_str() ); 
	}else{
		for (int i=0; i<j; i++) cmemBuf.AppendString( sta[i]->sKey1.c_str() );
	}

	//sta.clear(); ←これじゃだめみたい
	for (int i=0; i<j; i++) delete sta[i];

	CLayoutRange sSelectOld_Layout;
	GetDocument()->m_cLayoutMgr.LogicToLayout(sSelectOld, &sSelectOld_Layout);
	m_pCommanderView->ReplaceData_CEditView(
		sSelectOld_Layout,
		NULL,					/* 削除されたデータのコピー(NULL可能) */
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		false,
		m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
	);

	//	選択エリアの復元
	if(bBeginBoxSelectOld){
		m_pCommanderView->GetSelectionInfo().SetBoxSelect(bBeginBoxSelectOld);
		m_pCommanderView->GetSelectionInfo().m_sSelect=sRangeA;
	}else{
		m_pCommanderView->GetSelectionInfo().m_sSelect=sSelectOld_Layout;
	}
	if(nCaretPosYOLD==m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y || m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ) {
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom(), true );
	}else{
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo(), true );
	}
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		m_pCommanderView->m_pcOpeBlk->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
				GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
			)
		);
	}
	m_pCommanderView->RedrawAll();
}


/*! @brief 物理行のマージ

	連続する物理行で内容が同一の物を1行にまとめます．
	
	矩形選択時はなにも実行しません．
	
	@note 改行コードを含むデータを比較しているので、
	ファイルの最終行はソート対象外にしています
	
	@author hor
	@date 2001.12.03 hor 新規作成
	@date 2001.12.21 hor 選択範囲の調整ロジックを訂正
*/
void CViewCommander::Command_MERGE(void)
{
	CLayoutInt		nCaretPosYOLD;
	const wchar_t*	pLinew;
	CLogicInt		nLineLen;
	int			j;
	CNativeW	cmemBuf;

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){			/* テキストが選択されているか */
		return;
	}
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		return;
	}

	nCaretPosYOLD=GetCaret().GetCaretLayoutPos().GetY();
	CLogicRange sSelectOld; //範囲選択
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		m_pCommanderView->GetSelectionInfo().m_sSelect,
		&sSelectOld
	);

	// 2001.12.21 hor
	// カーソル位置が行頭じゃない ＆ 選択範囲の終端に改行コードがある場合は
	// その行も選択範囲に加える
	if ( sSelectOld.GetTo().x > 0 ) {
		const CLayout* pcLayout=GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().GetY2()); //2007.10.09 kobake 単位混在バグ修正
		if( NULL != pcLayout && EOL_NONE != pcLayout->GetLayoutEol() ){
			sSelectOld.GetToPointer()->y++;
			//sSelectOld.GetTo().y++;
		}
	}

	sSelectOld.SetFromX(CLogicInt(0));
	sSelectOld.SetToX(CLogicInt(0));

	//行選択されてない
	if(sSelectOld.IsLineOne()){
		return;
	}

	pLinew=NULL;
	cmemBuf.SetString(L"");
	for( CLogicInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().y; i++ ){
		const wchar_t*	pLine = GetDocument()->m_cDocLineMgr.GetLine(i)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ) continue;
		if( NULL == pLinew || wcscmp(pLine,pLinew) ){
			cmemBuf.AppendString( pLine );
		}
		pLinew=pLine;
	}
	j=GetDocument()->m_cDocLineMgr.GetLineCount();

	CLayoutRange sSelectOld_Layout;
	GetDocument()->m_cLayoutMgr.LogicToLayout(sSelectOld, &sSelectOld_Layout);

	m_pCommanderView->ReplaceData_CEditView(
		sSelectOld_Layout,
		NULL,					/* 削除されたデータのコピー(NULL可能) */
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		false,
		m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
	);
	j-=GetDocument()->m_cDocLineMgr.GetLineCount();

	//	選択エリアの復元
	m_pCommanderView->GetSelectionInfo().m_sSelect=sSelectOld_Layout;
	m_pCommanderView->GetSelectionInfo().m_sSelect.GetToPointer()->y -= j;

	if(nCaretPosYOLD==m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y){
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom(), true );
	}else{
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo(), true );
	}
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		m_pCommanderView->m_pcOpeBlk->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
				GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
			)
		);
	}
	m_pCommanderView->RedrawAll();

	if(j){
		TopOkMessage( m_pCommanderView->GetHwnd(), _T("%d行をマージしました。"), j);
	}else{
		InfoMessage( m_pCommanderView->GetHwnd(), _T("マージ可能な行がみつかりませんでした。") );
	}
}



// To Here 2001.12.03 hor
	
/* メニューからの再変換対応 minfu 2002.04.09

	@date 2002.04.11 YAZAKI COsVersionInfoのカプセル化を守りましょう。
	@date 2010.03.17 ATOK用はSCS_SETRECONVERTSTRING => ATRECONVERTSTRING_SETに変更
		2002.11.20 Stoneeさんの情報
*/
void CViewCommander::Command_Reconvert(void)
{
	const int ATRECONVERTSTRING_SET = 1;

	//サイズを取得
	int nSize = m_pCommanderView->SetReconvertStruct(NULL,UNICODE_BOOL);
	if( 0 == nSize )  // サイズ０の時は何もしない
		return ;
	
	bool bUseUnicodeATOK = false;
	//バージョンチェック
	COsVersionInfo cOs;
	if( cOs.OsDoesNOTSupportReconvert() ){
		
		// MSIMEかどうか
		HWND hWnd = ImmGetDefaultIMEWnd(m_pCommanderView->GetHwnd());
		if (SendMessage(hWnd, m_pCommanderView->m_uWM_MSIME_RECONVERTREQUEST, FID_RECONVERT_VERSION, 0)){
			SendMessage(hWnd, m_pCommanderView->m_uWM_MSIME_RECONVERTREQUEST, 0, (LPARAM)m_pCommanderView->GetHwnd());
			return ;
		}

		// ATOKが使えるかどうか
		TCHAR sz[256];
		ImmGetDescription(GetKeyboardLayout(0),sz,_countof(sz)); //説明の取得
		if ( (_tcsncmp(sz,_T("ATOK"),4) == 0) && (NULL != m_pCommanderView->m_AT_ImmSetReconvertString) ){
			bUseUnicodeATOK = true;
		}else{
			//対応IMEなし
			return;
		}
	}else{
		//現在のIMEが対応しているかどうか
		//IMEのプロパティ
		if ( !(ImmGetProperty(GetKeyboardLayout(0),IGP_SETCOMPSTR) & SCS_CAP_SETRECONVERTSTRING) ){
			//対応IMEなし			
			return ;
		}
#ifdef _UNICODE
#endif
	}
	
	//サイズ取得し直し
	if((UNICODE_BOOL || bUseUnicodeATOK) != UNICODE_BOOL){
		nSize = m_pCommanderView->SetReconvertStruct(NULL,UNICODE_BOOL || bUseUnicodeATOK);
		if( 0 == nSize )  // サイズ０の時は何もしない
			return ;
	}

	//IMEのコンテキスト取得
	HIMC hIMC = ::ImmGetContext( m_pCommanderView->GetHwnd() );
	
	//領域確保
	PRECONVERTSTRING pReconv = (PRECONVERTSTRING)::HeapAlloc(
		GetProcessHeap(),
		HEAP_GENERATE_EXCEPTIONS,
		nSize
	);
	
	//構造体設定
	// Sizeはバッファ確保側が設定
	pReconv->dwSize = nSize;
	pReconv->dwVersion = 0;
	m_pCommanderView->SetReconvertStruct( pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	//変換範囲の調整
	if(bUseUnicodeATOK){
		(*m_pCommanderView->m_AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize, NULL,0);
	}

	//調整した変換範囲を選択する
	m_pCommanderView->SetSelectionFromReonvert(pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	//再変換実行
	if(bUseUnicodeATOK){
		(*m_pCommanderView->m_AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize, NULL, 0);
	}

	//領域解放
	::HeapFree(GetProcessHeap(),0,(LPVOID)pReconv);
	::ImmReleaseContext( m_pCommanderView->GetHwnd(), hIMC);
}

/*!	コントロールコードの入力(ダイアログ)
	@author	MIK
	@date	2002/06/02
*/
void CViewCommander::Command_CtrlCode_Dialog( void )
{
	CDlgCtrlCode	cDlgCtrlCode;

	//コントロールコード入力ダイアログを表示する
	if( cDlgCtrlCode.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() ) )
	{
		//コントロールコードを入力する
		Command_WCHAR( cDlgCtrlCode.GetCharCode() );
	}
}

/*!	検索開始位置へ戻る
	@author	ai
	@date	02/06/26
*/
void CViewCommander::Command_JUMP_SRCHSTARTPOS(void)
{
	if( m_pCommanderView->m_ptSrchStartPos_PHY.BothNatural() )
	{
		CLayoutPoint pt;
		/* 範囲選択中か */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			m_pCommanderView->m_ptSrchStartPos_PHY,
			&pt
		);
		//	2006.07.09 genta 選択状態を保つ
		m_pCommanderView->MoveCursorSelecting( pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
	else
	{
		ErrorBeep();
	}
	return;
}

/*!	履歴の管理(ダイアログ)
	@author	MIK
	@date	2003/04/07
*/
void CViewCommander::Command_Favorite( void )
{
	CDlgFavorite	cDlgFavorite;

	//ダイアログを表示する
	if( !cDlgFavorite.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() ) )
	{
		return;
	}

	return;
}

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

/*! 常に手前に表示
	@date 2004.09.21 Moca
*/
void CViewCommander::Command_WINTOPMOST( LPARAM lparam )
{
	GetDocument()->m_pcEditWnd->WindowTopMost( int(lparam) );
}

/*!	@brief 引用符の設定
	@date Jan. 29, 2005 genta 新規作成
*/
void CViewCommander::Command_SET_QUOTESTRING( const wchar_t* quotestr )
{
	if( quotestr == NULL )
		return;

	wcsncpy( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou, quotestr,
		_countof( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou ));
	
	GetDllShareData().m_Common.m_sFormat.m_szInyouKigou[ _countof( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou ) - 1 ] = L'\0';
}

/*!	@brief ウィンドウ一覧ポップアップ表示処理（ファイル名のみ）
	@date  2006.03.23 fon 新規作成
	@date  2006.05.19 genta コマンド実行要因を表す引数追加
	@date  2007.07.07 genta コマンド実行要因の値を変更
*/
void CViewCommander::Command_WINLIST( int nCommandFrom )
{
	CEditWnd	*pCEditWnd;
	pCEditWnd = GetDocument()->m_pcEditWnd;

	//ウィンドウ一覧をポップアップ表示する
	pCEditWnd->PopupWinList(( nCommandFrom & FA_FROMKEYBOARD ) != FA_FROMKEYBOARD );
	// 2007.02.27 ryoji アクセラレータキーからでなければマウス位置に

}


/*!	@brief マクロ用アウトプットウインドウに表示
	@date 2006.04.26 maru 新規作成
*/
void CViewCommander::Command_TRACEOUT( const wchar_t* outputstr, int nFlgOpt )
{
	if( outputstr == NULL )
		return;

	// 0x01 ExpandParameterによる文字列展開有無
	if (nFlgOpt & 0x01) {
		wchar_t Buffer[2048];
		CSakuraEnvironment::ExpandParameter(outputstr, Buffer, 2047);
		CShareData::getInstance()->TraceOutString( Buffer );
	} else {
		CShareData::getInstance()->TraceOutString(outputstr );
	}

	// 0x02 改行コードの有無
	if ((nFlgOpt & 0x02) == 0) CShareData::getInstance()->TraceOutString( L"\r\n" );

}



