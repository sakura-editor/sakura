#include "stdafx.h"
#include "CEditView_Paint.h"
#include "view/CEditView.h"
#include <vector>
#include "types/CTypeSupport.h"
#include "doc/CLayout.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "CDrawStrategy.h"
#include "CDraw_Comment.h"
#include "CDraw_Quote.h"
#include "CDraw_RegexKeyword.h"
#include "CDraw_Space.h"
#include "CDraw_Found.h"

/*
	PAINT_LINENUMBER = (1<<0), //!< 行番号
	PAINT_RULER      = (1<<1), //!< ルーラー
	PAINT_BODY       = (1<<2), //!< 本文
*/

void CEditView_Paint::Call_OnPaint(
	int nPaintFlag,   //!< 描画する領域を選択する
	bool bUseMemoryDC //!< メモリDCを使用する
)
{
	CEditView* pView = GetEditView();

	//各要素
	CMyRect rcLineNumber(0,pView->GetTextArea().GetAreaTop(),pView->GetTextArea().GetAreaLeft(),pView->GetTextArea().GetAreaBottom());
	CMyRect rcRuler(pView->GetTextArea().GetAreaLeft(),0,pView->GetTextArea().GetAreaRight(),pView->GetTextArea().GetAreaTop());
	CMyRect rcBody(pView->GetTextArea().GetAreaLeft(),pView->GetTextArea().GetAreaTop(),pView->GetTextArea().GetAreaRight(),pView->GetTextArea().GetAreaBottom());

	//領域を作成 -> rc
	std::vector<CMyRect> rcs;
	if(nPaintFlag & PAINT_LINENUMBER)rcs.push_back(rcLineNumber);
	if(nPaintFlag & PAINT_RULER)rcs.push_back(rcRuler);
	if(nPaintFlag & PAINT_BODY)rcs.push_back(rcBody);
	if(rcs.size()==0)return;
	CMyRect rc=rcs[0];
	for(int i=1;i<(int)rcs.size();i++)
		rc=MergeRect(rc,rcs[i]);

	//描画
	PAINTSTRUCT	ps;
	ps.rcPaint = rc;
	HDC hdc = pView->GetDC();
	pView->OnPaint( hdc, &ps, bUseMemoryDC );
	pView->ReleaseDC( hdc );
}



/* フォーカス移動時の再描画

	@date 2001/06/21 asa-o 「スクロールバーの状態を更新する」「カーソル移動」削除
*/
void CEditView::RedrawAll()
{
	// ウィンドウ全体を再描画
	PAINTSTRUCT	ps;
	HDC hdc = ::GetDC( GetHwnd() );
	::GetClientRect( GetHwnd(), &ps.rcPaint );
	OnPaint( hdc, &ps, FALSE );
	::ReleaseDC( GetHwnd(), hdc );

	// キャレットの表示
	GetCaret().ShowEditCaret();

	// キャレットの行桁位置を表示する
	GetCaret().ShowCaretPosInfo();

	// 親ウィンドウのタイトルを更新
	m_pcEditWnd->UpdateCaption();

	//	Jul. 9, 2005 genta	選択範囲の情報をステータスバーへ表示
	GetSelectionInfo().PrintSelectionInfoMsg();

	// スクロールバーの状態を更新する
	AdjustScrollBars();
}

// 2001/06/21 Start by asa-o 再描画
void CEditView::Redraw()
{
	HDC			hdc;
	PAINTSTRUCT	ps;

	hdc = ::GetDC( GetHwnd() );

	::GetClientRect( GetHwnd(), &ps.rcPaint );

	OnPaint( hdc, &ps, FALSE );

	::ReleaseDC( GetHwnd(), hdc );
}
// 2001/06/21 End



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          色設定                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! 指定位置のColorIndexの取得
	CEditView::DrawLogicLineを元にしたためCEditView::DrawLogicLineに
	修正があった場合は、ここも修正が必要。

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
int CEditView::GetColorIndex(
	HDC						hdc,
	const CLayout*			pcLayout,
	int						nCol
)
{
	//	May 9, 2000 genta
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->m_cDocType.GetDocumentAttribute());

	int						nLineBgn;
	const CLayout*			pcLayout2;
	bool					bSearchFlg;			// 2002.02.08 hor
	int						nLineOffset;		// 2008/5/29 Uchi
	SColorInfo sInfo;
	SColorInfo* pInfo = &sInfo;
	sInfo.pcView = this;
	sInfo.bKeyWordTop = true;	//	Keyword Top


	pInfo->bSearchStringMode = FALSE;
	bSearchFlg	= true;	// 2002.02.08 hor

	CLogicInt	nSearchStart = CLogicInt(-1);
	CLogicInt	nSearchEnd   = CLogicInt(-1);

	/* 論理行データの取得 */
	if( pcLayout ){
		// 2002/2/10 aroka CMemory変更
		pInfo->nLineLen = pcLayout->GetDocLineRef()->GetLengthWithEOL()/* - pcLayout->GetLogicOffset()*/;	// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応
		pInfo->pLine = pcLayout->GetPtr()/* + pcLayout->GetLogicOffset()*/;			// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応

		// 2005.11.20 Moca 色が正しくないことがある問題に対処
		const CLayout* pcLayoutLineFirst = pcLayout;
		// 論理行の最初のレイアウト情報を取得する
		while( 0 != pcLayoutLineFirst->GetLogicOffset() ){
			pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
		}
		pInfo->nCOMMENTMODE = pcLayoutLineFirst->GetColorTypePrev();
		pInfo->nCOMMENTEND = 0;
		pcLayout2 = pcLayout;

		nLineOffset = pcLayout->GetLogicOffset();	// 2008/5/29 Uchi
	}
	else{
		pInfo->pLine = NULL;
		pInfo->nLineLen = CLogicInt(0);
		pInfo->nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		pInfo->nCOMMENTEND = 0;
		pcLayout2 = NULL;
		nLineOffset = 0;							// 2008/5/29 Uchi
	}

	/* 現在の色を指定 */
	//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
	pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai

	pInfo->nBgn = 0;
	pInfo->nPos = CLogicInt(0);
	nLineBgn = 0;

	CLogicInt				nCharChars = CLogicInt(0);
	CLogicInt				nCharChars_2; //##############################後で消す

	if( NULL != pInfo->pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		while( pInfo->nPos <= nCol ){	// 03/10/24 ai 行頭のColorIndexが取得できない問題に対応

			pInfo->nBgn = pInfo->nPos;
			nLineBgn = pInfo->nBgn;

			while( pInfo->nPos - nLineBgn <= nCol ){	// 02/12/18 ai
				/* 検索文字列の色分け */
				if( m_bCurSrchKeyMark	/* 検索文字列のマーク */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
				// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
					if(!pInfo->bSearchStringMode && (!m_sCurSearchOption.bRegularExp || (bSearchFlg && nSearchStart < pInfo->nPos))){
						bSearchFlg=IsSearchString( pInfo->pLine, pInfo->nLineLen, pInfo->nPos, &nSearchStart, &nSearchEnd );
					}
					if( !pInfo->bSearchStringMode && bSearchFlg && nSearchStart==pInfo->nPos ){
						pInfo->nBgn = pInfo->nPos;
						pInfo->bSearchStringMode = TRUE;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
						pInfo->nColorIndex = COLORIDX_SEARCH;	// 02/12/18 ai
					}
					else if( pInfo->bSearchStringMode && nSearchEnd == pInfo->nPos ){
						pInfo->nBgn = pInfo->nPos;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
						pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
						pInfo->bSearchStringMode = FALSE;
						goto searchnext;
					}
				}

				if( pInfo->nPos >= pInfo->nLineLen - pcLayout2->GetLayoutEol().GetLen() ){
					goto end_of_line;
				}
SEARCH_START:;
				switch( pInfo->nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
					if( CDraw_RegexKeyword().GetColorIndexImp(&sInfo) ) { }	//正規表現キーワード
					else if( CDraw_LineComment().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_BlockComment().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_BlockComment2().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_SingleQuote().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_DoubleQuote().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_URL().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_Numeric().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_KeywordSet().GetColorIndexImp(&sInfo) ) { }

					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pInfo->pLine[pInfo->nPos - nLineOffset] ))	pInfo->bKeyWordTop = false;		// 2008/5/29 Uchi
					else								pInfo->bKeyWordTop = true;
					//	To Here
					break;
// 2002/03/13 novice
				case COLORIDX_URL:		/* URLモードである */
				case COLORIDX_KEYWORD1:	/* 強調キーワード1 */
				case COLORIDX_DIGIT:	/* 半角数値である */  //@@@ 2001.02.17 by MIK
				case COLORIDX_KEYWORD2:	/* 強調キーワード2 */	//MIK
				case COLORIDX_KEYWORD3:
				case COLORIDX_KEYWORD4:
				case COLORIDX_KEYWORD5:
				case COLORIDX_KEYWORD6:
				case COLORIDX_KEYWORD7:
				case COLORIDX_KEYWORD8:
				case COLORIDX_KEYWORD9:
				case COLORIDX_KEYWORD10:
					if( pInfo->nPos == pInfo->nCOMMENTEND ){
						pInfo->nBgn = pInfo->nPos;
						pInfo->nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !pInfo->bSearchStringMode ){
							//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
							pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* コントロールコード */ // 2002/03/13 novice
					if( pInfo->nPos == pInfo->nCOMMENTEND ){
						pInfo->nBgn = pInfo->nPos;
						pInfo->nCOMMENTMODE = pInfo->nCOMMENTMODE_OLD;
						pInfo->nCOMMENTEND = pInfo->nCOMMENTEND_OLD;
						/* 現在の色を指定 */
						if( !pInfo->bSearchStringMode ){
							//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
							pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;

				case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
					if( CDraw_BlockComment().GetColorIndexImpEnd(pInfo) ){
						goto SEARCH_START;
					}
					break;
				case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
					if( CDraw_BlockComment2().GetColorIndexImpEnd(pInfo) ){
						goto SEARCH_START;
					}
					break;
				case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
					if( CDraw_SingleQuote().GetColorIndexImpEnd(pInfo) ){
						goto SEARCH_START;
					}
					break;
				case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
					if( CDraw_DoubleQuote().GetColorIndexImpEnd(pInfo) ){
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( pInfo->nCOMMENTMODE >= 1000 && pInfo->nCOMMENTMODE <= 1099 ){	//正規表現キーワード1～10
						if( pInfo->nPos == pInfo->nCOMMENTEND ){
							pInfo->nBgn = pInfo->nPos;
							pInfo->nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* 現在の色を指定 */
							if( !pInfo->bSearchStringMode ){
								//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
								pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}
				if( pInfo->pLine[pInfo->nPos - nLineOffset] == WCODE::TAB ){		// 2008/5/29 Uchi
					pInfo->nBgn = pInfo->nPos + 1;
					nCharChars = CLogicInt(1);
				}
				else if( WCODE::IsZenkakuSpace(pInfo->pLine[pInfo->nPos - nLineOffset]) && (pInfo->nCOMMENTMODE < 1000 || pInfo->nCOMMENTMODE > 1099) )	//@@@ 2002.01.04	// 2008/5/29 Uchi
				{
					pInfo->nBgn = pInfo->nPos + 1;
					nCharChars = CLogicInt(1);
				}
				//半角空白（半角スペース）を表示 2002.04.28 Add by KK 
				else if (pInfo->pLine[pInfo->nPos - nLineOffset] == L' ' && CTypeSupport(this,COLORIDX_SPACE).IsDisp() && (pInfo->nCOMMENTMODE < 1000 || pInfo->nCOMMENTMODE > 1099) )		// 2008/5/29 Uchi
				{
					pInfo->nBgn = pInfo->nPos + 1;
					nCharChars = CLogicInt(1);
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, pInfo->nPos );
					if( 0 == nCharChars ){
						nCharChars = CLogicInt(1);
					}
					if( !pInfo->bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != pInfo->nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* コントロールコードを色分け */
					 && WCODE::IsControlCode(pInfo->pLine[pInfo->nPos - nLineOffset])		// 2008/5/29 Uchi
					){
						pInfo->nBgn = pInfo->nPos;
						pInfo->nCOMMENTMODE_OLD = pInfo->nCOMMENTMODE;
						pInfo->nCOMMENTEND_OLD = pInfo->nCOMMENTEND;
						pInfo->nCOMMENTMODE = COLORIDX_CTRLCODE;	/* コントロールコード モード */ // 2002/03/13 novice
						/* コントロールコード列の終端を探す */
						int i;
						for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if(!WCODE::IsControlCode(pInfo->pLine[i - nLineOffset])){		// 2008/5/29 Uchi
								break;
							}
						}
						pInfo->nCOMMENTEND = i;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
						pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
					}
				}
				pInfo->nPos+= nCharChars;
			} //end of while( pInfo->nPos - nLineBgn < pcLayout2->m_nLength ){
			if( pInfo->nPos > nCol ){	// 03/10/24 ai 行頭のColorIndexが取得できない問題に対応
				break;
			}
		}

end_of_line:;

	}

//@end_of_func:;
	return pInfo->nColorIndex;
}


/* 現在の色を指定 */
void CEditView::SetCurrentColor( HDC hdc, int nCOMMENTMODE )
{
	int				nColorIdx;
	COLORREF		colText;
	COLORREF		colBack;

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
			colText = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* フォントを選ぶ */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				GetFontset().ChooseFontHandle(
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
			return;
		}
		break;
//@@@ 2001.11.17 add end MIK

	}

	if( -1 != nColorIdx ){
		if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bDisp ){
			colText = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* フォントを選ぶ */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				GetFontset().ChooseFontHandle(
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
		}
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           描画                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! 通常の描画処理 new 
	@param pPs  pPs.rcPaint は正しい必要がある
	@param bDrawFromComptibleBmp  TRUE 画面バッファからhdcに作画する(コピーするだけ)。
			TRUEの場合、pPs.rcPaint領域外は作画されないが、FALSEの場合は作画される事がある。
			互換DC/BMPが無い場合は、普通の作画処理をする。
@date 2007.09.09 Moca 元々無効化されていた第三パラメータのbUseMemoryDCをbDrawFromComptibleBmpに変更。
*/
void CEditView::OnPaint( HDC hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp )
{
//	MY_RUNNINGTIMER( cRunningTimer, "CEditView::OnPaint" );

	// 2004.01.28 Moca デスクトップに作画しないように
	if( NULL == hdc )return;

	if( !GetDrawSwitch() )return;

	//@@@
#ifdef _DEBUG
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
		if ( m_pcEditWnd->m_nActivePaneIndex == m_nMyIndex ){
			/* アクティブペインは、アンダーライン描画 */
			GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
		}
		return;
	}
	if( m_hdcCompatDC && NULL == m_hbmpCompatBMP
		 || (pPs->rcPaint.right - pPs->rcPaint.left) < m_nCompatBMPWidth
		 || (pPs->rcPaint.bottom - pPs->rcPaint.top) < m_nCompatBMPHeight ){
		RECT rect;
		::GetWindowRect( this->GetHwnd(), &rect );
		CreateOrUpdateCompatibleBitmap( rect.right - rect.left, rect.bottom - rect.top );
	}
	// To Here 2007.09.09 Moca

	// キャレットを隠す
	GetCaret().HideCaret_( this->GetHwnd() ); // 2002/07/22 novice

	//	May 9, 2000 genta
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->m_cDocType.GetDocumentAttribute());

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	RECT			rc;
	int				nLineHeight = GetTextMetrics().GetHankakuDy();
	int				nCharDx = GetTextMetrics().GetHankakuDx();

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
		rc.right  = GetTextArea().GetAreaLeft() - GetDllShareData().m_Common.m_sWindow.m_nLineNumRightSpace; //	Sep. 23 ,2002 genta 余白はテキスト色のまま残す
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



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//           描画開始レイアウト絶対行 -> nLayoutLine             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt nLayoutLine;
	if( 0 > nTop - GetTextArea().GetAreaTop() ){
		nLayoutLine = GetTextArea().GetViewTopLine(); //ビュー上部から描画
	}else{
		nLayoutLine = GetTextArea().GetViewTopLine() + CLayoutInt( ( nTop - GetTextArea().GetAreaTop() ) / nLineHeight ); //ビュー途中から描画
	}

	int nMaxRollBackLineNum = 260 / (Int)nWrapKeta + 1;
	int nRollBackLineNum = 0;
	const CLayout*	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLayoutLine );
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
	//          描画終了レイアウト絶対行 -> nLayoutLineTo            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt nLayoutLineTo = GetTextArea().GetViewTopLine()
		+ CLayoutInt( ( pPs->rcPaint.bottom - GetTextArea().GetAreaTop() ) / nLineHeight );


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
	while(sPos.GetLayoutLineRef() <= nLayoutLineTo)
	{
		//描画X位置リセット
		sPos.ResetDrawCol();

		//1行描画
		bool bDispResult = DrawLogicLine(
			hdc,
			&sPos,
			nLayoutLineTo
		);

		if(bDispResult){
			pPs->rcPaint.bottom += nLineHeight;	// EOF再描画対応
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

	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	//     アンダーライン描画をメモリDCからのコピー前処理から後に移動
	if ( m_pcEditWnd->m_nActivePaneIndex == m_nMyIndex ){
		/* アクティブペインは、アンダーライン描画 */
		GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
	}
	// To Here 2007.09.09 Moca

	/* 03/02/18 対括弧の強調表示(描画) ai */
	DrawBracketPair( true );

	/* キャレットを現在位置に表示します */
	GetCaret().ShowCaret_( this->GetHwnd() ); // 2002/07/22 novice
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
	1回で1ロジック行分を作画する。

	@return EOFを作画したらtrue
*/
bool CEditView::DrawLogicLine(
	HDC				_hdc,			//!< [in]     作画対象
	DispPos*		_pDispPos,		//!< [in/out] 描画する箇所、描画元ソース
	CLayoutInt		nLineTo			//!< [in]     作画終了するレイアウト行番号
)
{
//	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DrawLogicLine" );
	SDrawStrategyInfo sInfo;
	SDrawStrategyInfo* pInfo = &sInfo;
	pInfo->hdc = _hdc;
	pInfo->pDispPos = _pDispPos;
	pInfo->pcView = this;

	// 表示を開始するレイアウト
	const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );
//	if( !pcLayout ){
//		return true;
//	}


	//DispPosを保存しておく
	pInfo->sDispPosBegin = *pInfo->pDispPos;

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();


	//サイズ
	STypeConfig* TypeDataPtr = &m_pcEditDoc->m_cDocType.GetDocumentAttribute();
	int nLineHeight = GetTextMetrics().GetHankakuDy();  //行の縦幅？
	int nCharDx  = GetTextMetrics().GetHankakuDx();  //半角

	pInfo->bSearchStringMode = false; //☆開始 //検索ヒットフラグ？これで色分けを判定する
	pInfo->bSearchFlg        = true;  //☆開始 //？ 2002.02.08 hor
	pInfo->nSearchStart      = CLogicInt(-1); //☆開始   //？ 2002.02.08 hor
	pInfo->nSearchEnd        = CLogicInt(-1); //☆開始   //？ 2002.02.08 hor

	//処理する文字位置
	pInfo->nPos = CLogicInt(0); //☆開始
	#define SetNPos(N) pInfo->nPos=(N)
	#define GetNPos() (pInfo->nPos+CLogicInt(0))

	//通常テキスト開始位置 (ほぼ固定)
	pInfo->nBgn = 0; //☆開始
	#define SetNBgn(N) pInfo->nBgn=(N)
	#define GetNBgn() (pInfo->nBgn+0)

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          論理行データの取得 -> pLine, pLineLen              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if( pcLayout ){
		// 2002/2/10 aroka CMemory変更
		pInfo->nLineLen = pcLayout->GetDocLineRef()->GetLengthWithEOL() - pcLayout->GetLogicOffset();
		pInfo->pLine    = pcLayout->GetDocLineRef()->GetPtr() + pcLayout->GetLogicOffset();
	}
	else{
		pInfo->pLine = NULL;
		pInfo->nLineLen = CLogicInt(0);
	}

	// 前行の最終設定色
	pInfo->ChangeColor(pcLayout?pcLayout->GetColorTypePrev():COLORIDX_TEXT);
	pInfo->nCOMMENTEND = 0; //☆開始

	//サポート
	CTypeSupport cTextType(this,COLORIDX_TEXT);
	CTypeSupport cSearchType(this,COLORIDX_SEARCH);

	//状態変数
	bool bEOF = false;
	pInfo->bKeyWordTop = true;	//☆開始	//	Keyword Top
	pInfo->nCharChars = CLogicInt(0); //☆開始	//処理した文字数

	if( pInfo->pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		pInfo->pDispPos->ForwardDrawLine(-1);
		pInfo->pDispPos->ForwardLayoutLineRef(-1);

		while( GetNPos() < pInfo->nLineLen ){
			pInfo->pDispPos->ForwardDrawLine(1);
			pInfo->pDispPos->ForwardLayoutLineRef(1);
			if( GetTextArea().GetBottomLine() < pInfo->pDispPos->GetLayoutLineRef() ){
				pInfo->pDispPos->SetLayoutLineRef(nLineTo + CLayoutInt(1));
				goto end_of_func;
			}
			if( nLineTo < pInfo->pDispPos->GetLayoutLineRef() ){
				goto end_of_func;
			}

			//レイアウト行を1行描画
			bool bDrawLayoutLine = DrawLayoutLine(pInfo);
			if(bDrawLayoutLine)goto end_of_line;
		}
		// end of while( GetNPos() < nLineLen )

		if( pInfo->pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){

			/* テキスト表示 */
			GetTextDrawer().DispText(
				pInfo->hdc, pInfo->pDispPos, &pInfo->pLine[GetNBgn()], GetNPos() - GetNBgn() );

			/* EOF記号の表示 */
			if( pInfo->pDispPos->GetLayoutLineRef() + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount()
				&& pInfo->pDispPos->GetDrawCol() < nWrapKeta )
			{
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					//	May 29, 2004 genta (advised by MIK) 共通関数化
					GetTextDrawer().DispEOF(pInfo->hdc,pInfo->pDispPos);
				}
				bEOF = true;
			}

			if( !IsBkBitmap() ){
				// 行末背景描画
				RECT rcClip;
				if(GetTextArea().GenerateClipRectRight(&rcClip,*pInfo->pDispPos)){
					cTextType.FillBack(pInfo->hdc,rcClip);
				}
			}

			// 2006.04.29 Moca 選択処理のため縦線処理を追加
			GetTextDrawer().DispVerticalLines(
				pInfo->hdc,
				pInfo->pDispPos->GetDrawPos().y,
				pInfo->pDispPos->GetDrawPos().y + nLineHeight,
				CLayoutInt(0),
				CLayoutInt(-1)
			);

			if( GetSelectionInfo().IsTextSelected() ){
				/* テキスト反転 */
				DispTextSelected(
					pInfo->hdc,
					pInfo->pDispPos->GetLayoutLineRef(),
					CMyPoint(pInfo->sDispPosBegin.GetDrawPos().x, pInfo->pDispPos->GetDrawPos().y),
					pInfo->pDispPos->GetDrawCol()
				);
			}

		}
end_of_line:;
		pInfo->pDispPos->ForwardLayoutLineRef(1);
		pInfo->pDispPos->ForwardDrawLine(1);
	}
	// NULL == pLineの場合
	else{
		if( pInfo->pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
			if(GetTextDrawer().DispEmptyLine(pInfo->hdc,pInfo->pDispPos)){
				bEOF=true;
			}
		}
		return true;
	}

end_of_func:;


	return bEOF;
}

/*!
	レイアウト行を1行描画
*/
bool CEditView::DrawLayoutLine(SDrawStrategyInfo* pInfo)
{
	CTypeSupport cTextType(this,COLORIDX_TEXT);
	CTypeSupport cSearchType(this,COLORIDX_SEARCH);

	// コンフィグ
	int nLineHeight = GetTextMetrics().GetHankakuDy();  //行の縦幅？
	STypeConfig* TypeDataPtr = &m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	const CLayout*	pcLayout2; //ワーク用CLayoutポインタ
	pcLayout2 = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        行番号描画                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if( pInfo->pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
		// 行番号表示
		pInfo->pcView->GetTextDrawer().DispLineNumber(
			pInfo->hdc,
			pcLayout2,
			(Int)pInfo->pDispPos->GetLayoutLineRef(),
			pInfo->pDispPos->GetDrawPos().y
		);
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       本文描画開始                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pInfo->nBgn = pInfo->nPos;
	int nLineBgn = pInfo->nBgn;
	pInfo->pDispPos->ResetDrawCol();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                 行頭(インデント)背景描画                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if(pcLayout2 && pcLayout2->GetIndent()!=0)
	{
		RECT rcClip;
		if(GetTextArea().GenerateClipRect(&rcClip,*pInfo->pDispPos,(Int)pcLayout2->GetIndent())){
			cTextType.FillBack(pInfo->hdc,rcClip);
		}
		//描画位置進める
		pInfo->pDispPos->ForwardDrawCol((Int)pcLayout2->GetIndent());
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                 通常文字列以外描画ループ                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//行終端または折り返しに達するまでループ
	while( pInfo->nPos - nLineBgn < pcLayout2->GetLengthWithEOL() ){
		// マッチ文字列
		CDraw_Found().BeginColor(pInfo);

		//行末
		if(CDraw_Line().EndColor(pInfo)){
			return true;
		}

SEARCH_START:;
		switch( pInfo->nCOMMENTMODE ){
		case COLORIDX_TEXT:
			//色の開始判定
			if( CDraw_RegexKeyword().BeginColor(pInfo) ){ }			// 正規表現キーワード
			else if( CDraw_LineComment().BeginColor(pInfo) ){ }		// 行コメント
			else if( CDraw_BlockComment().BeginColor(pInfo) ){ }	// ブロックコメント
			else if( CDraw_BlockComment2().BeginColor(pInfo) ){ }	// ブロックコメント2
			else if( CDraw_SingleQuote().BeginColor(pInfo) ){ }		// シングルクォーテーション文字列
			else if( CDraw_DoubleQuote().BeginColor(pInfo) ){ }		// ダブルクォーテーション文字列
			else if( CDraw_URL().BeginColor(pInfo) ){ }				// URL
			else if( CDraw_Numeric().BeginColor(pInfo) ){ }			// 半角数字
			else if( CDraw_KeywordSet().BeginColor(pInfo) ){ }		// キーワードセット
			
			//単語判定
			if( IS_KEYWORD_CHAR( pInfo->pLine[pInfo->nPos] ))	pInfo->bKeyWordTop = false;
			else												pInfo->bKeyWordTop = true;

			break;

		//色の終了判定
		case COLORIDX_URL:		// URL
		case COLORIDX_DIGIT:	// 半角数値
		case COLORIDX_KEYWORD1:	// 強調キーワード1-10
		case COLORIDX_KEYWORD2:
		case COLORIDX_KEYWORD3:
		case COLORIDX_KEYWORD4:
		case COLORIDX_KEYWORD5:
		case COLORIDX_KEYWORD6:
		case COLORIDX_KEYWORD7:
		case COLORIDX_KEYWORD8:
		case COLORIDX_KEYWORD9:
		case COLORIDX_KEYWORD10:
			if( CDraw_ColorEnd().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;
		case COLORIDX_CTRLCODE:	/* コントロールコード */ // 2002/03/13 novice
			if( CDraw_CtrlCode().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;

		case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
			break;
		case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
			if( CDraw_BlockComment().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;
		case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
			if( CDraw_BlockComment2().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;
		case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
			if( CDraw_SingleQuote().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;
		case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
			if( CDraw_DoubleQuote().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;
		default:	//@@@ 2002.01.04 add start
			if( pInfo->nCOMMENTMODE >= 1000 && pInfo->nCOMMENTMODE <= 1099 ){	//正規表現キーワード1～10
				if( CDraw_RegexKeyword().EndColor(pInfo) ){
					goto SEARCH_START;
				}
			}
			break;	//@@@ 2002.01.04 add end
		}

		//タブ
		if( CDraw_Tab().BeginColor(pInfo) ){ }
		//全角スペース
		else if( CDraw_ZenSpace().BeginColor(pInfo) ){ }
		//半角空白（半角スペース）を表示 2002.04.28 Add by KK 
		else if( CDraw_HanSpace().BeginColor(pInfo) ){ }
		//コントロールコード 2008.04.20 kobake
		else if( CDraw_CtrlCode().BeginColor(pInfo) ){ }
		//その他
		else{
			pInfo->nCharChars = CLogicInt(1);
		}
		pInfo->nPos += pInfo->nCharChars;
	}
	//end of while( pInfo->nPos - nLineBgn < pcLayout2->GetLength() )


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         本文描画                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if( pInfo->nPos >= pInfo->nLineLen ){
		return false; //これで良い
	}
	if( pInfo->DrawToHere() ){
		/* 行末背景描画 */
		RECT rcClip;
		if(pInfo->pcView->GetTextArea().GenerateClipRectRight(&rcClip,*pInfo->pDispPos)){
			cTextType.FillBack(pInfo->hdc,rcClip);
		}

		/* 折り返し記号を表示する */
		pInfo->pcView->GetTextDrawer().DispWrap(pInfo->hdc,pInfo->pDispPos);

		// 2006.04.29 Moca 選択処理のため縦線処理を追加
		pInfo->pcView->GetTextDrawer().DispVerticalLines( pInfo->hdc, pInfo->pDispPos->GetDrawPos().y, pInfo->pDispPos->GetDrawPos().y + nLineHeight,  CLayoutInt(0), CLayoutInt(-1) );
		if( GetSelectionInfo().IsTextSelected() ){
			/* テキスト反転 */
			DispTextSelected(
				pInfo->hdc,
				pInfo->pDispPos->GetLayoutLineRef(),
				CMyPoint(pInfo->sDispPosBegin.GetDrawPos().x, pInfo->pDispPos->GetDrawPos().y),
				pInfo->pDispPos->GetDrawCol()
			);
		}
	}
	return false;
}








/* テキスト反転

	@param hdc      
	@param nLineNum 
	@param x        
	@param y        
	@param nX       

	@note
	CCEditView::DrawLogicLine() での作画(WM_PAINT)時に、1レイアウト行をまとめて反転処理するための関数。
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







// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       画面バッファ                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*!
	画面の互換ビットマップを作成または更新する。
		必要の無いときは何もしない。
	
	@param cx ウィンドウの高さ
	@param cy ウィンドウの幅
	@return true: ビットマップを利用可能 / false: ビットマップの作成・更新に失敗

	@date 2007.09.09 Moca CEditView::OnSizeから分離。
		単純に生成するだけだったものを、仕様変更に従い内容コピーを追加。
		サイズが同じときは何もしないように変更

	@par 互換BMPにはキャレット・カーソル位置横縦線・対括弧以外の情報を全て書き込む。
		選択範囲変更時の反転処理は、画面と互換BMPの両方を別々に変更する。
		カーソル位置横縦線変更時には、互換BMPから画面に元の情報を復帰させている。

*/
bool CEditView::CreateOrUpdateCompatibleBitmap( int cx, int cy )
{
	if( NULL == m_hdcCompatDC ){
		return false;
	}
	// サイズを64の倍数で整列
	int nBmpWidthNew  = ((cx + 63) & (0x7fffffff - 63));
	int nBmpHeightNew = ((cy + 63) & (0x7fffffff - 63));
	if( nBmpWidthNew != m_nCompatBMPWidth || nBmpHeightNew != m_nCompatBMPHeight ){
#ifdef _DEBUG
	MYTRACE( _T("CEditView::CreateOrUpdateCompatibleBitmap( %d, %d ): resized\n"), cx, cy );
#endif
		HDC	hdc = ::GetDC( GetHwnd() );
		HBITMAP hBitmapNew = NULL;
		if( m_hbmpCompatBMP ){
			// BMPの更新
			HDC hdcTemp = ::CreateCompatibleDC( hdc );
			hBitmapNew = ::CreateCompatibleBitmap( hdc, nBmpWidthNew, nBmpHeightNew );
			if( hBitmapNew ){
				HBITMAP hBitmapOld = (HBITMAP)::SelectObject( hdcTemp, hBitmapNew );
				// 前の画面内容をコピーする
				::BitBlt( hdcTemp, 0, 0,
					__min( nBmpWidthNew,m_nCompatBMPWidth ),
					__min( nBmpHeightNew, m_nCompatBMPHeight ),
					m_hdcCompatDC, 0, 0, SRCCOPY );
				::SelectObject( hdcTemp, hBitmapOld );
				::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
				::DeleteObject( m_hbmpCompatBMP );
			}
			::DeleteDC( hdcTemp );
		}else{
			// BMPの新規作成
			hBitmapNew = ::CreateCompatibleBitmap( hdc, nBmpWidthNew, nBmpHeightNew );
		}
		if( hBitmapNew ){
			m_hbmpCompatBMP = hBitmapNew;
			m_nCompatBMPWidth = nBmpWidthNew;
			m_nCompatBMPHeight = nBmpHeightNew;
			m_hbmpCompatBMPOld = (HBITMAP)::SelectObject( m_hdcCompatDC, m_hbmpCompatBMP );
		}else{
			// 互換BMPの作成に失敗
			// 今後も失敗を繰り返す可能性が高いので
			// m_hdcCompatDCをNULLにすることで画面バッファ機能をこのウィンドウのみ無効にする。
			//	2007.09.29 genta 関数化．既存のBMPも解放
			UseCompatibleDC(FALSE);
		}
		::ReleaseDC( GetHwnd(), hdc );
	}
	return NULL != m_hbmpCompatBMP;
}


/*!
	互換メモリBMPを削除

	@note 分割ビューが非表示になった場合と
		親ウィンドウが非表示・最小化された場合に削除される。
	@date 2007.09.09 Moca 新規作成 
*/
void CEditView::DeleteCompatibleBitmap()
{
	if( m_hbmpCompatBMP ){
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
		::DeleteObject( m_hbmpCompatBMP );
		m_hbmpCompatBMP = NULL;
		m_hbmpCompatBMPOld = NULL;
		m_nCompatBMPWidth = -1;
		m_nCompatBMPHeight = -1;
	}
}



/** 画面キャッシュ用CompatibleDCを用意する

	@param[in] TRUE: 画面キャッシュON

	@date 2007.09.30 genta 関数化
*/
void CEditView::UseCompatibleDC(BOOL fCache)
{
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	if( fCache ){
		if( m_hdcCompatDC == NULL ){
			HDC			hdc;
			hdc = ::GetDC( GetHwnd() );
			m_hdcCompatDC = ::CreateCompatibleDC( hdc );
			::ReleaseDC( GetHwnd(), hdc );
			DEBUG_TRACE(_T("CEditView::UseCompatibleDC: Created\n"), fCache);
		}
		else {
			DEBUG_TRACE(_T("CEditView::UseCompatibleDC: Reused\n"), fCache);
		}
	}
	else {
		//	CompatibleBitmapが残っているかもしれないので最初に削除
		DeleteCompatibleBitmap();
		if( m_hdcCompatDC != NULL ){
			::DeleteDC( m_hdcCompatDC );
			DEBUG_TRACE(_T("CEditView::UseCompatibleDC: Deleted.\n"));
			m_hdcCompatDC = NULL;
		}
	}
}
