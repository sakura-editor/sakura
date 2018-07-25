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
#include "view/colors/CColorStrategy.h"
#include "view/colors/CColor_Found.h"
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
	cmemBuf.Clear();

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
	cmemBuf.Clear();

	/* 選択範囲の後片付け */
	if( !bIgnoreLockAndDisable ){
		/* 選択状態のロック */
		if( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = false;
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

	// クリップボードからデータを取得 -> cmemClip, bColumnSelect
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
		if( !WCODE::IsLineDelimiter(pszText[nTextLen - 1], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
			cmemClip.AppendString(GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2());
			pszText = cmemClip.GetStringPtr( &nTextLen );
		}
	}

	if( bConvertEol ){
		CLogicInt nConvertedTextLen = ConvertEol( pszText, nTextLen, NULL );
		wchar_t	*pszConvertedText = new wchar_t[nConvertedTextLen];
		ConvertEol( pszText, nTextLen, pszConvertedText );
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
	CLayoutPoint	ptLayoutNew;	//挿入された部分の次の位置
	BOOL			bAddLastCR;
	CLayoutInt		nInsPosX;

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	bool bDrawSwitchOld = m_pCommanderView->SetDrawSwitch(false);	// 2002.01.25 hor

	// とりあえず選択範囲を削除
	// 2004.06.30 Moca m_pCommanderView->GetSelectionInfo().IsTextSelected()がないと未選択時、一文字消えてしまう
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		m_pCommanderView->DeleteData( false/*true 2002.01.25 hor*/ );
	}

	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd(), 10000 < nPasteSize );
	HWND hwndProgress = NULL;
	int nProgressPos = 0;
	if( cWaitCursor.IsEnable() ){
		hwndProgress = m_pCommanderView->StartProgress();
	}

	CLayoutPoint ptCurOld = GetCaret().GetCaretLayoutPos();

	CLayoutYInt	nCount = CLayoutInt(0); // カーソル位置からのオフセット行数
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	// Jul. 10, 2005 genta 貼り付けデータの最後にCR/LFが無い場合の対策
	//	データの最後まで処理 i.e. nBgnがnPasteSizeを超えたら終了
	//for( nPos = 0; nPos < nPasteSize; )
	for( nBgn = nPos = 0; nBgn < nPasteSize; )
	{
		// Jul. 10, 2005 genta 貼り付けデータの最後にCR/LFが無いと
		//	最終行のPaste処理が動かないので，
		//	データの末尾に来た場合は強制的に処理するようにする
		if( WCODE::IsLineDelimiter(szPaste[nPos], bExtEol) || nPos == nPasteSize )
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
				if( WCODE::IsLineDelimiter(pLine[nLineLen - 1], bExtEol) )
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
		if( (nPos % 100) == 0 && hwndProgress ){
			int newPos = ::MulDiv(nPos, 100, nPasteSize);
			if( newPos != nProgressPos ){
				nProgressPos = newPos;
				Progress_SetPos( hwndProgress, newPos + 1 );
				Progress_SetPos( hwndProgress, newPos );
			}
		}
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}

	/* 挿入データの先頭位置へカーソルを移動 */
	GetCaret().MoveCursor( ptCurOld, true );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	if( !m_pCommanderView->m_bDoing_UndoRedo )	/* アンドゥ・リドゥの実行中か */
	{
		/* 操作の追加 */
		GetOpeBlk()->AppendOpe( 
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}

	m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);	// 2002.01.25 hor
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
	@date 2013.05.10 Moca 高速モード
*/
void CViewCommander::Command_INSTEXT(
	bool			bRedraw,		//!< 
	const wchar_t*	pszText,		//!< [in] 貼り付ける文字列。
	CLogicInt		nTextLen,		//!< [in] pszTextの長さ。-1を指定すると、pszTextをNUL終端文字列とみなして長さを自動計算する
	bool			bNoWaitCursor,	//!< 
	bool			bLinePaste,		//!< [in] ラインモード貼り付け
	bool			bFastMode,		//!< [in] 高速モード(レイアウト座標は無視する)
	const CLogicRange*	pcSelectLogic	//!< [in] オプション。高速モードのときの削除範囲ロジック単位
)
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	if( nTextLen < 0 ){
		nTextLen = CLogicInt(wcslen( pszText ));
	}

	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd(),
		10000 < nTextLen && !m_pCommanderView->GetSelectionInfo().IsBoxSelecting() );

	GetDocument()->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta

	// テキストが選択されているか
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() || bFastMode ){
		// 矩形範囲選択中か
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			//改行までを抜き出す
			CLogicInt i;
			bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
			for( i = CLogicInt(0); i < nTextLen; i++ ){
				if( WCODE::IsLineDelimiter(pszText[i], bExtEol) ){
					break;
				}
			}
			Command_INDENT( pszText, i );
			goto end_of_func;
		}
		else{
			//	Jun. 23, 2000 genta
			//	同一行の行末以降のみが選択されている場合には選択無しと見なす
			bool bAfterEOLSelect = false;
			if( !bFastMode ){
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
					bAfterEOLSelect = true;
				}
			}
			if( !bAfterEOLSelect ){
				// データ置換 削除&挿入にも使える
				// 行コピーの貼り付けでは選択範囲は削除（後で行頭に貼り付ける）	// 2007.10.04 ryoji
				m_pCommanderView->ReplaceData_CEditView(
					GetSelect(),				// 選択範囲
					bLinePaste? L"": pszText,	// 挿入するデータ
					bLinePaste? CLogicInt(0): nTextLen,	// 挿入するデータの長さ
					bRedraw,
					m_pCommanderView->m_bDoing_UndoRedo?NULL:GetOpeBlk(),
					bFastMode,
					pcSelectLogic
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



static bool AppendHTMLColor(
	const SColorAttr& sColorAttrLast, SColorAttr& sColorAttrLast2,
	const SFontAttr& sFontAttrLast, SFontAttr& sFontAttrLast2,
	const WCHAR* pAppendStr, int nLen,
	CNativeW& cmemClip)
{
	if( sFontAttrLast.m_bBoldFont != sFontAttrLast2.m_bBoldFont || sFontAttrLast.m_bUnderLine != sFontAttrLast2.m_bUnderLine
	  || sColorAttrLast.m_cTEXT != sColorAttrLast2.m_cTEXT || sColorAttrLast.m_cBACK != sColorAttrLast2.m_cBACK ){
		if( sFontAttrLast2.m_bBoldFont ){
			cmemClip.AppendString( L"</b>", 4 );
		}
		if( sFontAttrLast2.m_bUnderLine ){
			if( sColorAttrLast.m_cTEXT != sColorAttrLast2.m_cTEXT || sColorAttrLast.m_cBACK != sColorAttrLast2.m_cBACK || sFontAttrLast.m_bUnderLine != sFontAttrLast2.m_bUnderLine ){
				cmemClip.AppendString( L"</u>", 4 );
			}
		}
		if( sColorAttrLast.m_cTEXT != sColorAttrLast2.m_cTEXT || sColorAttrLast.m_cBACK != sColorAttrLast2.m_cBACK ){
			if( sColorAttrLast2.m_cTEXT != (COLORREF)-1 ){
					cmemClip.AppendString( L"</span>", 7 );
				}
			if( sColorAttrLast.m_cTEXT != (COLORREF)-1 ){
				if( sColorAttrLast.m_cTEXT != sColorAttrLast2.m_cTEXT || sColorAttrLast.m_cBACK != sColorAttrLast2.m_cBACK ){
					WCHAR szColor[60];
					DWORD dwTEXTColor = (GetRValue(sColorAttrLast.m_cTEXT) << 16) + (GetGValue(sColorAttrLast.m_cTEXT) << 8) + GetBValue(sColorAttrLast.m_cTEXT);
					DWORD dwBACKColor = (GetRValue(sColorAttrLast.m_cBACK) << 16) + (GetGValue(sColorAttrLast.m_cBACK) << 8) + GetBValue(sColorAttrLast.m_cBACK);
					swprintf( szColor, L"<span style=\"color:#%06x;background-color:#%06x\">", dwTEXTColor, dwBACKColor);
					cmemClip.AppendString( szColor );
				}
			}
		}
		if( sFontAttrLast.m_bUnderLine ){
			if( sColorAttrLast.m_cTEXT != sColorAttrLast2.m_cTEXT || sColorAttrLast.m_cBACK != sColorAttrLast2.m_cBACK || sFontAttrLast.m_bUnderLine != sFontAttrLast2.m_bUnderLine ){
				cmemClip.AppendString( L"<u>", 3 );
			}
		}
		if( sFontAttrLast.m_bBoldFont ){
			cmemClip.AppendString( L"<b>", 3 );
		}
		sColorAttrLast2 = sColorAttrLast;
		sFontAttrLast2  = sFontAttrLast;
	}
	CNativeW cmemBuf(pAppendStr, nLen);
	cmemBuf.Replace(L"&", L"&amp;");
	cmemBuf.Replace(L"<", L"&lt;");
	cmemBuf.Replace(L">", L"&gt;");
	cmemClip.AppendNativeData(cmemBuf);
	if( 0 < nLen ){
		return WCODE::IsLineDelimiter(pAppendStr[nLen-1], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol);
	}
	return false;
}



//!選択範囲内色付きHTMLコピー
void CViewCommander::Command_COPY_COLOR_HTML(bool bLineNumber)
{
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected()
	  || GetSelect().GetFrom() == GetSelect().GetTo() ){
		return;
	}
	const STypeConfig& type = GetDocument()->m_cDocType.GetDocumentAttribute();
	bool bLineNumLayout = GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy
		|| m_pCommanderView->GetSelectionInfo().IsBoxSelecting();
	CLayoutRect rcSel;
	TwoPointToRect(
		&rcSel,
		GetSelect().GetFrom(),	// 範囲選択開始
		GetSelect().GetTo()		// 範囲選択終了
	);
	// 修飾分を除いたバッファの長さをだいたいで計算
	CLogicRange sSelectLogic;
	sSelectLogic.Clear(-1);
	int nBuffSize = 0;
	const CLayout* pcLayoutTop = NULL;
	{
		const CLayout* pcLayout;
		{
			CLogicInt nLineLenTmp;
			GetDocument()->m_cLayoutMgr.GetLineStr(rcSel.top, &nLineLenTmp, &pcLayout);
		}
		pcLayoutTop = pcLayout;
		CLayoutInt i = rcSel.top;
		for(; pcLayout != NULL && i <= rcSel.bottom; i++, pcLayout = pcLayout->GetNextLayout())
		{
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			CLogicInt nIdxFrom;
			CLogicInt nIdxTo;
			if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
				nIdxFrom = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.left );
				nIdxTo   = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.right);
				// 改行は除く
				if( nIdxTo - nIdxFrom > 0 ){
					const WCHAR* pLine = pcLayout->GetPtr();
					if( pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r' ){
						--nIdxTo;
					}
				}
				if( i == rcSel.top ){
					sSelectLogic.SetFromY(pcLayout->GetLogicLineNo());
					sSelectLogic.SetFromX(nIdxFrom);
				}
				if( i == rcSel.bottom ){
					sSelectLogic.SetToY(pcLayout->GetLogicLineNo());
					sSelectLogic.SetToX(nIdxTo);
				}
			}else{
				if( i == rcSel.top ){
					nIdxFrom = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.left);
					sSelectLogic.SetFromY(pcLayout->GetLogicLineNo());
					sSelectLogic.SetFromX(nIdxFrom);
				}else{
					nIdxFrom = CLogicInt(0);
				}
				if( i == rcSel.bottom ){
					nIdxTo = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.right);
					sSelectLogic.SetToY(pcLayout->GetLogicLineNo());
					sSelectLogic.SetToX(nIdxTo);
				}else{
					nIdxTo = pcLayout->GetLengthWithoutEOL();
				}
			}
			nBuffSize += nIdxTo - nIdxFrom;
			if( bLineNumLayout ){
				nBuffSize += 2;
			}else{
				nBuffSize += pcLayout->GetLayoutEol().GetLen();
			}
		}
		if( sSelectLogic.GetTo().x == -1 ){
			sSelectLogic.SetToY(GetDocument()->m_cDocLineMgr.GetLineCount());
			sSelectLogic.SetToX(CLogicInt(0));
		}
	}
	// 行番号の幅を計算
	int nLineNumberMaxLen = 0;
	WCHAR szLineFormat[10];
	szLineFormat[0] = L'\0';
	CNativeW cmemNullLine;
	if( bLineNumber ){
		int nLineNumberMax;
		if( type.m_bLineNumIsCRLF ){
			nLineNumberMax = sSelectLogic.GetTo().GetY();
		}else{
			nLineNumberMax = (Int)rcSel.bottom;
		}
		int nWork = 10;
		int i;
		cmemNullLine.AppendString(L" ");
		for( i = 1; i < 12; ++i ){
			if( nWork > nLineNumberMax ){
				break;
			}
			nWork *= 10;
			cmemNullLine.AppendString(L" ");
		}
		nLineNumberMaxLen = i + 1; // "%d:"
		cmemNullLine.AppendString(L":");
		swprintf(szLineFormat, L"%%%dd:", i);
	}
	if( bLineNumLayout ){
		nBuffSize += (Int)(nLineNumberMaxLen * (rcSel.bottom - rcSel.top + 1));
	}else{
		nBuffSize += (Int)(nLineNumberMaxLen * (sSelectLogic.GetTo().y - sSelectLogic.GetFrom().y + 1));
	}
	CNativeW cmemClip;
	cmemClip.AllocStringBuffer(nBuffSize + 11);
	{
		COLORREF cBACK = type.m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK;
		DWORD dwBACKColor = (GetRValue(cBACK) << 16) + (GetGValue(cBACK) << 8) + GetBValue(cBACK);
		WCHAR szBuf[50];
		swprintf(szBuf, L"<pre style=\"background-color:#%06x\">", dwBACKColor);
		cmemClip.AppendString( szBuf );
	}
	CLayoutInt nLayoutLineNum = rcSel.top;
	const CLogicInt nLineNumLast = sSelectLogic.GetTo().y;
	const CDocLine* pcDocLine = pcLayoutTop->GetDocLineRef();
	const CLayout* pcLayout = pcLayoutTop;
	while( pcLayout && pcLayout->GetLogicOffset() ){
		pcLayout = pcLayout->GetPrevLayout();
	}
	SColorAttr sColorAttr = { (COLORREF)-1, (COLORREF)-1 };
	SColorAttr sColorAttrNext = { (COLORREF)-1, (COLORREF)-1 };
	SColorAttr sColorAttrLast = { (COLORREF)-1, (COLORREF)-1 };
	SColorAttr sColorAttrLast2 = { (COLORREF)-1, (COLORREF)-1 };
	SFontAttr sFontAttr = { false, false };
	SFontAttr sFontAttrNext = { false, false };
	SFontAttr sFontAttrLast = { false, false };
	SFontAttr sFontAttrLast2 = { false, false };
	CColorStrategyPool* pool = CColorStrategyPool::getInstance();
	pool->SetCurrentView(m_pCommanderView);
	for(CLogicInt nLineNum = sSelectLogic.GetFrom().y; nLineNum <= nLineNumLast; nLineNum++, pcDocLine = pcDocLine->GetNextLine()){
		if( pcDocLine == NULL ){
			break;
		}
		pool->NotifyOnStartScanLogic();
		CColorStrategy* pStrategyNormal = NULL;
		CColorStrategy* pStrategyFound = NULL;
		CColorStrategy* pStrategy = NULL;
		CStringRef cStringLine(pcDocLine->GetPtr(), pcDocLine->GetLengthWithEOL());
		{
			pStrategy = pStrategyNormal = pool->GetStrategyByColor(pcLayout->GetColorTypePrev());
			if( pStrategy ){
				pStrategy->InitStrategyStatus();
				pStrategy->SetStrategyColorInfo(pcLayout->GetColorInfo());
			}
			int nColorIdx = ToColorInfoArrIndex(pcLayout->GetColorTypePrev());
			if (-1 != nColorIdx) {
				const ColorInfo& info = type.m_ColorInfoArr[nColorIdx];
				sFontAttr = info.m_sFontAttr;
				sColorAttr = info.m_sColorAttr;
			}
		}
		const WCHAR* pLine = pcDocLine->GetPtr();
		for(;pcLayout->GetLogicLineNo() == nLineNum; nLayoutLineNum++, pcLayout = pcLayout->GetNextLayout() )
		{
			CLogicInt nIdxFrom;
			CLogicInt nIdxTo;
			const int nLineLen = pcLayout->GetLengthWithoutEOL() + pcLayout->GetLayoutEol().GetLen();
			if( nLayoutLineNum < rcSel.top ){
				nIdxTo = nIdxFrom = CLogicInt(-1);
			}else{
				if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
					nIdxFrom = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.left );
					nIdxTo   = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.right);
					// 改行は除く
					if( nIdxTo - nIdxFrom > 0 ){
						const WCHAR* pLine = pcLayout->GetPtr();
						if( pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r' ){
							--nIdxTo;
						}
					}
				}else{
					if( nLayoutLineNum == rcSel.top ){
						nIdxFrom = sSelectLogic.GetFrom().x;
					}else{
						nIdxFrom = CLogicInt(0);
					}
					if( nLayoutLineNum == rcSel.bottom ){
						nIdxTo = sSelectLogic.GetTo().x;
					}else{
						nIdxTo = nLineLen;
					}
				}
			}
			// 最後の改行の次の行番号を表示しないように
			if( nIdxTo == 0 && nLayoutLineNum == rcSel.bottom ){
				break;
			}
			if( bLineNumber ){
				WCHAR szLineNum[14];
				if( type.m_bLineNumIsCRLF ){
					if( pcLayout->GetLogicOffset() != 0 ){
						if( bLineNumLayout ){
							cmemClip.AppendNativeData(cmemNullLine);
						}
					}else{
						int ret = swprintf(szLineNum, szLineFormat, nLineNum + 1);
						cmemClip.AppendString(szLineNum, ret);
					}
				}else{
					if( bLineNumLayout || pcLayout->GetLogicOffset() == 0 ){
						int ret = swprintf(szLineNum, szLineFormat, nLayoutLineNum + 1);
						cmemClip.AppendString(szLineNum, ret);
					}
				}
			}
			const int nLineStart = pcLayout->GetLogicOffset();
			int nBgnLogic = nIdxFrom + nLineStart;
			int iLogic = nLineStart;
			bool bAddCRLF = false;
			for( ; iLogic < nLineStart + nLineLen; ++iLogic ){
				bool bChange = false;
				pStrategy = GetColorStrategyHTML(cStringLine, iLogic, pool, &pStrategyNormal, &pStrategyFound, bChange);
				if( bChange ){
					int nColorIdx = ToColorInfoArrIndex( pStrategy ? pStrategy->GetStrategyColor() : COLORIDX_TEXT );
					if (-1 != nColorIdx) {
						const ColorInfo& info = type.m_ColorInfoArr[nColorIdx];
						sColorAttrNext = info.m_sColorAttr;
						sFontAttrNext  = info.m_sFontAttr;
					}
				}
				if( nIdxFrom != -1 && nIdxFrom + nLineStart <= iLogic && iLogic <= nIdxTo + nLineStart ){
					if( nIdxFrom + nLineStart == iLogic ){
						sColorAttrLast = sColorAttrNext;
						sFontAttrLast  = sFontAttrNext;
					}else if( nIdxFrom + nLineStart < iLogic
					  && (sFontAttr.m_bBoldFont != sFontAttrNext.m_bBoldFont || sFontAttr.m_bUnderLine != sFontAttrNext.m_bUnderLine
					  || sColorAttr.m_cTEXT != sColorAttrNext.m_cTEXT || sColorAttr.m_cBACK != sColorAttrNext.m_cBACK) ){
						bAddCRLF = AppendHTMLColor(sColorAttrLast, sColorAttrLast2,
							sFontAttrLast, sFontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, cmemClip);
						sColorAttrLast = sColorAttrNext;
						sFontAttrLast  = sFontAttrNext;
						nBgnLogic = iLogic;
					}else if( nIdxTo + nLineStart == iLogic ){
						bAddCRLF = AppendHTMLColor(sColorAttrLast, sColorAttrLast2,
							sFontAttrLast, sFontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, cmemClip);
						nBgnLogic = iLogic;
					}
				}
				sColorAttr = sColorAttrNext;
				sFontAttr = sFontAttrNext;
			}
			if( nIdxFrom != -1 && nIdxTo + nLineStart == iLogic ){
				bAddCRLF = AppendHTMLColor(sColorAttrLast, sColorAttrLast2,
					sFontAttrLast, sFontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, cmemClip);
			}
			if( bLineNumber ){
				bool bAddLineNum = true;
				const CLayout* pcLayoutNext = pcLayout->GetNextLayout();
				if( pcLayoutNext ){
					if( type.m_bLineNumIsCRLF ){
						if( bLineNumLayout && pcLayoutNext->GetLogicOffset() != 0 ){
							bAddLineNum = true;
						}else{
							bAddLineNum = true;
						}
					}else{
						if( bLineNumLayout || pcLayoutNext->GetLogicOffset() == 0 ){
							bAddLineNum = true;
						}
					}
				}
				if( bAddLineNum ){
					if( sFontAttrLast2.m_bBoldFont ){
						cmemClip.AppendString(L"</b>", 4);
					}
					if( sFontAttrLast2.m_bUnderLine ){
						cmemClip.AppendString(L"</u>", 4);
					}
					if( sColorAttrLast2.m_cTEXT != (COLORREF)-1 ){
						cmemClip.AppendString(L"</span>", 7);
					}
					sFontAttrLast.m_bBoldFont = sFontAttrLast2.m_bBoldFont = false;
					sFontAttrLast.m_bUnderLine = sFontAttrLast2.m_bUnderLine = false;
					sColorAttrLast.m_cTEXT = sColorAttrLast2.m_cTEXT = (COLORREF)-1;
					sColorAttrLast.m_cBACK = sColorAttrLast2.m_cBACK = (COLORREF)-1;
				}
			}
			if( bLineNumLayout && !bAddCRLF ){
				cmemClip.AppendString(WCODE::CRLF, 2);
			}
			// 2014.06.25 バッファ拡張
			if( cmemClip.capacity() < cmemClip.GetStringLength() + 100 ){
				cmemClip.AllocStringBuffer( cmemClip.capacity() + cmemClip.capacity() / 2 );
			}
		}
	}
	if( sFontAttrLast2.m_bBoldFont ){
		cmemClip.AppendString(L"</b>", 4);
	}
	if( sFontAttrLast2.m_bUnderLine ){
		cmemClip.AppendString(L"</u>", 4);
	}
	if( sColorAttrLast2.m_cTEXT != (COLORREF)-1 ){
		cmemClip.AppendString(L"</span>", 7);
	}
	cmemClip.AppendString(L"</pre>", 6);

	CClipboard cClipboard( GetEditWindow()->GetHwnd() );
	if(!cClipboard){
		return;
	}
	cClipboard.Empty();
	cClipboard.SetHtmlText(cmemClip);
	cClipboard.SetText(cmemClip.GetStringPtr(), cmemClip.GetStringLength(), false, false);
}



/*!
	@date 2014.12.30 Moca 同じCColorStrategyで違う色に切り替わったときに対応
*/
CColorStrategy* CViewCommander::GetColorStrategyHTML(
	const CStringRef&	cStringLine,
	int					iLogic,
	const CColorStrategyPool*	pool,
	CColorStrategy**	ppStrategy,
	CColorStrategy**	ppStrategyFound,		// [in,out]
	bool& bChange
)
{
	//検索色終了
	if(*ppStrategyFound){
		if((*ppStrategyFound)->EndColor(cStringLine, iLogic)){
			*ppStrategyFound = NULL;
			bChange = true;
		}
	}

	//検索色開始
	if(!*ppStrategyFound){
		CColor_Found*  pcFound  = pool->GetFoundStrategy();
		if(pcFound->BeginColor(cStringLine, iLogic)){
			*ppStrategyFound = pcFound;
			bChange = true;
		}
	}

	//色終了
	if(*ppStrategy){
		if((*ppStrategy)->EndColor(cStringLine, iLogic)){
			*ppStrategy = NULL;
			bChange = true;
		}
	}

	//色開始
	if(!*ppStrategy){
		int size = pool->GetStrategyCount();
		for(int i = 0; i < size; i++ ){
			if(pool->GetStrategy(i)->BeginColor(cStringLine, iLogic)){
				*ppStrategy = pool->GetStrategy(i);
				bChange = true;
				break;
			}
		}
	}
	if( *ppStrategyFound ){
		return *ppStrategyFound;
	}
	return *ppStrategy;
}

//!選択範囲内行番号色付きHTMLコピー
void CViewCommander::Command_COPY_COLOR_HTML_LINENUMBER()
{
	Command_COPY_COLOR_HTML(true);
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
