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
	Copyright (C) 2006, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditView.h"
#include "debug.h"
#include "funccode.h"
#include "charcode.h"
#include "mymessage.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
#include "etc_uty.h"
#include "Clayout.h" /// 2002/2/3 aroka
#include "CDocLine.h" /// 2002/2/3 aroka
#include "CMarkMgr.h" /// 2002/2/3 aroka
#include "my_icmp.h"	//@@@ 2002.01.13 add


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
				ChooseFontHandle(
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
				ChooseFontHandle(
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
		}
	}

	return;
}


/* 行番号表示 */
void CEditView::DispLineNumber(
		HDC						hdc,
		const CLayout*			pcLayout,
		int						nLineNum,
		int						y
)
{
	RECT			rcClip;
	HBRUSH			hBrush;
	COLORREF		colTextColorOld;
	COLORREF		colBkColorOld;
	char			szLineNum[18];
	int				nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	int				nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
	int				nLineCols;
	UINT			fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	HPEN			hPen, hPenOld;
	int				nColorIndex;
	const CDocLine*	pCDocLine;
	//	Sep. 23, 2002 genta 共通式のくくりだし
	int				nLineNumAreaWidth = m_nViewAlignLeftCols * nCharWidth;

	nColorIndex = COLORIDX_GYOU;	/* 行番号 */
	if( NULL != pcLayout ){
//		pCDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( pcLayout->m_nLinePhysical );
		pCDocLine = pcLayout->m_pCDocLine;

		if( m_pcEditDoc->IsModified()	/* ドキュメントが無変更の状態か */
		 && pCDocLine->IsModifyed() ){		/* 変更フラグ */
//			if( 0 == pCDocLine->m_nModifyCount ){	/* 変更回数 */
				nColorIndex = COLORIDX_GYOU_MOD;	/* 行番号（変更行） */
//			}
//		}else{
//			if( /* FALSE == m_pcEditDoc->IsModified() && --*/ /* ドキュメントが無変更の状態か */
//				0 < pCDocLine->m_nModifyCount	/* 変更回数 */
//			){
//				nColorIndex = COLORIDX_GYOU_MODSAVE;	/* 行番号（変更&保存済） */
//			}
		}
	}

	int type = pCDocLine->IsDiffMarked();
	{	//DIFF差分マーク表示	//@@@ 2002.05.25 MIK
		if( type )
		{
			switch( type )
			{
			case MARK_DIFF_APPEND:	//追加
				if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_DIFF_APPEND].m_bDisp )
					nColorIndex = COLORIDX_DIFF_APPEND;
				break;
			case MARK_DIFF_CHANGE:	//変更
				if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_DIFF_CHANGE].m_bDisp )
					nColorIndex = COLORIDX_DIFF_CHANGE;
				break;
			case MARK_DIFF_DELETE:	//削除
			case MARK_DIFF_DEL_EX:	//削除
				if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_DIFF_DELETE].m_bDisp )
					nColorIndex = COLORIDX_DIFF_DELETE;
				break;
			}
		}
	}

	/* 02/10/16 ai Start */
	// ブックマークの表示
	if(pCDocLine->IsBookMarked()){
		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_MARK].m_bDisp ) {
			nColorIndex = COLORIDX_MARK;
		}
	}
	/* 02/10/16 ai End */

//	if( m_pcEditDoc->GetDocumentAttribute().m_bDispLINE ){	/* 行番号表示／非表示 */
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[/*nColorIndex*/COLORIDX_GYOU].m_bDisp ){	/* 行番号表示／非表示 */
		/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		if( m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){
			/* 論理行番号表示モード */
			if( NULL == pcLayout || 0 != pcLayout->m_nOffset ){
				strcpy( szLineNum, " " );
			}else{
				_itoa( pcLayout->m_nLinePhysical + 1, szLineNum, 10 );	/* 対応する論理行番号 */
			}
		}else{
			/* 物理行（レイアウト行）番号表示モード */
			_itoa( nLineNum + 1, szLineNum, 10 );
		}
		nLineCols = lstrlen( szLineNum );

		colTextColorOld = ::SetTextColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );	/* 行番号の色 */
		//	Sep. 23, 2002 余白をテキストの背景色にする
		colBkColorOld = ::SetBkColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );		/* テキスト背景の色 */

		HFONT	hFontOld;
		/* フォントを選ぶ */
		hFontOld = (HFONT)::SelectObject( hdc,
			ChooseFontHandle(
				m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_bFatFont,
				m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_bUnderLine
			)
		);
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_bFatFont ){	/* 太字か */
//			hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//		}else{
//			hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//		}

		/* 余白を埋める */
		rcClip.left = nLineNumAreaWidth;
		rcClip.right = m_nViewAlignLeft;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOut( hdc,
			rcClip.left,
			y, fuOptions,
			&rcClip, " ", 1, m_pnDx
		);
		
		//	Sep. 23, 2002 余白をテキストの背景色にするため，背景色の設定を移動
		SetBkColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colBACK );		/* 行番号背景の色 */

//		/* 行番号のテキストを表示 */
//		m_pShareData->m_Types[nIdx].m_nLineTermType = 1;			/* 行番号区切り 0=なし 1=縦線 2=任意 */
//		m_pShareData->m_Types[nIdx].m_cLineTermChar = ':';			/* 行番号区切り文字 */

		int drawNumTop = (m_nViewAlignLeftCols - nLineCols - 1) * ( nCharWidth );

		/* 行番号区切り 0=なし 1=縦線 2=任意 */
		if( 2 == m_pcEditDoc->GetDocumentAttribute().m_nLineTermType ){
			//	Sep. 22, 2002 genta
			szLineNum[ nLineCols ] = m_pcEditDoc->GetDocumentAttribute().m_cLineTermChar;
			szLineNum[ ++nLineCols ] = '\0';
			//char szLineTerm[2];
			//wsprintf( szLineTerm, "%c", m_pcEditDoc->GetDocumentAttribute().m_cLineTermChar );	/* 行番号区切り文字 */
			//strcat( szLineNum, szLineTerm );
		}
		rcClip.left = 0;
		//rcClip.right = m_nViewAlignLeft/* - 3*/;
		//	Sep. 23, 2002 genta
		rcClip.right = nLineNumAreaWidth;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOut( hdc,
			drawNumTop,
			y, fuOptions,
			&rcClip, szLineNum, nLineCols, m_pnDx // Sep. 22, 2002 genta
		);


//		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_colorGYOU );


		/* 行番号区切り 0=なし 1=縦線 2=任意 */
		if( 1 == m_pcEditDoc->GetDocumentAttribute().m_nLineTermType ){
			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );
			hPenOld = (HPEN)::SelectObject( hdc, hPen );
			::MoveToEx( hdc, nLineNumAreaWidth - 2, y, NULL );
			::LineTo( hdc, nLineNumAreaWidth - 2, y + nLineHeight );
			::SelectObject( hdc, hPenOld );
			::DeleteObject( hPen );
		}
		::SetTextColor( hdc, colTextColorOld );
		::SetBkColor( hdc, colBkColorOld );
		::SelectObject( hdc, hFontOld );
	}else{
		rcClip.left = 0;
		//	Sep. 23 ,2002 genta 余白はテキスト色のまま残す
		rcClip.right = m_nViewAlignLeft - m_pShareData->m_Common.m_nLineNumRightSpace;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
//		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_colorBACK );
		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex/*COLORIDX_TEXT*/].m_colBACK );
		::FillRect( hdc, &rcClip, hBrush );
		::DeleteObject( hBrush );
		
		// From Here Mar. 5, 2003, Moca
		// 行番号とテキストの隙間の描画漏れ
		rcClip.left = rcClip.right;
		rcClip.right = m_nViewAlignLeft;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
		::FillRect( hdc, &rcClip, hBrush );
		::DeleteObject( hBrush );
		// To Here Mar. 5, 2003, Moca
	}

// From Here 2001.12.03 hor
	/* とりあえずブックマークに縦線 */
	if(pCDocLine->IsBookMarked() &&	// Dec. 24, 2002 ai
		(FALSE == m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_MARK].m_bDisp)){
		hPen = ::CreatePen( PS_SOLID, 2, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, 1, y, NULL );
		::LineTo( hdc, 1, y + nLineHeight );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
	}
// To Here 2001.12.03 hor

	if( type )	//DIFF差分マーク表示	//@@@ 2002.05.25 MIK
	{
		int	cy = y + nLineHeight / 2;

		hPen = ::CreatePen( PS_SOLID, 1, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );

		switch( type )
		{
		case MARK_DIFF_APPEND:	//追加
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo( hdc, 6, cy );
			::MoveToEx( hdc, 4, cy - 2, NULL );
			::LineTo( hdc, 4, cy + 3 );
			break;

		case MARK_DIFF_CHANGE:	//変更
			::MoveToEx( hdc, 3, cy - 4, NULL );
			::LineTo( hdc, 3, cy );
			::MoveToEx( hdc, 3, cy + 2, NULL );
			::LineTo( hdc, 3, cy + 3 );
			break;

		case MARK_DIFF_DELETE:	//削除
			cy -= 3;
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo( hdc, 5, cy );
			::LineTo( hdc, 3, cy + 2 );
			::LineTo( hdc, 3, cy );
			::LineTo( hdc, 7, cy + 4 );
			break;
		
		case MARK_DIFF_DEL_EX:	//削除(EOF)
			cy += 3;
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo( hdc, 5, cy );
			::LineTo( hdc, 3, cy - 2 );
			::LineTo( hdc, 3, cy );
			::LineTo( hdc, 7, cy - 4 );
			break;
		}

		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
	}
	
	return;
}



/* テキスト表示
	@@@ 2002.09.22 YAZAKI const unsigned char* pDataを、const char* pDataに変更
*/
int CEditView::DispText( HDC hdc, int x, int y, const char* pData, int nLength )
{
	if( 0 >= nLength ){
		return 0;
	}
	RECT	rcClip;
	int		nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	int		nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
	UINT	fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	rcClip.left = x;
	rcClip.right = rcClip.left + ( nCharWidth ) * nLength;
	rcClip.top = y;
	if( rcClip.left < m_nViewAlignLeft ){
		rcClip.left = m_nViewAlignLeft;
	}
	if( rcClip.left < rcClip.right
	 && rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft
	 && rcClip.top >= m_nViewAlignTop
	){
		rcClip.bottom = y + nLineHeight;

		//@@@	From Here 2002.01.30 YAZAKI ExtTextOutの制限回避
		if( rcClip.right - rcClip.left > m_nViewCx ){
			rcClip.right = rcClip.left + m_nViewCx;
		}
		int nBefore = 0;	//	ウィンドウの左にあふれた文字数
		int nAfter = 0;		//	ウィンドウの右にあふれた文字数
		if ( x < 0 ){
			int nLeft = ( 0 - x ) / nCharWidth - 1;
			while (nBefore < nLeft){
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nBefore += CMemory::GetSizeOfChar( pData, nLength, nBefore );
			}
		}
		if ( rcClip.right < x + nCharWidth * nLength ){
			//	-1してごまかす（うしろはいいよね？）
			nAfter = (x + nCharWidth * nLength - rcClip.right) / nCharWidth - 1;
		}
		::ExtTextOut( hdc, x + nBefore * nCharWidth, y, fuOptions, &rcClip, &pData[nBefore], nLength - nBefore - nAfter, m_pnDx );
		//@@@	To Here 2002.01.30 YAZAKI ExtTextOutの制限回避
	}
	return nLength;

}


/* テキスト反転 */
void CEditView::DispTextSelected( HDC hdc, int nLineNum, int x, int y, int nX  )
{
//	MYTRACE( "CEditView::DispTextSelected()\n" );

	int			nROP_Old;
	int			nSelectFrom;
	int			nSelectTo;
	RECT		rcClip;
//	HPEN		hPen;
//	HPEN		hPenOld;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	int			nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	int			nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
	HRGN		hrgnDraw;
	const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum );

	/* 選択範囲内の行かな */
//	if( IsTextSelected() ){
		if( nLineNum >= m_nSelectLineFrom && nLineNum <= m_nSelectLineTo ){
			if( m_bBeginBoxSelect){		/* 矩形範囲選択中 */
				nSelectFrom = m_nSelectColmFrom;
				nSelectTo   = m_nSelectColmTo;
			}else{
				if( m_nSelectLineFrom == m_nSelectLineTo ){
						nSelectFrom = m_nSelectColmFrom;
						nSelectTo   = m_nSelectColmTo;
				}else{
					if( nLineNum == m_nSelectLineFrom ){
						nSelectFrom = m_nSelectColmFrom;
						nSelectTo   = nX;
					}else
					if( nLineNum == m_nSelectLineTo ){
						nSelectFrom = pcLayout ? pcLayout->GetIndent() : 0;
						nSelectTo   = m_nSelectColmTo;
					}else{
						nSelectFrom = pcLayout ? pcLayout->GetIndent() : 0;
						nSelectTo   = nX;
					}
				}
			}
			// 2006.05.24 Moca 矩形選択/フリーカーソル選択(選択開始/終了行)で
			// To < From になることがある。必ず From < To になるように入れ替える。
			if( nSelectTo < nSelectFrom ){
				int t = nSelectFrom;
				nSelectFrom = nSelectTo;
				nSelectTo = t;
			}
			// 2006.03.28 Moca 表示域外なら何もしない
			if( m_nViewLeftCol + m_nViewColNum < nSelectFrom ){
				return;
			}
			if( nSelectTo <= m_nViewLeftCol ){
				return;
			}

			if( nSelectFrom < m_nViewLeftCol ){
				nSelectFrom = m_nViewLeftCol;
			}
			rcClip.left   = x + nSelectFrom * ( nCharWidth );
			rcClip.right  = x + nSelectTo   * ( nCharWidth );
			rcClip.top    = y;
			rcClip.bottom = y + nLineHeight;
			// 2005/04/02 かろと ０文字マッチだと反転幅が０となり反転されないので、1/3文字幅だけ反転させる
			// 2005/06/26 zenryaku 選択解除でキャレットの残骸が残る問題を修正
			// 2005/09/29 ryoji スクロール時にキャレットのようなゴミが表示される問題を修正
			if (IsTextSelected() && rcClip.right == rcClip.left &&
				m_nSelectLineFrom == m_nSelectLineTo &&
				m_nSelectColmFrom >= m_nViewLeftCol)
			{
				rcClip.right = rcClip.left + (nCharWidth/3 == 0 ? 1 : nCharWidth/3);
			}
			// 2006.03.28 Moca ウィンドウ幅が大きいと正しく反転しない問題を修正
			if( rcClip.right > m_nViewAlignLeft + m_nViewCx ){
				rcClip.right = m_nViewAlignLeft + m_nViewCx;
			}
			
			hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
			nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );
			hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
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
BOOL CEditView::IsSearchString( const char* pszData, int nDataLen, int nPos, int* pnSearchStart, int* pnSearchEnd )
{
	int		nKeyLength;

	//	From Here Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	*pnSearchStart = nPos;	// 2002.02.08 hor

	if( m_bCurSrchRegularExp ){
		/* 行頭ではない? */
		/* 行頭検索チェックは、CBregexpクラス内部で実施するので不要 2003.11.01 かろと */

		/* 位置を0でMatchInfo呼び出すと、行頭文字検索時に、全て true　となり、
		** 画面全体が検索文字列扱いになる不具合修正
		** 対策として、行頭を MacthInfoに教えないといけないので、文字列の長さ・位置情報を与える形に変更
		** 2003.05.04 かろと
		*/
		if( m_CurRegexp.Match( pszData, nDataLen, nPos )
		){
			*pnSearchStart = m_CurRegexp.GetIndex();	// 2002.02.08 hor
			*pnSearchEnd = m_CurRegexp.GetLastIndex();
	//	To Here Jun. 26, 2001 genta
			return TRUE;

		}else{
			return FALSE;
		}
	}else{
		nKeyLength = lstrlen( m_szCurSrchKey );		/* 検索条件 */

		// 2001/06/23 単語単位の検索のために追加
		if( m_pShareData->m_Common.m_bWordOnly ){	/* 検索／置換  1==単語のみ検索 */
			/* 現在位置の単語の範囲を調べる */
			/* 現在位置の単語の範囲を調べる */
			int nIdxFrom, nIdxTo;
			if( false == CDocLineMgr::WhereCurrentWord_2( pszData, nDataLen, nPos, &nIdxFrom, &nIdxTo, NULL, NULL ) ){
				return FALSE;
			}
			if( nPos != nIdxFrom || nKeyLength != nIdxTo - nIdxFrom ){
				return FALSE;
			}
		}

		//検索条件が未定義 または 検索条件の長さより調べるデータが短いときはヒットしない
		if( 0 == nKeyLength || nKeyLength > nDataLen - nPos ){
			return FALSE;
		}
		//英大文字小文字の区別をするかどうか
		if( m_bCurSrchLoHiCase ){	/* 1==英大文字小文字の区別 */
			if( 0 == memcmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return TRUE;
			}
		}else{
			if( 0 == memicmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*! 
	ルーラーのキャレットを再描画	2002.02.25 Add By KK
	@param hdc [in] デバイスコンテキスト
	DispRulerの内容を元に作成
*/
inline void CEditView::DrawRulerCaret( HDC hdc )
{
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	HRGN		hRgn;
	RECT		rc;
	int			nROP_Old;

	if( m_nViewLeftCol <= m_nCaretPosX
	 && m_nViewLeftCol + m_nViewColNum + 2 >= m_nCaretPosX
	){
		if (m_nOldCaretPosX == m_nCaretPosX && m_nCaretWidth == m_nOldCaretWidth) {
			//前描画した位置画同じ かつ ルーラーのキャレット幅が同じ 
			return;
		}

		rc.left = m_nViewAlignLeft + ( m_nOldCaretPosX - m_nViewLeftCol ) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) + 1;
		rc.right = rc.left + m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace - 1;
		rc.top = 0;
		rc.bottom = m_nViewAlignTop - m_nTopYohaku - 1;

		//元位置をクリア m_nOldCaretWidth
		if( 0 == m_nOldCaretWidth ){
			hBrush = ::CreateSolidBrush( RGB( 128, 128, 128 ) );
		}else{
			hBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
		}
		nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
		hRgn = ::CreateRectRgnIndirect( &rc );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
		::PaintRgn( hdc, hRgn );
		::DeleteObject( hRgn );
		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );

		if( 0 == m_nCaretWidth ){
			hBrush = ::CreateSolidBrush( RGB( 128, 128, 128 ) );
		}else{
			hBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
		}

		//新しい位置で描画
		rc.left = m_nViewAlignLeft + ( m_nCaretPosX - m_nViewLeftCol ) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) + 1;
		rc.right = rc.left + m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace - 1;

		hRgn = ::CreateRectRgnIndirect( &rc );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
		::SelectObject( hdc, hBrush );
		::PaintRgn( hdc, hRgn );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hRgn );
		::DeleteObject( hBrush );
		::SetROP2( hdc, nROP_Old );
	}
}



/*! ルーラー描画

	@date 2005.08.14 genta 折り返し幅をLayoutMgrから取得するように
*/
void CEditView::DispRuler( HDC hdc )
{

#ifdef _DEBUG
//	if( 0 != m_pShareData->m_Common.m_nRulerType ){	/* ルーラーのタイプ */
//		DispRulerEx( hdc );
//		return;
//	}
#endif


	if( !m_bDrawSWITCH ){
		return;
	}
	if( !m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		return;
	}

	// 2002.02.25 Add By KK ルーラー全体を描き直す必要がない場合は、ルーラ上のキャレットのみ描きなおす 
	if ( !m_bRedrawRuler ) {
		DrawRulerCaret( hdc );
	}
	else {
		/* 描画処理 */
		HBRUSH		hBrush;
		HBRUSH		hBrushOld;
		HRGN		hRgn;
		RECT		rc;
		int			i;
		int			nX;
		int			nY;
		LOGFONT		lf;
		HFONT		hFont;
 		HFONT		hFontOld;
		char		szColm[32];

		HPEN		hPen;
		HPEN		hPenOld;
		int			nROP_Old;
		COLORREF	colTextOld;
		int			nToX;
		const Types&	typeData = m_pcEditDoc->GetDocumentAttribute();
		const int	nMaxLineSize = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();

		/* LOGFONTの初期化 */
		memset( &lf, 0, sizeof(LOGFONT) );
		lf.lfHeight			= 1 - m_pShareData->m_Common.m_nRulerHeight;	//	2002/05/13 ai
		lf.lfWidth			= 5/*0*/;
		lf.lfEscapement		= 0;
		lf.lfOrientation	= 0;
		lf.lfWeight			= 400;
		lf.lfItalic			= 0;
		lf.lfUnderline		= 0;
		lf.lfStrikeOut		= 0;
		lf.lfCharSet		= 0;
		lf.lfOutPrecision	= 3;
		lf.lfClipPrecision	= 2;
		lf.lfQuality		= 1;
		lf.lfPitchAndFamily	= 34;
		strcpy( lf.lfFaceName, "Arial" );
		hFont = ::CreateFontIndirect( &lf );
		hFontOld = (HFONT)::SelectObject( hdc, hFont );
		::SetBkMode( hdc, TRANSPARENT );

		hBrush = ::CreateSolidBrush( typeData.m_ColorInfoArr[COLORIDX_RULER].m_colBACK );
		rc.left = 0;
		rc.top = 0;
		rc.right = m_nViewAlignLeft + m_nViewCx;
		rc.bottom = m_nViewAlignTop - m_nTopYohaku;
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );

		nX = m_nViewAlignLeft;
		nY = m_nViewAlignTop - m_nTopYohaku - 2;

		hPen = ::CreatePen( PS_SOLID, 0, typeData.m_ColorInfoArr[COLORIDX_RULER].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		colTextOld = ::SetTextColor( hdc, typeData.m_ColorInfoArr[COLORIDX_RULER].m_colTEXT );


		//nToX = m_nViewAlignLeft + m_nViewCx;
		//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
		//	2005.11.10 Moca 1dot足りない
		nToX = m_nViewAlignLeft + (nMaxLineSize - m_nViewLeftCol) * ( m_nCharWidth + typeData.m_nColmSpace ) + 1;
		if( nToX > m_nViewAlignLeft + m_nViewCx ){
			nToX = m_nViewAlignLeft + m_nViewCx;
		}
		::MoveToEx( hdc, m_nViewAlignLeft, nY + 1, NULL );
		::LineTo( hdc, nToX/*m_nViewAlignLeft + m_nViewCx*/, nY + 1 );


		for( i = m_nViewLeftCol;
			i <= m_nViewLeftCol + m_nViewColNum + 1
		 && i <= nMaxLineSize;
			i++
		){
			if( i == nMaxLineSize ){
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, 0/*nY - 8*/ );
			}
			if( 0 == ( (i) % 10 ) ){
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, 0/*nY - 8*/ );
				itoa( (i) / 10, szColm, 10 );
				::TextOut( hdc, nX + 2 + 0, -1 + 0, szColm, lstrlen( szColm ) );
			}else
			if( 0 == ( (i) % 5 ) ){
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, nY - 6 );
			}else{
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, nY - 3 );
			}
			nX += ( m_nCharWidth + typeData.m_nColmSpace );
		}
		::SetTextColor( hdc, colTextOld );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );

		/* キャレット描画（現在の位置に描画するだけ。古い位置はすでに消されている） */
		if( m_nViewLeftCol <= m_nCaretPosX
		 && m_nViewLeftCol + m_nViewColNum + 2 >= m_nCaretPosX
		){
			//	Aug. 18, 2000 あお
			rc.left = m_nViewAlignLeft + ( m_nCaretPosX - m_nViewLeftCol ) * ( m_nCharWidth + typeData.m_nColmSpace ) + 1;
			rc.right = rc.left + m_nCharWidth + typeData.m_nColmSpace - 1;
			rc.top = 0;
			rc.bottom = m_nViewAlignTop - m_nTopYohaku - 1;

			if( 0 == m_nCaretWidth ){
				hBrush = ::CreateSolidBrush( RGB( 128, 128, 128 ) );
			}else{
				hBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
			}
			nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
			hRgn = ::CreateRectRgnIndirect( &rc );
			hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
			::PaintRgn( hdc, hRgn );

			::SelectObject( hdc, hBrushOld );
			::DeleteObject( hRgn );
			::DeleteObject( hBrush );
			::SetROP2( hdc, nROP_Old );
		}

		::SelectObject( hdc, hFontOld );
		::DeleteObject( hFont );

		m_bRedrawRuler = false;	//m_bRedrawRuler = true で指定されるまで、ルーラのキャレットのみを再描画 2002.02.25 Add By KK
	}

	//描画したルーラーのキャレット位置・幅を保存 2002.02.25 Add By KK
	m_nOldCaretPosX = m_nCaretPosX;
	m_nOldCaretWidth = m_nCaretWidth ;

	return;
}

//======================================================================
//@@@ 2001.02.03 Start by MIK: 全角文字の対括弧
//! 全角括弧の対応表
const struct ZENKAKKO_T{
	char *sStr;
	char *eStr;
} zenkakkoarr[] = {
	"【", "】",
	"『", "』",
	"「", "」",
	"＜", "＞",
	"≪", "≫",
	"《", "》",
	"（", "）",
	"〈", "〉",
	"｛", "｝",
	"〔", "〕",
	"［", "］",
	"“", "”",
	"〝", "〟",
	NULL, NULL	//終端識別
};
//@@@ 全角文字の対括弧: End
//@@@ 2003.01.06 Start by ai: 半角文字の対括弧
//! 半角括弧の対応表
const struct HANKAKKO_T{
	char *sStr;
	char *eStr;
} hankakkoarr[] = {
	"(", ")",
	"[", "]",
	"{", "}",
	"<", ">",
	"｢", "｣",
	NULL, NULL	//終端識別
};
//@@@ 半角文字の対括弧: End
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
bool CEditView::SearchBracket( int LayoutX, int LayoutY, int* NewX, int* NewY, int* mode )
{
	int len;	//	行の長さ
	int nCharSize;	//	（メモリ上の）文字幅
	int PosX, PosY;	//	物理位置

	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys( LayoutX, LayoutY, &PosX, &PosY );
	const char *cline = m_pcEditDoc->m_cDocLineMgr.GetLineStr( PosY, &len );

	//	Jun. 19, 2000 genta
	if( cline == NULL )	//	最後の行に本文がない場合
		return false;
//	PosX = LineColmnToIndex( cline, len, PosX );	不要

	// 2005-09-02 D.S.Koba GetSizeOfChar
	nCharSize = CMemory::GetSizeOfChar( cline, len, PosX );
	m_nCharSize = nCharSize;	// 02/09/18 対括弧の文字サイズ設定 ai

	if( nCharSize == 1 ){	//	1バイト文字
		// 03/01/06 ai Start
		int i;
		const struct HANKAKKO_T *p;
		p = hankakkoarr;
		for( i = 0; p->sStr != NULL; i++, p++ )
		{
			if( strncmp(p->sStr, &cline[PosX], 1) == 0 )
			{
				return SearchBracketForward( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );
			}
			else if( strncmp(p->eStr, &cline[PosX], 1) == 0 )
			{
				return SearchBracketBackward( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );
			}
		}
		// 03/01/06 ai End
//@@@ 2001.02.03 Start by MIK: 全角文字の対括弧
	}else if( nCharSize == 2 ){	// 2バイト文字
		int i;
		const struct ZENKAKKO_T *p;
		p = zenkakkoarr;
		for(i = 0; p->sStr != NULL; i++, p++)
		{
			if(strncmp(p->sStr, &cline[PosX], 2) == 0)
			{
				return SearchBracketForward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );		// modeの追加 02/09/19 ai
			}
			else if(strncmp(p->eStr, &cline[PosX], 2) == 0)
			{
				return SearchBracketBackward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );	// modeの追加 02/09/19 ai
			}
		}
//@@@ 2001.02.03 End: 全角文字の対括弧
	}

	// 02/09/18 ai Start
	if( 0 == ( *mode & 2 ) ){
		/* カーソルの前方を調べない場合 */
		return false;
	}
	*mode |= 4;
	// 02/09/18 ai End

	//	括弧が見つからなかったら，カーソルの直前の文字を調べる

	if( PosX <= 0 ){
//		::MessageBox( NULL, "NO DATA", "Bracket", MB_OK );
		return false;	//	前の文字はない
	}
	const char *bPos = CMemory::MemCharPrev( cline, PosX, cline + PosX );
	nCharSize = cline + PosX - bPos;
	m_nCharSize = nCharSize;	// 02/10/01 対括弧の文字サイズ設定 ai
	if( nCharSize == 1 ){	//	1バイト文字
		// 03/01/06 ai Start
		int i;
		const struct HANKAKKO_T *p;
		PosX = bPos - cline;
		p = hankakkoarr;
		for( i = 0; p->sStr != NULL; i++, p++ )
		{
			if( strncmp(p->sStr, &cline[PosX], 1) == 0 )
			{
				return SearchBracketForward( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );
			}
			else if( strncmp(p->eStr, &cline[PosX], 1) == 0 )
			{
				return SearchBracketBackward( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );
			}
		}
		// 03/01/06 ai End
//@@@ 2001.02.03 Start by MIK: 全角文字の対括弧
	}else if( nCharSize == 2 ){	// 2バイト文字
		int i;
		const struct ZENKAKKO_T *p;
		PosX = bPos - cline;
		p = zenkakkoarr;
		for( i = 0; p->sStr != NULL; i++, p++ )
		{
			if( strncmp(p->sStr, &cline[PosX], 2) == 0 )
			{
				return SearchBracketForward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );		// modeの追加 02/09/19 ai
			}
			else if( strncmp(p->eStr, &cline[PosX], 2) == 0 )
			{
				return SearchBracketBackward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );	// modeの追加 02/09/19 ai
			}
		}
//@@@ 2001.02.03 End: 全角文字の対括弧
	}
	return false;
}

/*!
	@brief 半角対括弧の検索:順方向

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
bool CEditView::SearchBracketForward( int PosX, int PosY, int* NewX, int* NewY,
									char* upChar, char* dnChar, int* mode )	// 03/01/08 ai
{
	CDocLine* ci;

	int			len;
	const char	*cPos, *nPos;
	char		*cline, *lineend;
	int			level = 0;
	int			nCol, nLine, nSearchNum;	// 02/09/19 ai

//	char buf[50];	Debug用

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, &nCol, &nLine );	// 02/09/19 ai
	nSearchNum = ( m_nViewTopLine + m_nViewRowNum ) - nLine;					// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos < lineend ){
			nPos = CMemory::MemCharNext( cline, len, cPos );
			if( nPos - cPos > 1 ){
				//	skip
				cPos = nPos;
				continue;
			}
			// 03/01/08 ai Start
			if( strncmp(upChar, cPos, 1) == 0 ){
				++level;
			}
			else if( strncmp(dnChar, cPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	見つかった！
				PosX = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
//				wsprintf( buf, "Layout: %d, %d\nPhys: %d, %d", *NewX, *NewY, PosX, PosY );
//				::MessageBox( NULL, buf, "Bracket", MB_OK );
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
		++PosY;
		ci = ci->m_pNext;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_pLine->GetPtr( &len );
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
bool CEditView::SearchBracketBackward( int PosX, int PosY, int* NewX, int* NewY,
									char* dnChar, char* upChar, int* mode )
{
	CDocLine* ci;

	int			len;
	const char	*cPos, *pPos;
	char		*cline, *lineend;
	int			level = 1;
	int			nCol, nLine, nSearchNum;	// 02/09/19 ai

//	char buf[50];	Debug用

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, &nCol, &nLine );	// 02/09/19 ai
	nSearchNum = nLine - m_nViewTopLine;										// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos > cline ){
			pPos = CMemory::MemCharPrev( cline, len, cPos );
			if( cPos - pPos > 1 ){
				//	skip
				cPos = pPos;
				continue;
			}
			// 03/01/08 ai Start
			if( strncmp(upChar, pPos, 1) == 0 ){
				++level;
			}
			else if( strncmp(dnChar, pPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	見つかった！
				PosX = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
//				wsprintf( buf, "Layout: %d, %d\nPhys: %d, %d", *NewX, *NewY, PosX, PosY );
//				::MessageBox( NULL, buf, "Bracket", MB_OK );
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
		--PosY;
		ci = ci->m_pPrev;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_pLine->GetPtr( &len );
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
bool CEditView::SearchBracketForward2(  int		PosX,	int		PosY,
										int*	NewX,	int*	NewY,
										char*	upChar,	char*	dnChar,
										int*	mode )
{
	CDocLine* ci;

	int len;
	const char *cPos, *nPos;
	char *cline, *lineend;
	int level = 0;
	int			nCol, nLine, nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, &nCol, &nLine );	// 02/09/19 ai
	nSearchNum = ( m_nViewTopLine + m_nViewRowNum ) - nLine;					// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos < lineend ){
			nPos = CMemory::MemCharNext( cline, len, cPos );
			if( nPos - cPos != 2 ){
				//	skip
				cPos = nPos;
				continue;
			}
			if( strncmp(upChar, cPos, 2) == 0 ){
				++level;
			}
			else if( strncmp(dnChar, cPos, 2) == 0 ){
				--level;
			}

			if( level == 0 ){	//	見つかった！
				PosX = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
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
		++PosY;
		ci = ci->m_pNext;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_pLine->GetPtr( &len );
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
bool CEditView::SearchBracketBackward2( int   PosX,   int   PosY,
									    int*  NewX,   int*  NewY,
									    char* dnChar, char* upChar,
										int*  mode )
{
	CDocLine* ci;

	int len;
	const char *cPos, *pPos;
	char *cline, *lineend;
	int level = 1;
	int	nCol, nLine, nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, &nCol, &nLine );	// 02/09/19 ai
	nSearchNum = nLine - m_nViewTopLine;										// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos > cline ){
			pPos = CMemory::MemCharPrev( cline, len, cPos );
			if( cPos - pPos != 2 ){
				//	skip
				cPos = pPos;
				continue;
			}
			if( strncmp(upChar, pPos, 2) == 0 ){
				++level;
			}
			else if( strncmp(dnChar, pPos, 2) == 0 ){
				--level;
			}

			if( level == 0 ){	//	見つかった！
				PosX = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
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
		--PosY;
		ci = ci->m_pPrev;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_pLine->GetPtr( &len );
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
bool CEditView::IsBracket( const char *pLine, int x, int size )
{
	int	i;
	if( size == 1 ){
		const struct HANKAKKO_T *p;
		p = hankakkoarr;
		for( i = 0; p->sStr != NULL; i++, p++ )
		{
			if( strncmp( p->sStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
			else if( strncmp( p->eStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
		}
	}
	else if( size == 2 ) {
		const struct ZENKAKKO_T *p;
		p = zenkakkoarr;
		for( i = 0; p->sStr != NULL; i++, p++ )
		{
			if( strncmp( p->sStr, &pLine[x], 2 ) == 0 )
			{
				return true;
			}
			else if( strncmp( p->eStr, &pLine[x], 2 ) == 0 )
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
	int PosX, PosY;	//	物理位置（改行単位の計算）

	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys( m_nCaretPosX, m_nCaretPosY, &PosX, &PosY );

	CMarkMgr::CMark m( PosX, PosY );
	m_cHistory->Add( m );

}


//	2001/06/18 Start by asa-o: 補完ウィンドウ用のキーワードヘルプ表示
bool  CEditView::ShowKeywordHelp( POINT po, LPCTSTR pszHelp, LPRECT prcHokanWin)
{
	CMemory		cmemCurText;
	CMemory*	pcmemRefText;
	LPSTR		pszWork;
	RECT		rcTipWin,
				rcDesktop;

	if( m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp ){ /* キーワードヘルプを使用する */
		if( m_bInMenuLoop == FALSE	&&	/* メニュー モーダル ループに入っていない */
			0 != m_dwTipTimer			/* 辞書Tipを表示していない */
		){
			cmemCurText.SetDataSz( pszHelp );

			/* 既に検索済みか */
			if( CMemory::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) ){
				/* 該当するキーがなかった */
				if( !m_cTipWnd.m_KeyWasHit ){
					return false;
				}
			}else{
				m_cTipWnd.m_cKey = cmemCurText;
				/* 検索実行 */
				if( m_cDicMgr.Search( cmemCurText.GetPtr(), &pcmemRefText, m_pcEditDoc->GetDocumentAttribute().m_szKeyWordHelpFile ) ){
					/* 該当するキーがある */
					m_cTipWnd.m_KeyWasHit = TRUE;
					pszWork = pcmemRefText->GetPtr();
//								m_cTipWnd.m_cInfo.SetData( pszWork, lstrlen( pszWork ) );
					m_cTipWnd.m_cInfo.SetDataSz( pszWork );
					delete pcmemRefText;
				}else{
					/* 該当するキーがなかった */
					m_cTipWnd.m_KeyWasHit = FALSE;
					return false;
				}
			}
			m_dwTipTimer = 0;	/* 辞書Tipを表示している */

		// 2001/06/19 Start by asa-o: 辞書Tipの表示位置調整
			// 辞書Tipのサイズを取得
			m_cTipWnd.GetWindowSize(&rcTipWin);

			//	May 01, 2004 genta マルチモニタ対応
			::GetMonitorWorkRect( m_cTipWnd.m_hWnd, &rcDesktop );

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

	x1 = m_nCharWidth / 3;
	y1 = m_nCharHeight / 5;

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

		x += m_nCharWidth;
	}

	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );

	return nLength;
}
#endif


/*[EOF]*/
