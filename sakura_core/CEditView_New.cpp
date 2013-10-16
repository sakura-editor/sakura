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
	Copyright (C) 2006, Moca
	Copyright (C) 2007, ryoji, Moca
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditView.h"
#include "Debug.h"
#include "Funccode.h"
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
#include "CLayout.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/2/10 aroka

/*! 通常の描画処理 new 
	@param pPs  pPs.rcPaint は正しい必要がある
	@param bDrawFromComptibleBmp  TRUE 画面バッファからhdcに作画する(コピーするだけ)。
			TRUEの場合、pPs.rcPaint領域外は作画されないが、FALSEの場合は作画される事がある。
			互換DC/BMPが無い場合は、普通の作画処理をする。
	@date 2007.09.09 Moca 元々無効化されていた第三パラメータのbUseMemoryDCをbDrawFromComptibleBmpに変更。
	@date 2009.03.26 ryoji 行番号のみ描画を通常の行描画と分離（効率化）
*/
void CEditView::OnPaint( HDC hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp )
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

	//@@@
#if 0
	::MYTRACE( _T("OnPaint(%d,%d)-(%d,%d) : %d\n"),
		pPs->rcPaint.left,
		pPs->rcPaint.top,
		pPs->rcPaint.right,
		pPs->rcPaint.bottom,
		bDrawFromComptibleBmp
		);
#endif
	
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	// 互換BMPからの転送のみによる作画
	if( bDrawFromComptibleBmp
		&& m_hdcCompatDC && m_hbmpCompatBMP ){
		::BitBlt(
			hdc,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			m_hdcCompatDC,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
		if ( m_pcEditWnd->GetActivePane() == m_nMyIndex ){
			/* アクティブペインは、アンダーライン描画 */
			m_cUnderLine.CaretUnderLineON( true );
		}
		return;
	}
	if( m_hdcCompatDC && NULL == m_hbmpCompatBMP
		 || (pPs->rcPaint.right - pPs->rcPaint.left) < m_nCompatBMPWidth
		 || (pPs->rcPaint.bottom - pPs->rcPaint.top) < m_nCompatBMPHeight ){
		RECT rect;
		::GetWindowRect( m_hWnd, &rect );
		CreateOrUpdateCompatibleBitmap( rect.right - rect.left, rect.bottom - rect.top );
	}
	// To Here 2007.09.09 Moca

	/* キャレットを隠す */
	bool bCaretShowFlag_Old = m_bCaretShowFlag;	// 2008.06.09 ryoji
	HideCaret_( m_hWnd ); // 2002/07/22 novice

	//	May 9, 2000 genta
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	const int nWrapWidth = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

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
	int				nCharWidth = m_nCharWidth + TypeDataPtr->m_nColumnSpace;
	int				nLineTo;
	int				nX = m_nViewAlignLeft - m_nViewLeftCol * nCharWidth;
	int				nY;
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
	// 2007.09.09 Moca bUseMemoryDCを有効化。
	// bUseMemoryDC = FALSE;
	BOOL bUseMemoryDC = (m_hdcCompatDC != NULL);
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
		hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
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
		rc.right = m_nViewAlignLeft - m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace;
		rc.bottom = m_nViewAlignTop;
		hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_GYOU].m_sColorAttr.m_cBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}
	//	To Here Sep. 7, 2001 genta

	::SetBkMode( hdc, TRANSPARENT );
	hFontOld = (HFONT)::SelectObject( hdc, m_pcViewFont->GetFontHan() );
	m_hFontOld = NULL;


//	crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_colorBACK );
//	crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_colorTEXT );
	crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
	crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cTEXT );

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
	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( i );
	while( nRollBackLineNum < nMaxRollBackLineNum ){
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( i );
		if( NULL == pcLayout ){
			break;
		}
		if( 0 == pcLayout->m_ptLogicPos.x ){	/* 対応する論理行の先頭からのオフセット */
			break;
		}
		i--;
		nRollBackLineNum++;
	}

	nY = ( i - m_nViewTopLine ) * nLineHeight + m_nViewAlignTop;
	nLineTo = m_nViewTopLine + ( ( pPs->rcPaint.bottom - m_nViewAlignTop + (nLineHeight - 1) ) / nLineHeight ) - 1;	// 2009.02.17 ryoji 計算を精密化

	BOOL bSelected;
	bSelected = IsTextSelected();
	if( pPs->rcPaint.right <= m_nViewAlignLeft ){	// 2009.03.26 ryoji 行番号のみ描画を通常の行描画と分離（効率化）
		for( ; i <= nLineTo; ){
			pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( i );
			if( NULL == pcLayout )
				break;
			DispLineNumber( hdc, pcLayout, i, nY );
			nY += nLineHeight;	//描画Y座標＋＋
			i++;				//レイアウト行＋＋
		}
	}else{
		for( ; i <= nLineTo; ){
			pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( i );
			if( DispLineNew(
				hdc,
				pcLayout,
				i,
				nX,
				nY,
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
	}
	if( NULL != m_hFontOld ){
		::SelectObject( hdc, m_hFontOld );
		m_hFontOld = NULL;
	}

	/* テキストのない部分を背景色で塗りつぶす */
	if( nY < pPs->rcPaint.bottom ){
		rcBack.left = pPs->rcPaint.left;
		rcBack.right = pPs->rcPaint.right;
		rcBack.top = nY;
		rcBack.bottom = pPs->rcPaint.bottom;
		hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
		::FillRect( hdc, &rcBack, hBrush );
		::DeleteObject( hBrush );

		// 2006.04.29 行部分は行ごとに作画し、ここでは縦線の残りを作画
		DispVerticalLines( hdc, nY, pPs->rcPaint.bottom, 0, -1 );
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
			hPen = ::CreatePen( PS_SOLID, 0, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_sColorAttr.m_cTEXT );
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

	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	//     アンダーライン描画をメモリDCからのコピー前処理から後に移動
	if ( m_pcEditWnd->GetActivePane() == m_nMyIndex ){
		/* アクティブペインは、アンダーライン描画 */
		m_cUnderLine.CaretUnderLineON( true );
	}
	// To Here 2007.09.09 Moca

	/* 03/02/18 対括弧の強調表示(描画) ai */
	DrawBracketPair( true );

	/* キャレットを現在位置に表示します */
	if( bCaretShowFlag_Old )	// 2008.06.09 ryoji
		ShowCaret_( m_hWnd ); // 2002/07/22 novice
	return;
}









//@@@ 2001.02.17 Start by MIK
/*! 行のテキスト／選択状態の描画
	1回で1論理行分を作画する。

	@return EOFを作画したらtrue

	@date 2001.12.21 YAZAKI 改行記号の描きかたを変更
	@date 2007.08.31 kobake 引数 bDispBkBitmap を削除
*/
bool CEditView::DispLineNew(
	HDC						hdc,			//!< 作画対象
	const CLayout*			pcLayout,		//!< 表示を開始するレイアウト
	int&					nLineNum,		//!< 作画するレイアウト行番号(0開始), 次の物理行に対応するレイアウト行番号
	int						x,				//!< レイアウト0桁目の作画座標x
	int&					y,				//!< 作画座標y, 次の作画座標y
	int						nLineTo,		//!< 作画終了するレイアウト行番号
	BOOL					bSelected		//!< 選択中か
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DispLineNew" );

	//	May 9, 2000 genta
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	const int nWrapWidth = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	const char*				pLine;	//@@@ 2002.09.22 YAZAKI
	int						nLineLen;
	EColorIndexType			nCOMMENTMODE;
	EColorIndexType			nCOMMENTMODE_OLD;
	int						nCOMMENTEND;
	int						nCOMMENTEND_OLD;
	const CLayout*			pcLayout2;
	int						nColorIndex;
	bool					bDispEOF = false;

	/* 論理行データの取得 */
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory変更
		nLineLen = pcLayout->m_pCDocLine->m_cLine.GetStringLength() - pcLayout->m_ptLogicPos.x;
		pLine = pcLayout->m_pCDocLine->m_cLine.GetStringPtr() + pcLayout->m_ptLogicPos.x;

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
	SetCurrentColor( hdc, nCOMMENTMODE );

	int					nX = 0;
	//	テキスト描画モード
	const UINT			fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	const int			nLineHeight = m_nCharHeight + TypeDataPtr->m_nLineSpace;
	const int			nCharWidth = m_nCharWidth + TypeDataPtr->m_nColumnSpace;

	static const char*	pszSPACES = "        ";
	static const char*	pszZENSPACE	= "□";
	static const char*	pszWRAP	= "<";


	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		y -= nLineHeight;
		nLineNum--;
//		MYTRACE( _T("\n\n=======================================") );

		int			nBgn = 0;
		int			nPos = 0;
		int			nLineBgn =0;
		int			nCharChars = 0;
		BOOL		bSearchStringMode = FALSE;
		BOOL		bSearchFlg = TRUE;	// 2002.02.08 hor
		int			nSearchStart = -1;	// 2002.02.08 hor
		int			nSearchEnd	= -1;	// 2002.02.08 hor
		bool		bKeyWordTop	= true;	//	Keyword Top

		int			nNumLen;
		int			nUrlLen;
		//@@@ 2001.11.17 add start MIK
		int			nMatchLen;
		int			nMatchColor;

		//@@@ 2001.12.21 YAZAKI
		HBRUSH		hBrush;
		COLORREF	colTextColorOld;
		COLORREF	colBkColorOld;
		RECT		rcClip;
		RECT		rcClip2;

		while( nPos < nLineLen ){
//			MYTRACE( _T("nLineNum = %d\n"), nLineNum );

			y += nLineHeight;
			nLineNum++;
			if( m_nViewTopLine + m_nViewRowNum < nLineNum ){
				nLineNum = nLineTo + 1;
				goto end_of_func;
			}
			if( nLineTo < nLineNum ){
				goto end_of_func;
			}
			pcLayout2 = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
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
					hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
					::FillRect( hdc, &rcClip, hBrush );
					::DeleteObject( hBrush );
				}
				nX += pcLayout2->GetIndent();
			}

			while( nPos - nLineBgn < pcLayout2->m_nLength ){
				/* 検索文字列の色分け */
				if( m_bCurSrchKeyMark	/* 検索文字列のマーク */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
				// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
					if(!bSearchStringMode && (!m_sCurSearchOption.bRegularExp || (bSearchFlg && nSearchStart < nPos))){
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
						/* 現在の色を指定 */
						SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
					}else
					if( bSearchStringMode
					 && nSearchEnd <= nPos		//+ == では行頭文字の場合、nSearchEndも０であるために文字色の解除ができないバグを修正 2003.05.03 かろと
					){
						// 検索した文字列の終わりまできたら、文字色を標準に戻す処理
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						/* 現在の色を指定 */
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
							hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
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
								nColorIndex = COLORIDX_SEARCH;
							}else{
								nColorIndex = COLORIDX_EOL;
							}
							HFONT	hFontOld;
							/* フォントを選ぶ */
							hFontOld = (HFONT)::SelectObject( hdc,
								m_pcViewFont->ChooseFontHandle( TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sFontAttr )
							);
							colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cTEXT ); /* CRLFの色 */
							colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );	/* CRLF背景の色 */
							//	2003.08.17 ryoji 改行文字が欠けないように
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, (const char *)"  ", 2, m_pnDx );
							/* 改行記号の表示 */
							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOL].m_bDisp ){
								int nPosX = x + nX * ( nCharWidth );
								int nPosY = y;
								//	From Here 2003.08.17 ryoji 改行文字が欠けないように
								HRGN hRgn;
								hRgn = ::CreateRectRgnIndirect(&rcClip2);
								::SelectClipRgn(hdc, hRgn);
								//@@@ 2001.12.21 YAZAKI
								DrawEOL(hdc, nPosX + 1, nPosY, m_nCharWidth, m_nCharHeight,
									pcLayout2->m_cEol, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sFontAttr.m_bBoldFont,
										TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cTEXT );
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
						nCOMMENTMODE = (EColorIndexType)(COLORIDX_REGEX_FIRST + nMatchColor);	/* 色指定 */	//@@@ 2002.01.04 upd
						nCOMMENTEND = nPos + nMatchLen;  /* キーワード文字列の終端をセットする */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
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

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
					}else
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComments[0].Match_CommentFrom(nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComments[0].Match_CommentTo(nPos + (int)lstrlen( TypeDataPtr->m_cBlockComments[0].getBlockCommentFrom() ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComments[1].Match_CommentFrom(nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComments[1].Match_CommentTo(nPos + (int)lstrlen( TypeDataPtr->m_cBlockComments[1].getBlockCommentFrom() ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
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

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							int nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
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
							int nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
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
					}else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (nNumLen = IsNumber( pLine, nPos, nLineLen )) > 0 )		/* 半角数字を表示する */
					{
						/* キーワード文字列の終端をセットする */
						nNumLen = nPos + nNumLen;
						if( y/* + nLineHeight*/ >= m_nViewAlignTop )
						{
							/* テキスト表示 */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						/* 現在の色を指定 */
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_DIGIT;	/* 半角数値である */ // 2002/03/13 novice
						nCOMMENTEND = nNumLen;
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//@@@ 2001.02.17 End by MIK: 半角数値を強調表示
					}else
					if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* キーワードセット */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* 強調キーワードを表示する */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						//	Mar 4, 2001 genta comment out
						//	bKeyWordTop = false;
						/* キーワード文字列の終端を探す */
						int nKeyEnd;
						for( nKeyEnd = nPos + 1; nKeyEnd<= nLineLen - 1; ++nKeyEnd ){
							if( !IS_KEYWORD_CHAR( pLine[nKeyEnd] ) ){
								break;
							}
						}
						int nKeyLen = nKeyEnd - nPos;

						/* キーワードが登録単語ならば、色を変える */
						// 2005.01.13 MIK 強調キーワード数追加に伴う配列化 //MIK 2000.12.01 second keyword & binary search
						for( int n = 0; n < MAX_KEYWORDSET_PER_TYPE; n++ )
						{
							// 強調キーワードは前詰めで設定されるので、未設定のIndexがあれば中断
							if(TypeDataPtr->m_nKeyWordSetIdx[n] == -1 ){
									break;
							}
							else if(TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + n].m_bDisp)
							{
								/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
								int nIdx = m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SearchKeyWord2(
									TypeDataPtr->m_nKeyWordSetIdx[n],
									&pLine[nPos],
									nKeyLen
								);
								if( nIdx >= 0 ){
									if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
										/* テキスト表示 */
										nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
									}
									/* 現在の色を指定 */
									nBgn = nPos;
									nCOMMENTMODE = (EColorIndexType)(COLORIDX_KEYWORD1 + n);
									nCOMMENTEND = nKeyEnd;
									if( !bSearchStringMode ){
										SetCurrentColor( hdc, nCOMMENTMODE );
									}
									break;
								}
							}
						}
						//MIK END
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
						nCOMMENTEND = TypeDataPtr->m_cBlockComments[0].Match_CommentTo(nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
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
				case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComments[1].Match_CommentTo(nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
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
				case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							int nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
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
							int nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
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
					if( nCOMMENTMODE & COLORIDX_REGEX_BIT ){	//正規表現キーワード1～10
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
							 && !TypeDataPtr->m_bTabArrow ){	//タブ通常表示	//@@@ 2003.03.26 MIK
								if( bSearchStringMode ){
									nColorIndex = COLORIDX_SEARCH;
								}else{
									nColorIndex = COLORIDX_TAB;
								}
								colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cTEXT );	/* TAB文字の色 */
								colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );		/* TAB文字背景の色 */

								HFONT	hFontOld;
								/* フォントを選ぶ */
								hFontOld = (HFONT)::SelectObject( hdc,
									m_pcViewFont->ChooseFontHandle( TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sFontAttr )
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
									nColorIndex = COLORIDX_SEARCH;
									colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );	/* TAB文字背景の色 */
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
								 && TypeDataPtr->m_bTabArrow
								 && rcClip2.left <= x + nX * nCharWidth ) // Apr. 1, 2003 MIK 行番号と重なる
								{
									DrawTabArrow( hdc, x + nX * ( nCharWidth ), y, m_nCharWidth, m_nCharHeight,
										TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_sFontAttr.m_bBoldFont,
										TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_sColorAttr.m_cTEXT );
								}
							}
						}
						nX += tabDispWidth ;//	Sep. 22, 2002 genta
					}
					nBgn = nPos + 1;
					nCharChars = 1;
				}else
				if( (unsigned char)pLine[nPos] == 0x81 && (unsigned char)pLine[nPos + 1] == 0x40	//@@@ 2001.11.17 upd MIK
				 && !(nCOMMENTMODE & COLORIDX_REGEX_BIT) )	//@@@ 2002.01.04
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
									nColorIndex = COLORIDX_SEARCH;
								}else{
									nColorIndex = COLORIDX_ZENSPACE;
								}
								colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cTEXT );	/* 全角スペース文字の色 */
								colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );		/* 全角スペース文字背景の色 */


								HFONT	hFontOld;
								/* フォントを選ぶ */
								hFontOld = (HFONT)::SelectObject( hdc,
									m_pcViewFont->ChooseFontHandle( TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sFontAttr )
								);

								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, pszZENSPACE, lstrlen( pszZENSPACE ), m_pnDx );

								::SelectObject( hdc, hFontOld );
								::SetTextColor( hdc, colTextColorOld );
								::SetBkColor( hdc, colBkColorOld );

							}else{
							if( bSearchStringMode ){
								nColorIndex = COLORIDX_SEARCH;
								colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );	/* 文字背景の色 */
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
				 && !(nCOMMENTMODE & COLORIDX_REGEX_BIT) )
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
								nColorIndex = COLORIDX_SEARCH;
							}else{
								nColorIndex = COLORIDX_SPACE;
							}

							colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cTEXT );	/* 半角スペース文字の色 */
							colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );		/* 半角スペース文字背景の色 */
							HFONT	hFontOld = (HFONT)::SelectObject( hdc,
								m_pcViewFont->ChooseFontHandle( TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sFontAttr )
							);

							//小文字"o"の下半分を出力
							rcClip2.top = y + m_nCharHeight/2;
							rcClip2.bottom = y + nLineHeight;
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, "o", 1, m_pnDx );

							//上半分は普通の空白で出力（"o"の上半分を消す）
							rcClip2.top = y ;
							rcClip2.bottom = y + m_nCharHeight/2;
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
						int nCtrlEnd;
						for( nCtrlEnd = nPos + 1; nCtrlEnd <= nLineLen - 1; ++nCtrlEnd ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							int nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, nCtrlEnd );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if( (
								//	Jan. 23, 2002 genta 警告抑制
								( (unsigned char)pLine[nCtrlEnd] <= (unsigned char)0x1F ) ||
									( (unsigned char)'~' < (unsigned char)pLine[nCtrlEnd] && (unsigned char)pLine[nCtrlEnd] < (unsigned char)'｡' ) ||
									( (unsigned char)'ﾟ' < (unsigned char)pLine[nCtrlEnd] )
								) &&
								pLine[nCtrlEnd] != TAB && pLine[nCtrlEnd] != CR && pLine[nCtrlEnd] != LF
							){
							}else{
								break;
							}
						}
						nCOMMENTEND = nCtrlEnd;
						/* 現在の色を指定 */
						SetCurrentColor( hdc, nCOMMENTMODE );
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
					hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
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
						colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_sColorAttr.m_cTEXT );	/* 折り返し記号の色 */
						colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_sColorAttr.m_cBACK );		/* 折り返し記号背景の色 */
						/* フォントを選ぶ */
						hFontOld = (HFONT)::SelectObject( hdc,
							m_pcViewFont->ChooseFontHandle( TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_sFontAttr )
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
				bDispEOF = true;
			}

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
				hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
				::FillRect( hdc, &rcClip, hBrush );
				::DeleteObject( hBrush );
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

			/* 背景描画 */
			RECT		rcClip;
			rcClip.left = 0;
			rcClip.right = m_nViewAlignLeft + m_nViewCx;
			rcClip.top = y;
			rcClip.bottom = y + nLineHeight;
			HBRUSH hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
			::FillRect( hdc, &rcClip, hBrush );
			::DeleteObject( hBrush );

			/* EOF記号の表示 */
			int nCount = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			if( nCount == 0 && m_nViewTopLine == 0 && nLineNum == 0 ){
				/* EOF記号の表示 */
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					//	May 29, 2004 genta (advised by MIK) 共通関数化
					nX += DispEOF( hdc, x, y, nCharWidth, nLineHeight, fuOptions,
						TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF] );
				}
				y += nLineHeight;
				bDispEOF = true;
			}else{
				if( nCount > 0 && nLineNum == nCount ){
					const char*	pLine;
					int			nLineLen;
					const CLayout* pcLayout;
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCount - 1, &nLineLen, &pcLayout );
					int nLineCols = LineIndexToColumn( pcLayout, nLineLen );
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
						bDispEOF = true;
					}
				}
			}
			// 2006.04.29 Moca 選択処理のため縦線処理を追加
			DispVerticalLines( hdc, nYPrev, nYPrev + nLineHeight,  0, -1 );
		}
	}

end_of_func:;
	return bDispEOF;
}

//	May 23, 2000 genta
/*!	画面描画補助関数:
	行末の改行マークを改行コードによって書き分ける（メイン）

	@note bBoldがTRUEの時は横に1ドットずらして重ね書きを行うが、
	あまり太く見えない。
	
	@date 2001.12.21 YAZAKI 改行記号の描きかたを変更。ペンはこの関数内で作るようにした。
							矢印の先頭を、sx, syにして描画ルーチン書き直し。
*/
void CEditView::DrawEOL(
	HDC hdc,			//!< Device Context Handle
	int nPosX,			//!< 描画座標X
	int nPosY,			//!< 描画座標Y
	int nWidth,			//!< 描画エリアのサイズX
	int nHeight,		//!< 描画エリアのサイズY
	CEol cEol,			//!< 行末コード種別
	int bBold,			//!< TRUE: 太字
	COLORREF pColor		//!< 色
)
{
	int sx, sy;	//	矢印の先頭
	HANDLE	hPen;
	HPEN	hPenOld;
	hPen = ::CreatePen( PS_SOLID, 1, pColor );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	switch( cEol.GetType() ){
	case EOL_CRLF:	//	下左矢印
		{
			sx = nPosX;
			sy = nPosY + ( nHeight / 2);
			DWORD pp[] = { 3, 3 };
			POINT pt[6];
			pt[0].x = sx + nWidth;	//	上へ
			pt[0].y = sy - nHeight / 4;
			pt[1].x = sx + nWidth;	//	下へ
			pt[1].y = sy;
			pt[2].x = sx;	//	先頭へ
			pt[2].y = sy;
			pt[3].x = sx + nHeight / 4;	//	先頭から下へ
			pt[3].y = sy + nHeight / 4;
			pt[4].x = sx;	//	先頭へ戻り
			pt[4].y = sy;
			pt[5].x = sx + nHeight / 4;	//	先頭から上へ
			pt[5].y = sy - nHeight / 4;
			::PolyPolyline( hdc, pt, pp, _countof(pp));

			if ( bBold ) {
				pt[0].x += 1;	//	上へ（右へずらす）
				pt[0].y += 0;
				pt[1].x += 1;	//	右へ（右にひとつずれている）
				pt[1].y += 1;
				pt[2].x += 0;	//	先頭へ
				pt[2].y += 1;
				pt[3].x += 0;	//	先頭から下へ
				pt[3].y += 1;
				pt[4].x += 0;	//	先頭へ戻り
				pt[4].y += 1;
				pt[5].x += 0;	//	先頭から上へ
				pt[5].y += 1;
				::PolyPolyline( hdc, pt, pp, _countof(pp));
			}
		}
		break;
	case EOL_CR:	//	左向き矢印	// 2007.08.17 ryoji EOL_LF -> EOL_CR
		{
			sx = nPosX;
			sy = nPosY + ( nHeight / 2 );
			DWORD pp[] = { 3, 2 };
			POINT pt[5];
			pt[0].x = sx + nWidth;	//	右へ
			pt[0].y = sy;
			pt[1].x = sx;	//	先頭へ
			pt[1].y = sy;
			pt[2].x = sx + nHeight / 4;	//	先頭から下へ
			pt[2].y = sy + nHeight / 4;
			pt[3].x = sx;	//	先頭へ戻り
			pt[3].y = sy;
			pt[4].x = sx + nHeight / 4;	//	先頭から上へ
			pt[4].y = sy - nHeight / 4;
			::PolyPolyline( hdc, pt, pp, _countof(pp));

			if ( bBold ) {
				pt[0].x += 0;	//	右へ
				pt[0].y += 1;
				pt[1].x += 0;	//	先頭へ
				pt[1].y += 1;
				pt[2].x += 0;	//	先頭から下へ
				pt[2].y += 1;
				pt[3].x += 0;	//	先頭へ戻り
				pt[3].y += 1;
				pt[4].x += 0;	//	先頭から上へ
				pt[4].y += 1;
				::PolyPolyline( hdc, pt, pp, _countof(pp));
			}
		}
		break;
	case EOL_LF:	//	下向き矢印	// 2007.08.17 ryoji EOL_CR -> EOL_LF
		{
			sx = nPosX + ( nWidth / 2 );
			sy = nPosY + ( nHeight * 3 / 4 );

			DWORD pp[] = { 3, 2 };
			POINT pt[5];
			pt[0].x = sx;	//	上へ
			pt[0].y = nPosY + nHeight / 4 + 1;
			pt[1].x = sx;	//	上から下へ
			pt[1].y = sy;
			pt[2].x = sx - nHeight / 4;	//	そのまま左上へ
			pt[2].y = sy - nHeight / 4;
			pt[3].x = sx;	//	矢印の先端に戻る
			pt[3].y = sy;
			pt[4].x = sx + nHeight / 4;	//	そして右上へ
			pt[4].y = sy - nHeight / 4;
			::PolyPolyline( hdc, pt, pp, _countof(pp));

			if( bBold ){
				pt[0].x += 1;	//	上へ
				pt[0].y += 0;
				pt[1].x += 1;	//	上から下へ
				pt[1].y += 0;
				pt[2].x += 1;	//	そのまま左上へ
				pt[2].y += 0;
				pt[3].x += 1;	//	矢印の先端に戻る
				pt[3].y += 0;
				pt[4].x += 1;	//	そして右上へ
				pt[4].y += 0;
				::PolyPolyline( hdc, pt, pp, _countof(pp));
			}
		}
		break;
	case EOL_LFCR:
		{
			sx = nPosX + ( nWidth / 2 );
			sy = nPosY + ( nHeight * 3 / 4 );
			DWORD pp[] = { 4, 2 };
			POINT pt[6];
			pt[0].x = sx + nWidth / 2;	//	右上へ
			pt[0].y = nPosY + nHeight / 4 + 1;
			pt[1].x = sx;	//	右から左へ
			pt[1].y = nPosY + nHeight / 4 + 1;
			pt[2].x = sx;	//	上から下へ
			pt[2].y = sy;
			pt[3].x = sx - nHeight / 4;	//	そのまま左上へ
			pt[3].y = sy - nHeight / 4;
			pt[4].x = sx;	//	矢印の先端に戻る
			pt[4].y = sy;
			pt[5].x = sx + nHeight / 4;	//	矢印の先端に戻る
			pt[5].y = sy - nHeight / 4;
			::PolyPolyline( hdc, pt, pp, _countof(pp));

			if ( bBold ) {
				pt[0].x += 0;	//	右上へ
				pt[0].y += 1;
				pt[1].x += 1;	//	右から左へ
				pt[1].y += 1;
				pt[2].x += 1;	//	上から下へ
				pt[2].y += 0;
				pt[3].x += 1;	//	そのまま左上へ
				pt[3].y += 0;
				pt[4].x += 1;	//	矢印の先端に戻る
				pt[4].y += 0;
				pt[5].x += 1;	//	そして右上へ
				pt[5].y += 0;
				::PolyPolyline( hdc, pt, pp, _countof(pp));
			}

//			::MoveToEx( hdc, sx + nWidth / 2, nPosY + nHeight / 4 + 1, NULL );	//	右上へ
//			::LineTo(   hdc, sx, nPosY + nHeight / 4 + 1 );			//	右から左へ
//			::LineTo(   hdc, sx, sy );								//	上から下へ
//			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	そのまま左上へ
//			::MoveToEx( hdc, sx, sy, NULL);							//	矢印の先端に戻る
//			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	そして右上へ
//			if( bBold ){
//				::MoveToEx( hdc, sx + nWidth / 2, nPosY + nHeight / 4 + 2, NULL );	//	右上へ
//				++sx;
//				::LineTo(   hdc, sx, nPosY + nHeight / 4 + 2 );			//	右から左へ
//				::LineTo(   hdc, sx, sy );								//	上から下へ
//				::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	そのまま左上へ
//				::MoveToEx( hdc, sx, sy, NULL);							//	矢印の先端に戻る
//				::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	そして右上へ
//			}
		}
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
	static const char	pszEOF[] = "[EOF]";
	const int	szEOFlen = sizeof( pszEOF) - 1;

	colTextColorOld = ::SetTextColor( hdc, EofColInfo.m_sColorAttr.m_cTEXT );	/* EOFの色 */
	colBkColorOld = ::SetBkColor( hdc, EofColInfo.m_sColorAttr.m_cBACK );		/* EOF背景の色 */
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
			m_pcViewFont->ChooseFontHandle( EofColInfo.m_sFontAttr )
		);

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
	@note Common::m_sWindow::m_nVertLineOffsetにより、指定桁の前の文字の上に作画されることがある。
*/
void CEditView::DispVerticalLines( HDC hdc, int nTop, int nBottom, int nLeftCol, int nRightCol )
{
	const STypeConfig&	typeData = m_pcEditDoc->GetDocumentAttribute();
	if( !typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_bDisp ){
		return;
	}
	nLeftCol = t_max( m_nViewLeftCol, nLeftCol );
	const int nWrapWidth  = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
	const int nCharWidth  = m_nCharWidth + typeData.m_nColumnSpace;
	if( nRightCol < 0 ){
		nRightCol = nWrapWidth;
	}
	const int nPosXOffset = m_pShareData->m_Common.m_sWindow.m_nVertLineOffset + m_nViewAlignLeft;
	const int nPosXLeft   = t_max( m_nViewAlignLeft + (nLeftCol  - m_nViewLeftCol) * nCharWidth, m_nViewAlignLeft );
	const int nPosXRight  = t_min( m_nViewAlignLeft + (nRightCol - m_nViewLeftCol) * nCharWidth, m_nViewCx + m_nViewAlignLeft );
	const int nLineHeight = m_nCharHeight + typeData.m_nLineSpace;
	bool bOddLine = ((((nLineHeight % 2) ? m_nViewTopLine : 0) + m_nViewAlignTop + nTop) % 2 == 1);

	// 太線
	const BOOL bBold = typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_sFontAttr.m_bBoldFont;
	// ドット線(下線属性を転用/テスト用)
	const BOOL bDot = typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_sFontAttr.m_bUnderLine;
	const bool bExorPen = ( typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_sColorAttr.m_cTEXT 
		== typeData.m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
	HPEN hPen;
	int nROP_Old = 0;
	if( bExorPen ){
		hPen = ::CreatePen( PS_SOLID, 0, typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_sColorAttr.m_cBACK );
		nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
	}else{
		hPen = ::CreatePen( PS_SOLID, 0, typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_sColorAttr.m_cTEXT );
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
