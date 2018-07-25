/*!	@file
	@brief ウィンドウの基本クラス

	@author Norio Nakatani
	@date 2000/01/11 新規作成
*/
/*
	Copyright (C) 2000-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CWnd.h"
#include "util/os.h" //WM_MOUSEWHEEL




/* CWndウィンドウメッセージのコールバック関数 */
LRESULT CALLBACK CWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CWnd* pCWnd = (CWnd*)::GetWindowLongPtr( hwnd, GWLP_USERDATA );

	if( pCWnd ){
		/* クラスオブジェクトのポインタを使ってメッセージを配送する */
		return pCWnd->DispatchEvent( hwnd, uMsg, wParam, lParam );
	}
	else{
		/* ふつうはここには来ない */
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}

//!Windowsフック(CBT)
namespace CWindowCreationHook
{
	int		g_nCnt  = 0; //参照カウンタ
	HHOOK	g_hHook = NULL;

	//!フック用コールバック
	static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if(nCode==HCBT_CREATEWND){
			HWND hwnd = (HWND)wParam;
			CBT_CREATEWND* pCreateWnd = (CBT_CREATEWND*)lParam;
			CWnd* pcWnd = static_cast<CWnd*>(pCreateWnd->lpcs->lpCreateParams);

			//CWnd以外のウィンドウ生成イベントは無視する
			WNDPROC wndproc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
			if(wndproc!=CWndProc)goto next;

			//ウィンドウにCWndを関連付ける
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pcWnd);

			//CWndにウィンドウを関連付ける
			pcWnd->_SetHwnd(hwnd);
		}
next:
		return ::CallNextHookEx(g_hHook, nCode, wParam, lParam);
	}

	//!フック開始
	void Use()
	{
		if(++g_nCnt>=1 && g_hHook==NULL){
			g_hHook = ::SetWindowsHookEx(WH_CBT, CBTProc, NULL, GetCurrentThreadId());
		}
	}

	//!フック終了
	void Unuse()
	{
		if(--g_nCnt<=0 && g_hHook!=NULL){
			::UnhookWindowsHookEx(g_hHook);
			g_hHook=NULL;
		}
	}
} //namespace CWindowCreationHook


CWnd::CWnd(const TCHAR* pszInheritanceAppend)
{
	m_hInstance = NULL;	/* アプリケーションインスタンスのハンドル */
	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */
	m_hWnd = NULL;			/* このウィンドウのハンドル */
#ifdef _DEBUG
	_tcscpy( m_szClassInheritances, _T("CWnd") );
	_tcscat( m_szClassInheritances, pszInheritanceAppend );
#endif
}

CWnd::~CWnd()
{
	if( ::IsWindow( m_hWnd ) ){
		/* クラスオブジェクトのポインタをNULLにして拡張ウィンドウメモリに格納しておく */
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( m_hWnd, GWLP_USERDATA, (LONG_PTR)NULL );
		::DestroyWindow( m_hWnd );
	}
	m_hWnd = NULL;
	return;
}


/* ウィンドウクラス作成 */
ATOM CWnd::RegisterWC(
	/* WNDCLASS用 */
	HINSTANCE	hInstance,
	HICON		hIcon,			// Handle to the class icon.
	HICON		hIconSm,		// Handle to a small icon
	HCURSOR		hCursor,		// Handle to the class cursor.
	HBRUSH		hbrBackground,	// Handle to the class background brush.
	LPCTSTR		lpszMenuName,	// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
	LPCTSTR		lpszClassName	// Pointer to a null-terminated string or is an atom.
)
{
	m_hInstance = hInstance;

	/* ウィンドウクラスの登録 */
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	//	Apr. 27, 2000 genta
	//	サイズ変更時のちらつきを抑えるためCS_HREDRAW | CS_VREDRAW を外した
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc   = CWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 32;
	wc.hInstance     = m_hInstance;
	wc.hIcon         = hIcon;
	wc.hCursor       = hCursor;
	wc.hbrBackground = hbrBackground;
	wc.lpszMenuName  = lpszMenuName;
	wc.lpszClassName = lpszClassName;
	wc.hIconSm       = hIconSm;
	return ::RegisterClassEx( &wc );
}

/* 作成 */
HWND CWnd::Create(
	/* CreateWindowEx()用 */
	HWND		hwndParent,
	DWORD		dwExStyle,		// extended window style
	LPCTSTR		lpszClassName,	// Pointer to a null-terminated string or is an atom.
	LPCTSTR		lpWindowName,	// pointer to window name
	DWORD		dwStyle,		// window style
	int			x,				// horizontal position of window
	int			y,				// vertical position of window
	int			nWidth,			// window width
	int			nHeight,		// window height
	HMENU		hMenu			// handle to menu, or child-window identifier
)
{
	m_hwndParent = hwndParent;

	/* ウィンドウ作成前の処理(クラス登録前) ( virtual )*/
	PreviCreateWindow();

	/* 初期ウィンドウサイズ */
	/* ウィンドウの作成 */

	//Windowsフックにより、ウィンドウが作成されるタイミングを横取りする 2007.10.01 kobake
	CWindowCreationHook::Use();

	m_hWnd = ::CreateWindowEx(
		dwExStyle, // extended window style
		lpszClassName, // pointer to registered class name
		lpWindowName, // pointer to window name
		dwStyle, // window style
		x, // horizontal position of window
		y, // vertical position of window
		nWidth, // window width
		nHeight, // window height
		m_hwndParent, // handle to parent or owner window
		hMenu, // handle to menu, or child-window identifier
		m_hInstance, // handle to application instance
		(LPVOID)this	// pointer to window-creation data
	);

	//Windowsフック解除
	CWindowCreationHook::Unuse();

	if( NULL == m_hWnd ){
		::MessageBox( m_hwndParent, _T("CWnd::Create()\n\n::CreateWindowEx failed."), _T("error"), MB_OK );
		return NULL;
	}

	/* ウィンドウ作成後の処理 */
	AfterCreateWindow();
	return m_hWnd;
}


/* メッセージ配送 */
LRESULT CWnd::DispatchEvent( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	#define CALLH(message, method) case message: return method( hwnd, msg, wp, lp )
	switch( msg ){
	CALLH( WM_CREATE			, OnCreate			);
	CALLH( WM_CLOSE				, OnClose			);
	CALLH( WM_DESTROY			, OnDestroy			);
	CALLH( WM_SIZE				, OnSize			);
	CALLH( WM_MOVE				, OnMove			);
	CALLH( WM_COMMAND			, OnCommand			);
	CALLH( WM_LBUTTONDOWN		, OnLButtonDown		);
	CALLH( WM_LBUTTONUP			, OnLButtonUp		);
	CALLH( WM_LBUTTONDBLCLK		, OnLButtonDblClk	);
	CALLH( WM_RBUTTONDOWN		, OnRButtonDown		);
	CALLH( WM_RBUTTONUP			, OnRButtonUp		);
	CALLH( WM_RBUTTONDBLCLK		, OnRButtonDblClk	);
	CALLH( WM_MBUTTONDOWN		, OnMButtonDown		);
	CALLH( WM_MBUTTONUP			, OnMButtonUp		);
	CALLH( WM_MBUTTONDBLCLK		, OnMButtonDblClk	);
	CALLH( WM_MOUSEMOVE			, OnMouseMove		);
	CALLH( WM_MOUSEWHEEL		, OnMouseWheel		);
	CALLH( WM_MOUSEHWHEEL		, OnMouseHWheel		);
	CALLH( WM_PAINT				, OnPaint			);
	CALLH( WM_TIMER				, OnTimer			);
	CALLH( WM_QUERYENDSESSION	, OnQueryEndSession	);

	CALLH( WM_MEASUREITEM		, OnMeasureItem		);
	CALLH( WM_MENUCHAR			, OnMenuChar		);
	CALLH( WM_NOTIFY			, OnNotify			);	//@@@ 2003.05.31 MIK
	CALLH( WM_DRAWITEM			, OnDrawItem		);	// 2006.02.01 ryoji
	CALLH( WM_CAPTURECHANGED	, OnCaptureChanged	);	// 2006.11.30 ryoji

	default:
		if( WM_APP <= msg && msg <= 0xBFFF ){
			/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */
			return DispatchEvent_WM_APP( hwnd, msg, wp, lp );
		}
		break;	/* default */
	}
	return CallDefWndProc( hwnd, msg, wp, lp );
}

/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */
LRESULT CWnd::DispatchEvent_WM_APP( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	return CallDefWndProc( hwnd, msg, wp, lp );
}

/* デフォルトメッセージ処理 */
LRESULT CWnd::CallDefWndProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	return ::DefWindowProc( hwnd, msg, wp, lp );
}


/* ウィンドウを破棄 */
void CWnd::DestroyWindow()
{
	if(m_hWnd){
		::DestroyWindow( m_hWnd );
		m_hWnd = NULL;
	}
}



