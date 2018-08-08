/*!	@file
	@brief タブウィンドウ

	@author MIK
	@date 2003.5.30
	@date 2004.01.27 break漏れ対応。TCHAR化。タブ表示が崩れる(?)の対応。
*/
/*
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2004, Moca, MIK, genta, Kazika
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2012, Moca, syat, novice, uchi
	Copyright (C) 2013, Moca, Uchi, aroka, novice, syat, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include <limits.h>
#include "CTabWnd.h"
#include "window/CEditWnd.h"
#include "_main/global.h"
#include "_os/COSVersionInfo.h"
#include "charset/charcode.h"
#include "extmodule/CUxTheme.h"
#include "env/CShareData.h"
#include "env/CSakuraEnvironment.h"
#include "uiparts/CGraphics.h"
#include "recent/CRecentEditNode.h"
#include "util/os.h" //WM_THEMECHANGED
#include "util/window.h"
#include "util/module.h"
#include "util/string_ex2.h"
#include "sakura_rc.h"


//#if(WINVER >= 0x0500)
#ifndef	SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT        0x2000
#endif
#ifndef	SPI_SETFOREGROUNDLOCKTIMEOUT
#define SPI_SETFOREGROUNDLOCKTIMEOUT        0x2001
#endif
//#endif

// 2006.01.30 ryoji タブのサイズ／位置に関する定義
// 2009.10.01 ryoji 高DPI対応スケーリング
#define TAB_MARGIN_TOP		DpiScaleY(3)
#define TAB_MARGIN_LEFT		DpiScaleX(1)
#define TAB_MARGIN_RIGHT	DpiScaleX(47)

//#define TAB_FONT_HEIGHT		DpiPointsToPixels(9)
#define TAB_FONT_HEIGHT		abs(GetDllShareData().m_Common.m_sTabBar.m_lf.lfHeight)
#define TAB_ITEM_HEIGHT		(TAB_FONT_HEIGHT + DpiScaleY(7))
#define TAB_WINDOW_HEIGHT	(TAB_ITEM_HEIGHT + TAB_MARGIN_TOP + 2)

#define MAX_TABITEM_WIDTH	DpiScaleX(GetDllShareData().m_Common.m_sTabBar.m_nTabMaxWidth)
#define MIN_TABITEM_WIDTH	DpiScaleX(GetDllShareData().m_Common.m_sTabBar.m_nTabMinWidth)
#define MIN_TABITEM_WIDTH_MULTI	DpiScaleX(GetDllShareData().m_Common.m_sTabBar.m_nTabMinWidthOnMulti)

#define CX_SMICON			DpiScaleX(16)
#define CY_SMICON			DpiScaleY(16)

static const RECT rcBtnBase = { 0, 0, 16, 16 };

// 2006.02.01 ryoji タブ一覧メニュー用データ
typedef struct {
	HWND	hwnd;
	int		iItem;
	int		iImage;
	TCHAR	szText[_MAX_PATH];
} TABMENU_DATA;

/*!	タブ一覧メニュー用データの qsort() コールバック処理
	@date 2006.02.01 ryoji 新規作成
*/
static int compTABMENU_DATA( const void *arg1, const void *arg2 )
{
	int ret;

	// ここは文字列ソート（tcscmp）ではなく単語ソート（lstrcmp）を使用する
	// 文字列ソート: "XYZ" が "ABC" と "abc" との間に割って入る
	// 単語ソート: "ABC" と "abc" とは隣接し "XYZ" はそれらの後ろに入る（実際の辞書と同様な順序）
	ret = ::lstrcmp( ((TABMENU_DATA*)arg1)->szText, ((TABMENU_DATA*)arg2)->szText );
	if( 0 == ret )
		ret = ((TABMENU_DATA*)arg1)->iItem - ((TABMENU_DATA*)arg2)->iItem;
	return ret;
}


WNDPROC	gm_pOldWndProc = NULL;

/* 本来の TabWnd ウィンドウプロシージャ呼び出し */
inline LRESULT CALLBACK DefTabWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( gm_pOldWndProc )
		return ::CallWindowProc( gm_pOldWndProc, hwnd, uMsg, wParam, lParam );
	else
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
}

/* TabWndウィンドウメッセージのコールバック関数 */
LRESULT CALLBACK TabWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CTabWnd	*pcTabWnd;

	// Modified by KEITA for WIN64 2003.9.6
	pcTabWnd = (CTabWnd*)::GetWindowLongPtr( hwnd, GWLP_USERDATA );

	if( pcTabWnd )
	{
		//return
		if( 0L == pcTabWnd->TabWndDispatchEvent( hwnd, uMsg, wParam, lParam ) )
			return 0L;
	}

	return DefTabWndProc( hwnd, uMsg, wParam, lParam );
}

/* メッセージ配送 */
LRESULT CTabWnd::TabWndDispatchEvent( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// 2005.09.01 ryoji タブ部のメッセージ処理を個別に関数化し、タブ順序変更の処理を追加
	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
		return OnTabLButtonDown( wParam, lParam );

	case WM_LBUTTONUP:
		return OnTabLButtonUp( wParam, lParam );

	case WM_MOUSEMOVE:
		return OnTabMouseMove( wParam, lParam );

	case WM_TIMER:
		return OnTabTimer( wParam, lParam );

	case WM_CAPTURECHANGED:
		return OnTabCaptureChanged( wParam, lParam );

	case WM_RBUTTONDOWN:
		return OnTabRButtonDown( wParam, lParam );

	case WM_RBUTTONUP:
		return OnTabRButtonUp( wParam, lParam );

	case WM_MBUTTONDOWN:
		return OnTabMButtonDown( wParam, lParam );

	case WM_MBUTTONUP:
		return OnTabMButtonUp( wParam, lParam );

	case WM_NOTIFY:
		return OnTabNotify( wParam, lParam );

	case WM_HSCROLL:
		::InvalidateRect( GetHwnd(), NULL, TRUE );	// アクティブタブの位置が変わるのでトップバンドを更新する	// 2006.03.27 ryoji
		break;

	case WM_THEMECHANGED:
		m_bVisualStyle = ::IsVisualStyle();
		break;

	//default:
	}

	return 1L;	//デフォルトのディスパッチにまわす
}

/*! タブ部 WM_LBUTTONDOWN 処理 */
LRESULT CTabWnd::OnTabLButtonDown( WPARAM wParam, LPARAM lParam )
{
	// ボタンが押された位置を確認する
	TCHITTESTINFO hitinfo;
	hitinfo.pt.x = LOWORD( (DWORD)lParam );
	hitinfo.pt.y = HIWORD( (DWORD)lParam );
	int nSrcTab = TabCtrl_HitTest( m_hwndTab, (LPARAM)&hitinfo );
	if( 0 > nSrcTab )
		return 1L;

	// タブの閉じるボタン押下処理
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabClose ){
		// 閉じるボタンのチェック
		RECT rcItem;
		RECT rcClose;
		TabCtrl_GetItemRect(m_hwndTab, nSrcTab, &rcItem);
		GetTabCloseBtnRect(&rcItem, &rcClose, nSrcTab == TabCtrl_GetCurSel(m_hwndTab));
		if( ::PtInRect(&rcClose, hitinfo.pt) ){
			// 閉じるボタン上ならキャプチャー開始
			m_nTabCloseCapture = nSrcTab;
			::SetCapture( m_hwndTab );
			return 0L;
		}
	}

	// マウスドラッグ開始処理
	m_eDragState = DRAG_CHECK;	// ドラッグのチェックを開始

	// ドラッグ元タブを記憶する
	m_nSrcTab = nSrcTab;
	::GetCursorPos( &m_ptSrcCursor );

	::SetCapture( m_hwndTab );

	return 0L;
}

/*! タブ部 WM_LBUTTONUP 処理 */
LRESULT CTabWnd::OnTabLButtonUp( WPARAM wParam, LPARAM lParam )
{
	TCHITTESTINFO	hitinfo;
	hitinfo.pt.x = LOWORD( (DWORD)lParam );
	hitinfo.pt.y = HIWORD( (DWORD)lParam );
	int nDstTab = TabCtrl_HitTest( m_hwndTab, (LPARAM)&hitinfo );
	int nSelfTab = FindTabIndexByHWND( GetParentHwnd() );

	// タブの閉じるボタン押下処理
	if( m_nTabCloseCapture >= 0 ){	// タブ内の閉じるボタンが押し下げられていた?
		// 元の閉じるボタンと同一の閉じるボタン上ならタブを閉じる
		RECT rcItem;
		RECT rcClose;
		TabCtrl_GetItemRect(m_hwndTab, m_nTabCloseCapture, &rcItem);
		GetTabCloseBtnRect(&rcItem, &rcClose, m_nTabCloseCapture == TabCtrl_GetCurSel(m_hwndTab));
		if( ::PtInRect(&rcClose, hitinfo.pt) ){
			ExecTabCommand( F_WINCLOSE, MAKEPOINTS(lParam) );
		}
		// キャプチャー解除
		BreakDrag();
		return 0L;
	}

	// マウスドロップ処理
	switch( m_eDragState )
	{
	case DRAG_CHECK:
		if ( m_nSrcTab == nDstTab && m_nSrcTab != nSelfTab )
		{
			//指定のウインドウをアクティブに
			TCITEM	tcitem;
			tcitem.mask   = TCIF_PARAM;
			tcitem.lParam = 0;
			TabCtrl_GetItem( m_hwndTab, nDstTab, &tcitem );

			ShowHideWindow( (HWND)tcitem.lParam, TRUE );
		}
		break;

	case DRAG_DRAG:
		if ( 0 > nDstTab )	// タブの外でドロップ
		{
			// タブの分離処理
			if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
				HWND hwndAncestor;
				POINT ptCursor;

				::GetCursorPos( &ptCursor );
				hwndAncestor = MyGetAncestor( ::WindowFromPoint( ptCursor ), GA_ROOT );
				if( hwndAncestor != GetParentHwnd() )	// 自画面の外でドロップ
				{
					// タブ移動
					TCITEM	tcitem;
					tcitem.mask   = TCIF_PARAM;
					tcitem.lParam = 0;
					TabCtrl_GetItem( m_hwndTab, m_nSrcTab, &tcitem );
					HWND hwndSrc = (HWND)tcitem.lParam;
					HWND hwndDst = IsSakuraMainWindow( hwndAncestor )? hwndAncestor: NULL;

					SeparateGroup( hwndSrc, hwndDst, m_ptSrcCursor, ptCursor );
				}
			}
		}
		if ( m_bTabSwapped ) {
			// タブは移動済み。ほかのウィンドウのみ更新
			BroadcastRefreshToGroup();
		}
		if( m_nTabBorderArray ){
			delete[] m_nTabBorderArray;
			m_nTabBorderArray = NULL;
		}
		Tooltip_Activate( TabCtrl_GetToolTips( m_hwndTab ), TRUE );	// ツールチップ有効化
		break;

	default:
		break;
	}

	BreakDrag();	// 2006.01.28 ryoji ドラッグ状態を解除する(関数化)

	return 0L;
}

/*! タブ部 WM_MOUSEMOVE 処理 */
LRESULT CTabWnd::OnTabMouseMove( WPARAM wParam, LPARAM lParam )
{
	TCHITTESTINFO	hitinfo;
	int i;
	int nTabCount;
	hitinfo.pt.x = LOWORD( (DWORD)lParam );
	hitinfo.pt.y = HIWORD( (DWORD)lParam );
	int nDstTab = TabCtrl_HitTest( m_hwndTab, (LPARAM)&hitinfo );

	// 各タブの閉じるボタン描画用処理
	EDispTabClose bDispTabClose = m_pShareData->m_Common.m_sTabBar.m_bDispTabClose;
	if( bDispTabClose && ::GetCapture() != m_hwndTab ){
		int nTabHoverPrev = m_nTabHover;
		int nTabHoverCur = nDstTab;
		RECT rcPrev;
		RECT rcCur;
		RECT rcCurClose;
		TabCtrl_GetItemRect( m_hwndTab, nTabHoverPrev, &rcPrev );
		TabCtrl_GetItemRect( m_hwndTab, nTabHoverCur, &rcCur );
		GetTabCloseBtnRect(&rcCur, &rcCurClose, nTabHoverCur == TabCtrl_GetCurSel(m_hwndTab));

		m_nTabHover = nTabHoverCur;
		if( nTabHoverCur >= 0 ){	// カーソルがどれかタブ内にある
			if( nTabHoverPrev < 0 ){	// タブ外から入った
				::SetTimer( m_hwndTab, 1, 200, NULL );	// タイマー起動
			}

			// 閉じるボタンの自動表示
			if( bDispTabClose == DISPTABCLOSE_AUTO ){
				if( nTabHoverCur != nTabHoverPrev ){	// タブ外または別のタブから入った
					if( nTabHoverPrev >= 0 ){	// 別のタブから入った
						// 前回のタブを再描画する
						::InvalidateRect( m_hwndTab, &rcPrev, TRUE );
					}
					// このタブを再描画する
					::InvalidateRect( m_hwndTab, &rcCur, TRUE );
				}
			}

			// 閉じるボタン上のホバー状態を変える
			if( ::PtInRect(&rcCurClose, hitinfo.pt) ){	// 閉じるボタン上
				if( !m_bTabCloseHover || nTabHoverCur != nTabHoverPrev ){	// 以前はマウス下の閉じるボタンをハイライトしていなかった
					m_bTabCloseHover = true;
					if( nTabHoverCur != nTabHoverPrev ){
						// 前回のタブを再描画する
						::InvalidateRect( m_hwndTab, &rcPrev, TRUE );
					}
					// このタブを再描画する
					::InvalidateRect( m_hwndTab, &rcCur, TRUE );
				}
			}else{
				if( m_bTabCloseHover ){	// 閉じるボタンから出た
					// 前回、閉じるボタンをハイライトしていたタブを再描画する
					m_bTabCloseHover = false;
					::InvalidateRect( m_hwndTab, &rcPrev, TRUE );
				}
			}
		}else{	// カーソルがタブ外に出た
			::KillTimer( m_hwndTab, 1 );	// タイマー削除
			if( bDispTabClose == DISPTABCLOSE_AUTO || m_bTabCloseHover ){
				if( nTabHoverPrev >= 0 ){
					// 前回のタブを再描画する
					::InvalidateRect( m_hwndTab, &rcPrev, TRUE );
				}
			}
			m_bTabCloseHover = false;
		}
	}

	// マウスドラッグ中の処理
	switch( m_eDragState )
	{
	case DRAG_CHECK:
		// 元のタブから離れたらドラッグ開始
		if( m_nSrcTab == nDstTab )
			break;
		m_eDragState = DRAG_DRAG;
		m_hDefaultCursor = ::GetCursor();
		m_bTabSwapped = FALSE;

		// 現在のタブ境界位置を記憶する
		nTabCount = TabCtrl_GetItemCount(m_hwndTab);
		if( m_nTabBorderArray ){
			delete[] m_nTabBorderArray;
		}
		m_nTabBorderArray = new LONG[nTabCount];
		for (i = 0 ; i < nTabCount-1; i++) {
			RECT rc;
			TabCtrl_GetItemRect(m_hwndTab, i, &rc);
			m_nTabBorderArray[ i ] = rc.right;
		}
		m_nTabBorderArray[ i ] = 0;		// 最後の要素は番兵
		Tooltip_Activate( TabCtrl_GetToolTips( m_hwndTab ), FALSE );	// ツールチップ無効化
		// ここに来たらドラッグ開始なので break しないでそのまま DRAG_DRAG 処理に入る

	case DRAG_DRAG:
		// ドラッグ中のマウスカーソルを表示する
		HINSTANCE hInstance;
		LPCTSTR lpCursorName;
		lpCursorName = IDC_NO;	// 禁止カーソル
		if ( 0 <= nDstTab )	// タブの上にカーソルがある
		{
			lpCursorName = NULL;	// 開始時カーソル指定

			// ドラッグ開始時のタブ位置で移動先タブを再計算
			for( nDstTab = 0; m_nTabBorderArray[ nDstTab ] != 0; nDstTab++ ){
				if( hitinfo.pt.x < m_nTabBorderArray[ nDstTab ] ){
					break;
				}
			}

			// ドラッグ中に即時移動
			if( m_nSrcTab != nDstTab )
			{
				// 微調整：移動先タブの左端が負座標なら移動しない
				// ※ タブが多数あって左スクロール可能になっているときには、タブバー左端のほうの僅かな隙間に１個手前のタブが密かに存在する。
				RECT rc;
				TabCtrl_GetItemRect( m_hwndTab, nDstTab, &rc );
				if ( rc.left > 0 )
				{
					// TABまとめる => 自分だけ更新して後でRefresh通知
					// TABまとめない場合は、Refresh通知をした方がいいがマウスキャプチャが終了するので、まとめると同じ動きにする
					ReorderTab( m_nSrcTab, nDstTab );
					Refresh( FALSE );
					if( m_nTabHover == m_nSrcTab ){
						m_nTabHover = nDstTab;	// 自動表示の閉じるボタンも一緒に移動する
					}
					m_nSrcTab = nDstTab;
					m_bTabSwapped = TRUE;
					::InvalidateRect( GetHwnd(), NULL, TRUE );

					// 今回の WM_MOUSEMOVE が移動後のタブ上で発生したかのように偽装してマウスオーバーハイライトも移動する
					TabCtrl_GetItemRect(m_hwndTab, nDstTab, &rc);
					DefTabWndProc( m_hwndTab, WM_MOUSEMOVE, wParam, MAKELPARAM((rc.left + rc.right) / 2, HIWORD(lParam)) );
				}
			}
		}
		else
		{
			if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
			{
				HWND hwndAncestor;
				POINT ptCursor;

				::GetCursorPos( &ptCursor );
				hwndAncestor = MyGetAncestor( ::WindowFromPoint( ptCursor ), GA_ROOT );
				if( hwndAncestor != GetParentHwnd() )	// 自画面の外にカーソルがある
				{
					if( IsSakuraMainWindow( hwndAncestor ) )
						lpCursorName = MAKEINTRESOURCE(IDC_CURSOR_TAB_JOIN);	// 結合カーソル
					else
						lpCursorName = MAKEINTRESOURCE(IDC_CURSOR_TAB_SEPARATE);	// 分離カーソル
				}
			}
		}
		if( lpCursorName )
		{
			hInstance = (lpCursorName == IDC_NO)? NULL: ::GetModuleHandle( NULL );
			::SetCursor( ::LoadCursor( hInstance, lpCursorName ) );
		}
		else
		{
			::SetCursor( m_hDefaultCursor );
		}
		break;

	default:
		return 1L;
	}

	return 0L;
}

/*! タブ部 WM_TIMER 処理 */
LRESULT CTabWnd::OnTabTimer( WPARAM wParam, LPARAM lParam )
{
	if( wParam == 1 )
	{
		// カーソルがタブ外にある場合にも WM_MOUSEMOVE を送る
		TCHITTESTINFO	hitinfo;
		::GetCursorPos( &hitinfo.pt );
		::ScreenToClient( m_hwndTab, &hitinfo.pt );
		int nDstTab = TabCtrl_HitTest( m_hwndTab, (LPARAM)&hitinfo );
		if( nDstTab < 0 )
			::SendMessageAny( m_hwndTab, WM_MOUSEMOVE, 0, MAKELONG( hitinfo.pt.x, hitinfo.pt.y ) );
	}

	return 0L;
}

/*! タブ部 WM_CAPTURECHANGED 処理 */
LRESULT CTabWnd::OnTabCaptureChanged( WPARAM wParam, LPARAM lParam )
{
	if( m_eDragState != DRAG_NONE )
		m_eDragState = DRAG_NONE;
	return 0L;
}

/*! タブ部 WM_RBUTTONDOWN 処理 */
LRESULT CTabWnd::OnTabRButtonDown( WPARAM wParam, LPARAM lParam )
{
	BreakDrag();	// 2006.01.28 ryoji ドラッグ状態を解除する(関数化)

	return 0L;	// 2006.01.28 ryoji OnTabMButtonDown にあわせて 0 を返すように変更
}

/*! タブ部 WM_RBUTTONUP 処理 */
LRESULT CTabWnd::OnTabRButtonUp( WPARAM wParam, LPARAM lParam )
{
	// 2006.01.28 ryoji タブのカスタムメニュー表示コマンドを実行する(関数化)
	return ExecTabCommand( F_CUSTMENU_BASE + CUSTMENU_INDEX_FOR_TABWND, MAKEPOINTS(lParam) );
}

/*! タブ部 WM_MBUTTONDOWN 処理
	@date 2006.01.28 ryoji 新規作成
*/
LRESULT CTabWnd::OnTabMButtonDown( WPARAM wParam, LPARAM lParam )
{
	BreakDrag();	// 2006.01.28 ryoji ドラッグ状態を解除する(関数化)

	return 0L;	// フォーカスがタブに移らないよう、ここでは 0 を返す
}

/*! タブ部 WM_MBUTTONUP 処理
	@date 2006.01.28 ryoji 新規作成
*/
LRESULT CTabWnd::OnTabMButtonUp( WPARAM wParam, LPARAM lParam )
{
	// ウィンドウを閉じるコマンドを実行する
	return ExecTabCommand( F_WINCLOSE, MAKEPOINTS(lParam) );
}

/*! タブ部 WM_NOTIFY 処理

	@date 2005.09.01 ryoji 関数化
	@date 2006.10.31 ryoji ツールチップのフルパス名を簡易表示する
	@date 2007.12.06 ryoji ツールチップ処理をOnNotify()に移動（タブをTCS_TOOLTIPSスタイル化）
*/

LRESULT CTabWnd::OnTabNotify( WPARAM wParam, LPARAM lParam )
{
	// ツールチップ処理削除	// 2007.12.06 ryoji
	return 1L;
}

/*! @brief タブ順序変更処理

	@param[in] nSrcTab 移動するタブのインデックス
	@param[in] nDstTab 移動先タブのインデックス

	@date 2005.09.01 ryoji 新規作成
	@date 2007.07.07 genta ウィンドウリスト操作部をCShareData::ReorderTab()へ
	@date 2010.07.11 Moca ブロードキャスト部分を分離
*/
BOOL CTabWnd::ReorderTab( int nSrcTab, int nDstTab )
{
	TCITEM		tcitem;
	HWND		hwndSrc;	// 移動元ウィンドウ
	HWND		hwndDst;	// 移動先ウィンドウ

	if( 0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab )
		return FALSE;

	// 移動元タブ、移動先タブのウィンドウを取得する
	tcitem.mask   = TCIF_PARAM;
	tcitem.lParam = 0;
	TabCtrl_GetItem( m_hwndTab, nSrcTab, &tcitem );
	hwndSrc = (HWND)tcitem.lParam;

	tcitem.mask   = TCIF_PARAM;
	tcitem.lParam = 0;
	TabCtrl_GetItem( m_hwndTab, nDstTab, &tcitem );
	hwndDst = (HWND)tcitem.lParam;

	//	2007.07.07 genta CShareData::ReorderTabとして独立
	if( ! CAppNodeManager::getInstance()->ReorderTab( hwndSrc, hwndDst ) ){
		return FALSE;
	}
	return TRUE;
}

void CTabWnd::BroadcastRefreshToGroup()
{
	// 再表示メッセージをブロードキャストする。
	int nGroup = CAppNodeManager::getInstance()->GetEditNode( GetParentHwnd() )->GetGroup();
	CAppNodeGroupHandle(nGroup).PostMessageToAllEditors(
		MYWM_TAB_WINDOW_NOTIFY,
		(WPARAM)TWNT_REFRESH,
		(LPARAM)FALSE,
		GetParentHwnd()
	);
}

/** タブ分離処理

	アクティブなウィンドウ（先頭ウィンドウ）がアクティブを維持するように分離する
	※なるべく非アクティブにならないように分離し、非アクティブになってしまったら強制的に戻す

	@date 2007.06.20 ryoji 新規作成
	@date 2007.11.30 ryoji 強制的なアクティブ化戻しを追加して最大化時のタブ分離の封印を解除
*/
BOOL CTabWnd::SeparateGroup( HWND hwndSrc, HWND hwndDst, POINT ptDrag, POINT ptDrop )
{
	HWND hWnd = GetParentHwnd();
	EditNode* pTopEditNode = CAppNodeGroupHandle(0).GetEditNodeAt(0);
	if( pTopEditNode == NULL )
		return FALSE;
	if( hWnd != pTopEditNode->m_hWnd || hWnd != ::GetForegroundWindow() )
		return FALSE;
	if(	hWnd != CAppNodeManager::getInstance()->GetEditNode(hwndSrc)->GetGroup().GetTopEditNode()->GetHwnd() )
		return FALSE;
	if( hwndDst != NULL && hwndDst != CAppNodeManager::getInstance()->GetEditNode(hwndDst)->GetGroup().GetTopEditNode()->GetHwnd() )
		return FALSE;
	if( hwndSrc == hwndDst )
		return TRUE;

	EditNode* pSrcEditNode = CAppNodeManager::getInstance()->GetEditNode( hwndSrc );
	EditNode* pDstEditNode = CAppNodeManager::getInstance()->GetEditNode( hwndDst );
	int showCmdRestore = pDstEditNode? pDstEditNode->m_showCmdRestore: SW_SHOWNA;

	// グループ変更するウィンドウが先頭ウィンドウなら次のウィンドウを可視にする（手前には出さない）
	// そうでなければ新規グループになる場合に別ウィンドウよりは手前に表示されるよう不可視のまま先頭ウィンドウのすぐ後ろにもってきておく
	HWND hwndTop = CAppNodeManager::getInstance()->GetEditNode(hwndSrc)->GetGroup().GetTopEditNode()->GetHwnd();
	bool bSrcIsTop = ( hwndSrc == hwndTop );
	if( bSrcIsTop )
	{
		EditNode* pNextEditNode = CAppNodeGroupHandle(pSrcEditNode->m_nGroup).GetEditNodeAt( 1 );
		if( pNextEditNode != NULL )
		{
			DWORD_PTR dwResult;
			::SendMessageTimeout( pNextEditNode->m_hWnd, MYWM_TAB_WINDOW_NOTIFY, TWNT_WNDPL_ADJUST, (LPARAM)NULL,
									SMTO_ABORTIFHUNG | SMTO_BLOCK, 10000, &dwResult );
		}
	}
	else if( pDstEditNode == NULL )
	{
		::SetWindowPos( hwndSrc, hwndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
	}

	//	2007.07.07 genta 内部的なグループ移動の操作をCShareDataへ移動
	int notifygroups[2];
	hwndDst = CAppNodeManager::getInstance()->SeparateGroup( hwndSrc, hwndDst, bSrcIsTop, notifygroups );

	WINDOWPLACEMENT wp;
	RECT rcDstWork;
	GetMonitorWorkRect( ptDrop, &rcDstWork );
	wp.length = sizeof(wp);
	if( hwndDst == NULL )
	{	// 新規グループのウィンドウ処理
		// ウィンドウを移動先に表示する
		::GetWindowPlacement( hwndTop, &wp );
		if( wp.showCmd != SW_SHOWMAXIMIZED )
		{
			// 移動元の先頭ウィンドウのサイズで画面内を相対移動する
			wp.rcNormalPosition.left += (ptDrop.x - ptDrag.x);
			wp.rcNormalPosition.right += (ptDrop.x - ptDrag.x);
			wp.rcNormalPosition.top += (ptDrop.y - ptDrag.y);
			wp.rcNormalPosition.bottom += (ptDrop.y - ptDrag.y);

			// 上端がモニタ画面から出てしまわないように位置調整
			if( wp.rcNormalPosition.top < rcDstWork.top )
			{
				wp.rcNormalPosition.bottom += ( rcDstWork.top - wp.rcNormalPosition.top );
				wp.rcNormalPosition.top = rcDstWork.top;
			}
		}
		else
		{
			// 移動先モニタに最大表示する
			// （元に戻すサイズはモニタが異なる場合もモニタ内相対位置を維持するように移動しておく）
			RECT rcSrcWork;
			GetMonitorWorkRect( ptDrag, &rcSrcWork );
			wp.rcNormalPosition.left += (rcDstWork.left - rcSrcWork.left);
			wp.rcNormalPosition.right += (rcDstWork.left - rcSrcWork.left);
			wp.rcNormalPosition.top += (rcDstWork.top - rcSrcWork.top);
			wp.rcNormalPosition.bottom += (rcDstWork.top - rcSrcWork.top);

			// 元に戻すサイズがモニタ画面から出てしまわないように位置調整
			if( wp.rcNormalPosition.right > rcDstWork.right )
			{
				wp.rcNormalPosition.left -= (wp.rcNormalPosition.right - rcDstWork.right);
				wp.rcNormalPosition.right -= (wp.rcNormalPosition.right - rcDstWork.right);
			}
			if( wp.rcNormalPosition.bottom > rcDstWork.bottom )
			{
				wp.rcNormalPosition.top -= (wp.rcNormalPosition.bottom - rcDstWork.bottom);
				wp.rcNormalPosition.bottom -= (wp.rcNormalPosition.bottom - rcDstWork.bottom);
			}
			if( wp.rcNormalPosition.left < rcDstWork.left )
			{
				wp.rcNormalPosition.right += (rcDstWork.left - wp.rcNormalPosition.left);
				wp.rcNormalPosition.left += (rcDstWork.left - wp.rcNormalPosition.left);
			}
			if( wp.rcNormalPosition.top < rcDstWork.top )
			{
				wp.rcNormalPosition.bottom += (rcDstWork.top - wp.rcNormalPosition.top);
				wp.rcNormalPosition.top += (rcDstWork.top - wp.rcNormalPosition.top);
			}
		}

		SetCarmWindowPlacement( hwndSrc, &wp );
	}
	else
	{	// 既存グループのウィンドウ処理
		// 移動先の WS_EX_TOPMOST 状態を引き継ぐ
		if( bSrcIsTop )
		{	// 先頭ウィンドウの既存グループへの移動
			// 移動先の WS_EX_TOPMOST 状態を引き継ぐ
			HWND hWndInsertAfter = (::GetWindowLongPtr( hwndDst, GWL_EXSTYLE ) & WS_EX_TOPMOST)? HWND_TOPMOST: HWND_NOTOPMOST;
			::SetWindowPos( hwndSrc, hWndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

			// ウィンドウを移動先に表示する
			::GetWindowPlacement( hwndDst, &wp );
			if( wp.showCmd == SW_SHOWMINIMIZED )
				wp.showCmd = showCmdRestore;
			SetCarmWindowPlacement( hwndSrc, &wp );
			::ShowWindow( hwndDst, SW_HIDE );	// 移動先の以前の先頭ウィンドウを消す
		}
	}

	// 再表示メッセージをブロードキャストする。
	//	2007.07.07 genta 2回ループに
	for( int group = 0; group < _countof( notifygroups ); group++ ){
		CAppNodeGroupHandle(notifygroups[group]).PostMessageToAllEditors(
			MYWM_TAB_WINDOW_NOTIFY,
			(WPARAM)TWNT_REFRESH,
			(LPARAM)bSrcIsTop,
			NULL
		);
	}

	// 内部的な先頭ウィンドウが変化していたら元の先頭ウィンドウを先頭に戻す。
	// Note. 非アクティブウィンドウに対して次の操作を行った場合、画面が手前には出ない場合でも
	// アクティブ化は発生して内部管理のウィンドウ順序が変化してしまっている。
	// （内部管理のウィンドウ順序は各操作に同期して変化する）
	//   ・SW_SHOWMAXIMIZED操作
	//   ・元に戻すサイズが最大化のウィンドウに対するSW_SHOWNOACTIVATE操作
	// Windowsのアクティブウィンドウはスレッド単位に管理されるので複数のウィンドウがアクティブになっている場合がある。
	pTopEditNode = CAppNodeGroupHandle(0).GetEditNodeAt(0);	// 全体の先頭ウィンドウ情報を取得
	HWND hwndLastTop = pTopEditNode? pTopEditNode->m_hWnd: NULL;
	if( hwndLastTop != hwndTop )
	{
		HWND hwndFore = ::GetForegroundWindow();
		if( hwndFore == hwndTop )
		{	// 元の先頭ウィンドウが手前なのに最後にアクティブ化されたのが別ウィンドウになっている
			// 内部的な先頭ウィンドウを手前に出して状態整合（元の先頭ウィンドウは一旦非アクティブにする）
			SetForegroundWindow( hwndLastTop );
			hwndFore = ::GetForegroundWindow();
		}

		// 元の先頭ウィンドウ（自分）を手前に出す
		DWORD dwTidFore = ::GetWindowThreadProcessId( hwndFore, NULL );
		DWORD dwTidTop = ::GetWindowThreadProcessId( hwndTop, NULL );
		::AttachThreadInput( dwTidTop, dwTidFore, TRUE );
		SetForegroundWindow( hwndTop );
		::AttachThreadInput( dwTidTop, dwTidFore, FALSE );

		// WinXP Visual Styleでは上の操作で hwndLastTop が非アクティブ化しても
		// タイトルバー色だけアクティブのままということがあるかもしれない
	}

	return TRUE;
}

/*! タブ部 コマンド実行処理
	@date 2006.01.28 ryoji 新規作成
*/
LRESULT CTabWnd::ExecTabCommand( int nId, POINTS pts )
{
	// マウス位置(pt)のタブを取得する
	TCHITTESTINFO	hitinfo;
	hitinfo.pt.x = pts.x;
	hitinfo.pt.y = pts.y;
	int nTab = TabCtrl_HitTest( m_hwndTab, (LPARAM)&hitinfo );
	if( nTab < 0 )
		return 1L;

	// 対象ウィンドウを取得する
	TCITEM	tcitem;
	tcitem.mask   = TCIF_PARAM;
	tcitem.lParam = 0;
	if( !TabCtrl_GetItem( m_hwndTab, nTab, &tcitem ) )
		return 1L;
	HWND hWnd = (HWND)tcitem.lParam;

	// 対象ウインドウをアクティブにする。
	ShowHideWindow( hWnd, TRUE );

	// コマンドを対象ウインドウに送る。
	::PostMessageCmd( hWnd, WM_COMMAND, MAKEWPARAM( nId, 0 ), (LPARAM)NULL );

	return 0L;
}

CTabWnd::CTabWnd()
: CWnd(_T("::CTabWnd"))
, m_eTabPosition( TabPosition_None )
, m_eDragState( DRAG_NONE )
, m_bVisualStyle( FALSE )		// 2007.04.01 ryoji
, m_bHovering( FALSE )	//	2006.02.01 ryoji
, m_bListBtnHilighted( FALSE )	//	2006.02.01 ryoji
, m_bCloseBtnHilighted( FALSE )	//	2006.10.21 ryoji
, m_eCaptureSrc( CAPT_NONE )	//	2006.11.30 ryoji
, m_nTabBorderArray( NULL )		//  2012.04.22 syat
, m_nTabHover( -1 )
, m_bTabCloseHover( false )
, m_nTabCloseCapture( -1 )
,m_hwndSizeBox(NULL)
,m_bSizeBox(false)
{
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData();

	m_hwndTab    = NULL;
	m_hFont      = NULL;
	gm_pOldWndProc = NULL;
	m_hwndToolTip = NULL;
	m_hIml = NULL;

	// 2006.02.17 ryoji ImageList_Duplicate() のアドレスを取得する
	// （IE4.0 未満の環境でも動作可能なように動的ロード）
    HINSTANCE hinst = ::GetModuleHandle(TEXT("comctl32"));
    *(FARPROC*)&m_RealImageList_Duplicate = ::GetProcAddress(hinst, "ImageList_Duplicate");

	return;
}

CTabWnd::~CTabWnd()
{
	return;
}

/* ウィンドウ オープン */
HWND CTabWnd::Open( HINSTANCE hInstance, HWND hwndParent )
{
	LPCTSTR pszClassName = _T("CTabWnd");

	/* 初期化 */
	m_hwndTab    = NULL;
	m_hFont      = NULL;
	gm_pOldWndProc = NULL;
	m_hwndToolTip = NULL;
	m_bVisualStyle = ::IsVisualStyle();	// 2007.04.01 ryoji
	m_eDragState = DRAG_NONE;	//	2005.09.29 ryoji
	m_bHovering = FALSE;			// 2006.02.01 ryoji
	m_bListBtnHilighted = FALSE;	// 2006.02.01 ryoji
	m_bCloseBtnHilighted = FALSE;	// 2006.10.21 ryoji
	m_eCaptureSrc = CAPT_NONE;	// 2006.11.30 ryoji
	m_eTabPosition = TabPosition_None;

	/* ウィンドウクラス作成 */
	RegisterWC(
		hInstance,
		NULL,								// Handle to the class icon.
		NULL,								//Handle to a small icon
		::LoadCursor( NULL, IDC_ARROW ),	// Handle to the class cursor.
		// 2006.01.30 ryoji 背景は WM_PAINT で描画するほうがちらつかない（と思う）
		//(HBRUSH)(COLOR_3DFACE + 1),			// Handle to the class background brush.
		NULL,								// Handle to the class background brush.
		NULL,								// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		pszClassName						// Pointer to a null-terminated string or is an atom.
	);

	RECT rcParent;
	::GetWindowRect( hwndParent, &rcParent );

	/* 基底クラスメンバ呼び出し */
	CWnd::Create(
		hwndParent,
		0,									// extended window style
		pszClassName,						// Pointer to a null-terminated string or is an atom.
		pszClassName,						// pointer to window name
		WS_CHILD/* | WS_VISIBLE*/,			// window style	// 2007.03.08 ryoji WS_VISIBLE 除去
		// 2006.01.30 ryoji 初期配置見直し
		// ※タブ非表示 -> 表示切替で編集ウィンドウにゴミが表示されることがあるので初期幅はゼロに
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		rcParent.right - rcParent.left,		// window width
		TAB_WINDOW_HEIGHT,					// window height
		NULL								// handle to menu, or child-window identifier
	);

	//タブウインドウを作成する。
	m_hwndTab = ::CreateWindow(
		WC_TABCONTROL,
		_T(""),
		//	2004.05.22 MIK 消えるTAB対策でWS_CLIPSIBLINGS追加
		// 2007.12.06 ryoji TCS_TOOLTIPS追加（タブ用のツールチップはタブに作らせる）
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_TOOLTIPS,
		// 2006.01.30 ryoji 初期配置見直し
		TAB_MARGIN_LEFT,
		TAB_MARGIN_TOP,
		rcParent.right - rcParent.left - (TAB_MARGIN_LEFT + TAB_MARGIN_RIGHT),
		TAB_WINDOW_HEIGHT,
		GetHwnd(),
		(HMENU)NULL,
		GetAppInstance(),
		(LPVOID)NULL
		);
	if( m_hwndTab )
	{
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( m_hwndTab, GWLP_USERDATA, (LONG_PTR) this );
		gm_pOldWndProc = (WNDPROC)::SetWindowLongPtr( m_hwndTab, GWLP_WNDPROC, (LONG_PTR) TabWndProc );

		//スタイルを変更する。
		UINT lngStyle;
		lngStyle = (UINT)::GetWindowLongPtr( m_hwndTab, GWL_STYLE );
		//	Feb. 14, 2004 MIK マルチライン化の変更混入戻し
		lngStyle &= ~(TCS_BUTTONS | TCS_MULTILINE);
		if( m_pShareData->m_Common.m_sTabBar.m_bTabMultiLine ){
			lngStyle |= TCS_MULTILINE;
		}else{
			lngStyle |= TCS_SINGLELINE;
		}
		m_bMultiLine = m_pShareData->m_Common.m_sTabBar.m_bTabMultiLine;
		lngStyle |= TCS_TABS | TCS_FOCUSNEVER | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT;	// 2006.01.28 ryoji
		//lngStyle &= ~(TCS_BUTTONS | TCS_SINGLELINE);	//2004.01.31
		//lngStyle |= TCS_TABS | TCS_MULTILINE;
		::SetWindowLongPtr( m_hwndTab, GWL_STYLE, lngStyle );
		TabCtrl_SetItemSize( m_hwndTab, MAX_TABITEM_WIDTH, TAB_ITEM_HEIGHT );	// 2006.01.28 ryoji

		// タブのツールチップスタイルを変更する	// 2007.12.06 ryoji
		HWND hwndToolTips;
		hwndToolTips = TabCtrl_GetToolTips( m_hwndTab );
		lngStyle = (UINT)::GetWindowLongPtr( hwndToolTips, GWL_STYLE );
		lngStyle |= TTS_ALWAYSTIP | TTS_NOPREFIX;	// 従来通りTTS_ALWAYSTIPにしておく
		::SetWindowLongPtr( hwndToolTips, GWL_STYLE, lngStyle );

		/* 表示用フォント */
		/* LOGFONTの初期化 */
		m_lf = m_pShareData->m_Common.m_sTabBar.m_lf;
		m_hFont = ::CreateFontIndirect( &m_lf );
		/* フォント変更 */
		::SendMessageAny( m_hwndTab, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0) );

		//ツールチップを作成する。（タブではなく「閉じる」などのボタン用）
		//	2005.08.11 ryoji 「重ねて表示」のZ-orderがおかしくなるのでTOPMOST指定を解除
		m_hwndToolTip = ::CreateWindowEx(
			0,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			GetHwnd(), //m_hwndTab,
			NULL,
			GetAppInstance(),
			NULL
			);

		// ツールチップをマルチライン可能にする（SHRT_MAX: Win95でINT_MAXだと表示されない）	// 2007.03.03 ryoji
		Tooltip_SetMaxTipWidth( m_hwndToolTip, SHRT_MAX );

		// タブバーにツールチップを追加する
		TOOLINFO	ti;
		ti.cbSize      = CCSIZEOF_STRUCT(TOOLINFO, lpszText);
		ti.uFlags      = TTF_SUBCLASS | TTF_IDISHWND;	// TTF_IDISHWND: uId は HWND で rect は無視（HWND 全体）
		ti.hwnd        = GetHwnd();
		ti.hinst       = GetAppInstance();
		ti.uId         = (UINT_PTR)GetHwnd();
		ti.lpszText    = NULL;
		ti.rect.left   = 0;
		ti.rect.top    = 0;
		ti.rect.right  = 0;
		ti.rect.bottom = 0;
		Tooltip_AddTool( m_hwndToolTip, &ti );

		// 2006.02.22 ryoji イメージリストを初期化する
		InitImageList();

		Refresh();	// タブ非表示から表示に切り替わったときに各ウィンドウの情報をタブ登録する必要がある

		LayoutTab();
	}

	return GetHwnd();
}

void CTabWnd::UpdateStyle()
{
	if( m_bMultiLine != m_pShareData->m_Common.m_sTabBar.m_bTabMultiLine ){
		m_bMultiLine = m_pShareData->m_Common.m_sTabBar.m_bTabMultiLine;
		UINT lngStyle = (UINT)::GetWindowLongPtr( m_hwndTab, GWL_STYLE );
		if( m_pShareData->m_Common.m_sTabBar.m_bTabMultiLine ){
			lngStyle |= TCS_MULTILINE;
		}else{
			lngStyle &= ~TCS_MULTILINE;
		}
		::SetWindowLongPtr( m_hwndTab, GWL_STYLE, lngStyle );
	}
}

/* ウィンドウ クローズ */
void CTabWnd::Close( void )
{
	if( GetHwnd() )
	{
		if( gm_pOldWndProc )
		{
			// Modified by KEITA for WIN64 2003.9.6
			::SetWindowLongPtr( m_hwndTab, GWLP_WNDPROC, (LONG_PTR)gm_pOldWndProc );
			gm_pOldWndProc = NULL;
		}
		
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( m_hwndTab, GWLP_USERDATA, (LONG_PTR)NULL );

		if( m_hwndToolTip )
		{
			::DestroyWindow( m_hwndToolTip );
			m_hwndToolTip = NULL;
		}

		this->DestroyWindow();
	}
}

//WM_SIZE処理
LRESULT CTabWnd::OnSize( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( NULL == GetHwnd() || NULL == m_hwndTab ) return 0L;

	RECT rcWnd;
	::GetWindowRect( GetHwnd(), &rcWnd );

	int nSizeBoxWidth = 0;
	if( m_hwndSizeBox ){
		nSizeBoxWidth = ::GetSystemMetrics( SM_CXVSCROLL );
		int nSizeBoxHeight = ::GetSystemMetrics( SM_CYHSCROLL );
		::MoveWindow( m_hwndSizeBox,  rcWnd.right - rcWnd.left - nSizeBoxWidth,
			rcWnd.bottom - rcWnd.top - nSizeBoxHeight, nSizeBoxWidth, nSizeBoxHeight, TRUE );
	}

	LayoutTab();	// 2006.01.28 ryoji タブのレイアウト調整処理

	::InvalidateRect( GetHwnd(), NULL, FALSE );	//	2006.02.01 ryoji

	return 0L;
}

//WM_DSESTROY処理
LRESULT CTabWnd::OnDestroy( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	//タブコントロールを削除
	if( m_hwndTab )
	{
		::DestroyWindow( m_hwndTab );
		m_hwndTab = NULL;
	}

	//表示用フォント
	if( m_hFont )
	{
		::DeleteObject( m_hFont );
		m_hFont = NULL;
	}

	// 2006.01.28 ryoji イメージリストを削除
	if( NULL != m_hIml )
	{
		ImageList_Destroy( m_hIml );
		m_hIml = NULL;
	}

	::KillTimer( hwnd, 1 );	//	2006.02.01 ryoji

	_SetHwnd(NULL);

	return 0L;
}
 
/*! WM_LBUTTONDBLCLK処理
	@date 2006.03.26 ryoji 新規作成
*/
LRESULT CTabWnd::OnLButtonDblClk( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// 新規作成コマンドを実行する
	::SendMessageCmd( GetParentHwnd(), WM_COMMAND, MAKEWPARAM( F_FILENEW, 0 ), (LPARAM)NULL );
	return 0L;
}

/*!	WM_CAPTURECHANGED処理
	@date 2006.11.30 ryoji 新規作成
*/
LRESULT CTabWnd::OnCaptureChanged( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( m_eCaptureSrc != CAPT_NONE )
		m_eCaptureSrc = CAPT_NONE;

	return 0L;
}

/*!	WM_LBUTTONDOWN処理
	@date 2006.02.01 ryoji 新規作成
	@date 2006.11.30 ryoji タブ一覧ボタンクリック関数を廃止して処理取り込み
	                       閉じるボタン上ならキャプチャー開始
*/
LRESULT CTabWnd::OnLButtonDown( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	POINT pt;
	RECT rc;
	RECT rcBtn;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect( GetHwnd(), &rc );

	// タブ一覧ボタン上ならタブ一覧メニュー（タブ名）を表示する
	GetListBtnRect( &rc, &rcBtn );
	if( ::PtInRect( &rcBtn, pt ) )
	{
		pt.x = rcBtn.left;
		pt.y = rcBtn.bottom;
		::ClientToScreen( GetHwnd(), &pt );
		TabListMenu( pt, FALSE, FALSE, FALSE );	// タブ一覧メニュー（タブ名）
	}
	else
	{
		// 閉じるボタン上ならキャプチャー開始
		GetCloseBtnRect( &rc, &rcBtn );
		if( ::PtInRect( &rcBtn, pt ) )
		{
			m_eCaptureSrc = CAPT_CLOSE;	// キャプチャー元は閉じるボタン
			::SetCapture( GetHwnd() );
		}
	}

	return 0L;
}

/*!	WM_LBUTTONUP処理
	@date 2006.11.30 ryoji 新規作成
*/
LRESULT CTabWnd::OnLButtonUp( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	POINT pt;
	RECT rc;
	RECT rcBtn;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect( GetHwnd(), &rc );

	if( ::GetCapture() == GetHwnd() )	// 自ウィンドウがマウスキャプチャーしている?
	{
		if( m_eCaptureSrc == CAPT_CLOSE )	// キャプチャー元は閉じるボタン?
		{
			// 閉じるボタン上ならタブを閉じる
			GetCloseBtnRect( &rc, &rcBtn );
			if( ::PtInRect( &rcBtn, pt ) )
			{
				int nId;
				if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
				{
					if( !m_pShareData->m_Common.m_sTabBar.m_bTab_CloseOneWin )
					{
						nId = F_WINCLOSE;	// 閉じる（タイトルバーの閉じるボタンは編集の全終了）
					}
					else
					{
						nId = F_GROUPCLOSE;	// グループを閉じる
					}
				}
				else
				{
					nId = F_EXITALLEDITORS;	// 編集の全終了（タイトルバーの閉じるボタンは１個だけ閉じる）
				}
				::PostMessageAny( GetParentHwnd(), WM_COMMAND, MAKEWPARAM( nId, 0 ), (LPARAM)NULL );
			}
		}

		// キャプチャー解除
		m_eCaptureSrc = CAPT_NONE;
		::ReleaseCapture();
	}

	return 0L;
}

/*!	WM_RBUTTONDOWN処理
	@date 2006.02.01 ryoji 新規作成
	@date 2006.11.30 ryoji タブ一覧ボタンクリック関数を廃止して処理取り込み
*/
LRESULT CTabWnd::OnRButtonDown( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	POINT pt;
	RECT rc;
	RECT rcBtn;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect( GetHwnd(), &rc );

	// タブ一覧ボタン上ならタブ一覧メニュー（フルパス）を表示する	// 2006.11.30 ryoji
	GetListBtnRect( &rc, &rcBtn );
	if( ::PtInRect( &rcBtn, pt ) )
	{
		pt.x = rcBtn.left;
		pt.y = rcBtn.bottom;
		::ClientToScreen( GetHwnd(), &pt );
		TabListMenu( pt, FALSE, TRUE, FALSE );	// タブ一覧メニュー（フルパス）
	}

	return 0L;
}

/*!	WM_MEASUREITEM処理
	@date 2006.02.01 ryoji 新規作成
*/
LRESULT CTabWnd::OnMeasureItem( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	MEASUREITEMSTRUCT* lpmis = (MEASUREITEMSTRUCT*)lParam;
	if( lpmis->CtlType == ODT_MENU )
	{
		TABMENU_DATA* pData = (TABMENU_DATA*)lpmis->itemData;

		HDC hdc = ::GetDC( hwnd );
		HFONT hFont = CreateMenuFont();
		HFONT hFontOld = (HFONT)::SelectObject( hdc, hFont );

		SIZE size;
		::GetTextExtentPoint32( hdc, pData->szText, ::_tcslen(pData->szText), &size );

		int cxIcon = CX_SMICON;
		int cyIcon = CY_SMICON;
		if( NULL != m_hIml )
		{
			ImageList_GetIconSize( m_hIml, &cxIcon, &cyIcon );
		}

		lpmis->itemHeight = ::GetSystemMetrics( SM_CYMENU );
		lpmis->itemWidth = (cxIcon + DpiScaleX(8)) + size.cx;

		::SelectObject( hdc, hFontOld );
		::DeleteObject( hFont );
		::ReleaseDC( hwnd, hdc );
	}

	return 0L;
}

/*!	WM_DRAWITEM処理
	@date 2006.02.01 ryoji 新規作成
	@date 2012.04.14 syat タブのオーナードロー追加
*/
LRESULT CTabWnd::OnDrawItem( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*)lParam;
	if( lpdis->CtlType == ODT_MENU )
	{
		// タブ一覧メニューを描画する
		TABMENU_DATA* pData = (TABMENU_DATA*)lpdis->itemData;

		//描画対象
		HDC hdc = lpdis->hDC;
		CGraphics gr(hdc);
		RECT rcItem = lpdis->rcItem;

		// 状態に従ってテキストと背景色を決める
		COLORREF clrText;
		INT_PTR nSysClrBk;
		if (lpdis->itemState & ODS_SELECTED)
		{
			clrText = ::GetSysColor( COLOR_HIGHLIGHTTEXT );
			nSysClrBk = COLOR_HIGHLIGHT;
		}
		else
		{
			clrText = ::GetSysColor( COLOR_MENUTEXT );
			nSysClrBk = COLOR_MENU;
		}

		// 背景描画
		::FillRect( gr, &rcItem, (HBRUSH)(nSysClrBk + 1) );

		// アイコン描画
		int cxIcon = CX_SMICON;
		int cyIcon = CY_SMICON;
		if( NULL != m_hIml )
		{
			ImageList_GetIconSize( m_hIml, &cxIcon, &cyIcon );
			if( 0 <= pData->iImage )
			{
				int top = rcItem.top + ( rcItem.bottom - rcItem.top - cyIcon ) / 2;
				ImageList_Draw( m_hIml, pData->iImage, lpdis->hDC, rcItem.left + DpiScaleX(2), top, ILD_TRANSPARENT );
			}
		}

		// テキスト描画
		gr.PushTextForeColor( clrText );
		gr.SetTextBackTransparent(true);
		HFONT hFont = CreateMenuFont();
		gr.PushMyFont(hFont);
		RECT rcText = rcItem;
		rcText.left += (cxIcon + DpiScaleX(8));

		::DrawText( gr, pData->szText, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER );

		gr.PopTextForeColor();
		gr.PopMyFont();
		::DeleteObject( hFont );

		// チェック状態なら外枠描画
		if( lpdis->itemState & ODS_CHECKED )
		{
			gr.SetPen( ::GetSysColor(COLOR_HIGHLIGHT) );
			gr.SetBrushColor(-1); //NULL_BRUSH
			::Rectangle( gr, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom );
		}
	}
	else if( lpdis->CtlType == ODT_TAB ) {
		// タブを描画する
		int nTabIndex = lpdis->itemID;
		HWND hwndItem = lpdis->hwndItem;
		TCITEM item;
		TCHAR szBuf[256];
		int nSelIndex = TabCtrl_GetCurSel(m_hwndTab);
		bool bSelected = ( nSelIndex == nTabIndex );
		int nTabCount = TabCtrl_GetItemCount(m_hwndTab);

		item.mask = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
		item.pszText = szBuf;
		item.cchTextMax = sizeof(szBuf) / sizeof(TCHAR);
		TabCtrl_GetItem(hwndItem, nTabIndex, &item);

		//描画対象
		HDC hdc = lpdis->hDC;
		CGraphics gr(hdc);
		RECT rcItem = lpdis->rcItem;
		RECT rcFullItem(rcItem);

		// 状態に従ってテキストと背景色を決める

		// 背景描画
		if( !IsVisualStyle() ) {
			::FillRect( gr, &rcItem, (HBRUSH)(COLOR_BTNFACE + 1) );
		}else{
			CUxTheme& uxTheme = *CUxTheme::getInstance();
			int iPartId = TABP_TABITEM;
			int iStateId = TIS_NORMAL;
			HTHEME hTheme = uxTheme.OpenThemeData( m_hwndTab, L"TAB" );
			if( hTheme ) {
				if( !bSelected ){
					::InflateRect( &rcFullItem, DpiScaleX(2), DpiScaleY(2) );
					if( nTabIndex == nSelIndex - 1 ){
						rcFullItem.right -= DpiScaleX(1);
					}else if( nTabIndex == nSelIndex + 1 ){
						rcFullItem.left += DpiScaleX(1);
					}
				}
				bool bHotTracked = ::GetTextColor(hdc) == GetSysColor(COLOR_HOTLIGHT);

				RECT rcBk(rcFullItem);
				if( bSelected ){
					iStateId = TIS_SELECTED;
					if( nTabIndex == 0 ){
						if( nTabIndex == nTabCount - 1 ){
							iPartId = TABP_TOPTABITEMBOTHEDGE;
						}else{
							iPartId = TABP_TOPTABITEMLEFTEDGE;
						}
					}else if( nTabIndex == nTabCount - 1 ){
						iPartId = TABP_TOPTABITEMRIGHTEDGE;
					}else{
						iPartId = TABP_TOPTABITEM;
					}
				}else{
					rcFullItem.top += DpiScaleY(2);
					rcBk.top += DpiScaleY(2);
					iStateId = bHotTracked ? TIS_HOT : TIS_NORMAL;
					if( nTabIndex == 0 ){
						if( nTabIndex == nTabCount - 1 ){
							iPartId = TABP_TABITEMBOTHEDGE;
						}else{
							iPartId = TABP_TABITEMLEFTEDGE;
						}
					}else if( nTabIndex == nTabCount - 1 ){
						iPartId = TABP_TABITEMRIGHTEDGE;
					}else{
						iPartId = TABP_TABITEM;
					}
				}

				if( uxTheme.IsThemeBackgroundPartiallyTransparent(hTheme, iPartId, iStateId) ) {
					uxTheme.DrawThemeParentBackground(m_hwndTab, hdc, &rcFullItem);
				}
				uxTheme.DrawThemeBackground(hTheme, hdc, iPartId, iStateId, &rcBk, NULL);
			}
		}

		rcItem.left += DpiScaleX(4) + (bSelected ? DpiScaleX(4) : 0);

		// アイコン描画
		int cxIcon = CX_SMICON;
		int cyIcon = CY_SMICON;
		if( NULL != m_hIml )
		{
			ImageList_GetIconSize( m_hIml, &cxIcon, &cyIcon );
			if( 0 <= item.iImage )
			{
				int top = rcItem.top + ( rcItem.bottom - rcItem.top - cyIcon ) / 2 - 1;
				ImageList_Draw( m_hIml, item.iImage, lpdis->hDC, rcItem.left,
					top + (bSelected ? 0 : DpiScaleY(3)), ILD_TRANSPARENT );
				rcItem.left += cxIcon + DpiScaleX(6);
			}
		}

		// テキスト描画
		COLORREF clrText;
		clrText = ::GetSysColor(COLOR_MENUTEXT);
		gr.PushTextForeColor( clrText );
		gr.SetTextBackTransparent(true);
		RECT rcText = rcItem;
		rcText.top += (bSelected ? 0 : DpiScaleY(5)) - DpiScaleY(1);

		// テキスト矩形は最大でもタブを閉じるボタンの左端までに切り詰める
		// タブを閉じるボタンの矩形は他の箇所と同様 TabCtrl_GetItemRect の矩形から取得（lpdis->rcItem の矩形だと若干ずれる）
		EDispTabClose bDispTabClose = m_pShareData->m_Common.m_sTabBar.m_bDispTabClose;
		bool bDrawTabCloseBtn = (bDispTabClose == DISPTABCLOSE_ALLWAYS || (bDispTabClose == DISPTABCLOSE_AUTO && nTabIndex == m_nTabHover));
		RECT rcGetItemRect;
		TabCtrl_GetItemRect(m_hwndTab, nTabIndex, &rcGetItemRect);
		if( bDrawTabCloseBtn ){
			RECT rcClose;
			GetTabCloseBtnRect(&rcGetItemRect, &rcClose, nTabIndex == TabCtrl_GetCurSel(m_hwndTab));
			rcText.right = rcClose.left;
		}

		::DrawText( gr, szBuf, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER );

		gr.PopTextForeColor();

		// タブを閉じるボタンを描画
		if( bDrawTabCloseBtn ){
			DrawTabCloseBtn( gr, &rcGetItemRect, bSelected, (nTabIndex == m_nTabHover) && m_bTabCloseHover );
		}

		// Vista以降ではオーナードロータブに自動で3D枠が描画されてしまうため、
		// 描画範囲を無効化する
		if ( IsVisualStyle() ){
			ExcludeClipRect(hdc, rcFullItem.left, rcFullItem.top, rcFullItem.right, rcFullItem.bottom);
		}
	}

	return 0L;
}

/*!	WM_MOUSEMOVE処理
	@date 2006.02.01 ryoji 新規作成
	@date 2007.03.05 ryoji ボタンの出入りでツールチップを更新する
*/
LRESULT CTabWnd::OnMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// カーソルがウィンドウ内に入ったらタイマー起動
	// ウィンドウ外に出たらタイマー削除
	POINT pt;
	RECT rc;
	BOOL bHovering;

	pt.x = LOWORD( lParam );
	pt.y = HIWORD( lParam );
	::GetClientRect( hwnd, &rc );
	bHovering = ::PtInRect( &rc, pt );
	if( bHovering != m_bHovering )
	{
		m_bHovering = bHovering;
		if( m_bHovering )
			::SetTimer( hwnd, 1, 200, NULL );
		else
			::KillTimer( hwnd, 1 );
	}

	// カーソルがボタン上を出入りするときに再描画
	RECT rcBtn;
	LPTSTR pszTip = (LPTSTR)-1L;
	TCHAR szText[80];	// 2007.12.06 ryoji メンバ変数を使う必要は無いのでローカル変数にした

	GetListBtnRect( &rc, &rcBtn );
	bHovering = ::PtInRect( &rcBtn, pt );
	if( bHovering != m_bListBtnHilighted )
	{
		m_bListBtnHilighted = bHovering;
		::InvalidateRect( hwnd, &rcBtn, FALSE );

		// ツールチップ用の文字列作成	// 2007.03.05 ryoji
		pszTip = NULL;	// ボタンの外に出るときは消す
		if( m_bListBtnHilighted )	// ボタンに入ってきた?
		{
			pszTip = szText;
			_tcscpy( szText, LS(STR_TABWND_LR_INFO) );
		}
	}

	GetCloseBtnRect( &rc, &rcBtn );
	bHovering = ::PtInRect( &rcBtn, pt );
	if( bHovering != m_bCloseBtnHilighted )
	{
		m_bCloseBtnHilighted = bHovering;
		::InvalidateRect( hwnd, &rcBtn, FALSE );

		// ツールチップ用の文字列作成	// 2007.03.05 ryoji
		pszTip = NULL;	// ボタンの外に出るときは消す
		if( m_bCloseBtnHilighted )	// ボタンに入ってきた?
		{
			pszTip = szText;
			if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
			{
				if( !m_pShareData->m_Common.m_sTabBar.m_bTab_CloseOneWin )
				{
					_tcscpy( szText, LS(STR_TABWND_CLOSETAB) );
				}
				else
				{
					::LoadString( GetAppInstance(), F_GROUPCLOSE, szText, _countof(szText) );
					szText[_countof(szText) - 1] = _T('\0');
				}
			}
			else
			{
				::LoadString( GetAppInstance(), F_EXITALLEDITORS, szText, _countof(szText) );
				szText[_countof(szText) - 1] = _T('\0');
			}
		}
	}

	// ツールチップ更新	// 2007.03.05 ryoji
	if( pszTip != (LPTSTR)-1L )	// ボタンへの出入りがあった?
	{
		TOOLINFO ti;
		::ZeroMemory( &ti, sizeof(ti) );
		ti.cbSize       = CCSIZEOF_STRUCT(TOOLINFO, lpszText);
		ti.hwnd         = GetHwnd();
		ti.hinst        = GetAppInstance();
		ti.uId          = (UINT_PTR)GetHwnd();
		ti.lpszText     = pszTip;
		Tooltip_UpdateTipText( m_hwndToolTip, &ti );
	}

	return 0L;
}

/*!	WM_TIMER処理
	@date 2006.02.01 ryoji 新規作成
*/
LRESULT CTabWnd::OnTimer( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( wParam == 1 )
	{
		// カーソルがウィンドウ外にある場合にも WM_MOUSEMOVE を送る
		POINT pt;
		RECT rc;

		::GetCursorPos( &pt );
		::ScreenToClient( hwnd, &pt );
		::GetClientRect( hwnd, &rc );
		if( !::PtInRect( &rc, pt ) )
			::SendMessageAny( hwnd, WM_MOUSEMOVE, 0, MAKELONG( pt.x, pt.y ) );
	}

	return 0L;
}

/*!	WM_PAINT処理

	@date 2005.09.01 ryoji タブの上に境界線を追加
	@date 2006.01.30 ryoji 背景描画処理を追加（背景ブラシは NULL に変更）
	@date 2006.02.01 ryoji 一覧ボタンの描画処理を追加
	@date 2006.10.21 ryoji 閉じるボタンの描画処理を追加
	@date 2007.03.27 ryoji Windowsクラシックスタイルの場合はアクティブタブの上部にトップバンドを描画する
*/
LRESULT CTabWnd::OnPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rc;

	//描画対象
	hdc = ::BeginPaint( hwnd, &ps );
	CGraphics gr(hdc);

	// 背景を描画する
	::GetClientRect( hwnd, &rc );
	::FillRect( gr, &rc, (HBRUSH)(COLOR_3DFACE + 1) );

	// ボタンを描画する
	DrawListBtn( gr, &rc );
	DrawCloseBtn( gr, &rc );	// 2006.10.21 ryoji 追加

	// 上側に境界線を描画する
	::DrawEdge(gr, &rc, EDGE_ETCHED, BF_TOP);

	// Windowsクラシックスタイルの場合はアクティブタブの上部にトップバンドを描画する	// 2006.03.27 ryoji
	if( !m_bVisualStyle )
	{
		int nCurSel = TabCtrl_GetCurSel( m_hwndTab );
		if( nCurSel >= 0 )
		{
			POINT pt;
			RECT rcCurSel;

			TabCtrl_GetItemRect( m_hwndTab, nCurSel, &rcCurSel );
			pt.x = rcCurSel.left;
			pt.y = 0;
			::ClientToScreen( m_hwndTab, &pt );
			::ScreenToClient( GetHwnd(), &pt );
			rcCurSel.right = pt.x + (rcCurSel.right - rcCurSel.left) - 1;
			rcCurSel.left = pt.x + 1;
			rcCurSel.top = rc.top + TAB_MARGIN_TOP - 2;
			rcCurSel.bottom = rc.top + TAB_MARGIN_TOP;

			if( rcCurSel.left < rc.left + TAB_MARGIN_LEFT )
				rcCurSel.left = rc.left + TAB_MARGIN_LEFT;	// 左端限界値

			HWND hwndUpDown = ::FindWindowEx( m_hwndTab, NULL, UPDOWN_CLASS, 0 );	// タブ内の Up-Down コントロール
			if( hwndUpDown && ::IsWindowVisible( hwndUpDown ) )
			{
				POINT ptREnd;
				RECT rcUpDown;

				::GetWindowRect( hwndUpDown, &rcUpDown );
				ptREnd.x = rcUpDown.left;
				ptREnd.y = 0;
				::ScreenToClient( GetHwnd(), &ptREnd );
				if( rcCurSel.right > ptREnd.x )
					rcCurSel.right = ptREnd.x;	// 右端限界値
			}

			if( rcCurSel.left < rcCurSel.right )
			{
				HBRUSH hBr = ::CreateSolidBrush( RGB( 255, 128, 0 ) );
				::FillRect( gr, &rcCurSel, hBr );
				::DeleteObject( hBr );
			}
		}
	}

	// サイズボックスを描画する
	if (!m_pShareData->m_Common.m_sWindow.m_bDispSTATUSBAR 
		&& !m_pShareData->m_Common.m_sWindow.m_bDispFUNCKEYWND
		&& m_pShareData->m_Common.m_sTabBar.m_eTabPosition == TabPosition_Bottom) {
		SizeBox_ONOFF(true);
	}

	::EndPaint( hwnd, &ps );

	return 0L;
}

/*! WM_NOTIFY処理

	@date 2005.09.01 ryoji ウィンドウ切り替えは OnTabLButtonUp() に移動
	@date 2007.12.06 ryoji タブのツールチップ処理をOnTabNotify()から移動（タブをTCS_TOOLTIPSスタイル化）
*/
LRESULT CTabWnd::OnNotify( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// 2005.09.01 ryoji ウィンドウ切り替えは OnTabLButtonUp() に移動
	NMHDR* pnmh = (NMHDR*)lParam;
	if( pnmh->hwndFrom == TabCtrl_GetToolTips( m_hwndTab ) )
	{
		switch( pnmh->code )
		{
		//case TTN_NEEDTEXT:
		case TTN_GETDISPINFO:
			// ツールチップ表示情報を設定する
			TCITEM	tcitem;
			tcitem.mask   = TCIF_PARAM;
			tcitem.lParam = (LPARAM)NULL;
			if( TabCtrl_GetItem( m_hwndTab, pnmh->idFrom, &tcitem ) )
			{
				EditNode* pEditNode;
				pEditNode = CAppNodeManager::getInstance()->GetEditNode( (HWND)tcitem.lParam );
				GetTabName( pEditNode, TRUE, FALSE, m_szTextTip, _countof(m_szTextTip) );
				((NMTTDISPINFO*)pnmh)->lpszText = m_szTextTip;	// NMTTDISPINFO::szText[80]では短い
				((NMTTDISPINFO*)pnmh)->hinst = NULL;
			}
			return 0L;
		}
	}
	return 0L;
}

void CTabWnd::TabWindowNotify( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_hwndTab ) return;

	bool	bFlag = false;	//前回何もタブがなかったか？
	int		nCount;
	int		nIndex;
	HWND	hwndUpDown;
	DWORD nScrollPos;

	BreakDrag();	// 2006.01.28 ryoji ドラッグ状態を解除する(関数化)

	nCount = TabCtrl_GetItemCount( m_hwndTab );
	if( nCount <= 0 )
	{
		bFlag = true;
		//最初のときはすでに存在するウインドウの情報も登録する必要がある。
		// 起動時、CTabWnd::Open()内のRefresh()ではまだグループ入り前のため既に別ウィンドウがあってもタブは空
		if( wParam == TWNT_ADD )
			Refresh();	// 続けてTWNT_ADD処理で自分以外のウィンドウを隠す
	}

	switch( wParam )
	{
	case TWNT_ADD:	//ウインドウ登録
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 == nIndex )
		{
			TCITEM	tcitem;
			TCHAR	szName[1024];

			_tcscpy( szName, LS(STR_NO_TITLE1) );

			tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
			tcitem.pszText = szName;
			tcitem.lParam  = (LPARAM)lParam;

			// 2006.01.28 ryoji タブにアイコンイメージを追加する
			tcitem.mask |= TCIF_IMAGE;
			tcitem.iImage = GetImageIndex( NULL );

			TabCtrl_InsertItem( m_hwndTab, nCount, &tcitem );
			nIndex = nCount;
		}

		if( CAppNodeManager::getInstance()->GetEditNode( GetParentHwnd() )->IsTopInGroup() )
		{
			//自分ならアクティブに
			if( !::IsWindowVisible( GetParentHwnd() ) )
			{
				ShowHideWindow( GetParentHwnd(), TRUE );
				//ここに来たということはすでにアクティブ
				//コマンド実行時のアウトプットで問題があるのでアクティブにする
			}

			TabCtrl_SetCurSel( m_hwndTab, nIndex );

			// 自分以外を隠す
			HideOtherWindows( GetParentHwnd() );
		}
		break;

	case TWNT_DEL:	//ウインドウ削除
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 != nIndex )
		{
			if( CAppNodeManager::getInstance()->GetEditNode( GetParentHwnd() )->IsTopInGroup() )
			{
				if( !::IsWindowVisible( GetParentHwnd() ) )
				{
					ShowHideWindow( GetParentHwnd(), TRUE );
					ForceActiveWindow( GetParentHwnd() );
				}
			}
			TabCtrl_DeleteItem( m_hwndTab, nIndex );

			// 2005.09.01 ryoji スクロール位置調整
			// （右端のほうのタブアイテムを削除したとき、スクロール可能なのに右に余白ができることへの対策）
			hwndUpDown = ::FindWindowEx( m_hwndTab, NULL, UPDOWN_CLASS, 0 );	// タブ内の Up-Down コントロール
			if( hwndUpDown != NULL && ::IsWindowVisible( hwndUpDown ) )	// 2007.09.24 ryoji hwndUpDown可視の条件追加
			{
				nScrollPos = LOWORD( UpDown_GetPos( hwndUpDown ) );

				// 現在位置 nScrollPos と画面表示とを一致させる
				::SendMessageAny( m_hwndTab, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, LOWORD( nScrollPos ) ), (LPARAM)NULL );	// 設定位置にタブをスクロール
			}
		}
		break;

	case TWNT_ORDER:	//ウインドウ順序変更
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 != nIndex )
		{
			if( CAppNodeManager::getInstance()->GetEditNode(GetParentHwnd())->IsTopInGroup() )
			{
				//自分ならアクティブに
				if( !::IsWindowVisible( GetParentHwnd() ) )
				{
					ShowHideWindow( GetParentHwnd(), TRUE );
				}
				//ここに来たということはすでにアクティブ

				// 自タブアイテムを強制的に可視位置にするために、
				// 自タブアイテム選択前に一時的に画面左端のタブアイテムを選択する
				hwndUpDown = ::FindWindowEx( m_hwndTab, NULL, UPDOWN_CLASS, 0 );	// タブ内の Up-Down コントロール
				nScrollPos = ( hwndUpDown != NULL && ::IsWindowVisible( hwndUpDown ) )? LOWORD( UpDown_GetPos( hwndUpDown ) ): 0;	// 2007.09.24 ryoji hwndUpDown可視の条件追加
				TabCtrl_SetCurSel( m_hwndTab, nScrollPos );
				TabCtrl_SetCurSel( m_hwndTab, nIndex );

				// 自分以外を隠す
				// （連続切替時に TWNT_ORDER が大量発生・交錯して？画面がすべて消えてしまったりするのを防ぐ）
				HideOtherWindows( GetParentHwnd() );
			}
		}
		else
		{
			//指定のウインドウがないので再表示
			if( !CAppNodeManager::IsSameGroup( GetParentHwnd(), (HWND)lParam ) )
				Refresh();
		}
		break;

	case TWNT_FILE:	//ファイル名変更
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 != nIndex )
		{
			TCITEM	tcitem;
			CRecentEditNode	cRecentEditNode;
			TCHAR	szName[1024];
			//	Jun. 19, 2004 genta
			EditNode	*p;
			p = CAppNodeManager::getInstance()->GetEditNode( (HWND)lParam );
			GetTabName( p, FALSE, TRUE, szName, _countof(szName) );

			tcitem.mask    = TCIF_TEXT | TCIF_IMAGE;
			TCHAR	szNameOld[1024];
			tcitem.pszText = szNameOld;
			tcitem.cchTextMax = _countof(szNameOld);
			TabCtrl_GetItem( m_hwndTab, nIndex, &tcitem );
			if( 0 != auto_strcmp( szNameOld, szName )
				|| tcitem.iImage != GetImageIndex( p ) ){

				tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
				tcitem.pszText = szName;
				tcitem.lParam  = lParam;

				// 2006.01.28 ryoji タブのアイコンイメージを変更する
				tcitem.mask |= TCIF_IMAGE;
				tcitem.iImage = GetImageIndex( p );

				TabCtrl_SetItem( m_hwndTab, nIndex, &tcitem );
			}
		}
		else
		{
			//指定のウインドウがないので再表示
			if( !CAppNodeManager::IsSameGroup( GetParentHwnd(), (HWND)lParam ) )
				Refresh();
		}
		break;

	case TWNT_REFRESH:	//再表示
		Refresh( (BOOL)lParam );
		break;

	//Start 2004.07.14 Kazika 追加
	//タブモード有効になった場合、まとめられる側のウィンドウは隠れる
	case TWNT_MODE_ENABLE:
		Refresh();
		if( CAppNodeManager::getInstance()->GetEditNode( GetParentHwnd() )->IsTopInGroup() )
		{
			if( !::IsWindowVisible( GetParentHwnd() ) )
			{
				//表示状態とする(フォアグラウンドにはしない)
				TabWnd_ActivateFrameWindow( GetParentHwnd(), false );
			}
			// 自分以外を隠す
			HideOtherWindows( GetParentHwnd() );
		}
		break;
	//End 2004.07.14 Kazika

	//Start 2004.08.27 Kazika 追加
	//タブモード無効になった場合、隠れていたウィンドウは表示状態となる
	case TWNT_MODE_DISABLE:
		Refresh();
		if( !::IsWindowVisible( GetParentHwnd() ) )
		{
			//表示状態とする(フォアグラウンドにはしない)
			TabWnd_ActivateFrameWindow( GetParentHwnd(), false );
		}
		break;
	//End 2004.08.27 Kazika

	case TWNT_WNDPL_ADJUST:	// ウィンドウ位置合わせ	// 2007.04.03 ryoji
		AdjustWindowPlacement();
		return;

	default:
		break;
	}

	//タブの表示・非表示を切りかえる。
	nCount = TabCtrl_GetItemCount( m_hwndTab );
	if( nCount <= 0 )
	{
		::ShowWindow( m_hwndTab, SW_HIDE );
	}
	else
	{
		if( bFlag ) ::ShowWindow( m_hwndTab, SW_SHOW );
	}

//	LayoutTab();	// 2006.01.28 ryoji タブのレイアウト調整処理

	//更新
//	::InvalidateRect( m_hwndTab, NULL, FALSE );
//	::InvalidateRect( GetHwnd(), NULL, FALSE );		// 2006.10.21 ryoji タブ内ボタン再描画のために追加

	return;
}

/*! 指定のウインドウハンドル情報を持つタブ位置を探す */
int CTabWnd::FindTabIndexByHWND( HWND hWnd )
{
	int		i;
	int		nCount;
	TCITEM	tcitem;

	if( NULL == m_hwndTab ) return -1;

	nCount = TabCtrl_GetItemCount( m_hwndTab );
	for( i = 0; i < nCount; i++ )
	{
		tcitem.mask   = TCIF_PARAM;
		tcitem.lParam = (LPARAM)0;
		TabCtrl_GetItem( m_hwndTab, i, &tcitem );
		
		if( (HWND)tcitem.lParam == hWnd ) return i;
	}

	return -1;
}

/*! タブリストを再表示する

	@date 2004.06.19 genta &が含まれているファイル名が正しく表示されない
	@date 2006.02.06 ryoji 選択タブを指定するHWND引数およびその処理は不要なので削除（自ウィンドウを常時選択）
	@date 2006.06.24 ryoji スクロールしないで更新する方法を変更
*/
void CTabWnd::Refresh( BOOL bEnsureVisible/* = TRUE*/, BOOL bRebuild/* = FALSE*/ )
{
	TCITEM		tcitem;
	EditNode	*pEditNode;
	int			nCount;
	int			nGroup = 0;
	int			nTab;
	int			nSel = 0;
	int			nCurTab;
	int			nCurSel;
	int			i;
	int			j;

	if( NULL == m_hwndTab ) return;

	pEditNode = NULL;
	nCount = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );

	// 自ウィンドウのグループ番号を調べる
	for( i = 0; i < nCount; i++ )
	{
		if( pEditNode[i].m_hWnd == GetParentHwnd() ){
			nGroup = pEditNode[i].m_nGroup;
			break;
		}
	}

	if( i >= nCount )
	{
		// 見つからなかったので全タブを削除
		TabCtrl_DeleteAllItems( m_hwndTab );
	}
	else
	{
		::SendMessageAny( m_hwndTab, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0 );	// 2005.09.01 ryoji 再描画禁止

		if( bRebuild )
			TabCtrl_DeleteAllItems( m_hwndTab );	// 作成しなおす

		// 作成するタブ数と選択状態にするタブ位置（自ウィンドウの位置）を調べる
		for( i = 0, j = 0; i < nCount; i++ )
		{
			if( pEditNode[i].m_nGroup != nGroup )
				continue;
			if( pEditNode[i].m_bClosing )	// このあとすぐに閉じるウィンドウなのでタブ表示しない
				continue;
			if( pEditNode[i].m_hWnd == GetParentHwnd() )
				nSel = j;	// 選択状態にするタブ位置
			j++;
		}
		nTab = j;	// 作成するタブ数

		// タブが無ければ１つ作成して選択状態にする（自ウィンドウのタブ用）
		TCHAR		szName[2048];
		szName[0] = _T('\0');
		tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
		tcitem.pszText = szName;
		tcitem.lParam  = (LPARAM)GetParentHwnd();
		if( TabCtrl_GetItemCount( m_hwndTab ) == 0 )
		{
			TabCtrl_InsertItem( m_hwndTab, 0, &tcitem );
			TabCtrl_SetCurSel( m_hwndTab, 0 );
		}

		// 選択タブよりも前の過不足を調整する
		// （選択タブの直前位置への追加／削除を繰り返すことでスクロール発生を低減）
		nCurSel = TabCtrl_GetCurSel( m_hwndTab );	// 現在の選択タブ位置
		if( nCurSel > nSel )
		{
			for( i = 0; i < nCurSel - nSel; i++ )
				TabCtrl_DeleteItem( m_hwndTab, nCurSel - 1 - i );	// 余分を削除
		}
		else
		{
			for( i = 0; i < nSel - nCurSel; i++ )
				TabCtrl_InsertItem( m_hwndTab, nCurSel + i, &tcitem );	// 不足を追加
		}

		// 選択タブよりも後の過不足を調整する
		nCurTab = TabCtrl_GetItemCount( m_hwndTab );	// 現在のタブ数
		if( nCurTab > nTab )
		{
			for( i = 0; i < nCurTab - nTab; i++ )
				TabCtrl_DeleteItem( m_hwndTab, nSel + 1 );	// 余分を削除
		}
		else
		{
			for( i = 0; i < nTab - nCurTab; i++ )
				TabCtrl_InsertItem( m_hwndTab, nSel + 1, &tcitem );	// 不足を追加
		}

		// 作成したタブに各ウィンドウ情報を設定する
		for( i = 0, j = 0; i < nCount; i++ )
		{
			if( pEditNode[i].m_nGroup != nGroup )
				continue;
			if( pEditNode[i].m_bClosing )	// このあとすぐに閉じるウィンドウなのでタブ表示しない
				continue;

			GetTabName( &pEditNode[i], FALSE, TRUE, szName, _countof(szName) );

			tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
			tcitem.pszText = szName;
			tcitem.lParam  = (LPARAM)pEditNode[i].m_hWnd;

			// 2006.01.28 ryoji タブにアイコンを追加する
			tcitem.mask |= TCIF_IMAGE;
			tcitem.iImage = GetImageIndex( &pEditNode[i] );

			TabCtrl_SetItem( m_hwndTab, j, &tcitem );
			j++;
		}

		::SendMessageAny( m_hwndTab, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0 );	// 2005.09.01 ryoji 再描画許可

		// 選択タブを可視位置にする
		if( bEnsureVisible )
		{
			// TabCtrl_SetCurSel() を使うと等幅タブのときに選択タブが左端のほうに寄ってしまう
//			TabCtrl_SetCurSel( m_hwndTab, 0 );
//			TabCtrl_SetCurSel( m_hwndTab, nSel );
			::PostMessageAny( m_hwndTab, TCM_SETCURSEL, 0, 0 );
			::PostMessageAny( m_hwndTab, TCM_SETCURSEL, nSel, 0 );
		}
	}

	if( pEditNode ) delete[]pEditNode;

	return;
}


/*!	編集ウィンドウの位置合わせ

	@author ryoji
	@date 2007.04.03 新規作成
*/
void CTabWnd::AdjustWindowPlacement( void )
{
	// タブまとめ表示の場合は編集ウィンドウの表示位置を復元する
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		HWND hwnd = GetParentHwnd();	// 自身の編集ウィンドウ
		WINDOWPLACEMENT wp;
		if( !::IsWindowVisible( hwnd ) )	// 可視化するときだけ引き継ぐ
		{
			// なるべく画面を手前に出さずに可視化する
			// Note. 非アクティブスレッドから実行するのであればアクティブ化指定でも手前には出ない
			// Note. SW_xxxxx の中には「アクティブ化無しの最大化」指定は存在しない
			// Note. 不可視の状態からいきなり手前に出てしまうと次のような現象が起きる
			//  ・画面描画される際、クライアント領域全体が一時的に真っ白になる（Vista Aero）
			//  ・最大化切替（SW_SHOWMAXIMIZED）の際、以前に通常表示だった画面のステータスバーやファンクションキーが一時的に通常サイズで表示される

			// ウィンドウを背後に配置する
			// Note. WS_EX_TOPMOST については hwndInsertAfter ウィンドウの状態が引き継がれる
			EditNode* pEditNode;
			pEditNode = CAppNodeManager::getInstance()->GetEditNode(hwnd)->GetGroup().GetTopEditNode();
			if( pEditNode == NULL )
			{
				::ShowWindow( hwnd, SW_SHOWNA );
				return;
			}
			HWND hwndInsertAfter = pEditNode->m_hWnd;
			wp.length = sizeof( wp );
			::GetWindowPlacement( hwndInsertAfter, &wp );
			if( wp.showCmd == SW_SHOWMINIMIZED )
				wp.showCmd = pEditNode->m_showCmdRestore;
			::SetWindowPos( hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
			SetCarmWindowPlacement( hwnd, &wp );	// 位置を復元する
			::UpdateWindow( hwnd );	// 強制描画
		}
	}
}

/*!	アクティブ化の少ない SetWindowPlacement() を実行する

	@author ryoji
	@date 2007.11.30 新規作成
*/
int CTabWnd::SetCarmWindowPlacement( HWND hwnd, const WINDOWPLACEMENT* pWndpl )
{
	WINDOWPLACEMENT wp = *pWndpl;
	if( wp.showCmd == SW_SHOWMAXIMIZED && ::IsZoomed( hwnd ) )
	{
		WINDOWPLACEMENT wpCur;
		wpCur.length = sizeof( WINDOWPLACEMENT );
		::GetWindowPlacement( hwnd, &wpCur );
		if( !::EqualRect( &wp.rcNormalPosition, &wpCur.rcNormalPosition ) )
		{
			// ウィンドウの通常サイズが目的のサイズと違っているときは一旦通常サイズで表示してから最大化する
			// Note. マルチモニタで以前に別モニタで最大化されていた画面は一旦通常サイズに戻しておかないと元の別モニタ側に表示されてしまう
			// （本当はここは表示モニタが変わるときだけやればOKだけど、GetMonitorWorkRect()とSetWindowPlacementでは座標系がちょっと違うので）
			wp.showCmd = SW_SHOWNOACTIVATE;
			::SetWindowPlacement( hwnd, &wp );
			wp.showCmd = SW_SHOWMAXIMIZED;
		}
		else
		{
			wp.showCmd = SW_SHOWNA;	// そのまま最大表示
		}
	}
	else if( wp.showCmd != SW_SHOWMAXIMIZED )
	{
		wp.showCmd = SW_SHOWNOACTIVATE;
	}
	::SetWindowPlacement( hwnd, &wp );
	return wp.showCmd;
}

void CTabWnd::ShowHideWindow( HWND hwnd, BOOL bDisp )
{
	if( NULL == hwnd ) return;

	if( bDisp )
	{
		if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
		{
			if( m_pShareData->m_sFlags.m_bEditWndChanging )
				return;	// 切替の最中(busy)は要求を無視する
			m_pShareData->m_sFlags.m_bEditWndChanging = TRUE;	// 編集ウィンドウ切替中ON	2007.04.03 ryoji

			// 対象ウィンドウのスレッドに位置合わせを依頼する	// 2007.04.03 ryoji
			DWORD_PTR dwResult;
			::SendMessageTimeout( hwnd, MYWM_TAB_WINDOW_NOTIFY, TWNT_WNDPL_ADJUST, (LPARAM)NULL,
				SMTO_ABORTIFHUNG | SMTO_BLOCK, 10000, &dwResult );
		}

		TabWnd_ActivateFrameWindow( hwnd );

		m_pShareData->m_sFlags.m_bEditWndChanging = FALSE;	// 編集ウィンドウ切替中OFF	2007.04.03 ryoji
	}
	else
	{
		if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
		{
			::ShowWindow( hwnd, SW_HIDE );
		}
	}

	return;
}

/*!	他の編集ウィンドウを隠す

	@param hwndExclude [in] 非表示化から除外するウィンドウ

	@author ryoji
	@date 2007.05.17 新規作成
*/
void CTabWnd::HideOtherWindows( HWND hwndExclude )
{
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		HWND hwnd;
		int	i;
		for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; i++ )
		{
			hwnd = m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd;
			if( IsSakuraMainWindow( hwnd ) )
			{
				if( !CAppNodeManager::IsSameGroup( hwndExclude, hwnd ) )
					continue;
				if( hwnd != hwndExclude && ::IsWindowVisible( hwnd ) )
				{
					::ShowWindow( hwnd, SW_HIDE );
				}
			}
		}
	}
}

/*! ウインドウを強制的に前面に持ってくる */
void CTabWnd::ForceActiveWindow( HWND hwnd )
{
	int		nId1;
	int		nId2;
	DWORD	dwTime;

	nId2 = ::GetWindowThreadProcessId( ::GetForegroundWindow(), NULL );
	nId1 = ::GetWindowThreadProcessId( hwnd, NULL );

	::AttachThreadInput( nId1, nId2, TRUE );

	::SystemParametersInfo( SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &dwTime, 0 );
	::SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0, 0 );

	//ウィンドウをフォアグラウンドにする
	::SetForegroundWindow( hwnd );
	::BringWindowToTop( hwnd );

	::SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)(INT_PTR)dwTime, 0 );

	::AttachThreadInput( nId1, nId2, FALSE );
}

/*!	アクティブにする

	@param hwnd [in] 対象ウィンドウのウィンドウハンドル
	@param bForeground [in] true: active and forground / false: active

	@date 2004.08.27 Kazika 引数bForeground追加。bForegroundがfalseの場合はウィンドウをフォアグラウンドにしない。
	@date 2005.11.05 ryoji Grepダイアログがフォーカスを失わないようにするため，
		対象ウィンドウのプロセスが既にフォアグラウンドなら何もしないようにする．
	@date 2007.11.07 ryoji 対象がdisableのときは最近のポップアップをフォアグラウンド化する．
		（モーダルダイアログやメッセージボックスを表示しているようなとき）
*/
void CTabWnd::TabWnd_ActivateFrameWindow( HWND hwnd, bool bForeground )
{
	if ( bForeground )
	{
		// 2005.11.05 ryoji 対象ウィンドウのプロセスが既にフォアグラウンドなら切替え済みなので何もしないでおく
		DWORD dwPid1, dwPid2;
		::GetWindowThreadProcessId( hwnd, &dwPid1 );
		::GetWindowThreadProcessId( ::GetForegroundWindow(), &dwPid2 );
		if( dwPid1 == dwPid2 ){
			return;
		}

		// 対象がdisableのときは最近のポップアップをフォアグラウンド化する
		HWND hwndActivate;
		hwndActivate = ::IsWindowEnabled( hwnd )? hwnd: ::GetLastActivePopup( hwnd );

		if( ::IsIconic( hwnd ) )
		{
			::ShowWindow( hwnd, SW_RESTORE );	// Nov. 7. 2003 MIK アイコン時は元のサイズに戻す
		}
		else if( ::IsZoomed( hwnd ) )
		{
			::ShowWindow( hwnd, SW_MAXIMIZE );
		}
		else
		{
			::ShowWindow( hwnd, SW_SHOW );
		}

		::SetForegroundWindow( hwndActivate );
		::BringWindowToTop( hwndActivate );
	}
	else
	{
		// 2005.09.01 ryoji ::ShowWindow( hwnd, SW_SHOWNA ) だと非表示から表示に切り替わるときに Z-order がおかしくなることがあるので ::SetWindowPos に変更
		::SetWindowPos( hwnd, NULL,0,0,0,0,
						SWP_SHOWWINDOW | SWP_NOACTIVATE
						| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );
	}

	return;
}

/*! タブのレイアウト調整処理
	@date 2006.01.28 ryoji 新規作成
*/
void CTabWnd::LayoutTab( void )
{
	// フォントを切り替える 2011.12.01 Moca
	bool bChgFont = (0 != memcmp( &m_lf, &m_pShareData->m_Common.m_sTabBar.m_lf, sizeof(m_lf) ));
	int nSizeBoxWidth = 0;
	if( m_hwndSizeBox ){
		nSizeBoxWidth = ::GetSystemMetrics( SM_CXVSCROLL );
	}
	if( bChgFont ){
		HFONT hFontOld = m_hFont;
		m_lf = m_pShareData->m_Common.m_sTabBar.m_lf;
		m_hFont = ::CreateFontIndirect( &m_lf );
		::SendMessageAny( m_hwndTab, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0) );
		::DeleteObject( hFontOld );
		// ウィンドウの高さを修正
	}

	// アイコンの表示を切り替える
	BOOL bDispTabIcon = m_pShareData->m_Common.m_sTabBar.m_bDispTabIcon;
	HIMAGELIST hImg = TabCtrl_GetImageList( m_hwndTab );
	if( NULL == hImg && bDispTabIcon )
	{
		if( NULL != InitImageList() )
			Refresh( TRUE, TRUE );
	}
	else if( NULL != hImg && !bDispTabIcon )
	{
		InitImageList();
	}

	// 現在のウィンドウスタイルを取得する
	UINT lStyle = (UINT)::GetWindowLongPtr( m_hwndTab, GWL_STYLE );
	UINT lStyleOld = lStyle;

	// タブのアイテム幅の等幅を切り替える
	BOOL bSameTabWidth = m_pShareData->m_Common.m_sTabBar.m_bSameTabWidth;
	if( bSameTabWidth && !(lStyle & TCS_FIXEDWIDTH) ){
		lStyle |= (TCS_FIXEDWIDTH | TCS_FORCELABELLEFT);
	}else if( !bSameTabWidth && (lStyle & TCS_FIXEDWIDTH) ){
		lStyle &= ~(TCS_FIXEDWIDTH | TCS_FORCELABELLEFT);
	}

	// オーナードロー状態を共通設定に追随させる
	BOOL bDispTabClose = m_pShareData->m_Common.m_sTabBar.m_bDispTabClose;
	BOOL bOwnerDraw = bDispTabClose;
	if( bOwnerDraw && !(lStyle & TCS_OWNERDRAWFIXED) ){
		lStyle |= TCS_OWNERDRAWFIXED;
	}else if( !bOwnerDraw && (lStyle & TCS_OWNERDRAWFIXED) ){
		lStyle &= ~TCS_OWNERDRAWFIXED;
	}

	// タブのアイテムサイズを調整する（等幅のときのサイズやフォント切替時の高さ調整）
	// ※ 画面のちらつきや体感性能にさほど影響は無さそうなので条件を絞らず毎回 TabCtrl_SetItemSize() を実行する
	RECT rcTab;
	int nCount;
	int cx;
	::GetClientRect( m_hwndTab, &rcTab );
	nCount = TabCtrl_GetItemCount( m_hwndTab );
	if( 0 < nCount )
	{
		cx = (rcTab.right - rcTab.left - 8) / nCount;
		int min = MIN_TABITEM_WIDTH;
		if( m_pShareData->m_Common.m_sTabBar.m_bTabMultiLine ){
			min = MIN_TABITEM_WIDTH_MULTI;
		}
		if( MAX_TABITEM_WIDTH < cx )
			cx = MAX_TABITEM_WIDTH;
		else if( min > cx )
			cx = min;
		TabCtrl_SetItemSize( m_hwndTab, cx, TAB_ITEM_HEIGHT );
	}

	// タブ余白設定（「閉じるボタン」や「アイコン」の設定切替時の余白切替）
	// ※ 画面のちらつきや体感性能にさほど影響は無さそうなので条件を絞らず毎回 TabCtrl_SetPadding() を実行する
	cx = 6;
	if( bDispTabClose == DISPTABCLOSE_ALLWAYS ){
		// 閉じるボタンの分だけパディングを追加して横幅を広げる
		int nWidth = rcBtnBase.right - rcBtnBase.left;
		cx += bDispTabIcon? (nWidth + 2)/3: (nWidth + 1)/2;	// それっぽく調整: ボタン幅の 1/3（アイコン有） or 1/2（アイコン無）
	}
	TabCtrl_SetPadding( m_hwndTab, DpiScaleX(cx), DpiScaleY(3) );

	// 新しいウィンドウスタイルを適用する
	// ※ TabCtrl_SetPadding() の後でやらないと設定変更の直後にアイコンやテキストの描画位置がずれる場合がある
	// （例）Ver 2.1.0 では下記の設定変更をすると直後に描画位置がずれていた
	//           変更前：[等幅]OFF、[閉じるボタン]ON
	//           変更後：[等幅]ON、[閉じるボタン]OFF
	if( lStyle != lStyleOld ){
		::SetWindowLongPtr( m_hwndTab, GWL_STYLE, lStyle );
		if( bOwnerDraw ){
			// オーナードローに切り替えるときは再度ウィンドウスタイルを設定する。
			// ※ 設定が１度だけだと、非アクティブタブ上でマウスオーバーしてもタブがハイライト表示にならず、
			//    何かのタイミングで ::SetWindowLongPtr() が再実行されると以後はハイライト表示される。
			//    （Vista/7/8 で同様の症状を確認）
			::SetWindowLongPtr( m_hwndTab, GWL_STYLE, lStyle );
		}
	}
	RECT rcWnd;
	::GetWindowRect( GetHwnd(), &rcWnd );

	int nHeight = TAB_WINDOW_HEIGHT;
	::GetWindowRect( m_hwndTab, &rcTab );
	if( m_pShareData->m_Common.m_sTabBar.m_bTabMultiLine
		&& TabCtrl_GetItemCount( m_hwndTab ) ){
		// 正確に再配置（多段タブでは段数が変わることがあるので必須）
		RECT rcDisp = rcTab;
		rcDisp.left = TAB_MARGIN_LEFT;
		rcDisp.right = rcTab.left + (rcWnd.right - rcWnd.left) - (TAB_MARGIN_LEFT + TAB_MARGIN_RIGHT + nSizeBoxWidth);
		TabCtrl_AdjustRect( m_hwndTab, FALSE, &rcDisp );
		nHeight = (rcDisp.top - rcTab.top - 2) + TAB_MARGIN_TOP;
	}
	::SetWindowPos( GetHwnd(), NULL, 0, 0, rcWnd.right - rcWnd.left, nHeight, SWP_NOMOVE | SWP_NOZORDER );
	int nWidth = (rcWnd.right - rcWnd.left) - (TAB_MARGIN_LEFT + TAB_MARGIN_RIGHT + nSizeBoxWidth);
	if( (nWidth != rcTab.right - rcTab.left) || (nHeight != rcTab.bottom - rcTab.top) ){
		::MoveWindow( m_hwndTab, TAB_MARGIN_LEFT, TAB_MARGIN_TOP, nWidth, nHeight, TRUE );
	}
}

/*! イメージリストの初期化処理
	@date 2006.02.22 ryoji 新規作成
*/
HIMAGELIST CTabWnd::InitImageList( void )
{
	SHFILEINFO sfi;
	HIMAGELIST hImlSys;
	HIMAGELIST hImlNew;

	hImlNew = NULL;
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabIcon )
	{
		// システムイメージリストを取得する
		// 注：複製後に差し替えて利用するアイコンには事前にアクセスしておかないとイメージが入らない
		//     ここでは「フォルダを閉じたアイコン」、「フォルダを開いたアイコン」を差し替え用として利用
		//     WinNT4.0 では SHGetFileInfo() の第一引数に同名を指定すると同じインデックスを返してくることがある？
		// 2016.08.06 ".0" の場合に Win10で同じインデックスが返ってくるので、"C:\\"に変更

		hImlSys = (HIMAGELIST)::SHGetFileInfo(
			_T("C:\\"),
			FILE_ATTRIBUTE_DIRECTORY,
			&sfi,
			sizeof(sfi),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES
		);
		if( NULL == hImlSys )
			goto l_end;
		m_iIconApp = sfi.iIcon;

		hImlSys = (HIMAGELIST)::SHGetFileInfo(
			_T(".1"),
			FILE_ATTRIBUTE_DIRECTORY,
			&sfi,
			sizeof(sfi),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES | SHGFI_OPENICON
		);
		if( NULL == hImlSys )
			goto l_end;
		m_iIconGrep = sfi.iIcon;

		// システムイメージリストを複製する
		hImlNew = ImageList_Duplicate( hImlSys );
		if( NULL == hImlNew )
			goto l_end;
		ImageList_SetBkColor( hImlNew, CLR_NONE );

		// イメージリストにアプリケーションアイコンと Grepアイコンを登録する
		// （利用しないアイコンと差し替える）
		m_hIconApp = GetAppIcon( GetAppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true );
		ImageList_ReplaceIcon( hImlNew, m_iIconApp, m_hIconApp );
		if( m_iIconApp != m_iIconGrep ){
			m_hIconGrep = GetAppIcon( GetAppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, true );
			ImageList_ReplaceIcon( hImlNew, m_iIconGrep, m_hIconGrep );
		}
	}

l_end:
	// タブに新しいアイコンイメージを設定する
	TabCtrl_SetImageList( m_hwndTab, hImlNew );

	// 新しいイメージリストを記憶する
	if( NULL != m_hIml )
		ImageList_Destroy( m_hIml );
	m_hIml = hImlNew;

	return m_hIml;	// 新しいイメージリストを返す
}

/*! イメージリストのインデックス取得処理
	@date 2006.01.28 ryoji 新規作成
*/
int CTabWnd::GetImageIndex( EditNode* pNode )
{
	SHFILEINFO sfi;
	HIMAGELIST hImlSys;
	HIMAGELIST hImlNew;

	if( NULL == m_hIml )
		return -1;	// イメージリストを使っていない

	if( pNode )
	{
		if( pNode->m_szFilePath[0] )
		{
			// 拡張子を取り出す
			TCHAR szExt[_MAX_EXT];
			_tsplitpath( pNode->m_szFilePath, NULL, NULL, NULL, szExt );

			// 拡張子に関連付けられたアイコンイメージのインデックスを取得する
			hImlSys = (HIMAGELIST)::SHGetFileInfo( szExt, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES );
			if( NULL == hImlSys )
				return -1;
			if( ImageList_GetImageCount( m_hIml ) > sfi.iIcon )
				return sfi.iIcon;	// インデックスを返す

			// システムイメージリストを複製する
			hImlNew = ImageList_Duplicate( hImlSys );
			if( NULL == hImlNew )
				return -1;
			ImageList_SetBkColor( hImlNew, CLR_NONE );

			// イメージリストにアプリケーションアイコンと Grepアイコンを登録する
			// （利用しないアイコンと差し替える）
			ImageList_ReplaceIcon( hImlNew, m_iIconApp, m_hIconApp );
			if( m_iIconApp != m_iIconGrep ){
				ImageList_ReplaceIcon( hImlNew, m_iIconGrep, m_hIconGrep );
			}

			// タブにアイコンイメージを設定する
			if( m_pShareData->m_Common.m_sTabBar.m_bDispTabIcon )
				TabCtrl_SetImageList( m_hwndTab, hImlNew );

			// 新しいイメージリストを記憶する
			ImageList_Destroy( m_hIml );
			m_hIml = hImlNew;

			return sfi.iIcon;	// インデックスを返す
		}
		else if( pNode->m_bIsGrep )
			return m_iIconGrep;	// grepアイコンのインデックスを返す
	}

	return m_iIconApp;	// アプリケーションアイコンのインデックスを返す
}

/*! イメージリストの複製処理
	@date 2006.02.17 ryoji 新規作成
*/
HIMAGELIST CTabWnd::ImageList_Duplicate( HIMAGELIST himl )
{
	if( NULL == himl ) return NULL;

	// 本物の ImageList_Duplicate() があればそれを呼び出す
	HIMAGELIST hImlNew;
	if( m_RealImageList_Duplicate )
	{
		hImlNew = m_RealImageList_Duplicate( himl );
		if( NULL != hImlNew )
			return hImlNew;
		m_RealImageList_Duplicate = NULL;	// 2006.06.20 ryoji 失敗時は代替処理に切り替え
	}

	// 本物の ImageList_Duplicate() の代替処理
	// 新しいイメージリストを作成してアイコン単位でコピーする
	//（この場合、多色アイコンは綺麗には表示されないかもしれない）
	int cxIcon = CX_SMICON;
	int cyIcon = CY_SMICON;
	ImageList_GetIconSize( himl, &cxIcon, &cyIcon );
	hImlNew = ImageList_Create( cxIcon, cyIcon, ILC_COLOR32 | ILC_MASK, 4, 4 );
	if( hImlNew )
	{
		ImageList_SetBkColor( hImlNew, CLR_NONE );
		int nCount = ImageList_GetImageCount( himl );
		int i;
		for( i = 0; i < nCount; i++ )
		{
			HICON hIcon = ImageList_GetIcon( himl, i, ILD_TRANSPARENT );
			if( NULL == hIcon )
			{
				ImageList_Destroy( hImlNew );
				return NULL;
			}
			int iIcon = ImageList_AddIcon( hImlNew, hIcon );
			::DestroyIcon( hIcon );
			if( 0 > iIcon )
			{
				ImageList_Destroy( hImlNew );
				return NULL;
			}
		}
	}
	return hImlNew;
}

/*! ボタン背景描画処理
	@date 2006.10.21 ryoji 新規作成
*/
void CTabWnd::DrawBtnBkgnd( HDC hdc, const LPRECT lprcBtn, BOOL bBtnHilighted )
{
	if( bBtnHilighted )
	{
		CGraphics gr(hdc);
		gr.SetPen( ::GetSysColor(COLOR_HIGHLIGHT) );
		gr.SetBrushColor( ::GetSysColor(COLOR_MENU) );
		::Rectangle( gr, lprcBtn->left, lprcBtn->top, lprcBtn->right, lprcBtn->bottom );
	}
}

/*! 一覧ボタン描画処理
	@date 2006.02.01 ryoji 新規作成
	@date 2006.10.21 ryoji 背景描画を関数呼び出しに変更
	@date 2009.10.01 ryoji 描画イメージを矩形中央にもってくる
*/
void CTabWnd::DrawListBtn( CGraphics& gr, const LPRECT lprcClient )
{
	static const POINT ptBase[4] = { {4, 8}, {7, 11}, {8, 11}, {11, 8} };	// 描画イメージ形状
	POINT pt[4];

	RECT rcBtn;
	GetListBtnRect( lprcClient, &rcBtn );
	DrawBtnBkgnd( gr, &rcBtn, m_bListBtnHilighted );	// 2006.10.21 ryoji

	// 描画イメージを矩形中央にもってくる	// 2009.10.01 ryoji
	rcBtn.left = rcBtn.left + ((rcBtn.right - rcBtn.left) - (rcBtnBase.right - rcBtnBase.left)) / 2;
	rcBtn.top = rcBtn.top + ((rcBtn.bottom - rcBtn.top) - (rcBtnBase.bottom - rcBtnBase.top)) / 2;
	rcBtn.right = rcBtn.left + (rcBtnBase.right - rcBtnBase.left);
	rcBtn.bottom = rcBtn.top + (rcBtnBase.bottom - rcBtnBase.left);

	int nIndex = m_bListBtnHilighted? COLOR_MENUTEXT: COLOR_BTNTEXT;
	gr.SetPen( ::GetSysColor( nIndex ) );
	gr.SetBrushColor( ::GetSysColor( nIndex ) ); //$$ GetSysColorBrushを用いた実装のほうが効率は良い
	for( int i = 0; i < _countof(ptBase); i++ )
	{
		pt[i].x = ptBase[i].x + rcBtn.left;
		pt[i].y = ptBase[i].y + rcBtn.top;
	}
	::Polygon( gr, pt, _countof(pt) );
}

/*! 閉じるマーク描画処理 */
void CTabWnd::DrawCloseFigure( CGraphics& gr, const RECT& rcBtn )
{
	static const POINT ptBase1[6][2] =	// [x]描画イメージ形状（直線6本）
	{
		{{4, 5}, {12, 13}},
		{{4, 4}, {13, 13}},
		{{5, 4}, {13, 12}},
		{{11, 4}, {3, 12}},
		{{12, 4}, {3, 13}},
		{{12, 5}, {4, 13}}
	};
	POINT pt[2];
	int i;

	// [x]を描画（直線6本）
	for( i = 0; i < _countof(ptBase1); i++ )
	{
		pt[0].x = ptBase1[i][0].x + rcBtn.left;
		pt[0].y = ptBase1[i][0].y + rcBtn.top;
		pt[1].x = ptBase1[i][1].x + rcBtn.left;
		pt[1].y = ptBase1[i][1].y + rcBtn.top;
		::MoveToEx( gr, pt[0].x, pt[0].y, NULL );
		::LineTo( gr, pt[1].x, pt[1].y );
	}
}

/*! 閉じるボタン描画処理
	@date 2006.10.21 ryoji 新規作成
	@date 2009.10.01 ryoji 描画イメージを矩形中央にもってくる
	@date 2012.05.12 syat マーク描画部分を関数に切り出し
*/
void CTabWnd::DrawCloseBtn( CGraphics& gr, const LPRECT lprcClient )
{
	static const POINT ptBase2[10][2] = // [xx]描画イメージ形状（矩形10個）
	{
		{{3, 4}, {5, 6}},
		{{6, 4}, {8, 6}},
		{{4, 6}, {7, 10}},
		{{3, 10}, {5, 12}},
		{{6, 10}, {8, 12}},
		{{9, 4}, {11, 6}},
		{{12, 4}, {14, 6}},
		{{10, 6}, {13, 10}},
		{{9, 10}, {11, 12}},
		{{12, 10}, {14, 12}}
	};

	POINT pt[2];
	int i;

	RECT rcBtn;
	GetCloseBtnRect( lprcClient, &rcBtn );

	// ボタンの左側にセパレータを描画する	// 2007.02.27 ryoji
	gr.SetPen( ::GetSysColor( COLOR_3DSHADOW ) );
	::MoveToEx( gr, rcBtn.left - DpiScaleX(4), rcBtn.top + 1, NULL );
	::LineTo( gr, rcBtn.left - DpiScaleX(4), rcBtn.bottom - 1 );

	DrawBtnBkgnd( gr, &rcBtn, m_bCloseBtnHilighted );

	// 描画イメージを矩形中央にもってくる	// 2009.10.01 ryoji
	rcBtn.left = rcBtn.left + ((rcBtn.right - rcBtn.left) - (rcBtnBase.right - rcBtnBase.left)) / 2;
	rcBtn.top = rcBtn.top + ((rcBtn.bottom - rcBtn.top) - (rcBtnBase.bottom - rcBtnBase.top)) / 2;
	rcBtn.right = rcBtn.left + (rcBtnBase.right - rcBtnBase.left);
	rcBtn.bottom = rcBtn.top + (rcBtnBase.bottom - rcBtnBase.left);

	int nIndex = m_bCloseBtnHilighted? COLOR_MENUTEXT: COLOR_BTNTEXT;
	gr.SetPen( ::GetSysColor(nIndex) );
	gr.SetBrushColor( ::GetSysColor(nIndex) );
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd &&
		!m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin &&
		!m_pShareData->m_Common.m_sTabBar.m_bTab_CloseOneWin			// 2007.02.13 ryoji 条件追加（ウィンドウの閉じるボタンは全部閉じる）
		)
	{
		DrawCloseFigure( gr, rcBtn );
	}
	else
	{
		 // [xx]を描画（矩形10個）
		for( i = 0; i < _countof(ptBase2); i++ )
		{
			pt[0].x = ptBase2[i][0].x + rcBtn.left;
			pt[0].y = ptBase2[i][0].y + rcBtn.top;
			pt[1].x = ptBase2[i][1].x + rcBtn.left;
			pt[1].y = ptBase2[i][1].y + rcBtn.top;
			::Rectangle( gr, pt[0].x, pt[0].y, pt[1].x, pt[1].y );
		}
	}
}

/*! タブを閉じるボタン描画処理
	@date 2012.04.08 syat 新規作成
*/
void CTabWnd::DrawTabCloseBtn( CGraphics& gr, const LPRECT lprcClient, bool selected, bool bHover )
{
	RECT rcBtn;
	GetTabCloseBtnRect( lprcClient, &rcBtn, selected );

	DrawBtnBkgnd( gr, &rcBtn, bHover );

	// 描画イメージを矩形中央にもってくる	// 2009.10.01 ryoji
	rcBtn.left = rcBtn.left + ((rcBtn.right - rcBtn.left) - (rcBtnBase.right - rcBtnBase.left)) / 2;
	rcBtn.top = rcBtn.top + ((rcBtn.bottom - rcBtn.top) - (rcBtnBase.bottom - rcBtnBase.top)) / 2 - 1;
	rcBtn.right = rcBtn.left + (rcBtnBase.right - rcBtnBase.left);
	rcBtn.bottom = rcBtn.top + (rcBtnBase.bottom - rcBtnBase.left);

	int nIndex = COLOR_BTNTEXT;
	gr.SetPen( ::GetSysColor(nIndex) );
	gr.SetBrushColor( ::GetSysColor(nIndex) );
	DrawCloseFigure( gr, rcBtn );
}

/*! 一覧ボタンの矩形取得処理
	@date 2006.02.01 ryoji 新規作成
*/
void CTabWnd::GetListBtnRect( const LPRECT lprcClient, LPRECT lprc )
{
	*lprc = rcBtnBase;
	DpiScaleRect(lprc);	// 2009.10.01 ryoji 高DPI対応スケーリング
	int nSizeBoxWidth = 0;
	if( m_hwndSizeBox ){
		nSizeBoxWidth = ::GetSystemMetrics( SM_CXVSCROLL );
	}
	::OffsetRect(lprc, lprcClient->right - TAB_MARGIN_RIGHT - nSizeBoxWidth + DpiScaleX(4), lprcClient->top + TAB_MARGIN_TOP + DpiScaleY(2) );
}

/*! 閉じるボタンの矩形取得処理
	@date 2006.10.21 ryoji 新規作成
*/
void CTabWnd::GetCloseBtnRect( const LPRECT lprcClient, LPRECT lprc )
{
	*lprc = rcBtnBase;
	DpiScaleRect(lprc);	// 2009.10.01 ryoji 高DPI対応スケーリング
	int nSizeBoxWidth = 0;
	if( m_hwndSizeBox ){
		nSizeBoxWidth = ::GetSystemMetrics( SM_CXVSCROLL );
	}
	::OffsetRect(lprc,
		lprcClient->right - TAB_MARGIN_RIGHT - nSizeBoxWidth + DpiScaleX(4) + (DpiScaleX(rcBtnBase.right) - DpiScaleX(rcBtnBase.left)) + DpiScaleX(7),
		lprcClient->top + TAB_MARGIN_TOP + DpiScaleY(2) );
}

/*! タブを閉じるボタンの矩形取得処理
	@date 2012.04.08 syat 新規作成
*/
void CTabWnd::GetTabCloseBtnRect( const LPRECT lprcTab, LPRECT lprc, bool selected )
{
	*lprc = rcBtnBase;
	DpiScaleRect(lprc);	// 2009.10.01 ryoji 高DPI対応スケーリング
	::OffsetRect(lprc,
		(lprcTab->right + (selected ? 0: -2)) - ((DpiScaleX(rcBtnBase.right) - DpiScaleX(rcBtnBase.left)) + DpiScaleX(2)),
		(lprcTab->top + (selected ? -2: 0)) + DpiScaleY(2) );
}


/** タブ名取得処理

	@param[in] EditNode 編集ウィンドウ情報
	@param[in] bFull パス名で表示する
	@param[in] bDupamp &を&&に置き換える
	@param[out] pszName タブ名格納先
	@param[in] nLen 格納先最大文字数（終端のnull文字含む）

	@date 2007.06.28 ryoji 新規作成
*/
void CTabWnd::GetTabName( EditNode* pEditNode, BOOL bFull, BOOL bDupamp, LPTSTR pszName, int nLen )
{
	LPTSTR pszText = new TCHAR[nLen];

	if( pEditNode == NULL )
	{
		::lstrcpyn( pszText, LS(STR_NO_TITLE1), nLen );
	}
	else if( !bFull || pEditNode->m_szFilePath[0] == '\0' )
	{
		if( pEditNode->m_szTabCaption[0] )
		{
			::lstrcpyn( pszText, pEditNode->m_szTabCaption, nLen );
		}
		else
		{
			::lstrcpyn( pszText, LS(STR_NO_TITLE1), nLen );
		}
	}
	else
	{
		// フルパス名を簡易名に変換する
		HDC hdc = ::GetDC(m_hwndTab);
		HFONT hFontOld = (HFONT)SelectObject(hdc, m_hFont);
		CFileNameManager::getInstance()->GetTransformFileNameFast( pEditNode->m_szFilePath, pszText, nLen, hdc, false );
		SelectObject(hdc, hFontOld);
		::ReleaseDC(m_hwndTab, hdc);
	}

	if( bDupamp )
	{
		// &を&&に置き換える
		LPTSTR pszText_amp = new TCHAR[nLen * 2];
		dupamp( pszText, pszText_amp );
		::lstrcpyn( pszName, pszText_amp, nLen );
		delete []pszText_amp;
	}
	else
	{
		::lstrcpyn( pszName, pszText, nLen );
	}

	delete []pszText;
}



/**	タブ一覧表示処理

	@param pt [in] 表示位置
	@param bSel [in] 表示切替メニューを追加する
	@param bFull [in] パス名で表示する（bSelがTRUEの場合は無効）
	@param bOtherGroup [in] 他グループのウィンドウも表示する

	@date 2006.02.01 ryoji 新規作成
	@date 2006.03.23 fon OnListBtnClickから移動(行頭の//>が変更部)
	@date 2006.10.31 ryoji メニューのフルパス名を簡易表示する
	@date 2007.02.28 ryoji タブ名一覧／パス名一覧の表示をメニュー自身で切り替える
	@date 2007.06.28 ryoji グループ化対応（他グループのウィンドウを表示する／しない）
*/
LRESULT CTabWnd::TabListMenu( POINT pt, BOOL bSel/* = TRUE*/, BOOL bFull/* = FALSE*/, BOOL bOtherGroup/* = TRUE*/ )
{
	bool bRepeat;

	if( bSel )
		bFull = m_pShareData->m_Common.m_sTabBar.m_bTab_ListFull;

	do
	{
		EditNode* pEditNode;
		int i;
		int nGroup = 0;
		int nSelfTab;
		int nTab;
		int nCount;

		// タブメニュー用の情報を取得する
		nCount = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
		if( 0 >= nCount )
			return 0L;

		// 自ウィンドウのグループ番号を調べる
		for( i = 0; i < nCount; i++ )
		{
			if( pEditNode[i].m_hWnd == GetParentHwnd() )
			{
				nGroup = pEditNode[i].m_nGroup;
				break;
			}
		}
		if( i >= nCount ) {
			return 0L;
		}

		TABMENU_DATA* pData = new TABMENU_DATA[nCount];	// タブメニュー用の情報

		// 自グループのウィンドウ一覧情報を作成する
		nSelfTab = 0;
		if( i < nCount )
		{
			for( i = 0; i < nCount; i++ )
			{
				if( pEditNode[i].m_nGroup != nGroup )
					continue;
				if( pEditNode[i].m_bClosing )	// このあとすぐに閉じるウィンドウなのでタブ表示しない
					continue;
				GetTabName( &pEditNode[i], bFull, TRUE, pData[nSelfTab].szText, _countof(pData[0].szText) );
				pData[nSelfTab].hwnd = pEditNode[i].m_hWnd;
				pData[nSelfTab].iItem = i;
				pData[nSelfTab].iImage = GetImageIndex( &pEditNode[i] );
				nSelfTab++;
			}
			// 表示文字でソートする
			if( nSelfTab > 0 && m_pShareData->m_Common.m_sTabBar.m_bSortTabList )	// 2006.03.23 fon 変更
				qsort( pData, nSelfTab, sizeof(pData[0]), compTABMENU_DATA );
		}

		// 他グループのウィンドウ一覧情報を作成する
		nTab = nSelfTab;
		for( i = 0; i < nCount; i++ )
		{
			if( pEditNode[i].m_nGroup == nGroup )
				continue;
			if( pEditNode[i].m_bClosing )	// このあとすぐに閉じるウィンドウなのでタブ表示しない
				continue;
			GetTabName( &pEditNode[i], bFull, TRUE, pData[nTab].szText, _countof(pData[0].szText) );
			pData[nTab].hwnd = pEditNode[i].m_hWnd;
			pData[nTab].iItem = i;
			pData[nTab].iImage = GetImageIndex( &pEditNode[i] );
			nTab++;
		}
		// 表示文字でソートする
		if( nTab > nSelfTab && m_pShareData->m_Common.m_sTabBar.m_bSortTabList )
			qsort( pData + nSelfTab, nTab - nSelfTab, sizeof(pData[0]), compTABMENU_DATA );

		delete []pEditNode;

		// メニューを作成する
		// 2007.02.28 ryoji 表示切替をメニューに追加
		int iMenuSel = -1;
		UINT uFlags = MF_BYPOSITION | (m_hIml? MF_OWNERDRAW: MF_STRING);
		HMENU hMenu = ::CreatePopupMenu();
		for( i = 0; i < nSelfTab; i++ )
		{
			::InsertMenu( hMenu, i, uFlags, IDM_SELWINDOW + i, m_hIml? (LPCTSTR)&pData[i]: pData[i].szText );
			if( pData[i].hwnd == GetParentHwnd() )
				iMenuSel = i;
		}

		// 自ウィンドウに対応するメニューをチェック状態にする
		if( iMenuSel >= 0 )
		{
			::CheckMenuRadioItem( hMenu, 0, nSelfTab - 1, iMenuSel, MF_BYPOSITION );
		}

		// 他グループのウィンドウ一覧を追加する
		if( nTab > nSelfTab )
		{
			if( bOtherGroup )
			{
				for( i = nSelfTab; i < nTab; i++ )
				{
					::InsertMenu( hMenu, i, uFlags, IDM_SELWINDOW + i, m_hIml? (LPCTSTR)&pData[i]: pData[i].szText );
				}
			}
			else
			{
				::InsertMenu( hMenu, nSelfTab, MF_BYPOSITION, 101, LS(STR_TABWND_SHOWALL) );
			}
			::InsertMenu( hMenu, nSelfTab, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// セパレータ
		}

		// 表示切替メニューを追加する
		if( bSel )
		{
			::InsertMenu( hMenu, 0, MF_BYPOSITION | MF_STRING, 100, bFull? LS(STR_TABWND_SHOWTABNAME): LS(STR_TABWND_SHOWPATHNAME) );
			::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// セパレータ
		}

		// メニューを表示する
		// 2006.04.21 ryoji マルチモニタ対応の修正
		RECT rcWork;
		GetMonitorWorkRect( pt, &rcWork );	// モニタのワークエリア
		int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									( pt.x > rcWork.left )? pt.x: rcWork.left,
									( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
									0, GetHwnd(), NULL);
		::DestroyMenu( hMenu );

		// メニュー選択されたタブのウインドウをアクティブにする
		bRepeat = false;
		if( 100 == nId )	// 表示切替
		{
			bFull = !bFull;
			bRepeat = true;
		}
		else if( 101 == nId )
		{
			bOtherGroup = !bOtherGroup;
			bRepeat = true;
		}
		else if( IDM_SELWINDOW <= nId && nId < IDM_SELWINDOW + nTab )
		{
			ActivateFrameWindow( pData[nId - IDM_SELWINDOW].hwnd );
		}

		delete []pData;

	} while( bRepeat );

	if( bSel )
		m_pShareData->m_Common.m_sTabBar.m_bTab_ListFull = bFull;

	return 0L;
}


/** 次のグループの先頭ウィンドウを探す
	@date 2007.06.20 ryoji 新規作成
*/
HWND CTabWnd::GetNextGroupWnd( void )
{
	HWND hwndRet = NULL;

	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		EditNode* pWndArr;
		int i;
		int j;
		int n;

		n = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pWndArr, FALSE, TRUE );	// グループ番号順ソート
		if( 0 == n )
			return NULL;
		for( i = 0; i < n; i++ )
		{
			if( pWndArr[i].m_hWnd == GetParentHwnd() )
				break;
		}
		if( i < n )
		{
			for( j = i + 1; j < n; j++ )
			{
				if( pWndArr[j].m_nGroup != pWndArr[i].m_nGroup )
				{
					hwndRet = CAppNodeManager::getInstance()->GetEditNode(pWndArr[j].m_hWnd)->GetGroup().GetTopEditNode()->GetHwnd();
					break;
				}
			}
			if( j >= n )
			{
				for( j = 0; j < i; j++ )
				{
					if( pWndArr[j].m_nGroup != pWndArr[i].m_nGroup )
					{
						hwndRet = CAppNodeManager::getInstance()->GetEditNode(pWndArr[j].m_hWnd)->GetGroup().GetTopEditNode()->GetHwnd();
						break;
					}
				}
			}
		}
		delete []pWndArr;
	}

	return hwndRet;
}

/** 前のグループの先頭ウィンドウを探す
	@date 2007.06.20 ryoji 新規作成
*/
HWND CTabWnd::GetPrevGroupWnd( void )
{
	HWND hwndRet = NULL;
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		EditNode* pWndArr;
		int i;
		int j;
		int n;

		n = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pWndArr, FALSE, TRUE );	// グループ番号順ソート
		if( 0 == n )
			return NULL;
		for( i = 0; i < n; i++ )
		{
			if( pWndArr[i].m_hWnd == GetParentHwnd() )
				break;
		}
		if( i < n )
		{
			for( j = i - 1; j >= 0; j-- )
			{
				if( pWndArr[j].m_nGroup != pWndArr[i].m_nGroup )
				{
					hwndRet = CAppNodeManager::getInstance()->GetEditNode(pWndArr[j].m_hWnd)->GetGroup().GetTopEditNode()->GetHwnd();
					break;
				}
			}
			if( j < 0 )
			{
				for( j = n - 1; j > i; j-- )
				{
					if( pWndArr[j].m_nGroup != pWndArr[i].m_nGroup )
					{
						hwndRet = CAppNodeManager::getInstance()->GetEditNode(pWndArr[j].m_hWnd)->GetGroup().GetTopEditNode()->GetHwnd();
						break;
					}
				}
			}
		}
		delete []pWndArr;
	}

	return hwndRet;
}

/** 次のグループをアクティブにする
	@date 2007.06.20 ryoji 新規作成
*/
void CTabWnd::NextGroup( void )
{
	HWND hWnd = GetNextGroupWnd();
	if( hWnd )
	{
		ActivateFrameWindow( hWnd );
	}
}

/** 前のグループをアクティブにする
	@date 2007.06.20 ryoji 新規作成
*/
void CTabWnd::PrevGroup( void )
{
	HWND hWnd = GetPrevGroupWnd();
	if( hWnd )
	{
		ActivateFrameWindow( hWnd );
	}
}

/** タブを右に移動する
	@date 2007.06.20 ryoji 新規作成
*/
void CTabWnd::MoveRight( void )
{
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd )
	{
		int nIndex = FindTabIndexByHWND( GetParentHwnd() );
		if( -1 != nIndex )
		{
			int nCount = TabCtrl_GetItemCount( m_hwndTab );
			if( nCount - 1 > nIndex )
			{
				if( ReorderTab( nIndex, nIndex + 1 ) ){
					BroadcastRefreshToGroup();
				}
			}
		}
	}
}

/** タブを左に移動する
	@date 2007.06.20 ryoji 新規作成
*/
void CTabWnd::MoveLeft( void )
{
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd )
	{
		int nIndex = FindTabIndexByHWND( GetParentHwnd() );
		if( -1 != nIndex )
		{
			if( 0 < nIndex )
			{
				if( ReorderTab( nIndex, nIndex - 1 ) ){
					BroadcastRefreshToGroup();
				}
			}
		}
	}
}

/** 新規グループを作成する（現在のグループから分離）
	@date 2007.06.20 ryoji 新規作成
	@date 2007.11.30 ryoji 最大化時の分離対応
*/
void CTabWnd::Separate( void )
{
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		RECT rc;
		POINT ptSrc;
		POINT ptDst;
		RECT rcWork;
		int cy;

		::GetWindowRect( GetParentHwnd(), &rc );
		if( ::IsZoomed( GetParentHwnd() ) )
		{
			ptSrc.x = ptDst.x = ( rc.left + rc.right ) / 2;
			ptSrc.y = ptDst.y = ( rc.top + rc.bottom ) / 2;
		}
		else
		{
			ptSrc.x = rc.left;
			ptSrc.y = rc.top;
			cy = ::GetSystemMetrics( SM_CYCAPTION );
			rc.left += cy;
			rc.right += cy;
			rc.top += cy;
			rc.bottom += cy;
			GetMonitorWorkRect( GetParentHwnd(), &rcWork );
			if( rc.bottom > rcWork.bottom ){
				rc.top -= (rc.bottom - rcWork.bottom);
				rc.bottom = rcWork.bottom;
			}
			if( rc.right > rcWork.right ){
				rc.left -= (rc.right - rcWork.right);
				rc.right = rcWork.right;
			}
			if( rc.top < rcWork.top ){
				rc.bottom += (rcWork.top - rc.top);
				rc.top = rcWork.top;
			}
			if( rc.left < rcWork.left ){
				rc.right += (rcWork.left - rc.left);
				rc.left = rcWork.left;
			}
			ptDst.x = rc.left;
			ptDst.y = rc.top;
		}

		SeparateGroup( GetParentHwnd(), NULL, ptSrc, ptDst );
	}
}

/** 次のグループに移動する（現在のグループから分離、結合）
	@date 2007.06.20 ryoji 新規作成
*/
void CTabWnd::JoinNext( void )
{
	HWND hWnd = GetNextGroupWnd();
	if( hWnd )
	{
		POINT ptSrc;
		POINT ptDst;
		ptSrc.x = ptSrc.y = ptDst.x = ptDst.y = 0;
		SeparateGroup( GetParentHwnd(), hWnd, ptSrc, ptDst );
	}
}

/** 前のグループに移動する（現在のグループから分離、結合）
	@date 2007.06.20 ryoji 新規作成
*/
void CTabWnd::JoinPrev( void )
{
	HWND hWnd = GetPrevGroupWnd();
	if( hWnd )
	{
		POINT ptSrc;
		POINT ptDst;
		ptSrc.x = ptSrc.y = ptDst.x = ptDst.y = 0;
		SeparateGroup( GetParentHwnd(), hWnd, ptSrc, ptDst );
	}
}


/*! サイズボックスの表示／非表示切り替え */
void CTabWnd::SizeBox_ONOFF( bool bSizeBox )
{
	RECT		rc;
	::GetWindowRect( GetHwnd(), &rc );
	if( m_bSizeBox == bSizeBox ){
		return;
	}
	if( m_bSizeBox ){
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
		m_bSizeBox = false;
		OnSize();
	}else{
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 						/* no extended styles			*/
			_T("SCROLLBAR"),				/* scroll bar control class		*/
			NULL,						/* text for window title bar	*/
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,							/* horizontal position			*/
			0,							/* vertical position			*/
			200,						/* width of the scroll bar		*/
			CW_USEDEFAULT,				/* default height				*/
			GetHwnd(), 				/* handle of main window		*/
			(HMENU) NULL,				/* no menu for a scroll bar 	*/
			GetAppInstance(),				/* instance owning this window	*/
			(LPVOID) NULL			/* pointer not needed				*/
		);
		::ShowWindow( m_hwndSizeBox, SW_SHOW );
		m_bSizeBox = true;
		OnSize();
	}
	return;
}
