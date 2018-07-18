/*!	@file
	@brief マウスイベントの処理

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date   2008/04/13 CEditView.cppから分離
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, かろと, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta
	Copyright (C) 2007, ryoji, じゅうじ, maru

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <process.h> // _beginthreadex
#include <limits.h>
#include "CEditView.h"
#include "_main/CAppMode.h"
#include "CEditApp.h"
#include "CGrepAgent.h" // use CEditApp.h
#include "window/CEditWnd.h"
#include "_os/CDropTarget.h" // CDataObject
#include "_os/CClipboard.h"
#include "COpeBlk.h"
#include "doc/layout/CLayout.h"
#include "cmd/CViewCommander_inline.h"
#include "uiparts/CWaitCursor.h"
#include "uiparts/HandCursor.h"
#include "util/input.h"
#include "util/os.h"
#include "sakura_rc.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      マウスイベント                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* マウス左ボタン押下 */
void CEditView::OnLBUTTONDOWN( WPARAM fwKeys, int _xPos , int _yPos )
{
	CMyPoint ptMouse(_xPos,_yPos);

	if( m_bHokan ){
		m_pcEditWnd->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

	//isearch 2004.10.22 isearchをキャンセルする
	if (m_nISearchMode > SEARCH_NONE ){
		ISearchExit();
	}
	if( m_nAutoScrollMode ){
		AutoScrollExit();
	}
	if( m_bMiniMap ){
		::SetFocus( GetHwnd() );
		::SetCapture( GetHwnd() );
		m_bMiniMapMouseDown = true;
		OnMOUSEMOVE( fwKeys, _xPos, _yPos );
		return;
	}

	CNativeW	cmemCurText;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;

	CLayoutRange sRange;

	CLogicInt	nIdx;
	int			nWork;
	BOOL		tripleClickMode = FALSE;	// 2007.10.02 nasukoji	トリプルクリックであることを示す
	int			nFuncID = 0;				// 2007.12.02 nasukoji	マウス左クリックに対応する機能コード

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() == 0 ){
		return;
	}
	if( !GetCaret().ExistCaretFocus() ){ //フォーカスがないとき
		return;
	}

	/* 辞書Tipが起動されている */
	if( 0 == m_dwTipTimer ){
		/* 辞書Tipを消す */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
	}
	else{
		m_dwTipTimer = ::GetTickCount();		/* 辞書Tip起動タイマー */
	}

	// 2007.12.02 nasukoji	トリプルクリックをチェック
	tripleClickMode = CheckTripleClick(ptMouse);

	if(tripleClickMode){
		// マウス左トリプルクリックに対応する機能コードはm_Common.m_pKeyNameArr[5]に入っている
		nFuncID = GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_TRIPLECLICK].m_nFuncCodeArr[getCtrlKeyState()];
		if( 0 == nFuncID ){
			tripleClickMode = 0;	// 割り当て機能無しの時はトリプルクリック OFF
		}
	}else{
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}

	/* 現在のマウスカーソル位置→レイアウト位置 */
	CLayoutPoint ptNew;
	GetTextArea().ClientToLayout(ptMouse, &ptNew);

	// 2010.07.15 Moca マウスダウン時の座標を覚えて利用する
	m_cMouseDownPos = ptMouse;

	// OLEによるドラッグ & ドロップを使う
	// 2007.12.02 nasukoji	トリプルクリック時はドラッグを開始しない
	if( !tripleClickMode && GetDllShareData().m_Common.m_sEdit.m_bUseOLE_DragDrop ){
		if( GetDllShareData().m_Common.m_sEdit.m_bUseOLE_DropSource ){		/* OLEによるドラッグ元にするか */
			/* 行選択エリアをドラッグした */
			if( ptMouse.x < GetTextArea().GetAreaLeft() - GetTextMetrics().GetHankakuDx() ){
				goto normal_action;
			}
			/* 指定カーソル位置が選択エリア内にあるか */
			if( 0 == IsCurrentPositionSelected(ptNew) ){
				POINT ptWk = {ptMouse.x, ptMouse.y};
				::ClientToScreen(GetHwnd(), &ptWk);
				if( !::DragDetect(GetHwnd(), ptWk) ){
					// ドラッグ開始条件を満たさなかったのでクリック位置にカーソル移動する
					if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
						/* 現在の選択範囲を非選択状態に戻す */
						GetSelectionInfo().DisableSelectArea( true );
					}
//@@@ 2002.01.08 YAZAKI フリーカーソルOFFで複数行選択し、行の後ろをクリックするとそこにキャレットが置かれてしまうバグ修正
					/* カーソル移動。 */
					if( ptMouse.y >= GetTextArea().GetAreaTop() && ptMouse.y < GetTextArea().GetAreaBottom() ){
						if( ptMouse.x >= GetTextArea().GetAreaLeft() && ptMouse.x < GetTextArea().GetAreaRight() ){
							GetCaret().MoveCursorToClientPoint( ptMouse );
						}
						else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
							GetCaret().MoveCursorToClientPoint( CMyPoint(GetTextArea().GetDocumentLeftClientPointX(), ptMouse.y) );
						}
					}
					return;
				}
				/* 選択範囲のデータを取得 */
				if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
					DWORD dwEffects;
					DWORD dwEffectsSrc = ( !m_pcEditDoc->IsEditable() )?
											DROPEFFECT_COPY: DROPEFFECT_COPY | DROPEFFECT_MOVE;
					int nOpe = m_pcEditDoc->m_cDocEditor.m_cOpeBuf.GetCurrentPointer();
					m_pcEditWnd->SetDragSourceView( this );
					CDataObject data( cmemCurText.GetStringPtr(), cmemCurText.GetStringLength(), GetSelectionInfo().IsBoxSelecting() );
					dwEffects = data.DragDrop( TRUE, dwEffectsSrc );
					m_pcEditWnd->SetDragSourceView( NULL );
					if( m_pcEditDoc->m_cDocEditor.m_cOpeBuf.GetCurrentPointer() == nOpe ){	// ドキュメント変更なしか？	// 2007.12.09 ryoji
						m_pcEditWnd->SetActivePane( m_nMyIndex );
						if( DROPEFFECT_MOVE == (dwEffectsSrc & dwEffects) ){
							// 移動範囲を削除する
							// ドロップ先が移動を処理したが自ドキュメントにここまで変更が無い
							// →ドロップ先は外部のウィンドウである
							if( NULL == m_cCommander.GetOpeBlk() ){
								m_cCommander.SetOpeBlk(new COpeBlk);
							}
							m_cCommander.GetOpeBlk()->AddRef();

							// 選択範囲を削除
							DeleteData( true );

							// アンドゥバッファの処理
							SetUndoBuffer();
						}
					}
				}
				return;
			}
		}
	}

normal_action:;

	// ALTキーが押されている、かつトリプルクリックでない		// 2007.11.15 nasukoji	トリプルクリック対応
	if( GetKeyState_Alt() &&( ! tripleClickMode)){
		if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			GetSelectionInfo().DisableSelectArea( true );
		}
		if( ptMouse.y >= GetTextArea().GetAreaTop()  && ptMouse.y < GetTextArea().GetAreaBottom() ){
			if( ptMouse.x >= GetTextArea().GetAreaLeft() && ptMouse.x < GetTextArea().GetAreaRight() ){
				GetCaret().MoveCursorToClientPoint( ptMouse );
			}
			else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
				GetCaret().MoveCursorToClientPoint( CMyPoint(GetTextArea().GetDocumentLeftClientPointX(), ptMouse.y) );
			}else{
				return;
			}
		}
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)

		/* 範囲選択開始 & マウスキャプチャー */
		GetSelectionInfo().SelectBeginBox();

		::SetCapture( GetHwnd() );
		GetCaret().HideCaret_( GetHwnd() ); // 2002/07/22 novice
		/* 現在のカーソル位置から選択を開始する */
		GetSelectionInfo().BeginSelectArea( );
		GetCaret().m_cUnderLine.CaretUnderLineOFF( true );
		GetCaret().m_cUnderLine.UnderLineLock();
		if( ptMouse.x < GetTextArea().GetAreaLeft() ){
			/* カーソル下移動 */
			GetCommander().Command_DOWN( true, false );
		}
	}
	else{
		/* カーソル移動 */
		if( ptMouse.y >= GetTextArea().GetAreaTop() && ptMouse.y < GetTextArea().GetAreaBottom() ){
			if( ptMouse.x >= GetTextArea().GetAreaLeft() && ptMouse.x < GetTextArea().GetAreaRight() ){
			}
			else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
			}
			else{
				return;
			}
		}
		else if( ptMouse.y < GetTextArea().GetAreaTop() ){
			//	ルーラクリック
			return;
		}
		else {
			return;
		}

		/* マウスのキャプチャなど */
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)
		
		/* 範囲選択開始 & マウスキャプチャー */
		GetSelectionInfo().SelectBeginNazo();
		::SetCapture( GetHwnd() );
		GetCaret().HideCaret_( GetHwnd() ); // 2002/07/22 novice


		CLayoutPoint ptNewCaret = GetCaret().GetCaretLayoutPos();
		bool bSetPtNewCaret = false;
		if(tripleClickMode){		// 2007.11.15 nasukoji	トリプルクリックを処理する
			// 1行選択でない場合は選択文字列を解除
			// トリプルクリックが1行選択でなくてもクアドラプルクリックを有効とする
			if(F_SELECTLINE != nFuncID){
				OnLBUTTONUP( fwKeys, ptMouse.x, ptMouse.y );	// ここで左ボタンアップしたことにする

				if( GetSelectionInfo().IsTextSelected() )		// テキストが選択されているか
					GetSelectionInfo().DisableSelectArea( true );	// 現在の選択範囲を非選択状態に戻す
			}

			// 単語の途中で折り返されていると下の行が選択されてしまうことへの対処
			if(F_SELECTLINE != nFuncID){
				GetCaret().MoveCursorToClientPoint( ptMouse );	// カーソル移動
			}else{
				GetCaret().MoveCursorToClientPoint( ptMouse, true, &ptNewCaret );	// カーソル移動
				bSetPtNewCaret = true;
			}

			// コマンドコードによる処理振り分け
			// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
			::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ), (LPARAM)NULL );

			// 1行選択でない場合はここで抜ける（他の選択コマンドの時問題となるかも）
			if(F_SELECTLINE != nFuncID)
				return;
			ptNewCaret = GetCaret().GetCaretLayoutPos();

			// 選択するものが無い（[EOF]のみの行）時は通常クリックと同じ処理
			if(( ! GetSelectionInfo().IsTextSelected() )&&
			   ( GetCaret().GetCaretLogicPos().y >= m_pcEditDoc->m_cDocLineMgr.GetLineCount() ))
			{
				GetSelectionInfo().BeginSelectArea();				// 現在のカーソル位置から選択を開始する
				GetSelectionInfo().m_bBeginLineSelect = false;		// 行単位選択中 OFF
			}
		}else
		/* 選択開始処理 */
		/* SHIFTキーが押されていたか */
		if(GetKeyState_Shift()){
			if( GetSelectionInfo().IsTextSelected() ){		/* テキストが選択されているか */
				if( GetSelectionInfo().IsBoxSelecting() ){	/* 矩形範囲選択中 */
					/* 現在の選択範囲を非選択状態に戻す */
					GetSelectionInfo().DisableSelectArea( true );

					/* 現在のカーソル位置から選択を開始する */
					GetSelectionInfo().BeginSelectArea( );
				}
				else{
				}
			}
			else{
				/* 現在のカーソル位置から選択を開始する */
				GetSelectionInfo().BeginSelectArea( );
			}

			/* カーソル移動 */
			if( ptMouse.y >= GetTextArea().GetAreaTop() && ptMouse.y < GetTextArea().GetAreaBottom() ){
				if( ptMouse.x >= GetTextArea().GetAreaLeft() && ptMouse.x < GetTextArea().GetAreaRight() ){
					GetCaret().MoveCursorToClientPoint( ptMouse, true, &ptNewCaret );
				}
				else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
					GetCaret().MoveCursorToClientPoint( CMyPoint(GetTextArea().GetDocumentLeftClientPointX(), ptMouse.y), true, &ptNewCaret );
				}
				bSetPtNewCaret = true;
			}
		}
		else{
			if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在の選択範囲を非選択状態に戻す */
				GetSelectionInfo().DisableSelectArea( true );
			}
			/* カーソル移動 */
			if( ptMouse.y >= GetTextArea().GetAreaTop() && ptMouse.y < GetTextArea().GetAreaBottom() ){
				if( ptMouse.x >= GetTextArea().GetAreaLeft() && ptMouse.x < GetTextArea().GetAreaRight() ){
					GetCaret().MoveCursorToClientPoint( ptMouse, true, &ptNewCaret );
				}
				else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
					GetCaret().MoveCursorToClientPoint( CMyPoint(GetTextArea().GetDocumentLeftClientPointX(), ptMouse.y), true, &ptNewCaret );
				}
				bSetPtNewCaret = true;
			}
			/* 現在のカーソル位置から選択を開始する */
			GetSelectionInfo().BeginSelectArea( &ptNewCaret );
		}


		/******* この時点で必ず true == GetSelectionInfo().IsTextSelected() の状態になる ****:*/
		if( !GetSelectionInfo().IsTextSelected() ){
			WarningMessage( GetHwnd(), LS(STR_VIEW_MOUSE_BUG) );
			return;
		}

		int	nWorkRel;
		nWorkRel = IsCurrentPositionSelected(
			ptNewCaret	// カーソル位置
		);


		/* 現在のカーソル位置によって選択範囲を変更 */
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptNewCaret );

		bool bSelectWord = false;
		// CTRLキーが押されている、かつトリプルクリックでない		// 2007.11.15 nasukoji	トリプルクリック対応
		if( GetKeyState_Control() &&( ! tripleClickMode)){
			GetSelectionInfo().m_bBeginWordSelect = true;		/* 単語単位選択中 */
			if( !GetSelectionInfo().IsTextSelected() ){
				/* 現在位置の単語選択 */
				if ( GetCommander().Command_SELECTWORD( &ptNewCaret ) ){
					bSelectWord = true;
					GetSelectionInfo().m_sSelectBgn = GetSelectionInfo().m_sSelect;
				}
			}else{

				/* 選択領域描画 */
				GetSelectionInfo().DrawSelectArea();


				/* 指定された桁に対応する行のデータ内の位置を調べる */
				const CLayout* pcLayout;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr(
					GetSelectionInfo().m_sSelect.GetFrom().GetY2(),
					&nLineLen,
					&pcLayout
				);
				if( NULL != pLine ){
					nIdx = LineColumnToIndex( pcLayout, GetSelectionInfo().m_sSelect.GetFrom().GetX2() );
					/* 現在位置の単語の範囲を調べる */
					bool bWhareResult = m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						GetSelectionInfo().m_sSelect.GetFrom().GetY2(),
						nIdx,
						&sRange,
						NULL,
						NULL
					);
					if( bWhareResult ){
						// 指定された行のデータ内の位置に対応する桁の位置を調べる。
						// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
						/*
						pLine            = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetFrom().GetY2(), &nLineLen, &pcLayout );
						sRange.SetFromX( LineIndexToColumn( pcLayout, sRange.GetFrom().x ) );
						pLine            = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetTo().GetY2(), &nLineLen, &pcLayout );
						sRange.SetToX( LineIndexToColumn( pcLayout, sRange.GetTo().x ) );
						*/

						nWork = IsCurrentPositionSelected(
							sRange.GetFrom()	// カーソル位置
						);
						if( -1 == nWork || 0 == nWork ){
							GetSelectionInfo().m_sSelect.SetFrom(sRange.GetFrom());
							if( 1 == nWorkRel ){
								GetSelectionInfo().m_sSelectBgn = sRange;
							}
						}
					}
				}
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetSelectionInfo().m_sSelect.GetTo().GetY2(), &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColumnToIndex( pcLayout, GetSelectionInfo().m_sSelect.GetTo().GetX2() );
					/* 現在位置の単語の範囲を調べる */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						GetSelectionInfo().m_sSelect.GetTo().GetY2(), nIdx, &sRange, NULL, NULL )
					){
						// 指定された行のデータ内の位置に対応する桁の位置を調べる
						// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
						/*
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetFrom().GetY2(), &nLineLen, &pcLayout );
						sRange.SetFromX( LineIndexToColumn( pcLayout, sRange.GetFrom().x ) );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetTo().GetY2(), &nLineLen, &pcLayout );
						sRange.SetToX( LineIndexToColumn( pcLayout, sRange.GetTo().x ) );
						*/

						nWork = IsCurrentPositionSelected(sRange.GetFrom());
						if( -1 == nWork || 0 == nWork ){
							GetSelectionInfo().m_sSelect.SetTo(sRange.GetFrom());
						}
						if( 1 == IsCurrentPositionSelected(sRange.GetTo()) ){
							GetSelectionInfo().m_sSelect.SetTo(sRange.GetTo());
						}
						if( -1 == nWorkRel || 0 == nWorkRel ){
							GetSelectionInfo().m_sSelectBgn=sRange;
						}
					}
				}

				if( 0 < nWorkRel ){

				}
				/* 選択領域描画 */
				GetSelectionInfo().DrawSelectArea();
			}
		}
		// 行番号エリアをクリックした
		// 2008.05.22 nasukoji	シフトキーを押している場合は行頭クリックとして扱う
		if( ptMouse.x < GetTextArea().GetAreaLeft() && !GetKeyState_Shift() ){
			/* 現在のカーソル位置から選択を開始する */
			GetSelectionInfo().m_bBeginLineSelect = true;

			// 2009.02.22 ryoji 
			// Command_GOLINEEND()/Command_RIGHT()ではなく次のレイアウトを調べて移動選択する方法に変更
			// ※Command_GOLINEEND()/Command_RIGHT()は[折り返し末尾文字の右へ移動]＋[次行の先頭文字の右に移動]の仕様だとＮＧ
			const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( ptNewCaret.GetY2() );
			if( pcLayout ){
				CLayoutPoint ptCaret;
				const CLayout* pNext = pcLayout->GetNextLayout();
				if( pNext ){
					ptCaret.x = pNext->GetIndent();
				}else{
					ptCaret.x = CLayoutInt(0);
				}
				ptCaret.y = ptNewCaret.GetY2() + 1;	// 改行無しEOF行でも MoveCursor() が有効な座標に調整してくれる
				GetCaret().GetAdjustCursorPos( &ptCaret );
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptCaret );
				GetCaret().MoveCursor( ptCaret, true );
				GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			}else{
				/* 現在のカーソル位置によって選択範囲を変更 */
				if( bSetPtNewCaret ){
					GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptNewCaret );
					GetCaret().MoveCursor( ptNewCaret, true, 1000 );
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
			}

			//	Apr. 14, 2003 genta
			//	行番号の下をクリックしてドラッグを開始するとおかしくなるのを修正
			//	行番号をクリックした場合にはGetSelectionInfo().ChangeSelectAreaByCurrentCursor()にて
			//	GetSelectionInfo().m_sSelect.GetTo().x/GetSelectionInfo().m_sSelect.GetTo().yに-1が設定されるが、上の
			//	GetCommander().Command_GOLINEEND(), Command_RIGHT()によって行選択が行われる。
			//	しかしキャレットが末尾にある場合にはキャレットが移動しないので
			//	GetSelectionInfo().m_sSelect.GetTo().x/GetSelectionInfo().m_sSelect.GetTo().yが-1のまま残ってしまい、それが
			//	原点に設定されるためにおかしくなっていた。
			//	なので、範囲選択が行われていない場合は起点末尾の設定を行わないようにする
			if( GetSelectionInfo().IsTextSelected() ){
				GetSelectionInfo().m_sSelectBgn.SetTo( GetSelectionInfo().m_sSelect.GetTo() );
			}
		}
		else{
			/* URLがクリックされたら選択するか */
			if( FALSE != GetDllShareData().m_Common.m_sEdit.m_bSelectClickedURL ){

				CLogicRange cUrlRange;	//URL範囲
				// カーソル位置にURLが有る場合のその範囲を調べる
				bool bIsUrl = IsCurrentPositionURL(
					ptNewCaret,	// カーソル位置
					&cUrlRange,						// URL範囲
					NULL							// URL受け取り先
				);
				if( bIsUrl ){
					/* 現在の選択範囲を非選択状態に戻す */
					GetSelectionInfo().DisableSelectArea( true );

					/*
					  カーソル位置変換
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					  →レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						2002/04/08 YAZAKI 少しでもわかりやすく。
					*/
					CLayoutRange sRangeB;
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( cUrlRange, &sRangeB );
					/*
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( CLogicPoint(nUrlIdxBgn          , nUrlLine), sRangeB.GetFromPointer() );
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( CLogicPoint(nUrlIdxBgn + nUrlLen, nUrlLine), sRangeB.GetToPointer() );
					*/

					GetSelectionInfo().m_sSelectBgn = sRangeB;
					GetSelectionInfo().m_sSelect = sRangeB;

					/* 選択領域描画 */
					GetSelectionInfo().DrawSelectArea();
				}
			}
			if( bSetPtNewCaret && !bSelectWord ){
				/* 現在のカーソル位置によって選択範囲を変更 */
				GetCaret().MoveCursor( ptNewCaret, true, 1000 );
				GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			}
		}
	}
}


/*!	トリプルクリックのチェック
	@brief トリプルクリックを判定する
	
	2回目のクリックから3回目のクリックまでの時間がダブルクリック時間以内で、
	かつその時のクリック位置のずれがシステムメトリック（X:SM_CXDOUBLECLK,
	Y:SM_CYDOUBLECLK）の値（ピクセル）以下の時トリプルクリックとする。
	
	@param[in] xPos		マウスクリックX座標
	@param[in] yPos		マウスクリックY座標
	@return		トリプルクリックの時はTRUEを返す
	トリプルクリックでない時はFALSEを返す

	@note	m_dwTripleClickCheckが0でない時にチェックモードと判定するが、PCを
			連続稼動している場合49.7日毎にカウンタが0になる為、わずかな可能性
			であるがトリプルクリックが判定できない時がある。
			行番号表示エリアのトリプルクリックは通常クリックとして扱う。
	
	@date 2007.11.15 nasukoji	新規作成
*/
BOOL CEditView::CheckTripleClick( CMyPoint ptMouse )
{

	// トリプルクリックチェック有効でない（時刻がセットされていない）
	if(! m_dwTripleClickCheck)
		return FALSE;

	BOOL result = FALSE;

	// 前回クリックとのクリック位置のずれを算出
	CMyPoint dpos( GetSelectionInfo().m_ptMouseRollPosOld.x - ptMouse.x,
				   GetSelectionInfo().m_ptMouseRollPosOld.y - ptMouse.y );

	if(dpos.x < 0)
		dpos.x = -dpos.x;	// 絶対値化

	if(dpos.y < 0)
		dpos.y = -dpos.y;	// 絶対値化

	// 行番号表示エリアでない、かつクリックプレスからダブルクリック時間以内、
	// かつダブルクリックの許容ずれピクセル以下のずれの時トリプルクリックとする
	//	2007.10.12 genta/dskoba システムのダブルクリック速度，ずれ許容量を取得
	if( (ptMouse.x >= GetTextArea().GetAreaLeft())&&
		(::GetTickCount() - m_dwTripleClickCheck <= GetDoubleClickTime() )&&
		(dpos.x <= GetSystemMetrics(SM_CXDOUBLECLK) ) &&
		(dpos.y <= GetSystemMetrics(SM_CYDOUBLECLK)))
	{
		result = TRUE;
	}else{
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}
	
	return result;
}

/* マウス右ボタン押下 */
void CEditView::OnRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	if( m_nAutoScrollMode ){
		AutoScrollExit();
	}
	if( m_bMiniMap ){
		return;
	}
	/* 現在のマウスカーソル位置→レイアウト位置 */

	CLayoutPoint ptNew;
	GetTextArea().ClientToLayout(CMyPoint(xPos,yPos), &ptNew);
	/*
	ptNew.x = GetTextArea().GetViewLeftCol() + (xPos - GetTextArea().GetAreaLeft()) / GetTextMetrics().GetHankakuDx();
	ptNew.y = GetTextArea().GetViewTopLine() + (yPos - GetTextArea().GetAreaTop()) / GetTextMetrics().GetHankakuDy();
	*/
	/* 指定カーソル位置が選択エリア内にあるか */
	if( 0 == IsCurrentPositionSelected(
		ptNew		// カーソル位置
		)
	){
		return;
	}
	OnLBUTTONDOWN( fwKeys, xPos , yPos );
	return;
}

/* マウス右ボタン開放 */
void CEditView::OnRBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	if( GetSelectionInfo().IsMouseSelecting() ){	/* 範囲選択中 */
		/* マウス左ボタン開放のメッセージ処理 */
		OnLBUTTONUP( fwKeys, xPos, yPos );
	}


	int		nIdx;
	int		nFuncID;
// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス右クリックに対応する機能コードはm_Common.m_pKeyNameArr[1]に入っている */
	nFuncID = GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_RIGHT].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
//	/* 右クリックメニュー */
//	GetCommander().Command_MENU_RBUTTON();
	return;
}


// novice 2004/10/11 マウス中ボタン対応
/*!
	@brief マウス中ボタンを押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.11 novice 新規作成
	@date 2008.10.06 nasukoji	マウス中ボタン押下中のホイール操作対応
	@date 2009.01.17 nasukoji	ボタンUPでコマンドを起動するように変更
*/
void CEditView::OnMBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	int nIdx = getCtrlKeyState();
	if( F_AUTOSCROLL == GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_CENTER].m_nFuncCodeArr[nIdx] ){
		if( m_nAutoScrollMode ){
			AutoScrollExit();
			return;
		}else{
			m_nAutoScrollMode = 1;
			m_cAutoScrollMousePos = CMyPoint(xPos, yPos);
			::SetCapture( GetHwnd() );
		}
	}
}


/*!
	@brief マウス中ボタンを開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	
	@date 2009.01.17 nasukoji	新規作成（ボタンUPでコマンドを起動するように変更）
*/
void CEditView::OnMBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if( GetDllShareData().m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_CENTER &&
	    m_pcEditWnd->IsPageScrollByWheel() )
	{
		m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// ホイール操作によるページスクロールあり
	if( GetDllShareData().m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_CENTER &&
	    m_pcEditWnd->IsHScrollByWheel() )
	{
		m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス左サイドボタンに対応する機能コードはm_Common.m_pKeyNameArr[2]に入っている */
	nFuncID = GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_CENTER].m_nFuncCodeArr[nIdx];
	if( nFuncID == F_AUTOSCROLL ){
		if( 1 == m_nAutoScrollMode ){
			m_bAutoScrollDragMode = false;
			AutoScrollEnter();
			return;
		}else if( 2 == m_nAutoScrollMode && m_bAutoScrollDragMode ){
			AutoScrollExit();
			return;
		}
	}else
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
	if( m_nAutoScrollMode ){
		AutoScrollExit();
	}
}

void CALLBACK AutoScrollTimerProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
	CEditView*	pCEditView;
	pCEditView = ( CEditView* )::GetWindowLongPtr( hwnd, 0 );
	if( NULL != pCEditView ){
		pCEditView->AutoScrollOnTimer();
	}
}

void CEditView::AutoScrollEnter()
{
	m_bAutoScrollVertical = GetTextArea().m_nViewRowNum < m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2;
	m_bAutoScrollHorizontal = GetTextArea().m_nViewColNum < GetRightEdgeForScrollBar();
	if( m_bMiniMap ){
		m_bAutoScrollHorizontal = false;
	}
	if( !m_bAutoScrollHorizontal && !m_bAutoScrollVertical ){
		m_nAutoScrollMode = 0;
		::ReleaseCapture();
		return;
	}
	m_nAutoScrollMode = 2;
	m_cAutoScrollWnd.Create(G_AppInstance(), GetHwnd(), m_bAutoScrollVertical, m_bAutoScrollHorizontal, m_cAutoScrollMousePos, this);
	::SetTimer(GetHwnd(), 2, 200, AutoScrollTimerProc);
	HCURSOR hCursor;
	hCursor = ::LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_CURSOR_AUTOSCROLL_CENTER));
	::SetCursor(hCursor);
}

void CEditView::AutoScrollExit()
{
	if( m_nAutoScrollMode ){
		::ReleaseCapture();
	}
	if( 2 == m_nAutoScrollMode ){
		KillTimer(GetHwnd(), 2);
		m_cAutoScrollWnd.Close();
	}
	m_nAutoScrollMode = 0;
}

void CEditView::AutoScrollMove( CMyPoint& point )
{
	const CMyPoint relPos = point - m_cAutoScrollMousePos;
	int idcX, idcY;
	if( !m_bAutoScrollHorizontal || abs(relPos.x) < 16 ){
		idcX = 0;
	}else if( relPos.x < 0 ){
		idcX = 1;
	}else{
		idcX = 2;
	}
	if( !m_bAutoScrollVertical || abs(relPos.y) < 16 ){
		idcY = 0;
	}else if( relPos.y < 0 ){
		idcY = 1;
	}else{
		idcY = 2;
	}
	const int idcs[3][3] = {
		{IDC_CURSOR_AUTOSCROLL_CENTER, IDC_CURSOR_AUTOSCROLL_UP,       IDC_CURSOR_AUTOSCROLL_DOWN},
		{IDC_CURSOR_AUTOSCROLL_LEFT,   IDC_CURSOR_AUTOSCROLL_UP_LEFT,  IDC_CURSOR_AUTOSCROLL_DOWN_LEFT},
		{IDC_CURSOR_AUTOSCROLL_RIGHT,  IDC_CURSOR_AUTOSCROLL_UP_RIGHT, IDC_CURSOR_AUTOSCROLL_DOWN_RIGHT}};
	int cursor = idcs[idcX][idcY];
	if( cursor == IDC_CURSOR_AUTOSCROLL_CENTER ){
		if( !m_bAutoScrollVertical ){
			cursor = IDC_CURSOR_AUTOSCROLL_HORIZONTAL;
		}else if( !m_bAutoScrollHorizontal ){
			cursor = IDC_CURSOR_AUTOSCROLL_VERTICAL;
		}
	}
	const HCURSOR hCursor = ::LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(cursor));
	::SetCursor(hCursor);
}

void CEditView::AutoScrollOnTimer()
{
	CMyPoint cursorPos;
	::GetCursorPos(&cursorPos);
	::ScreenToClient(GetHwnd(), &cursorPos);
	
	const CMyPoint relPos = cursorPos - m_cAutoScrollMousePos;
	CMyPoint scrollPos = relPos / 8;
	if( m_bAutoScrollHorizontal ){
		if( scrollPos.x < 0 ){
			scrollPos.x += 1;
		}else if( scrollPos.x > 0 ){
			scrollPos.x -= 1;
		}
		SyncScrollH( ScrollAtH( GetTextArea().GetViewLeftCol() + GetTextMetrics().GetLayoutXDefault(scrollPos.x) ) );
	}
	if( m_bAutoScrollVertical ){
		if( scrollPos.y < 0 ){
			scrollPos.y += 1;
		}else if( scrollPos.y > 0 ){
			scrollPos.y -= 1;
		}
		SyncScrollV( ScrollAtV( GetTextArea().GetViewTopLine() + scrollPos.y ) );
	}
}

// novice 2004/10/10 マウスサイドボタン対応
/*!
	@brief マウスサイドボタン1を押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.10 novice 新規作成
	@date 2004.10.11 novice マウス中ボタン対応のため変更
	@date 2009.01.17 nasukoji	ボタンUPでコマンドを起動するように変更
*/
void CEditView::OnXLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	if( m_nAutoScrollMode ){
		AutoScrollExit();
	}
}


/*!
	@brief マウスサイドボタン1を開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標

	@date 2009.01.17 nasukoji	新規作成（ボタンUPでコマンドを起動するように変更）
*/
void CEditView::OnXLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if( GetDllShareData().m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_LEFTSIDE &&
	    m_pcEditWnd->IsPageScrollByWheel() )
	{
		m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// ホイール操作によるページスクロールあり
	if( GetDllShareData().m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_LEFTSIDE &&
	    m_pcEditWnd->IsHScrollByWheel() )
	{
		m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウスサイドボタン1に対応する機能コードはm_Common.m_pKeyNameArr[3]に入っている */
	nFuncID = GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_LEFTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}


/*!
	@brief マウスサイドボタン2を押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.10 novice 新規作成
	@date 2004.10.11 novice マウス中ボタン対応のため変更
	@date 2009.01.17 nasukoji	ボタンUPでコマンドを起動するように変更
*/
void CEditView::OnXRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	if( m_nAutoScrollMode ){
		AutoScrollExit();
	}
}


/*!
	@brief マウスサイドボタン2を開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標

	@date 2009.01.17 nasukoji	新規作成（ボタンUPでコマンドを起動するように変更）
*/
void CEditView::OnXRBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if( GetDllShareData().m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_RIGHTSIDE &&
	    m_pcEditWnd->IsPageScrollByWheel() )
	{
		// ホイール操作によるページスクロールありをOFF
		m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// ホイール操作によるページスクロールあり
	if( GetDllShareData().m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_RIGHTSIDE &&
	    m_pcEditWnd->IsHScrollByWheel() )
	{
		// ホイール操作による横スクロールありをOFF
		m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウスサイドボタン2に対応する機能コードはm_Common.m_pKeyNameArr[4]に入っている */
	nFuncID = GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_RIGHTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}

/* マウス移動のメッセージ処理 */
void CEditView::OnMOUSEMOVE( WPARAM fwKeys, int xPos_, int yPos_ )
{
	CMyPoint ptMouse(xPos_, yPos_);

	if( m_cMousePousePos != ptMouse ){
		m_cMousePousePos = ptMouse;
		if( m_nMousePouse < 0 ){
			m_nMousePouse = 0;
		}
	}

	CLayoutRange sSelect_Old    = GetSelectionInfo().m_sSelect;

	// オートスクロール
	if( 1 == m_nAutoScrollMode ){
		if( ::GetSystemMetrics(SM_CXDOUBLECLK) < abs(ptMouse.x - m_cAutoScrollMousePos.x) ||
		    ::GetSystemMetrics(SM_CYDOUBLECLK) < abs(ptMouse.y - m_cAutoScrollMousePos.y) ){
			m_bAutoScrollDragMode = true;
			AutoScrollEnter();
		}
		return;
	}else if( 2 == m_nAutoScrollMode ){
		AutoScrollMove(ptMouse);
		return;
	}

	if( m_bMiniMap ){
		POINT		po;
		::GetCursorPos( &po );
		// 辞書Tipが起動されている
		if( 0 == m_dwTipTimer ){
			if( (m_poTipCurPos.x != po.x || m_poTipCurPos.y != po.y ) ){
				m_cTipWnd.Hide();
				m_dwTipTimer = ::GetTickCount();
			}
		}else{
			m_dwTipTimer = ::GetTickCount();
		}
		if( m_bMiniMapMouseDown ){
			CLayoutPoint ptNew;
			CTextArea& area = GetTextArea();
			area.ClientToLayout( ptMouse, &ptNew );
			// ミニマップの上下スクロール
			if( ptNew.y < 0 ){
				ptNew.y = CLayoutYInt(0);
			}
			CLayoutYInt nScrollRow = CLayoutYInt(0);
			CLayoutYInt nScrollMargin = CLayoutYInt(15);
			nScrollMargin  = t_min(nScrollMargin,  (GetTextArea().m_nViewRowNum) / 2);
			if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() > area.m_nViewRowNum &&
				ptNew.y > area.GetViewTopLine() + area.m_nViewRowNum - nScrollMargin ){
				nScrollRow = (area.GetViewTopLine() + area.m_nViewRowNum - nScrollMargin) - ptNew.y;
			}else if( 0 < area.GetViewTopLine() && ptNew.y < area.GetViewTopLine() + nScrollMargin ){
				nScrollRow = area.GetViewTopLine() + nScrollMargin - ptNew.y;
				if( 0 > area.GetViewTopLine() - nScrollRow ){
					nScrollRow = area.GetViewTopLine();
				}
			}
			if( nScrollRow != 0 ){
				ScrollAtV( area.GetViewTopLine() - nScrollRow );
			}

			GetTextArea().ClientToLayout( ptMouse, &ptNew );
			if( ptNew.y < 0 ){
				ptNew.y = CLayoutYInt(0);
			}
			CEditView& view = m_pcEditWnd->GetActiveView();
			ptNew.x = 0;
			CLogicPoint ptNewLogic;
			view.GetCaret().GetAdjustCursorPos( &ptNew );
			GetDocument()->m_cLayoutMgr.LayoutToLogic( ptNew, &ptNewLogic );
			GetDocument()->m_cLayoutMgr.LogicToLayout( ptNewLogic, &ptNew, ptNew.y );
			if( GetKeyState_Shift() ){
				if( view.GetSelectionInfo().IsTextSelected() ){
					if( view.GetSelectionInfo().IsBoxSelecting() ){
						view.GetSelectionInfo().DisableSelectArea( true );
						view.GetSelectionInfo().BeginSelectArea();
					}
				}else{
					view.GetSelectionInfo().BeginSelectArea();
				}
				view.GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptNew );
			}else{
				if( view.GetSelectionInfo().IsTextSelected() ){
					view.GetSelectionInfo().DisableSelectArea( true );
				}
			}
			view.GetCaret().MoveCursor( ptNew, true );
			view.GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)
		return;
	}

	if( !GetSelectionInfo().IsMouseSelecting() ){
		// マウスによる範囲選択中でない場合
		POINT		po;
		::GetCursorPos( &po );
		//	2001/06/18 asa-o: 補完ウィンドウが表示されていない
		if(!m_bHokan){
			/* 辞書Tipが起動されている */
			if( 0 == m_dwTipTimer ){
				if( (m_poTipCurPos.x != po.x || m_poTipCurPos.y != po.y ) ){
					/* 辞書Tipを消す */
					m_cTipWnd.Hide();
					m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
				}
			}else{
				m_dwTipTimer = ::GetTickCount();		/* 辞書Tip起動タイマー */
			}
		}
		/* 現在のマウスカーソル位置→レイアウト位置 */
		CLayoutPoint ptNew;
		GetTextArea().ClientToLayout(ptMouse, &ptNew);

		CLogicRange	cUrlRange;	//URL範囲

		/* 選択テキストのドラッグ中か */
		if( m_bDragMode ){
			if( GetDllShareData().m_Common.m_sEdit.m_bUseOLE_DragDrop ){	/* OLEによるドラッグ & ドロップを使う */
				/* 座標指定によるカーソル移動 */
				GetCaret().MoveCursorToClientPoint( ptMouse );
			}
		}
		else{
			/* 行選択エリア? */
			if( ptMouse.x < GetTextArea().GetAreaLeft() || ptMouse.y < GetTextArea().GetAreaTop() ){	//	2002/2/10 aroka
				/* 矢印カーソル */
				if( ptMouse.y >= GetTextArea().GetAreaTop() )
					::SetCursor( ::LoadCursor( G_AppInstance(), MAKEINTRESOURCE( IDC_CURSOR_RVARROW ) ) );
				else
					::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}
			else if( GetDllShareData().m_Common.m_sEdit.m_bUseOLE_DragDrop	/* OLEによるドラッグ & ドロップを使う */
			 && GetDllShareData().m_Common.m_sEdit.m_bUseOLE_DropSource /* OLEによるドラッグ元にするか */
			 && 0 == IsCurrentPositionSelected(						/* 指定カーソル位置が選択エリア内にあるか */
				ptNew	// カーソル位置
				)
			){
				/* 矢印カーソル */
				::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}
			/* カーソル位置にURLが有る場合 */
			else if(
				IsCurrentPositionURL(
					ptNew,			// カーソル位置
					&cUrlRange,		// URL範囲
					NULL			// URL受け取り先
				)
			){
				/* 手カーソル */
				SetHandCursor();		// Hand Cursorを設定 2013/1/29 Uchi
			}else{
				//migemo isearch 2004.10.22
				if( m_nISearchMode > SEARCH_NONE ){
					if (m_nISearchDirection == SEARCH_FORWARD){
						::SetCursor( ::LoadCursor( G_AppInstance(),MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
					}else{
						::SetCursor( ::LoadCursor( G_AppInstance(),MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
					}
				}else
				/* アイビーム */
				if( 0 <= m_nMousePouse ){
					::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
				}
			}
		}
		return;
	}
	// 以下、マウスでの選択中(ドラッグ中)

	if( 0 <= m_nMousePouse ){
		::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
	}

	// 2010.07.15 Moca ドラッグ開始位置から移動していない場合はMOVEとみなさない
	// 遊びは 2px固定とする
	CMyPoint ptMouseMove = ptMouse - m_cMouseDownPos;
	if(m_cMouseDownPos.x != -INT_MAX && abs(ptMouseMove.x) <= 2 && abs(ptMouseMove.y) <= 2 ){
		return;
	}
	// 一度移動したら戻ってきたときも、移動とみなすように設定
	m_cMouseDownPos.Set(-INT_MAX, -INT_MAX);
	
	CLayoutPoint ptNewCursor(CLayoutInt(-1), CLayoutInt(-1));
	if( GetSelectionInfo().IsBoxSelecting() ){	/* 矩形範囲選択中 */
		/* 座標指定によるカーソル移動 */
		GetCaret().MoveCursorToClientPoint( ptMouse, true, &ptNewCursor );
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptNewCursor );
		GetCaret().MoveCursorToClientPoint( ptMouse );
		/* 現在のカーソル位置によって選択範囲を変更 */
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)
	}
	else{
		/* 座標指定によるカーソル移動 */
		if(( ptMouse.x < GetTextArea().GetAreaLeft() || m_dwTripleClickCheck )&& GetSelectionInfo().m_bBeginLineSelect ){	// 行単位選択中
			// 2007.11.15 nasukoji	上方向の行選択時もマウスカーソルの位置の行が選択されるようにする
			CMyPoint nNewPos(0, ptMouse.y);

			// 1行の高さ
			int nLineHeight = GetTextMetrics().GetHankakuDy();

			// 選択開始行以下へのドラッグ時は1行下にカーソルを移動する
			if( GetTextArea().GetViewTopLine() + (ptMouse.y - GetTextArea().GetAreaTop()) / nLineHeight >= GetSelectionInfo().m_sSelectBgn.GetTo().y)
				nNewPos.y += nLineHeight;

			// カーソルを移動
			nNewPos.x = GetTextArea().GetAreaLeft() - GetTextMetrics().GetCharPxWidth(GetTextArea().GetViewLeftCol());
			GetCaret().MoveCursorToClientPoint( nNewPos, false, &ptNewCursor );

			// 2.5クリックによる行単位のドラッグ
			if( m_dwTripleClickCheck ){
				// 選択開始行以上にドラッグした
				if( ptNewCursor.GetY() <= GetSelectionInfo().m_sSelectBgn.GetTo().y ){
					// GetCommander().Command_GOLINETOP( true, 0x09 );		// 改行単位の行頭へ移動
					CLogicInt nLineLen;
					const CLayout*	pcLayout;
					const wchar_t*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( ptNewCursor.GetY2(), &nLineLen, &pcLayout );
					ptNewCursor.x = CLayoutInt(0);
					if( pLine ){
						while( pcLayout->GetLogicOffset() ){
							ptNewCursor.y--;
							pcLayout = pcLayout->GetPrevLayout();
						}
					}
				}else{
					CLayoutPoint ptCaret;

					CLogicPoint ptCaretPrevLog(0, GetCaret().GetCaretLogicPos().y);

					// 選択開始行より下にカーソルがある時は1行前と物理行番号の違いをチェックする
					// 選択開始行にカーソルがある時はチェック不要
					if( ptNewCursor.GetY() > GetSelectionInfo().m_sSelectBgn.GetTo().y ){
						// 1行前の物理行を取得する
						m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( CLayoutPoint(CLayoutInt(0), ptNewCursor.GetY() - 1), &ptCaretPrevLog );
					}

					CLogicPoint ptNewCursorLogic;
					m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(ptNewCursor, &ptNewCursorLogic);
					// 前の行と同じ物理行
					if( ptCaretPrevLog.y == ptNewCursorLogic.y ){
						// 1行先の物理行からレイアウト行を求める
						m_pcEditDoc->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, GetCaret().GetCaretLogicPos().y + 1), &ptCaret );

						// カーソルを次の物理行頭へ移動する
						ptNewCursor = ptCaret;
					}
				}
			}
		}else{
			GetCaret().MoveCursorToClientPoint( ptMouse, true, &ptNewCursor );
		}
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)

		/* CTRLキーが押されていたか */
//		if( GetKeyState_Control() ){
		if( !GetSelectionInfo().m_bBeginWordSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptNewCursor );
			GetCaret().MoveCursor( ptNewCursor, true, 1000 );
		}else{
			CLayoutRange sSelect;
			
			/* 現在のカーソル位置によって選択範囲を変更(テストのみ) */
			GetSelectionInfo().ChangeSelectAreaByCurrentCursorTEST(
				GetCaret().GetCaretLayoutPos(),
				&sSelect
			);
			/* 選択範囲に変更なし */
			if( sSelect_Old == sSelect ){
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor(
					GetCaret().GetCaretLayoutPos()
				);
				GetCaret().MoveCursor( ptNewCursor, true, 1000 );
				return;
			}
			CLogicInt nLineLen;
			const CLayout* pcLayout;
			if( NULL != m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout ) ){
				CLogicInt	nIdx = LineColumnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
				CLayoutRange sRange;

				/* 現在位置の単語の範囲を調べる */
				bool bResult = m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
					GetCaret().GetCaretLayoutPos().GetY2(),
					nIdx,
					&sRange,
					NULL,
					NULL
				);
				if( bResult ){
					// 指定された行のデータ内の位置に対応する桁の位置を調べる
					// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
					/*
					pLine     = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetFrom().GetY2(), &nLineLen, &pcLayout );
					sRange.SetFromX( LineIndexToColumn( pcLayout, sRange.GetFrom().x ) );
					pLine     = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetTo().GetY2(), &nLineLen, &pcLayout );
					sRange.SetToX( LineIndexToColumn( pcLayout, sRange.GetTo().x ) );
					*/
					int nWorkF = IsCurrentPositionSelectedTEST(
						sRange.GetFrom(), //カーソル位置
						sSelect
					);
					int nWorkT = IsCurrentPositionSelectedTEST(
						sRange.GetTo(),	// カーソル位置
						sSelect
					);
					if( -1 == nWorkF ){
						/* 始点が前方に移動。現在のカーソル位置によって選択範囲を変更 */
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRange.GetFrom() );
					}
					else if( 1 == nWorkT ){
						/* 終点が後方に移動。現在のカーソル位置によって選択範囲を変更 */
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRange.GetTo() );
					}
					else if( sSelect_Old.GetFrom() == sSelect.GetFrom() ){
						/* 始点が無変更＝前方に縮小された */
						/* 現在のカーソル位置によって選択範囲を変更 */
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRange.GetTo() );
					}
					else if( sSelect_Old.GetTo()==sSelect.GetTo() ){
						/* 終点が無変更＝後方に縮小された */
						/* 現在のカーソル位置によって選択範囲を変更 */
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRange.GetFrom() );
					}
				}else{
					/* 現在のカーソル位置によって選択範囲を変更 */
					GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
				}
			}else{
				/* 現在のカーソル位置によって選択範囲を変更 */
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
			}
			GetCaret().MoveCursor( ptNewCursor, true, 1000 );
		}
	}
	return;
}
//m_dwTipTimerm_dwTipTimerm_dwTipTimer


#ifndef SPI_GETWHEELSCROLLCHARS
#define SPI_GETWHEELSCROLLCHARS 0x006C
#endif


/* マウスホイールのメッセージ処理
	2009.01.17 nasukoji	ホイールスクロールを利用したページスクロール・横スクロール対応
	2011.11.16 Moca スクロール変化量への対応
	2013.09.10 Moca スペシャルスクロールの不具合の修正
*/
LRESULT CEditView::OnMOUSEWHEEL2( WPARAM wParam, LPARAM lParam, bool bHorizontalMsg, EFunctionCode nCmdFuncID )
{
//	WORD	fwKeys;
	short	zDelta;
//	short	xPos;
//	short	yPos;
	int		i;
	int		nScrollCode;
	int		nRollLineNum;

//	fwKeys = LOWORD(wParam);			// key flags
	zDelta = (short) HIWORD(wParam);	// wheel rotation
//	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
//	yPos = (short) HIWORD(lParam);		// vertical position of pointer
//	MYTRACE( _T("CEditView::DispatchEvent() WM_MOUSEWHEEL fwKeys=%xh zDelta=%d xPos=%d yPos=%d \n"), fwKeys, zDelta, xPos, yPos );

	if( bHorizontalMsg ){
		if( 0 < zDelta ){
			nScrollCode = SB_LINEDOWN; // 右
		}else{
			nScrollCode = SB_LINEUP; // 左
		}
		zDelta *= -1; // 反対にする
	}else{
		if( 0 < zDelta ){
			nScrollCode = SB_LINEUP;
		}else{
			nScrollCode = SB_LINEDOWN;
		}
	}

	{
		// 2009.01.17 nasukoji	キー/マウスボタン + ホイールスクロールで横スクロールする
		bool bHorizontal = false;
		bool bKeyPageScroll = false;
		if( nCmdFuncID == F_0 ){
			// 通常スクロールの時だけ適用
			bHorizontal = IsSpecialScrollMode( GetDllShareData().m_Common.m_sGeneral.m_nHorizontalScrollByWheel );
			bKeyPageScroll = IsSpecialScrollMode( GetDllShareData().m_Common.m_sGeneral.m_nPageScrollByWheel );
		}

		// 2013.05.30 Moca ホイールスクロールにキー割り当て
		int nIdx = getCtrlKeyState();
		EFunctionCode nFuncID = nCmdFuncID;
		if( nFuncID != F_0 ){
		}else if( bHorizontalMsg ){
			if( nScrollCode == SB_LINEUP ){
				nFuncID = GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_WHEELLEFT].m_nFuncCodeArr[nIdx];
			}else{
				nFuncID = GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_WHEELRIGHT].m_nFuncCodeArr[nIdx];
			}
		}else{
			if( nScrollCode == SB_LINEUP ){
				nFuncID = GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_WHEELUP].m_nFuncCodeArr[nIdx];
			}else{
				nFuncID = GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_WHEELDOWN].m_nFuncCodeArr[nIdx];
			}
		}
		bool bExecCmd = false;
		{
			if( nFuncID < F_WHEEL_FIRST || F_WHEEL_LAST < nFuncID ){
				bExecCmd = true;
			}
			if( nFuncID == F_WHEELLEFT || nFuncID == F_WHEELRIGHT
				|| nFuncID == F_WHEELPAGELEFT ||  nFuncID == F_WHEELPAGERIGHT ){
				bHorizontal = true;
			}
			if( nFuncID == F_WHEELPAGEUP || nFuncID == F_WHEELPAGEDOWN
				|| nFuncID == F_WHEELPAGELEFT ||  nFuncID == F_WHEELPAGERIGHT ){
				bKeyPageScroll = true;
			}
			if( nFuncID == F_WHEELUP || nFuncID == F_WHEELLEFT
				|| nFuncID == F_WHEELPAGEUP || nFuncID == F_WHEELPAGELEFT ){
				if( nScrollCode != SB_LINEUP ){
					zDelta *= -1;
					nScrollCode = SB_LINEUP;
				}
			}else if( nFuncID == F_WHEELDOWN || nFuncID == F_WHEELRIGHT
				|| nFuncID == F_WHEELPAGEDOWN || nFuncID == F_WHEELPAGERIGHT ){
				if( nScrollCode != SB_LINEDOWN ){
					zDelta *= -1;
					nScrollCode = SB_LINEDOWN;
				}
			}
		}

		/* マウスホイールによるスクロール行数をレジストリから取得 */
		nRollLineNum = 3;

		/* レジストリの存在チェック */
		// 2006.06.03 Moca ReadRegistry に書き換え
		unsigned int uDataLen;	// size of value data
		TCHAR szValStr[256];
		uDataLen = _countof(szValStr) - 1;
		if( !bExecCmd ){
			bool bGetParam = false;
			if( bHorizontal ){
				int nScrollChars = 3;
				if( ::SystemParametersInfo( SPI_GETWHEELSCROLLCHARS, 0, &nScrollChars, 0 ) ){
					bGetParam = true;
					nRollLineNum = nScrollChars;
					if( nRollLineNum != -1 && m_bMiniMap ){
						nRollLineNum *= 10;
					}
				}
			}
			if( !bGetParam ){
				if( ReadRegistry( HKEY_CURRENT_USER, _T("Control Panel\\desktop"), _T("WheelScrollLines"), szValStr, uDataLen ) ){
					nRollLineNum = ::_ttoi( szValStr );
					if( nRollLineNum != -1 && m_bMiniMap ){
						nRollLineNum *= 10;
					}
				}
			}
		}

		if( -1 == nRollLineNum || bKeyPageScroll ){
			/* 「1画面分スクロールする」 */
			if( bHorizontal ){
				nRollLineNum = (Int)GetTextArea().m_nViewColNum - 1;	// 表示域の桁数
			}else{
				nRollLineNum = (Int)GetTextArea().m_nViewRowNum - 1;	// 表示域の行数
			}
		}
		else{
			if( nRollLineNum > 30 ){	//@@@ YAZAKI 2001.12.31 10→30へ。
				nRollLineNum = 30;
			}
		}
		if( nRollLineNum < 1 || bExecCmd ){
			nRollLineNum = 1;
		}

		// スクロール操作の種類(通常方法のページスクロールはNORMAL扱い)
		if( bKeyPageScroll ){
			if( bHorizontal ){
				// ホイール操作による横スクロールあり
				m_pcEditWnd->SetHScrollByWheel( TRUE );
			}
			// ホイール操作によるページスクロールあり
			m_pcEditWnd->SetPageScrollByWheel( TRUE );
		}else{
			if( bHorizontal ){
				// ホイール操作による横スクロールあり
				m_pcEditWnd->SetHScrollByWheel( TRUE );
			}
		}

		if( nFuncID != m_eWheelScroll
				|| ( zDelta < 0 && 0 < m_nWheelDelta )
				|| ( 0 < zDelta && m_nWheelDelta < 0 ) ){
			m_nWheelDelta = 0;
			m_eWheelScroll = nFuncID;
		}
		m_nWheelDelta += zDelta;

		// 2011.05.18 APIのスクロール量に従う
		int nRollNum = abs(m_nWheelDelta) * nRollLineNum / 120;
		// 次回持越しの変化量(上記式Deltaのあまり。スクロール方向とzDeltaは符号が反対)
		m_nWheelDelta = (abs(m_nWheelDelta) - nRollNum * 120 / nRollLineNum) * ((nScrollCode == SB_LINEUP) ? 1 : -1);

		if( bExecCmd ){
			if( nFuncID != F_0 ){
				// スクロール変化量分コマンド実行(zDeltaが120あたりで1回)
				for( int i = 0; i < nRollNum; i++ ){
					::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
				}
			}
			return bHorizontalMsg ? TRUE: 0;
		}

		const bool bSmooth = !! GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScroll_Smooth;
		const int nRollActions = bSmooth ? nRollNum : 1;
		int nCount = ((nScrollCode == SB_LINEUP) ? -1 : 1) * (bSmooth ? 1 : nRollNum);		// 1回あたりのスクロール数(縦横混在)

		for( i = 0; i < nRollActions; ++i ){
			//	Sep. 11, 2004 genta 同期スクロール行数
			if( bHorizontal ){
				const CLayoutXInt layoutCount = nCount * GetTextMetrics().GetLayoutXDefault();
				SyncScrollH( ScrollAtH( GetTextArea().GetViewLeftCol() + layoutCount ) );
			}else{
				SyncScrollV( ScrollAtV( GetTextArea().GetViewTopLine() + CLayoutInt(nCount) ) );
			}
		}
	}
	return bHorizontalMsg ? TRUE: 0;
}


/*! 垂直マウススクロール
*/
LRESULT CEditView::OnMOUSEWHEEL( WPARAM wParam, LPARAM lParam )
{
	return OnMOUSEWHEEL2( wParam, lParam, false, F_0 );
}

/*! 水平マウススクロール
	@note http://msdn.microsoft.com/en-us/library/ms997498.aspx
	Best Practices for Supporting Microsoft Mouse and Keyboard Devices
	によると、WM_MOUSEHWHEELを処理した場合はTRUEを返す必要があるそうです。
	MSDNのWM_MOUSEHWHEEL Messageのページは間違っているので注意。
*/
LRESULT CEditView::OnMOUSEHWHEEL( WPARAM wParam, LPARAM lParam )
{
	return OnMOUSEWHEEL2( wParam, lParam, true, F_0 );
}

/*!
	@brief キー・マウスボタン状態よりスクロールモードを判定する

	マウスホイール時、行スクロールすべきかページスクロール・横スクロール
	すべきかを判定する。
	現在のキーまたはマウス状態が引数で指定された組み合わせに合致する場合
	trueを返す。

	@param nSelect	[in] キー・マウスボタンの組み合わせ指定番号

	@return ページスクロールまたは横スクロールすべき状態の時trueを返す
	        通常の行スクロールすべき状態の時falseを返す

	@date 2009.01.17 nasukoji	新規作成
*/
bool CEditView::IsSpecialScrollMode( int nSelect )
{
	bool bSpecialScrollMode;

	switch( nSelect ){
	case 0:		// 指定の組み合わせなし
		bSpecialScrollMode = false;
		break;

	case MOUSEFUNCTION_CENTER:		// マウス中ボタン
		bSpecialScrollMode = ( 0 != ( 0x8000 & ::GetAsyncKeyState( VK_MBUTTON ) ) );
		break;

	case MOUSEFUNCTION_LEFTSIDE:	// マウスサイドボタン1
		bSpecialScrollMode = ( 0 != ( 0x8000 & ::GetAsyncKeyState( VK_XBUTTON1 ) ) );
		break;

	case MOUSEFUNCTION_RIGHTSIDE:	// マウスサイドボタン2
		bSpecialScrollMode = ( 0 != ( 0x8000 & ::GetAsyncKeyState( VK_XBUTTON2 ) ) );
		break;

	case VK_CONTROL:	// Controlキー
		bSpecialScrollMode = GetKeyState_Control();
		break;

	case VK_SHIFT:		// Shiftキー
		bSpecialScrollMode = GetKeyState_Shift();
		break;

	default:	// 上記以外（ここには来ない）
		bSpecialScrollMode = false;
		break;
	}

	return bSpecialScrollMode;
}







/* マウス左ボタン開放のメッセージ処理 */
void CEditView::OnLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
//	MYTRACE( _T("OnLBUTTONUP()\n") );

	/* 範囲選択終了 & マウスキャプチャーおわり */
	if( GetSelectionInfo().IsMouseSelecting() ){	/* 範囲選択中 */
		/* マウス キャプチャを解放 */
		::ReleaseCapture();
		GetCaret().ShowCaret_( GetHwnd() ); // 2002/07/22 novice

		GetSelectionInfo().SelectEnd();

		// 20100715 Moca マウスクリック座標をリセット
		m_cMouseDownPos.Set(-INT_MAX, -INT_MAX);

		GetCaret().m_cUnderLine.UnderLineUnLock();
		if( GetSelectionInfo().m_sSelect.IsOne() ){
			/* 現在の選択範囲を非選択状態に戻す */
			GetSelectionInfo().DisableSelectArea( true );
		}
	}
	if( m_bMiniMapMouseDown ){
		m_bMiniMapMouseDown = false;
		::ReleaseCapture();
	}
	return;
}



/* ShellExecuteを呼び出すプロシージャ */
/*   呼び出し前に lpParameter を new しておくこと */
static unsigned __stdcall ShellExecuteProc( LPVOID lpParameter )
{
	LPTSTR pszFile = (LPTSTR)lpParameter;
	::ShellExecute( NULL, _T("open"), pszFile, NULL, NULL, SW_SHOW );
	delete []pszFile;
	return 0;
}


// マウス左ボタンダブルクリック
// 2007.01.18 kobake IsCurrentPositionURL仕様変更に伴い、処理の書き換え
void CEditView::OnLBUTTONDBLCLK( WPARAM fwKeys, int _xPos , int _yPos )
{
	CMyPoint ptMouse(_xPos,_yPos);

	CLogicRange		cUrlRange;	// URL範囲
	std::wstring	wstrURL;
	const wchar_t*	pszMailTo = L"mailto:";

	// 2007.10.06 nasukoji	クアドラプルクリック時はチェックしない
	if(! m_dwTripleClickCheck){
		/* カーソル位置にURLが有る場合のその範囲を調べる */
		if(
			IsCurrentPositionURL(
				GetCaret().GetCaretLayoutPos(),	// カーソル位置
				&cUrlRange,				// URL範囲
				&wstrURL				// URL受け取り先
			)
		){
			std::wstring wstrOPEN;

			// URLを開く
		 	// 現在位置がメールアドレスならば、NULL以外と、その長さを返す
			if( IsMailAddress( wstrURL.c_str(), wstrURL.length(), NULL ) ){
				wstrOPEN = pszMailTo + wstrURL;
			}
			else{
				if( wcsnicmp( wstrURL.c_str(), L"ttp://", 6 ) == 0 ){	//抑止URL
					wstrOPEN = L"h" + wstrURL;
				}
				else if( wcsnicmp( wstrURL.c_str(), L"tp://", 5 ) == 0 ){	//抑止URL
					wstrOPEN = L"ht" + wstrURL;
				}
				else{
					wstrOPEN = wstrURL;
				}
			}
			{
				// URLを開く
				// 2009.05.21 syat UNCパスだと1分以上無応答になることがあるのでスレッド化
				CWaitCursor cWaitCursor( GetHwnd() );	// カーソルを砂時計にする

				unsigned int nThreadId;
				LPCTSTR szUrl = to_tchar(wstrOPEN.c_str());
				LPTSTR szUrlDup = new TCHAR[_tcslen( szUrl ) + 1];
				_tcscpy( szUrlDup, szUrl );
				HANDLE hThread = (HANDLE)_beginthreadex( NULL, 0, ShellExecuteProc, (LPVOID)szUrlDup, 0, &nThreadId );
				if( hThread != INVALID_HANDLE_VALUE ){
					// ユーザーのURL起動指示に反応した目印としてちょっとの時間だけ砂時計カーソルを表示しておく
					// ShellExecute は即座にエラー終了することがちょくちょくあるので WaitForSingleObject ではなく Sleep を使用（ex.存在しないパスの起動）
					// 【補足】いずれの API でも待ちを長め（2～3秒）にするとなぜか Web ブラウザ未起動からの起動が重くなる模様（PCタイプ, XP/Vista, IE/FireFox に関係なく）
					::Sleep(200);
					::CloseHandle(hThread);
				}else{
					//スレッド作成失敗
					delete[] szUrlDup;
				}
			}
			return;
		}

		/* GREP出力モードまたはデバッグモード かつ マウス左ボタンダブルクリックでタグジャンプ の場合 */
		//	2004.09.20 naoh 外部コマンドの出力からTagjumpできるように
		if( (CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode || CAppMode::getInstance()->IsDebugMode()) && GetDllShareData().m_Common.m_sSearch.m_bGTJW_LDBLCLK ){
			/* タグジャンプ機能 */
			if( GetCommander().Command_TAGJUMP() ){
				// 2013.05.27 タグジャンプ失敗時は通常の処理を実行する
				return;
			}
		}
	}

// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	int	nIdx = getCtrlKeyState();

	/* マウス左クリックに対応する機能コードはm_Common.m_pKeyNameArr[?]に入っている 2007.11.15 nasukoji */
	EFunctionCode	nFuncID = GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[
		m_dwTripleClickCheck ? MOUSEFUNCTION_QUADCLICK : MOUSEFUNCTION_DOUBLECLICK
	].m_nFuncCodeArr[nIdx];
	if(m_dwTripleClickCheck){
		// 非選択状態にした後左クリックしたことにする
		// すべて選択の場合は、3.5クリック時の選択状態保持とドラッグ開始時の
		// 範囲変更のため。
		// クアドラプルクリック機能が割り当てられていない場合は、ダブルクリック
		// として処理するため。
		if( GetSelectionInfo().IsTextSelected() )		// テキストが選択されているか
			GetSelectionInfo().DisableSelectArea( true );		// 現在の選択範囲を非選択状態に戻す

		if(! nFuncID){
			m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
			nFuncID = GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_DOUBLECLICK].m_nFuncCodeArr[nIdx];
			OnLBUTTONDOWN( fwKeys, ptMouse.x , ptMouse.y );	// カーソルをクリック位置へ移動する
		}
	}

	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::SendMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	// 2007.10.06 nasukoji	クアドラプルクリック時もここで抜ける
	if(m_dwTripleClickCheck){
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF（次回は通常クリック）
		return;
	}

	// 2007.11.06 nasukoji	ダブルクリックが単語選択でなくてもトリプルクリックを有効とする
	// 2007.10.02 nasukoji	トリプルクリックチェック用に時刻を取得
	m_dwTripleClickCheck = ::GetTickCount();

	// ダブルクリック位置として記憶
	GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)

	/*	2007.07.09 maru 機能コードの判定を追加
		ダブルクリックからのドラッグでは単語単位の範囲選択(エディタの一般的動作)になるが
		この動作は、ダブルクリック＝単語選択を前提としたもの。
		キー割り当ての変更により、ダブルクリック≠単語選択のときには GetSelectionInfo().m_bBeginWordSelect = true
		にすると、処理の内容によっては表示がおかしくなるので、ここで抜けるようにする。
	*/
	if(F_SELECTWORD != nFuncID) return;

	/* 範囲選択開始 & マウスキャプチャー */
	GetSelectionInfo().SelectBeginWord();

	if( GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
		/* ALTキーが押されていたか */
		if( GetKeyState_Alt() ){
			GetSelectionInfo().SetBoxSelect(true);	/* 矩形範囲選択中 */
		}
	}
	::SetCapture( GetHwnd() );
	GetCaret().HideCaret_( GetHwnd() ); // 2002/07/22 novice
	if( GetSelectionInfo().IsTextSelected() ){
		/* 常時選択範囲の範囲 */
		GetSelectionInfo().m_sSelectBgn.SetTo( GetSelectionInfo().m_sSelect.GetTo() );
	}
	else{
		/* 現在のカーソル位置から選択を開始する */
		GetSelectionInfo().BeginSelectArea( );
	}

	return;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           D&D                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

STDMETHODIMP CEditView::DragEnter( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::DragEnter()\n") );
	//「OLEによるドラッグ & ドロップを使う」オプションが無効の場合にはドロップを受け付けない
	if(!GetDllShareData().m_Common.m_sEdit.m_bUseOLE_DragDrop)return E_UNEXPECTED;

	//編集禁止の場合はドロップを受け付けない
	if(!m_pcEditDoc->IsEditable())return E_UNEXPECTED;


	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;

	m_cfDragData = GetAvailableClipFormat( pDataObject );
	if( m_cfDragData == 0 )
		return E_INVALIDARG;
	else if( m_cfDragData == CF_HDROP ){
		// 右ボタンで入ってきたときだけファイルをビューで取り扱う
		if( !(MK_RBUTTON & dwKeyState) )
			return E_INVALIDARG;
	}

	/* 自分をアクティブペインにする */
	m_pcEditWnd->SetActivePane( m_nMyIndex );

	// 現在のカーソル位置を記憶する	// 2007.12.09 ryoji
	m_ptCaretPos_DragEnter = GetCaret().GetCaretLayoutPos();
	m_nCaretPosX_Prev_DragEnter = GetCaret().m_nCaretPosX_Prev;

	// ドラッグデータは矩形か
	m_bDragBoxData = IsDataAvailable( pDataObject, (CLIPFORMAT)::RegisterClipboardFormat( _T("MSDEVColumnSelect") ) );

	/* 選択テキストのドラッグ中か */
	_SetDragMode( TRUE );

	DragOver( dwKeyState, pt, pdwEffect );
	return S_OK;
}

STDMETHODIMP CEditView::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::DragOver()\n") );

	/* マウス移動のメッセージ処理 */
	::ScreenToClient( GetHwnd(), (LPPOINT)&pt );
	OnMOUSEMOVE( dwKeyState, pt.x , pt.y );

	if ( pdwEffect == NULL )
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect( m_cfDragData, dwKeyState, pt, *pdwEffect );

	CEditView* pcDragSourceView = m_pcEditWnd->GetDragSourceView();

	// ドラッグ元が他ビューで、このビューのカーソルがドラッグ元の選択範囲内の場合は禁止マークにする
	// ※自ビューのときは禁止マークにしない（他アプリでも多くはそうなっている模様）	// 2009.06.09 ryoji
	if( pcDragSourceView && !IsDragSource() &&
		!pcDragSourceView->IsCurrentPositionSelected( GetCaret().GetCaretLayoutPos() )
	){
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

STDMETHODIMP CEditView::DragLeave( void )
{
	DEBUG_TRACE( _T("CEditView::DragLeave()\n") );
	/* 選択テキストのドラッグ中か */
	_SetDragMode( FALSE );

	// DragEnter時のカーソル位置を復元	// 2007.12.09 ryoji
	// ※範囲選択中のときに選択範囲とカーソルが分離すると変だから
	GetCaret().MoveCursor( m_ptCaretPos_DragEnter, false );
	GetCaret().m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
	RedrawAll();	// ルーラー、アンダーライン、カーソル位置表示更新

	// 非アクティブ時は表示状態を非アクティブに戻す	// 2007.12.09 ryoji
	if( ::GetActiveWindow() == NULL )
		OnKillFocus();

	return S_OK;
}

STDMETHODIMP CEditView::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::Drop()\n") );
	BOOL		bBoxData;
	BOOL		bMove;
	BOOL		bMoveToPrev = FALSE;
	RECT		rcSel;
	CNativeW	cmemBuf;
	bool		bBeginBoxSelect_Old = false;

	CLayoutRange sSelectBgn_Old;
	CLayoutRange sSelect_Old;



	/* 選択テキストのドラッグ中か */
	_SetDragMode( FALSE );

	// 非アクティブ時は表示状態を非アクティブに戻す	// 2007.12.09 ryoji
	if( ::GetActiveWindow() == NULL )
		OnKillFocus();

	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;

	CLIPFORMAT cf;
	cf = GetAvailableClipFormat( pDataObject );
	if( cf == 0 )
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect( cf, dwKeyState, pt, *pdwEffect );
	if( *pdwEffect == DROPEFFECT_NONE )
		return E_INVALIDARG;

	// ファイルドロップは PostMyDropFiles() で処理する
	if( cf == CF_HDROP )
		return PostMyDropFiles( pDataObject );

	// 外部からのドロップは以後の処理ではコピーと同様に扱う
	CEditView* pcDragSourceView = m_pcEditWnd->GetDragSourceView();
	bMove = (*pdwEffect == DROPEFFECT_MOVE) && pcDragSourceView;
	bBoxData = m_bDragBoxData;

	// カーソルが選択範囲内にあるときはコピー／移動しない	// 2009.06.09 ryoji
	if( pcDragSourceView &&
		!pcDragSourceView->IsCurrentPositionSelected( GetCaret().GetCaretLayoutPos() )
	){
		// DragEnter時のカーソル位置を復元
		// Note. ドラッグ元が他ビューでもマウス移動が速いと稀にここにくる可能性がありそう
		*pdwEffect = DROPEFFECT_NONE;
		GetCaret().MoveCursor( m_ptCaretPos_DragEnter, false );
		GetCaret().m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
		if( !IsDragSource() )	// ドラッグ元の場合はここでは再描画不要（DragDrop後処理のSetActivePaneで再描画される）
			RedrawAll();	// ←主に以後の非アクティブ化に伴うアンダーライン消しのために一度更新して整合をとる
		return S_OK;
	}

	// ドロップデータの取得
	HGLOBAL hData = GetGlobalData( pDataObject, cf );
	if (hData == NULL)
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock( hData );
	SIZE_T nSize = ::GlobalSize( hData );
	if( cf == CClipboard::GetSakuraFormat() ){
		if( nSize > sizeof(int) ){
			wchar_t* pszData = (wchar_t*)((BYTE*)pData + sizeof(int));
			cmemBuf.SetString( pszData, t_min( (SIZE_T)*(int*)pData, nSize / sizeof(wchar_t) ) );	// 途中のNUL文字も含める
		}
	}else if( cf == CF_UNICODETEXT ){
		cmemBuf.SetString( (wchar_t*)pData, wcsnlen( (wchar_t*)pData, nSize / sizeof(wchar_t) ) );
	}else{
		cmemBuf.SetStringOld( (char*)pData, strnlen( (char*)pData, nSize / sizeof(char) ) );
	}

	// アンドゥバッファの準備
	if( NULL == m_cCommander.GetOpeBlk() ){
		m_cCommander.SetOpeBlk(new COpeBlk);
	}
	m_cCommander.GetOpeBlk()->AddRef();

	/* 移動の場合、位置関係を算出 */
	if( bMove ){
		if( bBoxData ){
			/* 2点を対角とする矩形を求める */
			TwoPointToRect(
				&rcSel,
				pcDragSourceView->GetSelectionInfo().m_sSelect.GetFrom(),	// 範囲選択開始
				pcDragSourceView->GetSelectionInfo().m_sSelect.GetTo()		// 範囲選択終了
			);
			++rcSel.bottom;
			if( GetCaret().GetCaretLayoutPos().GetY() >= rcSel.bottom ){
				bMoveToPrev = FALSE;
			}else
			if( GetCaret().GetCaretLayoutPos().GetY() + rcSel.bottom - rcSel.top < rcSel.top ){
				bMoveToPrev = TRUE;
			}else
			if( GetCaret().GetCaretLayoutPos().GetX2() < rcSel.left ){
				bMoveToPrev = TRUE;
			}else{
				bMoveToPrev = FALSE;
			}
		}else{
			if( pcDragSourceView->GetSelectionInfo().m_sSelect.GetFrom().y > GetCaret().GetCaretLayoutPos().GetY() ){
				bMoveToPrev = TRUE;
			}else
			if( pcDragSourceView->GetSelectionInfo().m_sSelect.GetFrom().y == GetCaret().GetCaretLayoutPos().GetY() ){
				if( pcDragSourceView->GetSelectionInfo().m_sSelect.GetFrom().x > GetCaret().GetCaretLayoutPos().GetX2() ){
					bMoveToPrev = TRUE;
				}else{
					bMoveToPrev = FALSE;
				}
			}else{
				bMoveToPrev = FALSE;
			}
		}
	}

	CLayoutPoint ptCaretPos_Old = GetCaret().GetCaretLayoutPos();
	if( !bMove ){
		/* コピーモード */
		/* 現在の選択範囲を非選択状態に戻す */
		GetSelectionInfo().DisableSelectArea( true );
	}else{
		bBeginBoxSelect_Old = pcDragSourceView->GetSelectionInfo().IsBoxSelecting();
		sSelectBgn_Old = pcDragSourceView->GetSelectionInfo().m_sSelectBgn;
		sSelect_Old = pcDragSourceView->GetSelectionInfo().m_sSelect;
		if( bMoveToPrev ){
			/* 移動モード & 前に移動 */
			/* 選択エリアを削除 */
			if( this != pcDragSourceView ){
				pcDragSourceView->GetSelectionInfo().DisableSelectArea( true );
				GetSelectionInfo().DisableSelectArea( true );
				GetSelectionInfo().SetBoxSelect( bBeginBoxSelect_Old );
				GetSelectionInfo().m_sSelectBgn = sSelectBgn_Old;
				GetSelectionInfo().m_sSelect = sSelect_Old;
			}
			DeleteData( true );
			GetCaret().MoveCursor( ptCaretPos_Old, true );
		}else{
			/* 現在の選択範囲を非選択状態に戻す */
			pcDragSourceView->GetSelectionInfo().DisableSelectArea( true );
			if( this != pcDragSourceView )
				GetSelectionInfo().DisableSelectArea( true );
		}
	}
	if( !bBoxData ){	/* 矩形データ */
		//	2004,05.14 Moca 引数に文字列長を追加

		// 挿入前のキャレット位置を記憶する
		// （キャレットが行終端より右の場合は埋め込まれる空白分だけ桁位置をシフト）
		CLogicPoint ptCaretLogicPos_Old = GetCaret().GetCaretLogicPos();
		const CLayout* pcLayout;
		CLogicInt nLineLen;
		CLayoutPoint ptCaretLayoutPos_Old = GetCaret().GetCaretLayoutPos();
		if( m_pcEditDoc->m_cLayoutMgr.GetLineStr( ptCaretLayoutPos_Old.GetY2(), &nLineLen, &pcLayout ) ){
			CLayoutInt nLineAllColLen;
			LineColumnToIndex2( pcLayout, ptCaretLayoutPos_Old.GetX2(), &nLineAllColLen );
			if( nLineAllColLen > CLayoutInt(0) ){	// 行終端より右の場合には nLineAllColLen に行全体の表示桁数が入っている
				ptCaretLogicPos_Old.SetX(
					ptCaretLogicPos_Old.GetX2()
					+ (Int)(ptCaretLayoutPos_Old.GetX2() - nLineAllColLen)
				);
			}
		}

		GetCommander().Command_INSTEXT( true, cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), FALSE );

		// 挿入前のキャレット位置から挿入後のキャレット位置までを選択範囲にする
		CLayoutPoint ptSelectFrom;
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			ptCaretLogicPos_Old,
			&ptSelectFrom
		);
		GetSelectionInfo().SetSelectArea( CLayoutRange(ptSelectFrom, GetCaret().GetCaretLayoutPos()) );	// 2009.07.25 ryoji
	}else{
		// 2004.07.12 Moca クリップボードを書き換えないように
		// TRUE == bBoxSelected
		// FALSE == GetSelectionInfo().IsBoxSelecting()
		/* 貼り付け（クリップボードから貼り付け）*/
		GetCommander().Command_PASTEBOX( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength() );
		AdjustScrollBars(); // 2007.07.22 ryoji
		Redraw();
	}
	if( bMove ){
		if( bMoveToPrev ){
		}else{
			/* 移動モード & 後ろに移動*/

			// 現在の選択範囲を記憶する	// 2008.03.26 ryoji
			CLogicRange sSelLogic;
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				GetSelectionInfo().m_sSelect,
				&sSelLogic
			);

			// 以前の選択範囲を記憶する	// 2008.03.26 ryoji
			CLogicRange sDelLogic;
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				sSelect_Old,
				&sDelLogic
			);

			// 現在の行数を記憶する	// 2008.03.26 ryoji
			int nLines_Old = m_pcEditDoc->m_cDocLineMgr.GetLineCount();

			// 以前の選択範囲を選択する
			GetSelectionInfo().SetBoxSelect( bBeginBoxSelect_Old );
			GetSelectionInfo().m_sSelectBgn = sSelectBgn_Old;
			GetSelectionInfo().m_sSelect = sSelect_Old;

			/* 選択エリアを削除 */
			DeleteData( true );

			// 削除前の選択範囲を復元する	// 2008.03.26 ryoji
			if( !bBoxData ){
				// 削除された範囲を考慮して選択範囲を調整する
				if( sSelLogic.GetFrom().GetY2() == sDelLogic.GetTo().GetY2() ){	// 選択開始が削除末尾と同一行
					sSelLogic.SetFromX(
						sSelLogic.GetFrom().GetX2()
						- (sDelLogic.GetTo().GetX2() - sDelLogic.GetFrom().GetX2())
					);
				}
				if( sSelLogic.GetTo().GetY2() == sDelLogic.GetTo().GetY2() ){	// 選択終了が削除末尾と同一行
					sSelLogic.SetToX(
						sSelLogic.GetTo().GetX2()
						- (sDelLogic.GetTo().GetX2() - sDelLogic.GetFrom().GetX2())
					);
				}
				// Note.
				// (sDelLogic.GetTo().GetY2() - sDelLogic.GetFrom().GetY2()) だと実際の削除行数と同じになる
				// こともあるが、（削除行数－１）になることもある．
				// 例）フリーカーソルでの行番号クリック時の１行選択
				int nLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
				sSelLogic.SetFromY( sSelLogic.GetFrom().GetY2() - (nLines_Old - nLines) );
				sSelLogic.SetToY( sSelLogic.GetTo().GetY2() - (nLines_Old - nLines) );

				// 調整後の選択範囲を設定する
				CLayoutRange sSelect;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
					sSelLogic,
					&sSelect
				);
				GetSelectionInfo().SetSelectArea( sSelect );	// 2009.07.25 ryoji
				ptCaretPos_Old = GetSelectionInfo().m_sSelect.GetTo();
			}

			// キャレットを移動する
			GetCaret().MoveCursor( ptCaretPos_Old, true );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			// 削除位置から移動先へのカーソル移動をアンドゥ操作に追加する	// 2008.03.26 ryoji
			CLogicPoint ptBefore;
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				GetSelectionInfo().m_sSelect.GetFrom(),
				&ptBefore
			);
			m_cCommander.GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					sDelLogic.GetFrom(),
					GetCaret().GetCaretLogicPos()
				)
			);
		}
	}
	GetSelectionInfo().DrawSelectArea();

	/* アンドゥバッファの処理 */
	SetUndoBuffer();

	::GlobalUnlock( hData );
	// 2004.07.12 fotomo/もか メモリーリークの修正
	if( 0 == (GMEM_LOCKCOUNT & ::GlobalFlags(hData)) ){
		::GlobalFree( hData );
	}

	return S_OK;
}


/** 独自ドロップファイルメッセージをポストする
	@date 2008.06.20 ryoji 新規作成
*/
STDMETHODIMP CEditView::PostMyDropFiles( LPDATAOBJECT pDataObject )
{
	HGLOBAL hData = GetGlobalData( pDataObject, CF_HDROP );
	if( hData == NULL )
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock( hData );
	SIZE_T nSize = ::GlobalSize( hData );

	// ドロップデータをコピーしてあとで独自のドロップファイル処理を行う
	HGLOBAL hDrop = ::GlobalAlloc( GHND | GMEM_DDESHARE, nSize );
	memcpy_raw( ::GlobalLock( hDrop ), pData, nSize );
	::GlobalUnlock( hDrop );
	::PostMessageAny(
		GetHwnd(),
		MYWM_DROPFILES,
		(WPARAM)hDrop,
		0
	);

	::GlobalUnlock( hData );
	if( 0 == (GMEM_LOCKCOUNT & ::GlobalFlags( hData )) ){
		::GlobalFree( hData );
	}

	return S_OK;
}

/** 独自ドロップファイルメッセージ処理
	@date 2008.06.20 ryoji 新規作成
*/
void CEditView::OnMyDropFiles( HDROP hDrop )
{
	// 普通にメニュー操作ができるように入力状態をフォアグランドウィンドウにアタッチする
	int nTid2 = ::GetWindowThreadProcessId( ::GetForegroundWindow(), NULL );
	int nTid1 = ::GetCurrentThreadId();
	if( nTid1 != nTid2 ) ::AttachThreadInput( nTid1, nTid2, TRUE );

	// ダミーの STATIC を作ってフォーカスを当てる（エディタが前面に出ないように）
	HWND hwnd = ::CreateWindow(_T("STATIC"), _T(""), 0, 0, 0, 0, 0, NULL, NULL, G_AppInstance(), NULL );
	::SetFocus(hwnd);

	// メニューを作成する
	POINT pt;
	::GetCursorPos( &pt );
	RECT rcWork;
	GetMonitorWorkRect( pt, &rcWork );	// モニタのワークエリア
	HMENU hMenu = ::CreatePopupMenu();
	::InsertMenu( hMenu, 0, MF_BYPOSITION | MF_STRING, 100, LS(STR_VIEW_MOUSE_MENU_PATH) );
	::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_STRING, 101, LS(STR_VIEW_MOUSE_MENU_FILE) );
	::InsertMenu( hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// セパレータ
	::InsertMenu( hMenu, 3, MF_BYPOSITION | MF_STRING, 110, LS(STR_VIEW_MOUSE_MENU_OPEN) );
	::InsertMenu( hMenu, 4, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// セパレータ
	::InsertMenu( hMenu, 5, MF_BYPOSITION | MF_STRING, IDCANCEL, LS(STR_VIEW_MOUSE_MENU_CANCEL) );
	int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									( pt.x > rcWork.left )? pt.x: rcWork.left,
									( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
								0, hwnd, NULL);
	::DestroyMenu( hMenu );

	::DestroyWindow( hwnd );

	// 入力状態をデタッチする
	if( nTid1 != nTid2 ) ::AttachThreadInput( nTid1, nTid2, FALSE );

	// 選択されたメニューに対応する処理を実行する
	switch( nId ){
	case 110:	// ファイルを開く
		// 通常のドロップファイル処理を行う
		::SendMessageAny( m_pcEditWnd->GetHwnd(), WM_DROPFILES, (WPARAM)hDrop, 0 );
		break;

	case 100:	// パス名を貼り付ける
	case 101:	// ファイル名を貼り付ける
		CNativeW cmemBuf;
		UINT nFiles;
		TCHAR szPath[_MAX_PATH];
		TCHAR szExt[_MAX_EXT];
		TCHAR szWork[_MAX_PATH];

		nFiles = ::DragQueryFile( hDrop, 0xFFFFFFFF, NULL, 0 );
		for( UINT i = 0; i < nFiles; i++ ){
			::DragQueryFile( hDrop, i, szPath, sizeof(szPath)/sizeof(TCHAR) );
			if( !::GetLongFileName( szPath, szWork ) )
				continue;
			if( nId == 100 ){	// パス名
				::lstrcpy( szPath, szWork );
			}else if( nId == 101 ){	// ファイル名
				_tsplitpath( szWork, NULL, NULL, szPath, szExt );
				::lstrcat( szPath, szExt );
			}
#ifdef _UNICODE
			cmemBuf.AppendString( szPath );
#else
			cmemBuf.AppendStringOld( szPath );
#endif
			if( nFiles > 1 ){
				cmemBuf.AppendString( m_pcEditDoc->m_cDocEditor.GetNewLineCode().GetValue2() );
			}
		}
		::DragFinish( hDrop );

		// 選択範囲の選択解除
		if( GetSelectionInfo().IsTextSelected() ){
			GetSelectionInfo().DisableSelectArea( true );
		}

		// 挿入前のキャレット位置を記憶する
		// （キャレットが行終端より右の場合は埋め込まれる空白分だけ桁位置をシフト）
		CLogicPoint ptCaretLogicPos_Old = GetCaret().GetCaretLogicPos();
		const CLayout* pcLayout;
		CLogicInt nLineLen;
		CLayoutPoint ptCaretLayoutPos_Old = GetCaret().GetCaretLayoutPos();
		if( m_pcEditDoc->m_cLayoutMgr.GetLineStr( ptCaretLayoutPos_Old.GetY2(), &nLineLen, &pcLayout ) ){
			CLayoutInt nLineAllColLen;
			LineColumnToIndex2( pcLayout, ptCaretLayoutPos_Old.GetX2(), &nLineAllColLen );
			if( nLineAllColLen > CLayoutInt(0) ){	// 行終端より右の場合には nLineAllColLen に行全体の表示桁数が入っている
				ptCaretLogicPos_Old.SetX(
					ptCaretLogicPos_Old.GetX2()
					+ (Int)(ptCaretLayoutPos_Old.GetX2() - nLineAllColLen)
				);
			}
		}

		// テキスト挿入
		GetCommander().HandleCommand( F_INSTEXT_W, true, (LPARAM)cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), TRUE, 0 );

		// 挿入前のキャレット位置から挿入後のキャレット位置までを選択範囲にする
		CLayoutPoint ptSelectFrom;
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			ptCaretLogicPos_Old,
			&ptSelectFrom
		);
		GetSelectionInfo().SetSelectArea( CLayoutRange(ptSelectFrom, GetCaret().GetCaretLayoutPos()) );	// 2009.07.25 ryoji
		GetSelectionInfo().DrawSelectArea();
		break;
	}

	// メモリ解放
	::GlobalFree( hDrop );
}

CLIPFORMAT CEditView::GetAvailableClipFormat( LPDATAOBJECT pDataObject )
{
	CLIPFORMAT cf = 0;
	CLIPFORMAT cfSAKURAClip = CClipboard::GetSakuraFormat();

	if( IsDataAvailable(pDataObject, cfSAKURAClip) )
		cf = cfSAKURAClip;
	else if( IsDataAvailable(pDataObject, CF_UNICODETEXT) )
		cf = CF_UNICODETEXT;
	else if( IsDataAvailable(pDataObject, CF_TEXT) )
		cf = CF_TEXT;
	else if( IsDataAvailable(pDataObject, CF_HDROP) )	// 2008.06.20 ryoji
		cf = CF_HDROP;

	return cf;
}

DWORD CEditView::TranslateDropEffect( CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect )
{
	if( cf == CF_HDROP )	// 2008.06.20 ryoji
		return DROPEFFECT_LINK;

	CEditView* pcDragSourceView = m_pcEditWnd->GetDragSourceView();

	// 2008.06.21 ryoji
	// Win 98/Me 環境では外部からのドラッグ時に GetKeyState() ではキー状態を正しく取得できないため、
	// Drag & Drop インターフェースで渡される dwKeyState を用いて判定する。
#if 1
	// ドラッグ元が外部ウィンドウかどうかによって受け方を変える
	// ※汎用テキストエディタではこちらが主流っぽい
	if( pcDragSourceView ){
#else
	// ドラッグ元が移動を許すかどうかによって受け方を変える
	// ※MS 製品（MS Office, Visual Studioなど）ではこちらが主流っぽい
	if( dwEffect & DROPEFFECT_MOVE ){
#endif
		dwEffect &= (MK_CONTROL & dwKeyState)? DROPEFFECT_COPY: DROPEFFECT_MOVE;
	}else{
		dwEffect &= (MK_SHIFT & dwKeyState)? DROPEFFECT_MOVE: DROPEFFECT_COPY;
	}
	return dwEffect;
}

bool CEditView::IsDragSource( void )
{
	return ( this == m_pcEditWnd->GetDragSourceView() );
}
