/*!	@file
	@brief ツールチップ

	@author Norio Nakatani
	@date 1998/10/30 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, GAE
	Copyright (C) 2005, D.S.Koba
	Copyright (C) 2006, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CTipWnd.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"


// ダミー文字列
static constexpr TCHAR szDummy[] = { _T(" ") };

// 改行コードを表す定数
static constexpr TCHAR szEscapedLF[] = { _T("\\n") };


/* CTipWndクラス デストラクタ */
CTipWnd::CTipWnd()
: CWnd(_T("::CTipWnd"))
, m_bAlignLeft(false)
{
	m_hFont = NULL;
	m_KeyWasHit = FALSE;	/* キーがヒットしたか */
	return;
}



/* CTipWndクラス デストラクタ */
CTipWnd::~CTipWnd()
{
	if( NULL != m_hFont ){
		::DeleteObject( m_hFont );
		m_hFont = NULL;
	}
	return;
}



/* 初期化 */
void CTipWnd::Create( HINSTANCE hInstance, HWND hwndParent )
{
	LPCTSTR pszClassName = _T("CTipWnd");

	/* ウィンドウクラス作成 */
	RegisterWC(
		hInstance,
		/* WNDCLASS用 */
		NULL,// Handle to the class icon.
		NULL,	//Handle to a small icon
		::LoadCursor( NULL, IDC_ARROW ),// Handle to the class cursor.
		(HBRUSH)/*NULL*/(COLOR_INFOBK + 1),// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		pszClassName// Pointer to a null-terminated string or is an atom.
	);

	/* 基底クラスメンバ呼び出し */
	// 2006.01.09 ryoji 初期状態を不可視にする
	//	実質的には見えないCTipWndが最前面にあると判断されてしまう場合があるため
	CWnd::Create(
		hwndParent,
		WS_EX_TOOLWINDOW, // extended window style	// 2002/2/3 GAE
		pszClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName, // pointer to window name
		WS_POPUP | WS_CLIPCHILDREN | WS_BORDER, // window style
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

	m_hFont = ::CreateFontIndirect( &(GetDllShareData().m_Common.m_sHelper.m_lf) );
	return;
}

/*!	CreateWindowの後

	CWnd::AfterCreateWindowでウィンドウを表示するようになっているのを
	動かなくするための空関数

	@date 2006.01.09 genta 新規作成
*/
void CTipWnd::AfterCreateWindow( void )
{
}

/* Tipを表示 */
void CTipWnd::Show( int nX, int nY, const TCHAR* szText, RECT* pRect )
{
	HDC		hdc;
	RECT	rc;

	if( NULL != szText ){
		m_cInfo.SetString( szText );
	}

	hdc = ::GetDC( GetHwnd() );

	// サイズを計算済み	2001/06/19 asa-o
	if(pRect != NULL)
	{
		rc = *pRect;
	}
	else
	{
		/* ウィンドウのサイズを決める */
		ComputeWindowSize( hdc, &rc );
	}

	::ReleaseDC( GetHwnd(), hdc );

	if( m_bAlignLeft ){
		// 右側固定で表示(MiniMap)
		::MoveWindow( GetHwnd(), nX - rc.right, nY, rc.right + 8, rc.bottom + 8, TRUE );
	}else{
		// 左側固定で表示(通常)
		::MoveWindow( GetHwnd(), nX, nY, rc.right + 8, rc.bottom + 8/*nHeight*/, TRUE );
	}
	::InvalidateRect( GetHwnd(), NULL, TRUE );
	::ShowWindow( GetHwnd(), SW_SHOWNA );
	return;

}

/* ウィンドウのサイズを決める */
void CTipWnd::ComputeWindowSize(
	const HDC		hdc,
	RECT*			prcResult
)
{
	assert( m_hFont != NULL );
	assert( hdc != NULL );
	assert( prcResult != NULL );

	// システム設定値を取得
	const int cxScreen = ::GetSystemMetrics( SM_CXSCREEN );

	// 計測対象をメンバ変数からローカル変数に取得
	const TCHAR* pszText = m_cInfo.GetStringPtr();
	const size_t cchText = m_cInfo.GetStringLength();

	// 計測結果を格納する変数
	int nCurMaxWidth = 0;
	int nCurHeight = 0;

	HGDIOBJ hFontOld = ::SelectObject( hdc, m_hFont );

	for ( size_t i = 0, nLineBgn = 0; i <= cchText; ) {
		// iの位置にNUL終端、または"\n"がある場合
		if ( pszText[i] == _T('\0')
			|| ( i + 1 <= cchText && 0 == ::_tcsncmp( &pszText[i], szEscapedLF, _countof(szEscapedLF) - 1 ) ) ) {
			// 計測結果を格納する矩形
			CMyRect rc;
			// 計測対象の文字列がブランクでない場合
			if ( 0 < i - nLineBgn ) {
				// ワードラップを有効にするため幅だけ指定しておく
				rc.SetXYWH( 0, 0, cxScreen, 0 );

				// テキスト描画に必要な矩形を計測する
				::DrawText( hdc, &pszText[nLineBgn], i - nLineBgn, &rc,
					DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS | DT_EXTERNALLEADING
				);

				// 計測した幅が最大幅を越えたら更新する
				if ( nCurMaxWidth < rc.Width() ) {
					nCurMaxWidth = rc.Width();
				}
			}else{
				// ダミー文字列を計測して必要な高さを取得する
				::DrawText( hdc, szDummy, _countof( szDummy ) - 1, &rc, DT_CALCRECT );
			}

			// 計測した高さを加算する(行間は考慮しない)
			nCurHeight += rc.Height();

			// NUL終端の後に文字はないのでここで確実に抜ける
			if ( pszText[i] == _T('\0') ) {
				break;
			}

			// 次の行の開始位置を設定する
			nLineBgn = i + _countof(szEscapedLF) - 1;
			i = nLineBgn;
			continue;
		}

		// 次の文字位置を取得する
		LPCTSTR pNext = ::CharNext( &pszText[i] );
		i = pNext - pszText;
	}

	::SelectObject( hdc, hFontOld );

	prcResult->left = 0;
	prcResult->top = 0;
	prcResult->right = nCurMaxWidth + 4;
	prcResult->bottom = nCurHeight + 2;

	return;


}


/* ウィンドウのテキストを表示 */
void CTipWnd::DrawTipText(
	const HDC		hdc,
	const RECT*		prcPaint
)
{
	assert( m_hFont != NULL );
	assert( hdc != NULL );
	assert( prcPaint != NULL );

	// 行バッファとして使いまわす領域を確保。
	size_t maxBufWork = MAX_PATH;
	auto bufWork = std::make_unique<TCHAR[]>( maxBufWork );

	// 描画矩形
	CMyRect rc( *prcPaint );
	rc.left = 4;
	rc.top = 4;

	int nBkMode_Old = ::SetBkMode( hdc, TRANSPARENT );
	HGDIOBJ hFontOld = ::SelectObject( hdc, m_hFont );
	COLORREF colText_Old = ::SetTextColor( hdc, ::GetSysColor( COLOR_INFOTEXT ) );

	const TCHAR* pszText = m_cInfo.GetStringPtr();
	const size_t cchText = m_cInfo.GetStringLength();

	for ( size_t i = 0, nLineBgn = 0; i <= cchText; ) {
		// iの位置にNUL終端、または"\n"がある場合
		if ( pszText[i] == _T('\0')
			|| ( i + 1 <= cchText && 0 == ::_tcsncmp( &pszText[i], szEscapedLF, _countof(szEscapedLF) - 1 ) ) ) {
			int nHeight;
			// 描画対象の文字列長
			size_t cchWork = i - nLineBgn;
			if ( 0 < cchWork ) {
				// 1行の長さがバッファ長を越えたらバッファを拡張する
				if ( maxBufWork < cchWork + 1 ) {
					maxBufWork = cchWork + 1;
					bufWork = std::make_unique<TCHAR[]>( maxBufWork );
				}
				TCHAR* pszWork = bufWork.get();
				::_tcsncpy_s( pszWork, maxBufWork, &pszText[nLineBgn], _TRUNCATE );

				nHeight = ::DrawText( hdc, pszWork, cchWork, &rc,
					DT_WORDBREAK | DT_EXPANDTABS | DT_EXTERNALLEADING
					DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/
				);
			}else{
				nHeight = ::DrawText( hdc, szDummy, _countof(szDummy) - 1, &rc,
					DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/
				);
			}

			// 描画領域の上端を1行分ずらす
			rc.top += nHeight + 4;

			// NUL終端の後に文字はないのでここで確実に抜ける
			if ( pszText[i] == _T('\0') ) {
				break;
			}

			// 次の行の開始位置を設定する
			nLineBgn = i + _countof(szEscapedLF) - 1;
			i = nLineBgn;
			continue;
		}

		// 次の文字位置を取得する
		LPCTSTR pNext = ::CharNext( &pszText[i] );
		i = pNext - pszText;
	}

	::SetTextColor( hdc, colText_Old );
	::SelectObject( hdc, hFontOld );
	::SetBkMode( hdc, nBkMode_Old );

	return;


}



/* Tipを消す */
void CTipWnd::Hide( void )
{
	::ShowWindow( GetHwnd(), SW_HIDE );
//	::DestroyWindow( GetHwnd() );
	return;
}




/* 描画処理 */
LRESULT CTipWnd::OnPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM l_Param )
{
	PAINTSTRUCT	ps;
	HDC			hdc = ::BeginPaint(	hwnd, &ps );

	/* ウィンドウのテキストを表示 */
	DrawTipText( hdc, &ps.rcPaint );

	::EndPaint(	hwnd, &ps );
	return 0L;
}


// 2001/06/19 Start by asa-o: ウィンドウのサイズを得る
void CTipWnd::GetWindowSize(LPRECT pRect)
{
	HDC		hdc = ::GetDC( GetHwnd() );

	// ウィンドウのサイズを得る
	ComputeWindowSize( hdc, pRect );

	::ReleaseDC( GetHwnd(), hdc ); //2007.10.10 kobake ReleaseDCが抜けていたのを修正
}

// 2001/06/19 End



