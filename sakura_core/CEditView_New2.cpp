/*!	@file
	@brief CEditViewクラス

	@author Norio Nakatani
	@date	1998/12/08 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, MIK, asa-o, ao
	Copyright (C) 2001, hor, MIK, genta, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, novice, hor, ai, KK
	Copyright (C) 2003, Moca, ai, かろと
	Copyright (C) 2004, genta
	Copyright (C) 2005, genta, MIK, Moca, かろと, ryoji, zenryaku, D.S.Koba
	Copyright (C) 2006, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include "CEditView.h"
#include "debug.h"
#include "funccode.h"
#include "charcode.h"
#include "mymessage.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
#include "Clayout.h" /// 2002/2/3 aroka
#include "CDocLine.h" /// 2002/2/3 aroka
#include "CMarkMgr.h" /// 2002/2/3 aroka
#include "my_icmp.h"	//@@@ 2002.01.13 add
#include "CTextMetrics.h"
#include "CTypeSupport.h"
#include "parse/CWordParse.h"
#include "util/os.h"

/* 現在の色を指定 */
void CEditView::SetCurrentColor( HDC hdc, int nCOMMENTMODE )
{
	int				nColorIdx;
	COLORREF		colText;
	COLORREF		colBack;
//	if( NULL != m_hFontOld ){
//		::SelectObject( hdc, m_hFontOld );
//		m_hFontOld = NULL;
//	}
	nColorIdx = -1;
	switch( nCOMMENTMODE ){
// 2002/03/13 novice
	case COLORIDX_TEXT: /* テキスト */
	case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */
	case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */
	case COLORIDX_CTRLCODE:	/* コントロールコードである */
	case COLORIDX_DIGIT:	/* 半角数値である */
	case COLORIDX_KEYWORD1:	/* 強調キーワード1（登録単語）文字列である */
	case COLORIDX_KEYWORD2:	/* 強調キーワード2（登録単語）文字列である */
	case COLORIDX_KEYWORD3:	// 2005.01.13 MIK 強調キーワード3-10
	case COLORIDX_KEYWORD4:
	case COLORIDX_KEYWORD5:
	case COLORIDX_KEYWORD6:
	case COLORIDX_KEYWORD7:
	case COLORIDX_KEYWORD8:
	case COLORIDX_KEYWORD9:
	case COLORIDX_KEYWORD10:
	case COLORIDX_URL:	    /* URLである */
	case COLORIDX_SEARCH:	/* 検索文字列である */
	case COLORIDX_BRACKET_PAIR: /* 対括弧の強調表示である */	// 2002/12/18 ai
		nColorIdx = nCOMMENTMODE;
		break;
	case COLORIDX_COMMENT:	/* 行コメントである */
	case COLORIDX_BLOCK1:	/* ブロックコメント1である */
	case COLORIDX_BLOCK2:	/* ブロックコメント2である */	//@@@ 2001.03.10 by MIK
		nColorIdx = COLORIDX_COMMENT;
		break;

//@@@ 2001.11.17 add start MIK
	default:	/* 正規表現キーワード */
		if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 )
		{
			nColorIdx = nCOMMENTMODE - 1000;	//下駄を履かせているのをはずす
			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* フォントを選ぶ */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				GetFontset().ChooseFontHandle(
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
			return;
		}
		break;
//@@@ 2001.11.17 add end MIK

	}





	if( -1 != nColorIdx ){
		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bDisp ){
			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* フォントを選ぶ */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				GetFontset().ChooseFontHandle(
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
		}
	}

	return;
}





/* テキスト反転

	@param hdc      
	@param nLineNum 
	@param x        
	@param y        
	@param nX       

	@note
	CCEditView::DispLineNew() での作画(WM_PAINT)時に、1レイアウト行をまとめて反転処理するための関数。
	範囲選択の随時更新は、CEditView::DrawSelectArea() が選択・反転解除を行う。
	
*/
void CEditView::DispTextSelected(
	HDC				hdc,		//!< 作画対象ビットマップを含むデバイス
	CLayoutInt		nLineNum,	//!< 反転処理対象レイアウト行番号(0開始)
	const CMyPoint&	ptXY,		//!< (相対レイアウト0桁目の左端座標, 対象行の上端座標)
	CLayoutInt		nX_Layout	//!< 対象行の終了桁位置。　[ABC\n]なら改行の後ろで4
)
{
	CLayoutInt	nSelectFrom;
	CLayoutInt	nSelectTo;
	RECT		rcClip;
	int			nLineHeight = GetTextMetrics().GetHankakuDy();
	int			nCharWidth = GetTextMetrics().GetHankakuDx();
	HRGN		hrgnDraw;
	const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );

	/* 選択範囲内の行かな */
//	if( IsTextSelected() ){
		if( nLineNum >= GetSelectionInfo().m_sSelect.GetFrom().y && nLineNum <= GetSelectionInfo().m_sSelect.GetTo().y ){
			if( GetSelectionInfo().IsBoxSelecting() ){		/* 矩形範囲選択中 */
				nSelectFrom = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
				nSelectTo   = GetSelectionInfo().m_sSelect.GetTo().GetX2();
				// 2006.09.30 Moca From 矩形選択時[EOF]とその右側は反転しないように修正。処理を追加
				if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 <= nLineNum ){
					CLayoutPoint ptEnd(0, 0);
					m_pcEditDoc->m_cLayoutMgr.GetEndLayoutPos( &ptEnd );
					if( ptEnd.y == nLineNum ){
						if( ptEnd.GetX2() < nSelectFrom ){
							nSelectFrom = ptEnd.GetX2();
						}
						if( ptEnd.GetX2() < nSelectTo ){
							nSelectTo = ptEnd.GetX2();
						}
					}
				}
				// 2006.09.30 Moca To
			}
			else{
				if( GetSelectionInfo().m_sSelect.IsLineOne() ){
					nSelectFrom = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
					nSelectTo   = GetSelectionInfo().m_sSelect.GetTo().GetX2();
				}
				else{
					if( nLineNum == GetSelectionInfo().m_sSelect.GetFrom().y ){
						nSelectFrom = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
						nSelectTo   = nX_Layout;
					}
					else if( nLineNum == GetSelectionInfo().m_sSelect.GetTo().GetY2() ){
						nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
						nSelectTo   = GetSelectionInfo().m_sSelect.GetTo().GetX2();
					}
					else{
						nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
						nSelectTo   = nX_Layout;
					}
				}
			}
			// 2006.05.24 Moca 矩形選択/フリーカーソル選択(選択開始/終了行)で
			// To < From になることがある。必ず From < To になるように入れ替える。
			if( nSelectTo < nSelectFrom ){
				CLayoutInt t = nSelectFrom;
				nSelectFrom = nSelectTo;
				nSelectTo = t;
			}

			// 2006.03.28 Moca 表示域外なら何もしない
			if( GetTextArea().GetRightCol() < nSelectFrom ){
				return;
			}
			if( nSelectTo <= GetTextArea().GetViewLeftCol() ){
				return;
			}

			if( nSelectFrom < GetTextArea().GetViewLeftCol() ){
				nSelectFrom = GetTextArea().GetViewLeftCol();
			}
			rcClip.left   = ptXY.x + (Int)nSelectFrom * ( nCharWidth );
			rcClip.right  = ptXY.x + (Int)nSelectTo   * ( nCharWidth );
			rcClip.top    = ptXY.y;
			rcClip.bottom = ptXY.y + nLineHeight;

			// 2005/04/02 かろと ０文字マッチだと反転幅が０となり反転されないので、1/3文字幅だけ反転させる
			// 2005/06/26 zenryaku 選択解除でキャレットの残骸が残る問題を修正
			// 2005/09/29 ryoji スクロール時にキャレットのようなゴミが表示される問題を修正
			if (GetSelectionInfo().IsTextSelected() && rcClip.right == rcClip.left &&
				GetSelectionInfo().m_sSelect.IsLineOne() &&
				GetSelectionInfo().m_sSelect.GetFrom().x >= GetTextArea().GetViewLeftCol())
			{
				rcClip.right = rcClip.left + (nCharWidth/3 == 0 ? 1 : nCharWidth/3);
			}

			// 2006.03.28 Moca ウィンドウ幅が大きいと正しく反転しない問題を修正
			if( rcClip.right > GetTextArea().GetAreaRight() ){
				rcClip.right = GetTextArea().GetAreaRight();
			}
			
			HBRUSH hBrush    = ::CreateSolidBrush( SELECTEDAREA_RGB );

			int    nROP_Old  = ::SetROP2( hdc, SELECTEDAREA_ROP2 );
			HBRUSH hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
			hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
			::PaintRgn( hdc, hrgnDraw );
			::DeleteObject( hrgnDraw );

			SetROP2( hdc, nROP_Old );
			SelectObject( hdc, hBrushOld );
			DeleteObject( hBrush );
		}
//	}
	return;
}




/* 現在位置が検索文字列に該当するか */
//2002.02.08 hor
//正規表現で検索したときの速度改善のため、マッチ先頭位置を引数に追加
bool CEditView::IsSearchString(
	const wchar_t*	pszData,
	CLogicInt		nDataLen,
	CLogicInt		nPos,
	CLogicInt*		pnSearchStart,
	CLogicInt*		pnSearchEnd
)
{
	CLogicInt		nKeyLength;

	//	From Here Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	*pnSearchStart = nPos;	// 2002.02.08 hor

	if( m_sCurSearchOption.bRegularExp ){
		/* 行頭ではない? */
		/* 行頭検索チェックは、CBregexpクラス内部で実施するので不要 2003.11.01 かろと */

		/* 位置を0でMatchInfo呼び出すと、行頭文字検索時に、全て true　となり、
		** 画面全体が検索文字列扱いになる不具合修正
		** 対策として、行頭を MacthInfoに教えないといけないので、文字列の長さ・位置情報を与える形に変更
		** 2003.05.04 かろと
		*/
		if( m_CurRegexp.Match( pszData, nDataLen, nPos ) ){
			*pnSearchStart = m_CurRegexp.GetIndex();	// 2002.02.08 hor
			*pnSearchEnd = m_CurRegexp.GetLastIndex();
	//	To Here Jun. 26, 2001 genta
			return true;

		}else{
			return false;
		}
	}else{
		nKeyLength = CLogicInt(wcslen( m_szCurSrchKey ));		/* 検索条件 */

		// 2001/06/23 単語単位の検索のために追加
		if( m_pShareData->m_Common.m_sSearch.m_sSearchOption.bWordOnly ){	/* 検索／置換  1==単語のみ検索 */
			/* 現在位置の単語の範囲を調べる */
			/* 現在位置の単語の範囲を調べる */
			CLogicInt nIdxFrom, nIdxTo;
			if( !CWordParse::WhereCurrentWord_2( pszData, nDataLen, nPos, &nIdxFrom, &nIdxTo, NULL, NULL ) ){
				return false;
			}
			if( nPos != nIdxFrom || nKeyLength != nIdxTo - nIdxFrom ){
				return false;
			}
		}

		//検索条件が未定義 または 検索条件の長さより調べるデータが短いときはヒットしない
		if( 0 == nKeyLength || nKeyLength > nDataLen - nPos ){
			return false;
		}
		//英大文字小文字の区別をするかどうか
		if( m_sCurSearchOption.bLoHiCase ){	/* 1==英大文字小文字の区別 */
			if( 0 == wmemcmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return true;
			}
		}else{
			if( 0 == auto_memicmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return true;
			}
		}
	}
	return false;
}



//======================================================================
//!対括弧の対応表
//2007.10.16 kobake
struct KAKKO_T{
	wchar_t *sStr;
	wchar_t *eStr;
};
const KAKKO_T g_aKakkos[] = {
	//半角
	{ L"(", L")", },
	{ L"[", L"]", },
	{ L"{", L"}", },
	{ L"<", L">", },
	{ L"｢", L"｣", },
	//全角
	{ L"【", L"】", },
	{ L"『", L"』", },
	{ L"「", L"」", },
	{ L"＜", L"＞", },
	{ L"≪", L"≫", },
	{ L"《", L"》", },
	{ L"（", L"）", },
	{ L"〈", L"〉", },
	{ L"｛", L"｝", },
	{ L"〔", L"〕", },
	{ L"［", L"］", },
	{ L"“", L"”", },
	{ L"〝", L"〟", },
	//終端
	{ NULL, NULL, },
};



//	Jun. 16, 2000 genta
/*!
	@brief 対括弧の検索

	カーソル位置の括弧に対応する括弧を探す。カーソル位置が括弧でない場合は
	カーソルの後ろの文字が括弧かどうかを調べる。

	カーソルの前後いずれもが括弧でない場合は何もしない。

	括弧が半角か全角か、及び始まりか終わりかによってこれに続く4つの関数に
	制御を移す。

	@param LayoutX [in] 検索開始点の物理座標X
	@param LayoutY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param mode [in/out] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
						 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる (このbitを参照)
						 bit2(out) : 見つかった位置         0:後ろ      1:前     (このbitを更新)

	@retval true 成功
	@retval false 失敗

	@author genta
	@date Jun. 16, 2000 genta
	@date Feb. 03, 2001 MIK 全角括弧に対応
	@date Sep. 18, 2002 ai modeの追加
*/
bool CEditView::SearchBracket(
	const CLayoutPoint&	ptLayout,
	CLayoutPoint*		pptLayoutNew,
	int*				mode
)
{
	CLogicInt len;	//	行の長さ
	
	CLogicPoint ptPos;
	//int PosX, PosY;	//	物理位置

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptLayout, &ptPos );
	const wchar_t *cline = m_pcEditDoc->m_cDocLineMgr.GetLineStr( ptPos.GetY2(), &len );

	//	Jun. 19, 2000 genta
	if( cline == NULL )	//	最後の行に本文がない場合
		return false;

	// 括弧処理 2007.10.16 kobake
	{
		const KAKKO_T* p;
		for( p = g_aKakkos; p->sStr != NULL;  p++ )
		{
			if( wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketForward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
			else if( wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketBackward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
		}
	}

	// 02/09/18 ai Start
	if( 0 == ( *mode & 2 ) ){
		/* カーソルの前方を調べない場合 */
		return false;
	}
	*mode |= 4;
	// 02/09/18 ai End

	//	括弧が見つからなかったら，カーソルの直前の文字を調べる

	if( ptPos.x <= 0 ){
//		::MessageBoxA( NULL, "NO DATA", "Bracket", MB_OK );
		return false;	//	前の文字はない
	}

	const wchar_t *bPos = CNativeW2::GetCharPrev( cline, ptPos.x, cline + ptPos.x );
	int nCharSize = cline + ptPos.x - bPos;
//	m_nCharSize = nCharSize;	// 02/10/01 対括弧の文字サイズ設定 ai
	// 括弧処理 2007.10.16 kobake
	if(nCharSize==1){
		const KAKKO_T* p;
		ptPos.x = bPos - cline;
		for( p = g_aKakkos; p->sStr != NULL; p++ )
		{
			if( wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketForward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
			else if( wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketBackward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
		}
	}
	return false;
}

/*!
	@brief 半角対括弧の検索:順方向

	@author genta

	@param PosX   [in] 検索開始点の物理座標X
	@param PosY   [in] 検索開始点の物理座標Y
	@param NewX   [out] 移動先のレイアウト座標X
	@param NewY   [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字
	@param dnChar [in] 括弧を閉じる文字列
	@param mode   [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
// 03/01/08 ai
bool CEditView::SearchBracketForward(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	upChar,
	const wchar_t*	dnChar,
	int*			mode
)
{
	CDocLine* ci;

	int			len;
	const wchar_t* cPos;
	const wchar_t* nPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int			level = 0;

	CLayoutPoint ptColLine;

	CLayoutInt	nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ( GetTextArea().GetBottomLine() ) - ptColLine.y;					// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( ptPos.GetY2() );
	cline = ci->m_cLine.GetStringPtr( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos < lineend ){
			nPos = CNativeW2::GetCharNext( cline, len, cPos );
			if( nPos - cPos > 1 ){
				//	skip
				cPos = nPos;
				continue;
			}
			// 03/01/08 ai Start
			if( wcsncmp(upChar, cPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, cPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	見つかった！
				ptPos.x = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
				//	Happy Ending
			}
			cPos = nPos;	//	次の文字へ
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// 表示領域外を調べないモードで表示領域の終端の場合
			//SendStatusMessage( "対括弧の検索を中断しました" );
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y++;
		ci = ci->m_pNext;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_cLine.GetStringPtr( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}

/*!
	@brief 半角対括弧の検索:逆方向

	@author genta

	@param PosX [in] 検索開始点の物理座標X
	@param PosY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字
	@param dnChar [in] 括弧を閉じる文字列
	@param mode [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchBracketBackward(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	dnChar,
	const wchar_t*	upChar,
	int*			mode
)
{
	CDocLine* ci;

	int			len;
	const wchar_t* cPos;
	const wchar_t* pPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int			level = 1;
	
	CLayoutPoint ptColLine;
	//int			nCol, nLine;

	CLayoutInt		nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ptColLine.y - GetTextArea().GetViewTopLine();										// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( ptPos.GetY2() );
	cline = ci->m_cLine.GetStringPtr( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos > cline ){
			pPos = CNativeW2::GetCharPrev( cline, len, cPos );
			if( cPos - pPos > 1 ){
				//	skip
				cPos = pPos;
				continue;
			}
			// 03/01/08 ai Start
			if( wcsncmp(upChar, pPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, pPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	見つかった！
				ptPos.x = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
				//	Happy Ending
			}
			cPos = pPos;	//	次の文字へ
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// 表示領域外を調べないモードで表示領域の先頭の場合
			//SendStatusMessage( "対括弧の検索を中断しました" );
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y--;
		ci = ci->m_pPrev;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_cLine.GetStringPtr( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}

//@@@ 2001.02.03 Start by MIK:
/*!
	@brief 全角対括弧の検索:順方向

	@author MIK

	@param PosX [in] 検索開始点の物理座標X
	@param PosY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字へのポインタ
	@param dnChar [in] 括弧を閉じる文字列へのポインタ
	@param mode [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchBracketForward2(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	upChar,
	const wchar_t*	dnChar,
	int*			mode
)
{
	CDocLine* ci;

	int len;
	const wchar_t* cPos;
	const wchar_t* nPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int level = 0;
	
	CLayoutPoint ptColLine;
//	int			nCol, nLine;

	CLayoutInt	nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ( GetTextArea().GetBottomLine() ) - ptColLine.y;					// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( ptPos.GetY2() );
	cline = ci->m_cLine.GetStringPtr( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos < lineend ){
			nPos = CNativeW2::GetCharNext( cline, len, cPos );
			if( nPos - cPos != 1 ){
				//	skip
				cPos = nPos;
				continue;
			}
			if( wcsncmp(upChar, cPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, cPos, 1) == 0 ){
				--level;
			}

			if( level == 0 ){	//	見つかった！
				ptPos.x = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
			}
			cPos = nPos;	//	次の文字へ
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// 表示領域外を調べないモードで表示領域の終端の場合
			//SendStatusMessage( "対括弧の検索を中断しました" );
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y++;
		ci = ci->m_pNext;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_cLine.GetStringPtr( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}
//@@@ 2001.02.03 End

//@@@ 2001.02.03 Start by MIK:
/*!
	@brief 全角対括弧の検索:逆方向

	@author MIK

	@param PosX [in] 検索開始点の物理座標X
	@param PosY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字へのポインタ
	@param dnChar [in] 括弧を閉じる文字列へのポインタ
	@param mode [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchBracketBackward2(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	dnChar,
	const wchar_t*	upChar,
	int*			mode
)
{
	CDocLine* ci;

	int len;
	const wchar_t* cPos;
	const wchar_t* pPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int level = 1;

	CLayoutPoint ptColLine;
	//int	nCol, nLine;

	CLayoutInt nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ptColLine.y - GetTextArea().GetViewTopLine();										// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( ptPos.GetY2() );
	cline = ci->m_cLine.GetStringPtr( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos > cline ){
			pPos = CNativeW2::GetCharPrev( cline, len, cPos );
			if( cPos - pPos != 1 ){
				//	skip
				cPos = pPos;
				continue;
			}
			if( wcsncmp(upChar, pPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, pPos, 1) == 0 ){
				--level;
			}

			if( level == 0 ){	//	見つかった！
				ptPos.x = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
			}
			cPos = pPos;	//	次の文字へ
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// 表示領域外を調べないモードで表示領域の先頭の場合
			//SendStatusMessage( "対括弧の検索を中断しました" );
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y--;
		ci = ci->m_pPrev;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_cLine.GetStringPtr( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}
//@@@ 2001.02.03 End

//@@@ 2003.01.09 Start by ai:
/*!
	@brief 括弧判定

	@author ai

	@param pLine [in] 
	@param x
	@param size

	@retval true 括弧
	@retval false 非括弧
*/
bool CEditView::IsBracket( const wchar_t *pLine, CLogicInt x, CLogicInt size )
{
	// 括弧処理 2007.10.16 kobake
	if( size == 1 ){
		const struct KAKKO_T *p;
		for( p = g_aKakkos; p->sStr != NULL; p++ )
		{
			if( wcsncmp( p->sStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
			else if( wcsncmp( p->eStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
		}
	}

	return false;
}
//@@@ 2003.01.09 End

//!	現在のカーソル行位置を履歴に登録する
void CEditView::AddCurrentLineToHistory( void )
{
	CLogicPoint ptPos;
	//int PosX, PosY;	//	物理位置（改行単位の計算）

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( GetCaret().GetCaretLayoutPos(), &ptPos );

	CMarkMgr::CMark m( ptPos );
	m_cHistory->Add( m );

}


//	2001/06/18 Start by asa-o: 補完ウィンドウ用のキーワードヘルプ表示
bool  CEditView::ShowKeywordHelp( POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin)
{
	CNativeW2	cmemCurText;
	RECT		rcTipWin,
				rcDesktop;

	if( m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp ){ /* キーワードヘルプを使用する */
		if( m_bInMenuLoop == FALSE	&&	/* メニュー モーダル ループに入っていない */
			0 != m_dwTipTimer			/* 辞書Tipを表示していない */
		){
			cmemCurText.SetString( pszHelp );

			/* 既に検索済みか */
			if( CNativeW2::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) ){
				/* 該当するキーがなかった */
				if( !m_cTipWnd.m_KeyWasHit ){
					return false;
				}
			}else{
				m_cTipWnd.m_cKey = cmemCurText;
				/* 検索実行 */
				if(FALSE == KeySearchCore(&m_cTipWnd.m_cKey))	// 2006.04.10 fon
					return FALSE;
			}
			m_dwTipTimer = 0;	/* 辞書Tipを表示している */

		// 2001/06/19 Start by asa-o: 辞書Tipの表示位置調整
			// 辞書Tipのサイズを取得
			m_cTipWnd.GetWindowSize(&rcTipWin);

			//	May 01, 2004 genta マルチモニタ対応
			::GetMonitorWorkRect( m_cTipWnd.GetHwnd(), &rcDesktop );

			// 右に入る
			if(prcHokanWin->right + rcTipWin.right < rcDesktop.right){
				// そのまま
			}else
			// 左に入る
			if(rcDesktop.left < prcHokanWin->left - rcTipWin.right ){
				// 左に表示
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}else
			// どちらもスペースが無いとき広いほうに表示
			if(rcDesktop.right - prcHokanWin->right > prcHokanWin->left ){
				// 右に表示 そのまま
			}else{
				// 左に表示
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}
		// 2001/06/19 End

			/* 辞書Tipを表示 */
			m_cTipWnd.Show( po.x, po.y , NULL , &rcTipWin);
			return true;
		}
	}
	return false;
}
//	2001/06/18 End

#if 0
/*!	コントロールコードを "^@" 表示する。
	0x00 - 0x1f は "^@" - "^_" で表示する。
	0x7f は "^?" で表示する。
	その他は "・" で表示する。
*/
int CEditView::DispCtrlCode( HDC hdc, int x, int y, const unsigned char* pData, int nLength )
{
	int		i, x1, y1;
	unsigned char	c;
	HPEN	hPen, hPenOld;

	x1 = GetTextMetrics().GetHankakuWidth() / 3;
	y1 = GetTextMetrics().GetHankakuHeight() / 5;

	hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CTRLCODE].m_colTEXT );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	for( i = 0; i < nLength; i++, pData++ )
	{
		if     ( (*pData) <= 0x1f ) c = '@' + (*pData);
		else if( (*pData) == 0x7f ) c = '?';
		else                        c = '･';

		//文字を表示する
		DispText( hdc, x, y, &c, 1 );

		//制御文字を示す記号を描画する
		::MoveToEx( hdc, x, y + y1, NULL );
		::LineTo( hdc, x + x1, y );
		::LineTo( hdc, x + x1 * 2, y + y1 );

		x += GetTextMetrics().GetHankakuWidth();
	}

	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );

	return nLength;
}
#endif


/*[EOF]*/
 
