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
#include "CRegexKeyword.h"	//@@@ 2001.11.17 add MIK
#include "my_icmp.h"	//@@@ 2002.01.13 add
#include "Clayout.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/2/10 aroka
#include "CTypeSupport.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"

// 通常の描画処理 new
void CEditView::OnPaint( HDC hdc, PAINTSTRUCT *pPs, BOOL bUseMemoryDC )
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::OnPaint" );

	// 2004.01.28 Moca デスクトップに作画しないように
	if( NULL == hdc )return;

	if( !GetDrawSwitch() )return;

	// キャレットを隠す
	GetCaret().HideCaret_( m_hWnd ); // 2002/07/22 novice

	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	RECT			rc;
	int				nLineHeight = GetTextMetrics().GetHankakuDy();
	int				nCharDx = GetTextMetrics().GetHankakuDx();
	const CLayout*	pcLayout;

	//サポート
	CTypeSupport cTextType(this,COLORIDX_TEXT);

//@@@ 2001.11.17 add start MIK
	//変更があればタイプ設定を行う。
	if( TypeDataPtr->m_bUseRegexKeyword || m_cRegexKeyword->m_bUseRegexKeyword ) //OFFなのに前回のデータが残ってる
	{
		//タイプ別設定をする。設定済みかどうかは呼び先でチェックする。
		m_cRegexKeyword->RegexKeySetTypes(TypeDataPtr);
	}
//@@@ 2001.11.17 add end MIK

	// メモリＤＣを利用した再描画の場合は描画先のＤＣを切り替える
	HDC hdcOld;
	bUseMemoryDC = FALSE;
	if( bUseMemoryDC ){
		hdcOld = hdc;
		hdc = m_hdcCompatDC;
	}

	/* 03/02/18 対括弧の強調表示(消去) ai */
	DrawBracketPair( false );

	/* ルーラーとテキストの間の余白 */
	//@@@ 2002.01.03 YAZAKI 余白が0のときは無駄でした。
	if ( GetTextArea().GetTopYohaku() ){
		rc.left   = 0;
		rc.top    = GetTextArea().GetRulerHeight();
		rc.right  = GetTextArea().GetAreaRight();
		rc.bottom = GetTextArea().GetAreaTop();

		cTextType.FillBack(hdc,rc);
	}
	
	/* 行番号の表示 */
	//	From Here Sep. 7, 2001 genta
	//	Sep. 23, 2002 genta 行番号非表示でも行番号色の帯があるので隙間を埋める
	if( GetTextArea().GetTopYohaku() ){ 
		rc.left   = 0;
		rc.top    = GetTextArea().GetRulerHeight();
		rc.right  = GetTextArea().GetAreaLeft() - m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace; //	Sep. 23 ,2002 genta 余白はテキスト色のまま残す
		rc.bottom = GetTextArea().GetAreaTop();
		HBRUSH hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_GYOU].m_colBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}
	//	To Here Sep. 7, 2001 genta

	::SetBkMode( hdc, TRANSPARENT );
	m_hFontOld = NULL;

	cTextType.SetFont(hdc);
	cTextType.SetColors(hdc);


	int nTop = pPs->rcPaint.top;
	BOOL bEOF = FALSE;



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//           描画開始行 -> nLayoutLine             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt nLayoutLine;
	if( 0 > nTop - GetTextArea().GetAreaTop() ){
		nLayoutLine = GetTextArea().GetViewTopLine(); //ビュー上部から描画
	}else{
		nLayoutLine = GetTextArea().GetViewTopLine() + CLayoutInt( ( nTop - GetTextArea().GetAreaTop() ) / nLineHeight ); //ビュー途中から描画
	}

	int nMaxRollBackLineNum = 260 / (Int)nWrapKeta + 1;
	int nRollBackLineNum = 0;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLayoutLine );
	while( nRollBackLineNum < nMaxRollBackLineNum ){
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLayoutLine );
		if( NULL == pcLayout ){
			break;
		}
		if( 0 == pcLayout->GetLogicOffset() ){	/* 対応する論理行の先頭からのオフセット */
			break;
		}
		nLayoutLine--;
		nRollBackLineNum++;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          描画終了行 -> nLogicLineTo            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt nLogicLineTo = GetTextArea().GetViewTopLine() + CLayoutInt( ( pPs->rcPaint.bottom - GetTextArea().GetAreaTop() ) / nLineHeight );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         描画座標                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	DispPos sPos(GetTextMetrics().GetHankakuDx(),GetTextMetrics().GetHankakuDy());
	sPos.InitDrawPos(CMyPoint(
		GetTextArea().GetAreaLeft() - (Int)GetTextArea().GetViewLeftCol() * nCharDx,
		GetTextArea().GetAreaTop() + (Int)( nLayoutLine - GetTextArea().GetViewTopLine() ) * nLineHeight
	));
	sPos.SetLayoutLineRef(nLayoutLine);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      全部の行を描画                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//必要な行を描画する
	bool bSelected = GetSelectionInfo().IsTextSelected();
	while(sPos.GetLayoutLineRef() <= nLogicLineTo)
	{
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( sPos.GetLayoutLineRef() );
	
		sPos.ResetDrawCol();

		bool bDispResult = DispLineNew(
			hdc,
			pcLayout,
			&sPos,
			nLogicLineTo,
			bSelected
		);

		if(bDispResult){
			pPs->rcPaint.bottom += nLineHeight;	// EOF再描画対応
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


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              テキストの無い部分の塗りつぶし                 //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if( IsBkBitmap() ){
	}else{
		/* テキストのない部分を背景色で塗りつぶす */
		if( sPos.GetDrawPos().y < pPs->rcPaint.bottom ){
			RECT rcBack;
			rcBack.left   = pPs->rcPaint.left;
			rcBack.right  = pPs->rcPaint.right;
			rcBack.top    = sPos.GetDrawPos().y;
			rcBack.bottom = pPs->rcPaint.bottom;

			cTextType.FillBack(hdc,rcBack);

			// 2006.04.29 行部分は行ごとに作画し、ここでは縦線の残りを作画
			GetTextDrawer().DispVerticalLines( hdc, sPos.GetDrawPos().y, pPs->rcPaint.bottom, CLayoutInt(0), CLayoutInt(-1) );
		}
	}

	cTextType.RewindColors(hdc);
	cTextType.RewindFont(hdc);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                折り返し位置の表示 (縦線)                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CTypeSupport cWrapType(this,COLORIDX_WRAP);
	if( cWrapType.IsDisp() ){
		int nXPos = GetTextArea().GetAreaLeft() + (Int)( nWrapKeta - GetTextArea().GetViewLeftCol() ) * nCharDx;
		//	2005.11.08 Moca 作画条件変更
		if( GetTextArea().GetAreaLeft() < nXPos && nXPos < GetTextArea().GetAreaRight() ){
			/// 折り返し記号の色のペンを設定
			cWrapType.SetSolidPen(hdc,0);

			::MoveToEx( hdc, nXPos, GetTextArea().GetAreaTop(), NULL );
			::LineTo( hdc, nXPos, GetTextArea().GetAreaBottom() );

			cWrapType.RewindPen(hdc);
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ルーラー描画                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if ( pPs->rcPaint.top < GetTextArea().GetRulerHeight() ) { // ルーラーが再描画範囲にあるときのみ再描画する 2002.02.25 Add By KK
		GetRuler().SetRedrawFlag(); //2002.02.25 Add By KK ルーラー全体を描画。
		GetRuler().DispRuler( hdc );
	}

	if ( m_pcEditWnd->m_nActivePaneIndex == m_nMyIndex ){
		/* アクティブペインは、アンダーライン描画 */
		GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     その他後始末など                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
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
	GetCaret().ShowCaret_( m_hWnd ); // 2002/07/22 novice
	return;
}









/*
2002/03/13 novice
@par nCOMMENTMODE
関数内部で状態遷移のために使われる変数nCOMMENTMODEと状態の関係。
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
*/

//@@@ 2001.02.17 MIK
//@@@ 2001.12.21 YAZAKI 改行記号の描きかたを変更
//@@@ 2007.08.31 kobake 引数 bDispBkBitmap を削除
/*!
	行のテキスト／選択状態の描画
	1回で1論理行分を作画する。

	@return EOFを作画したらtrue
*/
bool CEditView::DispLineNew(
	HDC				hdc,			//!< [in]     作画対象
	const CLayout*	pcLayout,		//!< [in]     表示を開始するレイアウト
	DispPos*		pDispPos,		//!< [in/out] 描画する箇所、描画元ソース
	CLayoutInt		nLineTo,		//!< [in]     作画終了するレイアウト行番号
	bool			bSelected		//!< [in]     選択中か
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DispLineNew" );

	//DispPosを保存しておく
	DispPos sDispPosBegin = *pDispPos;

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	int		nCharChars_2;
	EColorIndexType		nCOMMENTMODE;
	EColorIndexType		nCOMMENTMODE_OLD;
	int		nCOMMENTEND;
	int		nCOMMENTEND_OLD;

	bool	bEOF = false;
	int		i, j;
	int		nIdx;
	int		nUrlLen;
	int		nColorIdx;
	bool	bKeyWordTop = true;	//	Keyword Top

	//サイズ
	Types* TypeDataPtr = &m_pcEditDoc->GetDocumentAttribute();
	int nLineHeight = GetTextMetrics().GetHankakuDy();  //行の縦幅？
	int nCharDx  = GetTextMetrics().GetHankakuDx();  //半角

	bool bSearchStringMode = false; //検索ヒットフラグ？これで色分けを判定する
	BOOL bSearchFlg        = TRUE;  //？ 2002.02.08 hor
	CLogicInt  nSearchStart      = CLogicInt(-1);    //？ 2002.02.08 hor
	CLogicInt  nSearchEnd        = CLogicInt(-1);    //？ 2002.02.08 hor

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          論理行データの取得 -> pLine, pLineLen              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	const CLayout*	pcLayout2; //ワーク用CLayoutポインタ
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory変更
		nLineLen = pcLayout->m_pCDocLine->m_cLine.GetStringLength() - pcLayout->GetLogicOffset();
		pLine    = pcLayout->m_pCDocLine->m_cLine.GetStringPtr()    + pcLayout->GetLogicOffset();

		// タイプ
		// 0=通常
		// 1=行コメント
		// 2=ブロックコメント
		// 3=シングルクォーテーション文字列
		// 4=ダブルクォーテーション文字列
		nCOMMENTMODE = pcLayout->GetColorTypePrev();
		nCOMMENTEND = 0;
		pcLayout2 = pcLayout;
	}
	else{
		pLine = NULL;
		nLineLen = CLogicInt(0);
		nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		nCOMMENTEND = 0;

		pcLayout2 = NULL;
	}

	/* 現在の色を指定 */
	SetCurrentColor( hdc, nCOMMENTMODE );

	int nLineBgn   = 0; //？
//	int nX         = 0; //テキストX位置
	CLogicInt nCharChars = CLogicInt(0); //処理した文字数

	//処理する文字位置
	CLogicInt nPos = CLogicInt(0);
	#define SetNPos(N) nPos=(N)
	#define GetNPos() (nPos+CLogicInt(0))

	//通常テキスト開始位置 (ほぼ固定)
	int nBgn = 0;
	#define SetNBgn(N) nBgn=(N)
	#define GetNBgn() (nBgn+0)

	//サポート
	CTypeSupport cTextType(this,COLORIDX_TEXT);
	CTypeSupport cSearchType(this,COLORIDX_SEARCH);

	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		pDispPos->ForwardDrawLine(-1);
		pDispPos->ForwardLayoutLineRef(-1);

		while( GetNPos() < nLineLen ){
			pDispPos->ForwardDrawLine(1);
			pDispPos->ForwardLayoutLineRef(1);
			if( GetTextArea().GetBottomLine() < pDispPos->GetLayoutLineRef() ){
				pDispPos->SetLayoutLineRef(nLineTo + CLayoutInt(1));
				goto end_of_func;
			}
			if( nLineTo < pDispPos->GetLayoutLineRef() ){
				goto end_of_func;
			}

			pcLayout2 = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( pDispPos->GetLayoutLineRef() );

			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                        行番号描画                           //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
				// 行番号表示
				GetTextDrawer().DispLineNumber( hdc, pcLayout2, (Int)pDispPos->GetLayoutLineRef(), pDispPos->GetDrawPos().y );
			}


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                       本文描画開始                          //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			SetNBgn(GetNPos());
			nLineBgn = GetNBgn();
			pDispPos->ResetDrawCol();


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                 行頭(インデント)背景描画                    //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			if(pcLayout2 && pcLayout2->GetIndent()!=0)
			{
				RECT rcClip;
				if(GetTextArea().GenerateClipRect(&rcClip,*pDispPos,(Int)pcLayout2->GetIndent())){
					cTextType.FillBack(hdc,rcClip);
				}
				//描画位置進める
				pDispPos->ForwardDrawCol((Int)pcLayout2->GetIndent());
			}


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                 通常文字列以外描画ループ                    //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//行終端または折り返しに達するまでループ
			while( GetNPos() - nLineBgn < pcLayout2->GetLength() ){
				/* 検索文字列の色分け */
				/* 検索文字列のマーク */
				if( m_bCurSrchKeyMark && cSearchType.IsDisp() ){
searchnext:;
					// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
					//        検索ヒットフラグ設定 -> bSearchStringMode            //
					// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

					// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
					if(!bSearchStringMode && (!m_sCurSearchOption.bRegularExp || (bSearchFlg && nSearchStart < GetNPos()))){
						bSearchFlg=IsSearchString( pLine, nLineLen, GetNPos(), &nSearchStart, &nSearchEnd );
					}
					if( !bSearchStringMode && bSearchFlg && nSearchStart==GetNPos()){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						bSearchStringMode = true;

						// 現在の色を指定
						SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
					}
					else if( bSearchStringMode && nSearchEnd <= GetNPos() ){ //+ == では行頭文字の場合、nSearchEndも０であるために文字色の解除ができないバグを修正 2003.05.03 かろと
						// 検索した文字列の終わりまできたら、文字色を標準に戻す処理
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							// テキスト表示
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						/* 現在の色を指定 */
						SetCurrentColor( hdc, nCOMMENTMODE );
						bSearchStringMode = false;
						goto searchnext;
					}


				}

				if( GetNPos() >= nLineLen - pcLayout2->m_cEol.GetLen() ){
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						/* テキスト表示 */
						GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						SetNBgn(GetNPos() + 1);

						// 行末背景描画
						RECT rcClip;
						if(GetTextArea().GenerateClipRectRight(&rcClip,*pDispPos)){
							cTextType.FillBack(hdc,rcClip);
						}

						// 改行記号の表示
						GetTextDrawer().DispEOL(hdc,pDispPos,pcLayout2->m_cEol,bSearchStringMode);

						// 2006.04.29 Moca 選択処理のため縦線処理を追加
						GetTextDrawer().DispVerticalLines( hdc, pDispPos->GetDrawPos().y, pDispPos->GetDrawPos().y + nLineHeight, CLayoutInt(0), CLayoutInt(-1) );
						if( bSelected ){
							/* テキスト反転 */
							DispTextSelected(
								hdc,
								pDispPos->GetLayoutLineRef(),
								CMyPoint(sDispPosBegin.GetDrawPos().x, pDispPos->GetDrawPos().y),
								pDispPos->GetDrawCol()
							);
						}
					}

					goto end_of_line;
				}
SEARCH_START:;
				int		nMatchLen;
				int		nMatchColor;
				switch( nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
//@@@ 2001.11.17 add start MIK
					//正規表現キーワード
					if( TypeDataPtr->m_bUseRegexKeyword
					 && m_cRegexKeyword->RegexIsKeyword( pLine, GetNPos(), nLineLen, &nMatchLen, &nMatchColor )
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() )
						{
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						/* 現在の色を指定 */
						SetNBgn(GetNPos());
						nCOMMENTMODE = MakeColorIndexType_RegularExpression(nMatchColor);	/* 色指定 */	//@@@ 2002.01.04 upd
						nCOMMENTEND = GetNPos() + nMatchLen;  /* キーワード文字列の終端をセットする */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
						}
					}
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					else if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( GetNPos(), nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());

						nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice

						/* コメントを表示する */
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
					}
					//	Mar. 15, 2000 genta
					else if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, GetNPos(), nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}

						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, GetNPos() + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI

					}
					else if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, GetNPos(), nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}

						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, GetNPos() + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}
					else if( pLine[GetNPos()] == L'\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* シングルクォーテーション文字列を表示する */
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
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
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
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
					}
					else if( pLine[GetNPos()] == L'"' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* ダブルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
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
					}
					else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URLを表示する */
					 && IsURL( &pLine[GetNPos()], nLineLen - GetNPos(), &nUrlLen )	/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_URL;	/* URLモード */ // 2002/03/13 novice
						nCOMMENTEND = GetNPos() + nUrlLen;
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//@@@ 2001.02.17 Start by MIK: 半角数値を強調表示
//#ifdef COMPILE_COLOR_DIGIT
					}
					else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (i = IsNumber( pLine, GetNPos(), nLineLen )) > 0 )		/* 半角数字を表示する */
					{
						/* キーワード文字列の終端をセットする */
						i = GetNPos() + i;
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() )
						{
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						/* 現在の色を指定 */
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_DIGIT;	/* 半角数値である */ // 2002/03/13 novice
						nCOMMENTEND = i;
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//#endif
//@@@ 2001.02.17 End by MIK: 半角数値を強調表示
					}
					else if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* キーワードセット */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* 強調キーワードを表示する */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[GetNPos()] )
					){
						//	Mar 4, 2001 genta comment out
						//	bKeyWordTop = false;
						/* キーワード文字列の終端を探す */
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							if( IS_KEYWORD_CHAR( pLine[i] ) ){
							}else{
								break;
							}
						}
						/* キーワードが登録単語ならば、色を変える */
						j = i - GetNPos();
						/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
						nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
							TypeDataPtr->m_nKeyWordSetIdx[0],
							&pLine[GetNPos()],
							j
						);
						if( nIdx != -1 ){
							if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
								/* テキスト表示 */
								GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
							}

							/* 現在の色を指定 */
							SetNBgn(GetNPos());
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
										&pLine[GetNPos()],																		//MIK
										j																					//MIK
									);																						//MIK
									if( nIdx != -1 ){																		//MIK
										if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){										//MIK
											/* テキスト表示 */																//MIK
											GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );	//MIK
										}																					//MIK
										/* 現在の色を指定 */																//MIK
										SetNBgn(GetNPos());																		//MIK
										nCOMMENTMODE = (EColorIndexType)(COLORIDX_KEYWORD1 + my_i);	/* 強調キーワード2 */ // 2002/03/13 novice		//MIK
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
					if( IS_KEYWORD_CHAR( pLine[GetNPos()] ))	bKeyWordTop = false;
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
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* コントロールコード */ // 2002/03/13 novice
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
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
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, GetNPos(), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
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
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, GetNPos(), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
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
						for( i = GetNPos()/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
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
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
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
						for( i = GetNPos()/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
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
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
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
						if( GetNPos() == nCOMMENTEND ){
							if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
								/* テキスト表示 */
								GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
							}
							SetNBgn(GetNPos());
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

				//タブ
				if( pLine[GetNPos()] == WCODE::TAB ){
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						// テキスト表示
						GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );

						// 色決定
						if( bSearchStringMode ){
							nColorIdx = COLORIDX_SEARCH;
						}else{
							nColorIdx = COLORIDX_TAB;
						}

						// タブ表示
						GetTextDrawer().DispTab( hdc, pDispPos, nColorIdx );
					}
					SetNBgn(GetNPos() + 1);
					nCharChars = CLogicInt(1);
				}
				//全角スペース
				else if( WCODE::isZenkakuSpace(pLine[GetNPos()]) && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						// 全角スペース以前のテキストを表示
						GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );

						// 全角空白を表示する
						GetTextDrawer().DispZenkakuSpace(hdc,pDispPos,bSearchStringMode);
					}
					//文字進める
					SetNBgn(GetNPos() + 1);
					nCharChars = CLogicInt(1);
				}
				//半角空白（半角スペース）を表示 2002.04.28 Add by KK 
				else if (pLine[GetNPos()] == ' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp 
					 && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{
					GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						GetTextDrawer().DispHankakuSpace(hdc,pDispPos,bSearchStringMode);
					}
					SetNBgn(GetNPos() + 1);
					nCharChars = CLogicInt(1);
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, GetNPos() );
					if( 0 == nCharChars ){
						nCharChars = CLogicInt(1);
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* コントロールコードを色分け */
					 &&	WCODE::isControlCode(pLine[GetNPos()])
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* テキスト表示 */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE_OLD = nCOMMENTMODE;
						nCOMMENTEND_OLD = nCOMMENTEND;
						nCOMMENTMODE = COLORIDX_CTRLCODE;	/* コントロールコード モード */ // 2002/03/13 novice
						/* コントロールコード列の終端を探す */
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if(!WCODE::isControlCode(pLine[i])){
								break;
							}
						}
						nCOMMENTEND = i;
						/* 現在の色を指定 */
						SetCurrentColor( hdc, nCOMMENTMODE );
					}
				}
				SetNPos( GetNPos() + nCharChars );
			}
			//end of while( GetNPos() - nLineBgn < pcLayout2->GetLength() )


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                         本文描画                            //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			if( GetNPos() >= nLineLen ){
				break;
			}
			if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
				/* テキスト表示 */
				GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
				SetNBgn(GetNPos());

				/* 行末背景描画 */
				RECT rcClip;
				if(GetTextArea().GenerateClipRectRight(&rcClip,*pDispPos)){
					cTextType.FillBack(hdc,rcClip);
				}

				/* 折り返し記号を表示する */
				GetTextDrawer().DispWrap(hdc,pDispPos);

				// 2006.04.29 Moca 選択処理のため縦線処理を追加
				GetTextDrawer().DispVerticalLines( hdc, pDispPos->GetDrawPos().y, pDispPos->GetDrawPos().y + nLineHeight,  CLayoutInt(0), CLayoutInt(-1) );
				if( bSelected ){
					/* テキスト反転 */
					DispTextSelected(
						hdc,
						pDispPos->GetLayoutLineRef(),
						CMyPoint(sDispPosBegin.GetDrawPos().x, pDispPos->GetDrawPos().y),
						pDispPos->GetDrawCol()
					);
				}
			}
		}
		// end of while( GetNPos() < nLineLen )

		if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){

			/* テキスト表示 */
			GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );

			/* EOF記号の表示 */
			if( pDispPos->GetLayoutLineRef() + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount()
				&& pDispPos->GetDrawCol() < nWrapKeta )
			{
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					//	May 29, 2004 genta (advised by MIK) 共通関数化
					GetTextDrawer().DispEOF(hdc,pDispPos);
				}
				bEOF = true;
			}

			if( IsBkBitmap() ){
			}else{
				// 行末背景描画
				RECT rcClip;
				if(GetTextArea().GenerateClipRectRight(&rcClip,*pDispPos)){
					cTextType.FillBack(hdc,rcClip);
				}
			}

			// 2006.04.29 Moca 選択処理のため縦線処理を追加
			GetTextDrawer().DispVerticalLines( hdc, pDispPos->GetDrawPos().y, pDispPos->GetDrawPos().y + nLineHeight,  CLayoutInt(0), CLayoutInt(-1) );

			if( bSelected ){
				/* テキスト反転 */
				DispTextSelected(
					hdc,
					pDispPos->GetLayoutLineRef(),
					CMyPoint(sDispPosBegin.GetDrawPos().x, pDispPos->GetDrawPos().y),
					pDispPos->GetDrawCol()
				);
			}

		}
end_of_line:;
		pDispPos->ForwardLayoutLineRef(1);
		pDispPos->ForwardDrawLine(1);
	}
	// NULL == pLineの場合
	else{
		if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
			if(GetTextDrawer().DispEmptyLine(hdc,pDispPos)){
				bEOF=true;
			}
		}
	}

end_of_func:;
//	2002/05/08 YAZAKI アンダーラインの再描画は不要でした
//	MYTRACE_A( "m_nOldUnderLineY=%d\n", m_nOldUnderLineY );
//	if( -1 != m_nOldUnderLineY ){
//		/* カーソル行アンダーラインのON */
//		CaretUnderLineON( TRUE );
//	}


	return bEOF;
}



// 　　　


/*[EOF]*/
