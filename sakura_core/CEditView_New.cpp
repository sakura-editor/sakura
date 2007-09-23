/*!	@file
	@brief CEditViewクラス

	@author Norio Nakatani
	@date	1998/12/08 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK
	Copyright (C) 2001, hor, YAZAKI
	Copyright (C) 2002, MIK, Moca, genta, hor, novice, YAZAKI, aroka, KK
	Copyright (C) 2003, MIK, ryoji, かろと
	Copyright (C) 2004, genta, Moca, MIK
	Copyright (C) 2005, genta, Moca, MIK, D.S.Koba
	Copyright (C) 2007, ryoji

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
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
#include "etc_uty.h"
#include "CRegexKeyword.h"	//@@@ 2001.11.17 add MIK
#include "my_icmp.h"	//@@@ 2002.01.13 add
#include "Clayout.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/2/10 aroka

/*! フォントを選ぶ
	@param bFat TRUEで太字
	@param bUnderLine TRUEでアンダーライン
*/
HFONT CEditView::ChooseFontHandle( BOOL bFat, BOOL bUnderLine )
{
	if( bFat ){	/* 太字か */
		if( bUnderLine ){	/* 下線か */
			return m_hFont_HAN_FAT_UL;
		}else{
			return m_hFont_HAN_FAT;
		}
	}else{
		if( bUnderLine ){	/* 下線か */
			return m_hFont_HAN_UL;
		}else{
			return m_hFont_HAN;
		}
	}
}


/* 通常の描画処理 new */
void CEditView::OnPaint( HDC hdc, PAINTSTRUCT *pPs, BOOL bUseMemoryDC )
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::OnPaint" );

	// 2004.01.28 Moca デスクトップに作画しないように
	if( NULL == hdc ){
		return;
	}
//	if( m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
//		return;
//	}
	if( !m_bDrawSWITCH ){
		return;
	}
	/* キャレットを隠す */
	HideCaret_( m_hWnd ); // 2002/07/22 novice

	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	const int nWrapWidth = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();

	int				i;
	HFONT			hFontOld;
	HBRUSH			hBrush;
	COLORREF		crBackOld;
	COLORREF		crTextOld;
	HDC				hdcOld;
	int				nTop;
	RECT			rc;
	RECT			rcBack;
	BOOL			bEOF;
	int				nLineHeight = m_nCharHeight + TypeDataPtr->m_nLineSpace;
	int				nCharWidth = m_nCharWidth + TypeDataPtr->m_nColmSpace;
	int				nLineTo;
	int				nX = m_nViewAlignLeft - m_nViewLeftCol * nCharWidth;
	int				nY;
	BOOL			bDispBkBitmap = /*TRUE*/FALSE;
	const CLayout*	pcLayout;
	HPEN			hPen;
	HPEN			hPenOld;

//@@@ 2001.11.17 add start MIK
	//変更があればタイプ設定を行う。
	if( TypeDataPtr->m_bUseRegexKeyword
	 || m_cRegexKeyword->m_bUseRegexKeyword )	//OFFなのに前回のデータが残ってる
	{
		//タイプ別設定をする。設定済みかどうかは呼び先でチェックする。
		m_cRegexKeyword->RegexKeySetTypes(TypeDataPtr);
	}
//@@@ 2001.11.17 add end MIK

	/* キャレットの行桁位置を表示する */
//	DrawCaretPosInfo();

	/* メモリＤＣを利用した再描画の場合は描画先のＤＣを切り替える */
	bUseMemoryDC = FALSE;
	if( bUseMemoryDC ){
		hdcOld = hdc;
		hdc = m_hdcCompatDC;
	}

	/* 03/02/18 対括弧の強調表示(消去) ai */
	DrawBracketPair( false );

	/* ルーラーとテキストの間の余白 */
	//@@@ 2002.01.03 YAZAKI 余白が0のときは無駄でした。
	if ( m_nTopYohaku ){
		rc.left = 0;
		rc.top = m_nViewAlignTop - m_nTopYohaku;
		rc.right = m_nViewAlignLeft + m_nViewCx;
		rc.bottom = m_nViewAlignTop;
		hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}
	
	/* 行番号の表示 */
	//	From Here Sep. 7, 2001 genta
	//	Sep. 23, 2002 genta 行番号非表示でも行番号色の帯があるので隙間を埋める
	if( m_nTopYohaku ){ 
		rc.left = 0;
		rc.top = m_nViewAlignTop - m_nTopYohaku;
		//	Sep. 23 ,2002 genta 余白はテキスト色のまま残す
		rc.right = m_nViewAlignLeft - m_pShareData->m_Common.m_nLineNumRightSpace;
		rc.bottom = m_nViewAlignTop;
		hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_GYOU].m_colBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}
	//	To Here Sep. 7, 2001 genta

	::SetBkMode( hdc, TRANSPARENT );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
	m_hFontOld = NULL;


//	crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_colorBACK );
//	crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_colorTEXT );
	crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
	crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colTEXT );

//	::FillRect( hdc, &pPs->rcPaint, ::GetStockObject( WHITE_BRUSH ) );


	nTop = pPs->rcPaint.top;
	bEOF = FALSE;
//	i = m_nViewTopLine;

	if( 0 > nTop - m_nViewAlignTop ){
		i = m_nViewTopLine;
	}else{
		i = m_nViewTopLine + ( ( nTop - m_nViewAlignTop ) / nLineHeight );
	}

	int nMaxRollBackLineNum = 260 / nWrapWidth + 1;
	int nRollBackLineNum = 0;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( i );
	while( nRollBackLineNum < nMaxRollBackLineNum ){
		pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( i );
		if( NULL == pcLayout ){
			break;
		}
		if( 0 == pcLayout->m_nOffset ){	/* 対応する論理行の先頭からのオフセット */
			break;
		}
		i--;
		nRollBackLineNum++;
	}

	nY = ( i - m_nViewTopLine ) * nLineHeight + m_nViewAlignTop;
	nLineTo = m_nViewTopLine + ( ( pPs->rcPaint.bottom - m_nViewAlignTop ) / nLineHeight );

	BOOL bSelected;
	bSelected = IsTextSelected();
	for( ; i <= nLineTo; ){
		pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( i );
#ifdef _DEBUG
		{
			if( NULL != pcLayout ){
				if( (void*)0xdddddddd == (void*)pcLayout->m_pCDocLine->m_pLine ){
					m_pcEditDoc->m_cDocLineMgr.DUMP();
					m_pcEditDoc->m_cLayoutMgr.DUMP();

					pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( i );
				}

				int nLineLen = pcLayout->m_pCDocLine->m_pLine->GetLength() - pcLayout->m_nOffset;
				const char * pLine = (const char *)pcLayout->m_pCDocLine->m_pLine->GetPtr() + pcLayout->m_nOffset;
			}
		}
#endif

		if( DispLineNew(
			hdc,
			pcLayout,
			i,
			nX,
			nY,
			bDispBkBitmap,
			nLineTo,
			bSelected
		) ){
			pPs->rcPaint.bottom += nLineHeight;	/* EOF再描画対応 */
			bEOF = TRUE;
			break;
		}
		if( NULL == pcLayout ){
			bEOF = TRUE;
			break;
		}
	}
	if( NULL != m_hFontOld ){
		::SelectObject( hdc, m_hFontOld );
		m_hFontOld = NULL;
	}

//	if( bEOF ){
//		nTop = ( i + 1 - m_nViewTopLine ) * nLineHeight + m_nViewAlignTop;
//	}else{
//		nTop = ( i	   - m_nViewTopLine ) * nLineHeight + m_nViewAlignTop;
//	}
	if( bDispBkBitmap ){
	}else{
		/* テキストのない部分を背景色で塗りつぶす */
		if( nY/*nTop*/ < pPs->rcPaint.bottom ){
			rcBack.left = pPs->rcPaint.left;
			rcBack.right = pPs->rcPaint.right;
			rcBack.top = nY/*nTop*/;
			rcBack.bottom = pPs->rcPaint.bottom;
//#ifdef _DEBUG
//			hBrush = ::CreateSolidBrush( RGB( 128, 128,  128 ) );
//#else
			hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
//#endif
			::FillRect( hdc, &rcBack, hBrush );
			::DeleteObject( hBrush );

			// 2006.04.29 行部分は行ごとに作画し、ここでは縦線の残りを作画
			DispVerticalLines( hdc, nY, pPs->rcPaint.bottom, 0, -1 );
		}
	}

	::SetTextColor( hdc, crTextOld );
	::SetBkColor( hdc, crBackOld );
	::SelectObject( hdc, hFontOld );

	/* 折り返し位置の表示 */
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_bDisp ){
		int nXPos = m_nViewAlignLeft + ( nWrapWidth - m_nViewLeftCol ) * nCharWidth;
		//	2005.11.08 Moca 作画条件変更
		if( m_nViewAlignLeft < nXPos && nXPos < m_nViewCx + m_nViewAlignLeft ){
			/* 折り返し記号の色のペンを作成 */
			hPen = ::CreatePen( PS_SOLID, 0, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_colTEXT );
			hPenOld = (HPEN)::SelectObject( hdc, hPen );
			::MoveToEx( hdc, nXPos, m_nViewAlignTop, NULL );
			::LineTo( hdc, nXPos, m_nViewAlignTop + m_nViewCy );
			::SelectObject( hdc, hPenOld );
			::DeleteObject( hPen );
		}
	}


	/* ルーラー描画 */
	if ( pPs->rcPaint.top < m_nViewAlignTop - m_nTopYohaku ) { // ルーラーが再描画範囲にあるときのみ再描画する 2002.02.25 Add By KK
		m_bRedrawRuler = true; //2002.02.25 Add By KK ルーラー全体を描画。
		DispRuler( hdc );
	}

	if ( m_pcEditDoc->m_nActivePaneIndex == m_nMyIndex ){
		/* アクティブペインは、アンダーライン描画 */
		m_cUnderLine.CaretUnderLineON( TRUE );
	}

	/* メモリＤＣを利用した再描画の場合はメモリＤＣに描画した内容を画面へコピーする */
	if( bUseMemoryDC ){
		::BitBlt(
			hdcOld,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			hdc,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
	}

	/* 03/02/18 対括弧の強調表示(描画) ai */
	DrawBracketPair( true );

	/* キャレットを現在位置に表示します */
	ShowCaret_( m_hWnd ); // 2002/07/22 novice
	return;
}









//@@@ 2001.02.17 Start by MIK
/*! 行のテキスト／選択状態の描画
	1回で1論理行分を作画する。
	@param          hdc            作画対象
	@param          pcLayout       表示を開始するレイアウト
	@param[in,out]  nLineNum       作画するレイアウト行番号(0開始), 次の物理行に対応するレイアウト行番号
	@param          x              レイアウト0桁目の作画座標x
	@param[in,out]  y              作画座標y, 次の作画座標y
	@param          bDispBkBitmap  背景にビットマップを使用する(事実上は常にFALSE。TRUEの場合背景作画しない)
	@param          nLineTo        作画終了するレイアウト行番号
	@param          bSelected      選択中か
	@return EOFを作画したらtrue

	@par nCOMMENTMODE
	関数内部で状態遷移のために使われる変数nCOMMENTMODEと状態の関係。

// 2002/03/13 novice
 - COLORIDX_TEXT     : テキスト
 - COLORIDX_COMMENT  : 行コメント
 - COLORIDX_BLOCK1   : ブロックコメント1
 - COLORIDX_SSTRING  : シングルコーテーション
 - COLORIDX_WSTRING  : ダブルコーテーション
 - COLORIDX_KEYWORD1 : 強調キーワード1
 - COLORIDX_CTRLCODE : コントロールコード
 - COLORIDX_DIGIT    : 半角数値
 - COLORIDX_BLOCK2   : ブロックコメント2
 - COLORIDX_KEYWORD2 : 強調キーワード2
 - COLORIDX_URL      : URL
 - COLORIDX_SEARCH   : 検索
 - 1000: 正規表現キーワード
 	色指定SetCurrentColorを呼ぶときにCOLORIDX_*値を加算するので、
 	1000～COLORIDX_LASTまでは正規表現で使用する。

	@date 2001.12.21 YAZAKI 改行記号の描きかたを変更
 */
//@@@ 2001.02.17 End by MIK
int CEditView::DispLineNew(
		HDC						hdc,
		const CLayout*			pcLayout,
		int&					nLineNum,
		int						x,
		int&					y,
		BOOL					bDispBkBitmap,
		int						nLineTo,
		BOOL					bSelected
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DispLineNew" );

	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	const int nWrapWidth = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();

	int						nLineNumOrg = nLineNum;
	const char*				pLine;	//@@@ 2002.09.22 YAZAKI
	int						nLineLen;
	int						nX;
	int						nLineBgn;
	int						nBgn;
	int						nPos;
	int						nCharChars;
	int						nCharChars_2;
	int						nCOMMENTMODE;
	int						nCOMMENTMODE_OLD;
	int						nCOMMENTEND;
	int						nCOMMENTEND_OLD;
	RECT					rcClip;
	RECT					rcClip2;
	int						nLineHeight = m_nCharHeight + TypeDataPtr->m_nLineSpace;
	int						nCharWidth = m_nCharWidth + TypeDataPtr->m_nColmSpace;
	//	テキスト描画モード
	UINT					fuOptions = ETO_CLIPPED | ETO_OPAQUE;
//@@@ 2001.12.21 YAZAKI
//	HPEN					hPen;
//	HPEN					hPenOld;
	HBRUSH					hBrush;
	COLORREF				colTextColorOld;
	COLORREF				colBkColorOld;
//#ifndef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
// //	static char*			pszTAB = ">       ";
// //	static char*			pszTAB = ">･･･････";
//	static char*			pszTAB = "^       ";
//#endif
	static char*			pszSPACES = "        ";
	static char*			pszZENSPACE	= "□";
	static char*			pszWRAP	= "<";
	int						nPosX;
	int						nPosY;
	int						bEOF = FALSE;
	int						nCount;
	int						nLineCols;
	const CLayout*			pcLayout2;
	int						i, j;
	int						nIdx;
	int						nUrlLen;
	BOOL					bSearchStringMode;
	BOOL					bSearchFlg;			// 2002.02.08 hor
	int						nSearchStart;		// 2002.02.08 hor
	int						nSearchEnd;
	int						nColorIdx;
	bool					bKeyWordTop = true;	//	Keyword Top
//	const CDocLine*			pCDocLine;

//@@@ 2001.11.17 add start MIK
	int		nMatchLen;
	int		nMatchColor;
//@@@ 2001.11.17 add end MIK

	bSearchStringMode = FALSE;
	bSearchFlg	= TRUE;	// 2002.02.08 hor
	nSearchStart= -1;	// 2002.02.08 hor
	nSearchEnd	= -1;	// 2002.02.08 hor

	/* 論理行データの取得 */
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory変更
		nLineLen = pcLayout->m_pCDocLine->m_pLine->GetLength() - pcLayout->m_nOffset;
		pLine = pcLayout->m_pCDocLine->m_pLine->GetPtr() + pcLayout->m_nOffset;

		nCOMMENTMODE = pcLayout->m_nTypePrev;	/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
		nCOMMENTEND = 0;
		pcLayout2 = pcLayout;

	}else{
		pLine = NULL;
		nLineLen = 0;
		nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		nCOMMENTEND = 0;

		pcLayout2 = NULL;
	}

	/* 現在の色を指定 */
//	SetCurrentColor( hdc, 0 );
	SetCurrentColor( hdc, nCOMMENTMODE );

	nBgn = 0;
	nPos = 0;
	nLineBgn = 0;
	nX = 0;
	nCharChars = 0;
	// my_icmpの利用によりlocaleに依存しない 2002/11/30 Moca
	// setlocale ( LC_ALL, "C" );	//	Oct. 29, 2001 genta 検索文字列のハイライトに関係する

	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		y -= nLineHeight;
		nLineNum--;
//		MYTRACE( "\n\n=======================================" );
		while( nPos < nLineLen ){
//			MYTRACE( "nLineNum = %d\n", nLineNum );

			y += nLineHeight;
			nLineNum++;
			if( m_nViewTopLine + m_nViewRowNum < nLineNum ){
				nLineNum = nLineTo + 1;
				goto end_of_func;
			}
			if( nLineTo < nLineNum ){
				goto end_of_func;
			}
			pcLayout2 = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum );
			if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
				/* 行番号表示 */
				DispLineNumber( hdc, pcLayout2, nLineNum, y );
			}
			nBgn = nPos;
			nLineBgn = nBgn;
			nX = 0;

			//	行頭背景
			if (pcLayout2 && pcLayout2->GetIndent()){
				rcClip.left = x;
				rcClip.right = x + pcLayout2->GetIndent() * ( nCharWidth );
				rcClip.top = y;
				rcClip.bottom = y + nLineHeight;
				if( rcClip.left < m_nViewAlignLeft ){
					rcClip.left = m_nViewAlignLeft;
				}
				if( rcClip.left < rcClip.right &&
					rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
					hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
					::FillRect( hdc, &rcClip, hBrush );
					::DeleteObject( hBrush );
				}
				nX += pcLayout2->GetIndent();
			}

			while( nPos - nLineBgn < pcLayout2->m_nLength ){
				/* 検索文字列の色分け */
				if( TRUE == m_bCurSrchKeyMark	/* 検索文字列のマーク */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
				// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
					if(!bSearchStringMode && (!m_bCurSrchRegularExp || (bSearchFlg && nSearchStart < nPos))){
						bSearchFlg=IsSearchString( pLine, nLineLen, nPos, &nSearchStart, &nSearchEnd );
					}
					if( !bSearchStringMode && bSearchFlg && nSearchStart==nPos
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						bSearchStringMode = TRUE;
//						nCOMMENTMODE = 90;
//						nCOMMENTMODE_OLD_2 = nCOMMENTMODE;
//						nCOMMENTEND_OLD_2 = nCOMMENTEND;
						/* 現在の色を指定 */
//						SetCurrentColor( hdc, nCOMMENTMODE );
						SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
					}else
					if( bSearchStringMode
					 && nSearchEnd <= nPos		//+ == では行頭文字の場合、nSearchEndも０であるために文字色の解除ができないバグを修正 2003.05.03 かろと
					){
						// 検索した文字列の終わりまできたら、文字色を標準に戻す処理
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
//							SetCurrentColor( hdc, 90 );
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						/* 現在の色を指定 */
//						nCOMMENTMODE = nCOMMENTMODE_OLD_2;
//						nCOMMENTEND = nCOMMENTEND_OLD_2;
						SetCurrentColor( hdc, nCOMMENTMODE );
						bSearchStringMode = FALSE;
						goto searchnext;
					}
				}

				if( nPos >= nLineLen - pcLayout2->m_cEol.GetLen() ){
					if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
						/* テキスト表示 */
						nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						nBgn = nPos + 1;
						/* 行末背景描画 */
						rcClip.left = x + nX * ( nCharWidth );
						rcClip.right = m_nViewAlignLeft + m_nViewCx;
						rcClip.top = y;
						rcClip.bottom = y + nLineHeight;
						if( rcClip.left < m_nViewAlignLeft ){
							rcClip.left = m_nViewAlignLeft;
						}
						if( rcClip.left < rcClip.right &&
							rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
							hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
							::FillRect( hdc, &rcClip, hBrush );
							::DeleteObject( hBrush );
						}

						/* 改行記号の表示 */
						rcClip2.left = x + nX * ( nCharWidth );
						// Jul. 20, 2003 ryoji 横スクロール時に改行コードが欠けないように
						rcClip2.right = rcClip2.left + ( nCharWidth ) * ( 2 );
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
							rcClip2.top = y;
							rcClip2.bottom = y + nLineHeight;
							// 2006.04.30 Moca 色選択を括弧内に移動
							if( bSearchStringMode ){
								nColorIdx = COLORIDX_SEARCH;
							}else{
								nColorIdx = COLORIDX_CRLF;
							}
							HFONT	hFontOld;
							/* フォントを選ぶ */
							hFontOld = (HFONT)::SelectObject( hdc,
								ChooseFontHandle(
									TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont,
									TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bUnderLine
								)
							);
							colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT ); /* CRLFの色 */
							colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );	/* CRLF背景の色 */
							//	2003.08.17 ryoji 改行文字が欠けないように
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, (const char *)"  ", 2, m_pnDx );
							/* 改行記号の表示 */
							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_CRLF].m_bDisp ){
								nPosX = x + nX * ( nCharWidth );
								nPosY = y;
								//	From Here 2003.08.17 ryoji 改行文字が欠けないように
								HRGN hRgn;
								hRgn = ::CreateRectRgnIndirect(&rcClip2);
								::SelectClipRgn(hdc, hRgn);
								//@@@ 2001.12.21 YAZAKI
								DrawEOL(hdc, nPosX + 1, nPosY, m_nCharWidth, m_nCharHeight,
									pcLayout2->m_cEol, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont,
										TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT );
								::SelectClipRgn(hdc, NULL);
								::DeleteObject(hRgn);
								//	To Here 2003.08.17 ryoji 改行文字が欠けないように
							}
							::SelectObject( hdc, hFontOld );
							::SetTextColor( hdc, colTextColorOld );
							::SetBkColor( hdc, colBkColorOld );
						}

						nX++;


						// 2006.04.29 Moca 選択処理のため縦線処理を追加
						DispVerticalLines( hdc, y, y + nLineHeight, 0, -1 );
						if( bSelected ){
							/* テキスト反転 */
							DispTextSelected( hdc, nLineNum, x, y, nX );
						}
					}

					goto end_of_line;
				}
				SEARCH_START:;
				switch( nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
//@@@ 2001.11.17 add start MIK
					//正規表現キーワード
					if( TypeDataPtr->m_bUseRegexKeyword
					 && m_cRegexKeyword->RegexIsKeyword( pLine, nPos, nLineLen, &nMatchLen, &nMatchColor )
					 /*&& TypeDataPtr->m_ColorInfoArr[nMatchColor].m_bDisp*/ )
					{
						if( y/* + nLineHeight*/ >= m_nViewAlignTop )
						{
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						/* 現在の色を指定 */
						nBgn = nPos;
						//nCOMMENTMODE = 1000;	/* 色指定 */
						nCOMMENTMODE = 1000 + nMatchColor;	/* 色指定 */	//@@@ 2002.01.04 upd
						nCOMMENTEND = nPos + nMatchLen;  /* キーワード文字列の終端をセットする */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
							//SetCurrentColor( hdc, nCOMMENTMODE + nMatchColor );	//@@@ 2002.01.04
						}
					}
					else
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;

						nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice

//						if( TypeDataPtr->m_bDispCOMMENT ){	/* コメントを表示する */
							/* 現在の色を指定 */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
					}else
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice

//						if( TypeDataPtr->m_bDispCOMMENT ){	/* コメントを表示する */
							/* 現在の色を指定 */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos + (int)lstrlen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI

//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
					}else
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
//						if( TypeDataPtr->m_bDispCOMMENT ){	/* コメントを表示する */
							/* 現在の色を指定 */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos + (int)lstrlen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
//#endif
					}else
					if( pLine[nPos] == '\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* シングルクォーテーション文字列を表示する */
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice

//						if( TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp ){	/* シングルクォーテーション文字列を表示する */
							/* 現在の色を指定 */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( pLine[nPos] == '"' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* ダブルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URLを表示する */
					 && ( TRUE == IsURL( &pLine[nPos], nLineLen - nPos, &nUrlLen ) )	/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_URL;	/* URLモード */ // 2002/03/13 novice
						nCOMMENTEND = nPos + nUrlLen;
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//@@@ 2001.02.17 Start by MIK: 半角数値を強調表示
//#ifdef COMPILE_COLOR_DIGIT
					}else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (i = IsNumber( pLine, nPos, nLineLen )) > 0 )		/* 半角数字を表示する */
					{
						/* キーワード文字列の終端をセットする */
						i = nPos + i;
						if( y/* + nLineHeight*/ >= m_nViewAlignTop )
						{
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						/* 現在の色を指定 */
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_DIGIT;	/* 半角数値である */ // 2002/03/13 novice
						nCOMMENTEND = i;
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//#endif
//@@@ 2001.02.17 End by MIK: 半角数値を強調表示
					}else
					if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* キーワードセット */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* 強調キーワードを表示する */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						//	Mar 4, 2001 genta comment out
						//	bKeyWordTop = false;
						/* キーワード文字列の終端を探す */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							if( IS_KEYWORD_CHAR( pLine[i] ) ){
							}else{
								break;
							}
						}
						/* キーワードが登録単語ならば、色を変える */
						j = i - nPos;
						/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
						nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
							TypeDataPtr->m_nKeyWordSetIdx[0],
							&pLine[nPos],
							j
						);
						if( nIdx != -1 ){
							if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
								/* テキスト表示 */
								nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
							}

							/* 現在の色を指定 */
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_KEYWORD1;	/* 強調キーワード1 */ // 2002/03/13 novice
							nCOMMENTEND = i;
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
						}else{		//MIK START ADD 2000.12.01 second keyword & binary search
							// 2005.01.13 MIK 強調キーワード数追加に伴う配列化
							for( int my_i = 1; my_i < 10; my_i++ )
							{
								if(TypeDataPtr->m_nKeyWordSetIdx[ my_i ] != -1 && /* キーワードセット */							//MIK 2000.12.01 second keyword
									TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + my_i].m_bDisp)									//MIK
								{																							//MIK
									/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */						//MIK
									nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
										TypeDataPtr->m_nKeyWordSetIdx[ my_i ] ,													//MIK
										&pLine[nPos],																		//MIK
										j																					//MIK
									);																						//MIK
									if( nIdx != -1 ){																		//MIK
										if( y/* + nLineHeight*/ >= m_nViewAlignTop ){										//MIK
											/* テキスト表示 */																//MIK
											nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );	//MIK
										}																					//MIK
										/* 現在の色を指定 */																//MIK
										nBgn = nPos;																		//MIK
										nCOMMENTMODE = COLORIDX_KEYWORD1 + my_i;	/* 強調キーワード2 */ // 2002/03/13 novice		//MIK
										nCOMMENTEND = i;																	//MIK
										if( !bSearchStringMode ){															//MIK
											SetCurrentColor( hdc, nCOMMENTMODE );											//MIK
										}																					//MIK
										break;
									}																						//MIK
								}																							//MIK
								else
								{
									if(TypeDataPtr->m_nKeyWordSetIdx[my_i] == -1 )
										break;
								}
							}
						}			//MIK END
					}
					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pLine[nPos] ))	bKeyWordTop = false;
					else								bKeyWordTop = true;
					//	To Here
					break;
// 2002/03/13 novice
				case COLORIDX_URL:		/* URLモードである */
				case COLORIDX_KEYWORD1:	/* 強調キーワード1 */
				case COLORIDX_DIGIT:	/* 半角数値である */  //@@@ 2001.02.17 by MIK
				case COLORIDX_KEYWORD2:	/* 強調キーワード2 */	//MIK
				case COLORIDX_KEYWORD3:	// 2005.01.13 MIK 強調キーワード3-10
				case COLORIDX_KEYWORD4:
				case COLORIDX_KEYWORD5:
				case COLORIDX_KEYWORD6:
				case COLORIDX_KEYWORD7:
				case COLORIDX_KEYWORD8:
				case COLORIDX_KEYWORD9:
				case COLORIDX_KEYWORD10:
				//case 1000:	//正規表現キーワード1～10	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04 del
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* コントロールコード */ // 2002/03/13 novice
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = nCOMMENTMODE_OLD;
						nCOMMENTEND = nCOMMENTEND_OLD;
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;

				case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
//#ifdef	COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
				case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
//#endif
				case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ダブルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 ){	//正規表現キーワード1～10
						if( nPos == nCOMMENTEND ){
							if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
								/* テキスト表示 */
								nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
							}
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* 現在の色を指定 */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}
				if( pLine[nPos] == TAB ){
					if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
						/* テキスト表示 */
						nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						//	Sep. 22, 2002 genta 共通式のくくりだし
						//	Sep. 23, 2002 genta LayoutMgrの値を使う
						int tabDispWidth = m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace( nX );
						/* タブ記号を表示する */
						rcClip2.left = x + nX * ( nCharWidth );
						rcClip2.right = rcClip2.left + ( nCharWidth ) * tabDispWidth;
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
							rcClip2.top = y;
							rcClip2.bottom = y + nLineHeight;
							/* TABを表示するか？ */
							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_bDisp
							 && 0 == TypeDataPtr->m_bTabArrow ){	//タブ通常表示	//@@@ 2003.03.26 MIK
								if( bSearchStringMode ){
									nColorIdx = COLORIDX_SEARCH;
								}else{
									nColorIdx = COLORIDX_TAB;
								}
								colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT );	/* TAB文字の色 */
								colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );		/* TAB文字背景の色 */

								HFONT	hFontOld;
								/* フォントを選ぶ */
								hFontOld = (HFONT)::SelectObject( hdc,
									ChooseFontHandle(
										TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont,
										TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bUnderLine
									)
								);
								
								//@@@ 2001.03.16 by MIK
								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, /*pszTAB*/ TypeDataPtr->m_szTabViewString,
									tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
									 m_pnDx );
								::SelectObject( hdc, hFontOld );
								::SetTextColor( hdc, colTextColorOld );
								::SetBkColor( hdc, colBkColorOld );
							}else{
								if( bSearchStringMode ){
									nColorIdx = COLORIDX_SEARCH;
									colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );	/* TAB文字背景の色 */
								}
								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, pszSPACES,
									tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
									 m_pnDx );
								if( bSearchStringMode ){
									::SetBkColor( hdc, colBkColorOld );
								}
								
								//タブ矢印表示	//@@@ 2003.03.26 MIK
								if( TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_bDisp
								 && 0 != TypeDataPtr->m_bTabArrow
								 && rcClip2.left <= x + nX * nCharWidth ) // Apr. 1, 2003 MIK 行番号と重なる
								{
									DrawTabArrow( hdc, x + nX * ( nCharWidth ), y, m_nCharWidth, m_nCharHeight,
										TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_bFatFont,
										TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_colTEXT );
								}
							}
						}
						nX += tabDispWidth ;//	Sep. 22, 2002 genta
					}
					nBgn = nPos + 1;
					nCharChars = 1;
				}else
				if( (unsigned char)pLine[nPos] == 0x81 && (unsigned char)pLine[nPos + 1] == 0x40	//@@@ 2001.11.17 upd MIK
				// && nCOMMENTMODE != 1000 )	//@@@ 2002.01.04
				 && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )	//@@@ 2002.01.04
				{	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04
					if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
						/* テキスト表示 */
						nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );

						/* 全角空白を表示する */
						rcClip2.left = x + nX * ( nCharWidth );
						rcClip2.right = rcClip2.left + ( nCharWidth ) * 2;
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
							rcClip2.top = y;
							rcClip2.bottom = y + nLineHeight;

							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_ZENSPACE].m_bDisp ){	/* 日本語空白を表示するか */
								if( bSearchStringMode ){
									nColorIdx = COLORIDX_SEARCH;
								}else{
									nColorIdx = COLORIDX_ZENSPACE;
								}
								colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT );	/* 全角スペース文字の色 */
							colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );		/* 全角スペース文字背景の色 */


								HFONT	hFontOld;
								/* フォントを選ぶ */
								hFontOld = (HFONT)::SelectObject( hdc,
									ChooseFontHandle(
										TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont,
										TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bUnderLine
									)
								);
//								if( TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont ){	/* 太字か */
//									hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//								}else{
//									hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//								}

								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, pszZENSPACE, lstrlen( pszZENSPACE ), m_pnDx );

								::SelectObject( hdc, hFontOld );
								::SetTextColor( hdc, colTextColorOld );
								::SetBkColor( hdc, colBkColorOld );

							}else{
							if( bSearchStringMode ){
								nColorIdx = COLORIDX_SEARCH;
								colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );	/* 文字背景の色 */
							}
								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, pszSPACES, 2, m_pnDx );
							}
							if( bSearchStringMode ){
								::SetBkColor( hdc, colBkColorOld );
							}
						}
						nX += 2;
					}
					nBgn = nPos + 2;
					nCharChars = 2;
				}
				//半角空白（半角スペース）を表示 2002.04.28 Add by KK 
				else if (pLine[nPos] == ' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp 
					 && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{
					nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
					if( y >= m_nViewAlignTop ){
						rcClip2.left = x + nX * ( nCharWidth );
						rcClip2.right = rcClip2.left + ( nCharWidth );
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){

							if( bSearchStringMode ){
								nColorIdx = COLORIDX_SEARCH;
							}else{
								nColorIdx = COLORIDX_SPACE;
							}

							colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT );	/* 半角スペース文字の色 */
							colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );		/* 半角スペース文字背景の色 */
							HFONT	hFontOld = (HFONT)::SelectObject( hdc,
								ChooseFontHandle(
									TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont,
									TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bUnderLine
								)
							);
							
							//小文字"o"の下半分を出力
							rcClip2.top = y + (int)((nLineHeight/2) ) ;
							rcClip2.bottom = y + nLineHeight;
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, "o", 1, m_pnDx );
							
							//上半分は普通の空白で出力（"o"の上半分を消す）
							rcClip2.top = y ;
							rcClip2.bottom = y + (int)((nLineHeight/2) ) ;
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, " ", 1, m_pnDx );

							::SelectObject( hdc, hFontOld );
							::SetTextColor( hdc, colTextColorOld );
							::SetBkColor( hdc, colBkColorOld );
						}
						nX++;
					}
					nBgn = nPos + 1;
					nCharChars = 1;
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* コントロールコードを色分け */
					 &&	(
								//	Jan. 23, 2002 genta 警告抑制
							( (unsigned char)pLine[nPos] <= (unsigned char)0x1F ) ||
							( (unsigned char)'~' < (unsigned char)pLine[nPos] && (unsigned char)pLine[nPos] < (unsigned char)'｡' ) ||
							( (unsigned char)'ﾟ' < (unsigned char)pLine[nPos] )
						)
					 && pLine[nPos] != TAB && pLine[nPos] != CR && pLine[nPos] != LF
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE_OLD = nCOMMENTMODE;
						nCOMMENTEND_OLD = nCOMMENTEND;
						nCOMMENTMODE = COLORIDX_CTRLCODE;	/* コントロールコード モード */ // 2002/03/13 novice
						/* コントロールコード列の終端を探す */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if( (
								//	Jan. 23, 2002 genta 警告抑制
								( (unsigned char)pLine[i] <= (unsigned char)0x1F ) ||
									( (unsigned char)'~' < (unsigned char)pLine[i] && (unsigned char)pLine[i] < (unsigned char)'｡' ) ||
									( (unsigned char)'ﾟ' < (unsigned char)pLine[i] )
								) &&
								pLine[i] != TAB && pLine[i] != CR && pLine[i] != LF
							){
							}else{
								break;
							}
						}
						nCOMMENTEND = i;
						/* 現在の色を指定 */
//						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
//						}
					}
				}
				nPos+= nCharChars;
			} //end of while( nPos - nLineBgn < pcLayout2->m_nLength )
			if( nPos >= nLineLen ){
				break;
			}
			if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
				/* テキスト表示 */
				nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
				nBgn = nPos;

				/* 行末背景描画 */
				rcClip.left = x + nX * ( nCharWidth );
				rcClip.right = m_nViewAlignLeft + m_nViewCx;
				rcClip.top = y;
				rcClip.bottom = y + nLineHeight;
				if( rcClip.left < m_nViewAlignLeft ){
					rcClip.left = m_nViewAlignLeft;
				}
				if( rcClip.left < rcClip.right &&
					rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
					hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
					::FillRect( hdc, &rcClip, hBrush );
					::DeleteObject( hBrush );
				}

				/* 折り返し記号 */
				/* 折り返し記号を表示する */
				rcClip2.left = x + nX * ( nCharWidth );
				rcClip2.right = rcClip2.left + ( nCharWidth ) * 1;
				if( rcClip2.left < m_nViewAlignLeft ){
					rcClip2.left = m_nViewAlignLeft;
				}
				if( rcClip2.left < rcClip2.right &&
					rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
					rcClip2.top = y;
					rcClip2.bottom = y + nLineHeight;

					/* 折り返し記号を表示する */
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_bDisp ){
						HFONT	hFontOld;
						colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_colTEXT );	/* 折り返し記号の色 */
						colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_colBACK );		/* 折り返し記号背景の色 */
						/* フォントを選ぶ */
						hFontOld = (HFONT)::SelectObject( hdc,
							ChooseFontHandle(
								TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_bFatFont,
								TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_bUnderLine
							)
						);
						::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
							&rcClip2, pszWRAP, lstrlen( pszWRAP ), m_pnDx );

						::SelectObject( hdc, hFontOld );
						::SetTextColor( hdc, colTextColorOld );
						::SetBkColor( hdc, colBkColorOld );
					}else{
						::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
							&rcClip2, " ", 1, m_pnDx );
					}
				}
				// 2006.04.29 Moca 選択処理のため縦線処理を追加
				DispVerticalLines( hdc, y, y + nLineHeight,  0, -1 );
				if( bSelected ){
					/* テキスト反転 */
					DispTextSelected( hdc, nLineNum, x, y, nX );
				}
			}
		}
		
		if( y >= m_nViewAlignTop ){
			/* テキスト表示 */
			nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
			/* EOF記号の表示 */
			if( nLineNum + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
				nX < nWrapWidth
			){
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					//	May 29, 2004 genta (advised by MIK) 共通関数化
					nX += DispEOF( hdc, x + nX * ( nCharWidth ), y, nCharWidth, nLineHeight, fuOptions,
						TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF] );
				}
				bEOF = TRUE;
			}
			if( bDispBkBitmap ){
			}else{
				/* 行末背景描画 */
				rcClip.left = x + nX * ( nCharWidth );
				rcClip.right = m_nViewAlignLeft + m_nViewCx;
				rcClip.top = y;
				rcClip.bottom = y + nLineHeight;
				if( rcClip.left < m_nViewAlignLeft ){
					rcClip.left = m_nViewAlignLeft;
				}
				if( rcClip.left < rcClip.right &&
					rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
					hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
					::FillRect( hdc, &rcClip, hBrush );
					::DeleteObject( hBrush );
				}
			}
			// 2006.04.29 Moca 選択処理のため縦線処理を追加
			DispVerticalLines( hdc, y, y + nLineHeight,  0, -1 );
			if( bSelected ){
				/* テキスト反転 */
				DispTextSelected( hdc, nLineNum, x, y, nX );
			}
		}
end_of_line:;
		nLineNum++;
		y += nLineHeight;
	}else{ // NULL == pLineの場合
		if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
			int nYPrev = y;
			
			if( bDispBkBitmap ){
			}else{
				/* 背景描画 */
				rcClip.left = 0;
				rcClip.right = m_nViewAlignLeft + m_nViewCx;
				rcClip.top = y;
				rcClip.bottom = y + nLineHeight;
				hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
				::FillRect( hdc, &rcClip, hBrush );
				::DeleteObject( hBrush );
			}
			/* EOF記号の表示 */
			nCount = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			if( nCount == 0 && m_nViewTopLine == 0 && nLineNum == 0 ){
				/* EOF記号の表示 */
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					//	May 29, 2004 genta (advised by MIK) 共通関数化
					nX += DispEOF( hdc, x, y, nCharWidth, nLineHeight, fuOptions,
						TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF] );
				}
				y += nLineHeight;
				bEOF = TRUE;
			}else{
				if( nCount > 0 && nLineNum == nCount ){
					const char*	pLine;
					int			nLineLen;
					const CLayout* pcLayout;
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCount - 1, &nLineLen, &pcLayout );
					nLineCols = LineIndexToColmn( pcLayout, nLineLen );
					if( ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ) ||
						nLineCols >= nWrapWidth
					 ){
						/* EOF記号の表示 */
						if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
							//	May 29, 2004 genta (advised by MIK) 共通関数化
							nX += DispEOF( hdc, x, y, nCharWidth, nLineHeight, fuOptions,
								TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF] );
						}
						y += nLineHeight;
						bEOF = TRUE;
					}
				}
			}
			// 2006.04.29 Moca 選択処理のため縦線処理を追加
			DispVerticalLines( hdc, nYPrev, nYPrev + nLineHeight,  0, -1 );
		}
	}

end_of_func:;
//	2002/05/08 YAZAKI アンダーラインの再描画は不要でした
//	MYTRACE( "m_nOldUnderLineY=%d\n", m_nOldUnderLineY );
//	if( -1 != m_nOldUnderLineY ){
//		/* カーソル行アンダーラインのON */
//		CaretUnderLineON( TRUE );
//	}
	return bEOF;
}

//	May 23, 2000 genta
/*! 画面描画補助関数:
	行末の改行マークを改行コードによって書き分ける（メイン）

	@param hdc Device Context Handle
	@param nPosX 描画座標X
	@param nPosY 描画座標Y
	@param nWidth  描画エリアのサイズX
	@param nHeight 描画エリアのサイズY
	@param cEol 行末コード種別
	@param bBold TRUE: 太字
	@param pColor 色

	@note bBoldがTRUEの時は横に1ドットずらして重ね書きを行うが、
	あまり太く見えない。
	
	@date 2001.12.21 YAZAKI 改行記号の描きかたを変更。ペンはこの関数内で作るようにした。
							矢印の先頭を、sx, syにして描画ルーチン書き直し。
*/
void CEditView::DrawEOL( HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight, CEOL cEol, int bBold, COLORREF pColor )
{
	int sx, sy;	//	矢印の先頭
	HANDLE	hPen;
	HPEN	hPenOld;
	hPen = ::CreatePen( PS_SOLID, 1, pColor );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	switch( cEol.GetType() ){
	case EOL_CRLF:	//	下左矢印
		sx = nPosX;
		sy = nPosY + ( nHeight / 2);
		::MoveToEx( hdc, sx + nWidth, sy - nHeight / 4, NULL );	//	上へ
		::LineTo(   hdc, sx + nWidth, sy );			//	下へ
		::LineTo(   hdc, sx, sy );					//	先頭へ
		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	先頭から下へ
		::MoveToEx( hdc, sx, sy, NULL);				//	先頭へ戻り
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	先頭から上へ
		if ( bBold ) {
			::MoveToEx( hdc, sx + nWidth + 1, sy - nHeight / 4, NULL );	//	上へ（右へずらす）
			++sy;
			::LineTo( hdc, sx + nWidth + 1, sy );	//	右へ（右にひとつずれている）
			::LineTo(   hdc, sx, sy );					//	先頭へ
			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	先頭から下へ
			::MoveToEx( hdc, sx, sy, NULL);				//	先頭へ戻り
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	先頭から上へ
		}
//		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//		::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy - ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL);
//		::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy - ( nHeight * 5 / 20 ) );
//		if( bBold ){
//			++sx;
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//			::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//			::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), nPosY + ( nHeight * 15/ 20) - ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL);
//			::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), nPosY + ( nHeight * 15/ 20) - ( nHeight * 5 / 20 ) );
//		}
		break;
	case EOL_CR:	//	左向き矢印	// 2007.08.17 ryoji EOL_LF -> EOL_CR
		sx = nPosX;
		sy = nPosY + ( nHeight / 2 );
		::MoveToEx( hdc, sx + nWidth, sy, NULL );	//	右へ
		::LineTo(   hdc, sx, sy );					//	先頭へ
		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	先頭から下へ
		::MoveToEx( hdc, sx, sy, NULL);				//	先頭へ戻り
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	先頭から上へ
		if ( bBold ) {
			++sy;
			::MoveToEx( hdc, sx + nWidth, sy, NULL );	//	右へ
			::LineTo(   hdc, sx, sy );					//	先頭へ
			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	先頭から下へ
			::MoveToEx( hdc, sx, sy, NULL);				//	先頭へ戻り
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	先頭から上へ
		}
//		::MoveToEx( hdc, nPosX, sy, NULL );
//		::LineTo(   hdc, nPosX + nWidth, sy );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4);
//		if( bBold ){
//			++sy;
//			::MoveToEx( hdc, nPosX, sy, NULL );
//			::LineTo(   hdc, nPosX + nWidth, sy );
//			::MoveToEx( hdc, sx, sy, NULL );
//			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4);
//		}
		break;
	case EOL_LF:	//	下向き矢印	// 2007.08.17 ryoji EOL_CR -> EOL_LF
		sx = nPosX + ( nWidth / 2 );
		sy = nPosY + ( nHeight * 3 / 4 );
		::MoveToEx( hdc, sx, nPosY + nHeight / 4 + 1, NULL );	//	上へ
		::LineTo(   hdc, sx, sy );								//	上から下へ
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	そのまま左上へ
		::MoveToEx( hdc, sx, sy, NULL);							//	矢印の先端に戻る
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	そして右上へ
		if( bBold ){
			++sx;
			::MoveToEx( hdc, sx, nPosY + nHeight / 4 + 1, NULL );
			::LineTo(   hdc, sx, sy );								//	上から下へ
			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	そのまま左上へ
			::MoveToEx( hdc, sx, sy, NULL);							//	矢印の先端に戻る
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	そして右上へ
		}
//#if 1
//		sx = nPosX + nWidth;
//		sy = nPosY + ( nHeight / 2 );
//		::MoveToEx( hdc, nPosX, sy, NULL );
//		::LineTo(   hdc, nPosX + nWidth, sy );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//		::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4);
//		if( bBold ){
//			++sy;
//			::MoveToEx( hdc, nPosX, sy, NULL );
//			::LineTo(   hdc, nPosX + nWidth, sy );
//			::MoveToEx( hdc, sx, sy, NULL );
//			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//			::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4);
//		}
//#else
//		sx = nPosX;
//		::MoveToEx( hdc, sx + nWidth - 3, nPosY + nHeight * 1 / 4, NULL );
//		::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4);
//		::LineTo(   hdc, sx, nPosY + nHeight * 3 / 4 );
//		::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4 - nHeight / 4);
//		if( bBold ){
//			++sx;
//			::MoveToEx( hdc, sx + nWidth - 3, nPosY + nHeight * 1 / 4, NULL );
//			::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4);
//			::LineTo(   hdc, sx, nPosY + nHeight * 3 / 4 );
//			::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4 - nHeight / 4);
//		}
//#endif
		break;
	case EOL_LFCR:
		sx = nPosX + ( nWidth / 2 );
		sy = nPosY + ( nHeight * 3 / 4 );
		::MoveToEx( hdc, sx + nWidth / 2, nPosY + nHeight / 4 + 1, NULL );	//	右上へ
		::LineTo(   hdc, sx, nPosY + nHeight / 4 + 1 );			//	右から左へ
		::LineTo(   hdc, sx, sy );								//	上から下へ
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	そのまま左上へ
		::MoveToEx( hdc, sx, sy, NULL);							//	矢印の先端に戻る
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	そして右上へ
		if( bBold ){
			::MoveToEx( hdc, sx + nWidth / 2, nPosY + nHeight / 4 + 2, NULL );	//	右上へ
			++sx;
			::LineTo(   hdc, sx, nPosY + nHeight / 4 + 2 );			//	右から左へ
			::LineTo(   hdc, sx, sy );								//	上から下へ
			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	そのまま左上へ
			::MoveToEx( hdc, sx, sy, NULL);							//	矢印の先端に戻る
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	そして右上へ
		}
//		sx = nPosX + ( nWidth / 2 );
//		sy = nPosY + ( nHeight * 1 / 4 );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//		::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, sy, NULL);
//		::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//		if( bBold ){
//			++sx;
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//			::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, sy, NULL );
//			::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, sy, NULL);
//			::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//		}
		break;
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}

/*
	タブ矢印描画関数
*/
void CEditView::DrawTabArrow( HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight, int bBold, COLORREF pColor )
{
	int sx, sy;	//	矢印の先頭
	HANDLE	hPen;
	HPEN	hPenOld;

	hPen = ::CreatePen( PS_SOLID, 1, pColor );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	nWidth--;
	sx = nPosX + nWidth;
	sy = nPosY + ( nHeight / 2 );
	::MoveToEx( hdc, sx - nWidth, sy, NULL );	//	左へ
	::LineTo(   hdc, sx, sy );					//	最後へ
	::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	最後から下へ
	::MoveToEx( hdc, sx, sy, NULL);				//	最後へ戻り
	::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	最後から上へ
	if ( bBold ) {
		++sy;
		::MoveToEx( hdc, sx - nWidth, sy, NULL );	//	左へ
		::LineTo(   hdc, sx, sy );					//	最後へ
		::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	最後から下へ
		::MoveToEx( hdc, sx, sy, NULL);				//	最後へ戻り
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	最後から上へ
	}

	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}

/*! EOF記号の描画

	@param hdc	[in] 描画対象のDevice Context
	@param x	[in] 表示座標x
	@param y	[in] 表示座標y
	@param nCharWidth	[in] 1文字の横幅(pixel)
	@param fuOptions	[in] ExtTextOut()に渡す描画オプション
	@param EofColInfo	[in] 描画に使う色情報

  @date 2004.05.29 genta MIKさんのアドバイスにより関数にくくりだし
*/
int CEditView::DispEOF( HDC hdc, int x, int y, int nCharWidth, int nLineHeight, UINT fuOptions, const ColorInfo& EofColInfo )
{
	//	元の色を保存するため
	COLORREF	colTextColorOld;
	COLORREF	colBkColorOld;
	RECT		rcClip;
	const char	pszEOF[] = "[EOF]";
	const int	szEOFlen = sizeof( pszEOF) - 1;

	colTextColorOld = ::SetTextColor( hdc, EofColInfo.m_colTEXT );	/* EOFの色 */
	colBkColorOld = ::SetBkColor( hdc, EofColInfo.m_colBACK );		/* EOF背景の色 */
	rcClip.left = /*m_nViewAlignLeft*/x;
	rcClip.right = rcClip.left + ( nCharWidth ) * szEOFlen;
	if( rcClip.left < m_nViewAlignLeft ){
		rcClip.left = m_nViewAlignLeft;
	}
	if( rcClip.left < rcClip.right &&
		rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;

		HFONT	hFontOld;
		/* フォントを選ぶ */
		hFontOld = (HFONT)::SelectObject( hdc,
			ChooseFontHandle(
				EofColInfo.m_bFatFont,
				EofColInfo.m_bUnderLine
			)
		);
//		if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bFatFont ){	/* 太字か */
//			hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//		}else{
//			hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//								}

		::ExtTextOut( hdc, x , y, fuOptions,
			&rcClip, pszEOF, szEOFlen, m_pnDx );

		::SelectObject( hdc, hFontOld );
	}

	::SetTextColor( hdc, colTextColorOld );
	::SetBkColor( hdc, colBkColorOld );
	
	return szEOFlen;
}
// 　　　

/*!	指定桁縦線の描画
	@param hdc     作画するウィンドウのDC
	@param nTop    線を引く上端のクライアント座標y
	@param nButtom 線を引く下端のクライアント座標y
	@param nColLeft  線を引く範囲の左桁の指定
	@param nColRight 線を引く範囲の右桁の指定(-1で未指定)

	@date 2005.11.08 Moca 新規作成
	@date 2006.04.29 Moca 太線・点線のサポート。選択中の反転対策に行ごとに作画するように変更
	    縦線の色がテキストの背景色と同じ場合は、縦線の背景色をEXORで作画する
	@note Common::m_nVertLineOffsetにより、指定桁の前の文字の上に作画されることがある。
*/
void CEditView::DispVerticalLines( HDC hdc, int nTop, int nBottom, int nLeftCol, int nRightCol )
{
	const Types&	typeData = m_pcEditDoc->GetDocumentAttribute();
	if( typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_bDisp == FALSE ){
		return;
	}
	nLeftCol = __max( m_nViewLeftCol, nLeftCol );
	const int nWrapWidth  = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();
	const int nCharWidth  = m_nCharWidth + typeData.m_nColmSpace;
	if( nRightCol < 0 ){
		nRightCol = nWrapWidth;
	}
	const int nPosXOffset = m_pShareData->m_Common.m_nVertLineOffset + m_nViewAlignLeft;
	const int nPosXLeft   = __max( m_nViewAlignLeft + (nLeftCol  - m_nViewLeftCol) * nCharWidth, m_nViewAlignLeft );
	const int nPosXRight  = __min( m_nViewAlignLeft + (nRightCol - m_nViewLeftCol) * nCharWidth, m_nViewCx + m_nViewAlignLeft );
	const int nLineHeight = m_nCharHeight + typeData.m_nLineSpace;
	bool bOddLine = ((((nLineHeight % 2) ? m_nViewTopLine : 0) + m_nViewAlignTop + nTop) % 2 == 1);

	// 太線
	const BOOL bBold = typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_bFatFont;
	// ドット線(下線属性を転用/テスト用)
	const BOOL bDot = typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_bUnderLine;
	const bool bExorPen = ( typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_colTEXT 
		== typeData.m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
	HPEN hPen;
	int nROP_Old = 0;
	if( bExorPen ){
		hPen = ::CreatePen( PS_SOLID, 0, typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_colBACK );
		nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
	}else{
		hPen = ::CreatePen( PS_SOLID, 0, typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_colTEXT );
	}
	HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );

	int k;
	for( k = 0; k < MAX_VERTLINES && typeData.m_nVertLineIdx[k] != 0; k++ ){
		// nXColは1開始。m_nViewLeftColは0開始なので注意。
		int nXCol = typeData.m_nVertLineIdx[k];
		int nXColEnd = nXCol;
		int nXColAdd = 1;
		// nXColがマイナスだと繰り返し。k+1を終了値、k+2をステップ幅として利用する
		if( nXCol < 0 ){
			if( k < MAX_VERTLINES - 2 ){
				nXCol = -nXCol;
				nXColEnd = typeData.m_nVertLineIdx[++k];
				nXColAdd = typeData.m_nVertLineIdx[++k];
				if( nXColEnd < nXCol || nXColAdd <= 0 ){
					continue;
				}
				// 作画範囲の始めまでスキップ
				if( nXCol < m_nViewLeftCol ){
					nXCol = m_nViewLeftCol + nXColAdd - (m_nViewLeftCol - nXCol) % nXColAdd;
				}
			}else{
				k += 2;
				continue;
			}
		}
		for(; nXCol <= nXColEnd; nXCol += nXColAdd ){
			if( nWrapWidth < nXCol ){
				break;
			}
			int nPosX = nPosXOffset + ( nXCol - 1 - m_nViewLeftCol ) * nCharWidth;
			// 2006.04.30 Moca 線の引く範囲・方法を変更
			// 太線の場合、半分だけ作画する可能性がある。
			int nPosXBold = nPosX;
			if( bBold ){
				nPosXBold -= 1;
			}
			if( nPosXRight <= nPosXBold ){
				break;
			}
			if( nPosXLeft <= nPosX ){
				if( bDot ){
					// 点線で作画。1ドットの線を作成
					int y = nTop;
					// スクロールしても線が切れないように座標を調整
					if( bOddLine ){
						y++;
					}
					for( ; y < nBottom; y += 2 ){
						if( nPosX < nPosXRight ){
							::MoveToEx( hdc, nPosX, y, NULL );
							::LineTo( hdc, nPosX, y + 1 );
						}
						if( bBold && nPosXLeft <= nPosXBold ){
							::MoveToEx( hdc, nPosXBold, y, NULL );
							::LineTo( hdc, nPosXBold, y + 1 );
						}
					}
				}else{
					if( nPosX < nPosXRight ){
						::MoveToEx( hdc, nPosX, nTop, NULL );
						::LineTo( hdc, nPosX, nBottom );
					}
					if( bBold && nPosXLeft <= nPosXBold ){
						::MoveToEx( hdc, nPosXBold, nTop, NULL );
						::LineTo( hdc, nPosXBold, nBottom );
					}
				}
			}
		}
	}
	if( bExorPen ){
		::SetROP2( hdc, nROP_Old );
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}


/*[EOF]*/
