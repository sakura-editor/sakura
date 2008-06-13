#include "stdafx.h"
#include "CEditView.h"
#include "util/input.h"
#include "types/CTypeSupport.h"
#include "util/os.h"
#include "parse/CWordParse.h"
#include "COpeBlk.h"
#include "CClipboard.h"
#include "doc/CLayout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      マウスイベント                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* マウス左ボタン押下 */
void CEditView::OnLBUTTONDOWN( WPARAM fwKeys, int _xPos , int _yPos )
{
	CMyPoint ptMouse(_xPos,_yPos);

	if( m_bHokan ){
		m_pcEditDoc->m_pcEditWnd->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

	//isearch 2004.10.22 isearchをキャンセルする
	if (m_nISearchMode > 0 ){
		ISearchExit();
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
		nFuncID = GetDllShareData().m_pKeyNameArr[MOUSEFUNCTION_TRIPLECLICK].m_nFuncCodeArr[getCtrlKeyState()];
		if( 0 == nFuncID ){
			tripleClickMode = 0;	// 割り当て機能無しの時はトリプルクリック OFF
		}
	}else{
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}

	/* 現在のマウスカーソル位置→レイアウト位置 */
	CLayoutPoint ptNew;
	GetTextArea().ClientToLayout(ptMouse, &ptNew);

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
				/* 選択範囲のデータを取得 */
				if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
					DWORD dwEffects;
					DWORD dwEffectsSrc = ( CAppMode::Instance()->IsViewMode() || !m_pcEditDoc->m_cDocLocker.IsDocWritable() )?
											DROPEFFECT_COPY: DROPEFFECT_COPY | DROPEFFECT_MOVE;
					int nOpe = m_pcEditDoc->m_cDocEditor.m_cOpeBuf.GetCurrentPointer();
					int nTickDrag = ::GetTickCount();
					m_pcEditWnd->SetDragSourceView( this );
					CDataObject data( cmemCurText.GetStringPtr(), cmemCurText.GetStringLength(), GetSelectionInfo().IsBoxSelecting() );
					dwEffects = data.DragDrop( TRUE, dwEffectsSrc );
					m_pcEditWnd->SetDragSourceView( NULL );
					if( m_pcEditDoc->m_cDocEditor.m_cOpeBuf.GetCurrentPointer() == nOpe ){	// ドキュメント変更なしか？	// 2007.12.09 ryoji
						m_pcEditWnd->SetActivePane( m_nMyIndex );
						if( ::GetTickCount() - nTickDrag <= ::GetDoubleClickTime() ){	// 短時間ならクリックとみなす
							// クリック位置にカーソル移動する
							if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
								/* 現在の選択範囲を非選択状態に戻す */
								GetSelectionInfo().DisableSelectArea( TRUE );
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
						}else if( DROPEFFECT_MOVE == (dwEffectsSrc & dwEffects) ){
							// 移動範囲を削除する
							// ドロップ先が移動を処理したが自ドキュメントにここまで変更が無い
							// →ドロップ先は外部のウィンドウである
							if( NULL == m_pcOpeBlk ){
								m_pcOpeBlk = new COpeBlk;
							}

							// 選択範囲を削除
							DeleteData( TRUE );

							// アンドゥバッファの処理
							if( NULL != m_pcOpeBlk ){
								if( 0 < m_pcOpeBlk->GetNum() ){
									m_pcEditDoc->m_cDocEditor.m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
									m_pcEditWnd->RedrawAllViews( this );	//	他のペインの表示を更新
								}else{
									delete m_pcOpeBlk;
								}
								m_pcOpeBlk = NULL;
							}
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
			GetSelectionInfo().DisableSelectArea( TRUE );
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
		/*
		m_nMouseRollPosXOld = xPos;		// マウス範囲選択前回位置(X座標)
		m_nMouseRollPosYOld = yPos;		// マウス範囲選択前回位置(Y座標)
		*/

		/* 範囲選択開始 & マウスキャプチャー */
		GetSelectionInfo().SelectBeginBox();

		::SetCapture( GetHwnd() );
		GetCaret().HideCaret_( GetHwnd() ); // 2002/07/22 novice
		/* 現在のカーソル位置から選択を開始する */
		GetSelectionInfo().BeginSelectArea( );
		GetCaret().m_cUnderLine.CaretUnderLineOFF( TRUE );
		GetCaret().m_cUnderLine.Lock();
		if( ptMouse.x < GetTextArea().GetAreaLeft() ){
			/* カーソル下移動 */
			GetCommander().Command_DOWN( TRUE, FALSE );
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


		if(tripleClickMode){		// 2007.11.15 nasukoji	トリプルクリックを処理する
			// 1行選択でない場合は選択文字列を解除
			// トリプルクリックが1行選択でなくてもクアドラプルクリックを有効とする
			if(F_SELECTLINE != nFuncID){
				OnLBUTTONUP( fwKeys, ptMouse.x, ptMouse.y );	// ここで左ボタンアップしたことにする

				if( GetSelectionInfo().IsTextSelected() )		// テキストが選択されているか
					GetSelectionInfo().DisableSelectArea( TRUE );	// 現在の選択範囲を非選択状態に戻す
			}

			// 単語の途中で折り返されていると下の行が選択されてしまうことへの対処
			GetCaret().MoveCursorToClientPoint( ptMouse );	// カーソル移動

			// コマンドコードによる処理振り分け
			// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
			::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ), (LPARAM)NULL );

			// 1行選択でない場合はここで抜ける（他の選択コマンドの時問題となるかも）
			if(F_SELECTLINE != nFuncID)
				return;

			// 選択するものが無い（[EOF]のみの行）時は通常クリックと同じ処理
			if(( ! GetSelectionInfo().IsTextSelected() )&&
			   ( GetCaret().GetCaretLogicPos().y >= m_pcEditDoc->m_cDocLineMgr.GetLineCount() ))
			{
				GetSelectionInfo().BeginSelectArea();				// 現在のカーソル位置から選択を開始する
				GetSelectionInfo().m_bBeginLineSelect = FALSE;		// 行単位選択中 OFF
			}
		}else
		/* 選択開始処理 */
		/* SHIFTキーが押されていたか */
		if(GetKeyState_Shift()){
			if( GetSelectionInfo().IsTextSelected() ){		/* テキストが選択されているか */
				if( GetSelectionInfo().IsBoxSelecting() ){	/* 矩形範囲選択中 */
					/* 現在の選択範囲を非選択状態に戻す */
					GetSelectionInfo().DisableSelectArea( TRUE );

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
					GetCaret().MoveCursorToClientPoint( ptMouse );
				}
				else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
					GetCaret().MoveCursorToClientPoint( CMyPoint(GetTextArea().GetDocumentLeftClientPointX(), ptMouse.y) );
				}
			}
		}
		else{
			if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在の選択範囲を非選択状態に戻す */
				GetSelectionInfo().DisableSelectArea( TRUE );
			}
			/* カーソル移動 */
			if( ptMouse.y >= GetTextArea().GetAreaTop() && ptMouse.y < GetTextArea().GetAreaBottom() ){
				if( ptMouse.x >= GetTextArea().GetAreaLeft() && ptMouse.x < GetTextArea().GetAreaRight() ){
					GetCaret().MoveCursorToClientPoint( ptMouse );
				}
				else if( ptMouse.x < GetTextArea().GetAreaLeft() ){
					GetCaret().MoveCursorToClientPoint( CMyPoint(GetTextArea().GetDocumentLeftClientPointX(), ptMouse.y) );
				}
			}
			/* 現在のカーソル位置から選択を開始する */
			GetSelectionInfo().BeginSelectArea( );
		}


		/******* この時点で必ず true == GetSelectionInfo().IsTextSelected() の状態になる ****:*/
		if( !GetSelectionInfo().IsTextSelected() ){
			WarningMessage( GetHwnd(), _T("バグってる") );
			return;
		}

		int	nWorkRel;
		nWorkRel = IsCurrentPositionSelected(
			GetCaret().GetCaretLayoutPos()	// カーソル位置
		);


		/* 現在のカーソル位置によって選択範囲を変更 */
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );


		// CTRLキーが押されている、かつトリプルクリックでない		// 2007.11.15 nasukoji	トリプルクリック対応
		if( GetKeyState_Control() &&( ! tripleClickMode)){
			GetSelectionInfo().m_bBeginWordSelect = TRUE;		/* 単語単位選択中 */
			if( !GetSelectionInfo().IsTextSelected() ){
				/* 現在位置の単語選択 */
				if ( GetCommander().Command_SELECTWORD() ){
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
					nIdx = LineColmnToIndex( pcLayout, GetSelectionInfo().m_sSelect.GetFrom().GetX2() );
					/* 現在位置の単語の範囲を調べる */
					int nWhareResult = m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						GetSelectionInfo().m_sSelect.GetFrom().GetY2(),
						nIdx,
						&sRange,
						NULL,
						NULL
					);
					if( nWhareResult ){
						// 指定された行のデータ内の位置に対応する桁の位置を調べる。
						// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
						/*
						pLine            = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetFrom().GetY2(), &nLineLen, &pcLayout );
						sRange.SetFromX( LineIndexToColmn( pcLayout, sRange.GetFrom().x ) );
						pLine            = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetTo().GetY2(), &nLineLen, &pcLayout );
						sRange.SetToX( LineIndexToColmn( pcLayout, sRange.GetTo().x ) );
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
					nIdx = LineColmnToIndex( pcLayout, GetSelectionInfo().m_sSelect.GetTo().GetX2() );
					/* 現在位置の単語の範囲を調べる */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						GetSelectionInfo().m_sSelect.GetTo().GetY2(), nIdx, &sRange, NULL, NULL )
					){
						// 指定された行のデータ内の位置に対応する桁の位置を調べる
						// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
						/*
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetFrom().GetY2(), &nLineLen, &pcLayout );
						sRange.SetFromX( LineIndexToColmn( pcLayout, sRange.GetFrom().x ) );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetTo().GetY2(), &nLineLen, &pcLayout );
						sRange.SetToX( LineIndexToColmn( pcLayout, sRange.GetTo().x ) );
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
			GetSelectionInfo().m_bBeginLineSelect = TRUE;

			// 2002.10.07 YAZAKI 折り返し行をインデントしているときに選択がおかしいバグの対策
			// １行が画面幅よりも長いと左右にスクロールしてちらつきが激しくなるので後で全体を再描画	// 2008.05.20 ryoji
			bool bDrawSwitchOld = GetDrawSwitch();
			bool bDrawAfter = false;
			if( bDrawSwitchOld ){
				const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
				if( pcLayout ){
					CLayoutInt nColumn = LineIndexToColmn( pcLayout, CLogicInt(pcLayout->GetLengthWithoutEOL()) );
					bDrawAfter = (nColumn + CLayoutInt(SCROLLMARGIN_RIGHT) >= GetTextArea().m_nViewColNum);
					if( bDrawAfter ){
						SetDrawSwitch( false );
					}
				}
			}
			GetCommander().Command_GOLINEEND( TRUE, FALSE );
			GetCommander().Command_RIGHT( true, false, false );
			if( bDrawSwitchOld && bDrawAfter ){
				SetDrawSwitch( true );
				Redraw();
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
			//	Sep. 7, 2003 genta URLの強調表示OFFの時はURLは普通の文字として扱う
			if( CTypeSupport(this,COLORIDX_URL).IsDisp() &&
				TRUE == GetDllShareData().m_Common.m_sEdit.m_bSelectClickedURL ){

				CLogicRange cUrlRange;	//URL範囲
				// カーソル位置にURLが有る場合のその範囲を調べる
				bool bIsUrl = IsCurrentPositionURL(
					GetCaret().GetCaretLayoutPos(),	// カーソル位置
					&cUrlRange,						// URL範囲
					NULL							// URL受け取り先
				);
				if( bIsUrl ){
					/* 現在の選択範囲を非選択状態に戻す */
					GetSelectionInfo().DisableSelectArea( TRUE );

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
	nFuncID = GetDllShareData().m_pKeyNameArr[MOUSEFUNCTION_RIGHT].m_nFuncCodeArr[nIdx];
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
	マウス中ボタンを押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.11 novice 新規作成
*/
void CEditView::OnMBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス左サイドボタンに対応する機能コードはm_Common.m_pKeyNameArr[2]に入っている */
	nFuncID = GetDllShareData().m_pKeyNameArr[MOUSEFUNCTION_CENTER].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
}


// novice 2004/10/10 マウスサイドボタン対応
/*!
	マウス左サイドボタンを押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.10 novice 新規作成
	@date 2004.10.11 novice マウス中ボタン対応のため変更
*/
void CEditView::OnXLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス左サイドボタンに対応する機能コードはm_Common.m_pKeyNameArr[3]に入っている */
	nFuncID = GetDllShareData().m_pKeyNameArr[MOUSEFUNCTION_LEFTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}


/*!
	マウス右サイドボタン押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.10 novice 新規作成
	@date 2004.10.11 novice マウス中ボタン対応のため変更
*/
void CEditView::OnXRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス右サイドボタンに対応する機能コードはm_Common.m_pKeyNameArr[4]に入っている */
	nFuncID = GetDllShareData().m_pKeyNameArr[MOUSEFUNCTION_RIGHTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessageCmd( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}

/* マウス移動のメッセージ処理 */
void CEditView::OnMOUSEMOVE( WPARAM fwKeys, int _xPos , int _yPos )
{
	CMyPoint ptMouse(_xPos,_yPos);

	CLayoutInt	nScrollRowNum;
	POINT		po;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;

	CLogicInt	nIdx;
	int			nWorkF;
	int			nWorkT;

	CLayoutRange sRange;
	CLayoutRange sSelectBgn_Old; // 範囲選択(原点)
	CLayoutRange sSelect_Old;
	CLayoutRange sSelect;

	sSelectBgn_Old = GetSelectionInfo().m_sSelectBgn;
	sSelect_Old    = GetSelectionInfo().m_sSelect;

	if( !GetSelectionInfo().IsMouseSelecting() ){	/* 範囲選択中 */
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
				nScrollRowNum = GetCaret().MoveCursorToClientPoint( ptMouse );
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
			//	Sep. 7, 2003 genta URLの強調表示OFFの時はURLチェックも行わない
			else if( CTypeSupport(this,COLORIDX_URL).IsDisp() &&
				IsCurrentPositionURL(
					ptNew,			// カーソル位置
					&cUrlRange,		// URL範囲
					NULL			// URL受け取り先
				)
			){
				/* 手カーソル */
				::SetCursor( ::LoadCursor( G_AppInstance(), MAKEINTRESOURCE( IDC_CURSOR_HAND ) ) );
			}else{
				//migemo isearch 2004.10.22
				if( m_nISearchMode > 0 ){
					if (m_nISearchDirection == 1){
						::SetCursor( ::LoadCursor( G_AppInstance(),MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
					}else{
						::SetCursor( ::LoadCursor( G_AppInstance(),MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
					}
				}else
				/* アイビーム */
				::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
			}
		}
		return;
	}
	::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
	if( GetSelectionInfo().IsBoxSelecting() ){	/* 矩形範囲選択中 */
		/* 座標指定によるカーソル移動 */
		nScrollRowNum = GetCaret().MoveCursorToClientPoint( ptMouse );
		/* 現在のカーソル位置によって選択範囲を変更 */
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)
	}
	else{
		/* 座標指定によるカーソル移動 */
		if(( ptMouse.x < GetTextArea().GetAreaLeft() || m_dwTripleClickCheck )&& GetSelectionInfo().m_bBeginLineSelect ){	// 行単位選択中
			// 2007.11.15 nasukoji	上方向の行選択時もマウスカーソルの位置の行が選択されるようにする
			CMyPoint nNewPos(0, ptMouse.y);

			// 1行の高さ
			int nLineHeight = GetTextMetrics().GetHankakuHeight() + m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nLineSpace;

			// 選択開始行以下へのドラッグ時は1行下にカーソルを移動する
			if( GetTextArea().GetViewTopLine() + (ptMouse.y - GetTextArea().GetAreaTop()) / nLineHeight >= GetSelectionInfo().m_sSelectBgn.GetTo().y)
				nNewPos.y += nLineHeight;

			// カーソルを移動
			nNewPos.x = GetTextArea().GetAreaLeft() - Int(GetTextArea().GetViewLeftCol()) * ( GetTextMetrics().GetHankakuWidth() + m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nColmSpace );
			nScrollRowNum = GetCaret().MoveCursorToClientPoint( nNewPos );

			// 2.5クリックによる行単位のドラッグ
			if( m_dwTripleClickCheck ){
				// 選択開始行以上にドラッグした
				if( GetCaret().GetCaretLayoutPos().GetY() <= GetSelectionInfo().m_sSelectBgn.GetTo().y ){
					GetCommander().Command_GOLINETOP( TRUE, 0x09 );		// 改行単位の行頭へ移動
				}else{
					CLayoutPoint ptCaret;

					CLogicPoint ptCaretPrevLog(0, GetCaret().GetCaretLogicPos().y);

					// 選択開始行より下にカーソルがある時は1行前と物理行番号の違いをチェックする
					// 選択開始行にカーソルがある時はチェック不要
					if( GetCaret().GetCaretLayoutPos().GetY() > GetSelectionInfo().m_sSelectBgn.GetTo().y ){
						// 1行前の物理行を取得する
						m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( CLayoutPoint(CLayoutInt(0), GetCaret().GetCaretLayoutPos().GetY() - 1), &ptCaretPrevLog );
					}

					// 前の行と同じ物理行
					if( ptCaretPrevLog.y == GetCaret().GetCaretLogicPos().y ){
						// 1行先の物理行からレイアウト行を求める
						m_pcEditDoc->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, GetCaret().GetCaretLogicPos().y + 1), &ptCaret );

						// カーソルを次の物理行頭へ移動する
						nScrollRowNum = GetCaret().MoveCursor( ptCaret, TRUE );
					}
				}
			}
		}else{
			nScrollRowNum = GetCaret().MoveCursorToClientPoint( ptMouse );
		}
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)

		/* CTRLキーが押されていたか */
//		if( GetKeyState_Control() ){
		if( !GetSelectionInfo().m_bBeginWordSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
		}else{
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
				return;
			}
			const CLayout* pcLayout;
			if( NULL != ( pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout ) ) ){
				nIdx = LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
				/* 現在位置の単語の範囲を調べる */
				int nResult=m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
					GetCaret().GetCaretLayoutPos().GetY2(),
					nIdx,
					&sRange,
					NULL,
					NULL
				);
				if( nResult ){
					// 指定された行のデータ内の位置に対応する桁の位置を調べる
					// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
					/*
					pLine     = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetFrom().GetY2(), &nLineLen, &pcLayout );
					sRange.SetFromX( LineIndexToColmn( pcLayout, sRange.GetFrom().x ) );
					pLine     = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.GetTo().GetY2(), &nLineLen, &pcLayout );
					sRange.SetToX( LineIndexToColmn( pcLayout, sRange.GetTo().x ) );
					*/

					nWorkF = IsCurrentPositionSelectedTEST(
						sRange.GetFrom(), //カーソル位置
						sSelect
					);
					nWorkT = IsCurrentPositionSelectedTEST(
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
		}
	}
	return;
}
//m_dwTipTimerm_dwTipTimerm_dwTipTimer




/* マウスホイールのメッセージ処理 */
LRESULT CEditView::OnMOUSEWHEEL( WPARAM wParam, LPARAM lParam )
{
	WORD	fwKeys;
	short	zDelta;
	short	xPos;
	short	yPos;
	int		i;
	int		nScrollCode;
	int		nRollLineNum;

	fwKeys = LOWORD(wParam);			// key flags
	zDelta = (short) HIWORD(wParam);	// wheel rotation
	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
	yPos = (short) HIWORD(lParam);		// vertical position of pointer
//	MYTRACE_A( "CEditView::DispatchEvent() WM_MOUSEWHEEL fwKeys=%xh zDelta=%d xPos=%d yPos=%d \n", fwKeys, zDelta, xPos, yPos );

	if( 0 < zDelta ){
		nScrollCode = SB_LINEUP;
	}else{
		nScrollCode = SB_LINEDOWN;
	}

	/* マウスホイールによるスクロール行数をレジストリから取得 */
	nRollLineNum = 6;

	/* レジストリの存在チェック */
	// 2006.06.03 Moca ReadRegistry に書き換え
	unsigned int uDataLen;	// size of value data
	TCHAR szValStr[256];
	uDataLen = _countof(szValStr) - 1;
	if( ReadRegistry( HKEY_CURRENT_USER, _T("Control Panel\\desktop"), _T("WheelScrollLines"), szValStr, uDataLen ) ){
		nRollLineNum = ::_ttoi( szValStr );
	}

	if( -1 == nRollLineNum ){/* 「1画面分スクロールする」 */
		nRollLineNum = (Int)GetTextArea().m_nViewRowNum;	// 表示域の行数
	}
	else{
		if( nRollLineNum < 1 ){
			nRollLineNum = 1;
		}
		if( nRollLineNum > 30 ){	//@@@ YAZAKI 2001.12.31 10→30へ。
			nRollLineNum = 30;
		}
	}
	for( i = 0; i < nRollLineNum; ++i ){
		//	Sep. 11, 2004 genta 同期スクロール行数
		CLayoutInt line;

		if( nScrollCode == SB_LINEUP ){
			line = ScrollAtV( GetTextArea().GetViewTopLine() - CLayoutInt(1) );
		}else{
			line = ScrollAtV( GetTextArea().GetViewTopLine() + CLayoutInt(1) );
		}
		SyncScrollV( line );
	}
	return 0;
}






/* マウス左ボタン開放のメッセージ処理 */
void CEditView::OnLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
//	MYTRACE_A( "OnLBUTTONUP()\n" );
	CMemory		cmemBuf, cmemClip;

	/* 範囲選択終了 & マウスキャプチャーおわり */
	if( GetSelectionInfo().IsMouseSelecting() ){	/* 範囲選択中 */
		/* マウス キャプチャを解放 */
		::ReleaseCapture();
		GetCaret().ShowCaret_( GetHwnd() ); // 2002/07/22 novice

		GetSelectionInfo().SelectEnd();

		if( GetSelectionInfo().m_sSelect.IsOne() ){
			/* 現在の選択範囲を非選択状態に戻す */
			GetSelectionInfo().DisableSelectArea( TRUE );

			// 対括弧の強調表示	// 2007.10.18 ryoji
			DrawBracketPair( false );
			SetBracketPairPos( true );
			DrawBracketPair( true );
		}
		GetCaret().m_cUnderLine.UnLock();
	}
	return;
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
		//	Sep. 7, 2003 genta URLの強調表示OFFの時はURLチェックも行わない
		if( CTypeSupport(this,COLORIDX_URL).IsDisp()
			&&
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
			::ShellExecute( NULL, _T("open"), to_tchar(wstrOPEN.c_str()), NULL, NULL, SW_SHOW );
			return;
		}

		/* GREP出力モードまたはデバッグモード かつ マウス左ボタンダブルクリックでタグジャンプ の場合 */
		//	2004.09.20 naoh 外部コマンドの出力からTagjumpできるように
		if( (CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode || CAppMode::Instance()->IsDebugMode()) && GetDllShareData().m_Common.m_sSearch.m_bGTJW_LDBLCLK ){
			/* タグジャンプ機能 */
			GetCommander().Command_TAGJUMP();
			return;
		}
	}

// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	int	nIdx = getCtrlKeyState();

	/* マウス左クリックに対応する機能コードはm_Common.m_pKeyNameArr[?]に入っている 2007.11.15 nasukoji */
	EFunctionCode	nFuncID = GetDllShareData().m_pKeyNameArr[
		m_dwTripleClickCheck ? MOUSEFUNCTION_QUADCLICK : MOUSEFUNCTION_DOUBLECLICK
		].m_nFuncCodeArr[nIdx];
	if(m_dwTripleClickCheck){
		// 非選択状態にした後左クリックしたことにする
		// すべて選択の場合は、3.5クリック時の選択状態保持とドラッグ開始時の
		// 範囲変更のため。
		// クアドラプルクリック機能が割り当てられていない場合は、ダブルクリック
		// として処理するため。
		if( GetSelectionInfo().IsTextSelected() )		// テキストが選択されているか
			GetSelectionInfo().DisableSelectArea( TRUE );		// 現在の選択範囲を非選択状態に戻す

		if(! nFuncID){
			m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
			nFuncID = GetDllShareData().m_pKeyNameArr[MOUSEFUNCTION_DOUBLECLICK].m_nFuncCodeArr[nIdx];
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
		キー割り当ての変更により、ダブルクリック≠単語選択のときには GetSelectionInfo().m_bBeginWordSelect = TRUE
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
#ifdef _DEBUG
	MYTRACE_A( "CEditView::DragEnter()\n" );
#endif
	//「OLEによるドラッグ & ドロップを使う」オプションが無効の場合にはドロップを受け付けない
	if(!GetDllShareData().m_Common.m_sEdit.m_bUseOLE_DragDrop)return E_UNEXPECTED;

	//ビューモードの場合はドロップを受け付けない
	if(CAppMode::Instance()->IsViewMode())return E_UNEXPECTED;

	//上書き禁止の場合はドロップを受け付けない
	if(!m_pcEditDoc->m_cDocLocker.IsDocWritable())return E_UNEXPECTED;


	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;

	CLIPFORMAT cf;
	cf = GetAvailableClipFormat( pDataObject );
	if( cf == 0 )
		return E_INVALIDARG;

	/* 自分をアクティブペインにする */
	m_pcEditWnd->SetActivePane( m_nMyIndex );

	// 現在のカーソル位置を記憶する	// 2007.12.09 ryoji
	m_ptCaretPos_DragEnter = GetCaret().GetCaretLayoutPos();
	m_nCaretPosX_Prev_DragEnter = GetCaret().m_nCaretPosX_Prev;

	// ドラッグデータは矩形か
	m_bDragBoxData = IsDataAvailable( pDataObject, ::RegisterClipboardFormat( _T("MSDEVColumnSelect") ) );

	/* 選択テキストのドラッグ中か */
	_SetDragMode( TRUE );

	DragOver( dwKeyState, pt, pdwEffect );
	return S_OK;
}

STDMETHODIMP CEditView::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE_A( "CEditView::DragOver()\n" );
#endif

	/* マウス移動のメッセージ処理 */
	::ScreenToClient( GetHwnd(), (LPPOINT)&pt );
	OnMOUSEMOVE( dwKeyState, pt.x , pt.y );

	if ( pdwEffect == NULL )
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect( dwKeyState, pt, *pdwEffect );

	return S_OK;
}

STDMETHODIMP CEditView::DragLeave( void )
{
#ifdef _DEBUG
	MYTRACE_A( "CEditView::DragLeave()\n" );
#endif
	/* 選択テキストのドラッグ中か */
	_SetDragMode( FALSE );

	// DragEnter時のカーソル位置を復元	// 2007.12.09 ryoji
	// ※範囲選択中のときに選択範囲とカーソルが分離すると変だから
	GetCaret().MoveCursor( m_ptCaretPos_DragEnter, FALSE );
	GetCaret().m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
	RedrawAll();

	// 非アクティブ時は表示状態を非アクティブに戻す	// 2007.12.09 ryoji
	if( ::GetActiveWindow() == NULL )
		OnKillFocus();

	return S_OK;
}

STDMETHODIMP CEditView::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE_A( "CEditView::Drop()\n" );
#endif
	BOOL		bBoxData;
	BOOL		bMove;
	BOOL		bMoveToPrev;
	RECT		rcSel;
	CNativeW	cmemBuf;
	bool		bBeginBoxSelect_Old;

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

	*pdwEffect = TranslateDropEffect( dwKeyState, pt, *pdwEffect );
	if( *pdwEffect == DROPEFFECT_NONE )
		return E_INVALIDARG;

	// 外部からのドロップは以後の処理ではコピーと同様に扱う
	CEditView* pcDragSourceView = m_pcEditWnd->GetDragSourceView();
	bMove = (*pdwEffect == DROPEFFECT_MOVE) && pcDragSourceView;
	bBoxData = m_bDragBoxData;

	// ドロップデータの取得
	HGLOBAL hData = GetGlobalData( pDataObject, cf );
	if (hData == NULL)
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock( hData );
	SIZE_T nSize = ::GlobalSize( hData );
	if( cf == CClipboard::GetSakuraFormat() ){
		wchar_t* pszData = (wchar_t*)((BYTE*)pData + sizeof(int));
		cmemBuf.SetString( pszData, wcsnlen( pszData, *(int*)pData ) );
	}else if( cf == CF_UNICODETEXT ){
		cmemBuf.SetString( (wchar_t*)pData, wcsnlen( (wchar_t*)pData, nSize / sizeof(wchar_t) ) );
	}else{
		cmemBuf.SetStringOld( (char*)pData, strnlen( (char*)pData, nSize / sizeof(char) ) );
	}

	// アンドゥバッファの準備
	if( NULL == m_pcOpeBlk ){
		m_pcOpeBlk = new COpeBlk;
	}

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
		GetSelectionInfo().DisableSelectArea( TRUE );
	}else{
		bBeginBoxSelect_Old = pcDragSourceView->GetSelectionInfo().IsBoxSelecting();
		sSelectBgn_Old = pcDragSourceView->GetSelectionInfo().m_sSelectBgn;
		sSelect_Old = pcDragSourceView->GetSelectionInfo().m_sSelect;
		if( bMoveToPrev ){
			/* 移動モード & 前に移動 */
			/* 選択エリアを削除 */
			if( this != pcDragSourceView ){
				pcDragSourceView->GetSelectionInfo().DisableSelectArea( TRUE );
				GetSelectionInfo().DisableSelectArea( TRUE );
				GetSelectionInfo().SetBoxSelect( bBeginBoxSelect_Old );
				GetSelectionInfo().m_sSelectBgn = sSelectBgn_Old;
				GetSelectionInfo().m_sSelect = sSelect_Old;
			}
			DeleteData( TRUE );
			GetCaret().MoveCursor( ptCaretPos_Old, TRUE );
		}else{
			/* 現在の選択範囲を非選択状態に戻す */
			pcDragSourceView->GetSelectionInfo().DisableSelectArea( TRUE );
			if( this != pcDragSourceView )
				GetSelectionInfo().DisableSelectArea( TRUE );
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
			LineColmnToIndex2( pcLayout, ptCaretLayoutPos_Old.GetX2(), &nLineAllColLen );
			if( nLineAllColLen > CLayoutInt(0) ){	// 行終端より右の場合には nLineAllColLen に行全体の表示桁数が入っている
				ptCaretLogicPos_Old.SetX(
					ptCaretLogicPos_Old.GetX2()
					+ (Int)(ptCaretLayoutPos_Old.GetX2() - nLineAllColLen)
				);
			}
		}

		GetCommander().Command_INSTEXT( TRUE, cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), FALSE );

		// 挿入前のキャレット位置から挿入後のキャレット位置までを選択範囲にする
		CLayoutPoint ptSelectFrom;
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			ptCaretLogicPos_Old,
			&ptSelectFrom
		);
		GetSelectionInfo().m_sSelect.SetFrom( ptSelectFrom );
		GetSelectionInfo().m_sSelect.SetTo( GetCaret().GetCaretLayoutPos() );
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
			DeleteData( TRUE );

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
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
					sSelLogic,
					&GetSelectionInfo().m_sSelect
				);
				ptCaretPos_Old = GetSelectionInfo().m_sSelect.GetTo();
			}

			// キャレットを移動する
			GetCaret().MoveCursor( ptCaretPos_Old, TRUE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			// 削除位置から移動先へのカーソル移動をアンドゥ操作に追加する	// 2008.03.26 ryoji
			CLogicPoint ptBefore;
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				GetSelectionInfo().m_sSelect.GetFrom(),
				&ptBefore
			);
			m_pcOpeBlk->AppendOpe(
				new CMoveCaretOpe(
					sDelLogic.GetFrom(),
					GetCaret().GetCaretLogicPos()
				)
			);
		}
	}
	GetSelectionInfo().DrawSelectArea();

	/* アンドゥバッファの処理 */
	if( NULL != m_pcOpeBlk ){
		if( 0 < m_pcOpeBlk->GetNum() ){	/* 操作の数を返す */
			/* 操作の追加 */
			m_pcEditDoc->m_cDocEditor.m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
			m_pcEditWnd->RedrawAllViews( this );	// 他のペインの表示を更新	// 2007.07.22 ryoji
		}else{
			delete m_pcOpeBlk;
		}
		m_pcOpeBlk = NULL;
	}

	::GlobalUnlock( hData );
	// 2004.07.12 fotomo/もか メモリーリークの修正
	if( 0 == (GMEM_LOCKCOUNT & ::GlobalFlags(hData)) ){
		::GlobalFree( hData );
	}

	return S_OK;
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

	return cf;
}

DWORD CEditView::TranslateDropEffect( DWORD dwKeyState, POINTL pt, DWORD dwEffect )
{
	CEditView* pcDragSourceView = m_pcEditWnd->GetDragSourceView();

	/* このビューのカーソルがドラッグ元の選択範囲内にあるか */
	if( pcDragSourceView &&
		!pcDragSourceView->IsCurrentPositionSelected( GetCaret().GetCaretLayoutPos() )
	){
		return DROPEFFECT_NONE;
	};

#if 1
	// ドラッグ元が外部ウィンドウかどうかによって受け方を変える
	// ※汎用テキストエディタではこちらが主流っぽい
	if( pcDragSourceView ){
#else
	// ドラッグ元が移動を許すかどうかによって受け方を変える
	// ※MS 製品（MS Office, Visual Studioなど）ではこちらが主流っぽい
	if( dwEffect & DROPEFFECT_MOVE ){
#endif
		dwEffect &= GetKeyState_Control()? DROPEFFECT_COPY: DROPEFFECT_MOVE;
	}else{
		dwEffect &= GetKeyState_Shift()? DROPEFFECT_MOVE: DROPEFFECT_COPY;
	}
	return dwEffect;
}

bool CEditView::IsDragSource( void )
{
	return ( this == m_pcEditWnd->GetDragSourceView() );
}
