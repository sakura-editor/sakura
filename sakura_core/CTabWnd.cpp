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

#include "stdafx.h"
#include <windows.h>
#include "CTabWnd.h"
#include "debug.h"
#include "CEditWnd.h"
#include "global.h"
#include "mymessage.h"
#include "etc_uty.h"
#include "charcode.h"
#include "my_tchar.h"	//Unicode対応 Moca

//#if(WINVER >= 0x0500)
#ifndef	SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT        0x2000
#endif
#ifndef	SPI_SETFOREGROUNDLOCKTIMEOUT
#define SPI_SETFOREGROUNDLOCKTIMEOUT        0x2001
#endif
//#endif

// 2007.04.01 ryoji WM_THEMECHANGED
#ifndef	WM_THEMECHANGED
#define WM_THEMECHANGED		0x031A
#endif

// 2006.01.30 ryoji タブのサイズ／位置に関する定義
#define TAB_WINDOW_HEIGHT	24
#define TAB_MARGIN_TOP		3
#define TAB_MARGIN_LEFT		1
#define TAB_MARGIN_RIGHT	47
#define TAB_ITEM_HEIGHT		(TAB_WINDOW_HEIGHT - 5)
#define MAX_TABITEM_WIDTH	200
#define MIN_TABITEM_WIDTH	60
#define CX_SMICON			16
#define CY_SMICON			16
static const RECT rcBtnBase = { 0, 0, 16, 16 };

// 2006.02.01 ryoji タブ一覧メニュー用データ
typedef struct {
	HWND hwnd;
	int iItem;
	int iImage;
	TCHAR szText[_MAX_PATH];
} TABMENU_DATA;

/*!	タブ一覧メニュー用データの qsort() コールバック処理
	@date 2006.02.01 ryoji 新規作成
*/
static int compTABMENU_DATA( const void *arg1, const void *arg2 )
{
	int ret;

	ret = ::lstrcmp( ((TABMENU_DATA*)arg1)->szText, ((TABMENU_DATA*)arg2)->szText );
	if( 0 == ret )
		ret = ((TABMENU_DATA*)arg1)->iItem - ((TABMENU_DATA*)arg2)->iItem;
	return ret;
}


WNDPROC	gm_pOldWndProc = NULL;

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

	if( gm_pOldWndProc )
		return ::CallWindowProc( (WNDPROC)gm_pOldWndProc, hwnd, uMsg, wParam, lParam );
	else
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
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
		::InvalidateRect( m_hWnd, NULL, TRUE );	// アクティブタブの位置が変わるのでトップバンドを更新する	// 2006.03.27 ryoji
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
	int nSelfTab = FindTabIndexByHWND( m_hwndParent );

	switch( m_eDragState )
	{
	case DRAG_CHECK:
		if ( m_nSrcTab == nDstTab && m_nSrcTab != nSelfTab )
		{
			//指定のウインドウをアクティブに
			TCITEM	tcitem;
			tcitem.mask   = TCIF_PARAM;
			tcitem.lParam = (LPARAM)0;
			TabCtrl_GetItem( m_hwndTab, nDstTab, &tcitem );

			ShowHideWindow( (HWND)tcitem.lParam, TRUE );
		}
		break;

	case DRAG_DRAG:
		if ( 0 <= nDstTab )	// タブの上でドロップ
		{
			// タブの順序変更処理
			ReorderTab( m_nSrcTab, nDstTab );
		}
		else
		{
			// タブの分離処理
			if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin ){
				if( !::IsZoomed( m_hwndParent ) )
				{
					HWND hwndAncestor;
					POINT ptCursor;

					::GetCursorPos( &ptCursor );
					hwndAncestor = MyGetAncestor( ::WindowFromPoint( ptCursor ), GA_ROOT );
					if( hwndAncestor != m_hwndParent )	// 自画面の外でドロップ
					{
						// タブ移動
						TCITEM	tcitem;
						tcitem.mask   = TCIF_PARAM;
						tcitem.lParam = (LPARAM)0;
						TabCtrl_GetItem( m_hwndTab, m_nSrcTab, &tcitem );
						HWND hwndSrc = (HWND)tcitem.lParam;
						HWND hwndDst = CShareData::getInstance()->IsEditWnd( hwndAncestor )? hwndAncestor: NULL;

						SeparateGroup( hwndSrc, hwndDst, m_ptSrcCursor, ptCursor );
					}
				}
			}
		}
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
	hitinfo.pt.x = LOWORD( (DWORD)lParam );
	hitinfo.pt.y = HIWORD( (DWORD)lParam );
	int nDstTab = TabCtrl_HitTest( m_hwndTab, (LPARAM)&hitinfo );

	switch( m_eDragState )
	{
	case DRAG_CHECK:
		// 元のタブから離れたらドラッグ開始
		if( m_nSrcTab == nDstTab )
			break;
		m_eDragState = DRAG_DRAG;
		// ここに来たらドラッグ開始なので break しないでそのまま DRAG_DRAG 処理に入る

	case DRAG_DRAG:
		// ドラッグ中のマウスカーソルを表示する
		HINSTANCE hInstance;
		LPCTSTR lpCursorName;
		lpCursorName = IDC_NO;	// 禁止カーソル
		if ( 0 <= nDstTab )	// タブの上にカーソルがある
		{
			if( m_nSrcTab > nDstTab )
				lpCursorName = MAKEINTRESOURCE(IDC_CURSOR_TAB_LEFT);	// 左へ移動カーソル
			else if( m_nSrcTab < nDstTab )
				lpCursorName = MAKEINTRESOURCE(IDC_CURSOR_TAB_RIGHT);	// 右へ移動カーソル
		}
		else
		{
			if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
			{
				if( !::IsZoomed( m_hwndParent ) )
				{
					HWND hwndAncestor;
					POINT ptCursor;

					::GetCursorPos( &ptCursor );
					hwndAncestor = MyGetAncestor( ::WindowFromPoint( ptCursor ), GA_ROOT );
					if( hwndAncestor != m_hwndParent )	// 自画面の外にカーソルがある
					{
						if( CShareData::getInstance()->IsEditWnd( hwndAncestor ) )
							lpCursorName = MAKEINTRESOURCE(IDC_CURSOR_TAB_JOIN);	// 結合カーソル
						else
							lpCursorName = MAKEINTRESOURCE(IDC_CURSOR_TAB_SEPARATE);	// 分離カーソル
					}
				}
			}
		}
		hInstance = (lpCursorName == IDC_NO)? NULL: ::GetModuleHandle( NULL );
		::SetCursor( ::LoadCursor( hInstance, lpCursorName ) );
		break;

	default:
		return 1L;
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
*/
LRESULT CTabWnd::OnTabNotify( WPARAM wParam, LPARAM lParam )
{
	LPNMTTDISPINFO	lpnmtdi;
	lpnmtdi = (LPNMTTDISPINFO)lParam;
	if( lpnmtdi->hdr.hwndFrom == m_hwndToolTip )
	{
		switch( lpnmtdi->hdr.code )
		{
		//case TTN_NEEDTEXT:
		case TTN_GETDISPINFOW:
		case TTN_GETDISPINFOA:
			{
				TCITEM	tcitem;

				tcitem.mask   = TCIF_PARAM;
				tcitem.lParam = (LPARAM)NULL;
				if( TabCtrl_GetItem( m_hwndTab, lpnmtdi->hdr.idFrom, &tcitem ) )
				{
					EditNode* pEditNode;
					pEditNode = CShareData::getInstance()->GetEditNode( (HWND)tcitem.lParam );
					GetTabName( pEditNode, TRUE, FALSE, m_szTextTip1, sizeof(m_szTextTip1)/sizeof(TCHAR) );

					if( TTN_GETDISPINFOW == lpnmtdi->hdr.code )
					{
#ifdef UNICODE
						lpnmtdi->lpszText = m_szTextTip1;
						lpnmtdi->hinst    = NULL;
#else
						//UNICODEの文字列が欲しい。
						int	Size = _tcslen( m_szTextTip1 );
						m_szTextTip2[ MultiByteToWideChar( CP_ACP, 0, m_szTextTip1, Size, m_szTextTip2, Size ) ] = 0;
						LPNMTTDISPINFOW	lpnmtdiw = (LPNMTTDISPINFOW)lParam;
						lpnmtdiw->lpszText = m_szTextTip2;
						lpnmtdiw->hinst    = NULL;
#endif	//UNICODE
					}
					else
					{
#ifdef UNICODE
						//SJISの文字列が欲しい。
						int	Size = _tcslen( m_szTextTip1 );
						m_szTextTip2[ WideCharToMultiByte( CP_ACP, 0, m_szTextTip1, Size, m_szTextTip2, Size, 0, 0 ) ] = 0;
						LPNMTTDISPINFOA	lpnmtdia = (LPNMTTDISPINFOA)lParam;
						lpnmtdia->lpszText = m_szTextTip2;
						lpnmtdia->hinst    = NULL;
#else
						lpnmtdi->lpszText = m_szTextTip1;
						lpnmtdi->hinst    = NULL;
#endif	//UNICODE
					}

					return 0L;
				}
			}
		}
	}

	return 1L;
}

/*! @brief タブ順序変更処理

	@param[in] nSrcTab 移動するタブのインデックス
	@param[in] nDstTab 移動先タブのインデックス

	@date 2005.09.01 ryoji 新規作成
	@date 2007.07.07 genta ウィンドウリスト操作部をCShareData::ReorderTab()へ

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
	tcitem.lParam = (LPARAM)0;
	TabCtrl_GetItem( m_hwndTab, nSrcTab, &tcitem );
	hwndSrc = (HWND)tcitem.lParam;

	tcitem.mask   = TCIF_PARAM;
	tcitem.lParam = (LPARAM)0;
	TabCtrl_GetItem( m_hwndTab, nDstTab, &tcitem );
	hwndDst = (HWND)tcitem.lParam;

	//	2007.07.07 genta CShareData::ReorderTabとして独立
	if( ! CShareData::getInstance()->ReorderTab( hwndSrc, hwndDst ) ){
		return FALSE;
	}

	// 再表示メッセージをブロードキャストする。
	int nGroup = CShareData::getInstance()->GetGroupId( m_hwndParent );
	CShareData::getInstance()->PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY, (WPARAM)TWNT_REFRESH, (LPARAM)FALSE, m_hwndParent, nGroup );

	return TRUE;
}

/** タブ分離処理

	@date 2007.06.20 ryoji 新規作成
*/
BOOL CTabWnd::SeparateGroup( HWND hwndSrc, HWND hwndDst, POINT ptDrag, POINT ptDrop )
{
	HWND hWnd = m_hwndParent;
	if( hWnd != ::GetForegroundWindow() )
		return FALSE;
	// 最大化時は切り離さない（最大化時の処理コードも残してあるが動作不安定のため、無理しないでおく）
	// Note. マルチモニタで別モニタに切り離すとき、
	//       両モニタの画面がアクティブになってしまうことがある
	//       この場合エディタ内部状態もおかしくなっており、
	//       結合しなおして別タブ選択すると離れてしまったりする（Win XPだけ？）
	if( ::IsZoomed( hWnd ) )
		return FALSE;
	if(	hWnd != CShareData::getInstance()->GetTopEditWnd( hwndSrc ) )
		return FALSE;
	if( hwndDst != NULL && hwndDst != CShareData::getInstance()->GetTopEditWnd( hwndDst ) )
		return FALSE;
	if( hwndSrc == hwndDst )
		return TRUE;

	EditNode* pSrcEditNode = CShareData::getInstance()->GetEditNode( hwndSrc );
	EditNode* pDstEditNode = CShareData::getInstance()->GetEditNode( hwndDst );

	// グループ変更するウィンドウが先頭ウィンドウなら次のウィンドウを可視にする（手前には出さない）
	// そうでなければ新規グループになる場合に別ウィンドウよりは手前に表示されるよう不可視のまま先頭ウィンドウのすぐ後ろにもってきておく
	HWND hwndTop = CShareData::getInstance()->GetTopEditWnd( hwndSrc );
	bool bSrcIsTop = ( hwndSrc == hwndTop );
	if( bSrcIsTop )
	{
		EditNode* pNextEditNode = CShareData::getInstance()->GetEditNodeAt( pSrcEditNode->m_nGroup, 1 );
		if( pNextEditNode != NULL )
		{
			DWORD dwResult;
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
	hwndDst = CShareData::getInstance()->SeparateGroup( hwndSrc, hwndDst, bSrcIsTop, notifygroups );

	WINDOWPLACEMENT wp;
	RECT rcDstWork;
	GetMonitorWorkRect( ptDrop, &rcDstWork );
	wp.length = sizeof(WINDOWPLACEMENT);
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
			if( ::IsZoomed( hwndSrc ) )
			{
				// もともと最大化されていた場合は一旦通常サイズで表示しないと新しいサイズが記憶されない
				wp.showCmd = SW_SHOWNOACTIVATE;
				::SetWindowPlacement( hwndSrc, &wp );
				wp.showCmd = SW_SHOWMAXIMIZED;
			}
		}

		if( wp.showCmd != SW_SHOWMAXIMIZED )
			wp.showCmd = SW_SHOWNOACTIVATE;	// 移動ウィンドウがアクティブでないときはそのままなるべくアクティブ化しない
		::SetWindowPlacement( hwndSrc, &wp );
	}
	else
	{	// 既存グループのウィンドウ処理
		// 移動先の WS_EX_TOPMOST 状態を引き継ぐ
		HWND hWndInsertAfter = (::GetWindowLongPtr( hwndDst, GWL_EXSTYLE ) & WS_EX_TOPMOST)? HWND_TOPMOST: HWND_NOTOPMOST;
		::SetWindowPos( hwndSrc, hWndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
		if( bSrcIsTop )
		{
			// ウィンドウを移動先に表示する（先頭ウィンドウが既存グループに移動した）
			::GetWindowPlacement( hwndDst, &wp );
			if( ::IsZoomed( hwndSrc ) && wp.showCmd == SW_SHOWMAXIMIZED )
			{
				wp.showCmd = SW_SHOWNOACTIVATE;
				::SetWindowPlacement( hwndSrc, &wp );
				wp.showCmd = SW_SHOWMAXIMIZED;
			}
			::SetWindowPlacement( hwndSrc, &wp );
			::ShowWindow( hwndDst, SW_HIDE );	// 移動先の以前の先頭ウィンドウを消す
		}
	}

	// 再表示メッセージをブロードキャストする。
	//	2007.07.07 genta 2回ループに
	for( int group = 0; group < sizeof( notifygroups )/sizeof( notifygroups[0] ); group++ ){
		CShareData::getInstance()->PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY,
			(WPARAM)TWNT_REFRESH, (LPARAM)bSrcIsTop, NULL, notifygroups[group] );
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
	tcitem.lParam = (LPARAM)0;
	if( !TabCtrl_GetItem( m_hwndTab, nTab, &tcitem ) )
		return 1L;
	HWND hWnd = (HWND)tcitem.lParam;

	// 対象ウインドウをアクティブにする。
	ShowHideWindow( hWnd, TRUE );

	// コマンドを対象ウインドウに送る。
	::PostMessage( hWnd, WM_COMMAND, MAKEWPARAM( nId, 0 ), (LPARAM)NULL );

	return 0L;
}


CTabWnd::CTabWnd()
  : m_bVisualStyle( FALSE ),		// 2007.04.01 ryoji
    m_eDragState( DRAG_NONE ),
    m_bHovering( FALSE ),	//	2006.02.01 ryoji
    m_bListBtnHilighted( FALSE ),	//	2006.02.01 ryoji
    m_bCloseBtnHilighted( FALSE ),	//	2006.10.21 ryoji
    m_eCaptureSrc( CAPT_NONE )	//	2006.11.30 ryoji
{
	strcat( m_szClassInheritances, _T("::CTabWnd") );

	m_pszClassName = _T("CTabWnd");
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_hInstance  = NULL;
	m_hwndParent = NULL;
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
	/* 初期化 */
	m_hInstance  = hInstance;
	m_hwndParent = hwndParent;
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

	/* ウィンドウクラス作成 */
	RegisterWC(
		/* WNDCLASS用 */
		NULL,								// Handle to the class icon.
		NULL,								//Handle to a small icon
		::LoadCursor( NULL, IDC_ARROW ),	// Handle to the class cursor.
		// 2006.01.30 ryoji 背景は WM_PAINT で描画するほうがちらつかない（と思う）
		//(HBRUSH)(COLOR_3DFACE + 1),			// Handle to the class background brush.
		NULL,								// Handle to the class background brush.
		NULL,								// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		m_pszClassName						// Pointer to a null-terminated string or is an atom.
	);

	/* 基底クラスメンバ呼び出し */
	CWnd::Create(
		/* CreateWindowEx()用 */
		0,									// extended window style
		m_pszClassName,						// Pointer to a null-terminated string or is an atom.
		m_pszClassName,						// pointer to window name
		WS_CHILD/* | WS_VISIBLE*/,			// window style	// 2007.03.08 ryoji WS_VISIBLE 除去
		// 2006.01.30 ryoji 初期配置見直し
		// ※タブ非表示 -> 表示切替で編集ウィンドウにゴミが表示されることがあるので初期幅はゼロに
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		0,									// window width
		TAB_WINDOW_HEIGHT,					// window height
		NULL								// handle to menu, or child-window identifier
	);

	//タブウインドウを作成する。
	m_hwndTab = ::CreateWindow(
		WC_TABCONTROL,
		_T(""),
		//	2004.05.22 MIK 消えるTAB対策でWS_CLIPSIBLINGS追加
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		// 2006.01.30 ryoji 初期配置見直し
		TAB_MARGIN_LEFT,
		TAB_MARGIN_TOP,
		0,
		TAB_WINDOW_HEIGHT,
		m_hWnd,
		(HMENU)NULL,
		m_hInstance,
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
		lngStyle |= TCS_TABS | TCS_SINGLELINE | TCS_FOCUSNEVER | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT;	// 2006.01.28 ryoji
		//lngStyle &= ~(TCS_BUTTONS | TCS_SINGLELINE);	//2004.01.31
		//lngStyle |= TCS_TABS | TCS_MULTILINE;
		::SetWindowLongPtr( m_hwndTab, GWL_STYLE, lngStyle );
		TabCtrl_SetItemSize( m_hwndTab, MAX_TABITEM_WIDTH, TAB_ITEM_HEIGHT );	// 2006.01.28 ryoji

		/* 表示用フォント */
		/* LOGFONTの初期化 */
		LOGFONT	lf;
		::ZeroMemory( &lf, sizeof(LOGFONT) );
		lf.lfHeight			= -12;
		lf.lfWidth			= 0;
		lf.lfEscapement		= 0;
		lf.lfOrientation	= 0;
		lf.lfWeight			= 400;
		lf.lfItalic			= 0x0;
		lf.lfUnderline		= 0x0;
		lf.lfStrikeOut		= 0x0;
		lf.lfCharSet		= 0x80;
		lf.lfOutPrecision	= 0x3;
		lf.lfClipPrecision	= 0x2;
		lf.lfQuality		= 0x1;
		lf.lfPitchAndFamily	= 0x31;
		_tcscpy( lf.lfFaceName, _T("ＭＳ Ｐゴシック") );
		m_hFont = ::CreateFontIndirect( &lf );
		
		/* フォント変更 */
		::SendMessage( m_hwndTab, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0) );

		//ツールチップを作成する。
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
			m_hWnd, //m_hwndTab,
			NULL,
			m_hInstance,
			NULL
			);

		// ツールチップをマルチライン可能にする（SHRT_MAX: Win95でINT_MAXだと表示されない）	// 2007.03.03 ryoji
		::SendMessage( m_hwndToolTip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)SHRT_MAX );

		// タブバーにツールチップを追加する
		TOOLINFO	ti;
		ti.cbSize      = sizeof( TOOLINFO );
		ti.uFlags      = TTF_SUBCLASS | TTF_IDISHWND;	// TTF_IDISHWND: uId は HWND で rect は無視（HWND 全体）
		ti.hwnd        = m_hWnd;
		ti.hinst       = m_hInstance;
		ti.uId         = (UINT)m_hWnd;
		ti.lpszText    = NULL;
		ti.rect.left   = 0;
		ti.rect.top    = 0;
		ti.rect.right  = 0;
		ti.rect.bottom = 0;
		::SendMessage( m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti );

		// タブバー内のタブコントロールにツールチップを追加する
		TabCtrl_SetToolTips( m_hwndTab, m_hwndToolTip );

		// 2006.02.22 ryoji イメージリストを初期化する
		InitImageList();

		//TabCtrl_DeleteAllItems( m_hwndTab );
		//::ShowWindow( m_hwndTab, SW_HIDE );
		Refresh();	// タブ非表示から表示に切り替わったときに各ウィンドウの情報をタブ登録する必要がある
	}

	return m_hWnd;
}

/* ウィンドウ クローズ */
void CTabWnd::Close( void )
{
	if( m_hWnd )
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

		::DestroyWindow( m_hWnd );
		m_hWnd = NULL;
	}
}

//WM_SIZE処理
LRESULT CTabWnd::OnSize( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	RECT	rcParent;

	if( NULL == m_hWnd || NULL == m_hwndTab ) return 0L;

	::GetWindowRect( m_hWnd, &rcParent );

	::MoveWindow( m_hwndTab, TAB_MARGIN_LEFT, TAB_MARGIN_TOP, (rcParent.right - rcParent.left) - (TAB_MARGIN_LEFT + TAB_MARGIN_RIGHT), TAB_WINDOW_HEIGHT, TRUE );	// 2005.01.30 ryoji

	LayoutTab();	// 2006.01.28 ryoji タブのレイアウト調整処理

	::InvalidateRect( m_hWnd, NULL, FALSE );	//	2006.02.01 ryoji

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

	m_hWnd = NULL;

	return 0L;
}
 
/*! WM_LBUTTONDBLCLK処理
	@date 2006.03.26 ryoji 新規作成
*/
LRESULT CTabWnd::OnLButtonDblClk( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// 新規作成コマンドを実行する
	::SendMessage( m_hwndParent, WM_COMMAND, MAKEWPARAM( F_FILENEW, 0 ), (LPARAM)NULL );
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
	::GetClientRect( m_hWnd, &rc );

	// タブ一覧ボタン上ならタブ一覧メニュー（タブ名）を表示する
	GetListBtnRect( &rc, &rcBtn );
	if( ::PtInRect( &rcBtn, pt ) )
	{
		pt.x = rcBtn.left;
		pt.y = rcBtn.bottom;
		::ClientToScreen( m_hWnd, &pt );
		TabListMenu( pt, FALSE, FALSE, FALSE );	// タブ一覧メニュー（タブ名）
	}
	else
	{
		// 閉じるボタン上ならキャプチャー開始
		GetCloseBtnRect( &rc, &rcBtn );
		if( ::PtInRect( &rcBtn, pt ) )
		{
			m_eCaptureSrc = CAPT_CLOSE;	// キャプチャー元は閉じるボタン
			::SetCapture( m_hWnd );
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
	::GetClientRect( m_hWnd, &rc );

	if( ::GetCapture() == m_hWnd )	// 自ウィンドウがマウスキャプチャーしている?
	{
		if( m_eCaptureSrc == CAPT_CLOSE )	// キャプチャー元は閉じるボタン?
		{
			// 閉じるボタン上ならタブを閉じる
			GetCloseBtnRect( &rc, &rcBtn );
			if( ::PtInRect( &rcBtn, pt ) )
			{
				int nId;
				if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
				{
					if( !m_pShareData->m_Common.m_bTab_CloseOneWin )
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
				::PostMessage( m_hwndParent, WM_COMMAND, MAKEWPARAM( nId, 0 ), (LPARAM)NULL );
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
	::GetClientRect( m_hWnd, &rc );

	// タブ一覧ボタン上ならタブ一覧メニュー（フルパス）を表示する	// 2006.11.30 ryoji
	GetListBtnRect( &rc, &rcBtn );
	if( ::PtInRect( &rcBtn, pt ) )
	{
		pt.x = rcBtn.left;
		pt.y = rcBtn.bottom;
		::ClientToScreen( m_hWnd, &pt );
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
		HFONT hfnt = CreateMenuFont();
		HFONT hfntOld = (HFONT)::SelectObject( hdc, hfnt );
		SIZE size;

		::GetTextExtentPoint32( hdc, pData->szText, ::lstrlen(pData->szText), &size );

		lpmis->itemHeight = ::GetSystemMetrics( SM_CYMENU );
		lpmis->itemWidth = size.cx + CX_SMICON + 8;

		::SelectObject( hdc, hfntOld );
		::DeleteObject( hfnt );
		::ReleaseDC( hwnd, hdc );
	}

	return 0L;
}

/*!	WM_DRAWITEM処理
	@date 2006.02.01 ryoji 新規作成
*/
LRESULT CTabWnd::OnDrawItem( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*)lParam;
	if( lpdis->CtlType == ODT_MENU )
	{
		// タブ一覧メニューを描画する
		TABMENU_DATA* pData = (TABMENU_DATA*)lpdis->itemData;

		HDC hdc = lpdis->hDC;
		RECT rcItem = lpdis->rcItem;

		// 状態に従ってテキストと背景色を決める
		COLORREF clrText;
		COLORREF clrBk;
		if (lpdis->itemState & ODS_SELECTED)
		{
			clrText = ::GetSysColor( COLOR_HIGHLIGHTTEXT );
			clrBk = COLOR_HIGHLIGHT;
		}
		else
		{
			clrText = ::GetSysColor( COLOR_MENUTEXT );
			clrBk = COLOR_MENU;
		}

		// 背景描画
		::FillRect( hdc, &rcItem, (HBRUSH)(clrBk + 1) );

		// アイコン描画
		if( NULL != m_hIml && 0 <= pData->iImage )
		{
			int top = rcItem.top + ( rcItem.bottom - rcItem.top - CY_SMICON ) / 2;
			ImageList_Draw( m_hIml, pData->iImage, lpdis->hDC, rcItem.left + 2, top, ILD_TRANSPARENT );
		}

		// テキスト描画
		COLORREF clrTextOld = ::SetTextColor( hdc, clrText );
		int iBkModeOld = ::SetBkMode( hdc, TRANSPARENT );
		HFONT hfnt = CreateMenuFont();
		HFONT hfntOld = (HFONT)::SelectObject( hdc, hfnt );
		RECT rcText = rcItem;
		rcText.left += (CX_SMICON + 8);

		::DrawText( hdc, pData->szText, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER );

		::SetTextColor( hdc, clrTextOld );
		::SetBkMode( hdc, iBkModeOld );
		::SelectObject( hdc, hfntOld );
		::DeleteObject( hfnt );

		// チェック状態なら外枠描画
		if( lpdis->itemState & ODS_CHECKED )
		{
			HPEN hpen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_HIGHLIGHT ) );
			HBRUSH hbr = (HBRUSH)::GetStockObject( NULL_BRUSH );
			HPEN hpenOld = (HPEN)::SelectObject( hdc, hpen );
			HBRUSH hbrOld = (HBRUSH)::SelectObject( hdc, hbr );

			::Rectangle( hdc, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom );

			::SelectObject( hdc, hpenOld );
			::SelectObject( hdc, hbrOld );
			::DeleteObject( hpen );
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
			pszTip = m_szTextTip1;
			_tcscpy( m_szTextTip1, _T("左クリック: タブ名一覧\n右クリック: パス名一覧") );
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
			pszTip = m_szTextTip1;
			if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
			{
				if( !m_pShareData->m_Common.m_bTab_CloseOneWin )
				{
					_tcscpy( m_szTextTip1, _T("タブを閉じる") );
				}
				else
				{
					::LoadString( m_hInstance, F_GROUPCLOSE, m_szTextTip1, sizeof(m_szTextTip1)/sizeof(TCHAR) );
					m_szTextTip1[sizeof(m_szTextTip1)/sizeof(TCHAR) - 1] = _T('\0');
				}
			}
			else
			{
				::LoadString( m_hInstance, F_EXITALLEDITORS, m_szTextTip1, sizeof(m_szTextTip1)/sizeof(TCHAR) );
				m_szTextTip1[sizeof(m_szTextTip1)/sizeof(TCHAR) - 1] = _T('\0');
			}
		}
	}

	// ツールチップ更新	// 2007.03.05 ryoji
	if( pszTip != (LPTSTR)-1L )	// ボタンへの出入りがあった?
	{
		TOOLINFO ti;
		::ZeroMemory( &ti, sizeof(ti) );
		ti.cbSize       = sizeof( TOOLINFO );
		ti.hwnd         = m_hWnd;
		ti.hinst        = m_hInstance;
		ti.uId          = (UINT)m_hWnd;
		ti.lpszText     = pszTip;
		::SendMessage( m_hwndToolTip, TTM_UPDATETIPTEXT, (WPARAM)0, (LPARAM)&ti );
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
			::SendMessage( hwnd, WM_MOUSEMOVE, 0, MAKELONG( pt.x, pt.y ) );
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

	hdc = ::BeginPaint( hwnd, &ps );

	// 背景を描画する
	::GetClientRect( hwnd, &rc );
	::FillRect( hdc, &rc, (HBRUSH)(COLOR_3DFACE + 1) );

	// ボタンを描画する
	DrawListBtn( hdc, &rc );
	DrawCloseBtn( hdc, &rc );	// 2006.10.21 ryoji 追加

	// 上側に境界線を描画する
	::DrawEdge(hdc, &rc, EDGE_ETCHED, BF_TOP);

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
			::ScreenToClient( m_hWnd, &pt );
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
				::ScreenToClient( m_hWnd, &ptREnd );
				if( rcCurSel.right > ptREnd.x )
					rcCurSel.right = ptREnd.x;	// 右端限界値
			}

			if( rcCurSel.left < rcCurSel.right )
			{
				HBRUSH hBr = ::CreateSolidBrush( RGB( 255, 128, 0 ) );
				::FillRect( hdc, &rcCurSel, hBr );
				::DeleteObject( hBr );
			}
		}
	}

	::EndPaint( hwnd, &ps );

	return 0L;
}

/*! WM_NOTIFY処理

	@date 2005.09.01 ryoji ウィンドウ切り替えは OnTabLButtonUp() に移動
*/
LRESULT CTabWnd::OnNotify( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// 2005.09.01 ryoji ウィンドウ切り替えは OnTabLButtonUp() に移動
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

			_tcscpy( szName, _T("(無題)") );

			tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
			tcitem.pszText = szName;
			tcitem.lParam  = (LPARAM)lParam;

			// 2006.01.28 ryoji タブにアイコンイメージを追加する
			tcitem.mask |= TCIF_IMAGE;
			tcitem.iImage = GetImageIndex( NULL );

			TabCtrl_InsertItem( m_hwndTab, nCount, &tcitem );
			nIndex = nCount;
		}

		if( CShareData::getInstance()->IsTopEditWnd( m_hwndParent ) )
		{
			//自分ならアクティブに
			if( !::IsWindowVisible( m_hwndParent ) )
			{
				ShowHideWindow( m_hwndParent, TRUE );
				//ここに来たということはすでにアクティブ
				//コマンド実行時のアウトプットで問題があるのでアクティブにする
			}

			TabCtrl_SetCurSel( m_hwndTab, nIndex );

			// 自分以外を隠す
			HideOtherWindows( m_hwndParent );
		}
		break;

	case TWNT_DEL:	//ウインドウ削除
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 != nIndex )
		{
			if( CShareData::getInstance()->IsTopEditWnd( m_hwndParent ) )
			{
				if( !::IsWindowVisible( m_hwndParent ) )
				{
					ShowHideWindow( m_hwndParent, TRUE );
					ForceActiveWindow( m_hwndParent );
				}
			}
			TabCtrl_DeleteItem( m_hwndTab, nIndex );

			// 2005.09.01 ryoji スクロール位置調整
			// （右端のほうのタブアイテムを削除したとき、スクロール可能なのに右に余白ができることへの対策）
			hwndUpDown = ::FindWindowEx( m_hwndTab, NULL, UPDOWN_CLASS, 0 );	// タブ内の Up-Down コントロール
			if( hwndUpDown != NULL && ::IsWindowVisible( hwndUpDown ) )	// 2007.09.24 ryoji hwndUpDown可視の条件追加
			{
				nScrollPos = LOWORD( ::SendMessage( hwndUpDown, UDM_GETPOS, (WPARAM)0, (LPARAM)0 ) );

				// 現在位置 nScrollPos と画面表示とを一致させる
				::SendMessage( m_hwndTab, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, LOWORD( nScrollPos ) ), (LPARAM)NULL );	// 設定位置にタブをスクロール
			}
		}
		break;

	case TWNT_ORDER:	//ウインドウ順序変更
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 != nIndex )
		{
			if( CShareData::getInstance()->IsTopEditWnd( m_hwndParent ) )
			{
				//自分ならアクティブに
				if( !::IsWindowVisible( m_hwndParent ) )
				{
					ShowHideWindow( m_hwndParent, TRUE );
				}
				//ここに来たということはすでにアクティブ

				// 自タブアイテムを強制的に可視位置にするために、
				// 自タブアイテム選択前に一時的に画面左端のタブアイテムを選択する
				hwndUpDown = ::FindWindowEx( m_hwndTab, NULL, UPDOWN_CLASS, 0 );	// タブ内の Up-Down コントロール
				nScrollPos = ( hwndUpDown != NULL && ::IsWindowVisible( hwndUpDown ) )? LOWORD( ::SendMessage( hwndUpDown, UDM_GETPOS, (WPARAM)0, (LPARAM)0 ) ): 0;	// 2007.09.24 ryoji hwndUpDown可視の条件追加
				TabCtrl_SetCurSel( m_hwndTab, nScrollPos );
				TabCtrl_SetCurSel( m_hwndTab, nIndex );

				// 自分以外を隠す
				// （連続切替時に TWNT_ORDER が大量発生・交錯して？画面がすべて消えてしまったりするのを防ぐ）
				HideOtherWindows( m_hwndParent );
			}
		}
		else
		{
			//指定のウインドウがないので再表示
			if( !CShareData::getInstance()->IsSameGroup( m_hwndParent, (HWND)lParam ) )
				Refresh();
		}
		break;

	case TWNT_FILE:	//ファイル名変更
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 != nIndex )
		{
			TCITEM	tcitem;
			CRecent	cRecentEditNode;
			TCHAR	szName[1024];
			//	Jun. 19, 2004 genta
			EditNode	*p;
			p = CShareData::getInstance()->GetEditNode( (HWND)lParam );
			GetTabName( p, FALSE, TRUE, szName, sizeof(szName)/sizeof(TCHAR) );

			tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
			tcitem.pszText = szName;
			tcitem.lParam  = (LPARAM)lParam;

			// 2006.01.28 ryoji タブのアイコンイメージを変更する
			tcitem.mask |= TCIF_IMAGE;
			tcitem.iImage = GetImageIndex( p );

			TabCtrl_SetItem( m_hwndTab, nIndex, &tcitem );
		}
		else
		{
			//指定のウインドウがないので再表示
			if( !CShareData::getInstance()->IsSameGroup( m_hwndParent, (HWND)lParam ) )
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
		if( CShareData::getInstance()->IsTopEditWnd( m_hwndParent ) )
		{
			if( !::IsWindowVisible( m_hwndParent ) )
			{
				//表示状態とする(フォアグラウンドにはしない)
				TabWnd_ActivateFrameWindow( m_hwndParent, false );
			}
			// 自分以外を隠す
			HideOtherWindows( m_hwndParent );
		}
		break;
	//End 2004.07.14 Kazika

	//Start 2004.08.27 Kazika 追加
	//タブモード無効になった場合、隠れていたウィンドウは表示状態となる
	case TWNT_MODE_DISABLE:
		Refresh();
		if( !::IsWindowVisible( m_hwndParent ) )
		{
			//表示状態とする(フォアグラウンドにはしない)
			TabWnd_ActivateFrameWindow( m_hwndParent, false );
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

	LayoutTab();	// 2006.01.28 ryoji タブのレイアウト調整処理

	//更新
	::InvalidateRect( m_hwndTab, NULL, TRUE );
	::InvalidateRect( m_hWnd, NULL, TRUE );		// 2006.10.21 ryoji タブ内ボタン再描画のために追加

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
	TCHAR		szName[2048];
	EditNode	*pEditNode;
	int			nCount;
	int			nGroup;
	int			nTab;
	int			nSel;
	int			nCurTab;
	int			nCurSel;
	int			i;
	int			j;

	if( NULL == m_hwndTab ) return;

	pEditNode = NULL;
	nCount = CShareData::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );

	// 自ウィンドウのグループ番号を調べる
	for( i = 0; i < nCount; i++ )
	{
		if( pEditNode[i].m_hWnd == m_hwndParent ){
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
		::SendMessage( m_hwndTab, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0 );	// 2005.09.01 ryoji 再描画禁止

		if( bRebuild )
			TabCtrl_DeleteAllItems( m_hwndTab );	// 作成しなおす

		// 作成するタブ数と選択状態にするタブ位置（自ウィンドウの位置）を調べる
		for( i = 0, j = 0; i < nCount; i++ )
		{
			if( pEditNode[i].m_nGroup != nGroup )
				continue;
			if( pEditNode[i].m_bClosing )	// このあとすぐに閉じるウィンドウなのでタブ表示しない
				continue;
			if( pEditNode[i].m_hWnd == m_hwndParent )
				nSel = j;	// 選択状態にするタブ位置
			j++;
		}
		nTab = j;	// 作成するタブ数

		// タブが無ければ１つ作成して選択状態にする（自ウィンドウのタブ用）
		_tcscpy( szName, _T("") );
		tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
		tcitem.pszText = szName;
		tcitem.lParam  = (LPARAM)m_hwndParent;
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

			GetTabName( &pEditNode[i], FALSE, TRUE, szName, sizeof(szName)/sizeof(TCHAR) );

			tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
			tcitem.pszText = szName;
			tcitem.lParam  = (LPARAM)pEditNode[i].m_hWnd;

			// 2006.01.28 ryoji タブにアイコンを追加する
			tcitem.mask |= TCIF_IMAGE;
			tcitem.iImage = GetImageIndex( &pEditNode[i] );

			TabCtrl_SetItem( m_hwndTab, j, &tcitem );
			j++;
		}

		::SendMessage( m_hwndTab, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0 );	// 2005.09.01 ryoji 再描画許可

		// 選択タブを可視位置にする
		if( bEnsureVisible )
		{
			// TabCtrl_SetCurSel() を使うと等幅タブのときに選択タブが左端のほうに寄ってしまう
//			TabCtrl_SetCurSel( m_hwndTab, 0 );
//			TabCtrl_SetCurSel( m_hwndTab, nSel );
			::PostMessage( m_hwndTab, TCM_SETCURSEL, 0, 0 );
			::PostMessage( m_hwndTab, TCM_SETCURSEL, nSel, 0 );
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
	if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		HWND hwnd = m_hwndParent;	// 自身の編集ウィンドウ
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
			pEditNode = CShareData::getInstance()->GetTopEditNode( hwnd );
			if( pEditNode == NULL )
			{
				::ShowWindow( hwnd, SW_SHOWNA );
				return;
			}
			HWND hwndInsertAfter = pEditNode->m_hWnd;
			wp.length = sizeof( WINDOWPLACEMENT );
			::GetWindowPlacement( hwndInsertAfter, &wp );
			if( wp.showCmd == SW_SHOWMINIMIZED )
				wp.showCmd = pEditNode->m_showCmdRestore;
			::SetWindowPos( hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

			if( wp.showCmd == SW_SHOWMAXIMIZED && ::IsZoomed( hwnd ) )
			{
				WINDOWPLACEMENT wpCur;
				wpCur.length = sizeof( WINDOWPLACEMENT );
				::GetWindowPlacement( hwnd, &wpCur );
				if( !::EqualRect( &wp.rcNormalPosition, &wpCur.rcNormalPosition ) )
				{
					// ウィンドウの通常サイズが目的のサイズと違っているときは一旦通常サイズで表示してから最大化する
					// Note. マルチモニタで以前に別モニタで最大化されていた画面は一旦通常サイズに戻しておかないと元の別モニタ側に表示されてしまう
					wp.showCmd = SW_SHOWNOACTIVATE;
					::SetWindowPlacement( hwnd, &wp );	// 通常サイズ表示
					wp.showCmd = SW_SHOWMAXIMIZED;
				}
				else
				{
					wp.showCmd = SW_SHOWNA;
				}
			}
			else if( wp.showCmd != SW_SHOWMAXIMIZED )
			{
				wp.showCmd = SW_SHOWNOACTIVATE;
			}
			::SetWindowPlacement( hwnd, &wp );	// 位置を復元する
			::UpdateWindow( hwnd );	// 強制描画
		}
	}
}

void CTabWnd::ShowHideWindow( HWND hwnd, BOOL bDisp )
{
	if( NULL == hwnd ) return;

	if( bDisp )
	{
		if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
		{
			if( m_pShareData->m_bEditWndChanging )
				return;	// 切替の最中(busy)は要求を無視する
			m_pShareData->m_bEditWndChanging = TRUE;	// 編集ウィンドウ切替中ON	2007.04.03 ryoji

			// 対象ウィンドウのスレッドに位置合わせを依頼する	// 2007.04.03 ryoji
			DWORD dwResult;
			::SendMessageTimeout( hwnd, MYWM_TAB_WINDOW_NOTIFY, TWNT_WNDPL_ADJUST, (LPARAM)NULL,
				SMTO_ABORTIFHUNG | SMTO_BLOCK, 10000, &dwResult );
		}

		TabWnd_ActivateFrameWindow( hwnd );

		m_pShareData->m_bEditWndChanging = FALSE;	// 編集ウィンドウ切替中OFF	2007.04.03 ryoji
	}
	else
	{
		if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
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
	if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		HWND hwnd;
		int	i;
		for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
		{
			hwnd = m_pShareData->m_pEditArr[i].m_hWnd;
			if( CShareData::getInstance()->IsEditWnd( hwnd ) )
			{
				if( !CShareData::getInstance()->IsSameGroup( hwndExclude, hwnd ) )
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

	::SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)dwTime, 0 );

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
	// アイコンの表示を切り替える
	HIMAGELIST hImg = TabCtrl_GetImageList( m_hwndTab );
	if( NULL == hImg && m_pShareData->m_Common.m_bDispTabIcon )
	{
		if( NULL != InitImageList() )
			Refresh( TRUE, TRUE );
	}
	else if( NULL != hImg && !m_pShareData->m_Common.m_bDispTabIcon )
	{
		InitImageList();
	}

	// タブのアイテム幅の等幅を切り替える
	UINT lStyle;
	lStyle = (UINT)::GetWindowLongPtr( m_hwndTab, GWL_STYLE );
	if( (lStyle & TCS_FIXEDWIDTH) && !m_pShareData->m_Common.m_bSameTabWidth )
	{
		lStyle &= ~(TCS_FIXEDWIDTH | TCS_FORCELABELLEFT);
		::SetWindowLongPtr( m_hwndTab, GWL_STYLE, lStyle );
		return;
	}
	else if( !(lStyle & TCS_FIXEDWIDTH) && m_pShareData->m_Common.m_bSameTabWidth )
	{
		lStyle |= TCS_FIXEDWIDTH | TCS_FORCELABELLEFT;
		::SetWindowLongPtr( m_hwndTab, GWL_STYLE, lStyle );
	}

	if( !m_pShareData->m_Common.m_bSameTabWidth )
		return;	// アイテム幅の調整は不要

	// タブのアイテム幅を調整する
	RECT rcTab;
	int nCount;
	int cx;

	::GetClientRect( m_hwndTab, &rcTab );
	nCount = TabCtrl_GetItemCount( m_hwndTab );
	if( 0 < nCount )
	{
		cx = (rcTab.right - rcTab.left - 8) / nCount;
		if( MAX_TABITEM_WIDTH < cx )
			cx = MAX_TABITEM_WIDTH;
		else if( MIN_TABITEM_WIDTH > cx )
			cx = MIN_TABITEM_WIDTH;
		TabCtrl_SetItemSize( m_hwndTab, cx, TAB_ITEM_HEIGHT );
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
	if( m_pShareData->m_Common.m_bDispTabIcon )
	{
		// システムイメージリストを取得する
		// 注：複製後に差し替えて利用するアイコンには事前にアクセスしておかないとイメージが入らない
		//     ここでは「フォルダを閉じたアイコン」、「フォルダを開いたアイコン」を差し替え用として利用
		//     WinNT4.0 では SHGetFileInfo() の第一引数に同名を指定すると同じインデックスを返してくることがある？

		hImlSys = (HIMAGELIST)::SHGetFileInfo( _T(".0"), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES );
		if( NULL == hImlSys )
			goto l_end;
		m_iIconApp = sfi.iIcon;

		hImlSys = (HIMAGELIST)::SHGetFileInfo( _T(".1"), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES | SHGFI_OPENICON );
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
		m_hIconApp = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
		ImageList_ReplaceIcon( hImlNew, m_iIconApp, m_hIconApp );
		m_hIconGrep = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true );
		ImageList_ReplaceIcon( hImlNew, m_iIconGrep, m_hIconGrep );
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
			hImlSys = (HIMAGELIST)::SHGetFileInfo( szExt, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES );
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
			ImageList_ReplaceIcon( hImlNew, m_iIconGrep, m_hIconGrep );

			// タブにアイコンイメージを設定する
			if( m_pShareData->m_Common.m_bDispTabIcon )
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
	hImlNew = ImageList_Create( CX_SMICON, CY_SMICON, ILC_COLOR32 | ILC_MASK, 4, 4 );
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
	HPEN hpen, hpenOld;
	HBRUSH hbr, hbrOld;

	if( bBtnHilighted )
	{
		hpen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_HIGHLIGHT ) );
		hbr = (HBRUSH)::GetSysColorBrush( COLOR_MENU );
		hpenOld = (HPEN)::SelectObject( hdc, hpen );
		hbrOld = (HBRUSH)::SelectObject( hdc, hbr );
		::Rectangle( hdc, lprcBtn->left, lprcBtn->top, lprcBtn->right, lprcBtn->bottom );
		::SelectObject( hdc, hpenOld );
		::SelectObject( hdc, hbrOld );
		::DeleteObject( hpen );
	}
}

/*! 一覧ボタン描画処理
	@date 2006.02.01 ryoji 新規作成
	@date 2006.10.21 ryoji 背景描画を関数呼び出しに変更
*/
void CTabWnd::DrawListBtn( HDC hdc, const LPRECT lprcClient )
{
	const POINT ptBase[4] = { {4, 8}, {7, 11}, {8, 11}, {11, 8} };	// 描画イメージ形状
	POINT pt[4];
	int i;
	HPEN hpen, hpenOld;
	HBRUSH hbr, hbrOld;

	RECT rcBtn;
	GetListBtnRect( lprcClient, &rcBtn );
	DrawBtnBkgnd( hdc, &rcBtn, m_bListBtnHilighted );	// 2006.10.21 ryoji

	int nIndex = m_bListBtnHilighted? COLOR_MENUTEXT: COLOR_BTNTEXT;
	hpen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( nIndex ) );
	hbr = (HBRUSH)::GetSysColorBrush( nIndex );
	hpenOld = (HPEN)::SelectObject( hdc, hpen );
	hbrOld = (HBRUSH)::SelectObject( hdc, hbr );
	for( i = 0; i < sizeof(ptBase)/sizeof(ptBase[0]); i++ )
	{
		pt[i].x = ptBase[i].x + rcBtn.left;
		pt[i].y = ptBase[i].y + rcBtn.top;
	}
	::Polygon( hdc, pt, sizeof(pt)/sizeof(pt[0]) );
	::SelectObject( hdc, hpenOld );
	::SelectObject( hdc, hbrOld );
	::DeleteObject( hpen );
}

/*! 閉じるボタン描画処理
	@date 2006.10.21 ryoji 新規作成
*/
void CTabWnd::DrawCloseBtn( HDC hdc, const LPRECT lprcClient )
{
	const POINT ptBase1[6][2] =	// [x]描画イメージ形状（直線6本）
	{
		{{4, 5}, {12, 13}},
		{{4, 4}, {13, 13}},
		{{5, 4}, {13, 12}},
		{{11, 4}, {3, 12}},
		{{12, 4}, {3, 13}},
		{{12, 5}, {4, 13}}
	};
	const POINT ptBase2[10][2] = // [xx]描画イメージ形状（矩形10個）
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
	HPEN hpen, hpenOld;
	HBRUSH hbr, hbrOld;

	RECT rcBtn;
	GetCloseBtnRect( lprcClient, &rcBtn );

	// ボタンの左側にセパレータを描画する	// 2007.02.27 ryoji
	hpen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_3DSHADOW ) );
	hpenOld = (HPEN)::SelectObject( hdc, hpen );
	::MoveToEx( hdc, rcBtn.left - 4, rcBtn.top + 1, NULL );
	::LineTo( hdc, rcBtn.left - 4, rcBtn.bottom - 1 );
	::SelectObject( hdc, hpenOld );
	::DeleteObject( hpen );

	DrawBtnBkgnd( hdc, &rcBtn, m_bCloseBtnHilighted );

	int nIndex = m_bCloseBtnHilighted? COLOR_MENUTEXT: COLOR_BTNTEXT;
	hpen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( nIndex ) );
	hbr = (HBRUSH)::GetSysColorBrush( nIndex );
	hpenOld = (HPEN)::SelectObject( hdc, hpen );
	hbrOld = (HBRUSH)::SelectObject( hdc, hbr );
	if( m_pShareData->m_Common.m_bDispTabWnd &&
		!m_pShareData->m_Common.m_bDispTabWndMultiWin &&
		!m_pShareData->m_Common.m_bTab_CloseOneWin			// 2007.02.13 ryoji 条件追加（ウィンドウの閉じるボタンは全部閉じる）
		)
	{
		// [x]を描画（直線6本）
		for( i = 0; i < sizeof(ptBase1)/sizeof(ptBase1[0]); i++ )
		{
			pt[0].x = ptBase1[i][0].x + rcBtn.left;
			pt[0].y = ptBase1[i][0].y + rcBtn.top;
			pt[1].x = ptBase1[i][1].x + rcBtn.left;
			pt[1].y = ptBase1[i][1].y + rcBtn.top;
			::MoveToEx( hdc, pt[0].x, pt[0].y, NULL );
			::LineTo( hdc, pt[1].x, pt[1].y );
		}
	}
	else
	{
		 // [xx]を描画（矩形10個）
		for( i = 0; i < sizeof(ptBase2)/sizeof(ptBase2[0]); i++ )
		{
			pt[0].x = ptBase2[i][0].x + rcBtn.left;
			pt[0].y = ptBase2[i][0].y + rcBtn.top;
			pt[1].x = ptBase2[i][1].x + rcBtn.left;
			pt[1].y = ptBase2[i][1].y + rcBtn.top;
			::Rectangle( hdc, pt[0].x, pt[0].y, pt[1].x, pt[1].y );
		}
	}
	::SelectObject( hdc, hpenOld );
	::SelectObject( hdc, hbrOld );
	::DeleteObject( hpen );
}

/*! 一覧ボタンの矩形取得処理
	@date 2006.02.01 ryoji 新規作成
*/
void CTabWnd::GetListBtnRect( const LPRECT lprcClient, LPRECT lprc )
{
	*lprc = rcBtnBase;
	::OffsetRect(lprc, lprcClient->right - TAB_MARGIN_RIGHT + 4, lprcClient->top + TAB_MARGIN_TOP + 2 );
}

/*! 閉じるボタンの矩形取得処理
	@date 2006.10.21 ryoji 新規作成
*/
void CTabWnd::GetCloseBtnRect( const LPRECT lprcClient, LPRECT lprc )
{
	*lprc = rcBtnBase;
	::OffsetRect(lprc, lprcClient->right - TAB_MARGIN_RIGHT + 4 + (rcBtnBase.right - rcBtnBase.left) + 7, lprcClient->top + TAB_MARGIN_TOP + 2 );
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
		::lstrcpyn( pszText, _T("(無題)"), nLen );
	}
	else if( !bFull || pEditNode->m_szFilePath[0] == '\0' )
	{
		if( pEditNode->m_szTabCaption[0] )
		{
			::lstrcpyn( pszText, pEditNode->m_szTabCaption, nLen );
		}
		else
		{
			::lstrcpyn( pszText, _T("(無題)"), nLen );
		}
	}
	else
	{
		// フルパス名を簡易名に変換する
		CShareData::getInstance()->GetTransformFileNameFast( pEditNode->m_szFilePath, pszText, nLen );
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
		bFull = m_pShareData->m_Common.m_bTab_ListFull;

	do
	{
		EditNode* pEditNode;
		int i;
		int nGroup;
		int nSelfTab;
		int nTab;
		int nCount;
		
		// タブメニュー用の情報を取得する
		nCount = CShareData::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
		if( 0 >= nCount )
			return 0L;

		TABMENU_DATA* pData = new TABMENU_DATA[nCount];	// タブメニュー用の情報

		// 自ウィンドウのグループ番号を調べる
		for( i = 0; i < nCount; i++ )
		{
			if( pEditNode[i].m_hWnd == m_hwndParent )
			{
				nGroup = pEditNode[i].m_nGroup;
				break;
			}
		}

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
				GetTabName( &pEditNode[i], bFull, TRUE, pData[nSelfTab].szText, sizeof(pData[0].szText) );
				pData[nSelfTab].hwnd = pEditNode[i].m_hWnd;
				pData[nSelfTab].iItem = i;
				pData[nSelfTab].iImage = GetImageIndex( &pEditNode[i] );
				nSelfTab++;
			}
			// 表示文字でソートする
			if( nSelfTab > 0 && m_pShareData->m_Common.m_bSortTabList )	// 2006.03.23 fon 変更
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
			GetTabName( &pEditNode[i], bFull, TRUE, pData[nTab].szText, sizeof(pData[0].szText) );
			pData[nTab].hwnd = pEditNode[i].m_hWnd;
			pData[nTab].iItem = i;
			pData[nTab].iImage = GetImageIndex( &pEditNode[i] );
			nTab++;
		}
		// 表示文字でソートする
		if( nTab > nSelfTab && m_pShareData->m_Common.m_bSortTabList )
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
			if( pData[i].hwnd == m_hwndParent )
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
				::InsertMenu( hMenu, nSelfTab, MF_BYPOSITION, 101, _T("すべて表示(&A)") );
			}
			::InsertMenu( hMenu, nSelfTab, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// セパレータ
		}

		// 表示切替メニューを追加する
		if( bSel )
		{
			::InsertMenu( hMenu, 0, MF_BYPOSITION | MF_STRING, 100, bFull? _T("タブ名一覧に切替える(&W)"): _T("パス名一覧に切替える(&W)") );
			::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// セパレータ
		}

		// メニューを表示する
		// 2006.04.21 ryoji マルチモニタ対応の修正
		RECT rcWork;
		GetMonitorWorkRect( pt, &rcWork );	// モニタのワークエリア
		int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									( pt.x > rcWork.left )? pt.x: rcWork.left,
									( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
									0, m_hWnd, NULL);
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
		m_pShareData->m_Common.m_bTab_ListFull = bFull;

	return 0L;
}


/** 次のグループの先頭ウィンドウを探す
	@date 2007.06.20 ryoji 新規作成
*/
HWND CTabWnd::GetNextGroupWnd( void )
{
	HWND hwndRet = NULL;

	if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		EditNode* pWndArr;
		int i;
		int j;
		int n;

		n = CShareData::getInstance()->GetOpenedWindowArr( &pWndArr, FALSE, TRUE );	// グループ番号順ソート
		if( 0 == n )
			return NULL;
		for( i = 0; i < n; i++ )
		{
			if( pWndArr[i].m_hWnd == m_hwndParent )
				break;
		}
		if( i < n )
		{
			for( j = i + 1; j < n; j++ )
			{
				if( pWndArr[j].m_nGroup != pWndArr[i].m_nGroup )
				{
					hwndRet = CShareData::getInstance()->GetTopEditWnd( pWndArr[j].m_hWnd );
					break;
				}
			}
			if( j >= n )
			{
				for( j = 0; j < i; j++ )
				{
					if( pWndArr[j].m_nGroup != pWndArr[i].m_nGroup )
					{
						hwndRet = CShareData::getInstance()->GetTopEditWnd( pWndArr[j].m_hWnd );
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
	if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		EditNode* pWndArr;
		int i;
		int j;
		int n;

		n = CShareData::getInstance()->GetOpenedWindowArr( &pWndArr, FALSE, TRUE );	// グループ番号順ソート
		if( 0 == n )
			return NULL;
		for( i = 0; i < n; i++ )
		{
			if( pWndArr[i].m_hWnd == m_hwndParent )
				break;
		}
		if( i < n )
		{
			for( j = i - 1; j >= 0; j-- )
			{
				if( pWndArr[j].m_nGroup != pWndArr[i].m_nGroup )
				{
					hwndRet = CShareData::getInstance()->GetTopEditWnd( pWndArr[j].m_hWnd );
					break;
				}
			}
			if( j < 0 )
			{
				for( j = n - 1; j > i; j-- )
				{
					if( pWndArr[j].m_nGroup != pWndArr[i].m_nGroup )
					{
						hwndRet = CShareData::getInstance()->GetTopEditWnd( pWndArr[j].m_hWnd );
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
	if( m_pShareData->m_Common.m_bDispTabWnd )
	{
		int nIndex = FindTabIndexByHWND( m_hwndParent );
		if( -1 != nIndex )
		{
			int nCount = TabCtrl_GetItemCount( m_hwndTab );
			if( nCount - 1 > nIndex )
			{
				ReorderTab( nIndex, nIndex + 1 );
			}
		}
	}
}

/** タブを左に移動する
	@date 2007.06.20 ryoji 新規作成
*/
void CTabWnd::MoveLeft( void )
{
	if( m_pShareData->m_Common.m_bDispTabWnd )
	{
		int nIndex = FindTabIndexByHWND( m_hwndParent );
		if( -1 != nIndex )
		{
			if( 0 < nIndex )
			{
				ReorderTab( nIndex, nIndex - 1 );
			}
		}
	}
}

/** 新規グループを作成する（現在のグループから分離）
	@date 2007.06.20 ryoji 新規作成
*/
void CTabWnd::Separate( void )
{
	if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		RECT rc;
		POINT ptSrc;
		POINT ptDst;
		RECT rcWork;
		int cy;

		::GetWindowRect( m_hwndParent, &rc );
		ptSrc.x = rc.left;
		ptSrc.y = rc.top;
		cy = ::GetSystemMetrics( SM_CYCAPTION );
		rc.left += cy;
		rc.right += cy;
		rc.top += cy;
		rc.bottom += cy;
		GetMonitorWorkRect( m_hwndParent, &rcWork );
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

		SeparateGroup( m_hwndParent, NULL, ptSrc, ptDst );
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
		SeparateGroup( m_hwndParent, hWnd, ptSrc, ptDst );
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
		SeparateGroup( m_hwndParent, hWnd, ptSrc, ptDst );
	}
}

/*[EOF]*/
