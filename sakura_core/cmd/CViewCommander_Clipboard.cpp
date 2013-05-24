/*!	@file
@brief CViewCommanderクラスのコマンド(クリップボード系)関数群

	2012/12/20	CViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, novice
	Copyright (C) 2002, hor, genta, Azumaiya, すなふき
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2007, ryoji
	Copyright (C) 2010, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "uiparts/CWaitCursor.h"
#include "util/os.h"


/** 切り取り(選択範囲をクリップボードにコピーして削除)

	@date 2007.11.18 ryoji 「選択なしでコピーを可能にする」オプション処理追加
*/
void CViewCommander::Command_CUT( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	CNativeW	cmemBuf;
	bool	bBeginBoxSelect;
	/* 範囲選択がされていない */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 非選択時は、カーソル行を切り取り */
		if( !GetDllShareData().m_Common.m_sEdit.m_bEnableNoSelectCopy ){	// 2007.11.18 ryoji
			return;	// 何もしない（音も鳴らさない）
		}
		//行切り取り(折り返し単位)
		Command_CUT_LINE();
		return;
	}
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		bBeginBoxSelect = true;
	}else{
		bBeginBoxSelect = false;
	}

	/* 選択範囲のデータを取得 */
	/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
	if( !m_pCommanderView->GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		ErrorBeep();
		return;
	}
	/* クリップボードにデータを設定 */
	if( !m_pCommanderView->MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), bBeginBoxSelect ) ){
		ErrorBeep();
		return;
	}

	/* カーソル位置または選択エリアを削除 */
	m_pCommanderView->DeleteData( true );
	return;
}



/**	選択範囲をクリップボードにコピー

	@date 2007.11.18 ryoji 「選択なしでコピーを可能にする」オプション処理追加
*/
void CViewCommander::Command_COPY(
	bool		bIgnoreLockAndDisable,	//!< [in] 選択範囲を解除するか？
	bool		bAddCRLFWhenCopy,		//!< [in] 折り返し位置に改行コードを挿入するか？
	EEolType	neweol					//!< [in] コピーするときのEOL。
)
{
	CNativeW	cmemBuf;
	bool		bBeginBoxSelect = false;

	/* クリップボードに入れるべきテキストデータを、cmemBufに格納する */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 非選択時は、カーソル行をコピーする */
		if( !GetDllShareData().m_Common.m_sEdit.m_bEnableNoSelectCopy ){	// 2007.11.18 ryoji
			return;	// 何もしない（音も鳴らさない）
		}
		m_pCommanderView->CopyCurLine(
			bAddCRLFWhenCopy,
			neweol,
			GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste
		);
	}
	else{
		/* テキストが選択されているときは、選択範囲のデータを取得 */

		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			bBeginBoxSelect = TRUE;
		}
		/* 選択範囲のデータを取得 */
		/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
		if( !m_pCommanderView->GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, bAddCRLFWhenCopy, neweol ) ){
			ErrorBeep();
			return;
		}

		/* クリップボードにデータcmemBufの内容を設定 */
		if( !m_pCommanderView->MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), bBeginBoxSelect, FALSE ) ){
			ErrorBeep();
			return;
		}
	}

	/* 選択範囲の後片付け */
	if( !bIgnoreLockAndDisable ){
		/* 選択状態のロック */
		if( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = FALSE;
			m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
			if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
				GetCaret().m_cUnderLine.CaretUnderLineON(true, false);
			}
		}
	}
	if( GetDllShareData().m_Common.m_sEdit.m_bCopyAndDisablSelection ){	/* コピーしたら選択解除 */
		/* テキストが選択されているか */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			/* 現在の選択範囲を非選択状態に戻す */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );
		}
	}
	return;
}



/** 貼り付け(クリップボードから貼り付け)
	@param [in] option 貼り付け時のオプション
	@li 0x01 改行コード変換有効
	@li 0x02 改行コード変換無効
	@li 0x04 ラインモード貼り付け有効
	@li 0x08 ラインモード貼り付け無効
	@li 0x10 矩形コピーは常に矩形貼り付け
	@li 0x20 矩形コピーは常に通常貼り付け

	@date 2007.10.04 ryoji MSDEVLineSelect形式の行コピー対応処理を追加（VS2003/2005のエディタと類似の挙動に）
*/
void CViewCommander::Command_PASTE( int option )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	//砂時計
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	// クリップボードからデータを取得 -> cmemClip, bColmnSelect
	CNativeW	cmemClip;
	bool		bColumnSelect;
	bool		bLineSelect = false;
	bool		bLineSelectOption = 
		((option & 0x04) == 0x04) ? true :
		((option & 0x08) == 0x08) ? false :
		GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste;

	if( !m_pCommanderView->MyGetClipboardData( cmemClip, &bColumnSelect, bLineSelectOption ? &bLineSelect: NULL ) ){
		ErrorBeep();
		return;
	}

	// クリップボードデータ取得 -> pszText, nTextLen
	CLogicInt		nTextLen;
	const wchar_t*	pszText = cmemClip.GetStringPtr(&nTextLen);

	bool bConvertEol = 
		((option & 0x01) == 0x01) ? true :
		((option & 0x02) == 0x02) ? false :
		GetDllShareData().m_Common.m_sEdit.m_bConvertEOLPaste;

	bool bAutoColumnPaste = 
		((option & 0x10) == 0x10) ? true :
		((option & 0x20) == 0x20) ? false :
		GetDllShareData().m_Common.m_sEdit.m_bAutoColumnPaste != FALSE;

	// 矩形コピーのテキストは常に矩形貼り付け
	if( bAutoColumnPaste ){
		// 矩形コピーのデータなら矩形貼り付け
		if( bColumnSelect ){
			if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){
				ErrorBeep();
				return;
			}
			if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH ){
				return;
			}
			Command_PASTEBOX(pszText, nTextLen);
			m_pCommanderView->AdjustScrollBars();
			m_pCommanderView->Redraw();
			return;
		}
	}

	// 2007.10.04 ryoji
	// 行コピー（MSDEVLineSelect形式）のテキストで末尾が改行になっていなければ改行を追加する
	// ※レイアウト折り返しの行コピーだった場合は末尾が改行になっていない
	if( bLineSelect ){
		if( pszText[nTextLen - 1] != WCODE::CR && pszText[nTextLen - 1] != WCODE::LF ){
			cmemClip.AppendString(GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2());
			pszText = cmemClip.GetStringPtr( &nTextLen );
		}
	}

	if( bConvertEol ){
		wchar_t	*pszConvertedText = new wchar_t[nTextLen * 2]; // 全文字\n→\r\n変換で最大の２倍になる
		CLogicInt nConvertedTextLen = ConvertEol( pszText, nTextLen, pszConvertedText );
		// テキストを貼り付け
		Command_INSTEXT( true, pszConvertedText, nConvertedTextLen, true, bLineSelect );	// 2010.09.17 ryoji
		delete [] pszConvertedText;
	}else{
		// テキストを貼り付け
		Command_INSTEXT( true, pszText, nTextLen, true, bLineSelect );	// 2010.09.17 ryoji
	}

	return;
}



//<< 2002/03/28 Azumaiya
// メモリデータを矩形貼り付け用のデータと解釈して処理する。
//  なお、この関数は Command_PASTEBOX(void) と、
// 2769 : GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta
// から、
// 3057 : m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
// 間まで、一緒です。
//  ですが、コメントを削ったり、#if 0 のところを削ったりしていますので、Command_PASTEBOX(void) は
// 残すようにしました(下にこの関数を使った使ったバージョンをコメントで書いておきました)。
//  なお、以下にあげるように Command_PASTEBOX(void) と違うところがあるので注意してください。
// > 呼び出し側が責任を持って、
// ・マウスによる範囲選択中である。
// ・現在のフォントは固定幅フォントである。
// の 2 点をチェックする。
// > 再描画を行わない
// です。
//  なお、これらを呼び出し側に期待するわけは、「すべて置換」のような何回も連続で呼び出す
// ときに、最初に一回チェックすればよいものを何回もチェックするのは無駄と判断したためです。
// @note 2004.06.30 現在、すべて置換では使用していない
void CViewCommander::Command_PASTEBOX( const wchar_t *szPaste, int nPasteSize )
{
	/* これらの動作は残しておきたいのだが、呼び出し側で責任を持ってやってもらうことに変更。
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )	// マウスによる範囲選択中
	{
		ErrorBeep();
		return;
	}
	if( !GetDllShareData().m_Common.m_bFontIs_FIXED_PITCH )	// 現在のフォントは固定幅フォントである
	{
		return;
	}
	*/

	int				nBgn;
	int				nPos;
	CLayoutInt		nCount;
	CLayoutPoint	ptLayoutNew;	//挿入された部分の次の位置
	BOOL			bAddLastCR;
	CLayoutInt		nInsPosX;

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	m_pCommanderView->SetDrawSwitch(false);	// 2002.01.25 hor

	// とりあえず選択範囲を削除
	// 2004.06.30 Moca m_pCommanderView->GetSelectionInfo().IsTextSelected()がないと未選択時、一文字消えてしまう
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		m_pCommanderView->DeleteData( false/*true 2002.01.25 hor*/ );
	}

	CLayoutPoint ptCurOld = GetCaret().GetCaretLayoutPos();

	nCount = CLayoutInt(0);

	// Jul. 10, 2005 genta 貼り付けデータの最後にCR/LFが無い場合の対策
	//	データの最後まで処理 i.e. nBgnがnPasteSizeを超えたら終了
	//for( nPos = 0; nPos < nPasteSize; )
	for( nBgn = nPos = 0; nBgn < nPasteSize; )
	{
		// Jul. 10, 2005 genta 貼り付けデータの最後にCR/LFが無いと
		//	最終行のPaste処理が動かないので，
		//	データの末尾に来た場合は強制的に処理するようにする
		if( szPaste[nPos] == WCODE::CR || szPaste[nPos] == WCODE::LF || nPos == nPasteSize )
		{
			/* 現在位置にデータを挿入 */
			if( nPos - nBgn > 0 ){
				m_pCommanderView->InsertData_CEditView(
					ptCurOld + CLayoutPoint(CLayoutInt(0), nCount),
					&szPaste[nBgn],
					nPos - nBgn,
					&ptLayoutNew,
					false
				);
			}

			/* この行の挿入位置へカーソルを移動 */
			GetCaret().MoveCursor( ptCurOld + CLayoutPoint(CLayoutInt(0), nCount), false );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			/* カーソル行が最後の行かつ行末に改行が無く、挿入すべきデータがまだある場合 */
			bAddLastCR = FALSE;
			const CLayout*	pcLayout;
			CLogicInt		nLineLen = CLogicInt(0);
			const wchar_t*	pLine;
			pLine = GetDocument()->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );

			if( NULL != pLine && 1 <= nLineLen )
			{
				if( pLine[nLineLen - 1] == WCODE::CR || pLine[nLineLen - 1] == WCODE::LF )
				{
				}
				else
				{
					bAddLastCR = TRUE;
				}
			}
			else
			{ // 2001/10/02 novice
				bAddLastCR = TRUE;
			}

			if( bAddLastCR )
			{
//				MYTRACE( _T(" カーソル行が最後の行かつ行末に改行が無く、\n挿入すべきデータがまだある場合は行末に改行を挿入。\n") );
				nInsPosX = m_pCommanderView->LineIndexToColumn( pcLayout, nLineLen );

				m_pCommanderView->InsertData_CEditView(
					CLayoutPoint(nInsPosX, GetCaret().GetCaretLayoutPos().GetY2()),
					GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2(),
					GetDocument()->m_cDocEditor.GetNewLineCode().GetLen(),
					&ptLayoutNew,
					false
				);
			}

			if(
				(nPos + 1 < nPasteSize ) &&
				 ( szPaste[nPos] == L'\r' && szPaste[nPos + 1] == L'\n')
			  )
			{
				nBgn = nPos + 2;
			}
			else
			{
				nBgn = nPos + 1;
			}

			nPos = nBgn;
			++nCount;
		}
		else
		{
			++nPos;
		}
	}

	/* 挿入データの先頭位置へカーソルを移動 */
	GetCaret().MoveCursor( ptCurOld, true );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	if( !m_pCommanderView->m_bDoing_UndoRedo )	/* アンドゥ・リドゥの実行中か */
	{
		/* 操作の追加 */
		GetOpeBlk()->AppendOpe( 
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
				GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
			)
		);
	}

	m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
	return;
}



/** 矩形貼り付け(クリップボードから矩形貼り付け)
	@param [in] option 未使用

	@date 2004.06.29 Moca 未使用だったものを有効にする
	オリジナルのCommand_PASTEBOX(void)はばっさり削除 (genta)
*/
void CViewCommander::Command_PASTEBOX( int option )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )	// マウスによる範囲選択中
	{
		ErrorBeep();
		return;
	}

	if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH )	// 現在のフォントは固定幅フォントである
	{
		return;
	}

	// クリップボードからデータを取得
	CNativeW	cmemClip;
	if( !m_pCommanderView->MyGetClipboardData( cmemClip, NULL ) ){
		ErrorBeep();
		return;
	}
	// 2004.07.13 Moca \0コピー対策
	int nstrlen;
	const wchar_t *lptstr = cmemClip.GetStringPtr( &nstrlen );

	Command_PASTEBOX(lptstr, nstrlen);
	m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
	m_pCommanderView->Redraw();			// 2002.01.25 hor
}



/*! 矩形文字列挿入
*/
void CViewCommander::Command_INSBOXTEXT( const wchar_t *pszPaste, int nPasteSize )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )	// マウスによる範囲選択中
	{
		ErrorBeep();
		return;
	}

	if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH )	// 現在のフォントは固定幅フォントである
	{
		return;
	}

	Command_PASTEBOX(pszPaste, nPasteSize);
	m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
	m_pCommanderView->Redraw();			// 2002.01.25 hor
}



/*! テキストを貼り付け
	@date 2004.05.14 Moca '\\0'を受け入れるように、引数に長さを追加
	@date 2010.09.17 ryoji ラインモード貼り付けオプションを追加して以前の Command_PASTE() との重複部を整理・統合
*/
void CViewCommander::Command_INSTEXT(
	bool			bRedraw,		//!< 
	const wchar_t*	pszText,		//!< [in] 貼り付ける文字列。
	CLogicInt		nTextLen,		//!< [in] pszTextの長さ。-1を指定すると、pszTextをNUL終端文字列とみなして長さを自動計算する
	bool			bNoWaitCursor,	//!< 
	bool			bLinePaste		//!< [in] ラインモード貼り付け
)
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	CWaitCursor*	pcWaitCursor;
	if( bNoWaitCursor ){
		pcWaitCursor = NULL;
	}else{
		pcWaitCursor = new CWaitCursor( m_pCommanderView->GetHwnd() );
	}

	if( nTextLen < 0 ){
		nTextLen = CLogicInt(wcslen( pszText ));
	}

	GetDocument()->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta

	// テキストが選択されているか
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		// 矩形範囲選択中か
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			//改行までを抜き出す
			CLogicInt i;
			for( i = CLogicInt(0); i < nTextLen; i++ ){
				if( pszText[i] == WCODE::CR || pszText[i] == WCODE::LF ){
					break;
				}
			}
			Command_INDENT( pszText, i );
			goto end_of_func;
		}
		else{
			//	Jun. 23, 2000 genta
			//	同一行の行末以降のみが選択されている場合には選択無しと見なす
			CLogicInt		len;
			int pos;
			const wchar_t	*line;
			const CLayout* pcLayout;
			line = GetDocument()->m_cLayoutMgr.GetLineStr( GetSelect().GetFrom().GetY2(), &len, &pcLayout );

			pos = ( line == NULL ) ? 0 : m_pCommanderView->LineColumnToIndex( pcLayout, GetSelect().GetFrom().GetX2() );

			//	開始位置が行末より後ろで、終了位置が同一行
			if( pos >= len && GetSelect().IsLineOne()){
				GetCaret().SetCaretLayoutPos(CLayoutPoint(GetSelect().GetFrom().x, GetCaret().GetCaretLayoutPos().y)); //キャレットX変更
				m_pCommanderView->GetSelectionInfo().DisableSelectArea(false);
			}
			else{
				// データ置換 削除&挿入にも使える
				// 行コピーの貼り付けでは選択範囲は削除（後で行頭に貼り付ける）	// 2007.10.04 ryoji
				m_pCommanderView->ReplaceData_CEditView(
					GetSelect(),				// 選択範囲
					NULL,					// 削除されたデータのコピー(NULL可能)
					bLinePaste? L"": pszText,	// 挿入するデータ
					bLinePaste? CLogicInt(0): nTextLen,	// 挿入するデータの長さ
					bRedraw,
					m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
				);
				if( !bLinePaste )	// 2007.10.04 ryoji
					goto end_of_func;
			}
		}
	}

	{	// 非選択時の処理 or ラインモード貼り付け時の残りの処理
		CLogicInt	nPosX_PHY_Delta(0);
		if( bLinePaste ){	// 2007.10.04 ryoji
			/* 挿入ポイント（折り返し単位行頭）にカーソルを移動 */
			CLogicPoint ptCaretBefore = GetCaret().GetCaretLogicPos();	// 操作前のキャレット位置
			Command_GOLINETOP( false, 1 );								// 行頭に移動(折り返し単位)
			CLogicPoint ptCaretAfter = GetCaret().GetCaretLogicPos();	// 操作後のキャレット位置

			// 挿入ポイントと元の位置との差分文字数
			nPosX_PHY_Delta = ptCaretBefore.x - ptCaretAfter.x;

			//UNDO用記録
			if( !m_pCommanderView->m_bDoing_UndoRedo ){
				GetOpeBlk()->AppendOpe(
					new CMoveCaretOpe(
						ptCaretBefore,	/* 操作前のキャレット位置 */
						ptCaretAfter	/* 操作後のキャレット位置 */
					)
				);
			}
		}

		// 現在位置にデータを挿入
		CLayoutPoint ptLayoutNew; //挿入された部分の次の位置
		m_pCommanderView->InsertData_CEditView(
			GetCaret().GetCaretLayoutPos(),
			pszText,
			nTextLen,
			&ptLayoutNew,
			bRedraw
		);

		// 挿入データの最後へカーソルを移動
		GetCaret().MoveCursor( ptLayoutNew, bRedraw );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if( bLinePaste ){	// 2007.10.04 ryoji
			/* 元の位置へカーソルを移動 */
			CLogicPoint ptCaretBefore = GetCaret().GetCaretLogicPos();	//操作前のキャレット位置
			CLayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				ptCaretBefore + CLogicPoint(nPosX_PHY_Delta, CLogicInt(0)),
				&ptLayout
			);
			GetCaret().MoveCursor( ptLayout, bRedraw );					//カーソル移動
			CLogicPoint ptCaretAfter = GetCaret().GetCaretLogicPos();	//操作後のキャレット位置
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;

			//UNDO用記録
			if( !m_pCommanderView->m_bDoing_UndoRedo ){
				GetOpeBlk()->AppendOpe(
					new CMoveCaretOpe(
						ptCaretBefore,	/* 操作前のキャレット位置Ｘ */
						ptCaretAfter	/* 操作後のキャレット位置Ｘ */
					)
				);
			}
		}
	}

end_of_func:
	delete pcWaitCursor;

	return;
}



/* 最後にテキストを追加 */
void CViewCommander::Command_ADDTAIL(
	const wchar_t*	pszData,	//!< 追加するテキスト
	int				nDataLen	//!< 追加するテキストの長さ。文字単位。-1を指定すると、テキスト終端まで。
)
{
	//テキスト長自動計算
	if(nDataLen==-1 && pszData!=NULL)nDataLen=wcslen(pszData);

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	/*ファイルの最後に移動 */
	Command_GOFILEEND( false );

	/* 現在位置にデータを挿入 */
	CLayoutPoint ptLayoutNew;	// 挿入された部分の次の位置
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		pszData,
		nDataLen,
		&ptLayoutNew,
		true
	);

	/* 挿入データの最後へカーソルを移動 */
	// Sep. 2, 2002 すなふき アンダーラインの表示が残ってしまう問題を修正
	GetCaret().MoveCursor( ptLayoutNew, true );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
}



//選択範囲内全行コピー
void CViewCommander::Command_COPYLINES( void )
{
	/* 選択範囲内の全行をクリップボードにコピーする */
	m_pCommanderView->CopySelectedAllLines(
		NULL,	/* 引用符 */
		FALSE	/* 行番号を付与する */
	);
	return;
}



//選択範囲内全行引用符付きコピー
void CViewCommander::Command_COPYLINESASPASSAGE( void )
{
	/* 選択範囲内の全行をクリップボードにコピーする */
	m_pCommanderView->CopySelectedAllLines(
		GetDllShareData().m_Common.m_sFormat.m_szInyouKigou,	/* 引用符 */
		FALSE 									/* 行番号を付与する */
	);
	return;
}



//選択範囲内全行行番号付きコピー
void CViewCommander::Command_COPYLINESWITHLINENUMBER( void )
{
	/* 選択範囲内の全行をクリップボードにコピーする */
	m_pCommanderView->CopySelectedAllLines(
		NULL,	/* 引用符 */
		TRUE	/* 行番号を付与する */
	);
	return;
}



/*!	現在編集中のファイル名をクリップボードにコピー
	2002/2/3 aroka
*/
void CViewCommander::Command_COPYFILENAME( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* クリップボードにデータを設定 */
		const WCHAR* pszFile = to_wchar(GetDocument()->m_cDocFile.GetFileName());
		m_pCommanderView->MySetClipboardData( pszFile , wcslen( pszFile ), false );
	}
	else{
		ErrorBeep();
	}
}



/* 現在編集中のファイルのパス名をクリップボードにコピー */
void CViewCommander::Command_COPYPATH( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* クリップボードにデータを設定 */
		const TCHAR* szPath = GetDocument()->m_cDocFile.GetFilePath();
		m_pCommanderView->MySetClipboardData( szPath, _tcslen(szPath), false );
	}
	else{
		ErrorBeep();
	}
}



//	May 9, 2000 genta
/* 現在編集中のファイルのパス名とカーソル位置をクリップボードにコピー */
void CViewCommander::Command_COPYTAG( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		wchar_t	buf[ MAX_PATH + 20 ];

		CLogicPoint ptColLine;

		//	論理行番号を得る
		GetDocument()->m_cLayoutMgr.LayoutToLogic( GetCaret().GetCaretLayoutPos(), &ptColLine );

		/* クリップボードにデータを設定 */
		auto_sprintf( buf, L"%ts (%d,%d): ", GetDocument()->m_cDocFile.GetFilePath(), ptColLine.y+1, ptColLine.x+1 );
		m_pCommanderView->MySetClipboardData( buf, wcslen( buf ), false );
	}
	else{
		ErrorBeep();
	}
}



////キー割り当て一覧をコピー
	//Dec. 26, 2000 JEPRO //Jan. 24, 2001 JEPRO debug version (directed by genta)
void CViewCommander::Command_CREATEKEYBINDLIST( void )
{
	CNativeW		cMemKeyList;

	CKeyBind::CreateKeyBindList(
		G_AppInstance(),
		GetDllShareData().m_Common.m_sKeyBind.m_nKeyNameArrNum,
		GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr,
		cMemKeyList,
		&GetDocument()->m_cFuncLookup,	//	Oct. 31, 2001 genta 追加
		FALSE	// 2007.02.22 ryoji 追加
	);

	// Windowsクリップボードにコピー
	//2004.02.17 Moca 関数化
	SetClipboardText( CEditWnd::getInstance()->m_cSplitterWnd.GetHwnd(), cMemKeyList.GetStringPtr(), cMemKeyList.GetStringLength() );
}
