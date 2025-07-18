﻿/*!	@file
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
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CTipWnd.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "util/window.h"

// ダミー文字列
static constexpr WCHAR szDummy[] = { L" " };

/* Tipの内容データを設定するためにエスケープ解除を行う */
const WCHAR* UnEscapeInfoText( CNativeW& cInfo )
{
	cInfo.Replace( L"\\n", L"\n" );
	cInfo.Replace( L"\\x5C", L"\\" );
	return cInfo.GetStringPtr();
}

/* CTipWndクラス デストラクタ */
CTipWnd::CTipWnd()
: CWnd(L"::CTipWnd")
, m_bAlignLeft(false)
{
	m_hFont = nullptr;
	m_KeyWasHit = FALSE;	/* キーがヒットしたか */
	return;
}

/* CTipWndクラス デストラクタ */
CTipWnd::~CTipWnd()
{
	if( nullptr != m_hFont ){
		::DeleteObject( m_hFont );
		m_hFont = nullptr;
	}
	return;
}

/* 初期化 */
void CTipWnd::Create( HINSTANCE hInstance, HWND hwndParent )
{
	LPCWSTR pszClassName = L"CTipWnd";

	/* ウィンドウクラス作成 */
	RegisterWC(
		hInstance,
		/* WNDCLASS用 */
		nullptr,// Handle to the class icon.
		nullptr,	//Handle to a small icon
		::LoadCursor( nullptr, IDC_ARROW ),// Handle to the class cursor.
		(HBRUSH)/*NULL*/(COLOR_INFOBK + 1),// Handle to the class background brush.
		nullptr/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
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
		nullptr // handle to menu, or child-window identifier
	);

	if( nullptr != m_hFont ){
		::DeleteObject( m_hFont );
		m_hFont = nullptr;
	}

	m_hFont = ::CreateFontIndirect( &(GetDllShareData().m_Common.m_sHelper.m_lf) );
	return;
}

/* Tipを表示 */
void CTipWnd::Show( int nX, int nY, RECT* pRect )
{
	HDC		hdc;
	RECT	rc;

	hdc = ::GetDC( GetHwnd() );

	// サイズを計算済み	2001/06/19 asa-o
	if(pRect != nullptr)
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
	::InvalidateRect( GetHwnd(), nullptr, TRUE );
	::ShowWindow( GetHwnd(), SW_SHOWNA );
	return;
}

/* ウィンドウのサイズを決める */
void CTipWnd::ComputeWindowSize(
	const HDC		hdc,
	RECT*			prcResult
)
{
	assert( m_hFont != nullptr );
	assert( hdc != nullptr );
	assert( prcResult != nullptr );

	// システム設定値を取得
	const int cxScreen = ::GetSystemMetrics( SM_CXSCREEN );

	// 余白の設計値をHighDPI対応の値にする
	const int cx4 = DpiScaleX( 4 );
	const int cy4 = DpiScaleY( 4 );

	// 計測対象をメンバ変数からローカル変数に取得
	const WCHAR* pszText = m_cInfo.GetStringPtr();
	const size_t cchText = m_cInfo.GetStringLength();

	// 計測結果を格納する変数
	int nCurMaxWidth = 0;
	int nCurHeight = 0;

	HGDIOBJ hFontOld = ::SelectObject( hdc, m_hFont );

	for ( size_t i = 0, nLineBgn = 0; i <= cchText; ) {
		// iの位置がNUL終端かどうか
		const bool isEndOfText = ( pszText[i] == '\0' );
		// iの位置にNUL終端、または"\n"がある場合
		if ( isEndOfText
			|| pszText[i] == '\n' ) {
			// 計測結果を格納する矩形
			CMyRect rc;
			// 計測対象の文字列がブランクでない場合
			if ( 0 < i - nLineBgn ) {
				// ワードラップを有効にするため幅だけ指定しておく
				rc.SetXYWH( 0, 0, cxScreen, 0 );

				// テキスト描画に必要な矩形を計測する
				::DrawText( hdc, &pszText[nLineBgn], static_cast<int>(i - nLineBgn), &rc,
					DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS | DT_EXTERNALLEADING
				);

				// 計測した幅が最大幅を超えたら更新する
				if ( nCurMaxWidth < rc.Width() ) {
					nCurMaxWidth = rc.Width();
				}
			}else{
				// ダミー文字列を計測して必要な高さを取得する
				::DrawText( hdc, szDummy, _countof( szDummy ) - 1, &rc, DT_CALCRECT | DT_EXTERNALLEADING );
			}

			// 計測した高さを加算する
			nCurHeight += rc.Height() + cy4;

			// NUL終端の後に文字はないのでここで確実に抜ける
			if ( isEndOfText ) {
				break;
			}

			// 次の行の開始位置を設定する
			nLineBgn = i + 1; // "\n" の文字数
			i = nLineBgn;
		}else{
			// 現在位置の文字がWCHAR単位で何文字に当たるか計算してインデックスを進める
			size_t nCharCount = CNativeW::GetSizeOfChar( pszText, cchText, i );
			i += nCharCount;
		}
	}

	::SelectObject( hdc, hFontOld );

	prcResult->left = 0;
	prcResult->top = 0;
	prcResult->right = nCurMaxWidth + cx4 * 2; //※左右マージンだから2倍
	prcResult->bottom = nCurHeight + cy4;

	return;
}

/* ウィンドウのテキストを表示 */
void CTipWnd::DrawTipText(
	const HDC		hdc,
	const RECT&		rcPaint
)
{
	assert( m_hFont != nullptr );
	assert( hdc != nullptr );

	// 余白の設計値をHighDPI対応の値にする
	const int cx4 = DpiScaleX( 4 );
	const int cy4 = DpiScaleY( 4 );

	// 描画対象をメンバ変数からローカル変数に取得
	const WCHAR* pszText = m_cInfo.GetStringPtr();
	const size_t cchText = m_cInfo.GetStringLength();

	// 描画矩形
	CMyRect rc( rcPaint );
	rc.left = cx4;
	rc.top = cy4;

	int nBkModeOld = ::SetBkMode( hdc, TRANSPARENT );
	HGDIOBJ hFontOld = ::SelectObject( hdc, m_hFont );
	COLORREF textColorOld = ::SetTextColor( hdc, ::GetSysColor( COLOR_INFOTEXT ) );

	for ( size_t i = 0, nLineBgn = 0; i <= cchText; ) {
		// iの位置がNUL終端かどうか
		const bool isEndOfText = ( pszText[i] == '\0' );
		// iの位置にNUL終端、または"\n"がある場合
		if ( isEndOfText
			|| pszText[i] == '\n' ) {
			int nHeight;
			// 計測対象の文字列がブランクでない場合
			if ( 0 < i - nLineBgn ) {
				// 指定されたテキストを描画する
				nHeight = ::DrawText( hdc, &pszText[nLineBgn], static_cast<int>(i - nLineBgn), &rc,
					DT_WORDBREAK | DT_EXPANDTABS | DT_EXTERNALLEADING
				);
			}else{
				// ダミー文字列の高さを取得する
				nHeight = ::DrawText( hdc, szDummy, _countof(szDummy) - 1, &rc, DT_EXTERNALLEADING );
			}

			// 描画領域の上端を1行分ずらす
			rc.top += nHeight + cy4;

			// NUL終端の後に文字はないのでここで確実に抜ける
			if ( isEndOfText ) {
				break;
			}

			// 次の行の開始位置を設定する
			nLineBgn = i + 1; // "\n" の文字数
			i = nLineBgn;
		}else{
			// 現在位置の文字がWCHAR単位で何文字に当たるか計算してインデックスを進める
			size_t nCharCount = CNativeW::GetSizeOfChar( pszText, cchText, i );
			i += nCharCount;
		}
	}

	::SetTextColor( hdc, textColorOld );
	::SelectObject( hdc, hFontOld );
	::SetBkMode( hdc, nBkModeOld );

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
	DrawTipText( hdc, ps.rcPaint );

	::EndPaint(	hwnd, &ps );
	return 0L;
}

// 2001/06/19 Start by asa-o: ウィンドウのサイズを得る
void CTipWnd::GetWindowSize(LPRECT pRect)
{
	// CEditView::ShowKeywordHelpから呼ばれる
	// 当面、pRectがNULLになることはないが、安全のため入れておく。
	if ( pRect == nullptr ) {
		return;
	}

	HDC		hdc = ::GetDC( GetHwnd() );

	// ウィンドウのサイズを得る
	ComputeWindowSize( hdc, pRect );

	::ReleaseDC( GetHwnd(), hdc ); //2007.10.10 kobake ReleaseDCが抜けていたのを修正
}

// 2001/06/19 End
