#include "stdafx.h"
#include "CEditView.h"

/*! スクロールバー作成
	@date 2006.12.19 ryoji 新規作成（CEditView::Createから分離）
*/
BOOL CEditView::CreateScrollBar()
{
	SCROLLINFO	si;

	/* スクロールバーの作成 */
	m_hwndVScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles */
		_T("SCROLLBAR"),						/* scroll bar control class */
		NULL,								/* text for window title bar */
		WS_VISIBLE | WS_CHILD | SBS_VERT,	/* scroll bar styles */
		0,									/* horizontal position */
		0,									/* vertical position */
		200,								/* width of the scroll bar */
		CW_USEDEFAULT,						/* default height */
		GetHwnd(),								/* handle of main window */
		(HMENU) NULL,						/* no menu for a scroll bar */
		G_AppInstance(),						/* instance owning this window */
		(LPVOID) NULL						/* pointer not needed */
	);
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin  = 0;
	si.nMax  = 29;
	si.nPage = 10;
	si.nPos  = 0;
	si.nTrackPos = 1;
	::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
	::ShowScrollBar( m_hwndVScrollBar, SB_CTL, TRUE );

	/* スクロールバーの作成 */
	m_hwndHScrollBar = NULL;
	if( GetDllShareData().m_Common.m_sWindow.m_bScrollBarHorz ){	/* 水平スクロールバーを使う */
		m_hwndHScrollBar = ::CreateWindowEx(
			0L,									/* no extended styles */
			_T("SCROLLBAR"),						/* scroll bar control class */
			NULL,								/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_HORZ,	/* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			GetHwnd(),								/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			G_AppInstance(),						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = 29;
		si.nPage = 10;
		si.nPos  = 0;
		si.nTrackPos = 1;
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
		::ShowScrollBar( m_hwndHScrollBar, SB_CTL, TRUE );
	}


	/* サイズボックス */
	if( GetDllShareData().m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
		m_hwndSizeBox = ::CreateWindowEx(
			WS_EX_CONTROLPARENT/*0L*/, 			/* no extended styles */
			_T("SCROLLBAR"),						/* scroll bar control class */
			NULL,								/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			GetHwnd(), 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			G_AppInstance(),						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}else{
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 								/* no extended styles */
			_T("STATIC"),						/* scroll bar control class */
			NULL,								/* text for window title bar */
			WS_VISIBLE | WS_CHILD/* | SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			GetHwnd(), 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			G_AppInstance(),						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}
	return TRUE;
}



/*! スクロールバー破棄
	@date 2006.12.19 ryoji 新規作成
*/
void CEditView::DestroyScrollBar()
{
	if( m_hwndVScrollBar )
	{
		::DestroyWindow( m_hwndVScrollBar );
		m_hwndVScrollBar = NULL;
	}

	if( m_hwndHScrollBar )
	{
		::DestroyWindow( m_hwndHScrollBar );
		m_hwndHScrollBar = NULL;
	}

	if( m_hwndSizeBox )
	{
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
	}
}

/*! 垂直スクロールバーメッセージ処理

	@param nScrollCode [in]	スクロール種別 (Windowsから渡されるもの)
	@param nPos [in]		スクロール位置(THUMBTRACK用)
	@retval	実際にスクロールした行数

	@date 2004.09.11 genta スクロール行数を返すように．
		未使用のhwndScrollBar引数削除．
*/
CLayoutInt CEditView::OnVScroll( int nScrollCode, int nPos )
{
	CLayoutInt nScrollVal = CLayoutInt(0);

	switch( nScrollCode ){
	case SB_LINEDOWN:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( GetTextArea().GetViewTopLine() + 1 );
//		}
		nScrollVal = ScrollAtV( GetTextArea().GetViewTopLine() + GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum );
		break;
	case SB_LINEUP:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( GetTextArea().GetViewTopLine() - 1 );
//		}
		nScrollVal = ScrollAtV( GetTextArea().GetViewTopLine() - GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum );
		break;
	case SB_PAGEDOWN:
		nScrollVal = ScrollAtV( GetTextArea().GetBottomLine() );
		break;
	case SB_PAGEUP:
		nScrollVal = ScrollAtV( GetTextArea().GetViewTopLine() - GetTextArea().m_nViewRowNum );
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtV( CLayoutInt(nPos) );
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtV( CLayoutInt(nPos) );
		break;
	case SB_TOP:
		nScrollVal = ScrollAtV( CLayoutInt(0) );
		break;
	case SB_BOTTOM:
		nScrollVal = ScrollAtV(( m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) - GetTextArea().m_nViewRowNum );
		break;
	default:
		break;
	}
	return nScrollVal;
}

/*! 水平スクロールバーメッセージ処理

	@param nScrollCode [in]	スクロール種別 (Windowsから渡されるもの)
	@param nPos [in]		スクロール位置(THUMBTRACK用)
	@retval	実際にスクロールした桁数

	@date 2004.09.11 genta スクロール桁数を返すように．
		未使用のhwndScrollBar引数削除．
*/
CLayoutInt CEditView::OnHScroll( int nScrollCode, int nPos )
{
	CLayoutInt nScrollVal = CLayoutInt(0);

	GetRuler().SetRedrawFlag(); // YAZAKI
	switch( nScrollCode ){
	case SB_LINELEFT:
		nScrollVal = ScrollAtH( GetTextArea().GetViewLeftCol() - CLayoutInt(4) );
		break;
	case SB_LINERIGHT:
		nScrollVal = ScrollAtH( GetTextArea().GetViewLeftCol() + CLayoutInt(4) );
		break;
	case SB_PAGELEFT:
		nScrollVal = ScrollAtH( GetTextArea().GetViewLeftCol() - GetTextArea().m_nViewColNum );
		break;
	case SB_PAGERIGHT:
		nScrollVal = ScrollAtH( GetTextArea().GetRightCol() );
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtH( CLayoutInt(nPos) );
//		MYTRACE_A( "nPos=%d\n", nPos );
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtH( CLayoutInt(nPos) );
//		MYTRACE_A( "nPos=%d\n", nPos );
		break;
	case SB_LEFT:
		nScrollVal = ScrollAtH( CLayoutInt(0) );
		break;
	case SB_RIGHT:
		//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
		nScrollVal = ScrollAtH( m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() - GetTextArea().m_nViewColNum );
		break;
	}
	return nScrollVal;
}

/** スクロールバーの状態を更新する

	タブバーのタブ切替時は SIF_DISABLENOSCROLL フラグでの有効化／無効化が正常に動作しない
	（不可視でサイズ変更していることによる影響か？）ので SIF_DISABLENOSCROLL で有効／無効
	の切替に失敗した場合には強制切替する

	@date 2008.05.24 ryoji 有効／無効の強制切替を追加
	@date 2008.06.08 ryoji 水平スクロール範囲にぶら下げ余白を追加
*/
void CEditView::AdjustScrollBars()
{
	if( !GetDrawSwitch() ){
		return;
	}


	CLayoutInt	nAllLines;
	int			nVScrollRate;
	SCROLLINFO	si;
	bool		bEnable;

	if( NULL != m_hwndVScrollBar ){
		/* 垂直スクロールバー */
		/* nAllLines / nVScrollRate < 65535 となる整数nVScrollRateを求める */
		nAllLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
		nAllLines+=2;
		nVScrollRate = 1;
		while( nAllLines / nVScrollRate > 65535 ){
			++nVScrollRate;
		}
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = (Int)nAllLines / nVScrollRate - 1;	/* 全行数 */
		si.nPage = (Int)GetTextArea().m_nViewRowNum / nVScrollRate;	/* 表示域の行数 */
		si.nPos  = (Int)GetTextArea().GetViewTopLine() / nVScrollRate;	/* 表示域の一番上の行(0開始) */
		si.nTrackPos = nVScrollRate;
		::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
		m_nVScrollRate = nVScrollRate;				/* 垂直スクロールバーの縮尺 */
		
		//	Nov. 16, 2002 genta
		//	縦スクロールバーがDisableになったときは必ず全体が画面内に収まるように
		//	スクロールさせる
		//	2005.11.01 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
		bEnable = ( GetTextArea().m_nViewRowNum < nAllLines );
		if( bEnable != (::IsWindowEnabled( m_hwndVScrollBar ) != 0) ){
			::EnableWindow( m_hwndVScrollBar, bEnable? TRUE: FALSE );	// SIF_DISABLENOSCROLL 誤動作時の強制切替
		}
		if( !bEnable ){
			ScrollAtV( CLayoutInt(0) );
		}
	}
	if( NULL != m_hwndHScrollBar ){
		/* 水平スクロールバー */
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = (Int)m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() + (Int)GetWrapOverhang() - 1; //	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
		si.nPage = (Int)GetTextArea().m_nViewColNum;			/* 表示域の桁数 */
		si.nPos  = (Int)GetTextArea().GetViewLeftCol();		/* 表示域の一番左の桁(0開始) */
		si.nTrackPos = 1;
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );

		//	2006.1.28 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
		bEnable = ( GetTextArea().m_nViewColNum < m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() + GetWrapOverhang() );
		if( bEnable != (::IsWindowEnabled( m_hwndHScrollBar ) != 0) ){
			::EnableWindow( m_hwndHScrollBar, bEnable? TRUE: FALSE );	// SIF_DISABLENOSCROLL 誤動作時の強制切替
		}
		if( !bEnable ){
			ScrollAtH( CLayoutInt(0) );
		}
	}
}

/*! 指定上端行位置へスクロール

	@param nPos [in] スクロール位置
	@retval 実際にスクロールした行数 (正:下方向/負:上方向)

	@date 2004.09.11 genta 行数を戻り値として返すように．(同期スクロール用)
*/
CLayoutInt CEditView::ScrollAtV( CLayoutInt nPos )
{
	CLayoutInt	nScrollRowNum;
	RECT		rcScrol;
	RECT		rcClip;
	if( nPos < 0 ){
		nPos = CLayoutInt(0);
	}
	else if( (m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 )- GetTextArea().m_nViewRowNum < nPos ){
		nPos = ( m_pcEditDoc->m_cLayoutMgr.GetLineCount() + CLayoutInt(2) ) - GetTextArea().m_nViewRowNum;
		if( nPos < 0 ){
			nPos = CLayoutInt(0);
		}
	}
	if( GetTextArea().GetViewTopLine() == nPos ){
		return CLayoutInt(0);	//	スクロール無し。
	}
	/* 垂直スクロール量（行数）の算出 */
	nScrollRowNum = GetTextArea().GetViewTopLine() - nPos;

	/* スクロール */
	if( t_abs( nScrollRowNum ) >= GetTextArea().m_nViewRowNum ){
		GetTextArea().SetViewTopLine( CLayoutInt(nPos) );
		::InvalidateRect( GetHwnd(), NULL, TRUE );
	}else{
		rcScrol.left = 0;
		rcScrol.right = GetTextArea().GetAreaRight();
		rcScrol.top = GetTextArea().GetAreaTop();
		rcScrol.bottom = GetTextArea().GetAreaBottom();
		if( nScrollRowNum > 0 ){
			rcScrol.bottom =
				GetTextArea().GetAreaBottom() -
				(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			GetTextArea().SetViewTopLine( CLayoutInt(nPos) );
			rcClip.left = 0;
			rcClip.right = GetTextArea().GetAreaRight();
			rcClip.top = GetTextArea().GetAreaTop();
			rcClip.bottom =
				GetTextArea().GetAreaTop() + (Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
		}
		else if( nScrollRowNum < 0 ){
			rcScrol.top =
				GetTextArea().GetAreaTop() - (Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			GetTextArea().SetViewTopLine( CLayoutInt(nPos) );
			rcClip.left = 0;
			rcClip.right = GetTextArea().GetAreaRight();
			rcClip.top =
				GetTextArea().GetAreaBottom() +
				(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			rcClip.bottom = GetTextArea().GetAreaBottom();
		}
		if( GetDrawSwitch() ){
			::ScrollWindowEx(
				GetHwnd(),
				0,	/* 水平スクロール量 */
				(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy(),		/* 垂直スクロール量 */
				&rcScrol,	/* スクロール長方形の構造体のアドレス */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
			// 互換BMPのスクロール
			if( m_hbmpCompatBMP ){
				::BitBlt(
					m_hdcCompatDC, rcScrol.left,
					rcScrol.top + (Int)nScrollRowNum * ( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nLineSpace + GetTextMetrics().GetHankakuHeight() ),
					rcScrol.right - rcScrol.left, rcScrol.bottom - rcScrol.top,
					m_hdcCompatDC, rcScrol.left, rcScrol.top, SRCCOPY
				);
			}
			::InvalidateRect( GetHwnd(), &rcClip, TRUE );
			::UpdateWindow( GetHwnd() );
		}
	}

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* キャレットの表示・更新 */
	GetCaret().ShowEditCaret();

	return -nScrollRowNum;	//方向が逆なので符号反転が必要
}




/*! 指定左端桁位置へスクロール

	@param nPos [in] スクロール位置
	@retval 実際にスクロールした桁数 (正:右方向/負:左方向)

	@date 2004.09.11 genta 桁数を戻り値として返すように．(同期スクロール用)
	@date 2008.06.08 ryoji 水平スクロール範囲にぶら下げ余白を追加
*/
CLayoutInt CEditView::ScrollAtH( CLayoutInt nPos )
{
	CLayoutInt	nScrollColNum;
	RECT		rcScrol;
	RECT		rcClip2;
	if( nPos < 0 ){
		nPos = CLayoutInt(0);
	}
	//	Aug. 18, 2003 ryoji 変数のミスを修正
	//	ウィンドウの幅をきわめて狭くしたときに編集領域が行番号から離れてしまうことがあった．
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	else if( m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() + GetWrapOverhang() - GetTextArea().m_nViewColNum  < nPos ){
		nPos = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() + GetWrapOverhang() - GetTextArea().m_nViewColNum ;
		//	May 29, 2004 genta 折り返し幅よりウィンドウ幅が大きいときにWM_HSCROLLが来ると
		//	nPosが負の値になることがあり，その場合にスクロールバーから編集領域が
		//	離れてしまう．
		if( nPos < 0 )
			nPos = CLayoutInt(0);
	}
	if( GetTextArea().GetViewLeftCol() == nPos ){
		return CLayoutInt(0);
	}
	/* 水平スクロール量（文字数）の算出 */
	nScrollColNum = GetTextArea().GetViewLeftCol() - nPos;

	/* スクロール */
	if( t_abs( nScrollColNum ) >= GetTextArea().m_nViewColNum /*|| abs( nScrollRowNum ) >= GetTextArea().m_nViewRowNum*/ ){
		GetTextArea().SetViewLeftCol( nPos );
		::InvalidateRect( GetHwnd(), NULL, TRUE );
	}else{
		rcScrol.left = 0;
		rcScrol.right = GetTextArea().GetAreaRight();
		rcScrol.top = GetTextArea().GetAreaTop();
		rcScrol.bottom = GetTextArea().GetAreaBottom();
		if( nScrollColNum > 0 ){
			rcScrol.left = GetTextArea().GetAreaLeft();
			rcScrol.right =
				GetTextArea().GetAreaRight() - (Int)nScrollColNum * GetTextMetrics().GetHankakuDx();
			rcClip2.left = GetTextArea().GetAreaLeft();
			rcClip2.right = GetTextArea().GetAreaLeft() + (Int)nScrollColNum * GetTextMetrics().GetHankakuDx();
			rcClip2.top = GetTextArea().GetAreaTop();
			rcClip2.bottom = GetTextArea().GetAreaBottom();
		}
		else if( nScrollColNum < 0 ){
			rcScrol.left = GetTextArea().GetAreaLeft() - (Int)nScrollColNum * GetTextMetrics().GetHankakuDx();
			rcClip2.left =
				GetTextArea().GetAreaRight() + (Int)nScrollColNum * GetTextMetrics().GetHankakuDx();
			rcClip2.right = GetTextArea().GetAreaRight();
			rcClip2.top = GetTextArea().GetAreaTop();
			rcClip2.bottom = GetTextArea().GetAreaBottom();
		}
		GetTextArea().SetViewLeftCol( nPos );
		if( GetDrawSwitch() ){
			::ScrollWindowEx(
				GetHwnd(),
				(Int)nScrollColNum * GetTextMetrics().GetHankakuDx(),		/* 水平スクロール量 */
				0,	/* 垂直スクロール量 */
				&rcScrol,	/* スクロール長方形の構造体のアドレス */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
			// 互換BMPのスクロール
			if( m_hbmpCompatBMP ){
				::BitBlt(
					m_hdcCompatDC, rcScrol.left + (Int)nScrollColNum * ( GetTextMetrics().GetHankakuWidth() + m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nColmSpace ),
						rcScrol.top, rcScrol.right - rcScrol.left, rcScrol.bottom - rcScrol.top,
					m_hdcCompatDC, rcScrol.left, rcScrol.top , SRCCOPY
				);
			}
			// カーソルの縦線がテキストと行番号の隙間にあるとき、スクロール時に縦線領域を更新
			if( m_nOldCursorLineX == GetTextArea().GetAreaLeft() - 1 ){
				RECT rcClip3;
				rcClip3.left = m_nOldCursorLineX;
				rcClip3.right = m_nOldCursorLineX + 1;
				rcClip3.top  = GetTextArea().GetAreaTop();
				rcClip3.bottom = GetTextArea().GetAreaBottom();
				::InvalidateRect( GetHwnd(), &rcClip3, TRUE );
			}
			::InvalidateRect( GetHwnd(), &rcClip2, TRUE );
			::UpdateWindow( GetHwnd() );
		}
	}
	//	2006.1.28 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
	// 先にAdjustScrollBarsを呼んでしまうと、二度目はここまでこないので、
	// GetRuler().DispRulerが呼ばれない。そのため、順序を入れ替えた。
	GetRuler().SetRedrawFlag(); // ルーラーを再描画する。
	HDC hdc = ::GetDC( GetHwnd() );
	GetRuler().DispRuler( hdc );
	::ReleaseDC( GetHwnd(), hdc );

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* キャレットの表示・更新 */
	GetCaret().ShowEditCaret();

	return -nScrollColNum;	//方向が逆なので符号反転が必要
}

/*!	垂直同期スクロール

	垂直同期スクロールがONならば，対応するウィンドウを指定行数同期スクロールする
	
	@param line [in] スクロール行数 (正:下方向/負:上方向/0:何もしない)
	
	@author asa-o
	@date 2001.06.20 asa-o 新規作成
	@date 2004.09.11 genta 関数化

	@note 動作の詳細は設定や機能拡張により変更になる可能性がある

*/
void CEditView::SyncScrollV( CLayoutInt line )
{
	if( GetDllShareData().m_Common.m_sWindow.m_bSplitterWndVScroll && line != 0 )
	{
		CEditView*	pcEditView = m_pcEditDoc->m_pcEditWnd->m_pcEditViewArr[m_nMyIndex^0x01];
#if 0
		//	差分を保ったままスクロールする場合
		pcEditView -> ScrollByV( line );
#else
		pcEditView -> ScrollAtV( GetTextArea().GetViewTopLine() );
#endif
	}
}

/*!	水平同期スクロール

	水平同期スクロールがONならば，対応するウィンドウを指定行数同期スクロールする．
	
	@param col [in] スクロール桁数 (正:右方向/負:左方向/0:何もしない)
	
	@author asa-o
	@date 2001.06.20 asa-o 新規作成
	@date 2004.09.11 genta 関数化

	@note 動作の詳細は設定や機能拡張により変更になる可能性がある
*/
void CEditView::SyncScrollH( CLayoutInt col )
{
	if( GetDllShareData().m_Common.m_sWindow.m_bSplitterWndHScroll && col != 0 )
	{
		CEditView*	pcEditView = m_pcEditDoc->m_pcEditWnd->m_pcEditViewArr[m_nMyIndex^0x02];
		HDC			hdc = ::GetDC( pcEditView->GetHwnd() );
		
#if 0
		//	差分を保ったままスクロールする場合
		pcEditView -> ScrollByH( col );
#else
		pcEditView -> ScrollAtH( GetTextArea().GetViewLeftCol() );
#endif
		GetRuler().SetRedrawFlag(); //2002.02.25 Add By KK スクロール時ルーラー全体を描きなおす。
		GetRuler().DispRuler( hdc );
		::ReleaseDC( GetHwnd(), hdc );
	}
}

/** 折り返し桁以後のぶら下げ余白計算
	@date 2008.06.08 ryoji 新規作成
*/
CLayoutInt CEditView::GetWrapOverhang( void ) const
{
	int nMargin = 0;
	if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuRet )
		nMargin += 2;	// 改行ぶら下げ
	if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bKinsokuKuto )
		nMargin += 2;	// 句読点ぶら下げ
	return CLayoutInt( nMargin );
}

/** 「右端で折り返す」用にビューの桁数から折り返し桁数を計算する
	@param nViewColNum	[in] ビューの桁数
	@retval 折り返し桁数
	@date 2008.06.08 ryoji 新規作成
*/
CLayoutInt CEditView::ViewColNumToWrapColNum( CLayoutInt nViewColNum ) const
{
	// ぶら下げ余白を差し引く
	int nWidth = (Int)(nViewColNum - GetWrapOverhang());

	// MINLINEKETAS未満の時はMINLINEKETASで折り返しとする
	if( nWidth < MINLINEKETAS )
		nWidth = MINLINEKETAS;		// 折り返し幅の最小桁数に設定

	return CLayoutInt( nWidth );
}
