/*!	@file
@brief CViewCommanderクラスのコマンド(編集系 基本形)関数群

	2012/12/16	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta
	Copyright (C) 2003, MIK, genta, かろと, zenryaku, Moca, ryoji, naoh, KEITA, じゅうじ
	Copyright (C) 2005, genta, D.S.Koba, ryoji
	Copyright (C) 2007, ryoji, kobake
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2010, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "view/CRuler.h"
#include "uiparts/CWaitCursor.h"
#include "plugin/CJackManager.h"
#include "plugin/CSmartIndentIfObj.h"
#include "debug/CRunningTimer.h"


/* wchar_t1個分の文字を入力 */
void CViewCommander::Command_WCHAR( wchar_t wcChar, bool bConvertEOL )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	CLogicInt		nPos;
	CLogicInt		nCharChars;

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	if( m_pCommanderView->m_bHideMouse && 0 <= m_pCommanderView->m_nMousePouse ){
		m_pCommanderView->m_nMousePouse = -1;
		::SetCursor( NULL );
	}

	/* 現在位置にデータを挿入 */
	CNativeW cmemDataW2;
	cmemDataW2 = wcChar;
	if( WCODE::IsLineDelimiter(wcChar, GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){ 
		/* 現在、Enterなどで挿入する改行コードの種類を取得 */
		if( bConvertEOL ){
			CEol cWork = GetDocument()->m_cDocEditor.GetNewLineCode();
			cmemDataW2.SetString( cWork.GetValue2(), cWork.GetLen() );
		}

		/* テキストが選択されているか */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			m_pCommanderView->DeleteData( true );
		}
		if( m_pCommanderView->m_pTypeData->m_bAutoIndent ){	/* オートインデント */
			const CLayout* pCLayout;
			const wchar_t*	pLine;
			CLogicInt		nLineLen;
			pLine = GetDocument()->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pCLayout );
			if( NULL != pCLayout ){
				const CDocLine* pcDocLine;
				pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( pCLayout->GetLogicLineNo() );
				pLine = pcDocLine->GetDocLineStrWithEOL( &nLineLen );
				if( NULL != pLine ){
					/*
					  カーソル位置変換
					  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
					  →
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					*/
					CLogicPoint ptXY;
					GetDocument()->m_cLayoutMgr.LayoutToLogic(
						GetCaret().GetCaretLayoutPos(),
						&ptXY
					);

					/* 指定された桁に対応する行のデータ内の位置を調べる */
					for( nPos = CLogicInt(0); nPos < nLineLen && nPos < ptXY.GetX2(); ){
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );

						/* その他のインデント文字 */
						if( 0 < nCharChars
						 && pLine[nPos] != L'\0'	// その他のインデント文字に L'\0' は含まれない	// 2009.02.04 ryoji L'\0'がインデントされてしまう問題修正
						 && m_pCommanderView->m_pTypeData->m_szIndentChars[0] != L'\0'
						){
							wchar_t szCurrent[10];
							wmemcpy( szCurrent, &pLine[nPos], nCharChars );
							szCurrent[nCharChars] = L'\0';
							/* その他のインデント対象文字 */
							if( NULL != wcsstr(
								m_pCommanderView->m_pTypeData->m_szIndentChars,
								szCurrent
							) ){
								goto end_of_for;
							}
						}
						
						{
							bool bZenSpace=m_pCommanderView->m_pTypeData->m_bAutoIndent_ZENSPACE;
							if(nCharChars==1 && WCODE::IsIndentChar(pLine[nPos],bZenSpace))
							{
								//下へ進む
							}
							else break;
						}

end_of_for:;
						nPos += nCharChars;
					}

					//インデント取得
					//CNativeW cmemIndent;
					//cmemIndent.SetString( pLine, nPos );

					//インデント付加
					cmemDataW2.AppendString(pLine, nPos);
				}
			}
		}
	}
	else{
		/* テキストが選択されているか */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			/* 矩形範囲選択中か */
			if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
				Command_INDENT( wcChar );
				return;
			}else{
				m_pCommanderView->DeleteData( true );
			}
		}
		else{
			if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */ ){
				DelCharForOverwrite(&wcChar, 1);	// 上書き用の一文字削除	// 2009.04.11 ryoji
			}
		}
	}

	//本文に挿入する
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		cmemDataW2.GetStringPtr(),
		cmemDataW2.GetStringLength(),
		&ptLayoutNew,
		true
	);

	/* 挿入データの最後へカーソルを移動 */
	GetCaret().MoveCursor( ptLayoutNew, true );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	/* スマートインデント */
	ESmartIndentType nSIndentType = m_pCommanderView->m_pTypeData->m_eSmartIndent;
	switch( nSIndentType ){	/* スマートインデント種別 */
	case SMARTINDENT_NONE:
		break;
	case SMARTINDENT_CPP:
		/* C/C++スマートインデント処理 */
		m_pCommanderView->SmartIndent_CPP( wcChar );
		break;
	default:
		//プラグインから検索する
		{
			CPlug::Array plugs;
			CJackManager::getInstance()->GetUsablePlug( PP_SMARTINDENT, nSIndentType, &plugs );

			if( plugs.size() > 0 ){
				assert_warning( 1 == plugs.size() );
				//インタフェースオブジェクト準備
				CWSHIfObj::List params;
				CSmartIndentIfObj* objIndent = new CSmartIndentIfObj( wcChar );	//スマートインデントオブジェクト
				objIndent->AddRef();
				params.push_back( objIndent );

				//キー入力をアンドゥバッファに反映
				m_pCommanderView->SetUndoBuffer();

				//キー入力とは別の操作ブロックにする（ただしプラグイン内の操作はまとめる）
				if( GetOpeBlk() == NULL ){
					SetOpeBlk(new COpeBlk);
				}
				GetOpeBlk()->AddRef();	// ※ReleaseはHandleCommandの最後で行う

				//プラグイン呼び出し
				( *plugs.begin() )->Invoke( m_pCommanderView, params );
				objIndent->Release();
			}
		}
		break;
	}

	/* 2005.10.11 ryoji 改行時に末尾の空白を削除 */
	if( WCODE::IsLineDelimiter(wcChar, GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) && m_pCommanderView->m_pTypeData->m_bRTrimPrevLine ){	/* 改行時に末尾の空白を削除 */
		/* 前の行にある末尾の空白を削除する */
		m_pCommanderView->RTrimPrevLine();
	}

	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta 関数化
}



/*!
	@brief 2バイト文字入力
	
	WM_IME_CHARで送られてきた文字を処理する．
	ただし，挿入モードではWM_IME_CHARではなくWM_IME_COMPOSITIONで文字列を
	取得するのでここには来ない．

	@param wChar [in] SJIS漢字コード．上位が1バイト目，下位が2バイト目．
	
	@date 2002.10.06 genta 引数の上下バイトの意味を逆転．
		WM_IME_CHARのwParamに合わせた．
*/
void CViewCommander::Command_IME_CHAR( WORD wChar )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	//	Oct. 6 ,2002 genta 上下逆転
	if( 0 == (wChar & 0xff00) ){
		Command_WCHAR( wChar & 0xff );
		return;
	}
	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

 	if( m_pCommanderView->m_bHideMouse && 0 <= m_pCommanderView->m_nMousePouse ){
		m_pCommanderView->m_nMousePouse = -1;
		::SetCursor( NULL );
	}

	// Oct. 6 ,2002 genta バッファに格納する
	// Aug. 15, 2007 kobake WCHARバッファに変換する
#ifdef _UNICODE
	wchar_t szWord[2]={wChar,0};
#else
	ACHAR szAnsiWord[3]={(wChar >> 8) & 0xff, wChar & 0xff, 0};
	const wchar_t* pUniData = to_wchar(szAnsiWord);
	wchar_t szWord[2]={pUniData[0],0};
#endif
	CLogicInt nWord=CLogicInt(1);

	/* テキストが選択されているか */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 矩形範囲選択中か */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			Command_INDENT( szWord, nWord );	//	Oct. 6 ,2002 genta 
			return;
		}else{
			m_pCommanderView->DeleteData( true );
		}
	}
	else{
		if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */ ){
			DelCharForOverwrite(szWord, nWord);	// 上書き用の一文字削除	// 2009.04.11 ryoji
		}
	}

	//	Oct. 6 ,2002 genta 
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView( GetCaret().GetCaretLayoutPos(), szWord, nWord, &ptLayoutNew, true );

	/* 挿入データの最後へカーソルを移動 */
	GetCaret().MoveCursor( ptLayoutNew, true );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta 関数化
}



//	from CViewCommander_New.cpp
/* Undo 元に戻す */
void CViewCommander::Command_UNDO( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	{
		COpeBlk* opeBlk = m_pCommanderView->m_cCommander.GetOpeBlk();
		if( opeBlk ){
			int nCount = opeBlk->GetRefCount();
			opeBlk->SetRefCount(1); // 強制的にリセットするため1を指定
			m_pCommanderView->SetUndoBuffer();
			if( m_pCommanderView->m_cCommander.GetOpeBlk() == NULL && 0 < nCount ){
				m_pCommanderView->m_cCommander.SetOpeBlk(new COpeBlk());
				m_pCommanderView->m_cCommander.GetOpeBlk()->SetRefCount( nCount );
			}
		}
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

	CLayoutPoint ptCaretPos_Before;

	CLayoutPoint ptCaretPos_After;

	/* 各種モードの取り消し */
	Command_CANCEL_MODE();

	m_pCommanderView->m_bDoing_UndoRedo = true;	/* アンドゥ・リドゥの実行中か */

	/* 現在のUndo対象の操作ブロックを返す */
	if( NULL != ( pcOpeBlk = GetDocument()->m_cDocEditor.m_cOpeBuf.DoUndo( &bIsModified ) ) ){
		nOpeBlkNum = pcOpeBlk->GetNum();
		bool bDraw = (nOpeBlkNum < 5) && m_pCommanderView->GetDrawSwitch();
		bool bDrawAll = false;
		const bool bDrawSwitchOld = m_pCommanderView->SetDrawSwitch(bDraw);	// hor

		CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd(), 1000 < nOpeBlkNum );
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if( cWaitCursor.IsEnable() ){
			hwndProgress = m_pCommanderView->StartProgress();
		}

		const bool bFastMode = (100 < nOpeBlkNum);
		for( i = nOpeBlkNum - 1; i >= 0; i-- ){
			pcOpe = pcOpeBlk->GetOpe( i );
			if( bFastMode ){
				GetCaret().MoveCursorFastMode( pcOpe->m_ptCaretPos_PHY_After );
			}else{
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
			}

			switch( pcOpe->GetCode() ){
			case OPE_INSERT:
				{
					CInsertOpe* pcInsertOpe = static_cast<CInsertOpe*>(pcOpe);

					/* 選択範囲の変更 */
					CLogicRange cSelectLogic;
					cSelectLogic.SetFrom(pcOpe->m_ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pcOpe->m_ptCaretPos_PHY_After);
					if( bFastMode ){
					}else{
						m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetFrom(ptCaretPos_Before);
						m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetTo(m_pCommanderView->GetSelectionInfo().m_sSelectBgn.GetFrom());
						m_pCommanderView->GetSelectionInfo().m_sSelect.SetFrom(ptCaretPos_Before);
						m_pCommanderView->GetSelectionInfo().m_sSelect.SetTo(ptCaretPos_After);
					}

					/* データ置換 削除&挿入にも使える */
					bDrawAll |= m_pCommanderView->ReplaceData_CEditView3(
						m_pCommanderView->GetSelectionInfo().m_sSelect,				// 削除範囲
						&pcInsertOpe->m_cOpeLineData,	// 削除されたデータのコピー(NULL可能)
						NULL,
						bDraw,						// 再描画するか否か
						NULL,
						pcInsertOpe->m_nOrgSeq,
						NULL,
						bFastMode,
						&cSelectLogic
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
					if( 0 < pcDeleteOpe->m_cOpeLineData.size() ){
						/* データ置換 削除&挿入にも使える */
						CLayoutRange sRange;
						sRange.Set(ptCaretPos_Before);
						CLogicRange cSelectLogic;
						cSelectLogic.Set(pcOpe->m_ptCaretPos_PHY_Before);
						bDrawAll |= m_pCommanderView->ReplaceData_CEditView3(
							sRange,
							NULL,										/* 削除されたデータのコピー(NULL可能) */
							&pcDeleteOpe->m_cOpeLineData,
							bDraw,										/*再描画するか否か*/
							NULL,
							0,
							&pcDeleteOpe->m_nOrgSeq,
							bFastMode,
							&cSelectLogic
						);
					}
					pcDeleteOpe->m_cOpeLineData.clear();
				}
				break;
			case OPE_REPLACE:
				{
					CReplaceOpe* pcReplaceOpe = static_cast<CReplaceOpe*>(pcOpe);

					CLayoutRange sRange;
					sRange.SetFrom(ptCaretPos_Before);
					sRange.SetTo(ptCaretPos_After);
					CLogicRange cSelectLogic;
					cSelectLogic.SetFrom(pcOpe->m_ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pcOpe->m_ptCaretPos_PHY_After);

					/* データ置換 削除&挿入にも使える */
					bDrawAll |= m_pCommanderView->ReplaceData_CEditView3(
						sRange,				// 削除範囲
						&pcReplaceOpe->m_pcmemDataIns,	// 削除されたデータのコピー(NULL可能)
						&pcReplaceOpe->m_pcmemDataDel,	// 挿入するデータ
						bDraw,						// 再描画するか否か
						NULL,
						pcReplaceOpe->m_nOrgInsSeq,
						&pcReplaceOpe->m_nOrgDelSeq,
						bFastMode,
						&cSelectLogic
					);
					pcReplaceOpe->m_pcmemDataDel.clear();
				}
				break;
			case OPE_MOVECARET:
				/* カーソルを移動 */
				if( bFastMode ){
					GetCaret().MoveCursorFastMode( pcOpe->m_ptCaretPos_PHY_After );
				}else{
					GetCaret().MoveCursor( ptCaretPos_After, false );
				}
				break;
			}

			if( bFastMode ){
				if( i == 0 ){
					GetDocument()->m_cLayoutMgr._DoLayout(false);
					GetEditWindow()->ClearViewCaretPosInfo();
					if( GetDocument()->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
						GetDocument()->m_cLayoutMgr.CalculateTextWidth();
					}
					GetDocument()->m_cLayoutMgr.LogicToLayout(
						pcOpe->m_ptCaretPos_PHY_Before,
						&ptCaretPos_Before
					);
					GetCaret().MoveCursor( ptCaretPos_Before, true );
					// 通常モードではReplaceData_CEditViewの中で設定される
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
				}else{
					GetCaret().MoveCursorFastMode( pcOpe->m_ptCaretPos_PHY_Before );
				}
			}else{
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
			if( hwndProgress && (i % 100) == 0 ){
				int newPos = ::MulDiv(nOpeBlkNum - i, 100, nOpeBlkNum);
				if( newPos != nProgressPos ){
					nProgressPos = newPos;
					Progress_SetPos( hwndProgress, newPos + 1 );
					Progress_SetPos( hwndProgress, newPos );
				}
			}
		}
		m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);	//	hor
		m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
		if (!bDraw) {
			GetCaret().ShowEditCaret();
		}

		/* Undo後の変更フラグ */
		GetDocument()->m_cDocEditor.SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_pCommanderView->m_bDoing_UndoRedo = false;	/* アンドゥ・リドゥの実行中か */

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

		if( !GetEditWindow()->UpdateTextWrap() && bDrawAll ){	// 折り返し方法関連の更新	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews( m_pCommanderView );	//	他のペインの表示を更新
		}

		if(hwndProgress) ::ShowWindow( hwndProgress, SW_HIDE );
	}

	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji 追加
	m_pCommanderView->m_bDoing_UndoRedo = false;	/* アンドゥ・リドゥの実行中か */

	return;
}



//	from CViewCommander_New.cpp
/* Redo やり直し */
void CViewCommander::Command_REDO( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	{
		COpeBlk* opeBlk = m_pCommanderView->m_cCommander.GetOpeBlk();
		if( opeBlk ){
			int nCount = opeBlk->GetRefCount();
			opeBlk->SetRefCount(1); // 強制的にリセットするため1を指定
			m_pCommanderView->SetUndoBuffer();
			if( m_pCommanderView->m_cCommander.GetOpeBlk() == NULL && 0 < nCount ){
				m_pCommanderView->m_cCommander.SetOpeBlk(new COpeBlk());
				m_pCommanderView->m_cCommander.GetOpeBlk()->SetRefCount( nCount );
			}
		}
		// 注意：Opeを追加するとRedoはできなくなる
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

	CLayoutPoint ptCaretPos_Before;
	CLayoutPoint ptCaretPos_To;
	CLayoutPoint ptCaretPos_After;


	/* 各種モードの取り消し */
	Command_CANCEL_MODE();

	m_pCommanderView->m_bDoing_UndoRedo = true;	/* アンドゥ・リドゥの実行中か */

	/* 現在のRedo対象の操作ブロックを返す */
	if( NULL != ( pcOpeBlk = GetDocument()->m_cDocEditor.m_cOpeBuf.DoRedo( &bIsModified ) ) ){
		nOpeBlkNum = pcOpeBlk->GetNum();
		bool bDraw = (nOpeBlkNum < 5) && m_pCommanderView->GetDrawSwitch();
		bool bDrawAll = false;
		const bool bDrawSwitchOld = m_pCommanderView->SetDrawSwitch(bDraw);	// 2007.07.22 ryoji

		CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd(), 1000 < nOpeBlkNum );
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if( cWaitCursor.IsEnable() ){
			hwndProgress = m_pCommanderView->StartProgress();
		}

		const bool bFastMode = (100 < nOpeBlkNum);
		for( i = 0; i < nOpeBlkNum; ++i ){
			pcOpe = pcOpeBlk->GetOpe( i );
			if( bFastMode ){
				if( i == 0 ){
					GetDocument()->m_cLayoutMgr.LogicToLayout(
						pcOpe->m_ptCaretPos_PHY_Before,
						&ptCaretPos_Before
					);
					GetCaret().MoveCursor( ptCaretPos_Before, true );
				}else{
					GetCaret().MoveCursorFastMode( pcOpe->m_ptCaretPos_PHY_Before );
				}
			}else{
				GetDocument()->m_cLayoutMgr.LogicToLayout(
					pcOpe->m_ptCaretPos_PHY_Before,
					&ptCaretPos_Before
				);
				if( i == 0 ){
					GetCaret().MoveCursor( ptCaretPos_Before, true );
				}else{
					GetCaret().MoveCursor( ptCaretPos_Before, false );
				}
			}
			switch( pcOpe->GetCode() ){
			case OPE_INSERT:
				{
					CInsertOpe* pcInsertOpe = static_cast<CInsertOpe*>(pcOpe);

					//2007.10.17 kobake メモリリークしてました。修正。
					if( 0 < pcInsertOpe->m_cOpeLineData.size() ){
						/* データ置換 削除&挿入にも使える */
						CLayoutRange sRange;
						sRange.Set(ptCaretPos_Before);
						CLogicRange cSelectLogic;
						cSelectLogic.Set(pcOpe->m_ptCaretPos_PHY_Before);
						bDrawAll |= m_pCommanderView->ReplaceData_CEditView3(
							sRange,
							NULL,										/* 削除されたデータのコピー(NULL可能) */
							&pcInsertOpe->m_cOpeLineData,				/* 挿入するデータ */
							bDraw,										/*再描画するか否か*/
							NULL,
							0,
							&pcInsertOpe->m_nOrgSeq,
							bFastMode,
							&cSelectLogic
						);

					}
					pcInsertOpe->m_cOpeLineData.clear();
				}
				break;
			case OPE_DELETE:
				{
					CDeleteOpe* pcDeleteOpe = static_cast<CDeleteOpe*>(pcOpe);

					if( bFastMode ){
					}else{
						GetDocument()->m_cLayoutMgr.LogicToLayout(
							pcDeleteOpe->m_ptCaretPos_PHY_To,
							&ptCaretPos_To
						);
					}
					CLogicRange cSelectLogic;
					cSelectLogic.SetFrom(pcOpe->m_ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pcDeleteOpe->m_ptCaretPos_PHY_To);

					/* データ置換 削除&挿入にも使える */
					bDrawAll |= m_pCommanderView->ReplaceData_CEditView3(
						CLayoutRange(ptCaretPos_Before,ptCaretPos_To),
						&pcDeleteOpe->m_cOpeLineData,	/* 削除されたデータのコピー(NULL可能) */
						NULL,
						bDraw,
						NULL,
						pcDeleteOpe->m_nOrgSeq,
						NULL,
						bFastMode,
						&cSelectLogic
					);
				}
				break;
			case OPE_REPLACE:
				{
					CReplaceOpe* pcReplaceOpe = static_cast<CReplaceOpe*>(pcOpe);

					if( bFastMode ){
					}else{
						GetDocument()->m_cLayoutMgr.LogicToLayout(
							pcReplaceOpe->m_ptCaretPos_PHY_To,
							&ptCaretPos_To
						);
					}
					CLogicRange cSelectLogic;
					cSelectLogic.SetFrom(pcOpe->m_ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pcReplaceOpe->m_ptCaretPos_PHY_To);

					/* データ置換 削除&挿入にも使える */
					bDrawAll |= m_pCommanderView->ReplaceData_CEditView3(
						CLayoutRange(ptCaretPos_Before,ptCaretPos_To),
						&pcReplaceOpe->m_pcmemDataDel,	// 削除されたデータのコピー(NULL可能)
						&pcReplaceOpe->m_pcmemDataIns,	// 挿入するデータ
						bDraw,
						NULL,
						pcReplaceOpe->m_nOrgDelSeq,
						&pcReplaceOpe->m_nOrgInsSeq,
						bFastMode,
						&cSelectLogic
					);
					pcReplaceOpe->m_pcmemDataIns.clear();
				}
				break;
			case OPE_MOVECARET:
				break;
			}
			if( bFastMode ){
				if( i == nOpeBlkNum - 1	){
					GetDocument()->m_cLayoutMgr._DoLayout(false);
					GetEditWindow()->ClearViewCaretPosInfo();
					if( GetDocument()->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
						GetDocument()->m_cLayoutMgr.CalculateTextWidth();
					}
					GetDocument()->m_cLayoutMgr.LogicToLayout(
						pcOpe->m_ptCaretPos_PHY_After, &ptCaretPos_After );
					GetCaret().MoveCursor( ptCaretPos_After, true );
					// 通常モードではReplaceData_CEditViewの中で設定される
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
				}else{
					GetCaret().MoveCursorFastMode( pcOpe->m_ptCaretPos_PHY_After );
				}
			}else{
				GetDocument()->m_cLayoutMgr.LogicToLayout(
					pcOpe->m_ptCaretPos_PHY_After, &ptCaretPos_After );
				if( i == nOpeBlkNum - 1	){
					GetCaret().MoveCursor( ptCaretPos_After, true );
				}else{
					GetCaret().MoveCursor( ptCaretPos_After, false );
				}
			}
			if( hwndProgress && (i % 100) == 0 ){
				int newPos = ::MulDiv(i + 1, 100, nOpeBlkNum);
				if( newPos != nProgressPos ){
					nProgressPos = newPos;
					Progress_SetPos( hwndProgress, newPos + 1 );
					Progress_SetPos( hwndProgress, newPos );
				}
			}
		}
		m_pCommanderView->SetDrawSwitch(bDrawSwitchOld); // 2007.07.22 ryoji
		m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
		if (!bDraw) {
			GetCaret().ShowEditCaret();
		}

		/* Redo後の変更フラグ */
		GetDocument()->m_cDocEditor.SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_pCommanderView->m_bDoing_UndoRedo = false;	/* アンドゥ・リドゥの実行中か */

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

		if( !GetEditWindow()->UpdateTextWrap() && bDrawAll ){	// 折り返し方法関連の更新	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews( m_pCommanderView );	//	他のペインの表示を更新
		}

		if(hwndProgress) ::ShowWindow( hwndProgress, SW_HIDE );
	}

	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji 追加
	m_pCommanderView->m_bDoing_UndoRedo = false;	/* アンドゥ・リドゥの実行中か */

	return;
}



//カーソル位置または選択エリアを削除
void CViewCommander::Command_DELETE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){		/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		// 2008.08.03 nasukoji	選択範囲なしでDELETEを実行した場合、カーソル位置まで半角スペースを挿入した後改行を削除して次行と連結する
		if( GetDocument()->m_cLayoutMgr.GetLineCount() > GetCaret().GetCaretLayoutPos().GetY2() ){
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
			if( pcLayout ){
				CLayoutInt nLineLen;
				CLogicInt nIndex;
				nIndex = m_pCommanderView->LineColumnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
				if( nLineLen != 0 ){	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					if( EOL_NONE != pcLayout->GetLayoutEol().GetType() ){	// 行終端は改行コードか?
						Command_INSTEXT( true, L"", CLogicInt(0), FALSE );	// カーソル位置まで半角スペース挿入
					}else{	// 行終端が折り返し
						// 折り返し行末ではスペース挿入後、次の文字を削除する	// 2009.02.19 ryoji

						// フリーカーソル時の折り返し越え位置での削除はどうするのが妥当かよくわからないが
						// 非フリーカーソル時（ちょうどカーソルが折り返し位置にある）には次の行の先頭文字を削除したい

						if( nLineLen < GetCaret().GetCaretLayoutPos().GetX2() ){	// 折り返し行末とカーソルの間に隙間がある
							Command_INSTEXT( true, L"", CLogicInt(0), FALSE );	// カーソル位置まで半角スペース挿入
							pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
							nIndex = m_pCommanderView->LineColumnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
						}
						if( nLineLen != 0 ){	// （スペース挿入後も）折り返し行末なら次文字を削除するために次行の先頭に移動する必要がある
							if( pcLayout->GetNextLayout() != NULL ){	// 最終行末ではない
								CLayoutPoint ptLay;
								CLogicPoint ptLog(pcLayout->GetLogicOffset() + nIndex, pcLayout->GetLogicLineNo());
								GetDocument()->m_cLayoutMgr.LogicToLayout( ptLog, &ptLay );
								GetCaret().MoveCursor( ptLay, true );
								GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
							}
						}
					}
				}
			}
		}
	}
	m_pCommanderView->DeleteData( true );
	return;
}



//カーソル前を削除
void CViewCommander::Command_DELETE_BACK( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	//	May 29, 2004 genta 実際に削除された文字がないときはフラグをたてないように
	//GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){				/* テキストが選択されているか */
		m_pCommanderView->DeleteData( true );
	}
	else{
		CLayoutPoint	ptLayoutPos_Old = GetCaret().GetCaretLayoutPos();
		CLogicPoint		ptLogicPos_Old = GetCaret().GetCaretLogicPos();
		BOOL	bBool = Command_LEFT( false, false );
		if( bBool ){
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
			if( pcLayout ){
				CLayoutInt nLineLen;
				CLogicInt nIdx = m_pCommanderView->LineColumnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
				if( nLineLen == 0 ){	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					// 右からの移動では折り返し末尾文字は削除するが改行は削除しない
					// 下から（下の行の行頭から）の移動では改行も削除する
					if( nIdx < pcLayout->GetLengthWithoutEOL() || GetCaret().GetCaretLayoutPos().GetY2() < ptLayoutPos_Old.GetY2() ){
						if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
							/* 操作の追加 */
							GetOpeBlk()->AppendOpe(
								new CMoveCaretOpe(
									ptLogicPos_Old,
									GetCaret().GetCaretLogicPos()
								)
							);
						}
						m_pCommanderView->DeleteData( true );
					}
				}
			}
		}
	}
	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta 関数化
}



/* 	上書き用の一文字削除	2009.04.11 ryoji */
void CViewCommander::DelCharForOverwrite( const wchar_t* pszInput, int nLen )
{
	bool bEol = false;
	BOOL bDelete = TRUE;
	const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	int nDelLen = 0;
	CKetaXInt nKetaDiff = CKetaXInt(0);
	CKetaXInt nKetaAfterIns = CKetaXInt(0);
	if( NULL != pcLayout ){
		/* 指定された桁に対応する行のデータ内の位置を調べる */
		CLogicInt nIdxTo = m_pCommanderView->LineColumnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
		if( nIdxTo >= pcLayout->GetLengthWithoutEOL() ){
			bEol = true;	// 現在位置は改行または折り返し以後
			if( pcLayout->GetLayoutEol() != EOL_NONE ){
				if( GetDllShareData().m_Common.m_sEdit.m_bNotOverWriteCRLF ){	/* 改行は上書きしない */
					/* 現在位置が改行ならば削除しない */
					bDelete = FALSE;
				}
			}
		}else{
			// 文字幅に合わせてスペースを詰める
			if( GetDllShareData().m_Common.m_sEdit.m_bOverWriteFixMode ){
				const CStringRef line = pcLayout->GetDocLineRef()->GetStringRefWithEOL();
				CLogicInt nPos = GetCaret().GetCaretLogicPos().GetX();
				if( line.At(nPos) != WCODE::TAB ){
					CKetaXInt nKetaBefore = CNativeW::GetKetaOfChar(line, nPos);
					CKetaXInt nKetaAfter = CNativeW::GetKetaOfChar(pszInput, nLen, 0);
					nKetaDiff = nKetaBefore - nKetaAfter;
					nPos += CNativeW::GetSizeOfChar(line.GetPtr(), line.GetLength(), nPos);
					nDelLen = 1;
					if( nKetaDiff < 0 && nPos < line.GetLength() ){
						wchar_t c = line.At(nPos);
						if( c != WCODE::TAB && !WCODE::IsLineDelimiter(c,
								GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
							nDelLen = 2;
							CKetaXInt nKetaBefore2 = CNativeW::GetKetaOfChar(line, nPos);
							nKetaAfterIns = nKetaBefore + nKetaBefore2 - nKetaAfter;
						}
					}
				}
			}
		}
	}
	if( bDelete ){
		/* 上書きモードなので、現在位置の文字を１文字消去 */
		CLayoutPoint posBefore;
		if( bEol ){
			Command_DELETE();	//行数減では再描画が必要＆行末以後の削除を処理統一
			posBefore = GetCaret().GetCaretLayoutPos();
		}else{
			// 1文字削除
			m_pCommanderView->DeleteData( false );
			posBefore = GetCaret().GetCaretLayoutPos();
			for(int i = 1; i < nDelLen; i++){
				m_pCommanderView->DeleteData( false );
			}
		}
		CNativeW tmp;
		for(CKetaXInt i = CKetaXInt(0); i < nKetaDiff; i++){
			tmp.AppendString(L" ");
		}
		for(CKetaXInt i = CKetaXInt(0); i < nKetaAfterIns; i++){
			tmp.AppendString(L" ");
		}
		if( 0 < tmp.GetStringLength() ){
			Command_INSTEXT( false, tmp.GetStringPtr(), tmp.GetStringLength(), false, false);
			GetCaret().MoveCursor(posBefore, false);
		}
	}
}
