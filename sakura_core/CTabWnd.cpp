//	$Id$
/*!	@file
	@brief タブウィンドウ

	@author MIK
	@date 2004.01.27 break漏れ対応。TCHAR化。タブ表示が崩れる(?)の対応。
	$Revision$
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2004, Moca, MIK

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
	int		i;
	RECT	rc;
	int		x, y;
	int		nCount;
	TCITEM	tcitem;
	int		nId;

	switch( uMsg )
	{
	case WM_RBUTTONUP:
		x = LOWORD( lParam );
		y = HIWORD( lParam );
		
		nCount = TabCtrl_GetItemCount( m_hwndTab );
		for( i = 0; i < nCount; i++ )
		{
			if( TabCtrl_GetItemRect( m_hwndTab, i, &rc ) )
			{
				if( rc.left <= x && x <= rc.right
				 && rc.top  <= y && y <= rc.bottom )
				{
					tcitem.mask   = TCIF_PARAM;
					tcitem.lParam = (LPARAM)NULL;
					if( TabCtrl_GetItem( m_hwndTab, i, &tcitem ) )
					{
						switch( CUSTMENU_INDEX_FOR_TABWND ){
						case 0:  nId = F_MENU_RBUTTON; break;	//break漏れ
						default: nId = F_CUSTMENU_BASE + CUSTMENU_INDEX_FOR_TABWND; break;	//break漏れ
						}
	
						//対象ウインドウをアクティブにする。
						ShowHideWindow( (HWND)tcitem.lParam, TRUE );

						//コマンドを対象ウインドウに送る。
						::SendMessage( (HWND)tcitem.lParam, WM_COMMAND, MAKELONG( nId, 0 ), (LPARAM)NULL );
						return 0L;
					}

					break;	//打ち切り
				}
			}
		}
		break;	//break漏れ
		//return 0L;
	//^^^ WM_RBUTTONUP

	case WM_NOTIFY:
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
						FileInfo*	pfi;
						::SendMessage( (HWND)tcitem.lParam, MYWM_GETFILEINFO, 0, 0 );
						pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

						if( pfi->m_szPath[0] )
						{
							_tcsncpy( m_szTextTip1, pfi->m_szPath, sizeof( m_szTextTip1 ) / sizeof( TCHAR ) );
							m_szTextTip1[ (sizeof( m_szTextTip1 ) / sizeof( TCHAR )) - 1 ] = _T('\0');
						}
						else if( pfi->m_bIsGrep )
						{
							_tcsncpy( m_szTextTip1, pfi->m_szGrepKey, sizeof( m_szTextTip1 ) / sizeof( TCHAR ) );
							m_szTextTip1[ (sizeof( m_szTextTip1 ) / sizeof( TCHAR )) - 1 ] = _T('\0');
						}
						else if( pfi->m_bIsDebug )
						{
							_tcscpy( m_szTextTip1, _T("(アウトプットウインドウ)") );
						}
						else
						{
							_tcscpy( m_szTextTip1, _T("(無題)") );
						}

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
		break;	//break漏れ
		//return 0L;
	//^^^ WM_NOTIFY

	//default:
	}

	return 1L;	//デフォルトのディスパッチにまわす
}

CTabWnd::CTabWnd()
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

	/* ウィンドウクラス作成 */
	RegisterWC(
		/* WNDCLASS用 */
		NULL,								// Handle to the class icon.
		NULL,								//Handle to a small icon
		::LoadCursor( NULL, IDC_ARROW ),	// Handle to the class cursor.
		(HBRUSH)(COLOR_3DFACE + 1),			// Handle to the class background brush.
		NULL,								// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		m_pszClassName						// Pointer to a null-terminated string or is an atom.
	);

	/* 基底クラスメンバ呼び出し */
	CWnd::Create(
		/* CreateWindowEx()用 */
		0,									// extended window style
		m_pszClassName,						// Pointer to a null-terminated string or is an atom.
		m_pszClassName,						// pointer to window name
		WS_CHILD | WS_VISIBLE,				// window style
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		100,								// window width
		::GetSystemMetrics( SM_CYMENU ),	// window height
		NULL								// handle to menu, or child-window identifier
	);

	//タブウインドウを作成する。
	m_hwndTab = ::CreateWindow(
		WC_TABCONTROL,
		_T(""),
		WS_CHILD | WS_VISIBLE,
		CW_USEDEFAULT,
		0,
		100,
		::GetSystemMetrics( SM_CYMENU ),
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
		//lngStyle &= ~(TCS_BUTTONS | TCS_MULTILINE);
		//lngStyle |= TCS_TABS | TCS_SINGLELINE;
		lngStyle &= ~(TCS_BUTTONS | TCS_SINGLELINE);	//2004.01.31
		lngStyle |= TCS_TABS | TCS_MULTILINE;
		::SetWindowLongPtr( m_hwndTab, GWL_STYLE, lngStyle );

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
		m_hwndToolTip = ::CreateWindowEx(
			WS_EX_TOPMOST,
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
		::SetWindowPos(
			m_hwndToolTip,
			HWND_TOPMOST,
			0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

		RECT		rect;
		TOOLINFO	ti;
		::GetClientRect( m_hwndTab, &rect );
		ti.cbSize      = sizeof( TOOLINFO );
		ti.uFlags      = TTF_SUBCLASS;
		ti.hwnd        = m_hWnd; //m_hwndTab;
		ti.hinst       = m_hInstance;
		ti.uId         = 0;
		ti.lpszText    = LPSTR_TEXTCALLBACK;
		ti.rect.left   = 0; //rect.left;
		ti.rect.top    = 0; //rect.top;
		ti.rect.right  = 0; //rect.right;
		ti.rect.bottom = 0; //rect.bottom;

		::SendMessage( m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti );
		TabCtrl_SetToolTips( m_hwndTab, m_hwndToolTip ); 

		//TabCtrl_DeleteAllItems( m_hwndTab );
		//::ShowWindow( m_hwndTab, SW_HIDE );
		Refresh();
	}

	::ShowWindow( m_hWnd, SW_SHOW );
	::InvalidateRect( m_hWnd, NULL, TRUE );

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

	::MoveWindow( m_hwndTab, 1, 1, rcParent.right - rcParent.left - 2, rcParent.bottom - rcParent.top - 2, TRUE );

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

	m_hWnd = NULL;

	return 0L;

}

//WM_NOTIFY処理
LRESULT CTabWnd::OnNotify( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_hwndTab ) return 0L;

	LPNMHDR	lpnmhdr;
	int		nIndex;
	TCITEM	tcitem;

	lpnmhdr = (LPNMHDR) lParam;
	if( lpnmhdr->hwndFrom == m_hwndTab )
	{
		switch( lpnmhdr->code )
		{
		case TCN_SELCHANGE:
			nIndex = TabCtrl_GetCurSel( m_hwndTab );
			if( -1 != nIndex )
			{
				tcitem.mask   = TCIF_PARAM;
				tcitem.lParam = (LPARAM)0;
				TabCtrl_GetItem( m_hwndTab, nIndex, &tcitem );

				//指定のウインドウをアクティブに
				ShowHideWindow( (HWND)tcitem.lParam, TRUE );
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

	nCount = TabCtrl_GetItemCount( m_hwndTab );
	if( nCount <= 0 )
	{
		bFlag = true;
		//最初のときはすでに存在するウインドウの情報も登録する必要がある。
		if( wParam == TWNT_ADD ) wParam = TWNT_REFRESH;
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
			TabCtrl_InsertItem( m_hwndTab, nCount, &tcitem );
			nIndex = nCount;
		}

		//TabCtrl_SetCurSel( m_hwndTab, nIndex );

		if( (HWND)lParam == m_hwndParent )
		{
			//自分ならアクティブに
			ShowHideWindow( (HWND)lParam, TRUE );
			//ここに来たということはすでにアクティブ
			//コマンド実行時のアウトプットで問題があるのでアクティブにする
		}
		else
		{
			//自分に用がなければ隠す。
			ShowHideWindow( m_hwndParent, FALSE );
		}

		TabCtrl_SetCurSel( m_hwndTab, nIndex );
		break;

	case TWNT_DEL:	//ウインドウ削除
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 != nIndex )
		{
			TabCtrl_DeleteItem( m_hwndTab, nIndex );

			//次のウインドウが自分ならアクティブに
			nIndex = GetFirstOpenedWindow();
			if( -1 != nIndex )
			{
				if( m_pShareData->m_pEditArr[ nIndex ].m_hWnd == m_hwndParent )
				{
					if( TRUE  == m_pShareData->m_Common.m_bDispTabWnd
					 //&& FALSE == m_pShareData->m_Common.m_bDispTabWndMultiWin
					 && FALSE == ::IsWindowVisible( m_hwndParent ) )
					{
						ShowHideWindow( m_hwndParent, TRUE );
						ForceActiveWindow( m_hwndParent );
					}
				}
			}
		}
		break;

	case TWNT_ORDER:	//ウインドウ順序変更
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 != nIndex )
		{
			//TabCtrl_SetCurSel( m_hwndTab, nIndex );

			if( (HWND)lParam == m_hwndParent )
			{
				//自分ならアクティブに
				if( FALSE == ::IsWindowVisible( (HWND)lParam ) )
				{
					ShowHideWindow( (HWND)lParam, TRUE );
				}
				//ここに来たということはすでにアクティブ
			}
			else
			{
				//自分に用がなければ隠す。
				ShowHideWindow( m_hwndParent, FALSE );
			}

			TabCtrl_SetCurSel( m_hwndTab, nIndex );
		}
		else
		{
			//指定のウインドウがないので再表示
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
			EditNode	*p;

			cRecentEditNode.EasyCreate( RECENT_FOR_EDITNODE );
			p = (EditNode*)cRecentEditNode.GetItem( cRecentEditNode.FindItem( (const char*)&lParam ) );
			if( p && p->m_szTabCaption[0] )
			{
				_tcsncpy( szName, p->m_szTabCaption, (sizeof( szName ) / sizeof( TCHAR )) );
				szName[ (sizeof( szName ) / sizeof( TCHAR )) - 1 ] = _T('\0');
			}
			else
			{
				_tcscpy( szName, _T("(無題)") );
			}
			cRecentEditNode.Terminate();

			tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
			tcitem.pszText = szName;
			tcitem.lParam  = (LPARAM)lParam;
			TabCtrl_SetItem( m_hwndTab, nIndex, &tcitem );
		}
		else
		{
			//指定のウインドウがないので再表示
			Refresh();
		}
		break;

	case TWNT_REFRESH:	//再表示
		Refresh();
		break;

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

	//更新
	::InvalidateRect( m_hwndTab, NULL, TRUE );

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

/*! タブリストを再表示する */
void CTabWnd::Refresh( void )
{
	int			i;
	int			nIndex;
	TCITEM		tcitem;
	TCHAR		szName[1024];
	EditNode	*p;
	int			nCount;

	if( NULL == m_hwndTab ) return;

	TabCtrl_DeleteAllItems( m_hwndTab );

	p = NULL;
	nCount = CShareData::getInstance()->GetOpenedWindowArr( &p, TRUE );

	nIndex = -1;
	for( i = 0; i < nCount; i++ )
	{
		if( m_hwndParent == p[ i ].m_hWnd ) nIndex = i;

		if( p[ i ].m_szTabCaption[0] )
		{
			_tcsncpy( szName, p[ i ].m_szTabCaption, (sizeof( szName ) / sizeof( TCHAR )) );
			szName[ (sizeof( szName ) / sizeof( TCHAR )) - 1 ] = _T('\0');
		}
		else
		{
			_tcscpy( szName, _T("(無題)") );
		}

		tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
		tcitem.pszText = szName;
		tcitem.lParam  = (LPARAM)p[ i ].m_hWnd;
		TabCtrl_InsertItem( m_hwndTab, i, &tcitem );
	}

	if( p ) delete [] p;

	if( -1 != nIndex )
	{
		TabCtrl_SetCurSel( m_hwndTab, nIndex );
	}

	return;
}

void CTabWnd::ShowHideWindow( HWND hwnd, BOOL bDisp )
{
	if( NULL == hwnd ) return;

	if( bDisp )
	{
		if( m_pShareData->m_Common.m_bDispTabWndMultiWin == FALSE )
		{
			//ウインドウ情報を引き継ぐ。
			m_pShareData->m_TabWndWndpl.length = sizeof( m_pShareData->m_TabWndWndpl );
			::SetWindowPlacement( hwnd, &(m_pShareData->m_TabWndWndpl) );
		}

		TabWnd_ActivateFrameWindow( hwnd );
	}
	else
	{
		if( m_pShareData->m_Common.m_bDispTabWnd )
		{
			if( m_pShareData->m_Common.m_bDispTabWndMultiWin == FALSE )
			{
				::ShowWindow( hwnd, SW_HIDE );
			}
		}
	}

	return;
}

int CTabWnd::GetFirstOpenedWindow( void )
{
	int	i;

	for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
	{
		if( CShareData::getInstance()->IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) )
		{
			return i;
		}
	}

	return -1;
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

/* アクティブにする */
void CTabWnd::TabWnd_ActivateFrameWindow( HWND hwnd )
{
	if( ::IsIconic( hwnd ) )
	{
		::ShowWindow( hwnd, SW_RESTORE );	// Nov. 7. 2003 MIK アイコン時は元のサイズに戻す
		return;
	}
	else if( ::IsZoomed( hwnd ) )
	{
		::ShowWindow( hwnd, SW_MAXIMIZE );
	}
	else
	{
		::ShowWindow( hwnd, SW_SHOW );
	}

	::SetForegroundWindow( hwnd );
	::BringWindowToTop( hwnd );

	return;
}

/*[EOF]*/
