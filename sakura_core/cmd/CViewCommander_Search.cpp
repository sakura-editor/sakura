/*!	@file
@brief CViewCommanderクラスのコマンド(検索系 基本形)関数群

	2012/12/17	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, hor, YAZAKI
	Copyright (C) 2002, hor, YAZAKI, novice, Azumaiya, Moca
	Copyright (C) 2003, かろと
	Copyright (C) 2004, Moca
	Copyright (C) 2005, かろと, Moca, D.S.Koba
	Copyright (C) 2006, genta, ryoji, かろと, yukihane
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2009, ryoji, genta
	Copyright (C) 2010, ryoji
	Copyright (C) 2011, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
#include "dlg/CDlgCancel.h"// 2002/2/8 hor
#include "window/CEditWnd.h"/// 2002/2/3 aroka 追加
#include "util/window.h"
#include "util/string_ex2.h"
#include <limits.h>
#include "sakura_rc.h"


/*!
検索(ボックス)コマンド実行.
ツールバーの検索ボックスにフォーカスを移動する.
	@date 2006.06.04 yukihane 新規作成
*/
void CViewCommander::Command_SEARCH_BOX( void )
{
	const CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;
	pCEditWnd->m_cToolbar.SetFocusSearchBox();
}



/* 検索(単語検索ダイアログ) */
void CViewCommander::Command_SEARCH_DIALOG( void )
{
//	int			nRet;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	CNativeW		cmemCurText;
	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji ダイアログ専用関数に変更

	/* 検索文字列を初期化 */
	if( 0 < cmemCurText.GetStringLength() ){
		GetEditWindow()->m_cDlgFind.m_strText = cmemCurText.GetStringPtr();
	}
	/* 検索ダイアログの表示 */
	if( NULL == GetEditWindow()->m_cDlgFind.GetHwnd() ){
		GetEditWindow()->m_cDlgFind.DoModeless( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)&GetEditWindow()->GetActiveView() );
	}
	else{
		/* アクティブにする */
		ActivateFrameWindow( GetEditWindow()->m_cDlgFind.GetHwnd() );
		::DlgItem_SetText( GetEditWindow()->m_cDlgFind.GetHwnd(), IDC_COMBO_TEXT, cmemCurText.GetStringT() );
	}
	return;
}



/*! 次を検索
	@param bChangeCurRegexp 共有データの検索文字列を使う
	@date 2003.05.22 かろと 無限マッチ対策．行頭・行末処理見直し．
	@date 2004.05.30 Moca bChangeCurRegexp=trueで従来通り。falseで、CEditViewの現在設定されている検索パターンを使う
*/
void CViewCommander::Command_SEARCH_NEXT(
	bool			bChangeCurRegexp,
	bool			bRedraw,
	HWND			hwndParent,
	const WCHAR*	pszNotFoundMessage
)
{
	bool		bSelecting;
	bool		bFlag1;
	bool		bSelectingLock_Old;
	bool		bFound = false;
	bool		bDisableSelect = false;
	const CLayout* pcLayout;
	bool b0Match = false;		//!< 長さ０でマッチしているか？フラグ by かろと
	const wchar_t *pLine;
	CLogicInt nIdx;
	CLogicInt	nLineLen;
	CLayoutInt	nLineNum;

	CLayoutRange sRangeA;
	sRangeA.Set(GetCaret().GetCaretLayoutPos());

	CLayoutRange sSelectBgn_Old;
	CLayoutRange sSelect_Old;
	CLayoutInt  nLineNumOld;

	bool bRedo;		//	hor
	int  nIdxOld;		//	hor
	int nSearchResult;

	bSelecting = FALSE;
	// 2002.01.16 hor
	// 共通部分のくくりだし
	// 2004.05.30 Moca CEditViewの現在設定されている検索パターンを使えるように
	if(bChangeCurRegexp && !m_pCommanderView->ChangeCurRegexp())return;
	if( 0 == m_pCommanderView->m_strCurSearchKey.size() ){
		goto end_of_func;
	}

	// 検索開始位置を調整
	bFlag1 = FALSE;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 矩形範囲選択中でない & 選択状態のロック */
		if( !m_pCommanderView->GetSelectionInfo().IsBoxSelecting() && m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			bSelecting = TRUE;
			bSelectingLock_Old = m_pCommanderView->GetSelectionInfo().m_bSelectingLock;

			sSelectBgn_Old = m_pCommanderView->GetSelectionInfo().m_sSelectBgn; //範囲選択(原点)
			sSelect_Old = GetSelect();

			if( PointCompare(m_pCommanderView->GetSelectionInfo().m_sSelectBgn.GetFrom(),GetCaret().GetCaretLayoutPos()) >= 0 ){
				// カーソル移動
				GetCaret().SetCaretLayoutPos(GetSelect().GetFrom());
				if (GetSelect().IsOne()) {
					// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
					b0Match = true;
				}
				bFlag1 = TRUE;
			}
			else{
				// カーソル移動
				GetCaret().SetCaretLayoutPos(GetSelect().GetTo());
				if (GetSelect().IsOne()) {
					// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
					b0Match = true;
				}
			}
		}
		else{
			/* カーソル移動 */
			GetCaret().SetCaretLayoutPos(GetSelect().GetTo());
			if (GetSelect().IsOne()) {
				// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
				b0Match = true;
			}

			/* 現在の選択範囲を非選択状態に戻す */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( bRedraw, false );
			bDisableSelect = true;
		}
	}
	nLineNum = GetCaret().GetCaretLayoutPos().GetY2();
	nLineLen = CLogicInt(0); // 2004.03.17 Moca NULL == pLineのとき、nLineLenが未設定になり落ちるバグ対策
	pLine = GetDocument()->m_cLayoutMgr.GetLineStr(nLineNum, &nLineLen, &pcLayout);

	/* 指定された桁に対応する行のデータ内の位置を調べる */
// 2002.02.08 hor EOFのみの行からも次検索しても再検索可能に (2/2)
	nIdx = pcLayout ? m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() ) : CLogicInt(0);
	if( b0Match ) {
		// 現在、長さ０でマッチしている場合は物理行で１文字進める(無限マッチ対策)
		if( nIdx < nLineLen ) {
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nIdx += CLogicInt(CNativeW::GetSizeOfChar(pLine, nLineLen, nIdx) == 2 ? 2 : 1);
		} else {
			// 念のため行末は別処理
			++nIdx;
		}
	}

	nLineNumOld = nLineNum;	//	hor
	bRedo			= TRUE;		//	hor
	nIdxOld			= nIdx;		//	hor

re_do:;
	 /* 現在位置より後ろの位置を検索する */
	// 2004.05.30 Moca 引数をGetShareData()からメンバ変数に変更。他のプロセス/スレッドに書き換えられてしまわないように。
	nSearchResult = GetDocument()->m_cLayoutMgr.SearchWord(
		nLineNum,						// 検索開始レイアウト行
		nIdx,							// 検索開始データ位置
		m_pCommanderView->m_strCurSearchKey.c_str(),					// 検索条件
		SEARCH_FORWARD,					// 0==前方検索 1==後方検索
		m_pCommanderView->m_sCurSearchOption,				// 検索オプション
		&sRangeA,						// マッチレイアウト範囲
		&m_pCommanderView->m_CurRegexp					// 正規表現コンパイルデータ
	);
	if( nSearchResult ){
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		if( bFlag1 && sRangeA.GetFrom()==GetCaret().GetCaretLayoutPos() ){
			CLogicRange sRange_Logic;
			GetDocument()->m_cLayoutMgr.LayoutToLogic(sRangeA,&sRange_Logic);

			nLineNum = sRangeA.GetTo().GetY2();
			nIdx     = sRange_Logic.GetTo().GetX2();
			if( sRange_Logic.GetFrom() == sRange_Logic.GetTo() ) { // 幅0マッチでの無限ループ対策。
				nIdx += 1; // wchar_t一個分進めるだけでは足りないかもしれないが。
			}
			goto re_do;
		}

		if( bSelecting ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRangeA.GetTo() );
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */
		}
		else{
			/* 選択範囲の変更 */
			//	2005.06.24 Moca
			m_pCommanderView->GetSelectionInfo().SetSelectArea( sRangeA );

			if( bRedraw ){
				/* 選択領域描画 */
				m_pCommanderView->GetSelectionInfo().DrawSelectArea();
			}
		}

		/* カーソル移動 */
		//	Sep. 8, 2000 genta
		if ( m_pCommanderView->GetDrawSwitch() ) m_pCommanderView->AddCurrentLineToHistory();	// 2002.02.16 hor すべて置換のときは不要
		GetCaret().MoveCursor( sRangeA.GetFrom(), bRedraw );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		bFound = TRUE;
	}
	else{
		if( bSelecting ){
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */

			/* 選択範囲の変更 */
			m_pCommanderView->GetSelectionInfo().m_sSelectBgn = sSelectBgn_Old; //範囲選択(原点)
			m_pCommanderView->GetSelectionInfo().m_sSelectOld = sSelect_Old;	// 2011.12.24
			GetSelect().SetFrom(sSelect_Old.GetFrom());
			GetSelect().SetTo(sRangeA.GetFrom());

			/* カーソル移動 */
			GetCaret().MoveCursor( sRangeA.GetFrom(), bRedraw );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			if( bRedraw ){
				/* 選択領域描画 */
				m_pCommanderView->GetSelectionInfo().DrawSelectArea();
			}
		}else{
			if( bDisableSelect ){
				// 2011.12.21 ロジックカーソル位置の修正/カーソル線・対括弧の表示
				CLogicPoint ptLogic;
				GetDocument()->m_cLayoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos(), &ptLogic);
				GetCaret().SetCaretLogicPos(ptLogic);
				m_pCommanderView->DrawBracketCursorLine(bRedraw);
			}
		}
	}

end_of_func:;
// From Here 2002.01.26 hor 先頭（末尾）から再検索
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&		// 見つからなかった
			bRedo	&&		// 最初の検索
			m_pCommanderView->GetDrawSwitch()	// 全て置換の実行中じゃない
		){
			nLineNum=CLayoutInt(0);
			nIdx=CLogicInt(0);
			bRedo=FALSE;
			goto re_do;		// 先頭から再検索
		}
	}

	if(bFound){
		if((nLineNumOld > nLineNum)||(nLineNumOld == nLineNum && nIdxOld > nIdx))
			m_pCommanderView->SendStatusMessage(_T("▼先頭から再検索しました"));
	}
	else{
		GetCaret().ShowEditCaret();	// 2002/04/18 YAZAKI
		GetCaret().ShowCaretPosInfo();	// 2002/04/18 YAZAKI
		m_pCommanderView->SendStatusMessage(_T("▽見つかりませんでした"));
// To Here 2002.01.26 hor

		/* 検索／置換  見つからないときメッセージを表示 */
		if( NULL == pszNotFoundMessage ){
			CNativeW KeyName;
			LimitStringLengthW(m_pCommanderView->m_strCurSearchKey.c_str(), m_pCommanderView->m_strCurSearchKey.size(),
				_MAX_PATH, KeyName);
			if( (size_t)KeyName.GetStringLength() < m_pCommanderView->m_strCurSearchKey.size() ){
				KeyName.AppendString( L"..." );
			}
			AlertNotFound(
				hwndParent,
				_T("前方(↓) に文字列 '%ls' が１つも見つかりません。"),
				KeyName.GetStringPtr()
			);
		}
		else{
			AlertNotFound(hwndParent, _T("%ls"),pszNotFoundMessage);
		}
	}
}



/* 前を検索 */
void CViewCommander::Command_SEARCH_PREV( bool bReDraw, HWND hwndParent )
{
	bool		bSelecting;
	bool		bSelectingLock_Old;
	bool		bFound = false;
	bool		bRedo = false;			//	hor
	bool		bDisableSelect = false;
	CLayoutInt	nLineNumOld;
	CLogicInt	nIdxOld;
	const CLayout* pcLayout = NULL;
	CLayoutInt nLineNum;
	CLogicInt nIdx;

	CLayoutRange sRangeA;
	sRangeA.Set(GetCaret().GetCaretLayoutPos());

	CLayoutRange sSelectBgn_Old;
	CLayoutRange sSelect_Old;

	//	bFlag1 = FALSE;
	bSelecting = FALSE;
	// 2002.01.16 hor
	// 共通部分のくくりだし
	if(!m_pCommanderView->ChangeCurRegexp()){
		return;
	}
	if( 0 == m_pCommanderView->m_strCurSearchKey.size() ){
		goto end_of_func;
	}
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		sSelectBgn_Old = m_pCommanderView->GetSelectionInfo().m_sSelectBgn; //範囲選択(原点)
		sSelect_Old = GetSelect();
		
		bSelectingLock_Old = m_pCommanderView->GetSelectionInfo().m_bSelectingLock;

		/* 矩形範囲選択中か */
		if( !m_pCommanderView->GetSelectionInfo().IsBoxSelecting() && TRUE == m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){	/* 選択状態のロック */
			bSelecting = TRUE;
		}
		else{
			/* 現在の選択範囲を非選択状態に戻す */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( bReDraw, false );
			bDisableSelect = true;
		}
	}

	nLineNum = GetCaret().GetCaretLayoutPos().GetY2();
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );


	if( NULL == pcLayout ){
		// pcLayoutはNULLとなるのは、[EOF]から前検索した場合
		// １行前に移動する処理
		nLineNum--;
		if( nLineNum < 0 ){
			goto end_of_func;
		}
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
		if( NULL == pcLayout ){
			goto end_of_func;
		}
		// カーソル左移動はやめて nIdxは行の長さとしないと[EOF]から改行を前検索した時に最後の改行を検索できない 2003.05.04 かろと
		CLayout* pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
		nIdx = CLogicInt(pCLayout->GetDocLineRef()->GetLengthWithEOL() + 1);		// 行末のヌル文字(\0)にマッチさせるために+1 2003.05.16 かろと
	} else {
		/* 指定された桁に対応する行のデータ内の位置を調べる */
		nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
	}

	bRedo		=	TRUE;		//	hor
	nLineNumOld	=	nLineNum;	//	hor
	nIdxOld		=	nIdx;		//	hor
re_do:;							//	hor
	/* 現在位置より前の位置を検索する */
	if( GetDocument()->m_cLayoutMgr.SearchWord(
		nLineNum,								// 検索開始レイアウト行
		nIdx,									// 検索開始データ位置
		m_pCommanderView->m_strCurSearchKey.c_str(),							// 検索条件
		SEARCH_BACKWARD,						// 0==前方検索 1==後方検索
		m_pCommanderView->m_sCurSearchOption,						// 検索オプション
		&sRangeA,								// マッチレイアウト範囲
		&m_pCommanderView->m_CurRegexp							// 正規表現コンパイルデータ
	) ){
		if( bSelecting ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRangeA.GetFrom() );
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */
		}else{
			/* 選択範囲の変更 */
			//	2005.06.24 Moca
			m_pCommanderView->GetSelectionInfo().SetSelectArea( sRangeA );

			if( bReDraw ){
				/* 選択領域描画 */
				m_pCommanderView->GetSelectionInfo().DrawSelectArea();
			}
		}
		/* カーソル移動 */
		//	Sep. 8, 2000 genta
		m_pCommanderView->AddCurrentLineToHistory();
		GetCaret().MoveCursor( sRangeA.GetFrom(), bReDraw );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		bFound = TRUE;
	}else{
		if( bSelecting ){
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */
			/* 選択範囲の変更 */
			m_pCommanderView->GetSelectionInfo().m_sSelectBgn = sSelectBgn_Old;
			GetSelect() = sSelect_Old;

			/* カーソル移動 */
			GetCaret().MoveCursor( sRangeA.GetFrom(), bReDraw );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			/* 選択領域描画 */
			m_pCommanderView->GetSelectionInfo().DrawSelectArea();
		}else{
			if( bDisableSelect ){
				m_pCommanderView->DrawBracketCursorLine(bReDraw);
			}
		}
	}
end_of_func:;
// From Here 2002.01.26 hor 先頭（末尾）から再検索
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&	// 見つからなかった
			bRedo		// 最初の検索
		){
			nLineNum=GetDocument()->m_cLayoutMgr.GetLineCount()-CLayoutInt(1);
			nIdx=CLogicInt(MAXLINEKETAS);
			bRedo=FALSE;
			goto re_do;	// 末尾から再検索
		}
	}
	if(bFound){
		if((nLineNumOld < nLineNum)||(nLineNumOld == nLineNum && nIdxOld < nIdx))
			m_pCommanderView->SendStatusMessage(_T("▲末尾から再検索しました"));
	}else{
		m_pCommanderView->SendStatusMessage(_T("△見つかりませんでした"));
//	if( !bFound ){
// To Here 2002.01.26 hor

		/* 検索／置換  見つからないときメッセージを表示 */
		CNativeW KeyName;
		LimitStringLengthW(m_pCommanderView->m_strCurSearchKey.c_str(), m_pCommanderView->m_strCurSearchKey.size(),
			_MAX_PATH, KeyName);
		if( (size_t)KeyName.GetStringLength() < m_pCommanderView->m_strCurSearchKey.size() ){
			KeyName.AppendString( L"..." );
		}
		AlertNotFound(
			hwndParent,
			_T("後方(↑) に文字列 '%ls' が１つも見つかりません。"),	//Jan. 25, 2001 jepro メッセージを若干変更
			KeyName.GetStringPtr()
		);
	}
	return;
}



//置換(置換ダイアログ)
void CViewCommander::Command_REPLACE_DIALOG( void )
{
	BOOL		bSelected = FALSE;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	CNativeW	cmemCurText;
	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji ダイアログ専用関数に変更

	/* 検索文字列を初期化 */
	if( 0 < cmemCurText.GetStringLength() ){
		GetEditWindow()->m_cDlgReplace.m_strText = cmemCurText.GetStringPtr();
	}
	if( 0 < GetDllShareData().m_sSearchKeywords.m_aReplaceKeys.size() ){
		if( GetEditWindow()->m_cDlgReplace.m_nReplaceKeySequence < GetDllShareData().m_Common.m_sSearch.m_nReplaceKeySequence ){
			GetEditWindow()->m_cDlgReplace.m_strText2 = GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0];	// 2006.08.23 ryoji 前回の置換後文字列を引き継ぐ
		}
	}
	
	if ( m_pCommanderView->GetSelectionInfo().IsTextSelected() && !GetSelect().IsLineOne() ) {
		bSelected = TRUE;	//選択範囲をチェックしてダイアログ表示
	}else{
		bSelected = FALSE;	//ファイル全体をチェックしてダイアログ表示
	}
	/* 置換オプションの初期化 */
	GetEditWindow()->m_cDlgReplace.m_nReplaceTarget=0;	/* 置換対象 */
	GetEditWindow()->m_cDlgReplace.m_nPaste=FALSE;		/* 貼り付ける？ */
// To Here 2001.12.03 hor

	/* 置換ダイアログの表示 */
	//	From Here Jul. 2, 2001 genta 置換ウィンドウの2重開きを抑止
	if( !::IsWindow( GetEditWindow()->m_cDlgReplace.GetHwnd() ) ){
		GetEditWindow()->m_cDlgReplace.DoModeless( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)m_pCommanderView, bSelected );
	}
	else {
		/* アクティブにする */
		ActivateFrameWindow( GetEditWindow()->m_cDlgReplace.GetHwnd() );
		::DlgItem_SetText( GetEditWindow()->m_cDlgReplace.GetHwnd(), IDC_COMBO_TEXT, cmemCurText.GetStringT() );
	}
	//	To Here Jul. 2, 2001 genta 置換ウィンドウの2重開きを抑止
	return;
}



/*! 置換実行
	
	@date 2002/04/08 親ウィンドウを指定するように変更。
	@date 2003.05.17 かろと 長さ０マッチの無限置換回避など
	@date 2011.12.18 Moca オプション・検索キーをDllShareDataからm_cDlgReplace/EditViewベースに変更。文字列長制限の撤廃
*/
void CViewCommander::Command_REPLACE( HWND hwndParent )
{
	if ( hwndParent == NULL ){	//	親ウィンドウが指定されていなければ、CEditViewが親。
		hwndParent = m_pCommanderView->GetHwnd();
	}
	//2002.02.10 hor
	int nPaste			=	GetEditWindow()->m_cDlgReplace.m_nPaste;
	int nReplaceTarget	=	GetEditWindow()->m_cDlgReplace.m_nReplaceTarget;

	// From Here 2001.12.03 hor
	if( nPaste && !GetDocument()->m_cDocEditor.IsEnablePaste()){
		OkMessage( hwndParent, _T("クリップボードに有効なデータがありません！") );
		::CheckDlgButton( GetEditWindow()->m_cDlgReplace.GetHwnd(), IDC_CHK_PASTE, FALSE );
		::EnableWindow( ::GetDlgItem( GetEditWindow()->m_cDlgReplace.GetHwnd(), IDC_COMBO_TEXT2 ), TRUE );
		return;	//	失敗return;
	}

	// 2002.01.09 hor
	// 選択エリアがあれば、その先頭にカーソルを移す
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			GetCaret().MoveCursor( GetSelect().GetFrom(), true );
		} else {
//			HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );
			Command_LEFT( FALSE, FALSE );
		}
	}
	// To Here 2002.01.09 hor
	
	// 矩形選択？
//			bBeginBoxSelect = m_pCommanderView->GetSelectionInfo().IsBoxSelecting();

	/* カーソル左移動 */
	//HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );	//？？？
	// To Here 2001.12.03 hor

	/* テキスト選択解除 */
	/* 現在の選択範囲を非選択状態に戻す */
	m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );

	// 2004.06.01 Moca 検索中に、他のプロセスによってm_aReplaceKeysが書き換えられても大丈夫なように
	const CNativeW	cMemRepKey( GetEditWindow()->m_cDlgReplace.m_strText2.c_str() );

	/* 次を検索 */
	Command_SEARCH_NEXT( true, TRUE, hwndParent, 0 );

	BOOL	bRegularExp = m_pCommanderView->m_sCurSearchOption.bRegularExp;
	int 	nFlag       = m_pCommanderView->m_sCurSearchOption.bLoHiCase ? 0x01 : 0x00;

	/* テキストが選択されているか */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		// From Here 2001.12.03 hor
		CLayoutPoint ptTmp(0,0);
		if ( nPaste || !bRegularExp ) {
			// 正規表現時は 後方参照($&)で実現するので、正規表現は除外
			if(nReplaceTarget==1){	//挿入位置へ移動
				ptTmp = GetSelect().GetTo() - GetSelect().GetFrom();
				GetSelect().Clear(-1);
			}
			else if(nReplaceTarget==2){	//追加位置へ移動
				// 正規表現を除外したので、「検索後の文字が改行やったら次の行の先頭へ移動」の処理を削除
				GetCaret().MoveCursor(GetSelect().GetTo(), false);
				GetSelect().Clear(-1);
			}
			else{
				// 位置指定ないので、何もしない
			}
		}
		/* コマンドコードによる処理振り分け */
		/* テキストを貼り付け */
		if(nPaste){
			Command_PASTE(0);
		} else if ( bRegularExp ) { /* 検索／置換  1==正規表現 */
			// 先読みに対応するために物理行末までを使うように変更 2005/03/27 かろと
			// 2002/01/19 novice 正規表現による文字列置換
			CBregexp cRegexp;

			if( !InitRegexp( m_pCommanderView->GetHwnd(), cRegexp, true ) ){
				return;	//	失敗return;
			}

			// 物理行、物理行長、物理行での検索マッチ位置
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
			const wchar_t* pLine = pcLayout->GetDocLineRef()->GetPtr();
			CLogicInt nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetSelect().GetFrom().GetX2() ) + pcLayout->GetLogicOffset();
			CLogicInt nLen = pcLayout->GetDocLineRef()->GetLengthWithEOL();
			// 正規表現で選択始点・終点への挿入を記述
			//	Jun. 6, 2005 かろと
			// →これでは「検索の後ろの文字が改行だったら次の行頭へ移動」が処理できない
			// → Oct. 30, 「検索の後ろの文字が改行だったら・・」の処理をやめる（誰もしらないみたいなので）
			// Nov. 9, 2005 かろと 正規表現で選択始点・終点への挿入方法を変更(再)
			CNativeW cMemMatchStr; cMemMatchStr.SetString(L"$&");
			CNativeW cMemRepKey2;
			if (nReplaceTarget == 1) {	//選択始点へ挿入
				cMemRepKey2 = cMemRepKey;
				cMemRepKey2 += cMemMatchStr;
			} else if (nReplaceTarget == 2) { // 選択終点へ挿入
				cMemRepKey2 = cMemMatchStr;
				cMemRepKey2 += cMemRepKey;
			} else {
				cMemRepKey2 = cMemRepKey;
			}
			cRegexp.Compile( m_pCommanderView->m_strCurSearchKey.c_str(), cMemRepKey2.GetStringPtr(), nFlag);
			if( cRegexp.Replace(pLine, nLen, nIdx) ){
				// From Here Jun. 6, 2005 かろと
				// 物理行末までINSTEXTする方法は、キャレット位置を調整する必要があり、
				// キャレット位置の計算が複雑になる。（置換後に改行がある場合に不具合発生）
				// そこで、INSTEXTする文字列長を調整する方法に変更する（実はこっちの方がわかりやすい）
				CLayoutMgr& rLayoutMgr = GetDocument()->m_cLayoutMgr;
				CLogicInt matchLen = cRegexp.GetMatchLen();
				CLogicInt nIdxTo = nIdx + matchLen;		// 検索文字列の末尾
				if (matchLen == 0) {
					// ０文字マッチの時(無限置換にならないように１文字進める)
					if (nIdxTo < nLen) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxTo += CLogicInt(CNativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					}
					// 無限置換しないように、１文字増やしたので１文字選択に変更
					// 選択始点・終点への挿入の場合も０文字マッチ時は動作は同じになるので
					rLayoutMgr.LogicToLayout( CLogicPoint(nIdxTo, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji 行位置も取得する
				}
				// 行末から検索文字列末尾までの文字数
				CLogicInt colDiff = nLen - nIdxTo;
				//	Oct. 22, 2005 Karoto
				//	\rを置換するとその後ろの\nが消えてしまう問題の対応
				if (colDiff < pcLayout->GetDocLineRef()->GetEol().GetLen()) {
					// 改行にかかっていたら、行全体をINSTEXTする。
					colDiff = CLogicInt(0);
					rLayoutMgr.LogicToLayout( CLogicPoint(nLen, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji 追加
				}
				// 置換後文字列への書き換え(行末から検索文字列末尾までの文字を除く)
				Command_INSTEXT( FALSE, cRegexp.GetString(), cRegexp.GetStringLen() - colDiff, TRUE );
				// To Here Jun. 6, 2005 かろと
			}
		}else{
			//	HandleCommand( F_INSTEXT_W, FALSE, (LPARAM)GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0], FALSE, 0, 0 );
			Command_INSTEXT( FALSE, cMemRepKey.GetStringPtr(), cMemRepKey.GetStringLength(), TRUE );
		}

		// 挿入後の検索開始位置を調整
		if(nReplaceTarget==1){
			GetCaret().SetCaretLayoutPos(GetCaret().GetCaretLayoutPos()+ptTmp);
		}

		// To Here 2001.12.03 hor
		/* 最後まで置換した時にOK押すまで置換前の状態が表示されるので、
		** 置換後、次を検索する前に書き直す 2003.05.17 かろと
		*/
		m_pCommanderView->Redraw();

		/* 次を検索 */
		Command_SEARCH_NEXT( true, TRUE, hwndParent, LTEXT("最後まで置換しました。") );
	}
}



/*! すべて置換実行

	@date 2003.05.22 かろと 無限マッチ対策．行頭・行末処理など見直し
	@date 2006.03.31 かろと 行置換機能追加
	@date 2007.01.16 ryoji 行置換機能を全置換のオプションに変更
	@date 2009.09.20 genta 左下～右上で矩形選択された領域の置換が行われない
	@date 2010.09.17 ryoji ラインモード貼り付け処理を追加
	@date 2011.12.18 Moca オプション・検索キーをDllShareDataからm_cDlgReplace/EditViewベースに変更。文字列長制限の撤廃
*/
void CViewCommander::Command_REPLACE_ALL()
{

	// m_sSearchOption選択のための先に適用
	if( !m_pCommanderView->ChangeCurRegexp() ){
		return;
	}

	//2002.02.10 hor
	BOOL nPaste			= GetEditWindow()->m_cDlgReplace.m_nPaste;
	BOOL nReplaceTarget	= GetEditWindow()->m_cDlgReplace.m_nReplaceTarget;
	BOOL bRegularExp	= m_pCommanderView->m_sCurSearchOption.bRegularExp;
	BOOL bSelectedArea	= GetEditWindow()->m_cDlgReplace.m_bSelectedArea;
	BOOL bConsecutiveAll = GetEditWindow()->m_cDlgReplace.m_bConsecutiveAll;	/* 「すべて置換」は置換の繰返し */	// 2007.01.16 ryoji

	GetEditWindow()->m_cDlgReplace.m_bCanceled=false;
	GetEditWindow()->m_cDlgReplace.m_nReplaceCnt=0;

	// From Here 2001.12.03 hor
	if( nPaste && !GetDocument()->m_cDocEditor.IsEnablePaste() ){
		OkMessage( m_pCommanderView->GetHwnd(), _T("クリップボードに有効なデータがありません！") );
		::CheckDlgButton( m_pCommanderView->GetHwnd(), IDC_CHK_PASTE, FALSE );
		::EnableWindow( ::GetDlgItem( m_pCommanderView->GetHwnd(), IDC_COMBO_TEXT2 ), TRUE );
		return;	// TRUE;
	}
	// To Here 2001.12.03 hor

	bool		bBeginBoxSelect; // 矩形選択？
	if(m_pCommanderView->GetSelectionInfo().IsTextSelected()){
		bBeginBoxSelect=m_pCommanderView->GetSelectionInfo().IsBoxSelecting();
	}
	else{
		bSelectedArea=FALSE;
		bBeginBoxSelect=false;
	}

	/* 表示処理ON/OFF */
	bool bDisplayUpdate = false;

	m_pCommanderView->SetDrawSwitch(bDisplayUpdate);

	int	nAllLineNum = (Int)GetDocument()->m_cLayoutMgr.GetLineCount();

	/* 進捗表示&中止ダイアログの作成 */
	CDlgCancel	cDlgCancel;
	HWND		hwndCancel = cDlgCancel.DoModeless( G_AppInstance(), m_pCommanderView->GetHwnd(), IDD_REPLACERUNNING );
	::EnableWindow( m_pCommanderView->GetHwnd(), FALSE );
	::EnableWindow( ::GetParent( m_pCommanderView->GetHwnd() ), FALSE );
	::EnableWindow( ::GetParent( ::GetParent( m_pCommanderView->GetHwnd() ) ), FALSE );
	//<< 2002/03/26 Azumaiya
	// 割り算掛け算をせずに進歩状況を表せるように、シフト演算をする。
	int nShiftCount;
	for ( nShiftCount = 0; SHRT_MAX < nAllLineNum; nShiftCount++ )
	{
		nAllLineNum/=2;
	}
	//>> 2002/03/26 Azumaiya

	/* プログレスバー初期化 */
	HWND		hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS_REPLACE );
	Progress_SetRange( hwndProgress, 0, nAllLineNum );
	int			nNewPos = 0;
	Progress_SetPos( hwndProgress, nNewPos);

	/* 置換個数初期化 */
	int			nReplaceNum = 0;
	HWND		hwndStatic = ::GetDlgItem( hwndCancel, IDC_STATIC_KENSUU );
	TCHAR szLabel[64];
	_itot( nReplaceNum, szLabel, 10 );
	::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

	CLayoutRange sRangeA;	//選択範囲
	CLogicPoint ptColLineP;

	// From Here 2001.12.03 hor
	if (bSelectedArea){
		/* 選択範囲置換 */
		/* 選択範囲開始位置の取得 */
		sRangeA = GetSelect();

		//	From Here 2007.09.20 genta 矩形範囲の選択置換ができない
		//	左下～右上と選択した場合，m_nSelectColmTo < m_nSelectColmFrom となるが，
		//	範囲チェックで colFrom < colTo を仮定しているので，
		//	矩形選択の場合は左上～右下指定になるよう桁を入れ換える．
		if( bBeginBoxSelect && sRangeA.GetTo().x < sRangeA.GetFrom().x )
			t_swap(sRangeA.GetFromPointer()->x,sRangeA.GetToPointer()->x);
		//	To Here 2007.09.20 genta 矩形範囲の選択置換ができない

		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			sRangeA.GetTo(),
			&ptColLineP
		);
		//選択範囲開始位置へ移動
		GetCaret().MoveCursor( sRangeA.GetFrom(), bDisplayUpdate );
	}
	else{
		/* ファイル全体置換 */
		/* ファイルの先頭に移動 */
	//	HandleCommand( F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0 );
		Command_GOFILETOP(bDisplayUpdate);
	}

	CLayoutPoint ptLast = GetCaret().GetCaretLayoutPos();

	/* テキスト選択解除 */
	/* 現在の選択範囲を非選択状態に戻す */
	m_pCommanderView->GetSelectionInfo().DisableSelectArea( bDisplayUpdate );
	/* 次を検索 */
	Command_SEARCH_NEXT( true, bDisplayUpdate, 0, 0 );
	// To Here 2001.12.03 hor

	//<< 2002/03/26 Azumaiya
	// 速く動かすことを最優先に組んでみました。
	// ループの外で文字列の長さを特定できるので、一時変数化。
	const wchar_t *szREPLACEKEY;		// 置換後文字列。
	bool		bColmnSelect;	// 矩形貼り付けを行うかどうか。
	bool		bLineSelect = false;	// ラインモード貼り付けを行うかどうか
	CNativeW	cmemClip;		// 置換後文字列のデータ（データを格納するだけで、ループ内ではこの形ではデータを扱いません）。

	// クリップボードからのデータ貼り付けかどうか。
	if( nPaste != 0 )
	{
		// クリップボードからデータを取得。
		if ( !m_pCommanderView->MyGetClipboardData( cmemClip, &bColmnSelect, GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste? &bLineSelect: NULL ) )
		{
			ErrorBeep();
			return;
		}

		// 矩形貼り付けが許可されていて、クリップボードのデータが矩形選択のとき。
		if ( GetDllShareData().m_Common.m_sEdit.m_bAutoColmnPaste && bColmnSelect )
		{
			// マウスによる範囲選択中
			if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )
			{
				ErrorBeep();
				return;
			}

			// 現在のフォントは固定幅フォントである
			if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH )
			{
				return;
			}
		}
		else
		// クリップボードからのデータは普通に扱う。
		{
			bColmnSelect = false;
		}
	}
	else
	{
		// 2004.05.14 Moca 全置換の途中で他のウィンドウで置換されるとまずいのでコピーする
		cmemClip.SetString( GetEditWindow()->m_cDlgReplace.m_strText2.c_str() );
	}

	CLogicInt nREPLACEKEY;			// 置換後文字列の長さ。
	szREPLACEKEY = cmemClip.GetStringPtr(&nREPLACEKEY);

	// 行コピー（MSDEVLineSelect形式）のテキストで末尾が改行になっていなければ改行を追加する
	// ※レイアウト折り返しの行コピーだった場合は末尾が改行になっていない
	if( bLineSelect ){
		if( szREPLACEKEY[nREPLACEKEY - 1] != WCODE::CR && szREPLACEKEY[nREPLACEKEY - 1] != WCODE::LF ){
			cmemClip.AppendString(GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2());
			szREPLACEKEY = cmemClip.GetStringPtr( &nREPLACEKEY );
		}
	}

	if( GetDllShareData().m_Common.m_sEdit.m_bConvertEOLPaste ){
		wchar_t	*pszConvertedText = new wchar_t[nREPLACEKEY * 2]; // 全文字\n→\r\n変換で最大の２倍になる
		CLogicInt nConvertedTextLen = ConvertEol(szREPLACEKEY, nREPLACEKEY, pszConvertedText);
		cmemClip.SetString(pszConvertedText, nConvertedTextLen);
		szREPLACEKEY = cmemClip.GetStringPtr(&nREPLACEKEY);
		delete [] pszConvertedText;
	}

	// 取得にステップがかかりそうな変数などを、一時変数化する。
	// とはいえ、これらの操作をすることによって得をするクロック数は合わせても 1 ループで数十だと思います。
	// 数百クロック毎ループのオーダーから考えてもそんなに得はしないように思いますけど・・・。
	bool bAddCRLFWhenCopy = GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy;
	BOOL &bCANCEL = cDlgCancel.m_bCANCEL;
	CDocLineMgr& rDocLineMgr = GetDocument()->m_cDocLineMgr;
	CLayoutMgr& rLayoutMgr = GetDocument()->m_cLayoutMgr;

	//  クラス関係をループの中で宣言してしまうと、毎ループごとにコンストラクタ、デストラクタが
	// 呼ばれて遅くなるので、ここで宣言。
	CBregexp cRegexp;
	// 初期化も同様に毎ループごとにやると遅いので、最初に済ましてしまう。
	if( bRegularExp )
	{
		if ( !InitRegexp( m_pCommanderView->GetHwnd(), cRegexp, true ) )
		{
			return;
		}

		const CNativeW	cMemRepKey( szREPLACEKEY, wcslen(szREPLACEKEY) );
		// Nov. 9, 2005 かろと 正規表現で選択始点・終点への挿入方法を変更(再)
		CNativeW cMemRepKey2;
		CNativeW cMemMatchStr;
		cMemMatchStr.SetString(L"$&");
		if (nReplaceTarget == 1 ) {	//選択始点へ挿入
			cMemRepKey2 = cMemRepKey;
			cMemRepKey2 += cMemMatchStr;
		} else if (nReplaceTarget == 2) { // 選択終点へ挿入
			cMemRepKey2 = cMemMatchStr;
			cMemRepKey2 += cMemRepKey;
		} else {
			cMemRepKey2 = cMemRepKey;
		}
		// 正規表現オプションの設定2006.04.01 かろと
		int nFlag = (m_pCommanderView->m_sCurSearchOption.bLoHiCase ? CBregexp::optCaseSensitive : CBregexp::optNothing);
		nFlag |= (bConsecutiveAll ? CBregexp::optNothing : CBregexp::optGlobal);	// 2007.01.16 ryoji
		cRegexp.Compile(m_pCommanderView->m_strCurSearchKey.c_str(), cMemRepKey2.GetStringPtr(), nFlag);
	}

	//$$ 単位混在
	CLayoutPoint	ptOld;						//検索後の選択範囲
	/*CLogicInt*/int		lineCnt;					//置換前の行数
	/*CLayoutInt*/int		linDif = (0);		//置換後の行調整
	/*CLayoutInt*/int		colDif = (0);		//置換後の桁調整
	/*CLayoutInt*/int		linPrev = (0);	//前回の検索行(矩形) @@@2001.12.31 YAZAKI warning退治
	/*CLogicInt*/int		linOldLen = (0);	//検査後の行の長さ
	/*CLayoutInt*/int		linNext;					//次回の検索行(矩形)

	/* テキストが選択されているか */
	while( m_pCommanderView->GetSelectionInfo().IsTextSelected() )
	{
		/* キャンセルされたか */
		if( bCANCEL )
		{
			break;
		}

		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( hwndCancel ) )
		{
			return;// -1;
		}

		// 128 ごとに表示。
		if( 0 == (nReplaceNum & 0x7F ) )
		// 時間ごとに進歩状況描画だと時間取得分遅くなると思うが、そちらの方が自然だと思うので・・・。
		// と思ったけど、逆にこちらの方が自然ではないので、やめる。
		{
			nNewPos = (Int)GetSelect().GetFrom().GetY2() >> nShiftCount;
			Progress_SetPos( hwndProgress, nNewPos);
			_itot( nReplaceNum, szLabel, 10 );
			::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );
		}

		// From Here 2001.12.03 hor
		/* 検索後の位置を確認 */
		if( bSelectedArea )
		{
			// 矩形選択
			//	o レイアウト座標をチェックしながら置換する
			//	o 折り返しがあると変になるかも・・・
			//
			if ( bBeginBoxSelect )
			{
				// 検索時の行数を記憶
				lineCnt = (Int)rLayoutMgr.GetLineCount();
				// 検索後の範囲終端
				ptOld = GetSelect().GetTo();
				// 前回の検索行と違う？
				if(ptOld.y!=linPrev){
					colDif=(0);
				}
				linPrev=(Int)ptOld.GetY2();
				// 行は範囲内？
				if ((sRangeA.GetTo().y+linDif == ptOld.y && sRangeA.GetTo().GetX2()+colDif < ptOld.x) ||
					(sRangeA.GetTo().y+linDif <  ptOld.y)) {
					break;
				}
				// 桁は範囲内？
				if(!(sRangeA.GetFrom().x<=GetSelect().GetFrom().x && ptOld.GetX2()<=sRangeA.GetTo().GetX2()+colDif)){
					if(ptOld.x<sRangeA.GetTo().GetX2()+colDif){
						linNext=(Int)GetSelect().GetTo().GetY2();
					}else{
						linNext=(Int)GetSelect().GetTo().GetY2()+1;
					}
					//次の検索開始位置へシフト
					GetCaret().SetCaretLayoutPos(CLayoutPoint(sRangeA.GetFrom().x,CLayoutInt(linNext)));
					// 2004.05.30 Moca 現在の検索文字列を使って検索する
					Command_SEARCH_NEXT( false, bDisplayUpdate, 0, 0 );
					colDif=(0);
					continue;
				}
			}
			// 普通の選択
			//	o 物理座標をチェックしながら置換する
			//
			else {
				// 検索時の行数を記憶
				lineCnt = rDocLineMgr.GetLineCount();

				// 検索後の範囲終端
				CLogicPoint ptOldTmp;
				rLayoutMgr.LayoutToLogic(
					GetSelect().GetTo(),
					&ptOldTmp
				);
				ptOld.x=(CLayoutInt)ptOldTmp.x; //$$ レイアウト型に無理やりロジック型を代入。気持ち悪い
				ptOld.y=(CLayoutInt)ptOldTmp.y;

				// 置換前の行の長さ(改行は１文字と数える)を保存しておいて、置換前後で行位置が変わった場合に使用
				linOldLen = rDocLineMgr.GetLine(ptOldTmp.GetY2())->GetLengthWithoutEOL() + CLogicInt(1);

				// 行は範囲内？
				// 2007.01.19 ryoji 条件追加: 選択終点が行頭(ptColLineP.x == 0)になっている場合は前の行の行末までを選択範囲とみなす
				// （選択始点が行頭ならその行頭は選択範囲に含み、終点が行頭ならその行頭は選択範囲に含まない、とする）
				// 論理的に少し変と指摘されるかもしれないが、実用上はそのようにしたほうが望ましいケースが多いと思われる。
				// ※行選択で行末までを選択範囲にしたつもりでも、UI上は次の行の行頭にカーソルが行く
				// ※終点の行頭を「^」にマッチさせたかったら１文字以上選択してね、ということで．．．
				// $$ 単位混在しまくりだけど、大丈夫？？
				if ((ptColLineP.y+linDif == (Int)ptOld.y && (ptColLineP.x+colDif < (Int)ptOld.x || ptColLineP.x == 0))
					|| ptColLineP.y+linDif < (Int)ptOld.y) {
					break;
				}
			}
		}


		CLayoutPoint ptTmp(0,0);

		if ( nPaste || !bRegularExp ) {
			// 正規表現時は 後方参照($&)で実現するので、正規表現は除外
			if( nReplaceTarget == 1 )	//挿入位置セット
			{
				ptTmp.x = GetSelect().GetTo().x - GetSelect().GetFrom().x;
				ptTmp.y = GetSelect().GetTo().y - GetSelect().GetFrom().y;
				GetSelect().Clear(-1);
			}
			else if( nReplaceTarget == 2 )	//追加位置セット
			{
				// 正規表現を除外したので、「検索後の文字が改行やったら次の行の先頭へ移動」の処理を削除
				GetCaret().MoveCursor(GetSelect().GetTo(), false);
				GetSelect().Clear(-1);
		    }
			else {
				// 位置指定ないので、何もしない
			}
		}

		/* コマンドコードによる処理振り分け */
		/* テキストを貼り付け */
		if( nPaste )
		{
			if ( !bColmnSelect )
			{
				/* 本当は Command_INSTEXT を使うべきなんでしょうが、無駄な処理を避けるために直接たたく。
				** →m_nSelectXXXが-1の時に m_pCommanderView->ReplaceData_CEditViewを直接たたくと動作不良となるため
				**   直接たたくのやめた。2003.05.18 by かろと
				*/
				Command_INSTEXT( FALSE, szREPLACEKEY, nREPLACEKEY, TRUE, bLineSelect );
			}
			else
			{
				Command_PASTEBOX(szREPLACEKEY, nREPLACEKEY);
				// 再描画を行わないとどんな結果が起きているのか分からずみっともないので・・・。
				m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
				m_pCommanderView->Redraw();
			}
			++nReplaceNum;
		}
		// 2002/01/19 novice 正規表現による文字列置換
		else if( bRegularExp ) /* 検索／置換  1==正規表現 */
		{
			// 物理行、物理行長、物理行での検索マッチ位置
			const CLayout* pcLayout = rLayoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
			const wchar_t* pLine = pcLayout->GetDocLineRef()->GetPtr();
			CLogicInt nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetSelect().GetFrom().GetX2() ) + pcLayout->GetLogicOffset();
			CLogicInt nLen = pcLayout->GetDocLineRef()->GetLengthWithEOL();
			CLogicInt colDiff = CLogicInt(0);
			if( !bConsecutiveAll ){	// 一括置換
				// 2007.01.16 ryoji
				// 選択範囲置換の場合は行内の選択範囲末尾まで置換範囲を縮め，
				// その位置を記憶する．
				if( bSelectedArea ){
					if( bBeginBoxSelect ){	// 矩形選択
						CLogicPoint ptWork;
						rLayoutMgr.LayoutToLogic(
							CLayoutPoint(sRangeA.GetTo().x,ptOld.y),
							&ptWork
						);
						ptColLineP.x = ptWork.x;
						if( nLen - pcLayout->GetDocLineRef()->GetEol().GetLen() > ptColLineP.x + colDif )
							nLen = ptColLineP.GetX2() + CLogicInt(colDif);
					} else {	// 通常の選択
						if( ptColLineP.y+linDif == (Int)ptOld.y ){ //$$ 単位混在
							if( nLen - pcLayout->GetDocLineRef()->GetEol().GetLen() > ptColLineP.x + colDif )
								nLen = ptColLineP.GetX2() + CLogicInt(colDif);
						}
					}
				}

				if(pcLayout->GetDocLineRef()->GetLengthWithoutEOL() < nLen)
					ptOld.x = (CLayoutInt)(Int)pcLayout->GetDocLineRef()->GetLengthWithoutEOL() + 1; //$$ 単位混在
				else
					ptOld.x = (CLayoutInt)(Int)nLen; //$$ 単位混在
			}

			if( int nReplace = cRegexp.Replace(pLine, nLen, nIdx) ){
				nReplaceNum += nReplace;
				if ( !bConsecutiveAll ) { // 2006.04.01 かろと	// 2007.01.16 ryoji
					// 行単位での置換処理
					// 選択範囲を物理行末までにのばす
					rLayoutMgr.LogicToLayout( CLogicPoint(nLen, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );
				} else {
				    // From Here Jun. 6, 2005 かろと
				    // 物理行末までINSTEXTする方法は、キャレット位置を調整する必要があり、
				    // キャレット位置の計算が複雑になる。（置換後に改行がある場合に不具合発生）
				    // そこで、INSTEXTする文字列長を調整する方法に変更する（実はこっちの方がわかりやすい）
				    CLogicInt matchLen = cRegexp.GetMatchLen();
				    CLogicInt nIdxTo = nIdx + matchLen;		// 検索文字列の末尾
				    if (matchLen == 0) {
					    // ０文字マッチの時(無限置換にならないように１文字進める)
					    if (nIdxTo < nLen) {
						    // 2005-09-02 D.S.Koba GetSizeOfChar
						    nIdxTo += CLogicInt(CNativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					    }
					    // 無限置換しないように、１文字増やしたので１文字選択に変更
					    // 選択始点・終点への挿入の場合も０文字マッチ時は動作は同じになるので
						rLayoutMgr.LogicToLayout( CLogicPoint(nIdxTo, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji 行位置も取得する
				    }
				    // 行末から検索文字列末尾までの文字数
					colDiff =  nLen - nIdxTo;
					ptOld.x = (CLayoutInt)(Int)nIdxTo;	// 2007.01.19 ryoji 追加  // $$ 単位混在
				    //	Oct. 22, 2005 Karoto
				    //	\rを置換するとその後ろの\nが消えてしまう問題の対応
				    if (colDiff < pcLayout->GetDocLineRef()->GetEol().GetLen()) {
					    // 改行にかかっていたら、行全体をINSTEXTする。
					    colDiff = CLogicInt(0);
						rLayoutMgr.LogicToLayout( CLogicPoint(nLen, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji 追加
						ptOld.x = (CLayoutInt)(Int)pcLayout->GetDocLineRef()->GetLengthWithoutEOL() + 1;	// 2007.01.19 ryoji 追加 //$$ 単位混在
				    }
				}
				// 置換後文字列への書き換え(行末から検索文字列末尾までの文字を除く)
				Command_INSTEXT( FALSE, cRegexp.GetString(), cRegexp.GetStringLen() - colDiff, TRUE );
				// To Here Jun. 6, 2005 かろと
			}
		}
		else
		{
			/* 本当は元コードを使うべきなんでしょうが、無駄な処理を避けるために直接たたく。
			** →m_nSelectXXXが-1の時に m_pCommanderView->ReplaceData_CEditViewを直接たたくと動作不良となるため直接たたくのやめた。2003.05.18 かろと
			*/
			Command_INSTEXT( FALSE, szREPLACEKEY, nREPLACEKEY, TRUE );
			++nReplaceNum;
		}

		// 挿入後の位置調整
		if( nReplaceTarget == 1 )
		{
			GetCaret().SetCaretLayoutPos(GetCaret().GetCaretLayoutPos()+ptTmp);
			if (!bBeginBoxSelect)
			{
				CLogicPoint p;
				rLayoutMgr.LayoutToLogic(
					GetCaret().GetCaretLayoutPos(),
					&p
				);
				GetCaret().SetCaretLogicPos(p);
			}
		}

		// 最後に置換した位置を記憶
		ptLast = GetCaret().GetCaretLayoutPos();

		/* 置換後の位置を確認 */
		if( bSelectedArea )
		{
			// 検索→置換の行補正値取得
			if( bBeginBoxSelect )
			{
				colDif += (Int)(ptLast.GetX2() - ptOld.GetX2());
				linDif += (Int)(rLayoutMgr.GetLineCount() - lineCnt);
			}
			else{
				// 置換前の検索文字列の最終位置は ptOld
				// 置換後のカーソル位置
				CLogicPoint ptTmp2 = GetCaret().GetCaretLogicPos();
				int linDif_thistime = rDocLineMgr.GetLineCount() - lineCnt;	// 今回置換での行数変化
				linDif += linDif_thistime;
				if( ptColLineP.y + linDif == ptTmp2.y)
				{
					// 最終行で置換した時、又は、置換の結果、選択エリア最終行まで到達した時
					// 最終行なので、置換前後の文字数の増減で桁位置を調整する
					colDif += (Int)ptTmp2.GetX2() - (Int)ptOld.GetX2(); //$$ 単位混在

					// 但し、以下の場合は置換前後で行が異なってしまうので、行の長さで補正する必要がある
					// １）最終行直前で行連結が起こり、行が減っている場合（行連結なので、桁位置は置換後のカーソル桁位置分増加する）
					// 　　ptTmp2.x-ptOld.xだと、\r\n → "" 置換で行連結した場合に、桁位置が負になり失敗する（負とは前行の後ろの方になることなので補正する）
					// 　　今回置換での行数の変化(linDif_thistime)で、最終行が行連結されたかどうかを見ることにする
					// ２）改行を置換した（ptTmp2.y!=ptOld.y）場合、改行を置換すると置換後の桁位置が次行の桁位置になっているため
					//     ptTmp2.x-ptOld.xだと、負の数となり、\r\n → \n や \n → "abc" などで桁位置がずれる
					//     これも前行の長さで調整する必要がある
					if (linDif_thistime < 0 || ptTmp2.y != (Int)ptOld.y) { //$$ 単位混在
						colDif += linOldLen;
					}
				}
			}
		}
		// To Here 2001.12.03 hor

		/* 次を検索 */
		// 2004.05.30 Moca 現在の検索文字列を使って検索する
		Command_SEARCH_NEXT( false, bDisplayUpdate, 0, 0 );
	}

	if( 0 < nAllLineNum )
	{
		nNewPos = (Int)GetSelect().GetFrom().GetY2() >> nShiftCount;
		Progress_SetPos( hwndProgress, nNewPos);
	}
	//>> 2002/03/26 Azumaiya

	_itot( nReplaceNum, szLabel, 10 );
	::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

	if( !cDlgCancel.IsCanceled() ){
		nNewPos = nAllLineNum;
		Progress_SetPos( hwndProgress, nNewPos);
	}
	cDlgCancel.CloseDialog( 0 );
	::EnableWindow( m_pCommanderView->GetHwnd(), TRUE );
	::EnableWindow( ::GetParent( m_pCommanderView->GetHwnd() ), TRUE );
	::EnableWindow( ::GetParent( ::GetParent( m_pCommanderView->GetHwnd() ) ), TRUE );

	// From Here 2001.12.03 hor

	/* テキスト選択解除 */
	m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );

	/* カーソル・選択範囲復元 */
	if((!bSelectedArea) ||			// ファイル全体置換
	   (cDlgCancel.IsCanceled())) {		// キャンセルされた
		// 最後に置換した文字列の右へ
		GetCaret().MoveCursor( ptLast, TRUE );
	}
	else{
		if (bBeginBoxSelect) {
			// 矩形選択
			m_pCommanderView->GetSelectionInfo().SetBoxSelect(bBeginBoxSelect);
			sRangeA.GetToPointer()->y += linDif;
			if(sRangeA.GetTo().y<0)sRangeA.SetToY(CLayoutInt(0));
		}
		else{
			// 普通の選択
			ptColLineP.x+=colDif;
			if(ptColLineP.x<0)ptColLineP.x=0;
			ptColLineP.y+=linDif;
			if(ptColLineP.y<0)ptColLineP.y=0;
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				ptColLineP,
				sRangeA.GetToPointer()
			);
		}
		if(sRangeA.GetFrom().y<sRangeA.GetTo().y || sRangeA.GetFrom().x<sRangeA.GetTo().x){
			m_pCommanderView->GetSelectionInfo().SetSelectArea( sRangeA );	// 2009.07.25 ryoji
		}
		GetCaret().MoveCursor( sRangeA.GetTo(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();	// 2009.07.25 ryoji
	}
	// To Here 2001.12.03 hor

	GetEditWindow()->m_cDlgReplace.m_bCanceled = (cDlgCancel.IsCanceled() != FALSE);
	GetEditWindow()->m_cDlgReplace.m_nReplaceCnt=nReplaceNum;
	m_pCommanderView->SetDrawSwitch(true);
	ActivateFrameWindow( GetMainWindow() );
}



//検索マークの切替え	// 2001.12.03 hor クリア を 切替え に変更
void CViewCommander::Command_SEARCH_CLEARMARK( void )
{
// From Here 2001.12.03 hor

	//検索マークのセット

	if(m_pCommanderView->GetSelectionInfo().IsTextSelected()){

		// 検索文字列取得
		CNativeW	cmemCurText;
		m_pCommanderView->GetCurrentTextForSearch( cmemCurText, false );

		m_pCommanderView->m_strCurSearchKey = cmemCurText.GetStringPtr();
		if( m_pCommanderView->m_nCurSearchKeySequence < GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence ){
			m_pCommanderView->m_sCurSearchOption = GetDllShareData().m_Common.m_sSearch.m_sSearchOption;
		}
		m_pCommanderView->m_sCurSearchOption.bRegularExp = false;		//正規表現使わない
		m_pCommanderView->m_sCurSearchOption.bWordOnly = false;		//単語で検索しない

		// 共有データへ登録
		if( cmemCurText.GetStringLength() < _MAX_PATH ){
			CSearchKeywordManager().AddToSearchKeyArr( cmemCurText.GetStringPtr() );
			GetDllShareData().m_Common.m_sSearch.m_sSearchOption = m_pCommanderView->m_sCurSearchOption;
		}
		m_pCommanderView->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;
		m_pCommanderView->m_bCurSearchUpdate = true;

		m_pCommanderView->ChangeCurRegexp(false); // 2002.11.11 Moca 正規表現で検索した後，色分けができていなかった

		// 再描画
		m_pCommanderView->RedrawAll();
		return;
	}
// To Here 2001.12.03 hor

	//検索マークのクリア

	m_pCommanderView->m_bCurSrchKeyMark = false;	/* 検索文字列のマーク */
	/* フォーカス移動時の再描画 */
	m_pCommanderView->RedrawAll();
	return;
}



//	Jun. 16, 2000 genta
//	対括弧の検索
void CViewCommander::Command_BRACKETPAIR( void )
{
	CLayoutPoint ptColLine;
	//int nLine, nCol;

	int mode = 3;
	/*
	bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
	bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
	bit2(out) : 見つかった位置         0:後ろ      1:前
	*/
	if( m_pCommanderView->SearchBracket( GetCaret().GetCaretLayoutPos(), &ptColLine, &mode ) ){	// 02/09/18 ai
		//	2005.06.24 Moca
		//	2006.07.09 genta 表示更新漏れ：新規関数にて対応
		m_pCommanderView->MoveCursorSelecting( ptColLine, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
	else{
		//	失敗した場合は nCol/nLineには有効な値が入っていない.
		//	何もしない
	}
}
