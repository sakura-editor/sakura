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
