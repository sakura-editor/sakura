/*!	@file
	@brief ファンクションキーウィンドウ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, MIK, Moca
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2004, novice
	Copyright (C) 2006, aroka, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "_main/global.h"
#include "func/CFuncKeyWnd.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "window/CEditWnd.h"
#include "doc/CEditDoc.h"
#include "util/input.h"
#include "util/window.h"

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
: CWnd(_T("::CFuncKeyWnd"))
{
	int		i;
	LOGFONT	lf;
	m_pcEditDoc = NULL;
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData();
	m_nCurrentKeyState = -1;
	for( i = 0; i < _countof(m_szFuncNameArr); ++i ){
		m_szFuncNameArr[i][0] = LTEXT('\0');
	}
//	2002.11.04 Moca Open()側で設定
//	m_nButtonGroupNum = 4;

	for( i = 0; i < _countof( m_hwndButtonArr ); ++i ){
		m_hwndButtonArr[i] = NULL;
	}

	/* 表示用フォント */
	/* LOGFONTの初期化 */
	memset_raw( &lf, 0, sizeof(lf) );
	lf.lfHeight			= DpiPointsToPixels(-9);	// 2009.10.01 ryoji 高DPI対応（ポイント数から算出）
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

	m_bSizeBox = false;
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
HWND CFuncKeyWnd::Open( HINSTANCE hInstance, HWND hwndParent, CEditDoc* pCEditDoc, bool bSizeBox )
{
	LPCTSTR pszClassName = _T("CFuncKeyWnd");

	m_pcEditDoc = pCEditDoc;
	m_bSizeBox = bSizeBox;
	m_hwndSizeBox = NULL;
	m_nCurrentKeyState = -1;

	// 2002.11.04 Moca 変更できるように
	m_nButtonGroupNum = m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum;
	if( 1 > m_nButtonGroupNum || 12 < m_nButtonGroupNum ){
		m_nButtonGroupNum = 4;
	}

	/* ウィンドウクラス作成 */
	RegisterWC(
		hInstance,
		NULL,// Handle to the class icon.
		NULL,	//Handle to a small icon
		::LoadCursor( NULL, IDC_ARROW ),// Handle to the class cursor.
		(HBRUSH)(COLOR_3DFACE + 1),// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		pszClassName// Pointer to a null-terminated string or is an atom.
	);

	/* 基底クラスメンバ呼び出し */
	CWnd::Create(
		hwndParent,
		0, // extended window style
		pszClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName, // pointer to window name
		WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPCHILDREN, // window style	// 2006.06.17 ryoji WS_CLIPCHILDREN 追加	// 2007.03.08 ryoji WS_VISIBLE 除去
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		0, // window width	// 2007.02.05 ryoji 100->0（半端なサイズで一瞬表示されるより見えないほうがいい）
		::GetSystemMetrics( SM_CYMENU ), // window height
		NULL // handle to menu, or child-window identifier
	);


	m_hwndSizeBox = NULL;
	if( m_bSizeBox ){
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 						/* no extended styles			*/
			_T("SCROLLBAR"),				/* scroll bar control class		*/
			NULL,						/* text for window title bar	*/
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,							/* horizontal position			*/
			0,							/* vertical position			*/
			200,						/* width of the scroll bar		*/
			CW_USEDEFAULT,				/* default height				*/
			GetHwnd(), 					/* handle of main window		*/
			(HMENU) NULL,				/* no menu for a scroll bar 	*/
			GetAppInstance(),				/* instance owning this window	*/
			(LPVOID) NULL				/* pointer not needed			*/
		);
	}

	/* ボタンの生成 */
	CreateButtons();

	Timer_ONOFF( true ); // 20060126 aroka
	OnTimer( GetHwnd(), WM_TIMER, IDT_FUNCWND, ::GetTickCount() );	// 初回更新	// 2006.12.20 ryoji

	return GetHwnd();
}




/* ウィンドウ クローズ */
void CFuncKeyWnd::Close( void )
{
	this->DestroyWindow();
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

	if( NULL == GetHwnd() ){
		return 0L;
	}

	nButtonNum = _countof( m_hwndButtonArr );

	/* ボタンのサイズを計算 */
	nButtonWidth = CalcButtonSize();

	::GetWindowRect( GetHwnd(), &rcParent );
	nButtonHeight = rcParent.bottom - rcParent.top - 2;

	nX = 1;
	for( i = 0; i < nButtonNum; ++i ){
		if( 0 < i  && 0 == ( i % m_nButtonGroupNum ) ){
			nX += 12;
		}
		::MoveWindow( m_hwndButtonArr[i], nX, 1, nButtonWidth, nButtonHeight, TRUE );
		nX += nButtonWidth + 1;
	}
	::InvalidateRect( GetHwnd(), NULL, TRUE );	//再描画してね。	//@@@ 2003.06.11 MIK
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
//	if( NULL == GetHwnd() ){
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
	HWND	hwndCtl;

	hwndCtl = (HWND) lParam;		// handle of control
//	switch( wNotifyCode ){
//	case BN_PUSHED:
		for( i = 0; i < _countof( m_hwndButtonArr ); ++i ){
			if( hwndCtl == m_hwndButtonArr[i] ){
				if( 0 != m_nFuncCodeArr[i] ){
					::SendMessageCmd( GetParentHwnd(), WM_COMMAND, MAKELONG( m_nFuncCodeArr[i], 0 ),  (LPARAM)hwnd );
				}
				break;
			}
		}
		::SetFocus( GetParentHwnd() );
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
	if( NULL == GetHwnd() ){
		return 0;
	}

	if( ::GetActiveWindow() != GetParentHwnd() && m_nCurrentKeyState != -1 ) {	//	2002/06/02 MIK	// 2006.12.20 ryoji 初回更新は処理する
		return 0;
	}

	int			nIdx;
//	int			nFuncId;
	int			i;

// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* ALT,Shift,Ctrlキーの状態が変化したか */
	if( nIdx != m_nCurrentKeyState ){
		m_nTimerCount = TIMER_CHECKFUNCENABLE + 1;

		/* ファンクションキーの機能名を取得 */
		for( i = 0; i < _countof( m_szFuncNameArr ); ++i ){
			// 2007.02.22 ryoji CKeyBind::GetFuncCode()を使う
			EFunctionCode	nFuncCode = CKeyBind::GetFuncCode(
					(WORD)(((VK_F1 + i) | ((WORD)((BYTE)(nIdx))) << 8)),
					m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
					m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
			);
			if( nFuncCode != m_nFuncCodeArr[i] ){
				m_nFuncCodeArr[i] = nFuncCode;
				if( 0 == m_nFuncCodeArr[i] ){
					m_szFuncNameArr[i][0] = LTEXT('\0');
				}else{
					//	Oct. 2, 2001 genta
					m_pcEditDoc->m_cFuncLookup.Funccode2Name(
						m_nFuncCodeArr[i],
						m_szFuncNameArr[i],
						_countof(m_szFuncNameArr[i]) - 1
					);
				}
				Wnd_SetText( m_hwndButtonArr[i], m_szFuncNameArr[i] );
			}
		}
	}
	m_nTimerCount += TIMER_TIMEOUT;
	if( m_nTimerCount > TIMER_CHECKFUNCENABLE ||
		nIdx != m_nCurrentKeyState
	){
		m_nTimerCount = 0;
		/* 機能が利用可能か調べる */
		for( i = 0; i < _countof(	m_szFuncNameArr ); ++i ){
			if( IsFuncEnable( (CEditDoc*)m_pcEditDoc, m_pShareData, m_nFuncCodeArr[i]  ) ){
				::EnableWindow( m_hwndButtonArr[i], TRUE );
			}else{
				::EnableWindow( m_hwndButtonArr[i], FALSE );
			}
		}
	}
	m_nCurrentKeyState = nIdx;
	return 0;
}


// WM_DESTROY処理
LRESULT CFuncKeyWnd::OnDestroy( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int i;

	/* タイマーを削除 */
	Timer_ONOFF( false ); // 20060126 aroka

	/* ボタンを削除 */
	for( i = 0; i < _countof( m_hwndButtonArr ); ++i ){
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

	_SetHwnd(NULL);

	return 0L;
}



/*! ボタンのサイズを計算 */
int CFuncKeyWnd::CalcButtonSize( void )
{
	int			nButtonNum;
	RECT		rc;
	int			nCyHScroll;
	int			nCxVScroll;
	::GetWindowRect( GetHwnd(), &rc );

	nButtonNum = _countof( m_hwndButtonArr );

	if( NULL == m_hwndSizeBox ){
//		return ( rc.right - rc.left - nButtonNum - ( (nButtonNum + m_nButtonGroupNum - 1) / m_nButtonGroupNum - 1 ) * 12 ) / nButtonNum;
		nCxVScroll = 0;
	}else{
		/* サイズボックスの位置、サイズ変更 */
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
		::MoveWindow( m_hwndSizeBox,  rc.right - rc.left - nCxVScroll, rc.bottom - rc.top - nCyHScroll, nCxVScroll, nCyHScroll, TRUE );
//		::MoveWindow( m_hwndSizeBox,  0, 0, nCxVScroll, nCyHScroll, TRUE );

//		return ( rc.right - rc.left - nCxVScroll = - nButtonNum -  ( (nButtonNum + m_nButtonGroupNum - 1) / m_nButtonGroupNum - 1 ) * 12 ) / nButtonNum;
	}
	return ( rc.right - rc.left - nCxVScroll - nButtonNum -  ( (nButtonNum + m_nButtonGroupNum - 1) / m_nButtonGroupNum - 1 ) * 12 ) / nButtonNum;

}



/*! ボタンの生成
	@date 2007.02.05 ryoji ボタンの水平位置・幅の設定処理を削除（OnSizeで再配置されるので不要）
*/
void CFuncKeyWnd::CreateButtons( void )
{
	RECT	rcParent;
	int		nButtonHeight;
	int		i;

	::GetWindowRect( GetHwnd(), &rcParent );
	nButtonHeight = rcParent.bottom - rcParent.top - 2;

	for( i = 0; i < _countof(	m_nFuncCodeArr ); ++i ){
		m_nFuncCodeArr[i] = F_0;
	}

	for( i = 0; i < _countof( m_hwndButtonArr ); ++i ){
		m_hwndButtonArr[i] = ::CreateWindow(
			_T("BUTTON"),						// predefined class
			_T(""),								// button text
			WS_VISIBLE | WS_CHILD | BS_LEFT,	// styles
			// Size and position values are given explicitly, because
			// the CW_USEDEFAULT constant gives zero values for buttons.
			0,					// starting x position
			0 + 1,				// starting y position
			0,					// button width
			nButtonHeight,		// button height
			GetHwnd(),				// parent window
			NULL,				// No menu
			(HINSTANCE) GetWindowLongPtr(GetHwnd(), GWLP_HINSTANCE),	// Modified by KEITA for WIN64 2003.9.6
			NULL				// pointer not needed
		);
		/* フォント変更 */
		::SendMessageAny( m_hwndButtonArr[i], WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0) );
	}
	m_nCurrentKeyState = -1;
	return;
}




/*! サイズボックスの表示／非表示切り替え */
void CFuncKeyWnd::SizeBox_ONOFF( bool bSizeBox )
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
		OnSize( NULL, 0, 0, 0 );
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
		OnSize( NULL, 0, 0, 0 );
	}
	return;
}



// タイマーの更新を開始／停止する。 20060126 aroka
// ファンクションキー表示はタイマーにより更新しているが、
// アプリのフォーカスが外れたときに親ウィンドウからON/OFFを
//	呼び出してもらうことにより、余計な負荷を停止したい。
void CFuncKeyWnd::Timer_ONOFF( bool bStart )
{
	if( NULL != GetHwnd() ){
		if( bStart ){
			/* タイマーを起動 */
			if( 0 == ::SetTimer( GetHwnd(), IDT_FUNCWND, TIMER_TIMEOUT, NULL ) ){
				WarningMessage(	GetHwnd(), LS(STR_ERR_DLGFUNCKEYWN1) );
			}
		} else {
			/* タイマーを削除 */
			::KillTimer( GetHwnd(), IDT_FUNCWND );
			m_nCurrentKeyState = -1;
		}
	}
	return;
}



