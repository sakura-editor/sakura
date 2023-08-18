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
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "window/CWnd.h"

/*!
 * コンストラクタ
 */
CWnd::CWnd(const WCHAR* pszInheritanceAppend, std::shared_ptr<User32Dll> User32Dll_) noexcept
	: CCustomWnd(std::move(User32Dll_))
{
#ifdef _DEBUG
	wcscpy( m_szClassInheritances, L"CWnd" );
	wcscat( m_szClassInheritances, pszInheritanceAppend );
#endif
}

CWnd::~CWnd()
{
	if( ::IsWindow( m_hWnd ) ){
		::DestroyWindow( m_hWnd );
	}
}

/* ウィンドウクラス作成 */
ATOM CWnd::RegisterWC(
	HINSTANCE	hInstance,
	HICON		hIcon,			// Handle to the class icon.
	HICON		hIconSm,		// Handle to a small icon
	HCURSOR		hCursor,		// Handle to the class cursor.
	HBRUSH		hbrBackground,	// Handle to the class background brush.
	LPCWSTR		lpszMenuName,	// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
	LPCWSTR		lpszClassName	// Pointer to a null-terminated string or is an atom.
)
{
	UNREFERENCED_PARAMETER(lpszMenuName);

	m_hInstance = hInstance;

	/* ウィンドウクラスの登録 */
	const auto ret = RegisterWnd(
		lpszClassName,
		hCursor,
		hbrBackground,
		CS_DBLCLKS,
		hIcon,
		hIconSm);

	return static_cast<ATOM>(ret);
}

/* 作成 */
HWND CWnd::Create(
	HWND		hwndParent,
	DWORD		dwExStyle,		// extended window style
	LPCWSTR		lpszClassName,	// Pointer to a null-terminated string or is an atom.
	LPCWSTR		lpWindowName,	// pointer to window name
	DWORD		dwStyle,		// window style
	int			x,				// horizontal position of window
	int			y,				// vertical position of window
	int			nWidth,			// window width
	int			nHeight,		// window height
	HMENU		hMenu			// handle to menu, or child-window identifier
)
{
	UNREFERENCED_PARAMETER(lpszClassName);

	m_hwndParent = hwndParent;

	/* 初期ウィンドウサイズ */
	const RECT rcWin = { x, y, nWidth, nHeight };

	/* ウィンドウの作成 */
	m_hWnd = CreateWnd(hwndParent, lpWindowName, dwStyle, dwExStyle, static_cast<UINT>(std::bit_cast<size_t>(hMenu)), &rcWin);

	if( NULL == m_hWnd ){
		::MessageBox( m_hwndParent, L"CWnd::Create()\n\n::CreateWindowEx failed.", L"error", MB_OK );
		return NULL;
	}

	return m_hWnd;
}

/*!
 * CWndのメッセージ配送
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CWnd::DispatchEvent( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	#define CALLH(message, method) case message: return method( hwnd, msg, wp, lp )
	switch( msg ){
	CALLH( WM_DESTROY			, OnDestroy			);
	CALLH( WM_SIZE				, OnSize			);
	CALLH( WM_COMMAND			, OnCommand			);
	CALLH( WM_LBUTTONDOWN		, OnLButtonDown		);
	CALLH( WM_LBUTTONUP			, OnLButtonUp		);
	CALLH( WM_LBUTTONDBLCLK		, OnLButtonDblClk	);
	CALLH( WM_RBUTTONDOWN		, OnRButtonDown		);
	CALLH( WM_MBUTTONDOWN		, OnMButtonDown		);
	CALLH( WM_MOUSEMOVE			, OnMouseMove		);
	CALLH( WM_PAINT				, OnPaint			);
	CALLH( WM_TIMER				, OnTimer			);

	CALLH( WM_MEASUREITEM		, OnMeasureItem		);
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

	return __super::DispatchEvent( hwnd, msg, wp, lp );
}

/*!
 * WM_CREATEハンドラ
 *
 * WM_CREATEはCreateWindowEx関数によるウインドウ作成中にポストされます。
 * メッセージの戻り値はウインドウの作成を続行するかどうかの判断に使われます。
 *
 * @retval true  ウィンドウの作成を続行する
 * @retval false ウィンドウの作成を中止する
 */
bool CWnd::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	if (!__super::OnCreate(hWnd, lpCreateStruct))
	{
		return false;
	}

	m_hInstance  = lpCreateStruct->hInstance;
	m_hwndParent = lpCreateStruct->hwndParent;

	return true;
}

/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */
LRESULT CWnd::DispatchEvent_WM_APP( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	return CallDefWndProc( hwnd, msg, wp, lp );
}

/* デフォルトメッセージ処理 */
LRESULT CWnd::CallDefWndProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	return DefWindowProcW( hwnd, msg, wp, lp );
}

/* ウィンドウを破棄 */
void CWnd::DestroyWindow()
{
	if(m_hWnd){
		::DestroyWindow( m_hWnd );
	}
}
