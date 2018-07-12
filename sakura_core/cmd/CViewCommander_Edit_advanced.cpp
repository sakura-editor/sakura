/*!	@file
@brief CViewCommanderクラスのコマンド(編集系 高度な操作(除単語/行操作))関数群

	2012/12/17	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta, みつ
	Copyright (C) 2001, MIK, Stonee, Misaka, asa-o, novice, hor, YAZAKI
	Copyright (C) 2002, hor, YAZAKI, novice, genta, aroka, Azumaiya, minfu, MIK, oak, すなふき, Moca, ai
	Copyright (C) 2003, MIK, genta, かろと, zenryaku, Moca, ryoji, naoh, KEITA, じゅうじ
	Copyright (C) 2004, isearch, Moca, gis_dur, genta, crayonzen, fotomo, MIK, novice, みちばな, Kazika
	Copyright (C) 2005, genta, novice, かろと, MIK, Moca, D.S.Koba, aroka, ryoji, maru
	Copyright (C) 2006, genta, aroka, ryoji, かろと, fon, yukihane, Moca
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2010, ryoji
	Copyright (C) 2011, ryoji
	Copyright (C) 2012, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "uiparts/CWaitCursor.h"
#include "mem/CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "_os/COsVersionInfo.h"


using namespace std; // 2002/2/3 aroka to here

#ifndef FID_RECONVERT_VERSION  // 2002.04.10 minfu 
#define FID_RECONVERT_VERSION 0x10000000
#endif
#ifndef SCS_CAP_SETRECONVERTSTRING
#define SCS_CAP_SETRECONVERTSTRING 0x00000004
#define SCS_QUERYRECONVERTSTRING 0x00020000
#define SCS_SETRECONVERTSTRING 0x00010000
#endif


/* インデント ver1 */
void CViewCommander::Command_INDENT( wchar_t wcChar, EIndentType eIndent )
{
	using namespace WCODE;

#if 1	// ↓ここを残せば選択幅ゼロを最大にする（従来互換挙動）。無くても Command_INDENT() ver0 が適切に動作するように変更されたので、削除しても特に不都合にはならない。
	// From Here 2001.12.03 hor
	/* SPACEorTABインンデントで矩形選択桁がゼロの時は選択範囲を最大にする */
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if( INDENT_NONE != eIndent && m_pCommanderView->GetSelectionInfo().IsBoxSelecting() && GetSelect().GetFrom().x==GetSelect().GetTo().x ){
		GetSelect().SetToX( GetDocument()->m_cLayoutMgr.GetMaxLineLayout() );
		m_pCommanderView->RedrawAll();
		return;
	}
	// To Here 2001.12.03 hor
#endif
	Command_INDENT( &wcChar, CLogicInt(1), eIndent );
	return;
}



/* インデント ver0 */
/*
	選択された各行の範囲の直前に、与えられた文字列( pData )を挿入する。
	@param eIndent インデントの種別
*/
void CViewCommander::Command_INDENT( const wchar_t* const pData, const CLogicInt nDataLen, EIndentType eIndent )
{
	if( nDataLen <= 0 ) return;

	CLayoutRange sSelectOld;		//範囲選択
	CLayoutPoint ptInserted;		//挿入後の挿入位置
	const struct IsIndentCharSpaceTab{
		IsIndentCharSpaceTab(){}
		bool operator()( const wchar_t ch ) const
		{ return ch == WCODE::SPACE || ch == WCODE::TAB; }
	} IsIndentChar;
	struct SSoftTabData {
		SSoftTabData( CLayoutXInt nTab, int width ) : m_szTab(NULL), m_nTab((Int)nTab), m_nXWidth(width - 1), m_nSpWidth(width) {}
		~SSoftTabData() { delete []m_szTab; }
		operator const wchar_t* ()
		{
			if( !m_szTab ){
				m_szTab = new wchar_t[m_nTab];
				wmemset( m_szTab, WCODE::SPACE, m_nTab );
			}
			return m_szTab;
		}
		// TAB=4 だとしても、TAB="x"幅*4 なのでSPとは幅が違うので、PPFontだとレイアウト上は桁が一致しません
		// @see CConvert_TabToSpace::DoConvert() convert/CConvert_TabToSpace.cpp
		// とりあえずCMemoryIterator/CLayoutMgr::GetActualTabSpace互換で計算してx幅での個数分を追加する
		// nColまでの文字のGetKetaOfCharとGetTabSpaceKetasを使うとTAB指定文字数分になる
		int Len( CLayoutInt nCol ) { return (m_nTab + m_nXWidth - ((Int)nCol + m_nXWidth) % m_nTab) / m_nSpWidth; }
		wchar_t* m_szTab;
		int m_nTab;
		int m_nXWidth;
		int m_nSpWidth;
	} stabData( GetDocument()->m_cLayoutMgr.GetTabSpace(), GetDocument()->m_cLayoutMgr.GetWidthPerKeta() );

	const bool bSoftTab = ( eIndent == INDENT_TAB && m_pCommanderView->m_pTypeData->m_bInsSpace );
	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){			/* テキストが選択されているか */
		if( INDENT_NONE != eIndent && !bSoftTab ){
			// ※矩形選択ではないので Command_WCHAR から呼び戻しされるようなことはない
			Command_WCHAR( pData[0] );	// 1文字入力
		}
		else{
			// ※矩形選択ではないのでここへ来るのは実際にはソフトタブのときだけ
			if( bSoftTab && !m_pCommanderView->IsInsMode() ){
				DelCharForOverwrite(pData, nDataLen);
			}
			m_pCommanderView->InsertData_CEditView(
				GetCaret().GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(GetCaret().GetCaretLayoutPos().GetX2()),
				&ptInserted,
				true
			);
			GetCaret().MoveCursor( ptInserted, true );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		return;
	}
	const bool bDrawSwitchOld = m_pCommanderView->SetDrawSwitch(false);	// 2002.01.25 hor
	/* 矩形範囲選択中か */
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
// 2012.10.31 Moca 上書きモードのときの選択範囲削除をやめる
// 2014.06.02 Moca 旧仕様を選べるように
		if( GetDllShareData().m_Common.m_sEdit.m_bOverWriteBoxDelete ){
			// From Here 2001.12.03 hor
			/* 上書モードのときは選択範囲削除 */
			if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */){
				sSelectOld = GetSelect();
				m_pCommanderView->DeleteData( false );
				GetSelect() = sSelectOld;
				m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);
			}
			// To Here 2001.12.03 hor
		}

		/* 2点を対角とする矩形を求める */
		CLayoutRange rcSel;
		TwoPointToRange(
			&rcSel,
			GetSelect().GetFrom(),	// 範囲選択開始
			GetSelect().GetTo()		// 範囲選択終了
		);
		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( false/*true 2002.01.25 hor*/ );

		/*
			文字を直前に挿入された文字が、それにより元の位置からどれだけ後ろにずれたか。
			これに従い矩形選択範囲を後ろにずらす。
		*/
		CLayoutInt minOffset( -1 );
		/*
			■全角文字の左側の桁揃えについて
			(1) eIndent == INDENT_TAB のとき
				選択範囲がタブ境界にあるときにタブを入力すると、全角文字の前半が選択範囲から
				はみ出している行とそうでない行でタブの幅が、1から設定された最大までと大きく異なり、
				最初に選択されていた文字を選択範囲内にとどめておくことができなくなる。
				最初は矩形選択範囲内にきれいに収まっている行にはタブを挿入せず、ちょっとだけはみ
				出している行にだけタブを挿入することとし、それではどの行にもタブが挿入されない
				とわかったときはやり直してタブを挿入する。
			(2) eIndent == INDENT_SPACE のとき（※従来互換的な動作）
				幅1で選択している場合のみ全角文字の左側を桁揃えする。
				最初は矩形選択範囲内にきれいに収まっている行にはスペースを挿入せず、ちょっとだけはみ
				出している行にだけスペースを挿入することとし、それではどの行にもスペースが挿入されない
				とわかったときはやり直してスペースを挿入する。
		*/
		bool alignFullWidthChar = eIndent == INDENT_TAB && 0 == rcSel.GetFrom().x % this->GetDocument()->m_cLayoutMgr.GetTabSpace();
#if 1	// ↓ここを残せば選択幅1のSPACEインデントで全角文字を揃える機能(2)が追加される。
		alignFullWidthChar = alignFullWidthChar || (eIndent == INDENT_SPACE && 1 == rcSel.GetTo().x - rcSel.GetFrom().x);
#endif
		CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd(), 1000 < rcSel.GetTo().y - rcSel.GetFrom().y );
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if( cWaitCursor.IsEnable() ){
			hwndProgress = m_pCommanderView->StartProgress();
		}
		for( bool insertionWasDone = false; ; alignFullWidthChar = false ) {
			minOffset = CLayoutInt( -1 );
			for( CLayoutInt nLineNum = rcSel.GetFrom().y; nLineNum <= rcSel.GetTo().y; ++nLineNum ){
				const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
				//	Nov. 6, 2002 genta NULLチェック追加
				//	これがないとEOF行を含む矩形選択中の文字列入力で落ちる
				CLogicInt nIdxFrom, nIdxTo;
				CLayoutInt xLayoutFrom, xLayoutTo;
				bool reachEndOfLayout = false;
				if( pcLayout ) {
					/* 指定された桁に対応する行のデータ内の位置を調べる */
					const struct {
						CLayoutInt keta;
						CLogicInt* outLogicX;
						CLayoutInt* outLayoutX;
					} sortedKetas[] = {
						{ rcSel.GetFrom().x, &nIdxFrom, &xLayoutFrom },
						{ rcSel.GetTo().x, &nIdxTo, &xLayoutTo },
						{ CLayoutInt(-1), 0, 0 }
					};
					CMemoryIterator it = GetDocument()->m_cLayoutMgr.CreateCMemoryIterator(pcLayout);
					for( int i = 0; 0 <= sortedKetas[i].keta; ++i ) {
						for( ; ! it.end(); it.addDelta() ) {
							if( sortedKetas[i].keta == it.getColumn() ) {
								break;
							}
							it.scanNext();
							if( sortedKetas[i].keta < it.getColumn() + it.getColumnDelta() ) {
								break;
							}
						}
						*sortedKetas[i].outLogicX = it.getIndex();
						*sortedKetas[i].outLayoutX = it.getColumn();
					}
					reachEndOfLayout = it.end();
				}else{
					nIdxFrom = nIdxTo = CLogicInt(0);
					xLayoutFrom = xLayoutTo = CLayoutInt(0);
					reachEndOfLayout = true;
				}
				const bool emptyLine = ! pcLayout || 0 == pcLayout->GetLengthWithoutEOL();
				const bool selectionIsOutOfLine = reachEndOfLayout && (
					(pcLayout && pcLayout->GetLayoutEol() != EOL_NONE) ? xLayoutFrom == xLayoutTo : xLayoutTo < rcSel.GetFrom().x
				);

				// 入力文字の挿入位置
				const CLayoutPoint ptInsert( selectionIsOutOfLine ? rcSel.GetFrom().x : xLayoutFrom, nLineNum );

				/* TABやスペースインデントの時 */
				if( INDENT_NONE != eIndent ) {
					if( emptyLine || selectionIsOutOfLine ) {
						continue; // インデント文字をインデント対象が存在しない部分(改行文字の後ろや空行)に挿入しない。
					}
					/*
						入力がインデント用の文字のとき、ある条件で入力文字を挿入しないことで
						インデントを揃えることができる。
						http://sakura-editor.sourceforge.net/cgi-bin/cyclamen/cyclamen.cgi?log=dev&v=4103
					*/
					if( nIdxFrom == nIdxTo // 矩形選択範囲の右端までに範囲の左端にある文字の末尾が含まれておらず、
						&& ! selectionIsOutOfLine && pcLayout && IsIndentChar( pcLayout->GetPtr()[nIdxFrom] ) // その、末尾の含まれていない文字がインデント文字であり、
						&& rcSel.GetFrom().x < rcSel.GetTo().x // 幅0矩形選択ではない(<<互換性とインデント文字挿入の使い勝手のために除外する)とき。
					) {
						continue;
					}
					/*
						全角文字の左側の桁揃え
					*/
					if( alignFullWidthChar
						&& (ptInsert.x == rcSel.GetFrom().x || (pcLayout && IsIndentChar( pcLayout->GetPtr()[nIdxFrom] )))
					) {	// 文字の左側が範囲にぴったり収まっている
						minOffset = CLayoutInt(0);
						continue;
					}
				}

				/* 現在位置にデータを挿入 */
				m_pCommanderView->InsertData_CEditView(
					ptInsert,
					!bSoftTab? pData: stabData,
					!bSoftTab? nDataLen: stabData.Len(ptInsert.x),
					&ptInserted,
					false
				);
				insertionWasDone = true;
				minOffset = t_min(
					0 <= minOffset ? minOffset : this->GetDocument()->m_cLayoutMgr.GetMaxLineLayout(),
					ptInsert.x <= ptInserted.x ? ptInserted.x - ptInsert.x : t_max( CLayoutInt(0), this->GetDocument()->m_cLayoutMgr.GetMaxLineLayout() - ptInsert.x)
				);

				GetCaret().MoveCursor( ptInserted, false );
				GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

				if( hwndProgress ){
					int newPos = ::MulDiv((Int)nLineNum, 100, (Int)rcSel.GetTo().y);
					if( newPos != nProgressPos ){
						nProgressPos = newPos;
						Progress_SetPos( hwndProgress, newPos + 1 );
						Progress_SetPos( hwndProgress, newPos );
					}
				}
			}
			if( insertionWasDone || !alignFullWidthChar ) {
				break; // ループの必要はない。(1.文字の挿入が行われたから。2.そうではないが文字の挿入を控えたせいではないから)
			}
		}

		if( hwndProgress ){
			::ShowWindow( hwndProgress, SW_HIDE );
		}

		// 挿入された文字の分だけ選択範囲を後ろにずらし、rcSelにセットする。
		if( 0 < minOffset ) {
			rcSel.GetFromPointer()->x = t_min( rcSel.GetFrom().x + minOffset, this->GetDocument()->m_cLayoutMgr.GetMaxLineLayout() );
			rcSel.GetToPointer()->x = t_min( rcSel.GetTo().x + minOffset, this->GetDocument()->m_cLayoutMgr.GetMaxLineLayout() );
		}

		/* カーソルを移動 */
		GetCaret().MoveCursor( rcSel.GetFrom(), true );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			/* 操作の追加 */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
		GetSelect().SetFrom(rcSel.GetFrom());	//範囲選択開始位置
		GetSelect().SetTo(rcSel.GetTo());		//範囲選択終了位置
		m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);
	}
	else if( GetSelect().IsLineOne() ){	// 通常選択(1行内)
		if( INDENT_NONE != eIndent && !bSoftTab ){
			// ※矩形選択ではないので Command_WCHAR から呼び戻しされるようなことはない
			Command_WCHAR( pData[0] );	// 1文字入力
		}
		else{
			// ※矩形選択ではないのでここへ来るのは実際にはソフトタブのときだけ
			m_pCommanderView->DeleteData( false );
			m_pCommanderView->InsertData_CEditView(
				GetCaret().GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(GetCaret().GetCaretLayoutPos().GetX2()),
				&ptInserted,
				false
			);
			GetCaret().MoveCursor( ptInserted, true );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
	}
	else{	// 通常選択(複数行)
		sSelectOld.SetFrom(CLayoutPoint(CLayoutInt(0),GetSelect().GetFrom().y));
		sSelectOld.SetTo  (CLayoutPoint(CLayoutInt(0),GetSelect().GetTo().y  ));
		if( GetSelect().GetTo().x > 0 ){
			sSelectOld.GetToPointer()->y++;
		}

		// 現在の選択範囲を非選択状態に戻す
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( false );

		CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd(), 1000 < sSelectOld.GetTo().GetY2() - sSelectOld.GetFrom().GetY2() );
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if( cWaitCursor.IsEnable() ){
			hwndProgress = m_pCommanderView->StartProgress();
		}

		for( CLayoutInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().GetY2(); i++ ){
			CLayoutInt nLineCountPrev = GetDocument()->m_cLayoutMgr.GetLineCount();
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( i );
			if( NULL == pcLayout ||						//	テキストが無いEOLの行は無視
				pcLayout->GetLogicOffset() > 0 ||				//	折り返し行は無視
				pcLayout->GetLengthWithoutEOL() == 0 ){	//	改行のみの行は無視する。
				continue;
			}

			/* カーソルを移動 */
			GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), i), false );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			/* 現在位置にデータを挿入 */
			m_pCommanderView->InsertData_CEditView(
				CLayoutPoint(CLayoutInt(0),i),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(CLayoutInt(0)),
				&ptInserted,
				false
			);
			/* カーソルを移動 */
			GetCaret().MoveCursor( ptInserted, false );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			if ( nLineCountPrev != GetDocument()->m_cLayoutMgr.GetLineCount() ){
				//	行数が変化した!!
				sSelectOld.GetToPointer()->y += GetDocument()->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
			if( hwndProgress ){
				int newPos = ::MulDiv((Int)i, 100, (Int)sSelectOld.GetTo().GetY());
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

		GetSelect() = sSelectOld;

		// From Here 2001.12.03 hor
		GetCaret().MoveCursor( GetSelect().GetTo(), true );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
		// To Here 2001.12.03 hor
	}
	/* 再描画 */
	m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);	// 2002.01.25 hor
	m_pCommanderView->RedrawAll();			// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
	return;
}



/* 逆インデント */
void CViewCommander::Command_UNINDENT( wchar_t wcChar )
{
	//	Aug. 9, 2003 genta
	//	選択されていない場合に逆インデントした場合に
	//	注意メッセージを出す
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		EIndentType eIndent;
		switch( wcChar ){
		case WCODE::TAB:
			eIndent = INDENT_TAB;	// ※[SPACEの挿入]オプションが ON ならソフトタブにする（Wiki BugReport/66）
			break;
		case WCODE::SPACE:
			eIndent = INDENT_SPACE;
			break;
		default:
			eIndent = INDENT_NONE;
		}
		Command_INDENT( wcChar, eIndent );
		m_pCommanderView->SendStatusMessage(LS(STR_ERR_UNINDENT1));
		return;
	}

	/* 矩形範囲選択中か */
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		ErrorBeep();
//**********************************************
//	 箱型逆インデントについては、保留とする (1998.10.22)
//**********************************************
	}
	else{
		GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

		CLayoutRange sSelectOld;	//範囲選択
		sSelectOld.SetFrom(CLayoutPoint(CLayoutInt(0),GetSelect().GetFrom().y));
		sSelectOld.SetTo  (CLayoutPoint(CLayoutInt(0),GetSelect().GetTo().y  ));
		if( GetSelect().GetTo().x > 0 ){
			sSelectOld.GetToPointer()->y++;
		}

		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( false );

		CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd(), 1000 < sSelectOld.GetTo().GetY() - sSelectOld.GetFrom().GetY() );
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if( cWaitCursor.IsEnable() ){
			hwndProgress = m_pCommanderView->StartProgress();
		}

		CLogicInt		nDelLen;
		for( CLayoutInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().GetY2(); i++ ){
			CLayoutInt nLineCountPrev = GetDocument()->m_cLayoutMgr.GetLineCount();

			const CLayout*	pcLayout;
			CLogicInt		nLineLen;
			const wchar_t*	pLine = GetDocument()->m_cLayoutMgr.GetLineStr( i, &nLineLen, &pcLayout );
			if( NULL == pcLayout || pcLayout->GetLogicOffset() > 0 ){ //折り返し以降の行はインデント処理を行わない
				continue;
			}

			if( WCODE::TAB == wcChar ){
				if( pLine[0] == wcChar ){
					nDelLen = CLogicInt(1);
				}
				else{
					//削り取る半角スペース数 (1～タブ幅分) -> nDelLen
					CLogicInt i;
					CLogicInt nTabSpaces = CLogicInt((Int)GetDocument()->m_cLayoutMgr.GetTabSpaceKetas());
					for( i = CLogicInt(0); i < nLineLen; i++ ){
						if( WCODE::SPACE != pLine[i] ){
							break;
						}
						//	Sep. 23, 2002 genta LayoutMgrの値を使う
						if( i >= nTabSpaces ){
							break;
						}
					}
					if( 0 == i ){
						continue;
					}
					nDelLen = i;
				}
			}
			else{
				if( pLine[0] != wcChar ){
					continue;
				}
				nDelLen = CLogicInt(1);
			}

			/* カーソルを移動 */
			GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), i), false );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			/* 指定位置の指定長データ削除 */
			m_pCommanderView->DeleteData2(
				CLayoutPoint(CLayoutInt(0),i),
				nDelLen,	// 2001.12.03 hor
				NULL
			);
			if ( nLineCountPrev != GetDocument()->m_cLayoutMgr.GetLineCount() ){
				//	行数が変化した!!
				sSelectOld.GetToPointer()->y += GetDocument()->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
			if( hwndProgress ){
				int newPos = ::MulDiv((Int)i, 100, (Int)sSelectOld.GetTo().GetY());
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
		GetSelect() = sSelectOld;	//範囲選択

		// From Here 2001.12.03 hor
		GetCaret().MoveCursor( GetSelect().GetTo(), true );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
		// To Here 2001.12.03 hor
	}

	/* 再描画 */
	m_pCommanderView->RedrawAll();	// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
}



//	from CViewCommander_New.cpp
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
				GetDocument()->m_cLayoutMgr.GetMaxLineLayout(),
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
		cViewSelect.DisableSelectArea( true );
}



//	from CViewCommander_New.cpp
/*!	物理行のソートに使う構造体*/
struct SORTDATA {
	const CNativeW* pCmemLine;
	CStringRef sKey;
};

inline int CNativeW_comp(const CNativeW& lhs, const CNativeW& rhs )
{
	// 比較長には終端NULを含めないといけない
	return wmemcmp(lhs.GetStringPtr(), rhs.GetStringPtr(),
			t_min(lhs.GetStringLength() + 1, rhs.GetStringLength() + 1));
}

/*!	物理行のソートに使う関数(昇順) */
bool SortByLineAsc (SORTDATA* pst1, SORTDATA* pst2) {return CNativeW_comp(*pst1->pCmemLine, *pst2->pCmemLine) < 0;}

/*!	物理行のソートに使う関数(降順) */
bool SortByLineDesc(SORTDATA* pst1, SORTDATA* pst2) {return CNativeW_comp(*pst1->pCmemLine, *pst2->pCmemLine) > 0;}

inline int CStringRef_comp(const CStringRef& c1, const CStringRef& c2)
{
	int ret = wmemcmp(c1.GetPtr(), c2.GetPtr(), t_min(c1.GetLength(), c2.GetLength()));
	if( ret == 0 ){
		return c1.GetLength() - c2.GetLength();
	}
	return ret;
}

/*!	物理行のソートに使う関数(昇順) */
bool SortByKeyAsc(SORTDATA* pst1, SORTDATA* pst2)  {return CStringRef_comp(pst1->sKey, pst2->sKey) < 0 ;}

/*!	物理行のソートに使う関数(降順) */
bool SortByKeyDesc(SORTDATA* pst1, SORTDATA* pst2) {return CStringRef_comp(pst1->sKey, pst2->sKey) > 0 ;}

/*!	@brief 物理行のソート

	非選択時は何も実行しない．矩形選択時は、その範囲をキーにして物理行をソート．
	
	@note とりあえず改行コードを含むデータをソートしているので、
	ファイルの最終行はソート対象外にしています
	@author hor
	@date 2001.12.03 hor 新規作成
	@date 2001.12.21 hor 選択範囲の調整ロジックを訂正
	@date 2010.07.27 行ソートでコピーを減らす/NULより後ろも比較対照に
	@date 2013.06.19 Moca 矩形選択時最終行に改行がない場合は付加+ソート後の最終行の改行を削除
*/
void CViewCommander::Command_SORT(BOOL bAsc)	//bAsc:TRUE=昇順,FALSE=降順
{
	CLayoutRange sRangeA;
	CLogicRange sSelectOld;

	int			nColumnFrom, nColumnTo;
	CLayoutInt	nCF(0), nCT(0);
	CLayoutInt	nCaretPosYOLD;
	bool		bBeginBoxSelectOld;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int			j;
	std::vector<SORTDATA*> sta;

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){			/* テキストが選択されているか */
		return;
	}

	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		sRangeA=m_pCommanderView->GetSelectionInfo().m_sSelect;
		if( m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().x==m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().x ){
			//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
			m_pCommanderView->GetSelectionInfo().m_sSelect.SetToX( GetDocument()->m_cLayoutMgr.GetMaxLineLayout() );
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

	sta.reserve(sSelectOld.GetTo().GetY2() - sSelectOld.GetFrom().GetY2() );
	for( CLogicInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().y; i++ ){
		const CDocLine* pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( i );
		const CNativeW& cmemLine = pcDocLine->_GetDocLineDataWithEOL();
		pLine = cmemLine.GetStringPtr(&nLineLen);
		CLogicInt nLineLenWithoutEOL = pcDocLine->GetLengthWithoutEOL();
		if( NULL == pLine ) continue;
		SORTDATA* pst = new SORTDATA;
		if( bBeginBoxSelectOld ){
			nColumnFrom = m_pCommanderView->LineColumnToIndex( pcDocLine, nCF );
			nColumnTo   = m_pCommanderView->LineColumnToIndex( pcDocLine, nCT );
			if(nColumnTo<nLineLenWithoutEOL){	// BOX選択範囲の右端が行内に収まっている場合
				// 2006.03.31 genta std::string::assignを使って一時変数削除
				pst->sKey = CStringRef( &pLine[nColumnFrom], nColumnTo-nColumnFrom );
			}else if(nColumnFrom<nLineLenWithoutEOL){	// BOX選択範囲の右端が行末より右にはみ出している場合
				pst->sKey = CStringRef( &pLine[nColumnFrom], nLineLenWithoutEOL-nColumnFrom );
			}else{
				// 選択範囲の左端もはみ出している==データなし
				pst->sKey = CStringRef( L"", 0 );
			}
		}
		pst->pCmemLine = &cmemLine;
		sta.push_back(pst);
	}
	const wchar_t* pStrLast = NULL; // 最後の行に改行がなければそのポインタ
	if( 0 < sta.size() ){
		pStrLast = sta[sta.size()-1]->pCmemLine->GetStringPtr();
		int nlen = sta[sta.size()-1]->pCmemLine->GetStringLength();
		if( 0 < nlen ){
			if( WCODE::IsLineDelimiter(pStrLast[nlen-1], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
				pStrLast = NULL;
			}
		}
	}
	if( bBeginBoxSelectOld ){
		if(bAsc){
			std::stable_sort(sta.begin(), sta.end(), SortByKeyAsc);
		}else{
			std::stable_sort(sta.begin(), sta.end(), SortByKeyDesc);
		}
	}else{
		if(bAsc){
			std::stable_sort(sta.begin(), sta.end(), SortByLineAsc);
		}else{
			std::stable_sort(sta.begin(), sta.end(), SortByLineDesc);
		}
	}
	COpeLineData repData;
	j=(int)sta.size();
	repData.resize(sta.size());
	int opeSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetNextSeq();
	for (int i=0; i<j; i++){
		repData[i].nSeq = opeSeq;
		repData[i].cmemLine.SetString( sta[i]->pCmemLine->GetStringPtr(), sta[i]->pCmemLine->GetStringLength() );
		if( pStrLast == sta[i]->pCmemLine->GetStringPtr() ){
			// 元最終行に改行がないのでつける
			CEol cWork = GetDocument()->m_cDocEditor.GetNewLineCode();
			repData[i].cmemLine.AppendString( cWork.GetValue2(), cWork.GetLen() );
		}
	}
	if( pStrLast ){
		// 最終行の改行を削除
		CLineData& lastData = repData[repData.size()-1];
		int nLen = lastData.cmemLine.GetStringLength();
		bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
		while( 0 <nLen && WCODE::IsLineDelimiter(lastData.cmemLine[nLen-1], bExtEol) ){
			nLen--;
		}
		lastData.cmemLine._SetStringLength(nLen);
	}
	{
		// 2016.03.04 Moca sta内データの削除忘れ修正
		int nSize = (int)sta.size();
		for(int k = 0; k < nSize; k++){
			delete sta[k];
		}
	}

	CLayoutRange sSelectOld_Layout;
	GetDocument()->m_cLayoutMgr.LogicToLayout(sSelectOld, &sSelectOld_Layout);
	m_pCommanderView->ReplaceData_CEditView3(
		sSelectOld_Layout,
		NULL,
		&repData,
		false,
		m_pCommanderView->m_bDoing_UndoRedo?NULL:GetOpeBlk(),
		opeSeq,
		NULL
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
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}
	m_pCommanderView->RedrawAll();
}



//	from CViewCommander_New.cpp
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
	CLayoutInt		nMergeLayoutLines;

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
#if 0
		const CLayout* pcLayout=GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().GetY2()); //2007.10.09 kobake 単位混在バグ修正
		if( NULL != pcLayout && EOL_NONE != pcLayout->GetLayoutEol() ){
			sSelectOld.GetToPointer()->y++;
			//sSelectOld.GetTo().y++;
		}
#else
		// 2010.08.22 Moca ソートと仕様を合わせる
		const CDocLine* pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( sSelectOld.GetTo().GetY2() );
		if( NULL != pcDocLine && EOL_NONE != pcDocLine->GetEol() ){
			sSelectOld.GetToPointer()->y++;
		}
#endif
	}

	sSelectOld.SetFromX(CLogicInt(0));
	sSelectOld.SetToX(CLogicInt(0));

	//行選択されてない
	if(sSelectOld.IsLineOne()){
		return;
	}

	j=GetDocument()->m_cDocLineMgr.GetLineCount();
	nMergeLayoutLines = GetDocument()->m_cLayoutMgr.GetLineCount();

	CLayoutRange sSelectOld_Layout;
	GetDocument()->m_cLayoutMgr.LogicToLayout(sSelectOld, &sSelectOld_Layout);

	// 2010.08.22 NUL対応修正
	std::vector<CStringRef> lineArr;
	pLinew=NULL;
	int nLineLenw = 0;
	bool bMerge = false;
	lineArr.reserve(sSelectOld.GetTo().y - sSelectOld.GetFrom().GetY2());
	for( CLogicInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().y; i++ ){
		const wchar_t*	pLine = GetDocument()->m_cDocLineMgr.GetLine(i)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ) continue;
		if( NULL == pLinew || nLineLen != nLineLenw || wmemcmp(pLine, pLinew, nLineLen) ){
			lineArr.push_back( CStringRef(pLine, nLineLen) );
		}else{
			bMerge = true;
		}
		pLinew=pLine;
		nLineLenw=nLineLen;
	}
	if( bMerge ){
		COpeLineData repData;
		int nSize = (int)lineArr.size();
		repData.resize(nSize);
		int opeSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetNextSeq();
		for( int idx = 0; idx < nSize; idx++ ){
			repData[idx].nSeq = opeSeq;
			repData[idx].cmemLine.SetString( lineArr[idx].GetPtr(), lineArr[idx].GetLength() );
		}
		m_pCommanderView->ReplaceData_CEditView3(
			sSelectOld_Layout,
			NULL,
			&repData,
			false,
			m_pCommanderView->m_bDoing_UndoRedo?NULL:GetOpeBlk(),
			opeSeq,
			NULL
		);
	}else{
		// 2010.08.23 未変更なら変更しない
	}

	j-=GetDocument()->m_cDocLineMgr.GetLineCount();
	nMergeLayoutLines -= GetDocument()->m_cLayoutMgr.GetLineCount();

	//	選択エリアの復元
	m_pCommanderView->GetSelectionInfo().m_sSelect=sSelectOld_Layout;
	// 2010.08.22 座標混在バグ
	m_pCommanderView->GetSelectionInfo().m_sSelect.GetToPointer()->y -= nMergeLayoutLines;

	if(nCaretPosYOLD==m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y){
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom(), true );
	}else{
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo(), true );
	}
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}
	m_pCommanderView->RedrawAll();

	if(j){
		TopOkMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_DLGEDITVWCMDNW7), j);
	}else{
		InfoMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_DLGEDITVWCMDNW8) );
	}
}



//	from CViewCommander_New.cpp
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
	if( !OsSupportReconvert() ){
		
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
	}

	//サイズ取得し直し
	if (!UNICODE_BOOL && bUseUnicodeATOK) {
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
