//	$Id$
/*!	@file
	@brief ファンクションキーウィンドウ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CFuncKeyWnd.h"
#include "debug.h"
#include "CEditWnd.h"
#include "global.h"
#include "KeyCode.h"// 2002/2/10 aroka
#include "etc_uty.h" // novice 2004/10/10

#define IDT_FUNCWND 1248
#define TIMER_TIMEOUT 100
#define TIMER_CHECKFUNCENABLE 300

/****
LRESULT CALLBACK CFuncKeyWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	CFuncKeyWnd*	pCFuncKeyWnd;
	pCFuncKeyWnd = ( CFuncKeyWnd* )::GetWindowLongPtr( hwnd, GWLP_USERDATA );
	if( NULL != pCFuncKeyWnd ){
		return pCFuncKeyWnd->DispatchEvent( hwnd, uMsg, wParam, lParam );
	}
	return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
}
***/






//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CFuncKeyWnd::CFuncKeyWnd()
{
	strcat( m_szClassInheritances, "::CFuncKeyWnd" );

	int		i;
	LOGFONT	lf;
//	m_hInstance = NULL;
//	m_hwndParent = NULL;
//	m_hWnd = NULL;
//	m_pszAppName = "CFuncKeyWnd";
	m_pszClassName = "CFuncKeyWnd";
	m_pCEditDoc = NULL;
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();
	m_nCurrentKeyState = -1;
	for( i = 0; i < sizeof(	m_szFuncNameArr ) / sizeof(	m_szFuncNameArr[0] ); ++i ){
		strcpy( m_szFuncNameArr[i], "" );
	}
//	2002.11.04 Moca Open()側で設定
//	m_nButtonGroupNum = 4;

	for( i = 0; i < sizeof( m_hwndButtonArr ) / sizeof( m_hwndButtonArr[0] ); ++i ){
		m_hwndButtonArr[i] = NULL;
	}

	/* 表示用フォント */
	/* LOGFONTの初期化 */
	memset( &lf, 0, sizeof(LOGFONT) );
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
	strcpy( lf.lfFaceName, "ＭＳ Ｐゴシック" );
	m_hFont = ::CreateFontIndirect( &lf );

	m_bSizeBox = FALSE;
	m_hwndSizeBox = NULL;
	m_nTimerCount = 0;

	return;
}




CFuncKeyWnd::~CFuncKeyWnd()
{
	/* 表示用フォント */
	::DeleteObject( m_hFont );
	return;
}




/* ウィンドウ オープン */
HWND CFuncKeyWnd::Open( HINSTANCE hInstance, HWND hwndParent, CEditDoc* pCEditDoc, BOOL bSizeBox )
{
//	WNDCLASS	wc;
//	HWND		hWnd;
//	ATOM		atom;

	/* 初期化 */
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;

//	m_hInstance = hInstance;
//	m_hwndParent = hwndParent;

	m_pCEditDoc = pCEditDoc;
	m_bSizeBox = bSizeBox;
	m_hwndSizeBox = NULL;
	m_nCurrentKeyState = -1;

	// 2002.11.04 Moca 変更できるように
	m_nButtonGroupNum = m_pShareData->m_Common.m_nFUNCKEYWND_GroupNum;
	if( 1 > m_nButtonGroupNum || 12 < m_nButtonGroupNum ){
		m_nButtonGroupNum = 4;
	}

/*
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= CFuncKeyWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 32;
	wc.hInstance		= m_hInstance;
	wc.hIcon			= NULL;
	wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground	= (HBRUSH)(COLOR_3DFACE + 1);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= m_pszAppName;
	atom = RegisterClass( &wc );
*/
	/* ウィンドウクラス作成 */
	RegisterWC(
		/* WNDCLASS用 */
		NULL,// Handle to the class icon.
		NULL,	//Handle to a small icon
		::LoadCursor( NULL, IDC_ARROW ),// Handle to the class cursor.
		(HBRUSH)(COLOR_3DFACE + 1),// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		m_pszClassName// Pointer to a null-terminated string or is an atom.
	);

/**
	hWnd = CreateWindow(
		m_pszAppName,		// pointer to registered class name
		m_pszAppName,		// pointer to window name
		WS_VISIBLE |
		0
		| WS_VISIBLE
		| WS_CHILD
		,	// window style

		CW_USEDEFAULT,		// horizontal position of window
		0,					// vertical position of window
		100,				// window width
		::GetSystemMetrics( SM_CYMENU ),			// window height
		m_hwndParent,		// handle to parent or owner window
		NULL,				// handle to menu or child-window identifier
		m_hInstance,		// handle to application instance
		NULL				// pointer to window-creation data
	);
	m_hWnd = hWnd;
**/

	/* 基底クラスメンバ呼び出し */
	CWnd::Create(
		/* CreateWindowEx()用 */
		0, // extended window style
		m_pszClassName,	// Pointer to a null-terminated string or is an atom.
		m_pszClassName, // pointer to window name
		WS_CHILD | WS_VISIBLE, // window style
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		100, // window width
		::GetSystemMetrics( SM_CYMENU ), // window height
		NULL // handle to menu, or child-window identifier
	);


	m_hwndSizeBox = NULL;
	if( m_bSizeBox ){
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 						/* no extended styles			*/
			"SCROLLBAR",				/* scroll bar control class		*/
			(LPSTR) NULL,				/* text for window title bar	*/
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,							/* horizontal position			*/
			0,							/* vertical position			*/
			200,						/* width of the scroll bar		*/
			CW_USEDEFAULT,				/* default height				*/
			m_hWnd, 					/* handle of main window		*/
			(HMENU) NULL,				/* no menu for a scroll bar 	*/
			m_hInstance,				/* instance owning this window	*/
			(LPVOID) NULL				/* pointer not needed			*/
		);
		::ShowWindow( m_hwndSizeBox, SW_SHOW );
	}

	/* ボタンの生成 */
	CreateButtons();

//	if( NULL != m_hWnd ){
//		::SetWindowLong( m_hWnd, GWL_USERDATA, (LONG)this );
//	}
	::ShowWindow( m_hWnd, SW_SHOW );
	if( NULL != m_hWnd ){
		/* タイマーを起動 */
//		if( 0 == ::SetTimer( m_hWnd, IDT_FUNCWND, TIMER_TIMEOUT, (TIMERPROC)CFuncKeyWndTimerProc ) ){
		if( 0 == ::SetTimer( m_hWnd, IDT_FUNCWND, TIMER_TIMEOUT, NULL ) ){
			::MYMESSAGEBOX(	m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
				"CFuncKeyWnd::Open()\nタイマーが起動できません。\nシステムリソースが不足しているのかもしれません。"
			);
		}
	}
	::InvalidateRect( m_hWnd, NULL, TRUE );

	return m_hWnd;
}




/* ウィンドウ クローズ */
void CFuncKeyWnd::Close( void )
{
	::DestroyWindow( m_hWnd );
	m_hWnd = NULL;
}





///* WM_SIZE処理 */
//void CFuncKeyWnd::OnSize(
//	WPARAM	wParam,	// first message parameter
//	LPARAM	lParam 	// second message parameter

// WM_SIZE処理
LRESULT CFuncKeyWnd::OnSize( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int			nButtonWidth;
	int			nButtonHeight;
	int			i;
	int			nX;
//	RECT		rc;
	int			nButtonNum;
	RECT		rcParent;

	if( NULL == m_hWnd ){
		return 0L;
	}

//	/* メインウィンドウが最大化されている場合はサイズボックスを表示しない */
//	BOOL			bSizeBox;
//	WINDOWPLACEMENT	wp;
//	wp.length = sizeof( WINDOWPLACEMENT );
//	::GetWindowPlacement( m_hwndParent, &wp );
//	if( SW_SHOWMAXIMIZED == wp.showCmd ){
//		bSizeBox = FALSE;
//	}else{
//		bSizeBox = TRUE;
//	}
//	/* サイズボックスの表示／非表示切り替え */
//	SizeBox_ONOFF( bSizeBox );


	nButtonNum = sizeof( m_hwndButtonArr ) / sizeof( m_hwndButtonArr[0] );

	/* ボタンのサイズを計算 */
	nButtonWidth = CalcButtonSize();

	::GetWindowRect( m_hWnd, &rcParent );
	nButtonHeight = rcParent.bottom - rcParent.top - 2;

	nX = 1;
	for( i = 0; i < nButtonNum; ++i ){
		if( 0 < i  && 0 == ( i % m_nButtonGroupNum ) ){
			nX += 12;
		}
		::MoveWindow( m_hwndButtonArr[i], nX, 1, nButtonWidth, nButtonHeight, TRUE );
		nX += nButtonWidth + 1;
	}
	::InvalidateRect( m_hWnd, NULL, TRUE );	//再描画してね。	//@@@ 2003.06.11 MIK
	return 0L;
}


#if 0//////////////////////////////////////////////////////////////
LRESULT CFuncKeyWnd::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
//	if( NULL == m_hWnd ){
//		return 0L;
//	}

	int		i;
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl;
	switch ( uMsg ){

	case WM_TIMER:		return OnTimer( hwnd, uMsg, wParam, lParam );
	case WM_COMMAND:	return OnCommand( hwnd, uMsg, wParam, lParam );
	case WM_SIZE:		return OnSize( hwnd, uMsg, wParam, lParam );
	case WM_DESTROY:	return OnDestroy( hwnd, uMsg, wParam, lParam );

	default:
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}
#endif//////////////////////////////////////////////////////////////



LRESULT CFuncKeyWnd::OnCommand( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	int		i;
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl;

	wNotifyCode = HIWORD(wParam);	// notification code
	wID = LOWORD(wParam);			// item, control, or accelerator identifier
	hwndCtl = (HWND) lParam;		// handle of control
//	switch( wNotifyCode ){
//	case BN_PUSHED:
		for( i = 0; i < sizeof( m_hwndButtonArr ) / sizeof( m_hwndButtonArr[0] ); ++i ){
			if( hwndCtl == m_hwndButtonArr[i] ){
				if( 0 != m_nFuncCodeArr[i] ){
					::SendMessage( m_hwndParent, WM_COMMAND, MAKELONG( m_nFuncCodeArr[i], 0 ),  (LPARAM)hwnd );
				}
				break;
			}
		}
		::SetFocus( m_hwndParent );
//		break;
//	}
	return 0L;
}


// WM_TIMERタイマーの処理
LRESULT CFuncKeyWnd::OnTimer( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
// 	HWND hwnd,	// handle of window for timer messages
//	UINT uMsg,	// WM_TIMER message
//	UINT idEvent,	// timer identifier
//	DWORD dwTime 	// current system time


	//	return;
	if( NULL == m_hWnd ){
		return 0;
	}

	if( GetActiveWindow() != m_hwndParent ) {	//	2002/06/02 MIK
		return 0;
	}

	int			nIdx;
//	int			nFuncId;
	int			i;
	int			nFuncCode;
	int			nOffF1;

// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* ALT,Shift,Ctrlキーの状態が変化したか */
	if( nIdx != m_nCurrentKeyState ){
		m_nTimerCount = TIMER_CHECKFUNCENABLE + 1;

		/* [F1]キーの位置を捜す */
		for( i = 0; i < m_pShareData->m_nKeyNameArrNum; ++i ){
			if( VK_F1 == m_pShareData->m_pKeyNameArr[i].m_nKeyCode ){
				break;
			}
		}
		if( i >= m_pShareData->m_nKeyNameArrNum ){
			m_nCurrentKeyState = nIdx;
			return 0;
		}
		nOffF1 = i;
		/* ファンクションキーの機能名を取得 */
		for( i = 0; i < sizeof( m_szFuncNameArr ) / sizeof( m_szFuncNameArr[0] ); ++i ){
			nFuncCode = m_pShareData->m_pKeyNameArr[nOffF1 + i].m_nFuncCodeArr[nIdx];
			if( nFuncCode != m_nFuncCodeArr[i] ){
				m_nFuncCodeArr[i] = nFuncCode;
				if( 0 == m_nFuncCodeArr[i] ){
					strcpy( m_szFuncNameArr[i], "" );
				}else{
					//	Oct. 2, 2001 genta
					m_pCEditDoc->m_cFuncLookup.Funccode2Name( m_nFuncCodeArr[i],
						m_szFuncNameArr[i], sizeof(m_szFuncNameArr[i]) - 1 );
//					::LoadString( m_hInstance, m_nFuncCodeArr[i], m_szFuncNameArr[i], sizeof(m_szFuncNameArr[i]) - 1 );
				}
				::SetWindowText( m_hwndButtonArr[i], m_szFuncNameArr[i] );
			}
		}
	}
	m_nTimerCount += TIMER_TIMEOUT;
	if( m_nTimerCount > TIMER_CHECKFUNCENABLE ||
		nIdx != m_nCurrentKeyState
	){
		m_nTimerCount = 0;
		/* 機能が利用可能か調べる */
		for( i = 0; i < sizeof(	m_szFuncNameArr ) / sizeof(	m_szFuncNameArr[0] ); ++i ){
			if( CEditWnd::IsFuncEnable( (CEditDoc*)m_pCEditDoc, m_pShareData, m_nFuncCodeArr[i]  ) ){
				::EnableWindow( m_hwndButtonArr[i], TRUE );
			}else{
				::EnableWindow( m_hwndButtonArr[i], FALSE );
			}
		}
	}
//	MYTRACE( "\n" );
	m_nCurrentKeyState = nIdx;
	return 0;
}


// WM_DSESTROY処理
LRESULT CFuncKeyWnd::OnDestroy( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int i;

	/* タイマーを削除 */
	::KillTimer( hwnd, IDT_FUNCWND );

	/* ボタンを削除 */
	for( i = 0; i < sizeof( m_hwndButtonArr ) / sizeof( m_hwndButtonArr[0] ); ++i ){
		if( NULL != m_hwndButtonArr[i] ){
			::DestroyWindow( m_hwndButtonArr[i]	);
			m_hwndButtonArr[i] = NULL;
		}
	}

	/* サイズボックスを削除 */
	if( NULL != m_hwndSizeBox ){
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
	}

	m_hWnd = NULL;
	return 0L;

}



/*! ボタンのサイズを計算 */
int CFuncKeyWnd::CalcButtonSize( void )
{
	int			nButtonNum;
	RECT		rc;
	int			nCxHScroll;
	int			nCyHScroll;
	int			nCxVScroll;
	int			nCyVScroll;
	::GetWindowRect( m_hWnd, &rc );

	nButtonNum = sizeof( m_hwndButtonArr ) / sizeof( m_hwndButtonArr[0] );

	if( NULL == m_hwndSizeBox ){
//		return ( rc.right - rc.left - nButtonNum - ( (nButtonNum + m_nButtonGroupNum - 1) / m_nButtonGroupNum - 1 ) * 12 ) / nButtonNum;
		nCxVScroll = 0;
	}else{
		/* サイズボックスの位置、サイズ変更 */
		nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
		nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );
		::MoveWindow( m_hwndSizeBox,  rc.right - rc.left - nCxVScroll, rc.bottom - rc.top - nCyHScroll, nCxVScroll, nCyHScroll, TRUE );
//		::MoveWindow( m_hwndSizeBox,  0, 0, nCxVScroll, nCyHScroll, TRUE );

//		return ( rc.right - rc.left - nCxVScroll = - nButtonNum -  ( (nButtonNum + m_nButtonGroupNum - 1) / m_nButtonGroupNum - 1 ) * 12 ) / nButtonNum;
	}
	return ( rc.right - rc.left - nCxVScroll - nButtonNum -  ( (nButtonNum + m_nButtonGroupNum - 1) / m_nButtonGroupNum - 1 ) * 12 ) / nButtonNum;

}



/*! ボタンの生成 */
void CFuncKeyWnd::CreateButtons( void )
{
//	HWND	hwndButton;
	RECT	rcParent;
	int		nButtonHeight;
	int		nButtonWidth;
	int		i;
	int		nX;

	::GetWindowRect( m_hWnd, &rcParent );
	nButtonHeight = nButtonHeight = rcParent.bottom - rcParent.top - 2;

	/* ボタンのサイズを計算 */
	nButtonWidth = CalcButtonSize();

	for( i = 0; i < sizeof(	m_nFuncCodeArr ) / sizeof(	m_nFuncCodeArr[0] ); ++i ){
		m_nFuncCodeArr[i] = 0;
	}

	nX = 1;
	for( i = 0; i < sizeof( m_hwndButtonArr ) / sizeof( m_hwndButtonArr[0] ); ++i ){
		m_hwndButtonArr[i] = ::CreateWindow(
			"BUTTON",	// predefined class
			"",			// button text
			WS_VISIBLE | WS_CHILD | BS_LEFT
			,			// styles
			// Size and position values are given explicitly, because
			// the CW_USEDEFAULT constant gives zero values for buttons.
			nX,			// starting x position
			0 + 1,		// starting y position
			nButtonWidth,		// button width
			nButtonHeight,		// button height
			m_hWnd,		// parent window
			NULL,		// No menu
			// Modified by KEITA for WIN64 2003.9.6
			(HINSTANCE) GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE),
			NULL		// pointer not needed
		);
		/* フォント変更 */
		::SendMessage( m_hwndButtonArr[i], WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0) );

		nX += nButtonWidth + 1;
	}
	m_nCurrentKeyState = -1;
	return;
}




/*! サイズボックスの表示／非表示切り替え */
void CFuncKeyWnd::SizeBox_ONOFF( BOOL bSizeBox )
{

	RECT		rc;
	::GetWindowRect( m_hWnd, &rc );
	if( m_bSizeBox == bSizeBox ){
		return;
	}
	if( m_bSizeBox ){
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
		m_bSizeBox = FALSE;
		OnSize( NULL, 0, 0, 0 );
	}else{
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 						/* no extended styles			*/
			"SCROLLBAR",				/* scroll bar control class		*/
			(LPSTR) NULL,				/* text for window title bar	*/
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,							/* horizontal position			*/
			0,							/* vertical position			*/
			200,						/* width of the scroll bar		*/
			CW_USEDEFAULT,				/* default height				*/
			m_hWnd, 				/* handle of main window		*/
			(HMENU) NULL,				/* no menu for a scroll bar 	*/
			m_hInstance,				/* instance owning this window	*/
			(LPVOID) NULL			/* pointer not needed				*/
		);
		::ShowWindow( m_hwndSizeBox, SW_SHOW );
		m_bSizeBox = TRUE;
		OnSize( NULL, 0, 0, 0 );
	}
	return;
}


/*[EOF]*/
