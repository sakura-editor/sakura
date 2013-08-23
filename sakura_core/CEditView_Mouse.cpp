/*!	@file
	@brief 文書ウィンドウの管理

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date   2005/09/02 D.S.Koba GetSizeOfCharで書き換え
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, かろと, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta, maru
	Copyright (C) 2007, ryoji, じゅうじ, maru, genta, Moca, nasukoji, D.S.Koba
	Copyright (C) 2008, ryoji, nasukoji, bosagami, Moca, genta
	Copyright (C) 2009, nasukoji, ryoji, syat
	Copyright (C) 2010, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <process.h> // _beginthreadex
#include "CEditView.h"
#include "Debug.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "CEditWnd.h"
#include "os.h"
#include "CLayout.h"/// 2002/2/3 aroka
#include "COpeBlk.h"///
#include "my_icmp.h" // 2002/11/30 Moca 追加

/* マウス左ボタン押下 */
void CEditView::OnLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{

	if( m_bHokan ){
		m_pcEditDoc->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

	//isearch 2004.10.22 isearchをキャンセルする
	if (m_nISearchMode > 0 ){
		ISearchExit();
	}

	int			nCaretPosY_Old;
	CMemory		cmemCurText;
	const char*	pLine;
	int			nLineLen;

	CLayoutRange sRange;

	int			nIdx;

	int			nWork;
	BOOL		tripleClickMode = FALSE;	// 2007.10.02 nasukoji	トリプルクリックであることを示す
	int			nFuncID = 0;				// 2007.11.30 nasukoji	マウス左クリックに対応する機能コード

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() == 0 ){
		return;
	}
	if( m_nCaretWidth == 0 ){ //フォーカスがないとき
		return;
	}
	nCaretPosY_Old = m_ptCaretPos.y;

	/* 辞書Tipが起動されている */
	if( 0 == m_dwTipTimer ){
		/* 辞書Tipを消す */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
	}
	else{
		m_dwTipTimer = ::GetTickCount();		/* 辞書Tip起動タイマー */
	}

	// 2007.11.30 nasukoji	トリプルクリックをチェック
	tripleClickMode = CheckTripleClick(xPos, yPos);

	if(tripleClickMode){
		// マウス左トリプルクリックに対応する機能コードはm_Common.m_sKeyBind.m_pKeyNameArr[5]に入っている
		nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_TRIPLECLICK].m_nFuncCodeArr[getCtrlKeyState()];
		if( 0 == nFuncID ){
			tripleClickMode = 0;	// 割り当て機能無しの時はトリプルクリック OFF
		}
	}else{
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}

	/* 現在のマウスカーソル位置→レイアウト位置 */
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );

	// OLEによるドラッグ & ドロップを使う
	// 2007.11.30 nasukoji	トリプルクリック時はドラッグを開始しない
	if( !tripleClickMode && TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop ){
		if( m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource ){		/* OLEによるドラッグ元にするか */
			/* 行選択エリアをドラッグした */
			if( xPos < m_nViewAlignLeft - ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) ){
				goto normal_action;
			}
			/* 指定カーソル位置が選択エリア内にあるか */
			if( 0 == IsCurrentPositionSelected(
				nNewX,		// カーソル位置X
				nNewY		// カーソル位置Y
				)
			){
				POINT ptWk = {xPos, yPos};
				::ClientToScreen(m_hWnd, &ptWk);
				if( !::DragDetect(m_hWnd, ptWk) ){
					// ドラッグ開始条件を満たさなかったのでクリック位置にカーソル移動する
					if( IsTextSelected() ){	/* テキストが選択されているか */
						/* 現在の選択範囲を非選択状態に戻す */
						DisableSelectArea( true );
					}
//@@@ 2002.01.08 YAZAKI フリーカーソルOFFで複数行選択し、行の後ろをクリックするとそこにキャレットが置かれてしまうバグ修正
					/* カーソル移動。 */
					if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
						if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
							MoveCursorToPoint( xPos, yPos );
						}else
						if( xPos < m_nViewAlignLeft ){
							MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
						}
					}
					return;
				}
				/* 選択範囲のデータを取得 */
				if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
					DWORD dwEffects;
					DWORD dwEffectsSrc = (
							m_pcEditDoc->IsReadOnly()	// 読み取り専用
							|| ( SHAREMODE_NOT_EXCLUSIVE != m_pcEditDoc->m_nFileShareModeOld && INVALID_HANDLE_VALUE == m_pcEditDoc->m_hLockedFile )	// 上書き禁止
						)? DROPEFFECT_COPY: DROPEFFECT_COPY | DROPEFFECT_MOVE;
					int nOpe = m_pcEditDoc->m_cOpeBuf.GetCurrentPointer();
					m_pcEditDoc->SetDragSourceView( this );
					CDataObject data( cmemCurText.GetStringPtr(), cmemCurText.GetStringLength(), m_bBeginBoxSelect );	// 2008.03.26 ryoji テキスト長、矩形の指定を追加
					dwEffects = data.DragDrop( TRUE, dwEffectsSrc );
					m_pcEditDoc->SetDragSourceView( NULL );
//					MYTRACE( _T("dwEffects=%d\n"), dwEffects );
					if( m_pcEditDoc->m_cOpeBuf.GetCurrentPointer() == nOpe ){	// ドキュメント変更なしか？	// 2007.12.09 ryoji
						m_pcEditDoc->SetActivePane( m_nMyIndex );
						if( DROPEFFECT_MOVE == (dwEffectsSrc & dwEffects) ){
							// 移動範囲を削除する
							// ドロップ先が移動を処理したが自ドキュメントにここまで変更が無い
							// →ドロップ先は外部のウィンドウである
							if( NULL == m_pcOpeBlk ){
								m_pcOpeBlk = new COpeBlk;
							}

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

	// ALTキーが押されている、かつトリプルクリックでない		// 2007.10.10 nasukoji	トリプルクリック対応
	if( GetKeyState_Alt() &&( ! tripleClickMode)){
		if( IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( true );
		}
		if( yPos >= m_nViewAlignTop  && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
				MoveCursorToPoint( xPos, yPos );
			}else
			if( xPos < m_nViewAlignLeft ){
				MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
			}else{
				return;
			}
		}
		m_nMouseRollPosXOld = xPos;		// マウス範囲選択前回位置(X座標)
		m_nMouseRollPosYOld = yPos;		// マウス範囲選択前回位置(Y座標)
		/* 範囲選択開始 & マウスキャプチャー */
		m_bBeginSelect = true;			/* 範囲選択中 */
		m_bBeginBoxSelect = true;		/* 矩形範囲選択中 */
		m_bBeginLineSelect = false;		/* 行単位選択中 */
		m_bBeginWordSelect = false;		/* 単語単位選択中 */

		::SetCapture( m_hWnd );
		HideCaret_( m_hWnd ); // 2002/07/22 novice
		/* 現在のカーソル位置から選択を開始する */
		BeginSelectArea( );
		m_cUnderLine.CaretUnderLineOFF( true );
		m_cUnderLine.Lock();
		if( xPos < m_nViewAlignLeft ){
			/* カーソル下移動 */
			Command_DOWN( true, false );
		}
	}
	else{
		/* カーソル移動 */
		if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
			}else
			if( xPos < m_nViewAlignLeft ){
			}else{
				return;
			}
		}
		else if( yPos < m_nViewAlignTop ){
			//	ルーラクリック
			return;
		}
		else {
			return;
		}

		/* マウスのキャプチャなど */
		m_nMouseRollPosXOld = xPos;		/* マウス範囲選択前回位置(X座標) */
		m_nMouseRollPosYOld = yPos;		/* マウス範囲選択前回位置(Y座標) */
		/* 範囲選択開始 & マウスキャプチャー */
		m_bBeginSelect = true;			/* 範囲選択中 */
		m_bBeginLineSelect = false;		/* 行単位選択中 */
		m_bBeginWordSelect = false;		/* 単語単位選択中 */
		::SetCapture( m_hWnd );
		HideCaret_( m_hWnd ); // 2002/07/22 novice


		if(tripleClickMode){		// 2007.10.10 nasukoji	トリプルクリックを処理する
			// 1行選択でない場合は選択文字列を解除
			// 2007.11.05 nasukoji	トリプルクリックが1行選択でなくてもクアドラプルクリックを有効とする
			if(F_SELECTLINE != nFuncID){
				OnLBUTTONUP( fwKeys, xPos, yPos );	// ここで左ボタンアップしたことにする

				if( IsTextSelected() )		// テキストが選択されているか
					DisableSelectArea( true );		// 現在の選択範囲を非選択状態に戻す
			}

			// 2007.10.10 nasukoji	単語の途中で折り返されていると下の行が選択されてしまうことへの対処
			MoveCursorToPoint( xPos, yPos );	// カーソル移動

			// コマンドコードによる処理振り分け
			// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
			::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ), (LPARAM)NULL );

			// 1行選択でない場合はここで抜ける（他の選択コマンドの時問題となるかも）
			if(F_SELECTLINE != nFuncID)
				return;

			// 2007.10.13 nasukoji	選択するものが無い（[EOF]のみの行）時は通常クリックと同じ処理
			if(( ! IsTextSelected() )&&( m_ptCaretPos_PHY.y >= m_pcEditDoc->m_cDocLineMgr.GetLineCount() )){
				BeginSelectArea();				// 現在のカーソル位置から選択を開始する
				m_bBeginLineSelect = false;		// 行単位選択中 OFF
			}
		}else
		/* 選択開始処理 */
		/* SHIFTキーが押されていたか */
		if(GetKeyState_Shift()){
			if( IsTextSelected() ){			/* テキストが選択されているか */
				if( m_bBeginBoxSelect ){	/* 矩形範囲選択中 */
					/* 現在の選択範囲を非選択状態に戻す */
					DisableSelectArea( true );
					/* 現在のカーソル位置から選択を開始する */
					BeginSelectArea( );
				}
				else{
				}
			}
			else{
				/* 現在のカーソル位置から選択を開始する */
				BeginSelectArea( );
			}

			/* カーソル移動 */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
		}
		else{
			if( IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在の選択範囲を非選択状態に戻す */
				DisableSelectArea( true );
			}
			/* カーソル移動 */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
			/* 現在のカーソル位置から選択を開始する */
			BeginSelectArea( );
		}


		/******* この時点で必ず true == IsTextSelected() の状態になる ****:*/
		if( !IsTextSelected() ){
			WarningMessage( m_hWnd, _T("バグってる") );
			return;
		}

		int	nWorkRel;
		nWorkRel = IsCurrentPositionSelected(
			m_ptCaretPos.x,	// カーソル位置X
			m_ptCaretPos.y	// カーソル位置Y
		);


		/* 現在のカーソル位置によって選択範囲を変更 */
		ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );


		// CTRLキーが押されている、かつトリプルクリックでない		// 2007.10.10 nasukoji	トリプルクリック対応
		if( GetKeyState_Control() &&( ! tripleClickMode)){
			m_bBeginWordSelect = true;		/* 単語単位選択中 */
			if( !IsTextSelected() ){
				/* 現在位置の単語選択 */
				if ( Command_SELECTWORD() ){
					m_sSelectBgn.m_ptFrom.y = m_sSelect.m_ptFrom.y;	/* 範囲選択開始行(原点) */
					m_sSelectBgn.m_ptFrom.x = m_sSelect.m_ptFrom.x;	/* 範囲選択開始桁(原点) */
					m_sSelectBgn.m_ptTo.y = m_sSelect.m_ptTo.y;		/* 範囲選択開始行(原点) */
					m_sSelectBgn.m_ptTo.x = m_sSelect.m_ptTo.x;		/* 範囲選択開始桁(原点) */
				}
			}else{

				/* 選択領域描画 */
				DrawSelectArea();


				/* 指定された桁に対応する行のデータ内の位置を調べる */
				const CLayout* pcLayout;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.m_ptFrom.y, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, m_sSelect.m_ptFrom.x );
					/* 現在位置の単語の範囲を調べる */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_sSelect.m_ptFrom.y, nIdx, &sRange, NULL, NULL )
					){
						/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptFrom.y, &nLineLen, &pcLayout );
						sRange.m_ptFrom.x = LineIndexToColmn( pcLayout, sRange.m_ptFrom.x );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptTo.y, &nLineLen, &pcLayout );
						sRange.m_ptTo.x = LineIndexToColmn( pcLayout, sRange.m_ptTo.x );


						nWork = IsCurrentPositionSelected(
							sRange.m_ptFrom.x,	// カーソル位置X
							sRange.m_ptFrom.y	// カーソル位置Y
						);
						if( -1 == nWork || 0 == nWork ){
							m_sSelect.m_ptFrom = sRange.m_ptFrom;
							if( 1 == nWorkRel ){
								m_sSelectBgn = sRange;
							}
						}
					}
				}
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.m_ptTo.y, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, m_sSelect.m_ptTo.x );
					/* 現在位置の単語の範囲を調べる */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_sSelect.m_ptTo.y, nIdx,
						&sRange, NULL, NULL )
					){
						// 指定された行のデータ内の位置に対応する桁の位置を調べる
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptFrom.y, &nLineLen, &pcLayout );
						sRange.m_ptFrom.x = LineIndexToColmn( pcLayout, sRange.m_ptFrom.x );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptTo.y, &nLineLen, &pcLayout );
						sRange.m_ptTo.x = LineIndexToColmn( pcLayout, sRange.m_ptTo.x );

						nWork = IsCurrentPositionSelected(
							sRange.m_ptFrom.x,	// カーソル位置X
							sRange.m_ptFrom.y	// カーソル位置Y
						);
						if( -1 == nWork || 0 == nWork ){
							m_sSelect.m_ptTo = sRange.m_ptFrom;
						}
						if( 1 == IsCurrentPositionSelected(
							sRange.m_ptTo.x,	// カーソル位置X
							sRange.m_ptTo.y		// カーソル位置Y
						) ){
							m_sSelect.m_ptTo = sRange.m_ptTo;
						}
						if( -1 == nWorkRel || 0 == nWorkRel ){
							m_sSelectBgn = sRange;
						}
					}
				}

				if( 0 < nWorkRel ){

				}
				/* 選択領域描画 */
				DrawSelectArea();
			}
		}
		// 行番号エリアをクリックした
		// 2007.12.08 nasukoji	シフトキーを押している場合は行頭クリックとして扱う
		if(( xPos < m_nViewAlignLeft )&& !GetKeyState_Shift() ){
			/* 現在のカーソル位置から選択を開始する */
			m_bBeginLineSelect = true;

			// 2002.10.07 YAZAKI 折り返し行をインデントしているときに選択がおかしいバグの対策
			// １行が画面幅よりも長いと左右にスクロールしてちらつきが激しくなるので後で全体を再描画	// 2008.05.20 ryoji
			bool bDrawSwitchOld = m_bDrawSWITCH;
			BOOL bDrawAfter = FALSE;
			if( bDrawSwitchOld ){
				const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_ptCaretPos.y );
				if( pcLayout ){
					int nColumn = LineIndexToColmn( pcLayout, pcLayout->GetLengthWithoutEOL() );
					bDrawAfter = (nColumn + SCROLLMARGIN_RIGHT >= m_nViewColNum);
					if( bDrawAfter ){
						m_bDrawSWITCH = false;
					}
				}
			}
			Command_GOLINEEND( true, 0 );
			Command_RIGHT( true, false, false );
			if( bDrawSwitchOld && bDrawAfter ){
				m_bDrawSWITCH = true;
				Redraw();
			}

			//	Apr. 14, 2003 genta
			//	行番号の下をクリックしてドラッグを開始するとおかしくなるのを修正
			//	行番号をクリックした場合にはChangeSelectAreaByCurrentCursor()にて
			//	m_sSelect.m_ptTo.x/m_sSelect.m_ptTo.yに-1が設定されるが、上の
			//	Command_GOLINEEND(), Command_RIGHT()によって行選択が行われる。
			//	しかしキャレットが末尾にある場合にはキャレットが移動しないので
			//	m_sSelect.m_ptTo.x/m_sSelect.m_ptTo.yが-1のまま残ってしまい、それが
			//	原点に設定されるためにおかしくなっていた。
			//	なので、範囲選択が行われていない場合は起点末尾の設定を行わないようにする
			if( IsTextSelected() ){
				m_sSelectBgn.m_ptTo.y = m_sSelect.m_ptTo.y;	/* 範囲選択開始行(原点) */
				m_sSelectBgn.m_ptTo.x = m_sSelect.m_ptTo.x;	/* 範囲選択開始桁(原点) */
			}
		}
		else{
			/* URLがクリックされたら選択するか */
			if( TRUE == m_pShareData->m_Common.m_sEdit.m_bSelectClickedURL ){

				int			nUrlLine;	// URLの行(折り返し単位)
				int			nUrlIdxBgn;	// URLの位置(行頭からのバイト位置)
				int			nUrlLen;	// URLの長さ(バイト数)
				// カーソル位置にURLが有る場合のその範囲を調べる
				bool bIsUrl = IsCurrentPositionURL(
					m_ptCaretPos.x,	// カーソル位置X
					m_ptCaretPos.y,	// カーソル位置Y
					&nUrlLine,		// URLの行(改行単位)
					&nUrlIdxBgn,	// URLの位置(行頭からのバイト位置)
					&nUrlLen,		// URLの長さ(バイト数)
					NULL			// URL受け取り先
				);
				if( bIsUrl ){
					/* 現在の選択範囲を非選択状態に戻す */
					DisableSelectArea( true );

					/*
					  カーソル位置変換
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					  →レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						2002/04/08 YAZAKI 少しでもわかりやすく。
					*/
					CLayoutRange sRangeB;
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nUrlIdxBgn          , nUrlLine, &sRangeB.m_ptFrom.x, &sRangeB.m_ptFrom.y );
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nUrlIdxBgn + nUrlLen, nUrlLine, &sRangeB.m_ptTo.x,   &sRangeB.m_ptTo.y );

					m_sSelectBgn = sRangeB;
					m_sSelect = sRangeB;

					/* 選択領域描画 */
					DrawSelectArea();
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
	
	@date 2007.10.02 nasukoji	新規作成
	@date 2007.10.11 nasukoji	改行文字より左側でないとトリプルクリックが認識されない不具合を修正
	@date 2007.10.13 nasukoji	行番号表示エリアのトリプルクリックは通常クリックとして扱う
*/
BOOL CEditView::CheckTripleClick( int xPos, int yPos )
{

	// トリプルクリックチェック有効でない（時刻がセットされていない）
	if(! m_dwTripleClickCheck)
		return FALSE;

	BOOL result = FALSE;

	// 前回クリックとのクリック位置のずれを算出
	int dxpos = m_nMouseRollPosXOld - xPos;
	int dypos = m_nMouseRollPosYOld - yPos;

	if(dxpos < 0)
		dxpos = -dxpos;		// 絶対値化

	if(dypos < 0)
		dypos = -dypos;		// 絶対値化

	// 行番号表示エリアでない、かつクリックプレスからダブルクリック時間以内、
	// かつダブルクリックの許容ずれピクセル以下のずれの時トリプルクリックとする
	//	2007.10.12 genta/dskoba システムのダブルクリック速度，ずれ許容量を取得
	if( (xPos >= m_nViewAlignLeft)&&
		(::GetTickCount() - m_dwTripleClickCheck <= GetDoubleClickTime() )&&
		(dxpos <= GetSystemMetrics(SM_CXDOUBLECLK) ) &&
		(dypos <= GetSystemMetrics(SM_CYDOUBLECLK)))
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
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
	/* 指定カーソル位置が選択エリア内にあるか */
	if( 0 == IsCurrentPositionSelected(
		nNewX,		// カーソル位置X
		nNewY		// カーソル位置Y
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
	if( m_bBeginSelect ){	/* 範囲選択中 */
		/* マウス左ボタン開放のメッセージ処理 */
		OnLBUTTONUP( fwKeys, xPos, yPos );
	}


	int		nIdx;
	int		nFuncID;
// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス右クリックに対応する機能コードはm_Common.m_sKeyBind.m_pKeyNameArr[1]に入っている */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_RIGHT].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
//	/* 右クリックメニュー */
//	Command_MENU_RBUTTON();
	return;
}


// novice 2004/10/11 マウス中ボタン対応
/*!
	@brief マウス中ボタンを押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.11 novice 新規作成
	@date 2009.01.12 nasukoji	ボタンUPでコマンドを起動するように変更
*/
void CEditView::OnMBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
}


/*!
	@brief マウス中ボタンを開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	
	@date 2009.01.12 nasukoji	新規作成（ボタンUPでコマンドを起動するように変更）
*/
void CEditView::OnMBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_CENTER &&
	    m_pcEditDoc->m_pcEditWnd->IsPageScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// ホイール操作によるページスクロールあり
	if( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_CENTER &&
	    m_pcEditDoc->m_pcEditWnd->IsHScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	// ホイール操作によるページスクロール・横スクロールあり
	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス中ボタンに対応する機能コードはm_Common.m_sKeyBind.m_pKeyNameArr[2]に入っている */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_CENTER].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
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
	@date 2009.01.12 nasukoji	ボタンUPでコマンドを起動するように変更
*/
void CEditView::OnXLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
}


/*!
	@brief マウスサイドボタン1を開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標

	@date 2009.01.12 nasukoji	新規作成（ボタンUPでコマンドを起動するように変更）
*/
void CEditView::OnXLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_LEFTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsPageScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// ホイール操作によるページスクロールあり
	if( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_LEFTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsHScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウスサイドボタン1に対応する機能コードはm_Common.m_sKeyBind.m_pKeyNameArr[3]に入っている */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_LEFTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
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
	@date 2009.01.12 nasukoji	ボタンUPでコマンドを起動するように変更
*/
void CEditView::OnXRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
}


/*!
	@brief マウスサイドボタン2を開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標

	@date 2009.01.12 nasukoji	新規作成（ボタンUPでコマンドを起動するように変更）
*/
void CEditView::OnXRBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_RIGHTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsPageScrollByWheel() )
	{
		// ホイール操作によるページスクロールありをOFF
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// ホイール操作によるページスクロールあり
	if( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_RIGHTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsHScrollByWheel() )
	{
		// ホイール操作による横スクロールありをOFF
		m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウスサイドボタン2に対応する機能コードはm_Common.m_sKeyBind.m_pKeyNameArr[4]に入っている */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_RIGHTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}

/* マウス移動のメッセージ処理 */
void CEditView::OnMOUSEMOVE( WPARAM fwKeys, int xPos , int yPos )
{
	int			nScrollRowNum;
	POINT		po;
	const char*	pLine;
	int			nLineLen;
	int			nIdx;
	int			nWorkF;
	int			nWorkT;

	CLayoutRange	sSelect_Old = m_sSelect;

	if( !m_bBeginSelect ){	/* 範囲選択中 */
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
		int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
		int			nUrlLine;	// URLの行(折り返し単位)
		int			nUrlIdxBgn;	// URLの位置(行頭からのバイト位置)
		int			nUrlLen;	// URLの長さ(バイト数)


		/* 選択テキストのドラッグ中か */
		if( m_bDragMode ){
			if( TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop ){	/* OLEによるドラッグ & ドロップを使う */
				/* 座標指定によるカーソル移動 */
				nScrollRowNum = MoveCursorToPoint( xPos , yPos );
			}
		}
		else{
			/* 行選択エリア? */
			if( xPos < m_nViewAlignLeft || yPos < m_nViewAlignTop ){	//	2002/2/10 aroka
				/* 矢印カーソル */
				if( yPos >= m_nViewAlignTop )
					::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_RVARROW ) ) );
				else
					::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}else

			if( TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop	/* OLEによるドラッグ & ドロップを使う */
			 && TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource /* OLEによるドラッグ元にするか */
			 && 0 == IsCurrentPositionSelected(						/* 指定カーソル位置が選択エリア内にあるか */
				nNewX,	// カーソル位置X
				nNewY	// カーソル位置Y
				)
			){
				/* 矢印カーソル */
				::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}
			/* カーソル位置にURLが有る場合 */
			else if(
				IsCurrentPositionURL(
				nNewX,			// カーソル位置X
				nNewY,			// カーソル位置Y
				&nUrlLine,		// URLの行(改行単位)
				&nUrlIdxBgn,	// URLの位置(行頭からのバイト位置)
				&nUrlLen,		// URLの長さ(バイト数)
				NULL			// URL受け取り先
			) ){
				/* 手カーソル */
				::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_HAND ) ) );
			}else{
				//migemo isearch 2004.10.22
				if( m_nISearchMode > 0 ){
					if (m_nISearchDirection == 1){
						::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
					}else{
						::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
					}
				}else
				/* アイビーム */
				::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
			}
		}
		return;
	}
	::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
	if( m_bBeginBoxSelect ){	/* 矩形範囲選択中 */
		/* 座標指定によるカーソル移動 */
		nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		/* 現在のカーソル位置によって選択範囲を変更 */
		ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
		m_nMouseRollPosXOld = xPos;	/* マウス範囲選択前回位置(X座標) */
		m_nMouseRollPosYOld = yPos;	/* マウス範囲選択前回位置(Y座標) */
	}
	else{
		/* 座標指定によるカーソル移動 */
		if(( xPos < m_nViewAlignLeft || m_dwTripleClickCheck )&& m_bBeginLineSelect ){		// 2007.10.02 nasukoji	行単位選択中
			// 2007.10.13 nasukoji	上方向の行選択時もマウスカーソルの位置の行が選択されるようにする
			int nNewY = yPos;
			int nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;		// 1行の高さ

			// 選択開始行以下へのドラッグ時は1行下にカーソルを移動する
			if( m_nViewTopLine + (yPos - m_nViewAlignTop) / nLineHeight >= m_sSelectBgn.m_ptTo.y )
				nNewY += nLineHeight;

			// カーソルを移動
			nScrollRowNum = MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) , nNewY );

			// 2007.10.13 nasukoji	2.5クリックによる行単位のドラッグ
			if( m_dwTripleClickCheck ){
				int nSelectStartLine = m_sSelectBgn.m_ptTo.y;

				// 選択開始行以上にドラッグした
				if( m_ptCaretPos.y <= nSelectStartLine ){
					Command_GOLINETOP( true, 0x09 );	// 改行単位の行頭へ移動
				}else{
					int nCaretPosX;
					int nCaretPosY;

					int nCaretPrevPosX_PHY;
					int nCaretPrevPosY_PHY = m_ptCaretPos_PHY.y;

					// 選択開始行より下にカーソルがある時は1行前と物理行番号の違いをチェックする
					// 選択開始行にカーソルがある時はチェック不要
					if(m_ptCaretPos.y > nSelectStartLine){
						// 1行前の物理行を取得する
						m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
							0, m_ptCaretPos.y - 1, &nCaretPrevPosX_PHY, &nCaretPrevPosY_PHY
						);
					}

					// 前の行と同じ物理行
					if( nCaretPrevPosY_PHY == m_ptCaretPos_PHY.y ){
						// 1行先の物理行からレイアウト行を求める
						m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
							0, m_ptCaretPos_PHY.y + 1, &nCaretPosX, &nCaretPosY
						);

						// カーソルを次の物理行頭へ移動する
						nScrollRowNum = MoveCursor( nCaretPosX, nCaretPosY, true );
					}
				}
			}
		}else{
			nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		}
		m_nMouseRollPosXOld = xPos;	/* マウス範囲選択前回位置(X座標) */
		m_nMouseRollPosYOld = yPos;	/* マウス範囲選択前回位置(Y座標) */

		if( !m_bBeginWordSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
		}else{
			CLayoutRange sSelect;

			/* 現在のカーソル位置によって選択範囲を変更(テストのみ) */
			ChangeSelectAreaByCurrentCursorTEST(
				(int)m_ptCaretPos.x,
				(int)m_ptCaretPos.y,
				&sSelect
			);
			/* 選択範囲に変更なし */
			if( sSelect_Old.m_ptFrom.y == sSelect.m_ptFrom.y
			 && sSelect_Old.m_ptFrom.x == sSelect.m_ptFrom.x
			 && sSelect_Old.m_ptTo.y == sSelect.m_ptTo.y
			 && sSelect_Old.m_ptTo.x == sSelect.m_ptTo.x
			){
				ChangeSelectAreaByCurrentCursor(
					(int)m_ptCaretPos.x,
					(int)m_ptCaretPos.y
				);
				return;
			}
			const CLayout* pcLayout;
			if( NULL != ( pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout ) ) ){
				nIdx = LineColmnToIndex( pcLayout, m_ptCaretPos.x );
				CLayoutRange sRange;

				/* 現在位置の単語の範囲を調べる */
				if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
					m_ptCaretPos.y, nIdx, &sRange, NULL, NULL )
				){
					/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptFrom.y, &nLineLen, &pcLayout );
					sRange.m_ptFrom.x = LineIndexToColmn( pcLayout, sRange.m_ptFrom.x );
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptTo.y, &nLineLen, &pcLayout );
					sRange.m_ptTo.x = LineIndexToColmn( pcLayout, sRange.m_ptTo.x );

					nWorkF = IsCurrentPositionSelectedTEST(
						sRange.m_ptFrom.x,	// カーソル位置X
						sRange.m_ptFrom.y,	// カーソル位置Y
						sSelect
					);
					nWorkT = IsCurrentPositionSelectedTEST(
						sRange.m_ptTo.x,	// カーソル位置X
						sRange.m_ptTo.y,	// カーソル位置Y
						sSelect
					);
					if( -1 == nWorkF/* || 0 == nWorkF*/ ){
						/* 始点が前方に移動。現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( sRange.m_ptFrom.x, sRange.m_ptFrom.y );
					}
					else if( /*0 == nWorkT ||*/ 1 == nWorkT ){
						/* 終点が後方に移動。現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( sRange.m_ptTo.x, sRange.m_ptTo.y );
					}
					else if( sSelect_Old.m_ptFrom.y == sSelect.m_ptFrom.y
					 && sSelect_Old.m_ptFrom.x == sSelect.m_ptFrom.x
					){
						/* 始点が無変更＝前方に縮小された */
						/* 現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( sRange.m_ptTo.x, sRange.m_ptTo.y );
					}
					else if( sSelect_Old.m_ptTo.y == sSelect.m_ptTo.y
					 && sSelect_Old.m_ptTo.x == sSelect.m_ptTo.x
					){
						/* 終点が無変更＝後方に縮小された */
						/* 現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( sRange.m_ptFrom.x, sRange.m_ptFrom.y );
					}
				}else{
					/* 現在のカーソル位置によって選択範囲を変更 */
					ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
				}
			}else{
				/* 現在のカーソル位置によって選択範囲を変更 */
				ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
			}
		}
	}
	return;
}
//m_dwTipTimerm_dwTipTimerm_dwTipTimer




/* マウスホイールのメッセージ処理
*/
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
//	MYTRACE( _T("CEditView::DispatchEvent() WM_MOUSEWHEEL fwKeys=%xh zDelta=%d xPos=%d yPos=%d \n"), fwKeys, zDelta, xPos, yPos );

	if( 0 < zDelta ){
		nScrollCode = SB_LINEUP;
	}else{
		nScrollCode = SB_LINEDOWN;
	}




	// 2009.01.12 nasukoji	ホイールスクロールを利用したページスクロール・横スクロール対応
	if( IsSpecialScrollMode( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel ) ){				// ページスクロール？
		if( IsSpecialScrollMode( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel ) ){		// 横スクロール？
			int line = m_nViewLeftCol + (( nScrollCode == SB_LINEUP ) ? -m_nViewColNum : m_nViewColNum );
			SyncScrollH( ScrollAtH( line ) );

			// ホイール操作による横スクロールあり
			m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( TRUE );
		}else{
			int line = m_nViewTopLine + (( nScrollCode == SB_LINEUP ) ? -m_nViewRowNum : m_nViewRowNum );
			SyncScrollV( ScrollAtV( line ) );
		}
		// ホイール操作によるページスクロールあり
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( TRUE );
	}else{
		/* マウスホイールによるスクロール行数をレジストリから取得 */
		nRollLineNum = 6;
		/* レジストリの存在チェック */
		// 2006.06.03 Moca ReadRegistry に書き換え
		unsigned int uDataLen;	// size of value data
		char szValStr[256];
		uDataLen = sizeof(szValStr) - 1;
		if( ReadRegistry( HKEY_CURRENT_USER, "Control Panel\\desktop", "WheelScrollLines", szValStr, uDataLen ) ){
			nRollLineNum = ::atoi( szValStr );
		}
		if( -1 == nRollLineNum ){/* 「1画面分スクロールする」 */
			nRollLineNum = m_nViewRowNum;	// 表示域の行数
		}else{
			if( nRollLineNum < 1 ){
				nRollLineNum = 1;
			}
			if( nRollLineNum > 30 ){	//@@@ YAZAKI 2001.12.31 10→30へ。
				nRollLineNum = 30;
			}
		}

		// 2009.01.12 nasukoji	キー/マウスボタン + ホイールスクロールで横スクロールする
		bool bHorizontal = IsSpecialScrollMode( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel );
		int nCount = ( nScrollCode == SB_LINEUP ) ? -1 : 1;		// スクロール数

		for( i = 0; i < nRollLineNum; ++i ){

			//	Sep. 11, 2004 genta 同期スクロール行数
			if( bHorizontal ){
				SyncScrollH( ScrollAtH( m_nViewLeftCol + nCount ) );

				// ホイール操作による横スクロールあり
				m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( TRUE );
			}else{
				SyncScrollV( ScrollAtV( m_nViewTopLine + nCount ) );
			}
		}
	}
	return 0;
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

	@date 2009.01.12 nasukoji	新規作成
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
	if( m_bBeginSelect ){	/* 範囲選択中 */
		/* マウス キャプチャを解放 */
		::ReleaseCapture();
		ShowCaret_( m_hWnd ); // 2002/07/22 novice

		m_bBeginSelect = false;

		if( m_sSelect.m_ptFrom.y == m_sSelect.m_ptTo.y &&
			m_sSelect.m_ptFrom.x == m_sSelect.m_ptTo.x
		){
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( true );

			// 対括弧の強調表示	// 2007.10.18 ryoji
			DrawBracketPair( false );
			SetBracketPairPos( true );
			DrawBracketPair( true );
		}

		m_cUnderLine.UnLock();
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


/* マウス左ボタンダブルクリック */
void CEditView::OnLBUTTONDBLCLK( WPARAM fwKeys, int xPos , int yPos )
{
	int			nIdx;
	int			nFuncID;
	int			nUrlLine;	// URLの行(折り返し単位)
	int			nUrlIdxBgn;	// URLの位置(行頭からのバイト位置)
	int			nUrlLen;	// URLの長さ(バイト数)
	char*		pszURL;
	const char*	pszMailTo = "mailto:";

	// 2007.10.06 nasukoji	クアドラプルクリック時はチェックしない
	if(! m_dwTripleClickCheck){
		/* カーソル位置にURLが有る場合のその範囲を調べる */
		if(
			IsCurrentPositionURL(
				m_ptCaretPos.x,	// カーソル位置X
				m_ptCaretPos.y,	// カーソル位置Y
				&nUrlLine,		// URLの行(改行単位)
				&nUrlIdxBgn,	// URLの位置(行頭からのバイト位置)
				&nUrlLen,		// URLの長さ(バイト数)
				&pszURL			// URL受け取り先
			)
		 ){
			char*		pszWork = NULL;
			char*		pszOPEN;

			/* URLを開く */
			/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */
			if( TRUE == IsMailAddress( pszURL, lstrlen( pszURL ), NULL ) ){
				pszWork = new char[ lstrlen( pszURL ) + lstrlen( pszMailTo ) + 1];
				strcpy( pszWork, pszMailTo );
				strcat( pszWork, pszURL );
				pszOPEN = pszWork;
			}else{
				if( _tcsnicmp( pszURL, _T("ttp://"), 6 ) == 0 ){	//抑止URL
					pszWork = new TCHAR[ _tcslen( pszURL ) + 1 + 1 ];
					_tcscpy( pszWork, _T("h") );
					_tcscat( pszWork, pszURL );
					pszOPEN = pszWork;
				}else if( _tcsnicmp( pszURL, _T("tp://"), 5 ) == 0 ){	//抑止URL
					pszWork = new TCHAR[ _tcslen( pszURL ) + 2 + 1 ];
					_tcscpy( pszWork, _T("ht") );
					_tcscat( pszWork, pszURL );
					pszOPEN = pszWork;
				}else{
					pszOPEN = pszURL;
				}
			}
			{
				// URLを開く
				// 2009.05.21 syat UNCパスだと1分以上無応答になることがあるのでスレッド化
				CWaitCursor cWaitCursor( m_hWnd );	// カーソルを砂時計にする

				unsigned int nThreadId;
				LPCTSTR szUrl = pszOPEN;
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
			delete [] pszURL;
			delete [] pszWork;
			return;
		}

		/* GREP出力モードまたはデバッグモード かつ マウス左ボタンダブルクリックでタグジャンプ の場合 */
		//	2004.09.20 naoh 外部コマンドの出力からTagjumpできるように
		if( (m_pcEditDoc->m_bGrepMode || m_pcEditDoc->m_bDebugMode) && m_pShareData->m_Common.m_sSearch.m_bGTJW_LDBLCLK ){
			/* タグジャンプ機能 */
			Command_TAGJUMP();
			return;
		}
	}

// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();

	/* マウス左クリックに対応する機能コードはm_Common.m_sKeyBind.m_pKeyNameArr[?]に入っている 2007.10.06 nasukoji */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[
		m_dwTripleClickCheck ? MOUSEFUNCTION_QUADCLICK : MOUSEFUNCTION_DOUBLECLICK
		].m_nFuncCodeArr[nIdx];
	if(m_dwTripleClickCheck){
		// 非選択状態にした後左クリックしたことにする
		// すべて選択の場合は、3.5クリック時の選択状態保持とドラッグ開始時の
		// 範囲変更のため。
		// クアドラプルクリック機能が割り当てられていない場合は、ダブルクリック
		// として処理するため。
		if( IsTextSelected() )		// テキストが選択されているか
			DisableSelectArea( true );		// 現在の選択範囲を非選択状態に戻す

		if(! nFuncID){
			m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
			nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_DOUBLECLICK].m_nFuncCodeArr[nIdx];
			OnLBUTTONDOWN( fwKeys, xPos , yPos );	// カーソルをクリック位置へ移動する
		}
	}

	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
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
	m_nMouseRollPosXOld = xPos;			// マウス範囲選択前回位置(X座標)
	m_nMouseRollPosYOld = yPos;			// マウス範囲選択前回位置(Y座標)

	/*	2007.07.09 maru 機能コードの判定を追加
		ダブルクリックからのドラッグでは単語単位の範囲選択(エディタの一般的動作)になるが
		この動作は、ダブルクリック＝単語選択を前提としたもの。
		キー割り当ての変更により、ダブルクリック≠単語選択のときには m_bBeginWordSelect = true
		にすると、処理の内容によっては表示がおかしくなるので、ここで抜けるようにする。
	*/
	if(F_SELECTWORD != nFuncID) return;

	/* 範囲選択開始 & マウスキャプチャー */
	m_bBeginSelect = true;				/* 範囲選択中 */
	m_bBeginBoxSelect = false;			/* 矩形範囲選択中でない */
	m_bBeginLineSelect = false;			/* 行単位選択中 */
	m_bBeginWordSelect = true;			/* 単語単位選択中 */

	if( m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
		/* ALTキーが押されていたか */
		if(GetKeyState_Alt()){
			m_bBeginBoxSelect = true;	/* 矩形範囲選択中 */
		}
	}
	::SetCapture( m_hWnd );
	HideCaret_( m_hWnd ); // 2002/07/22 novice
	if( IsTextSelected() ){
		/* 常時選択範囲の範囲 */
		m_sSelectBgn.m_ptTo.y = m_sSelect.m_ptTo.y;
		m_sSelectBgn.m_ptTo.x = m_sSelect.m_ptTo.x;
	}else{
		/* 現在のカーソル位置から選択を開始する */
		BeginSelectArea( );
	}

	return;
}






/** DragEnter 処理
	@date 2008.03.26 ryoji SAKURAClipフォーマット（NULL文字を含むテキスト）への対応を追加
	@date 2008.06.20 ryoji CF_HDROPフォーマットへの対応を追加
*/
STDMETHODIMP CEditView::DragEnter( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::DragEnter()\n") );

	if( TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop	/* OLEによるドラッグ & ドロップを使う */
		//	Oct. 22, 2005 genta 上書き禁止(ファイルがロックされている)場合も不可
		 && !( SHAREMODE_NOT_EXCLUSIVE != m_pcEditDoc->m_nFileShareModeOld && INVALID_HANDLE_VALUE == m_pcEditDoc->m_hLockedFile )
		 && !m_pcEditDoc->IsReadOnly() ){ // Mar. 30, 2003 読み取り専用のファイルにはドロップさせない
	}else{
		return E_UNEXPECTED;	//	Moca E_INVALIDARGから変更
	}

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
	m_pcEditDoc->SetActivePane( m_nMyIndex );

	// 現在のカーソル位置を記憶する	// 2007.12.09 ryoji
	m_ptCaretPos_DragEnter.x = m_ptCaretPos.x;
	m_ptCaretPos_DragEnter.y = m_ptCaretPos.y;
	m_nCaretPosX_Prev_DragEnter = m_nCaretPosX_Prev;

	// ドラッグデータは矩形か
	m_bDragBoxData = IsDataAvailable( pDataObject, ::RegisterClipboardFormat( _T("MSDEVColumnSelect") ) );

	/* 選択テキストのドラッグ中か */
	m_bDragMode = TRUE;

	DragOver( dwKeyState, pt, pdwEffect );
	return S_OK;
}

STDMETHODIMP CEditView::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::DragOver()\n") );

	/* マウス移動のメッセージ処理 */
	::ScreenToClient( m_hWnd, (LPPOINT)&pt );
	OnMOUSEMOVE( dwKeyState, pt.x , pt.y );

	if ( pdwEffect == NULL )
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect( m_cfDragData, dwKeyState, pt, *pdwEffect );

	CEditView* pcDragSourceView = m_pcEditDoc->GetDragSourceView();

	// ドラッグ元が他ビューで、このビューのカーソルがドラッグ元の選択範囲内の場合は禁止マークにする
	// ※自ビューのときは禁止マークにしない（他アプリでも多くはそうなっている模様）	// 2009.06.09 ryoji
	if( pcDragSourceView && !IsDragSource() &&
		!pcDragSourceView->IsCurrentPositionSelected( m_ptCaretPos.x, m_ptCaretPos.y )
	){
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

STDMETHODIMP CEditView::DragLeave( void )
{
	DEBUG_TRACE( _T("CEditView::DragLeave()\n") );
	/* 選択テキストのドラッグ中か */
	m_bDragMode = FALSE;

	// DragEnter時のカーソル位置を復元	// 2007.12.09 ryoji
	MoveCursor( m_ptCaretPos_DragEnter.x, m_ptCaretPos_DragEnter.y, false );
	m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
	RedrawAll();	// ルーラー、アンダーライン、カーソル位置表示更新

	// 非アクティブ時は表示状態を非アクティブに戻す	// 2007.12.09 ryoji
	if( ::GetActiveWindow() == NULL )
		OnKillFocus();

	return S_OK;
}

/** ドロップ処理
	@date 2008.03.26 ryoji ドロップで貼り付けた範囲を選択状態にする
	                       SAKURAClipフォーマット（NULL文字を含むテキスト）への対応を追加
	@date 2008.06.20 ryoji CF_HDROPフォーマットへの対応を追加
*/
STDMETHODIMP CEditView::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::Drop()\n") );
	CMemory		cmemBuf;
	COpe*		pcOpe;
	BOOL		bBoxData;
	BOOL		bMove;
	BOOL		bMoveToPrev;
	RECT		rcSel;
	int			nCaretPosX_Old;
	int			nCaretPosY_Old;
	bool		bBeginBoxSelect_Old;
	CLayoutRange	sSelectBgn_Old;
	CLayoutRange	sSelect_Old;

	/* 選択テキストのドラッグ中か */
	m_bDragMode = FALSE;

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
	CEditView* pcDragSourceView = m_pcEditDoc->GetDragSourceView();
	bMove = (*pdwEffect == DROPEFFECT_MOVE) && pcDragSourceView;
	bBoxData = m_bDragBoxData;

	// カーソルが選択範囲内にあるときはコピー／移動しない	// 2009.06.09 ryoji
	if( pcDragSourceView &&
		!pcDragSourceView->IsCurrentPositionSelected( m_ptCaretPos.x, m_ptCaretPos.y )
	){
		// DragEnter時のカーソル位置を復元
		// Note. ドラッグ元が他ビューでもマウス移動が速いと稀にここにくる可能性がありそう
		*pdwEffect = DROPEFFECT_NONE;
		MoveCursor( m_ptCaretPos_DragEnter.x, m_ptCaretPos_DragEnter.y, false );
		m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
		if( !IsDragSource() )	// ドラッグ元の場合はここでは再描画不要（DragDrop後処理のSetActivePaneで再描画される）
			RedrawAll();	// ←主に以後の非アクティブ化に伴うアンダーライン消しのために一度更新して整合をとる
		return S_OK;
	}

	// ドロップデータの取得
	HGLOBAL hData = GetGlobalData( pDataObject, cf );
	if( hData == NULL )
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock( hData );
	SIZE_T nSize = ::GlobalSize( hData );
	if( cf == ::RegisterClipboardFormat( _T("SAKURAClip") ) ){
		cmemBuf.SetString( (char*)pData + sizeof(int), *(int*)pData );
	}else{
		CMemory cmemTemp;
		cmemTemp.SetString( (char*)pData, nSize );	// 安全のため末尾に null 文字を付加
		cmemBuf.SetString( cmemTemp.GetStringPtr() );		// 文字列終端までコピー
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
				pcDragSourceView->m_sSelect.m_ptFrom.y,		/* 範囲選択開始行 */
				pcDragSourceView->m_sSelect.m_ptFrom.x,		/* 範囲選択開始桁 */
				pcDragSourceView->m_sSelect.m_ptTo.y,		/* 範囲選択終了行 */
				pcDragSourceView->m_sSelect.m_ptTo.x			/* 範囲選択終了桁 */
			);
			++rcSel.bottom;
			if( m_ptCaretPos.y >= rcSel.bottom ){
				bMoveToPrev = FALSE;
			}else
			if( m_ptCaretPos.y + rcSel.bottom - rcSel.top < rcSel.top ){
				bMoveToPrev = TRUE;
			}else
			if( m_ptCaretPos.x < rcSel.left ){
				bMoveToPrev = TRUE;
			}else{
				bMoveToPrev = FALSE;
			}
		}else{
			if( pcDragSourceView->m_sSelect.m_ptFrom.y > m_ptCaretPos.y ){
				bMoveToPrev = TRUE;
			}else
			if( pcDragSourceView->m_sSelect.m_ptFrom.y == m_ptCaretPos.y ){
				if( pcDragSourceView->m_sSelect.m_ptFrom.x > m_ptCaretPos.x ){
					bMoveToPrev = TRUE;
				}else{
					bMoveToPrev = FALSE;
				}
			}else{
				bMoveToPrev = FALSE;
			}
		}
	}

	nCaretPosX_Old = m_ptCaretPos.x;
	nCaretPosY_Old = m_ptCaretPos.y;
	if( !bMove ){
		/* コピーモード */
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( true );
	}else{
		// ドラッグ元の選択範囲を記憶
		bBeginBoxSelect_Old = pcDragSourceView->m_bBeginBoxSelect;
		sSelectBgn_Old = pcDragSourceView->m_sSelectBgn;
		sSelect_Old = pcDragSourceView->m_sSelect;

		if( bMoveToPrev ){
			/* 移動モード & 前に移動 */
			/* 選択エリアを削除 */
			if( this != pcDragSourceView ){
				// ドラッグ元の選択範囲を復元
				pcDragSourceView->DisableSelectArea( true );
				DisableSelectArea( true );
				m_bBeginBoxSelect = bBeginBoxSelect_Old;
				m_sSelectBgn = sSelectBgn_Old;
				m_sSelect = sSelect_Old;
			}
			DeleteData( true );
			MoveCursor( nCaretPosX_Old, nCaretPosY_Old, true );
		}else{
			/* 現在の選択範囲を非選択状態に戻す */
			pcDragSourceView->DisableSelectArea( true );
			if( this != pcDragSourceView )
				DisableSelectArea( true );
		}
	}
	if( !bBoxData ){	/* 矩形データ */
		//	2004,05.14 Moca 引数に文字列長を追加

		// 挿入前のキャレット位置を記憶する
		// （キャレットが行終端より右の場合は埋め込まれる空白分だけ桁位置をシフト）
		int nCaretPosX_PHY_Old = m_ptCaretPos_PHY.x;
		int nCaretPosY_PHY_Old = m_ptCaretPos_PHY.y;
		const CLayout* pcLayout;
		int nLineLen;
		if( m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout ) ){
			LineColmnToIndex2( pcLayout, m_ptCaretPos.x, nLineLen );
			if( nLineLen > 0 ){	// 行終端より右の場合には nLineLen に行全体の表示桁数が入っている
				nCaretPosX_PHY_Old += (m_ptCaretPos.x - nLineLen);
			}
		}

		Command_INSTEXT( true, cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), FALSE );

		// 挿入前のキャレット位置から挿入後のキャレット位置までを選択範囲にする
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			nCaretPosX_PHY_Old, nCaretPosY_PHY_Old,
			&m_sSelect.m_ptFrom.x, &m_sSelect.m_ptFrom.y
		);
		CLayoutRange sRange;
		sRange.m_ptFrom = m_sSelect.m_ptFrom;
		sRange.m_ptTo = m_ptCaretPos;
		SetSelectArea( sRange );	// 2009.07.25 ryoji
	}else{
		// 2004.07.12 Moca クリップボードを書き換えないように
		// TRUE == bBoxData
		// false == m_bBeginBoxSelect
		/* 貼り付け（クリップボードから貼り付け）*/
		Command_PASTEBOX( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength() );
		AdjustScrollBars(); // 2007.07.22 ryoji
		Redraw();
	}
	if( bMove ){
		if( bMoveToPrev ){
		}else{
			/* 移動モード & 後ろに移動*/

			// 現在の選択範囲を記憶する	// 2008.03.26 ryoji
			CLogicPoint ptSelectFrom_PHY;
			CLogicPoint ptSelectTo_PHY;
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y,
				&ptSelectFrom_PHY.x, &ptSelectFrom_PHY.y
			);
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y,
				&ptSelectTo_PHY.x, &ptSelectTo_PHY.y
			);

			// 以前の選択範囲を記憶する	// 2008.03.26 ryoji
			CLogicPoint ptSelectFrom_PHY_Old;
			CLogicPoint ptSelect_PHY_Old;
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				sSelect_Old.m_ptFrom.x, sSelect_Old.m_ptFrom.y,
				&ptSelectFrom_PHY_Old.x, &ptSelectFrom_PHY_Old.y
			);
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				sSelect_Old.m_ptTo.x, sSelect_Old.m_ptTo.y,
				&ptSelect_PHY_Old.x, &ptSelect_PHY_Old.y
			);

			// 現在の行数を記憶する	// 2008.03.26 ryoji
			int nLines_Old = m_pcEditDoc->m_cDocLineMgr.GetLineCount();

			// 以前の選択範囲を選択する
			m_bBeginBoxSelect = bBeginBoxSelect_Old;
			m_sSelectBgn = sSelectBgn_Old;	/* 範囲選択開始(原点) */
			m_sSelect = sSelect_Old;

			/* 選択エリアを削除 */
			DeleteData( true );

			// 削除前の選択範囲を復元する	// 2008.03.26 ryoji
			if( !bBoxData ){
				// 削除された範囲を考慮して選択範囲を調整する
				if( ptSelectFrom_PHY.y == ptSelect_PHY_Old.y ){	// 選択開始が削除末尾と同一行
					ptSelectFrom_PHY.x -= (ptSelect_PHY_Old.x - ptSelectFrom_PHY_Old.x);
				}
				if( ptSelectTo_PHY.y == ptSelect_PHY_Old.y ){	// 選択終了が削除末尾と同一行
					ptSelectTo_PHY.x -= (ptSelect_PHY_Old.x - ptSelectFrom_PHY_Old.x);
				}
				// Note.
				// (ptSelect_PHY_Old.y - ptSelectFrom_PHY_Old.y) は実際の削除行数と同じになる
				// こともあるが、（削除行数－１）になることもある．
				// 例）フリーカーソルでの行番号クリック時の１行選択
				int nLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
				ptSelectFrom_PHY.y -= (nLines_Old - nLines);
				ptSelectTo_PHY.y -= (nLines_Old - nLines);

				// 調整後の選択範囲を設定する
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
					ptSelectFrom_PHY.x, ptSelectFrom_PHY.y,
					&m_sSelect.m_ptFrom.x, &m_sSelect.m_ptFrom.y
				);
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
					ptSelectTo_PHY.x, ptSelectTo_PHY.y,
					&m_sSelect.m_ptTo.x, &m_sSelect.m_ptTo.y
				);
				SetSelectArea( m_sSelect );	// 2009.07.25 ryoji
				nCaretPosX_Old = m_sSelect.m_ptTo.x;
				nCaretPosY_Old = m_sSelect.m_ptTo.y;
			}

			// キャレットを移動する
			MoveCursor( nCaretPosX_Old, nCaretPosY_Old, true );
			m_nCaretPosX_Prev = m_ptCaretPos.x;

			// 削除位置から移動先へのカーソル移動をアンドゥ操作に追加する	// 2008.03.26 ryoji
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;
			pcOpe->m_ptCaretPos_PHY_Before = ptSelectFrom_PHY_Old;
			pcOpe->m_ptCaretPos_PHY_After = m_ptCaretPos_PHY;
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}
	DrawSelectArea();

	// アンドゥバッファの処理
	SetUndoBuffer();

	::GlobalUnlock( hData );
	// 2004.07.12 fotomo/もか メモリーリークの修正
	if( 0 == (GMEM_LOCKCOUNT & ::GlobalFlags( hData )) ){
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
	memcpy( ::GlobalLock( hDrop ), pData, nSize );
	::GlobalUnlock( hDrop );
	::PostMessage(
		m_hWnd,
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
	HWND hwnd = ::CreateWindow(_T("STATIC"), _T(""), 0, 0, 0, 0, 0, NULL, NULL, m_hInstance, NULL );
	::SetFocus(hwnd);

	// メニューを作成する
	POINT pt;
	::GetCursorPos( &pt );
	RECT rcWork;
	GetMonitorWorkRect( pt, &rcWork );	// モニタのワークエリア
	HMENU hMenu = ::CreatePopupMenu();
	::InsertMenu( hMenu, 0, MF_BYPOSITION | MF_STRING, 100, _T("パス名貼り付け(&P)") );
	::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_STRING, 101, _T("ファイル名貼り付け(&F)") );
	::InsertMenu( hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// セパレータ
	::InsertMenu( hMenu, 3, MF_BYPOSITION | MF_STRING, 110, _T("ファイルを開く(&O)") );
	::InsertMenu( hMenu, 4, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// セパレータ
	::InsertMenu( hMenu, 5, MF_BYPOSITION | MF_STRING, IDCANCEL, _T("キャンセル") );
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
		::SendMessage( m_pcEditDoc->m_pcEditWnd->m_hWnd, WM_DROPFILES, (WPARAM)hDrop, 0 );
		break;

	case 100:	// パス名を貼り付ける
	case 101:	// ファイル名を貼り付ける
		CMemory cmemBuf;
		UINT nFiles;
		TCHAR szPath[_MAX_PATH];
		TCHAR szExt[_MAX_EXT];
		TCHAR szWork[_MAX_PATH];

		nFiles = ::DragQueryFile( hDrop, 0xFFFFFFFF, (LPSTR) NULL, 0 );
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
			cmemBuf.AppendString( szPath );
			if( nFiles > 1 ){
				cmemBuf.AppendString( m_pcEditDoc->GetNewLineCode().GetValue() );
			}
		}
		::DragFinish( hDrop );

		// 選択範囲の選択解除
		if( IsTextSelected() ){
			DisableSelectArea( true );
		}

		// 挿入前のキャレット位置を記憶する
		// （キャレットが行終端より右の場合は埋め込まれる空白分だけ桁位置をシフト）
		int nCaretPosX_PHY_Old = m_ptCaretPos_PHY.x;
		int nCaretPosY_PHY_Old = m_ptCaretPos_PHY.y;
		const CLayout* pcLayout;
		int nLineLen;
		if( m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout ) ){
			LineColmnToIndex2( pcLayout, m_ptCaretPos.x, nLineLen );
			if( nLineLen > 0 ){	// 行終端より右の場合には nLineLen に行全体の表示桁数が入っている
				nCaretPosX_PHY_Old += (m_ptCaretPos.x - nLineLen);
			}
		}

		// テキスト挿入
		HandleCommand( F_INSTEXT, true, (LPARAM)cmemBuf.GetStringPtr(), TRUE, 0, 0 );

		// 挿入前のキャレット位置から挿入後のキャレット位置までを選択範囲にする
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			nCaretPosX_PHY_Old, nCaretPosY_PHY_Old,
			&m_sSelect.m_ptFrom.x, &m_sSelect.m_ptFrom.y
		);
		CLayoutRange sRange;
		sRange.m_ptFrom = m_sSelect.m_ptFrom;
		sRange.m_ptTo = m_ptCaretPos;
		SetSelectArea( sRange );	// 2009.07.25 ryoji
		DrawSelectArea();
		break;
	}

	// メモリ解放
	::GlobalFree( hDrop );
}

CLIPFORMAT CEditView::GetAvailableClipFormat( LPDATAOBJECT pDataObject )
{
	CLIPFORMAT cf = 0;
	CLIPFORMAT cfSAKURAClip = ::RegisterClipboardFormat( _T("SAKURAClip") );

	if( IsDataAvailable( pDataObject, cfSAKURAClip ) )
		cf = cfSAKURAClip;
	//else if( IsDataAvailable(pDataObject, CF_UNICODETEXT) )
	//	cf = CF_UNICODETEXT;
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

	CEditView* pcDragSourceView = m_pcEditDoc->GetDragSourceView();

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
	return ( this == m_pcEditDoc->GetDragSourceView() );
}

/*[EOF]*/
