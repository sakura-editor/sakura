//	$Id$
/*!	@file
	ツールチップ
	
	@author Norio Nakatani
	@date 1998/10/30 新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "CTipWnd.h"



/* CTipWndクラス　デストラクタ */
CTipWnd::CTipWnd()
{
	strcat( m_szClassInheritances, "::CTipWnd" );
	m_pszClassName = "CTipWnd";
//	m_hInstance = NULL;	/* アプリケーションインスタンスのハンドル */
//	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */
//	m_hWnd = NULL;			/* このダイアログのハンドル */
//	m_hFont = NULL;;
//	m_hFontOld = NULL;;
	m_KeyWasHit = FALSE;	/* キーがヒットしたか */
	return;
}



/* CTipWndクラス　デストラクタ */
CTipWnd::~CTipWnd()
{
//	HDC			hdc;
//	if( NULL != m_hWnd ){
//		hdc = ::GetDC( m_hWnd );
//		if( NULL != m_hFontOld ){
//			::SelectObject( hdc, m_hFontOld );
//			m_hFontOld = NULL;
//		}
		if( NULL != m_hFont ){
			::DeleteObject( m_hFont );
			m_hFont = NULL;
		}
//		::ReleaseDC( m_hWnd, hdc );
//		::DestroyWindow( m_hWnd );
//		m_hWnd = NULL;
//	}
	return;
}



/* 初期化 */
void CTipWnd::Create( HINSTANCE hInstance, HWND hwndParent )
{

	LOGFONT		lf;
//	HDC			hdc;
//	WNDCLASS	wc;
//	ATOM		atom;

	/* 初期化 */
	m_hInstance = hInstance;	/* アプリケーションインスタンスのハンドル */
	m_hwndParent = hwndParent;	/* オーナーウィンドウのハンドル */

//	/* 初期化 */
//	Init(
//		hInstance,	// handle to application instance
//		hwndParent	 // handle to parent or owner window
//	);
	/* ウィンドウクラス作成 */
	ATOM atWork;
	atWork = RegisterWC( 
		/* WNDCLASS用 */
		NULL,// Handle to the class icon. 
		NULL,	//Handle to a small icon  
		::LoadCursor( NULL, IDC_ARROW ),// Handle to the class cursor. 
		(HBRUSH)/*NULL*/(COLOR_INFOBK + 1),// Handle to the class background brush. 
		NULL/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file. 
		m_pszClassName// Pointer to a null-terminated string or is an atom.
	);

	/* 基底クラスメンバ呼び出し */
	CWnd::Create( 
		/* CreateWindowEx()用 */
		0, // extended window style
		m_pszClassName,	// Pointer to a null-terminated string or is an atom.
		m_pszClassName, // pointer to window name
		WS_VISIBLE | WS_POPUP | WS_CLIPCHILDREN | WS_BORDER, // window style
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		CW_USEDEFAULT, // window width
		0, // window height
		NULL // handle to menu, or child-window identifier
	);
	
	if( NULL != m_hFont ){
		::DeleteObject( m_hFont );
		m_hFont = NULL;
	}
	::SystemParametersInfo(
		SPI_GETICONTITLELOGFONT,	// system parameter to query or set
		sizeof(LOGFONT),	// depends on action to be taken
		(PVOID)&lf,	// depends on action to be taken
		NULL	// user profile update flag
	);
	m_hFont = ::CreateFontIndirect( &lf );
	return;
}


/* Tipを表示 */
void CTipWnd::Show( int nX, int nY, char* szText )
{
//	LOGFONT	lf;
	HDC		hdc;
	RECT	rc;
//	HFONT	hFontOld;
//	int		nHeight;
	char*	pszInfo;


	

//	/* Tipウィンドウの作成 */
//	g_m_pcTipWnd = this;
//	m_hWnd = CreateWindowEx(
//		0
//		| WS_EX_TOPMOST
//		,	// extended window style
//		m_pszClassName,			// pointer to registered class name
//		m_pszClassName,			// pointer to window name
//		0
//		| WS_VISIBLE
//		| WS_POPUP
//		| WS_CLIPCHILDREN
//		| WS_BORDER
//		, // window style
//		CW_USEDEFAULT,			// horizontal position of window
//		0,						// vertical position of window
//		CW_USEDEFAULT,			// window width
//		0,						// window height
//		/*NULL*/m_hwndParent,				// handle to parent or owner window
//		NULL,					// handle to menu or child-window identifier
//		m_hInstance,			// handle to application instance
//		(LPVOID)this			// pointer to window-creation data
//	);
//	if( NULL == m_hWnd ){
//		return;
//	}





	if( NULL != szText ){
		m_cInfo.SetData( szText, strlen( szText ) ); 
	}
	pszInfo = m_cInfo.GetPtr( NULL );

	hdc = ::GetDC( m_hWnd );
//	hFontOld = (HFONT)::SelectObject( hdc, m_hFont );

	/* ウィンドウのサイズを決める */
	ComputeWindowSize( hdc, m_hFont, pszInfo, &rc );
	
	
	
//	
//	pszInfo = m_cInfo.GetPtr( NULL );
//	nHeight = ::DrawText( hdc, pszInfo, strlen( pszInfo ), &rc, 
//		DT_CALCRECT | DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/ );
//	::SelectObject( hdc, hFontOld );

	::ReleaseDC( m_hWnd, hdc );
	
	::MoveWindow( m_hWnd, nX, nY, rc.right + 8, rc.bottom + 8/*nHeight*/, TRUE );
	::ShowWindow( m_hWnd, SW_SHOWNA );
	return;

}

/* ウィンドウのサイズを決める */
void CTipWnd::ComputeWindowSize( 
		HDC			hdc, 
		HFONT		hFont, 
		const char*	pszText, 
		RECT*		pRect
)
{
	int		nTextLength;
	int		nCurMaxWidth;
	int		nCurHeight;
	int		nBgn;
	RECT	rc;
	HFONT	hFontOld;
	int		i;
	int		nCharChars;

	hFontOld = (HFONT)::SelectObject( hdc, hFont );
	
	nCurMaxWidth = 0;
	nCurHeight = 0;
	nTextLength = strlen( pszText );
	nBgn = 0;
	for( i = 0; i <= nTextLength; ++i ){
//		nCharChars = &pszText[i] - CMemory::MemCharPrev( pszText, nTextLength, &pszText[i] );
		nCharChars = CMemory::MemCharNext( (const char *)pszText, nTextLength, (const char *)&pszText[i] ) - (const char*)&pszText[i];
		if( ( 1 == nCharChars && '\\' == pszText[i] && 'n' == pszText[i + 1]) || '\0' == pszText[i] ){
			if( 0 < i - nBgn ){
				char*	pszWork;
				pszWork = new char[i - nBgn + 1];
				memcpy( pszWork, &pszText[nBgn], i - nBgn );
				pszWork[i - nBgn] = '\0';
			
				rc.left = 0;
				rc.top = 0;
				rc.right = ::GetSystemMetrics( SM_CXSCREEN );
				rc.bottom = 0;
				::DrawText( hdc, pszWork, strlen(pszWork), &rc, 
					DT_CALCRECT | DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/
				);
				delete [] pszWork;
				if( nCurMaxWidth < rc.right ){
					nCurMaxWidth = rc.right; 
				}
			}else{
				::DrawText( hdc, " ", 1, &rc, 
					DT_CALCRECT | DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/
				);
			}
			nCurHeight += rc.bottom;
		
			nBgn = i + 2;
		}
		if( 2 == nCharChars ){
			++i;
		}
	}

	pRect->left = 0;
	pRect->top = 0;
	pRect->right = nCurMaxWidth + 4;
	pRect->bottom = nCurHeight + 2;

	::SelectObject( hdc, hFontOld );
	
	return;


}
	

/* ウィンドウのテキストを表示 */
void CTipWnd::DrawTipText( 
		HDC			hdc, 
		HFONT		hFont, 
		const char*	pszText 
)
{
	int			nTextLength;
	int			nCurMaxWidth;
	int			nCurHeight;
	int			nBgn;
	RECT		rc;
	HFONT		hFontOld;
	int			i;
	int			nBkMode_Old;
	COLORREF	colText_Old;
	int			nCharChars;

	nBkMode_Old = ::SetBkMode( hdc, TRANSPARENT );
	hFontOld = (HFONT)::SelectObject( hdc, hFont );
	colText_Old = ::SetTextColor( hdc, ::GetSysColor( COLOR_INFOTEXT ) );

	nCurMaxWidth = 0;
	nCurHeight = 0;
	nTextLength = strlen( pszText );
	nBgn = 0;
	for( i = 0; i <= nTextLength; ++i ){
//		nCharChars = &pszText[i] - CMemory::MemCharPrev( pszText, nTextLength, &pszText[i] );
		nCharChars = CMemory::MemCharNext( (const char *)pszText, nTextLength, (const char *)&pszText[i] ) - (const char*)&pszText[i];
		if( ( 1 == nCharChars && '\\' == pszText[i] && 'n' == pszText[i + 1]) || '\0' == pszText[i] ){
			if( 0 < i - nBgn ){
				char*	pszWork;
				pszWork = new char[i - nBgn + 1];
				memcpy( pszWork, &pszText[nBgn], i - nBgn );
				pszWork[i - nBgn] = '\0';
			
				rc.left = 4;
				rc.top = 4 + nCurHeight;
				rc.right = ::GetSystemMetrics( SM_CXSCREEN );
				rc.bottom = rc.top + 200;
				nCurHeight += ::DrawText( hdc, pszWork, strlen(pszWork), &rc, 
					DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/
				);
				delete [] pszWork;
				if( nCurMaxWidth < rc.right ){
					nCurMaxWidth = rc.right; 
				}
			}else{
				rc.left = 4;
				rc.top = 4 + nCurHeight;
				rc.right = ::GetSystemMetrics( SM_CXSCREEN );
				rc.bottom = rc.top + 200;
				nCurHeight += ::DrawText( hdc, " ", 1, &rc, 
					DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/
				);
			}
		
			nBgn = i + 2;
		}
		if( 2 == nCharChars ){
			++i;
		}
	}


	::SetTextColor( hdc, colText_Old );
	::SelectObject( hdc, hFontOld );
	::SetBkMode( hdc, nBkMode_Old );
	
	return;


}



/* Tipを消す */
void CTipWnd::Hide( void )
{
	::ShowWindow( m_hWnd, SW_HIDE );
//	::DestroyWindow( m_hWnd );
	return;
}




/* 描画処理 */
LRESULT CTipWnd::OnPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM l_Param )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	RECT		rc;
	char*		pszText;
	int			nTextLen;
//	HFONT		hFontOld;
	hdc = ::BeginPaint(	hwnd, &ps );
	pszText = m_cInfo.GetPtr( &nTextLen );
	::GetClientRect( hwnd, &rc );
	/* ウィンドウのテキストを表示 */
	DrawTipText( hdc, m_hFont, pszText );   

//	hFontOld = (HFONT)::SelectObject( hdc, m_hFont );
//
//	pszText = m_cInfo.GetPtr( &nTextLen );
//	::GetClientRect( hwnd, &rc );
//	rc.left = 4;
//	rc.top = 4;
//	::DrawText( hdc, pszText, nTextLen, &rc, 
//		DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/ );
//	::SelectObject( hdc, hFontOld );

	::EndPaint(	hwnd, &ps );
	return 0L;


}
