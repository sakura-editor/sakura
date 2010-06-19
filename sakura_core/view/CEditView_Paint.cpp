#include "stdafx.h"
#include "CEditView_Paint.h"
#include <vector>
#include <memory> // auto_ptr
#include "view/CEditView.h"
#include "types/CTypeSupport.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "doc/CLayout.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "view/colors/CColorStrategy.h"
#include "view/colors/CColor_Found.h"
#include "view/figures/CFigureStrategy.h"

void _DispWrap(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);

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
EColorIndexType CEditView::GetColorIndex(
	const CLayout*			pcLayout,
	int						nIndex,
	bool					bPrev,			// 指定位置の色変更直前まで	2010.06.19 ryoji 追加
	CColorStrategy**		ppStrategy,		// 2010.03.31 ryoji 追加
	CColorStrategy**		ppStrategyFound	// 2010.03.31 ryoji 追加
)
{
	EColorIndexType eRet = COLORIDX_TEXT;

	if(!pcLayout){
		return COLORIDX_TEXT;
	}

	//	May 9, 2000 genta
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->m_cDocType.GetDocumentAttribute());

	/* 論理行データの取得 */
	DispPos _sPos(0,0);
	SColorStrategyInfo _sInfo;
	SColorStrategyInfo* pInfo = &_sInfo;
	pInfo->pcView = this;
	pInfo->pDispPos=&_sPos;
	{
		// 2002/2/10 aroka CMemory変更
		pInfo->pLineOfLogic = pcLayout->GetDocLineRef()->GetPtr();

		// 論理行の最初のレイアウト情報を取得 -> pcLayoutLineFirst
		const CLayout* pcLayoutLineFirst = pcLayout;
		while( 0 != pcLayoutLineFirst->GetLogicOffset() ){
			pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();

			// 論理行の先頭まで戻らないと確実には正確な色は得られない
			// （正規表現キーワードにマッチした長い強調表示がその位置のレイアウト行頭をまたいでいる場合など）
			//if( pcLayout->GetLogicOffset() - pcLayoutLineFirst->GetLogicOffset() > 260 )
			//	break;
		}

		// 2005.11.20 Moca 色が正しくないことがある問題に対処
		eRet = pcLayoutLineFirst->GetColorTypePrev();	/* 現在の色を指定 */	// 02/12/18 ai
		pInfo->nPosInLogic = pcLayoutLineFirst->GetLogicOffset();

		//CColorStrategyPool初期化
		CColorStrategyPool* pool = CColorStrategyPool::Instance();
		pool->NotifyOnStartScanLogic();


		// 2009.02.07 ryoji この関数では pInfo->DoChangeColor() で色を調べるだけなので以下の処理は不要
		//
		////############超仮。本当はVisitorを使うべき
		//class TmpVisitor{
		//public:
		//	static int CalcLayoutIndex(const CLayout* pcLayout)
		//	{
		//		int n = -1;
		//		while(pcLayout){
		//			pcLayout = pcLayout->GetPrevLayout(); //prev or null
		//			n++;
		//		}
		//		return n;
		//	}
		//};
		//pInfo->pDispPos->SetLayoutLineRef(CLayoutInt(TmpVisitor::CalcLayoutIndex(pcLayout)));
	}

	//@@@ 2001.11.17 add start MIK
	if( TypeDataPtr->m_bUseRegexKeyword )
	{
		m_cRegexKeyword->RegexKeyLineStart();
	}
	//@@@ 2001.11.17 add end MIK

	//文字列参照
	const CDocLine* pcDocLine = pcLayout->GetDocLineRef();
	CStringRef cLineStr(pcDocLine->GetPtr(),pcDocLine->GetLengthWithEOL());

	//color strategy
	CColorStrategyPool* pool = CColorStrategyPool::Instance();
	pInfo->pStrategy = pool->GetStrategyByColor(eRet);
	if(pInfo->pStrategy)pInfo->pStrategy->InitStrategyStatus();

	int nPosTo = pcLayout->GetLogicOffset() + __min(nIndex, pcLayout->GetLengthWithEOL() - 1);
	while(pInfo->nPosInLogic <= nPosTo){
		if( bPrev && pInfo->nPosInLogic == nPosTo )
			break;

		//色切替
		pInfo->DoChangeColor(cLineStr);

		//1文字進む
		pInfo->nPosInLogic += CNativeW::GetSizeOfChar(
									cLineStr.GetPtr(),
									cLineStr.GetLength(),
									pInfo->nPosInLogic
								);
	}
	eRet = pInfo->GetCurrentColor();
	if(ppStrategy) *ppStrategy = pInfo->pStrategy;
	if(ppStrategyFound) *ppStrategyFound = pInfo->pStrategyFound;

	return eRet;
}


/* 現在の色を指定 */
void CEditView::SetCurrentColor( CGraphics& gr, EColorIndexType eColorIndex )
{
	//インデックス決定
	int		nColorIdx = ToColorInfoArrIndex(eColorIndex);

	//実際に色を設定
	if( -1 != nColorIdx ){
		const ColorInfo& info = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx];
		gr.SetForegroundColor(info.m_colTEXT);
		gr.SetBackgroundColor(info.m_colBACK);
		gr.SetMyFont(
			GetFontset().ChooseFontHandle(
				info.m_bFatFont,
				info.m_bUnderLine
			)
		);
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
	@date 2009.03.26 ryoji 行番号のみ描画を通常の行描画と分離（効率化）
*/
void CEditView::OnPaint( HDC _hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp )
{
//	MY_RUNNINGTIMER( cRunningTimer, "CEditView::OnPaint" );
	CGraphics gr(_hdc);

	// 2004.01.28 Moca デスクトップに作画しないように
	if( NULL == gr )return;

	if( !GetDrawSwitch() )return;
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
			gr,
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
	bool bCaretShowFlag_Old = GetCaret().GetCaretShowFlag();	// 2008.06.09 ryoji
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
		hdcOld = gr;
		gr = m_hdcCompatDC;
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

		cTextType.FillBack(gr,rc);
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
		::FillRect( gr, &rc, hBrush );
		::DeleteObject( hBrush );
	}
	//	To Here Sep. 7, 2001 genta

	::SetBkMode( gr, TRANSPARENT );

	cTextType.SetGraphicsState_WhileThisObj(gr);


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

	// ※ ここにあった描画範囲の 260 文字ロールバック処理は GetColorIndex() に吸収	// 2009.02.11 ryoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          描画終了レイアウト絶対行 -> nLayoutLineTo            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt nLayoutLineTo = GetTextArea().GetViewTopLine()
		+ CLayoutInt( ( pPs->rcPaint.bottom - GetTextArea().GetAreaTop() + (nLineHeight - 1) ) / nLineHeight ) - 1;	// 2007.02.17 ryoji 計算を精密化


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

	//必要な行を描画する	// 2009.03.26 ryoji 行番号のみ描画を通常の行描画と分離（効率化）
	if(pPs->rcPaint.right <= GetTextArea().GetAreaLeft()){
		while(sPos.GetLayoutLineRef() <= nLayoutLineTo)
		{
			if(!sPos.GetLayoutRef())
				break;

			//1行描画（行番号のみ）
			GetTextDrawer().DispLineNumber(
				gr,
				sPos.GetLayoutLineRef(),
				sPos.GetDrawPos().y
			);
			//行を進める
			sPos.ForwardDrawLine(1);		//描画Y座標＋＋
			sPos.ForwardLayoutLineRef(1);	//レイアウト行＋＋
		}
	}else{
		while(sPos.GetLayoutLineRef() <= nLayoutLineTo)
		{
			//描画X位置リセット
			sPos.ResetDrawCol();

			//1行描画
			bool bDispResult = DrawLogicLine(
				gr,
				&sPos,
				nLayoutLineTo
			);

			if(bDispResult){
				pPs->rcPaint.bottom += nLineHeight;	// EOF再描画対応
				break;
			}
		}
	}

	gr.ClearMyFont();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              テキストの無い部分の塗りつぶし                 //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if( IsBkBitmap() ){
	}
	else{
		/* テキストのない部分を背景色で塗りつぶす */
		if( sPos.GetDrawPos().y < pPs->rcPaint.bottom ){
			RECT rcBack;
			rcBack.left   = pPs->rcPaint.left;
			rcBack.right  = pPs->rcPaint.right;
			rcBack.top    = sPos.GetDrawPos().y;
			rcBack.bottom = pPs->rcPaint.bottom;

			cTextType.FillBack(gr,rcBack);

			// 2006.04.29 行部分は行ごとに作画し、ここでは縦線の残りを作画
			GetTextDrawer().DispVerticalLines( gr, sPos.GetDrawPos().y, pPs->rcPaint.bottom, CLayoutInt(0), CLayoutInt(-1) );
			GetTextDrawer().DispWrapLine( gr, sPos.GetDrawPos().y, pPs->rcPaint.bottom );	// 2009.10.24 ryoji
		}
	}

	cTextType.RewindGraphicsState(gr);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ルーラー描画                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if ( pPs->rcPaint.top < GetTextArea().GetRulerHeight() ) { // ルーラーが再描画範囲にあるときのみ再描画する 2002.02.25 Add By KK
		GetRuler().SetRedrawFlag(); //2002.02.25 Add By KK ルーラー全体を描画。
		GetRuler().DispRuler( gr );
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
			gr,
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
	if( bCaretShowFlag_Old )	// 2008.06.09 ryoji
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
	bool bDispEOF = false;
	SColorStrategyInfo _sInfo;
	SColorStrategyInfo* pInfo = &_sInfo;
	pInfo->gr.Init(_hdc);
	pInfo->pDispPos = _pDispPos;
	pInfo->pcView = this;

//	if( !pcLayout ){
//		return true;
//	}

	//CColorStrategyPool初期化
	CColorStrategyPool* pool = CColorStrategyPool::Instance();
	pool->NotifyOnStartScanLogic();

	//DispPosを保存しておく
	pInfo->sDispPosBegin = *pInfo->pDispPos;

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();


	//サイズ
	STypeConfig* TypeDataPtr = &m_pcEditDoc->m_cDocType.GetDocumentAttribute();
	int nLineHeight = GetTextMetrics().GetHankakuDy();  //行の縦幅？
	int nCharDx  = GetTextMetrics().GetHankakuDx();  //半角

	//処理する文字位置
	pInfo->nPosInLogic = CLogicInt(0); //☆開始

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          論理行データの取得 -> pLine, pLineLen              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 前行の最終設定色
	{
		const CLayout* pcLayout = pInfo->pDispPos->GetLayoutRef();
		EColorIndexType eType = GetColorIndex(pcLayout, 0, true, &pInfo->pStrategy, &pInfo->pStrategyFound);
		pInfo->ChangeColor(eType);
	}

	//開始ロジック位置を算出
	{
		const CLayout* pcLayout = pInfo->pDispPos->GetLayoutRef();
		pInfo->nPosInLogic = pcLayout?pcLayout->GetLogicOffset():CLogicInt(0);
	}

	//サポート
	CTypeSupport cTextType(this,COLORIDX_TEXT);
	CTypeSupport cSearchType(this,COLORIDX_SEARCH);

	//正規表現キーワードを使うか	//@@@ 2001.11.17 add MIK
	if( TypeDataPtr->m_bUseRegexKeyword ){
		m_cRegexKeyword->RegexKeyLineStart();
	}

	while(1){
		//対象行が描画範囲外だったら終了
		if( GetTextArea().GetBottomLine() < pInfo->pDispPos->GetLayoutLineRef() ){
			pInfo->pDispPos->SetLayoutLineRef(nLineTo + CLayoutInt(1));
			break;
		}
		if( nLineTo < pInfo->pDispPos->GetLayoutLineRef() ){
			break;
		}

		//レイアウト行を1行描画
		bDispEOF = DrawLayoutLine(pInfo);

		//行を進める
		CLogicInt nOldLogicLineNo = pInfo->pDispPos->GetLayoutRef()->GetLogicLineNo();
		pInfo->pDispPos->ForwardDrawLine(1);		//描画Y座標＋＋
		pInfo->pDispPos->ForwardLayoutLineRef(1);	//レイアウト行＋＋

		// ロジック行を描画し終わったら抜ける
		if(pInfo->pDispPos->GetLayoutRef()->GetLogicLineNo()!=nOldLogicLineNo){
			break;
		}

		// nLineToを超えたら抜ける
		if(pInfo->pDispPos->GetLayoutLineRef() >= nLineTo + CLayoutInt(1)){
			break;
		}
	}

	return bDispEOF;
}

/*!
	レイアウト行を1行描画
*/
//改行記号を描画した場合はtrueを返す？
bool CEditView::DrawLayoutLine(SColorStrategyInfo* pInfo)
{
	bool bDispEOF = false;
	CTypeSupport cTextType(this,COLORIDX_TEXT);
	CTypeSupport cSearchType(this,COLORIDX_SEARCH);

	const CLayout* pcLayout = pInfo->pDispPos->GetLayoutRef(); //m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );

	// レイアウト情報
	if( pcLayout ){
		pInfo->pLineOfLogic					= pcLayout->GetDocLineRef()->GetPtr();
	}
	else{
		pInfo->pLineOfLogic					= NULL;
	}

	//文字列参照
	const CDocLine* pcDocLine = pInfo->GetDocLine();
	CStringRef cLineStr = pcDocLine->GetStringRefWithEOL();

	//color strategy
	if(pcLayout && pcLayout->GetLogicOffset()==0){
		CColorStrategyPool* pool = CColorStrategyPool::Instance();
		pInfo->pStrategy = pool->GetStrategyByColor(pcLayout->GetColorTypePrev());
		if(pInfo->pStrategy)pInfo->pStrategy->InitStrategyStatus();
		pInfo->ChangeColor(pcLayout->GetColorTypePrev());
	}

	// 描画範囲外の場合は色切替だけで抜ける
	if(pInfo->pDispPos->GetDrawPos().y < pInfo->pcView->GetTextArea().GetAreaTop()){
		if(pcLayout){
			while(pInfo->nPosInLogic < pcLayout->GetLogicOffset() + pcLayout->GetLengthWithEOL()){
				//色切替
				pInfo->DoChangeColor(cLineStr);

				//1文字進む
				pInfo->nPosInLogic += CNativeW::GetSizeOfChar(
											cLineStr.GetPtr(),
											cLineStr.GetLength(),
											pInfo->nPosInLogic
										);
			}
		}
		return false;
	}

	// コンフィグ
	int nLineHeight = GetTextMetrics().GetHankakuDy();  //行の縦幅？
	STypeConfig* TypeDataPtr = &m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        行番号描画                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pInfo->pcView->GetTextDrawer().DispLineNumber(
		pInfo->gr,
		pInfo->pDispPos->GetLayoutLineRef(),
		pInfo->pDispPos->GetDrawPos().y
	);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       本文描画開始                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pInfo->pDispPos->ResetDrawCol();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                 行頭(インデント)背景描画                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if(pcLayout && pcLayout->GetIndent()!=0)
	{
		RECT rcClip;
		if(GetTextArea().GenerateClipRect(&rcClip,*pInfo->pDispPos,(Int)pcLayout->GetIndent())){
			cTextType.FillBack(pInfo->gr,rcClip);
		}
		//描画位置進める
		pInfo->pDispPos->ForwardDrawCol((Int)pcLayout->GetIndent());
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         本文描画                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//行終端または折り返しに達するまでループ
	if(pcLayout){
		while(pInfo->nPosInLogic < pcLayout->GetLogicOffset() + pcLayout->GetLengthWithEOL()){
			//色切替
			pInfo->DoChangeColor(cLineStr);

			//1文字情報取得 $$高速化可能
			CFigure& cFigure = CFigureManager::Instance()->GetFigure(&cLineStr.GetPtr()[pInfo->GetPosInLogic()]);

			//1文字描画
			CLogicInt nPosOld = pInfo->nPosInLogic;
			cFigure.DrawImp(pInfo);
			if(pInfo->nPosInLogic == nPosOld){
				pInfo->nPosInLogic += CNativeW::GetSizeOfChar(
											cLineStr.GetPtr(),
											cLineStr.GetLength(),
											pInfo->nPosInLogic
										);
			}
		}
	}

	// 必要ならEOF描画
	void _DispEOF( CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);
	if(pcLayout && pcLayout->GetNextLayout()==NULL && pcLayout->GetLayoutEol().GetLen()==0){
		// 有文字行のEOF
		_DispEOF(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
		bDispEOF = true;
	}
	else if(!pcLayout && pInfo->pDispPos->GetLayoutLineRef()==m_pcEditDoc->m_cLayoutMgr.GetLineCount()){
		// 空行のEOF
		CLayout* pBottom = m_pcEditDoc->m_cLayoutMgr.GetBottomLayout();
		if(pBottom==NULL || (pBottom && pBottom->GetLayoutEol().GetLen())){
			_DispEOF(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
			bDispEOF = true;
		}
	}

	// 必要なら折り返し記号描画
	if(pcLayout && pcLayout->GetLayoutEol().GetLen()==0 && pcLayout->GetNextLayout()!=NULL){
		_DispWrap(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
	}

	// 行末背景描画
	RECT rcClip;
	if(pInfo->pcView->GetTextArea().GenerateClipRectRight(&rcClip,*pInfo->pDispPos)){
		cTextType.FillBack(pInfo->gr,rcClip);
	}

	// 指定桁縦線描画
	pInfo->pcView->GetTextDrawer().DispVerticalLines(
		pInfo->gr,
		pInfo->pDispPos->GetDrawPos().y,
		pInfo->pDispPos->GetDrawPos().y + nLineHeight,
		CLayoutInt(0),
		CLayoutInt(-1)
	);

	// 折り返し桁縦線描画
	pInfo->pcView->GetTextDrawer().DispWrapLine(
		pInfo->gr,
		pInfo->pDispPos->GetDrawPos().y,
		pInfo->pDispPos->GetDrawPos().y + nLineHeight
	);

	// 反転描画
	if( pcLayout && pInfo->pcView->GetSelectionInfo().IsTextSelected() ){
		pInfo->pcView->DispTextSelected(
			pInfo->gr,
			pInfo->pDispPos->GetLayoutLineRef(),
			CMyPoint(pInfo->sDispPosBegin.GetDrawPos().x, pInfo->pDispPos->GetDrawPos().y),
			pcLayout->GetIndent() + pcLayout->CalcLayoutWidth(CEditDoc::GetInstance(0)->m_cLayoutMgr) + CLayoutInt(pcLayout->GetLayoutEol().GetLen()?1:0)
		);
	}

	return bDispEOF;
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
			if( nSelectTo < GetTextArea().GetViewLeftCol() ){	// nSelectTo == GetTextArea().GetViewLeftCol()のケースは後で０文字マッチでないことを確認してから抜ける
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
				HWND hWnd = ::GetForegroundWindow();
				if( hWnd && (hWnd == m_pcEditWnd->m_cDlgFind.GetHwnd() || hWnd == m_pcEditWnd->m_cDlgReplace.GetHwnd()) ){
					rcClip.right = rcClip.left + (nCharWidth/3 == 0 ? 1 : nCharWidth/3);
				}
			}
			if( rcClip.right == rcClip.left ){
				return;	//０文字マッチによる反転幅拡張なし
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
#if 0
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
