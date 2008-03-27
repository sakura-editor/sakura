/*!	@file
	@brief 印刷プレビュー管理クラス

	@author YAZAKI
	@date 2002/1/11 新規作成
*/
/*
	Copyright (C) 2001, Stonee, jepro, genta
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta
	Copyright (C) 2003, genta, かろと, おきた, KEITA
	Copyright (C) 2005, D.S.Koba
	Copyright (C) 2006, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CPrintPreview.h"
#include "CLayout.h"
#include "CEditWnd.h"
#include "charcode.h"
#include "CDlgPrintPage.h"
#include "CDlgCancel.h"/// 2002/2/3 aroka from here
#include "CDlgInput1.h" /// 2007.02.11 Moca
#include "Debug.h"///
#include <stdio.h>/// 2002/2/3 aroka to here
#include <vector>
#include "CEditApp.h"
#include "util/window.h"
#include "util/shell.h"
#include "CSakuraEnvironment.h"
using namespace std;

#define MIN_PREVIEW_ZOOM 10
#define MAX_PREVIEW_ZOOM 400

#define		LINE_RANGE_X	48		/* 水平方向の１回のスクロール幅 */
#define		LINE_RANGE_Y	24		/* 垂直方向の１回のスクロール幅 */

#define		PAGE_RANGE_X	160		/* 水平方向の１回のページスクロール幅 */
#define		PAGE_RANGE_Y	160		/* 垂直方向の１回のページスクロール幅 */

#define		COMPAT_BMP_BASE     1   /* COMPAT_BMP_SCALEピクセル幅を複写する画面ピクセル幅 */
#define		COMPAT_BMP_SCALE    2   /* 互換BMPのCOMPAT_BMP_BASEに対する倍率(1以上の整数倍) */

CPrint CPrintPreview::m_cPrint;		//!< 現在のプリンタ情報 2003.05.02 かろと

/*! コンストラクタ
	印刷プレビューを表示するために必要な情報を初期化、領域確保。
	コントロールも作成する。
*/
CPrintPreview::CPrintPreview(CEditWnd* pParentWnd ) :
	m_nPreview_Zoom( 100 ),			/* 印刷プレビュー倍率 */
	m_nPreviewVScrollPos( 0 ),
	m_nPreviewHScrollPos( 0 ),
	m_nCurPageNum( 0 ),				/* 現在のページ */
	m_pParentWnd( pParentWnd ),
	m_hdcCompatDC( NULL ),			// 再描画用コンパチブルDC
	m_hbmpCompatBMP( NULL ),		// 再描画用メモリBMP
	m_hbmpCompatBMPOld( NULL ),		// 再描画用メモリBMP(OLD)
	m_nbmpCompatScale( COMPAT_BMP_BASE )
{
	/* 印刷用のレイアウト情報の作成 */
	m_pLayoutMgr_Print = new CLayoutMgr;

	/* 印刷プレビュー コントロール 作成 */
	CreatePrintPreviewControls();

	// 再描画用コンパチブルDC
	HDC hdc = ::GetDC( pParentWnd->GetHwnd() );
	m_hdcCompatDC = ::CreateCompatibleDC( hdc );
	::ReleaseDC( pParentWnd->GetHwnd(), hdc );
}

CPrintPreview::~CPrintPreview()
{
	/* 印刷プレビュー コントロール 破棄 */
	DestroyPrintPreviewControls();
	
	/* 印刷用のレイアウト情報の削除 */
	delete m_pLayoutMgr_Print;
	
	// 2006.08.17 Moca CompatDC削除。CEditWndから移設
	// 再描画用メモリBMP
	if( m_hbmpCompatBMP != NULL ){
		// 再描画用メモリBMP(OLD)
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
		::DeleteObject( m_hbmpCompatBMP );
	}
	// 再描画用コンパチブルDC
	if( m_hdcCompatDC != NULL ){
		::DeleteDC( m_hdcCompatDC );
	}
}

/*!	印刷プレビュー時の、WM_PAINTを処理

	@date 2007.02.11 Moca プレビューを滑らかにする機能．
		拡大描画してから縮小することでアンチエイリアス効果を出す．
*/
LRESULT CPrintPreview::OnPaint(
	HWND			hwnd,	// handle of window
	UINT			uMsg,	// message identifier
	WPARAM			wParam,	// first message parameter
	LPARAM			lParam 	// second message parameter
)
{
	PAINTSTRUCT		ps;
	HDC				hdcOld = ::BeginPaint( hwnd, &ps );
	HDC				hdc = m_hdcCompatDC;	//	親ウィンドウのComatibleDCに描く

	/* 印刷プレビュー 操作バー */
	
	// BMPはあとで縮小コピーするので拡大して作画する必要あり

	// クライアント領域全体をグレーで塗りつぶす
	{
		RECT bmpRc;
		::GetClientRect( hwnd, &bmpRc );
		bmpRc.right  = (bmpRc.right  * m_nbmpCompatScale) / COMPAT_BMP_BASE;
		bmpRc.bottom = (bmpRc.bottom * m_nbmpCompatScale) / COMPAT_BMP_BASE;
		::FillRect( hdc, &bmpRc, (HBRUSH)::GetStockObject( GRAY_BRUSH ) );
	}

	// ツールバー高さ -> nToolBarHeight
	int nToolBarHeight = 0;
	if( NULL != m_hwndPrintPreviewBar ){
		RECT rc;
		::GetWindowRect( m_hwndPrintPreviewBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
	}

	// プリンタ情報の表示 -> IDD_PRINTPREVIEWBAR右上のSTATICへ
	TCHAR	szText[1024];
	::DlgItem_SetText(
		m_hwndPrintPreviewBar,
		IDC_STATIC_PRNDEV,
		m_pPrintSetting->m_mdmDevMode.m_szPrinterDeviceName
	);

	// 要素情報の表示 -> IDD_PRINTPREVIEWBAR右下のSTATICへ
	TCHAR	szPaperName[256];
	CPrint::GetPaperName( m_pPrintSetting->m_mdmDevMode.dmPaperSize , szPaperName );
	auto_sprintf(
		szText,
		_T("%ts  %ts"),
		szPaperName,
		(m_pPrintSetting->m_mdmDevMode.dmOrientation & DMORIENT_LANDSCAPE) ? _T("横") : _T("縦")
	);
	::DlgItem_SetText( m_hwndPrintPreviewBar, IDC_STATIC_PAPER, szText );

	// バックグラウンド モードを変更
	::SetBkMode( hdc, TRANSPARENT );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        マッピング                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// マッピングモードの変更
	int nMapModeOld =
	::SetMapMode( hdc, MM_LOMETRIC );
	::SetMapMode( hdc, MM_ANISOTROPIC );

	// 出力倍率の変更
	SIZE			sz;
	::GetWindowExtEx( hdc, &sz );
	int nCx = sz.cx;
	int nCy = sz.cy;
	nCx = (int)( ((long)nCx) * 100L / ((long)m_nPreview_Zoom) );
	nCy = (int)( ((long)nCy) * 100L / ((long)m_nPreview_Zoom) );
	// 作画時は、 COMPAT_BMP_SCALE/COMPAT_BMP_BASE倍の座標 (SetWindowExtExは逆なので反対になる)
	nCx = (nCx * COMPAT_BMP_BASE) / m_nbmpCompatScale;
	nCy = (nCy * COMPAT_BMP_BASE) / m_nbmpCompatScale;
	::SetWindowExtEx( hdc, nCx, nCy, &sz );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         フォント                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 印刷用半角フォントを作成 -> hFontHan
	m_lfPreviewHan.lfHeight	= m_pPrintSetting->m_nPrintFontHeight;
	m_lfPreviewHan.lfWidth	= m_pPrintSetting->m_nPrintFontWidth;
	_tcscpy( m_lfPreviewHan.lfFaceName, m_pPrintSetting->m_szPrintFontFaceHan );
	HFONT	hFontHan = CreateFontIndirect( &m_lfPreviewHan );

	// 印刷用全角フォントを作成 -> hFontZen
	m_lfPreviewZen.lfHeight	= m_pPrintSetting->m_nPrintFontHeight;
	m_lfPreviewZen.lfWidth	= m_pPrintSetting->m_nPrintFontWidth;
	_tcscpy( m_lfPreviewZen.lfFaceName, m_pPrintSetting->m_szPrintFontFaceZen );
	HFONT	hFontZen = CreateFontIndirect( &m_lfPreviewZen );

	// 印刷用半角フォントに設定し、以前のフォントを保持
	HFONT	hFontOld = (HFONT)::SelectObject( hdc, hFontHan );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           原点                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 操作ウィンドウの下に物理座標原点を移動
	POINT poViewPortOld;
	::SetViewportOrgEx(
		hdc,
		((-1 * m_nPreviewHScrollPos) * m_nbmpCompatScale) / COMPAT_BMP_BASE, 
		((nToolBarHeight + m_nPreviewVScrollPos) * m_nbmpCompatScale) / COMPAT_BMP_BASE,
		&poViewPortOld
	);


	// 以下 0.1mm座標でレンダリング

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           背景                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 用紙の描画
	int	nDirectY = -1;	//	Y座標の下をプラス方向にするため？
	::Rectangle( hdc,
		m_nPreview_ViewMarginLeft,
		nDirectY * ( m_nPreview_ViewMarginTop ),
		m_nPreview_ViewMarginLeft + m_nPreview_PaperAllWidth + 1,
		nDirectY * (m_nPreview_ViewMarginTop + m_nPreview_PaperAllHeight + 1 )
	);

	// マージン枠の表示
	HPEN			hPen, hPenOld;
	hPen = ::CreatePen( PS_SOLID, 0, RGB(128,128,128) ); // 2006.08.14 Moca 127を128に変更
	hPenOld = (HPEN)::SelectObject( hdc, hPen );
	::Rectangle( hdc,
		m_nPreview_ViewMarginLeft + m_pPrintSetting->m_nPrintMarginLX,
		nDirectY * ( m_nPreview_ViewMarginTop + m_pPrintSetting->m_nPrintMarginTY ),
		m_nPreview_ViewMarginLeft + m_nPreview_PaperAllWidth - m_pPrintSetting->m_nPrintMarginRX + 1,
		nDirectY * ( m_nPreview_ViewMarginTop + m_nPreview_PaperAllHeight - m_pPrintSetting->m_nPrintMarginBY )
	);
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );

	::SetTextColor( hdc, RGB( 0, 0, 0 ) );

	RECT cRect;	/* 紙の大きさをあらわすRECT */
	cRect.left   = m_nPreview_ViewMarginLeft +                             m_pPrintSetting->m_nPrintMarginLX + 5;
	cRect.right  = m_nPreview_ViewMarginLeft + m_nPreview_PaperAllWidth - (m_pPrintSetting->m_nPrintMarginRX + 5);
	cRect.top    = nDirectY * ( m_nPreview_ViewMarginTop +                              m_pPrintSetting->m_nPrintMarginTY + 5);
	cRect.bottom = nDirectY * ( m_nPreview_ViewMarginTop + m_nPreview_PaperAllHeight - (m_pPrintSetting->m_nPrintMarginBY + 5));


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         テキスト                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// ヘッダ
	DrawHeader( hdc, cRect, hFontZen );

	// 印刷/印刷プレビュー ページテキストの描画
	DrawPageText(
		hdc,
		m_nPreview_ViewMarginLeft + m_pPrintSetting->m_nPrintMarginLX,
		m_nPreview_ViewMarginTop  + m_pPrintSetting->m_nPrintMarginTY + 2 * ( m_pPrintSetting->m_nPrintFontHeight + (m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100) ),
		m_nCurPageNum,
		hFontZen,
		NULL
	);

	// フッタ
	DrawFooter( hdc, cRect, hFontZen );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          後始末                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// 印刷フォント解除 & 破棄
	::SelectObject( hdc, hFontOld );
	::DeleteObject( hFontZen );
	::DeleteObject( hFontHan );

	// マッピングモードの変更
	::SetMapMode( hdc, nMapModeOld );

	// 物理座標原点をもとに戻す
	::SetViewportOrgEx( hdc, poViewPortOld.x, poViewPortOld.y, NULL );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       実画面へ転送                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// メモリＤＣを利用した再描画の場合はメモリＤＣに描画した内容を画面へコピーする
	RECT rc;
	rc = ps.rcPaint;
	::DPtoLP( hdc, (POINT*)&rc, 2 );
	if( 1 == (m_nbmpCompatScale / COMPAT_BMP_BASE) ){
		::BitBlt(
			hdcOld,
			ps.rcPaint.left,
			ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left,
			ps.rcPaint.bottom - ps.rcPaint.top,
			hdc,
			ps.rcPaint.left,
			ps.rcPaint.top,
			SRCCOPY
		);
	}
	else{
		int stretchModeOld = SetStretchBltMode( hdcOld, STRETCH_HALFTONE );
		::StretchBlt(
			hdcOld,
			ps.rcPaint.left,
			ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left,
			ps.rcPaint.bottom - ps.rcPaint.top,
			hdc,
			(ps.rcPaint.left * m_nbmpCompatScale) / COMPAT_BMP_BASE,
			(ps.rcPaint.top * m_nbmpCompatScale) / COMPAT_BMP_BASE,
			((ps.rcPaint.right - ps.rcPaint.left) * m_nbmpCompatScale) / COMPAT_BMP_BASE,
			((ps.rcPaint.bottom - ps.rcPaint.top) * m_nbmpCompatScale) / COMPAT_BMP_BASE,
			SRCCOPY
		);
		SetStretchBltMode( hdcOld, stretchModeOld );
	}
	::EndPaint( hwnd, &ps );
	return 0L;
}

LRESULT CPrintPreview::OnSize( WPARAM wParam, LPARAM lParam )
{
	int	cx = LOWORD( lParam );
	int	cy = HIWORD( lParam );

	/* 印刷プレビュー 操作バー */
	int nToolBarHeight = 0;
	if( NULL != m_hwndPrintPreviewBar ){
		RECT			rc;
		::GetWindowRect( m_hwndPrintPreviewBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
		::MoveWindow( m_hwndPrintPreviewBar, 0, 0, cx, nToolBarHeight, TRUE );
	}

	/* 印刷プレビュー 垂直スクロールバーウィンドウ */
	int	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	int	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );
	if( NULL != m_hwndVScrollBar ){
		::MoveWindow( m_hwndVScrollBar, cx - nCxVScroll, nToolBarHeight, nCxVScroll, cy - nCyVScroll - nToolBarHeight, TRUE );
	}
	
	/* 印刷プレビュー 水平スクロールバーウィンドウ */
	int	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	int	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	if( NULL != m_hwndHScrollBar ){
		::MoveWindow( m_hwndHScrollBar, 0, cy - nCyHScroll, cx - nCxVScroll, nCyHScroll, TRUE );
	}
	
	/* 印刷プレビュー サイズボックスウィンドウ */
	if( NULL != m_hwndSizeBox){
		::MoveWindow( m_hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE );
	}

	HDC			hdc = ::GetDC( m_pParentWnd->GetHwnd() );
	int nMapModeOld = ::SetMapMode( hdc, MM_LOMETRIC );
	::SetMapMode( hdc, MM_ANISOTROPIC );

	/* 出力倍率の変更 */
	SIZE		sz;
	::GetWindowExtEx( hdc, &sz );
	int nCx = sz.cx;
	int nCy = sz.cy;
	nCx = (int)( ((long)nCx) * 100L / ((long)m_nPreview_Zoom) );
	nCy = (int)( ((long)nCy) * 100L / ((long)m_nPreview_Zoom) );
	::SetWindowExtEx( hdc, nCx, nCy, &sz );

	/* ビューのサイズ */
	POINT		po;
	po.x = m_nPreview_PaperAllWidth + m_nPreview_ViewMarginLeft * 2;
	po.y = m_nPreview_PaperAllHeight + m_nPreview_ViewMarginTop * 2;
	::LPtoDP( hdc, &po, 1 );

	/* 再描画用メモリＢＭＰ */
	if( m_hbmpCompatBMP != NULL ){
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );	/* 再描画用メモリＢＭＰ(OLD) */
		::DeleteObject( m_hbmpCompatBMP );
	}
	// 2007.02.11 Moca プレビューを滑らかにする
	COsVersionInfo osVer;
	// Win9xでは 巨大なBMPは作成できないことと
	// StretchBltでSTRETCH_HALFTONEが未サポートであるので Win2K 以上のみで有効にする。
	if( BST_CHECKED == ::IsDlgButtonChecked( m_hwndPrintPreviewBar, IDC_CHECK_ANTIALIAS ) &&
			osVer.IsWin2000_or_later() ){
		m_nbmpCompatScale = COMPAT_BMP_SCALE;
	}else{
		// Win9x: BASE = SCALE で 1:1
		m_nbmpCompatScale = COMPAT_BMP_BASE;
	}
	m_hbmpCompatBMP = ::CreateCompatibleBitmap( hdc, (cx * m_nbmpCompatScale + COMPAT_BMP_BASE - 1) / COMPAT_BMP_BASE,
		(cy * m_nbmpCompatScale + COMPAT_BMP_BASE - 1) / COMPAT_BMP_BASE);
	m_hbmpCompatBMPOld = (HBITMAP)::SelectObject( m_hdcCompatDC, m_hbmpCompatBMP );

	::SetMapMode( hdc, nMapModeOld );

	::ReleaseDC( m_pParentWnd->GetHwnd(), hdc );

	/* 印刷プレビュー：ビュー幅(ピクセル) */
	m_nPreview_ViewWidth = abs( po.x );
	
	/* 印刷プレビュー：ビュー高さ(ピクセル) */
	m_nPreview_ViewHeight = abs( po.y );
	
	/* 印刷プレビュー スクロールバー初期化 */
	InitPreviewScrollBar();
	
	/* 印刷プレビュー スクロールバーの初期化 */
	
	m_pParentWnd->SetDragPosOrg(CMyPoint(0,0));
	m_pParentWnd->SetDragMode(true);
	OnMouseMove( 0, MAKELONG( 0, 0 ) );
	m_pParentWnd->SetDragMode(false);
	//	SizeBox問題テスト
	if( NULL != m_hwndSizeBox ){
		if( wParam == SIZE_MAXIMIZED ){
			::ShowWindow( m_hwndSizeBox, SW_HIDE );
		}else
		if( wParam == SIZE_RESTORED ){
			if( ::IsZoomed( m_pParentWnd->GetHwnd() ) ){
				::ShowWindow( m_hwndSizeBox, SW_HIDE );
			}else{
				::ShowWindow( m_hwndSizeBox, SW_SHOW );
			}
		}else{
			::ShowWindow( m_hwndSizeBox, SW_SHOW );
		}
	}
	::InvalidateRect( m_pParentWnd->GetHwnd(), NULL, TRUE );
	return 0L;
}

/*!
	@date 2006.08.14 Moca SB_TOP, SB_BOTTOMへの対応
*/
LRESULT CPrintPreview::OnVScroll( WPARAM wParam, LPARAM lParam )
{
	int			nPreviewVScrollPos;
	SCROLLINFO	si;
	int			nNowPos;
	int			nMove;
	int			nNewPos;
	int			nScrollCode;
	int			nPos;
	HWND		hwndScrollBar;
	nScrollCode = (int) LOWORD(wParam);
	nPos = (int) HIWORD(wParam);
	hwndScrollBar = (HWND) lParam;
	si.cbSize = sizeof( si );
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	::GetScrollInfo( hwndScrollBar, SB_CTL, &si );
	nNowPos = ::GetScrollPos( hwndScrollBar, SB_CTL );
	nNewPos = 0;
	nMove = 0;
	switch( nScrollCode ){
	case SB_LINEUP:
		nMove = -1 * LINE_RANGE_Y;
		break;
	case SB_LINEDOWN:
		nMove = LINE_RANGE_Y;
		break;
	case SB_PAGEUP:
		nMove = -1 * PAGE_RANGE_Y;
		break;
	case SB_PAGEDOWN:
		nMove = PAGE_RANGE_Y;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nMove = nPos - nNowPos;
		break;
	// 2006.08.14 Moca SB_TOP, SB_BOTTOMへの対応
	case SB_TOP:
		nMove = -1 * nNowPos;
		break;
	case SB_BOTTOM:
		nMove = si.nMax - nNowPos;
		break;
	default:
		return 0;
	}
	nNewPos = nNowPos + nMove;
	if( nNewPos < 0 ){
		nNewPos = 0;
	}else
	if( nNewPos > (int)(si.nMax - si.nPage + 1) ){
		nNewPos = (int)(si.nMax - si.nPage + 1);
	}
	nMove = nNowPos - nNewPos;
	nPreviewVScrollPos = -1 * nNewPos;
	if( nPreviewVScrollPos != m_nPreviewVScrollPos ){
		::SetScrollPos( hwndScrollBar, SB_CTL, nNewPos, TRUE);
		m_nPreviewVScrollPos = nPreviewVScrollPos;
		/* 描画 */
		::ScrollWindowEx( m_pParentWnd->GetHwnd(), 0, nMove, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE );
	}
	return 0;
}

/*!
	@date 2006.08.14 Moca SB_LEFT, SB_RIGHTへの対応
*/
LRESULT CPrintPreview::OnHScroll( WPARAM wParam, LPARAM lParam )
{
	int			nPreviewHScrollPos;
	SCROLLINFO	si;
	int			nNowPos;
	int			nMove;
	int			nNewPos;
	int			nScrollCode;
	int			nPos;
	HWND		hwndScrollBar;
	nScrollCode = (int) LOWORD(wParam);
	nPos = (int) HIWORD(wParam);
	hwndScrollBar = (HWND) lParam;
	si.cbSize = sizeof( si );
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	::GetScrollInfo( hwndScrollBar, SB_CTL, &si );
	nNowPos = ::GetScrollPos( hwndScrollBar, SB_CTL );
	nMove = 0;
	switch( nScrollCode ){
	case SB_LINEUP:
		nMove = -1 * LINE_RANGE_Y;
		break;
	case SB_LINEDOWN:
		nMove = LINE_RANGE_Y;
		break;
	case SB_PAGEUP:
		nMove = -1 * PAGE_RANGE_Y;
		break;
	case SB_PAGEDOWN:
		nMove = PAGE_RANGE_Y;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nMove = nPos - nNowPos;
		break;
	// 2006.08.14 Moca SB_LEFT, SB_RIGHTへの対応
	case SB_LEFT:
		nMove = -1 * nNowPos;
		break;
	case SB_RIGHT:
		nMove = si.nMax - nNowPos;
	default:
		return 0;
	}
	nNewPos = nNowPos + nMove;
	if( nNewPos < 0 ){
		nNewPos = 0;
	}else
	if( nNewPos > (int)(si.nMax - si.nPage + 1) ){
		nNewPos = (int)(si.nMax - si.nPage + 1);
	}
	nMove = nNowPos - nNewPos;
	nPreviewHScrollPos = nNewPos;
	if( nPreviewHScrollPos != m_nPreviewHScrollPos ){
		::SetScrollPos( hwndScrollBar, SB_CTL, nNewPos, TRUE);
		m_nPreviewHScrollPos = nPreviewHScrollPos;
		/* 描画 */
		::ScrollWindowEx( m_pParentWnd->GetHwnd(), nMove, 0, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE );
	}
	return 0;
}

LRESULT CPrintPreview::OnMouseMove( WPARAM wParam, LPARAM lParam )
{
	/* 手カーソル */
	::SetCursor( ::LoadCursor( CEditApp::Instance()->GetAppInstance(), MAKEINTRESOURCE( IDC_CURSOR_HAND ) ) );
	if( !m_pParentWnd->GetDragMode() ){
		return 0;
	}
//	WPARAM		fwKeys = wParam;			// key flags
	int			xPos = LOWORD( lParam );	// horizontal position of cursor
	int			yPos = HIWORD( lParam );	// vertical position of cursor
	RECT		rc;
	GetClientRect( m_pParentWnd->GetHwnd(), &rc );
	POINT		po;
	po.x = xPos;
	po.y = yPos;
	if( !PtInRect( &rc, po ) ){	//	プレビュー内かチェック。
		return 0;
	}

	//	Y軸
	SCROLLINFO	siV;
	siV.cbSize = sizeof( siV );
	siV.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo( m_hwndVScrollBar, SB_CTL, &siV );
	int			nMoveY;
	if( m_SCROLLBAR_VERT ){
		int		nNowPosY = GetScrollPos( m_hwndVScrollBar, SB_CTL );
		nMoveY = m_pParentWnd->GetDragPosOrg().y - yPos;

		int		nNewPosY = nNowPosY + nMoveY;
		if( nNewPosY < 0 ){
			nNewPosY = 0;
		}else
		if( nNewPosY > (int)(siV.nMax - siV.nPage + 1) ){
			nNewPosY = (int)(siV.nMax - siV.nPage + 1);
		}
		nMoveY = nNowPosY - nNewPosY;
		SetScrollPos( m_hwndVScrollBar, SB_CTL, nNewPosY, TRUE );
		m_nPreviewVScrollPos = -1 * nNewPosY;
	}else{
		nMoveY = 0;
	}

	//	X軸
	SCROLLINFO	siH;
	siH.cbSize = sizeof( siH );
	siH.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo( m_hwndHScrollBar, SB_CTL, &siH );
	int			nMoveX;
	if( m_SCROLLBAR_HORZ ){
		int		nNowPosX = GetScrollPos( m_hwndHScrollBar, SB_CTL );
		nMoveX = m_pParentWnd->GetDragPosOrg().x - xPos;
		
		int		nNewPosX = nNowPosX + nMoveX;
		if( nNewPosX < 0 ){
			nNewPosX = 0;
		}else
		if( nNewPosX > (int)(siH.nMax - siH.nPage + 1) ){
			nNewPosX = (int)(siH.nMax - siH.nPage + 1);
		}
		nMoveX = nNowPosX - nNewPosX;
		SetScrollPos( m_hwndHScrollBar, SB_CTL, nNewPosX, TRUE );
		m_nPreviewHScrollPos = nNewPosX;
	}else{
		nMoveX = 0;
	}

	m_pParentWnd->SetDragPosOrg(CMyPoint(xPos,yPos));
	/* 描画 */
	ScrollWindowEx( m_pParentWnd->GetHwnd(), nMoveX, nMoveY, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE );
	return 0;
}

LRESULT CPrintPreview::OnMouseWheel( WPARAM wParam, LPARAM lParam )
{
//	WORD	fwKeys = LOWORD(wParam);			// key flags
	short	zDelta = (short) HIWORD(wParam);	// wheel rotation
//	short	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
//	short	yPos = (short) HIWORD(lParam);		// vertical position of pointer

	int		nScrollCode;
	if( 0 < zDelta ){
		nScrollCode = SB_LINEUP;
	}else{
		nScrollCode = SB_LINEDOWN;
	}

	int		i;
	for( i = 0; i < 3; ++i ){
		/* 印刷プレビュー 垂直スクロールバーメッセージ処理 WM_VSCROLL */
		::PostMessageAny( m_pParentWnd->GetHwnd(), WM_VSCROLL, MAKELONG( nScrollCode, 0 ), (LPARAM)m_hwndVScrollBar );

		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( NULL ) ){
			return -1;
		}
	}
	return 0;
}

void CPrintPreview::OnChangePrintSetting( void )
{
	HDC		hdc = ::GetDC( m_pParentWnd->GetHwnd() );
	::SetMapMode( hdc, MM_LOMETRIC ); //MM_HIMETRIC それぞれの論理単位は、0.01 mm にマップされます
	::SetMapMode( hdc, MM_ANISOTROPIC );

	::EnumFontFamilies(
		hdc,
		NULL,
		(FONTENUMPROC)CPrintPreview::MyEnumFontFamProc,
		(LPARAM)this
	);

	/* 印刷プレビュー表示情報 */
	m_nPreview_LineNumberColmns = 0;	/* 行番号エリアの幅(文字数) */

	/* 行番号を表示するか */
	if( m_pPrintSetting->m_bPrintLineNumber ){
		/* 行番号表示に必要な桁数を計算 */
		m_nPreview_LineNumberColmns = m_pParentWnd->m_pcEditViewArr[0]->GetTextArea().DetectWidthOfLineNumberArea_calculate();
	}
	/* 現在のページ設定の、用紙サイズと用紙方向を反映させる */
	m_pPrintSetting->m_mdmDevMode.dmPaperSize = m_pPrintSetting->m_nPrintPaperSize;
	m_pPrintSetting->m_mdmDevMode.dmOrientation = m_pPrintSetting->m_nPrintPaperOrientation;
	// 用紙サイズ、用紙方向は変更したのでビットを立てる
	m_pPrintSetting->m_mdmDevMode.dmFields |= ( DM_ORIENTATION | DM_PAPERSIZE );
	// 用紙の長さ、幅は決まっていないので、ビットを下ろす
	m_pPrintSetting->m_mdmDevMode.dmFields &= (~DM_PAPERLENGTH );
	m_pPrintSetting->m_mdmDevMode.dmFields &= (~DM_PAPERWIDTH);

	/* 印刷/プレビューに必要な情報を取得 */
	TCHAR	szErrMsg[1024];
	if( FALSE == m_cPrint.GetPrintMetrics(
		&m_pPrintSetting->m_mdmDevMode,	/* プリンタ設定 DEVMODE用*/
		&m_nPreview_PaperAllWidth,		/* 用紙幅 */
		&m_nPreview_PaperAllHeight,		/* 用紙高さ */
		&m_nPreview_PaperWidth,			/* 用紙印刷有効幅 */
		&m_nPreview_PaperHeight,		/* 用紙印刷有効高さ */
		&m_nPreview_PaperOffsetLeft,	/* 印刷可能位置左端 */
		&m_nPreview_PaperOffsetTop,		/* 印刷可能位置上端 */
		szErrMsg						/* エラーメッセージ格納場所 */
	) ){
		/* エラーの場合、A4縦(210mm×297mm)で初期化 */
		m_nPreview_PaperAllWidth = 210 * 10;	/* 用紙幅 */
		m_nPreview_PaperAllHeight = 297 * 10;	/* 用紙高さ */
		m_nPreview_PaperWidth = 210 * 10;		/* 用紙印刷有効幅 */
		m_nPreview_PaperHeight = 297 * 10;		/* 用紙印刷有効高さ */
		m_nPreview_PaperOffsetLeft = 0;			/* 印刷可能位置左端 */
		m_nPreview_PaperOffsetTop = 0;			/* 印刷可能位置上端 */
		// DEVMODE構造体もA4縦で初期化 2003.07.03 かろと
		m_pPrintSetting->m_mdmDevMode.dmPaperSize = DMPAPER_A4;
		m_pPrintSetting->m_mdmDevMode.dmOrientation = DMORIENT_PORTRAIT;
		m_pPrintSetting->m_mdmDevMode.dmPaperLength = m_nPreview_PaperHeight;
		m_pPrintSetting->m_mdmDevMode.dmPaperWidth = m_nPreview_PaperWidth;
		m_pPrintSetting->m_mdmDevMode.dmFields |= ( DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	}else{
		if( m_pPrintSetting->m_nPrintPaperSize != m_pPrintSetting->m_mdmDevMode.dmPaperSize ){
			TCHAR	szPaperNameOld[256];
			TCHAR	szPaperNameNew[256];
			/* 用紙の名前を取得 */
			CPrint::GetPaperName( m_pPrintSetting->m_nPrintPaperSize , szPaperNameOld );
			CPrint::GetPaperName( m_pPrintSetting->m_mdmDevMode.dmPaperSize , szPaperNameNew );

			TopWarningMessage(
				m_pParentWnd->GetHwnd(),
				_T("現在のプリンタ %ts では、\n指定された用紙 %ts は使用できません。\n")
				_T("利用可能な用紙 %ts に変更しました。"),
				m_pPrintSetting->m_mdmDevMode.m_szPrinterDeviceName,
				szPaperNameOld,
				szPaperNameNew
			);
		}
	}
	/* 現在のページ設定の、用紙サイズと用紙方向を反映させる(エラーでA4縦になった場合も考慮してif文の外へ移動 2003.07.03 かろと) */
	m_pPrintSetting->m_nPrintPaperSize = m_pPrintSetting->m_mdmDevMode.dmPaperSize;
	m_pPrintSetting->m_nPrintPaperOrientation = m_pPrintSetting->m_mdmDevMode.dmOrientation;	// 用紙方向の反映忘れを修正 2003/07/03 かろと

	m_nPreview_ViewMarginLeft = 8 * 10;		/* 印刷プレビュー：ビュー左端と用紙の間隔(1/10mm単位) */
	m_nPreview_ViewMarginTop = 8 * 10;		/* 印刷プレビュー：ビュー左端と用紙の間隔(1/10mm単位) */

	m_bPreview_EnableColms =
		CLayoutInt(
			( m_nPreview_PaperAllWidth - m_pPrintSetting->m_nPrintMarginLX - m_pPrintSetting->m_nPrintMarginRX
			- ( m_pPrintSetting->m_nPrintDansuu - 1 ) * m_pPrintSetting->m_nPrintDanSpace
			- ( m_pPrintSetting->m_nPrintDansuu ) * ( ( m_nPreview_LineNumberColmns /*+ (m_nPreview_LineNumberColmns?1:0)*/ ) * m_pPrintSetting->m_nPrintFontWidth )
			) / m_pPrintSetting->m_nPrintFontWidth / m_pPrintSetting->m_nPrintDansuu	/* 印字可能桁数/ページ */
		);
	m_bPreview_EnableLines = ( m_nPreview_PaperAllHeight - m_pPrintSetting->m_nPrintMarginTY - m_pPrintSetting->m_nPrintMarginBY ) / ( m_pPrintSetting->m_nPrintFontHeight + ( m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100 ) ) - 4;	/* 印字可能行数/ページ */

	/* 印刷用のレイアウト管理情報の初期化 */
	m_pLayoutMgr_Print->Create( &m_pParentWnd->GetDocument(), &m_pParentWnd->GetDocument().m_cDocLineMgr );

	/* 印刷用のレイアウト情報の変更 */
//	Types& ref = m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute();
	Types ref = m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute();
	ref.m_nMaxLineKetas = 		m_bPreview_EnableColms;
	ref.m_bWordWrap =			m_pPrintSetting->m_bPrintWordWrap;	/* 英文ワードラップをする */
	//	Sep. 23, 2002 genta LayoutMgrの値を使う
	ref.m_nTabSpace =			m_pParentWnd->GetDocument().m_cLayoutMgr.GetTabSpace();

	//@@@ 2002.09.22 YAZAKI
	ref.m_cLineComment.CopyTo(0, L"", -1);	/* 行コメントデリミタ */
	ref.m_cLineComment.CopyTo(1, L"", -1);	/* 行コメントデリミタ2 */
	ref.m_cLineComment.CopyTo(2, L"", -1);	/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
	ref.m_cBlockComment.CopyTo(0, L"", L"");	/* ブロックコメントデリミタ */
	ref.m_cBlockComment.CopyTo(1, L"", L"");	/* ブロックコメントデリミタ2 */

	ref.m_nStringType =			0;		/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	ref.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp = FALSE;
	ref.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	ref.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
	ref.m_bKinsokuHead = m_pPrintSetting->m_bPrintKinsokuHead,	/* 行頭禁則する */	//@@@ 2002.04.08 MIK
	ref.m_bKinsokuTail = m_pPrintSetting->m_bPrintKinsokuTail,	/* 行末禁則する */	//@@@ 2002.04.08 MIK
	ref.m_bKinsokuRet = m_pPrintSetting->m_bPrintKinsokuRet,	/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
	ref.m_bKinsokuKuto = m_pPrintSetting->m_bPrintKinsokuKuto,	/* 句読点をぶら下げる */	//@@@ 2002.04.17 MIK
	m_pLayoutMgr_Print->SetLayoutInfo(
		TRUE,	
		ref
	);
	m_nAllPageNum = (Int)m_pLayoutMgr_Print->GetLineCount() / ( m_bPreview_EnableLines * m_pPrintSetting->m_nPrintDansuu );		/* 全ページ数 */
	if( 0 < (Int)m_pLayoutMgr_Print->GetLineCount() % ( m_bPreview_EnableLines * m_pPrintSetting->m_nPrintDansuu ) ){
		m_nAllPageNum++;
	}
	if( m_nAllPageNum <= m_nCurPageNum ){	/* 現在のページ */
		m_nCurPageNum = 0;
	}

	/* WM_SIZE 処理 */
	RECT	rc;
	::GetClientRect( m_pParentWnd->GetHwnd(), &rc );
	OnSize( SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
	::ReleaseDC( m_pParentWnd->GetHwnd(), hdc );
	/* プレビュー ページ指定 */
	OnPreviewGoPage( m_nCurPageNum );
	return;
}

/*! @brief ページ番号直接指定によるジャンプ

	@author Moca
**/
void CPrintPreview::OnPreviewGoDirectPage( void )
{
	const int  INPUT_PAGE_NUM_LEN = 12;

	CDlgInput1 cDlgInputPage;
	TCHAR      szMessage[512];
	TCHAR      szPageNum[INPUT_PAGE_NUM_LEN];
	
	auto_sprintf( szMessage, _T("表示するページ番号を指定してください。(1 - %d)") , m_nAllPageNum );
	auto_sprintf( szPageNum, _T("%d"), m_nCurPageNum + 1 );

	BOOL bDlgInputPageResult=cDlgInputPage.DoModal(
		CEditApp::Instance()->GetAppInstance(),
		m_hwndPrintPreviewBar, 
		_T("プレビューページ指定"),
		szMessage,
		INPUT_PAGE_NUM_LEN,
		szPageNum
	);
	if( FALSE != bDlgInputPageResult ){
		int i;
		int nPageNumLen = _tcslen( szPageNum );
		for( i = 0; i < nPageNumLen;  i++ ){
			if( !(_T('0') <= szPageNum[i] &&  szPageNum[i] <= _T('9')) ){
				return;
			}
		}
		int nPage = _ttoi( szPageNum );
		OnPreviewGoPage( nPage - 1 );
	}
}

void CPrintPreview::OnPreviewGoPage( int nPage )
{
	if( m_nAllPageNum <= nPage ){	/* 現在のページ */
		nPage = m_nAllPageNum - 1;
	}
	if( 0 > nPage ){				/* 現在のページ */
		nPage = 0;
	}
	m_nCurPageNum = nPage;

	if( 0 == m_nCurPageNum ){
		//	最初のページのときは、前のページボタンをオフ。
		//	Jul. 18, 2001 genta FocusのあるWindowをDisableにすると操作できなくなるのを回避
		::SetFocus( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE ));
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_PREVPAGE ), FALSE );
	}else{
		//	前のページボタンをオン
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_PREVPAGE ), TRUE );
	}

	if( m_nAllPageNum <= m_nCurPageNum + 1 ){
		//	最後のページのときは、次のページボタンをオフ。
		//	Jul. 18, 2001 genta FocusのあるWindowをDisableにすると操作できなくなるのを回避
		//	Mar. 9, 2003 genta 1ページしか無いときは「前へ」ボタンもDisableされているので、
		//	最後のページまで達したら「戻る」にフォーカスを移すように
		::SetFocus( ::GetDlgItem( m_hwndPrintPreviewBar, IDCANCEL ));
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE ), FALSE );
	}else{
		//	次のページボタンをオン。
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE ), TRUE );
	}

	wchar_t	szEdit[1024];
	auto_sprintf( szEdit, L"%d/%d頁", m_nCurPageNum + 1, m_nAllPageNum );
	::DlgItem_SetText( m_hwndPrintPreviewBar, IDC_STATIC_PAGENUM, szEdit );

	auto_sprintf( szEdit, L"%d %%", m_nPreview_Zoom );
	::DlgItem_SetText( m_hwndPrintPreviewBar, IDC_STATIC_ZOOM, szEdit );

	::InvalidateRect( m_pParentWnd->GetHwnd(), NULL, TRUE );
	return;
}

void CPrintPreview::OnPreviewZoom( BOOL bZoomUp )
{
	if( bZoomUp ){
		m_nPreview_Zoom += 10;	/* 印刷プレビュー倍率 */
		if( MAX_PREVIEW_ZOOM < m_nPreview_Zoom ){
			m_nPreview_Zoom = MAX_PREVIEW_ZOOM;
		}
	}else{
		/* スクロール位置を調整 */
		m_nPreviewVScrollPos = 0;
		m_nPreviewHScrollPos = 0;

		m_nPreview_Zoom -= 10;	/* 印刷プレビュー倍率 */
		if( MIN_PREVIEW_ZOOM > m_nPreview_Zoom ){
			m_nPreview_Zoom = MIN_PREVIEW_ZOOM;
		}
	}
	
	//	縮小ボタンのON/OFF
	if( MIN_PREVIEW_ZOOM == m_nPreview_Zoom ){
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_ZOOMDOWN ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_ZOOMDOWN ), TRUE );
	}
	//	拡大ボタンのON/OFF
	if( MAX_PREVIEW_ZOOM == m_nPreview_Zoom ){
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_ZOOMUP ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_ZOOMUP ), TRUE );
	}

	wchar_t	szEdit[1024];
	auto_sprintf( szEdit, L"%d %%", m_nPreview_Zoom );
	::DlgItem_SetText( m_hwndPrintPreviewBar, IDC_STATIC_ZOOM, szEdit );

	/* WM_SIZE 処理 */
	RECT		rc1;
	::GetClientRect( m_pParentWnd->GetHwnd(), &rc1 );
	OnSize( SIZE_RESTORED, MAKELONG( rc1.right - rc1.left, rc1.bottom - rc1.top ) );

	/* 印刷プレビュー スクロールバー初期化 */
	InitPreviewScrollBar();

	/* 再描画 */
	::InvalidateRect( m_pParentWnd->GetHwnd(), NULL, TRUE );
	return;
}


/*!
	滑らか
	チェック時、2倍(COMPAT_BMP_SCALE/COMPAT_BMP_BASE)サイズでレンダリングする
*/
void CPrintPreview::OnCheckAntialias( void )
{
	/* WM_SIZE 処理 */
	RECT	rc;
	::GetClientRect( m_pParentWnd->GetHwnd(), &rc );
	OnSize( SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
}




void CPrintPreview::OnPrint( void )
{
	HDC			hdc;
	TCHAR		szJobName[256 + 1];
	TCHAR		szProgress[100];
	TCHAR		szErrMsg[1024];
	int			nDirectY = -1;
	int			i;
	HFONT		hFontOld;	//	OnPrint以前のフォント
	HFONT		hFontHan;	//	印刷用半角フォント
	HFONT		hFontZen;	//	印刷用全角フォント

	if( 0 == m_nAllPageNum ){
		TopWarningMessage( m_pParentWnd->GetHwnd(), _T("印刷するページがありません。") );
		return;
	}

	/* プリンタに渡すジョブ名を生成 */
	if( ! m_pParentWnd->GetDocument().m_cDocFile.GetFilePathClass().IsValidPath() ){	/* 現在編集中のファイルのパス */
		_tcscpy( szJobName, _T("無題") );
	}else{
		TCHAR	szFileName[_MAX_FNAME];
		TCHAR	szExt[_MAX_EXT];
		_tsplitpath( m_pParentWnd->GetDocument().m_cDocFile.GetFilePath(), NULL, NULL, szFileName, szExt );
		auto_sprintf( szJobName, _T("%ts%ts"), szFileName, szExt );
	}

	/* 印刷範囲を指定できるプリンタダイアログを作成 */
	//	2003.05.02 かろと
	PRINTDLG pd;
	memset_raw( &pd, 0, sizeof(pd) );
#ifndef _DEBUG
// Debugモードで、hwndOwnerを指定すると、Win2000では落ちるので・・・
	pd.hwndOwner = m_pParentWnd->GetHwnd();
#endif
	pd.nMinPage = 1;
	pd.nMaxPage = m_nAllPageNum;
	pd.nFromPage = 1;
	pd.nToPage = m_nAllPageNum;
	pd.Flags = PD_ALLPAGES | PD_NOSELECTION | PD_USEDEVMODECOPIESANDCOLLATE;
	if( !m_cPrint.PrintDlg(&pd, &m_pPrintSetting->m_mdmDevMode) ){
		return;
	}
	// 印刷開始ページと、印刷ページ数を確認
	int			nFrom;
	int			nNum;
	if( 0 != (pd.Flags & PD_PAGENUMS) ){	// 2003.05.02 かろと
		nFrom = pd.nFromPage - 1;
		nNum  = pd.nToPage - nFrom;
	}else{
		nFrom = 0;
		nNum  = m_nAllPageNum;
	}

	/* 印刷過程を表示して、キャンセルするためのダイアログを作成 */
	CDlgCancel	cDlgPrinting;
	cDlgPrinting.DoModeless( CEditApp::Instance()->GetAppInstance(), m_pParentWnd->GetHwnd(), IDD_PRINTING );
	::DlgItem_SetText( cDlgPrinting.GetHwnd(), IDC_STATIC_JOBNAME, szJobName );

	/* 親ウィンドウを無効化 */
	::EnableWindow( m_pParentWnd->GetHwnd(), FALSE );

	/* 印刷 ジョブ開始 */
	if( FALSE == m_cPrint.PrintOpen(
		szJobName,
		&m_pPrintSetting->m_mdmDevMode,	/* プリンタ設定 DEVMODE用*/
		&hdc,
		szErrMsg						/* エラーメッセージ格納場所 */
	) ){
//		MYTRACE_A( "%ts\n", szErrMsg );
	}

	/* 印刷用半角フォントと、印刷用全角フォントを作成 */
	hFontHan = CreateFontIndirect( &m_lfPreviewHan );
	hFontZen = CreateFontIndirect( &m_lfPreviewZen );

	// 現在のフォントを印刷用半角フォントに設定＆以前のフォントを保持
	hFontOld = (HFONT)::SelectObject( hdc, hFontHan );

	/* 紙の大きさをあらわすRECTを設定 */
	RECT cRect;
	cRect.left   =                             m_pPrintSetting->m_nPrintMarginLX - m_nPreview_PaperOffsetLeft + 5;
	cRect.right  = m_nPreview_PaperAllWidth - (m_pPrintSetting->m_nPrintMarginRX + m_nPreview_PaperOffsetLeft + 5);
	cRect.top    = nDirectY * (                              m_pPrintSetting->m_nPrintMarginTY - m_nPreview_PaperOffsetTop + 5 );
	cRect.bottom = nDirectY * ( m_nPreview_PaperAllHeight - (m_pPrintSetting->m_nPrintMarginBY + m_nPreview_PaperOffsetTop + 5) );

	/* ヘッダ・フッタの$pを展開するために、m_nCurPageNumを保持 */
	int nCurPageNumOld = m_nCurPageNum;
	for( i = 0; i < nNum; ++i ){
		m_nCurPageNum = nFrom + i;

		/* 印刷過程を表示 */
		//	Jun. 18, 2001 genta ページ番号表示の計算ミス修正
		auto_sprintf( szProgress, _T("%d/%d"), i + 1, nNum );
		::DlgItem_SetText( cDlgPrinting.GetHwnd(), IDC_STATIC_PROGRESS, szProgress );

		/* 印刷 ページ開始 */
		m_cPrint.PrintStartPage( hdc );

		//	From Here Jun. 26, 2003 かろと / おきた
		//	Windows 95/98ではStartPage()関数の呼び出し時に、属性はリセットされて既定値へ戻ります．
		//	このとき開発者は次のページの印刷を始める前にオブジェクトを選択し直し，
		//	マッピングモードをもう一度設定しなければなりません
		//	Windows NT/2000ではStartPageでも属性はリセットされません．
		
		/* マッピングモードの変更 */
		::SetMapMode( hdc, MM_LOMETRIC );		//それぞれの論理単位は、0.1 mm にマップされます
		::SetMapMode( hdc, MM_ANISOTROPIC );	//論理単位は、任意にスケーリングされた軸上の任意の単位にマップされます

		// 現在のフォントを印刷用半角フォントに設定
		::SelectObject( hdc, hFontHan );
		//	To Here Jun. 26, 2003 かろと / おきた

		/* ヘッダ印刷 */
		DrawHeader( hdc, cRect, hFontZen );

		/* 印刷/印刷プレビュー ページテキストの描画 */
		DrawPageText(
			hdc,
			m_pPrintSetting->m_nPrintMarginLX - m_nPreview_PaperOffsetLeft ,
			m_pPrintSetting->m_nPrintMarginTY - m_nPreview_PaperOffsetTop+ 2 * ( m_pPrintSetting->m_nPrintFontHeight + (m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100) ),
			nFrom + i,
			hFontZen,
			&cDlgPrinting
		);

		/* フッタ印刷 */
		DrawFooter( hdc, cRect, hFontZen );

		/* 印刷 ページ終了 */
		m_cPrint.PrintEndPage( hdc );

		/* 中断ボタン押下チェック */
		if( cDlgPrinting.IsCanceled() ){
			break;
		}
	}
	//	印刷前のフォントに戻す 2003.05.02 かろと hdc解放の前に処理順序を変更
	::SelectObject( hdc, hFontOld );

	/* 印刷 ジョブ終了 */
	m_cPrint.PrintClose( hdc );

	//	印刷用フォントを削除。
	::DeleteObject( hFontZen );
	::DeleteObject( hFontHan );

	::EnableWindow( m_pParentWnd->GetHwnd(), TRUE );
	cDlgPrinting.CloseDialog( 0 );

	m_nCurPageNum = nCurPageNumOld;

	// 印刷が終わったら、Previewから抜ける 2003.05.02 かろと
	m_pParentWnd->PrintPreviewModeONOFF();
	return;
}

/*! 印刷/印刷プレビュー ヘッダの描画
*/
void CPrintPreview::DrawHeader( HDC hdc, const CMyRect& rect, HFONT hFontZen )
{
	int nTextWidth = 0;

	// ヘッダ
	const int nHeaderWorkLen = 1024;
	wchar_t     szHeaderWork[1024 + 1];
	
	//文字間隔
	int nDx = m_pPrintSetting->m_nPrintFontWidth;

	// 左寄せ
	CSakuraEnvironment::ExpandParameter(m_pPrintSetting->m_szHeaderForm[POS_LEFT], szHeaderWork, nHeaderWorkLen);
	Print_DrawLine(
		hdc,
		rect.UpperLeft(),
		szHeaderWork,
		wcslen( szHeaderWork ),
		CLayoutInt(0),
		hFontZen
	);

	// 中央寄せ
	CSakuraEnvironment::ExpandParameter(m_pPrintSetting->m_szHeaderForm[POS_CENTER], szHeaderWork, nHeaderWorkLen);
	nTextWidth = CTextMetrics::CalcTextWidth2(szHeaderWork, wcslen(szHeaderWork), nDx); //テキスト幅
	Print_DrawLine(
		hdc,
		CMyPoint(
			( rect.right + rect.left - nTextWidth) / 2,
			rect.top
		),
		szHeaderWork,
		wcslen( szHeaderWork ),
		CLayoutInt(0),
		hFontZen
	);
	
	// 右寄せ
	CSakuraEnvironment::ExpandParameter(m_pPrintSetting->m_szHeaderForm[POS_RIGHT], szHeaderWork, nHeaderWorkLen);
	nTextWidth = CTextMetrics::CalcTextWidth2(szHeaderWork, wcslen(szHeaderWork), nDx); //テキスト幅
	Print_DrawLine(
		hdc,
		CMyPoint(
			rect.right - nTextWidth,
			rect.top
		),
		szHeaderWork,
		wcslen( szHeaderWork ),
		CLayoutInt(0),
		hFontZen
	);
}

/*! 印刷/印刷プレビュー フッタの描画
*/
void CPrintPreview::DrawFooter( HDC hdc, const CMyRect& rect, HFONT hFontZen )
{
	int nTextWidth = 0;

	// フッタ
	const int nFooterWorkLen = 1024;
	wchar_t   szFooterWork[1024 + 1];
	
	//文字間隔
	int nDx = m_pPrintSetting->m_nPrintFontWidth;

	// Y座標基準
	int nY = rect.bottom + m_pPrintSetting->m_nPrintFontHeight;

	// 左寄せ
	CSakuraEnvironment::ExpandParameter(m_pPrintSetting->m_szFooterForm[POS_LEFT], szFooterWork, nFooterWorkLen);
	Print_DrawLine(
		hdc,
		CMyPoint(
			rect.left,
			nY
		),
		szFooterWork,
		wcslen( szFooterWork ),
		CLayoutInt(0),
		hFontZen
	);

	// 中央寄せ
	CSakuraEnvironment::ExpandParameter(m_pPrintSetting->m_szFooterForm[POS_CENTER], szFooterWork, nFooterWorkLen);
	nTextWidth = CTextMetrics::CalcTextWidth2(szFooterWork, wcslen(szFooterWork), nDx); //テキスト幅
	Print_DrawLine(
		hdc,
		CMyPoint(
			( rect.right + rect.left - nTextWidth) / 2,
			nY
		),
		szFooterWork,
		wcslen( szFooterWork ),
		CLayoutInt(0),
		hFontZen
	);
	
	// 右寄せ
	CSakuraEnvironment::ExpandParameter(m_pPrintSetting->m_szFooterForm[POS_RIGHT], szFooterWork, nFooterWorkLen);
	nTextWidth = CTextMetrics::CalcTextWidth2(szFooterWork, wcslen(szFooterWork), nDx); //テキスト幅
	Print_DrawLine(
		hdc,
		CMyPoint(
			rect.right - nTextWidth,
			nY
		),
		szFooterWork,
		wcslen( szFooterWork ),
		CLayoutInt(0),
		hFontZen
	);
}

/* 印刷/印刷プレビュー ページテキストの描画
	DrawPageTextでは、行番号を（半角フォントで）印刷。
	本文はPrint_DrawLineにお任せ
	@date 2006.08.14 Moca 共通式のくくりだしと、コードの整理 
*/
void CPrintPreview::DrawPageText(
	HDC				hdc,
	int				nOffX,
	int				nOffY,
	int				nPageNum,
	HFONT			hFontZen,
	CDlgCancel*		pCDlgCancel
)
{
	int				nDirectY = -1;
	TEXTMETRIC		tm;

	const int		nLineHeight = m_pPrintSetting->m_nPrintFontHeight + ( m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100 );
	// 段と段の間隔の幅
	const int		nDanWidth = (Int)m_bPreview_EnableColms * m_pPrintSetting->m_nPrintFontWidth + m_pPrintSetting->m_nPrintDanSpace;
	// 行番号の幅
	const int		nLineNumWidth = m_nPreview_LineNumberColmns * m_pPrintSetting->m_nPrintFontWidth;

	/* 半角フォントの情報を取得＆半角フォントに設定 */
	::GetTextMetrics( hdc, &tm );
	int nAscentHan = tm.tmAscent;

	int				nDan;	//	段数カウンタ
	int				i;	//	行数カウンタ
	for( nDan = 0; nDan < m_pPrintSetting->m_nPrintDansuu; ++nDan ){
		// 本文1桁目の左隅の座標(行番号がある場合はこの座標より左側)
		const int nBasePosX = nOffX + nDanWidth * nDan + nLineNumWidth * (nDan + 1);
		
		for( i = 0; i < m_bPreview_EnableLines; ++i ){
			if( NULL != pCDlgCancel ){
				/* 処理中のユーザー操作を可能にする */
				if( !::BlockingHook( pCDlgCancel->GetHwnd() ) ){
					return;
				}
			}

			/*	現在描画しようとしている行の物理行数（折り返しごとにカウントした行数）
				関係するものは、
				「ページ数（nPageNum）」
				「段数（m_pPrintSetting->m_nPrintDansuu）」
				「段数が1のときに、1ページあたりに何行入るか（m_bPreview_EnableLines）」
			*/
			const CLayoutInt nLineNum = CLayoutInt( (nPageNum * m_pPrintSetting->m_nPrintDansuu + nDan) * m_bPreview_EnableLines + i );
			const CLayout*	pcLayout = m_pLayoutMgr_Print->SearchLineByLayoutY( nLineNum );
			if( NULL == pcLayout ){
				break;
			}
			/* 行番号を表示するか */
			if( m_pPrintSetting->m_bPrintLineNumber ){
				wchar_t		szLineNum[64];	//	行番号を入れる。
				/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
				if( m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute().m_bLineNumIsCRLF ){
					/* 論理行番号表示モード */
					if( 0 != pcLayout->GetLogicOffset() ){
						wcscpy( szLineNum, L" " );
					}else{
						_itow( pcLayout->GetLogicLineNo() + 1, szLineNum, 10 );	/* 対応する論理行番号 */
					}
				}
				else{
					/* 物理行(レイアウト行)番号表示モード */
					_itow( (Int)nLineNum + 1, szLineNum, 10 );
				}

				/* 行番号区切り  0=なし 1=縦線 2=任意 */
				if( 2 == m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute().m_nLineTermType ){
					wchar_t szLineTerm[2];
					auto_sprintf( szLineTerm, L"%lc", m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute().m_cLineTermChar );	/* 行番号区切り文字 */
					wcscat( szLineNum, szLineTerm );
				}
				else{
					wcscat( szLineNum, L" " );
				}

				//文字列長
				const int nLineCols = wcslen( szLineNum );

				//文字間隔配列を生成
				vector<int> vDxArray;
				const int* pDxArray = CTextMetrics::GenerateDxArray(&vDxArray, szLineNum, nLineCols, m_pPrintSetting->m_nPrintFontWidth);

				ApiWrap::ExtTextOutW_AnyBuild(
					hdc,
					nBasePosX - nLineCols * m_pPrintSetting->m_nPrintFontWidth,
					nDirectY * ( nOffY + nLineHeight * i + ( m_pPrintSetting->m_nPrintFontHeight - nAscentHan ) ),
					0,
					NULL,
					szLineNum,
					nLineCols,
					pDxArray
				);
			}

			const int nLineLen = pcLayout->GetLengthWithoutEOL();
			if( 0 == nLineLen ){
				continue;
			}

			// 印刷／プレビュー 行描画
			Print_DrawLine(
				hdc,
				CMyPoint(
					nBasePosX,
					nDirectY * ( nOffY + nLineHeight * i )
				),
				pcLayout->GetPtr(),
				nLineLen,
				pcLayout->GetIndent(), // 2006.05.16 Add Moca. レイアウトインデント分ずらす。
				hFontZen
			);
		}

		// 2006.08.14 Moca 行番号が縦線の場合は1度に引く
		if( m_pPrintSetting->m_bPrintLineNumber &&
				1 == m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute().m_nLineTermType ){
			// 縦線は本文と行番号の隙間1桁の中心に作画する(画面作画では、右詰め)
			::MoveToEx( hdc,
				nBasePosX - (m_pPrintSetting->m_nPrintFontWidth / 2 ),
				nDirectY * nOffY,
				NULL );
			::LineTo( hdc,
				nBasePosX - (m_pPrintSetting->m_nPrintFontWidth / 2 ),
				nDirectY * ( nOffY + nLineHeight * i )
			);
		}
	}
	return;
}






/* 印刷プレビュー スクロールバー初期化 */
void CPrintPreview::InitPreviewScrollBar( void )
{
	SCROLLINFO	si;
	RECT		rc;
	int			cx, cy;
	int			nToolBarHeight = 0;
	if( NULL != m_hwndPrintPreviewBar ){
		::GetWindowRect( m_hwndPrintPreviewBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
	}
	::GetClientRect( m_pParentWnd->GetHwnd(), &rc );
	cx = rc.right - rc.left;
	cy = rc.bottom - rc.top - nToolBarHeight;

	if( NULL != m_hwndVScrollBar ){
		/* 垂直スクロールバー */
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		if( m_nPreview_ViewHeight <= cy - nToolBarHeight ){
			si.nMax  = cy - nToolBarHeight;			/* 全幅 */
			si.nPage = cy - nToolBarHeight;			/* 表示域の桁数 */
			si.nPos  = -1 * m_nPreviewVScrollPos;	/* 表示域の一番左の位置 */
			si.nTrackPos = 0;
			m_SCROLLBAR_VERT = FALSE;
		}else{
			si.nMax  = m_nPreview_ViewHeight;		/* 全幅 */
			si.nPage = cy - nToolBarHeight;			/* 表示域の桁数 */
			si.nPos  = -1 * m_nPreviewVScrollPos;	/* 表示域の一番左の位置 */
			si.nTrackPos = 100;
			m_SCROLLBAR_VERT = TRUE;
		}
		::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
	}
	/* 印刷プレビュー 水平スクロールバーウィンドウハンドル */
	if( NULL != m_hwndHScrollBar ){
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		/* 水平スクロールバー */
//		si.cbSize = sizeof( si );
//		si.fMask = SIF_ALL;
		si.nMin  = 0;
		if( m_nPreview_ViewWidth <= cx ){
			si.nMax  = cx;							/* 全幅 */
			si.nPage = cx;							/* 表示域の桁数 */
			si.nPos  = m_nPreviewHScrollPos;		/* 表示域の一番左の位置 */
			si.nTrackPos = 0;
			m_SCROLLBAR_HORZ = FALSE;
		}else{
			si.nMax  = m_nPreview_ViewWidth;		/* 全幅 */
			si.nPage = cx;							/* 表示域の桁数 */
			si.nPos  = m_nPreviewHScrollPos;		/* 表示域の一番左の位置 */
			si.nTrackPos = 100;
			m_SCROLLBAR_HORZ = TRUE;
		}
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
	}
	return;
}

/*! 印刷／プレビュー 行描画
	@param[in] nIndent 行頭折り返しインデント桁数

	@date 2006.08.14 Moca   折り返しインデントが印刷時に反映されるように
	@date 2007.08    kobake 機械的にUNICODE化
	@date 2007.12.12 kobake 全角フォントが反映されていない問題を修正
*/
void CPrintPreview::Print_DrawLine(
	HDC				hdc,
	POINT			ptDraw, //!< 描画座標。HDC内部単位。
	const wchar_t*	pLine,
	int				nLineLen,
	CLayoutInt		nIndent,  // 2006.08.14 Moca 追加
	HFONT			hFontZen
)
{
	const int MODE_SINGLEBYTE = 1;
	const int MODE_DOUBLEBYTE = 2;

	TEXTMETRIC	tm;

	/* 全角文字のアセント（文字高）を取得 */
	HFONT	hFontHan = (HFONT)::SelectObject( hdc, hFontZen );
	::GetTextMetrics( hdc, &tm );
	int nAscentZen = tm.tmAscent;

	/* 半角文字のアセント（文字高）を取得 */
	//	半角文字用フォントが選択されていることを期待している。
	::SelectObject( hdc, hFontHan );
	::GetTextMetrics( hdc, &tm );
	int nAscentHan = tm.tmAscent;

	/*	pLineをスキャンして、半角文字は半角文字でまとめて、全角文字は全角文字でまとめて描画する。
	*/

	//文字間隔
	int nDx = m_pPrintSetting->m_nPrintFontWidth;

	//文字間隔配列を生成
	vector<int> vDxArray;
	const int* pDxArray = CTextMetrics::GenerateDxArray(
		&vDxArray,
		pLine,
		nLineLen,
		nDx
	);

	//タブ幅取得
	CLayoutInt nTabSpace = m_pParentWnd->GetDocument().m_cLayoutMgr.GetTabSpace(); //	Sep. 23, 2002 genta LayoutMgrの値を使う

	int nBgnLogic = 0;		// TABを展開する前のバイト数で、pLineの何バイト目まで描画したか？
	int iLogic;				// pLineの何文字目をスキャン？
	CLayoutInt nLayoutX = nIndent;	// TABを展開した後のバイト数で、テキストの何バイト目まで描画したか？

	//文字種判定フラグ
	int nKind     = 0; //0:半角 1:全角 2:タブ
	int nKindLast = 2; //直前のnKind状態

	for( iLogic = 0; iLogic < nLineLen; ++iLogic, nKindLast = nKind ){
		//文字の種類
		if(pLine[iLogic]==WCODE::TAB){
			nKind = 2;
		}
		else if(WCODE::isHankaku(pLine[iLogic])){
			nKind = 0;
		}
		else{
			nKind = 1;
		}

		// タブ文字出現 or 文字種の境界
		if( nKind == 2 || (nKindLast!=2 && nKind!=nKindLast)){
			//iLogicの直前までを描画
			if( 0 < iLogic - nBgnLogic ){
				// タブ文字の前までを描画。
				::SelectObject(hdc,nKindLast==0?hFontHan:hFontZen);
				::ExtTextOutW_AnyBuild(
					hdc,
					ptDraw.x + (Int)nLayoutX * nDx,
					ptDraw.y - ( m_pPrintSetting->m_nPrintFontHeight - (nKindLast==0?nAscentHan:nAscentZen) ),
					0,
					NULL,
					&pLine[nBgnLogic],
					iLogic - nBgnLogic,
					&pDxArray[nBgnLogic]
				);

				//桁進め
				for(int i=nBgnLogic;i<iLogic;i++){
					nLayoutX += CLayoutInt(pDxArray[i]/nDx);
				}

				//ロジック進め
				nBgnLogic = iLogic;
			}

			//タブ進め
			if(pLine[iLogic]==WCODE::TAB){
				//桁進め
				nLayoutX += ( nTabSpace - nLayoutX % nTabSpace );
				//ロジック進め
				nBgnLogic = iLogic + 1;
			}
		}
	}
	
	//残りを描画
	if( 0 < iLogic - nBgnLogic ){
		::SelectObject(hdc,nKindLast==0?hFontHan:hFontZen);
		::ExtTextOutW_AnyBuild(
			hdc,
			ptDraw.x + (Int)nLayoutX * nDx,
			ptDraw.y - ( m_pPrintSetting->m_nPrintFontHeight - (nKindLast==0?nAscentHan:nAscentZen) ),
			0,
			NULL,
			&pLine[nBgnLogic],
			iLogic - nBgnLogic,
			&pDxArray[nBgnLogic]
		);
	}

	//フォントを元 (半角) に戻す
	::SelectObject( hdc, hFontHan );
}

/*	印刷プレビューフォント（半角）を設定する
	typedef struct tagLOGFONT {
	   LONG lfHeight; 
	   LONG lfWidth; 
	   LONG lfEscapement; 
	   LONG lfOrientation; 
	   LONG lfWeight; 
	   BYTE lfItalic; 
	   BYTE lfUnderline; 
	   BYTE lfStrikeOut; 
	   BYTE lfCharSet; 
	   BYTE lfOutPrecision; 
	   BYTE lfClipPrecision; 
	   BYTE lfQuality; 
	   BYTE lfPitchAndFamily; 
	   TCHAR lfFaceName[LF_FACESIZE]; 
	} LOGFONT;
*/
void CPrintPreview::SetPreviewFontHan( const LOGFONT* lf )
{
	m_lfPreviewHan = *lf;

	//	PrintSettingからコピー
	m_lfPreviewHan.lfHeight			= m_pPrintSetting->m_nPrintFontHeight;
	m_lfPreviewHan.lfWidth			= m_pPrintSetting->m_nPrintFontWidth;
	_tcscpy(m_lfPreviewHan.lfFaceName, m_pPrintSetting->m_szPrintFontFaceHan);
}

void CPrintPreview::SetPreviewFontZen( const LOGFONT* lf )
{
	m_lfPreviewZen = *lf;
	//	PrintSettingからコピー
	m_lfPreviewZen.lfHeight	= m_pPrintSetting->m_nPrintFontHeight;
	m_lfPreviewZen.lfWidth	= m_pPrintSetting->m_nPrintFontWidth;
	_tcscpy(m_lfPreviewZen.lfFaceName, m_pPrintSetting->m_szPrintFontFaceZen );
}

int CALLBACK CPrintPreview::MyEnumFontFamProc(
	ENUMLOGFONT*	pelf,		// pointer to logical-font data
	NEWTEXTMETRIC*	pntm,		// pointer to physical-font data
	int				nFontType,	// type of font
	LPARAM			lParam 		// address of application-defined data
)
{
	CPrintPreview* pCPrintPreview = (CPrintPreview*)lParam;
	if( 0 == _tcscmp( pelf->elfLogFont.lfFaceName, pCPrintPreview->m_pPrintSetting->m_szPrintFontFaceHan ) ){
		pCPrintPreview->SetPreviewFontHan(&pelf->elfLogFont);
	}
	if( 0 == _tcscmp( pelf->elfLogFont.lfFaceName, pCPrintPreview->m_pPrintSetting->m_szPrintFontFaceZen ) ){
		pCPrintPreview->SetPreviewFontZen(&pelf->elfLogFont);
	}

	return 1;
}

/*!
	印刷プレビューに必要なコントロールを作成する
*/
void CPrintPreview::CreatePrintPreviewControls( void )
{
	int			nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	int			nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	int			nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	int			nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );

	/* 印刷プレビュー 操作バー */
	m_hwndPrintPreviewBar = ::CreateDialogParam(
		CEditApp::Instance()->GetAppInstance(),				// handle to application instance
		MAKEINTRESOURCE( IDD_PRINTPREVIEWBAR ),				// identifies dialog box template name
		m_pParentWnd->GetHwnd(),							// handle to owner window
		(DLGPROC)CPrintPreview::PrintPreviewBar_DlgProc,	// pointer to dialog box procedure
		(LPARAM)this
	);

	/* 縦スクロールバーの作成 */
	m_hwndVScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles			*/
		_T("SCROLLBAR"),						/* scroll bar control class		*/
		NULL,								/* text for window title bar	*/
		WS_VISIBLE | WS_CHILD | SBS_VERT,	/* scroll bar styles			*/
		0,									/* horizontal position			*/
		0,									/* vertical position			*/
		200,								/* width of the scroll bar		*/
		CW_USEDEFAULT,						/* default height				*/
		m_pParentWnd->GetHwnd(),								/* handle of main window		*/
		(HMENU) NULL,						/* no menu for a scroll bar		*/
		CEditApp::Instance()->GetAppInstance(),						/* instance owning this window	*/
		(LPVOID) NULL						/* pointer not needed			*/
	);
	SCROLLINFO	si;
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin	 = 0;
	si.nMax	 = 29;
	si.nPage = 10;
	si.nPos	 = 0;
	si.nTrackPos = 1;
	::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
	::ShowScrollBar( m_hwndVScrollBar, SB_CTL, TRUE );

	/* 横スクロールバーの作成 */
	m_hwndHScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles			*/
		_T("SCROLLBAR"),						/* scroll bar control class		*/
		NULL,								/* text for window title bar	*/
		WS_VISIBLE | WS_CHILD | SBS_HORZ,	/* scroll bar styles			*/
		0,									/* horizontal position			*/
		0,									/* vertical position			*/
		200,								/* width of the scroll bar		*/
		CW_USEDEFAULT,						/* default height				*/
		m_pParentWnd->GetHwnd(),								/* handle of main window		*/
		(HMENU) NULL,						/* no menu for a scroll bar		*/
		CEditApp::Instance()->GetAppInstance(),						/* instance owning this window	*/
		(LPVOID) NULL						/* pointer not needed			*/
	);
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin	 = 0;
	si.nMax	 = 29;
	si.nPage = 10;
	si.nPos	 = 0;
	si.nTrackPos = 1;
	::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
	::ShowScrollBar( m_hwndHScrollBar, SB_CTL, TRUE );

	/* サイズボックスの作成 */
	m_hwndSizeBox = ::CreateWindowEx(
		WS_EX_CONTROLPARENT/*0L*/, 							/* no extended styles			*/
		_T("SCROLLBAR"),										/* scroll bar control class		*/
		NULL,												/* text for window title bar	*/
		WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles			*/
		0,													/* horizontal position			*/
		0,													/* vertical position			*/
		200,												/* width of the scroll bar		*/
		CW_USEDEFAULT,										/* default height				*/
		m_pParentWnd->GetHwnd(), 											/* handle of main window		*/
		(HMENU) NULL,										/* no menu for a scroll bar 	*/
		CEditApp::Instance()->GetAppInstance(),										/* instance owning this window	*/
		(LPVOID) NULL										/* pointer not needed			*/
	);
	::ShowWindow( m_hwndPrintPreviewBar, SW_SHOW );


	/* WM_SIZE 処理 */
	RECT		rc1;
	::GetClientRect( m_pParentWnd->GetHwnd(), &rc1 );
	OnSize( SIZE_RESTORED, MAKELONG( rc1.right - rc1.left, rc1.bottom - rc1.top ) );
	return;
}


/*!
	印刷プレビューに必要だったコントロールを破棄する
*/
void CPrintPreview::DestroyPrintPreviewControls( void )
{
	/* 印刷プレビュー 操作バー 削除 */
	if ( m_hwndPrintPreviewBar ){
		::DestroyWindow( m_hwndPrintPreviewBar );
		m_hwndPrintPreviewBar = NULL;
	}

	/* 印刷プレビュー 垂直スクロールバーウィンドウ 削除 */
	if( m_hwndVScrollBar ){
		::DestroyWindow( m_hwndVScrollBar );
		m_hwndVScrollBar = NULL;
	}
	/* 印刷プレビュー 水平スクロールバーウィンドウ 削除 */
	if( m_hwndHScrollBar ){
		::DestroyWindow( m_hwndHScrollBar );
		m_hwndHScrollBar = NULL;
	}
	/* 印刷プレビュー サイズボックスウィンドウ 削除 */
	if ( m_hwndSizeBox ){
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
	}
}

/* ダイアログプロシージャ */
INT_PTR CALLBACK CPrintPreview::PrintPreviewBar_DlgProc(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
	WPARAM wParam,	// first message parameter
	LPARAM lParam 	// second message parameter
)
{
	CPrintPreview* pCPrintPreview;
	switch( uMsg ){
	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );
		// 2007.02.11 Moca WM_INITもDispatchEvent_PPBを呼ぶように
		pCPrintPreview = ( CPrintPreview* )lParam;
		if( NULL != pCPrintPreview ){
			return pCPrintPreview->DispatchEvent_PPB( hwndDlg, uMsg, wParam, lParam );
		}
		return TRUE;
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPrintPreview = ( CPrintPreview* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPrintPreview ){
			return pCPrintPreview->DispatchEvent_PPB( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}

/* 印刷プレビュー 操作バーにフォーカスを当てる */
void CPrintPreview::SetFocusToPrintPreviewBar( void )
{
	if( NULL != m_hwndPrintPreviewBar ){
		::SetFocus( m_hwndPrintPreviewBar );
	}
}

/* 印刷プレビュー 操作バー ダイアログのメッセージ処理 */
// IDD_PRINTPREVIEWBAR
INT_PTR CPrintPreview::DispatchEvent_PPB(
	HWND				hwndDlg,	// handle to dialog box
	UINT				uMsg,		// message
	WPARAM				wParam,		// first message parameter
	LPARAM				lParam 		// second message parameter
)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	CMemory				cMemBuf;



	switch( uMsg ){

	case WM_INITDIALOG:
		// 2007.02.11 Moca DWLP_USER設定は不要
		//// Modified by KEITA for WIN64 2003.9.6
		//::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );
		{
			COsVersionInfo cOsVer;
			if( cOsVer.IsWin2000_or_later() ){
				::EnableWindow( ::GetDlgItem(hwndDlg, IDC_CHECK_ANTIALIAS), TRUE );
			}
		}
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID、コントロールID またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_BUTTON_PRINTERSELECT:
				// From Here 2003.05.03 かろと
				// PRINTDLGを初期化
				PRINTDLG	pd;
				memset_raw( &pd, 0, sizeof(pd) );
				pd.Flags = PD_PRINTSETUP | PD_NONETWORKBUTTON;
				pd.hwndOwner = m_pParentWnd->GetHwnd();
				if (m_cPrint.PrintDlg( &pd, &m_pPrintSetting->m_mdmDevMode )) {
					// 用紙サイズと用紙方向を反映させる 2003.05.03 かろと
					m_pPrintSetting->m_nPrintPaperSize = m_pPrintSetting->m_mdmDevMode.dmPaperSize;
					m_pPrintSetting->m_nPrintPaperOrientation = m_pPrintSetting->m_mdmDevMode.dmOrientation;
					/* 印刷プレビュー スクロールバー初期化 */
					InitPreviewScrollBar();
					OnChangePrintSetting();
					::InvalidateRect( m_pParentWnd->GetHwnd(), NULL, TRUE );
				}
				// To Here 2003.05.03 かろと
				break;
			case IDC_BUTTON_PRINTSETTING:
				m_pParentWnd->OnPrintPageSetting();
				break;
			case IDC_BUTTON_ZOOMUP:
				/* プレビュー拡大縮小 */
				OnPreviewZoom( TRUE );
				break;
			case IDC_BUTTON_ZOOMDOWN:
				/* プレビュー拡大縮小 */
				OnPreviewZoom( FALSE );
				break;
			case IDC_BUTTON_PREVPAGE:
				/* 前ページ */
				OnPreviewGoPreviousPage( );
				break;
			case IDC_BUTTON_NEXTPAGE:
				/* 次ページ */
				OnPreviewGoNextPage( );
				break;
			//From Here 2007.02.11 Moca ダイレクトジャンプおよびアンチエイリアス
			case IDC_BUTTON_DIRECTPAGE:
				OnPreviewGoDirectPage( );
				break;
			case IDC_CHECK_ANTIALIAS:
				OnCheckAntialias();
				break;
			//To Here 2007.02.11 Moca
			case IDC_BUTTON_HELP:
				/* 印刷プレビューのヘルプ */
				//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
				MyWinHelp( hwndDlg, CEditApp::Instance()->GetHelpFilePath(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PRINT_PREVIEW) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
				break;
			case IDOK:
				/* 印刷実行 */
				OnPrint();
				return TRUE;
			case IDCANCEL:
				/* 印刷プレビューモードのオン/オフ */
				m_pParentWnd->PrintPreviewModeONOFF();
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	}
	return FALSE;
}


/*[EOF]*/
