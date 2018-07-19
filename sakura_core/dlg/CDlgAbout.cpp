/*!	@file
	@brief バージョン情報ダイアログ

	@author Norio Nakatani
	@date	1998/3/13 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, genta, jepro
	Copyright (C) 2002, MIK, genta, aroka
	Copyright (C) 2003, Moca
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <ShellAPI.h>
#include "dlg/CDlgAbout.h"
#include "uiparts/HandCursor.h"
#include "util/file.h"
#include "util/module.h"
#include "sakura_rc.h" // 2002/2/10 aroka 復帰
#include "version.h"
#include "sakura.hh"

// バージョン情報 CDlgAbout.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12900
	IDOK,					HIDOK_ABOUT,
	IDC_EDIT_ABOUT,			HIDC_ABOUT_EDIT_ABOUT,
//	IDC_STATIC_URL_UR,		12970,
//	IDC_STATIC_URL_ORG,		12971,
//	IDC_STATIC_UPDATE,		12972,
//	IDC_STATIC_VER,			12973,
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

//	From Here Feb. 7, 2002 genta
// 2006.01.17 Moca COMPILER_VERを追加
// 2010.04.15 Moca icc/dmcを追加しCPUを分離
#if defined(_M_IA64)
#  define TARGET_M_SUFFIX "_I64"
#elif defined(_M_AMD64)
#  define TARGET_M_SUFFIX "_A64"
#else
#  define TARGET_M_SUFFIX ""
#endif

#if defined(__BORLANDC__)
#  define COMPILER_TYPE "B"
#  define COMPILER_VER  __BORLANDC__ 
#elif defined(__GNUG__)
#  define COMPILER_TYPE "G"
#  define COMPILER_VER (__GNUC__ * 10000 + __GNUC_MINOR__  * 100 + __GNUC_PATCHLEVEL__)
#elif defined(__INTEL_COMPILER)
#  define COMPILER_TYPE "I"
#  define COMPILER_VER __INTEL_COMPILER
#elif defined(__DMC__)
#  define COMPILER_TYPE "D"
#  define COMPILER_VER __DMC__
#elif defined(_MSC_VER)
#  define COMPILER_TYPE "V"
#  define COMPILER_VER _MSC_VER
#else
#  define COMPILER_TYPE "U"
#  define COMPILER_VER 0
#endif
//	To Here Feb. 7, 2002 genta

#ifdef _UNICODE
	#define TARGET_STRING_MODEL "WP"
#else
	#define TARGET_STRING_MODEL "AP"
#endif

#ifdef _DEBUG
	#define TARGET_DEBUG_MODE "D"
#else
	#define TARGET_DEBUG_MODE "R"
#endif

#define TSTR_TARGET_MODE _T(TARGET_STRING_MODEL) _T(TARGET_DEBUG_MODE)

#ifdef _WIN32_WINDOWS
	#define MY_WIN32_WINDOWS _WIN32_WINDOWS
#else
	#define MY_WIN32_WINDOWS 0
#endif

#ifdef _WIN32_WINNT
	#define MY_WIN32_WINNT _WIN32_WINNT
#else
	#define MY_WIN32_WINNT 0
#endif

#if defined(APPVEYOR_BUILD_NUMBER)
#define APPVEYOR_BUILD_TEXT	"Build " APPVEYOR_BUILD_NUMBER
#endif

#if defined(APPVEYOR_BUILD_URL)
#pragma message("APPVEYOR_BUILD_URL: " APPVEYOR_BUILD_URL)
#endif
#if defined(APPVEYOR_BUILD_TEXT)
#pragma message("APPVEYOR_BUILD_TEXT: " APPVEYOR_BUILD_TEXT)
#endif

//	From Here Nov. 7, 2000 genta
/*!
	標準以外のメッセージを捕捉する
*/
INT_PTR CDlgAbout::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		// EDITも READONLY か DISABLEの場合 WM_CTLCOLORSTATIC になります
		if( (HWND)lParam == GetDlgItem(hWnd, IDC_EDIT_ABOUT) ){
			::SetTextColor( (HDC)wParam, RGB( 102, 102, 102 ) );
		} else {
			::SetTextColor( (HDC)wParam, RGB( 0, 0, 0 ) );
        }
		return (INT_PTR)GetStockObject( WHITE_BRUSH );
	}
	return result;
}
//	To Here Nov. 7, 2000 genta

/* モーダルダイアログの表示 */
int CDlgAbout::DoModal( HINSTANCE hInstance, HWND hwndParent )
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_ABOUT, (LPARAM)NULL );
}

/*! 初期化処理
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
	@date 2011.04.10 nasukoji	各国語メッセージリソース対応
	@date 2013.04.07 novice svn revision 情報追加
*/
BOOL CDlgAbout::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );

	TCHAR			szFile[_MAX_PATH];

	/* この実行ファイルの情報 */
	::GetModuleFileName( NULL, szFile, _countof( szFile ) );
	
	//	Oct. 22, 2005 genta タイムスタンプ取得の共通関数利用

	/* バージョン情報 */
	//	Nov. 6, 2000 genta	Unofficial Releaseのバージョンとして設定
	//	Jun. 8, 2001 genta	GPL化に伴い、OfficialなReleaseとしての道を歩み始める
	//	Feb. 7, 2002 genta コンパイラ情報追加
	//	2004.05.13 Moca バージョン番号は、プロセスごとに取得する
	//	2010.04.15 Moca コンパイラ情報を分離/WINヘッダ,N_SHAREDATA_VERSION追加

	// 以下の形式で出力
	//サクラエディタ   Ver. 2.0.0.0
	//(GitHash xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx)
	//
	//      Share Ver: 96
	//      Compile Info: V 1400  WR WIN600/I601/C000/N600
	//      Last Modified: 1999/9/9 00:00:00
	//      (あればSKR_PATCH_INFOの文字列がそのまま表示)
	CNativeT cmemMsg;
	cmemMsg.AppendString(LS(STR_DLGABOUT_APPNAME)); // e.g. "サクラエディタ", "Sakura Editor"
	cmemMsg.AppendString(_T("   "));

	// バージョン情報・コンフィグ情報 //
#ifdef GIT_COMMIT_HASH
#define VER_GITHASH "(GitHash " GIT_COMMIT_HASH ")"
#endif
	DWORD dwVersionMS, dwVersionLS;
	GetAppVersionInfo( NULL, VS_VERSION_INFO, &dwVersionMS, &dwVersionLS );
	
	// 1行目
	cmemMsg.AppendStringF(
		_T("v%d.%d.%d.%d"),
		HIWORD(dwVersionMS), LOWORD(dwVersionMS), HIWORD(dwVersionLS), LOWORD(dwVersionLS) // e.g. {2, 3, 2, 0}
	);
	cmemMsg.AppendString( _T(" ") _T(VER_PLATFORM) );
	cmemMsg.AppendString( _T(SPACE_WHEN_DEBUG) _T(VER_CONFIG) );
#ifdef ALPHA_VERSION
	cmemMsg.AppendString( _T(" ") _T(ALPHA_VERSION_STR));
#endif
	cmemMsg.AppendString( _T("\r\n") );

	// 2行目
#ifdef VER_GITHASH
	cmemMsg.AppendString( _T(VER_GITHASH) _T("\r\n"));
#endif

	// 3行目
#ifdef GIT_URL
	cmemMsg.AppendString( _T("(GitURL ") _T(GIT_URL) _T(")\r\n"));
#endif

	// 段落区切り
	cmemMsg.AppendString( _T("\r\n") );

	// 共有メモリ情報
	cmemMsg.AppendStringF(_T("      Share Ver: %3d\r\n"), N_SHAREDATA_VERSION);

	// コンパイル情報
	cmemMsg.AppendString( _T("      Compile Info: ") );
	cmemMsg.AppendStringF(
		_T(COMPILER_TYPE) _T(TARGET_M_SUFFIX) _T("%d ") TSTR_TARGET_MODE _T(" WIN%03x/I%03x/C%03x/N%03x\r\n"),
		COMPILER_VER, WINVER, _WIN32_IE, MY_WIN32_WINDOWS, MY_WIN32_WINNT
	);

	// 更新日情報
	CFileTime cFileTime;
	GetLastWriteTimestamp( szFile, &cFileTime );
	cmemMsg.AppendStringF(
		_T("      Last Modified: %d/%d/%d %02d:%02d:%02d\r\n"),
		cFileTime->wYear,
		cFileTime->wMonth,
		cFileTime->wDay,
		cFileTime->wHour,
		cFileTime->wMinute,
		cFileTime->wSecond
	);

	// パッチの情報をコンパイル時に渡せるようにする
#ifdef SKR_PATCH_INFO
	cmemMsg.AppendString( _T("      ") );
	const TCHAR* ptszPatchInfo = to_tchar(SKR_PATCH_INFO);
	int patchInfoLen = auto_strlen(ptszPatchInfo);
	cmemMsg.AppendString( ptszPatchInfo, t_min(80, patchInfoLen) );
#endif
	cmemMsg.AppendString( _T("\r\n"));

	::DlgItem_SetText( GetHwnd(), IDC_EDIT_VER, cmemMsg.GetStringPtr() );

	//	From Here Jun. 8, 2001 genta
	//	Edit Boxにメッセージを追加する．
	// 2011.06.01 nasukoji	各国語メッセージリソース対応
	LPCTSTR pszDesc = LS( IDS_ABOUT_DESCRIPTION );
	TCHAR szMsg[2048];
	if( _tcslen(pszDesc) > 0 ){
		_tcsncpy( szMsg, pszDesc, _countof(szMsg) - 1 );
		szMsg[_countof(szMsg) - 1] = 0;
		::DlgItem_SetText( GetHwnd(), IDC_EDIT_ABOUT, szMsg );
	}
	//	To Here Jun. 8, 2001 genta

	//	From Here Dec. 2, 2002 genta
	//	アイコンをカスタマイズアイコンに合わせる
	HICON hIcon = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );
	HWND hIconWnd = GetDlgItem( GetHwnd(), IDC_STATIC_MYICON );
	
	if( hIconWnd != NULL && hIcon != NULL ){
		StCtl_SetIcon( hIconWnd, hIcon );
	}
	//	To Here Dec. 2, 2002 genta

	// URLウィンドウをサブクラス化する
	m_UrlUrWnd.SetSubclassWindow( GetDlgItem( GetHwnd(), IDC_STATIC_URL_UR ) );
	m_UrlGitWnd.SetSubclassWindow(GetDlgItem( GetHwnd(), IDC_STATIC_URL_GIT));
	m_UrlBuildLinkWnd.SetSubclassWindow(GetDlgItem(GetHwnd(), IDC_STATIC_URL_APPVEYOR_BUILD));
#if defined(GITHUB_COMMIT_URL) && defined(APPVEYOR_SHORTHASH)
	m_UrlGitHubCommitWnd.SetSubclassWindow(GetDlgItem(GetHwnd(), IDC_STATIC_URL_GITHUB_COMMIT));
#endif
#if defined(GITHUB_COMMIT_URL_PR_HEAD) && defined(APPVEYOR_PULL_REQUEST_NUMBER)
	m_UrlGitHubPRWnd.SetSubclassWindow(GetDlgItem(GetHwnd(), IDC_STATIC_URL_GITHUB_PR));
#endif

#ifdef GIT_URL
	::SetWindowText(::GetDlgItem(GetHwnd(), IDC_STATIC_URL_GIT), _T(GIT_URL));
#else
	ShowWindow(::GetDlgItem(GetHwnd(), IDC_STATIC_GIT_CAPTION), SW_HIDE);
	ShowWindow(::GetDlgItem(GetHwnd(), IDC_STATIC_URL_GIT), SW_HIDE);
#endif
#if defined(APPVEYOR_BUILD_TEXT)
	::SetWindowText(::GetDlgItem(GetHwnd(), IDC_STATIC_URL_APPVEYOR_BUILD), _T(APPVEYOR_BUILD_TEXT));
#else
	ShowWindow(::GetDlgItem(GetHwnd(), IDC_STATIC_URL_APPVEYOR_CAPTION), SW_HIDE);
	ShowWindow(::GetDlgItem(GetHwnd(), IDC_STATIC_URL_APPVEYOR_BUILD), SW_HIDE);
#endif

	// GitHub の Commit のリンク
#if defined(GITHUB_COMMIT_URL) && defined(APPVEYOR_SHORTHASH)
	::SetWindowText(::GetDlgItem(GetHwnd(), IDC_STATIC_URL_GITHUB_COMMIT), _T(APPVEYOR_SHORTHASH));
#else
	ShowWindow(::GetDlgItem(GetHwnd(), IDC_STATIC_URL_GITHUB_COMMIT), SW_HIDE);
#endif

	// GitHub の PR のリンク
#if defined(GITHUB_COMMIT_URL_PR_HEAD) && defined(APPVEYOR_PULL_REQUEST_NUMBER)
	::SetWindowText(::GetDlgItem(GetHwnd(), IDC_STATIC_URL_GITHUB_PR), _T("PR ") _T(APPVEYOR_PULL_REQUEST_NUMBER));
#else
	ShowWindow(::GetDlgItem(GetHwnd(), IDC_STATIC_URL_GITHUB_PR), SW_HIDE);
#endif

	// GitHub のリンクのテキスト
#if !defined(GITHUB_COMMIT_URL) && !defined(GITHUB_COMMIT_URL_PR_HEAD)
	ShowWindow(::GetDlgItem(GetHwnd(), IDC_STATIC_URL_GITHUB_CAPTION), SW_HIDE);
#endif

	//	Oct. 22, 2005 genta 原作者ホームページが無くなったので削除
	//m_UrlOrgWnd.SubclassWindow( GetDlgItem( GetHwnd(), IDC_STATIC_URL_ORG ) );

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}


BOOL CDlgAbout::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_COPY:
		{
			HWND hwndEditVer = GetDlgItem( GetHwnd(), IDC_EDIT_VER );
	 		EditCtl_SetSel( hwndEditVer, 0, -1); 
	 		SendMessage( hwndEditVer, WM_COPY, 0, 0 );
	 		EditCtl_SetSel( hwndEditVer, -1, 0); 
 		}
		return TRUE;
	}
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgAbout::OnStnClicked( int wID )
{
	switch( wID ){
	//	2006.07.27 genta 原作者連絡先のボタンを削除 (ヘルプから削除されているため)
	case IDC_STATIC_URL_UR:
	case IDC_STATIC_URL_GIT:
//	case IDC_STATIC_URL_ORG:	del 2008/7/4 Uchi
		//	Web Browserの起動
		{
			TCHAR buf[512];
			::GetWindowText( ::GetDlgItem( GetHwnd(), wID ), buf, _countof(buf) );
			::ShellExecute( GetHwnd(), NULL, buf, NULL, NULL, SW_SHOWNORMAL );
			return TRUE;
		}
	case IDC_STATIC_URL_APPVEYOR_BUILD:
		{
#if defined(APPVEYOR_BUILD_URL)
			::ShellExecute(GetHwnd(), NULL, _T(APPVEYOR_BUILD_URL), NULL, NULL, SW_SHOWNORMAL);
#elif defined(GIT_URL)
			::ShellExecute(GetHwnd(), NULL, _T(GIT_URL), NULL, NULL, SW_SHOWNORMAL);
#endif
			return TRUE;
		}
	case IDC_STATIC_URL_GITHUB_COMMIT:
#if defined(GITHUB_COMMIT_URL)
		::ShellExecute(GetHwnd(), NULL, _T(GITHUB_COMMIT_URL), NULL, NULL, SW_SHOWNORMAL);
#endif
		return TRUE;
	case IDC_STATIC_URL_GITHUB_PR:
#if defined(GITHUB_COMMIT_URL_PR_HEAD)
		::ShellExecute(GetHwnd(), NULL, _T(GITHUB_COMMIT_URL_PR_HEAD), NULL, NULL, SW_SHOWNORMAL);
#endif
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnStnClicked( wID );
}

//@@@ 2002.01.18 add start
LPVOID CDlgAbout::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

BOOL CUrlWnd::SetSubclassWindow( HWND hWnd )
{
	// STATICウィンドウをサブクラス化する
	// 元のSTATICは WS_TABSTOP, SS_NOTIFY スタイルのものを使用すること
	if( GetHwnd() != NULL )
		return FALSE;
	if( !IsWindow( hWnd ) )
		return FALSE;

	// サブクラス化を実行する
	LONG_PTR lptr;
	SetLastError( 0 );
	lptr = SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)this );
	if( lptr == 0 && GetLastError() != 0 )
		return FALSE;
	m_pOldProc = (WNDPROC)SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR)UrlWndProc );
	if( m_pOldProc == NULL )
		return FALSE;
	m_hWnd = hWnd;

	// 下線付きフォントに変更する
	HFONT hFont;
	LOGFONT lf;
	hFont = (HFONT)SendMessageAny( hWnd, WM_GETFONT, (WPARAM)0, (LPARAM)0 );
	GetObject( hFont, sizeof(lf), &lf );
	lf.lfUnderline = TRUE;
	m_hFont = CreateFontIndirect( &lf );
	if(m_hFont != NULL)
		SendMessageAny( hWnd, WM_SETFONT, (WPARAM)m_hFont, (LPARAM)FALSE );

	return TRUE;
}

LRESULT CALLBACK CUrlWnd::UrlWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	CUrlWnd* pUrlWnd = (CUrlWnd*)GetWindowLongPtr( hWnd, GWLP_USERDATA );

	HDC hdc;
	POINT pt;
	RECT rc;

	switch ( msg ) {
	case WM_SETCURSOR:
		// カーソル形状変更
		SetHandCursor();		// Hand Cursorを設定 2013/1/29 Uchi
		return (LRESULT)0;
	case WM_LBUTTONDOWN:
		// キーボードフォーカスを自分に当てる
		SendMessageAny( GetParent(hWnd), WM_NEXTDLGCTL, (WPARAM)hWnd, (LPARAM)1 );
		break;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		// 再描画
		InvalidateRect( hWnd, NULL, TRUE );
		UpdateWindow( hWnd );
		break;
	case WM_GETDLGCODE:
		// デフォルトプッシュボタンのように振舞う（Enterキーの有効化）
		// 方向キーは無効化（IEのバージョン情報ダイアログと同様）
		return DLGC_DEFPUSHBUTTON | DLGC_WANTARROWS;
	case WM_MOUSEMOVE:
		// カーソルがウィンドウ内に入ったらタイマー起動
		// ウィンドウ外に出たらタイマー削除
		// 各タイミングで再描画
		BOOL bHilighted;
		pt.x = LOWORD( lParam );
		pt.y = HIWORD( lParam );
		GetClientRect( hWnd, &rc );
		bHilighted = PtInRect( &rc, pt );
		if( bHilighted != pUrlWnd->m_bHilighted ){
			pUrlWnd->m_bHilighted = bHilighted;
			InvalidateRect( hWnd, NULL, TRUE );
			if( pUrlWnd->m_bHilighted )
				SetTimer( hWnd, 1, 200, NULL );
			else
				KillTimer( hWnd, 1 );
		}
		break;
	case WM_TIMER:
		// カーソルがウィンドウ外にある場合にも WM_MOUSEMOVE を送る
		GetCursorPos( &pt );
		ScreenToClient( hWnd, &pt );
		GetClientRect( hWnd, &rc );
		if( !PtInRect( &rc, pt ) )
			SendMessageAny( hWnd, WM_MOUSEMOVE, 0, MAKELONG( pt.x, pt.y ) );
		break;
	case WM_PAINT:
		// ウィンドウの描画
		PAINTSTRUCT ps;
		HFONT hFont;
		HFONT hFontOld;
		TCHAR szText[512];

		hdc = BeginPaint( hWnd, &ps );

		// 現在のクライアント矩形、テキスト、フォントを取得する
		GetClientRect( hWnd, &rc );
		GetWindowText( hWnd, szText, _countof(szText) );
		hFont = (HFONT)SendMessageAny( hWnd, WM_GETFONT, (WPARAM)0, (LPARAM)0 );

		// テキスト描画
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, pUrlWnd->m_bHilighted? RGB( 0x84, 0, 0 ): RGB( 0, 0, 0xff ) );
		hFontOld = (HFONT)SelectObject( hdc, (HGDIOBJ)hFont );
		TextOut( hdc, 2, 0, szText, _tcslen( szText ) );
		SelectObject( hdc, (HGDIOBJ)hFontOld );

		// フォーカス枠描画
		if( GetFocus() == hWnd )
			DrawFocusRect( hdc, &rc );

		EndPaint( hWnd, &ps );
		return (LRESULT)0;
	case WM_ERASEBKGND:
		hdc = (HDC)wParam;
		GetClientRect( hWnd, &rc );

		// 背景描画
		if( pUrlWnd->m_bHilighted ){
			// ハイライト時背景描画
			SetBkColor( hdc, RGB( 0xff, 0xff, 0 ) );
			::ExtTextOutW_AnyBuild( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
		}else{
			// 親にWM_CTLCOLORSTATICを送って背景ブラシを取得し、背景描画する
			HBRUSH hbr;
			HBRUSH hbrOld;
			hbr = (HBRUSH)SendMessageAny( GetParent( hWnd ), WM_CTLCOLORSTATIC, wParam, (LPARAM)hWnd );
			hbrOld = (HBRUSH)SelectObject( hdc, hbr );
			FillRect( hdc, &rc, hbr );
			SelectObject( hdc, hbrOld );
		}
		return (LRESULT)1;
	case WM_DESTROY:
		// 後始末
		KillTimer( hWnd, 1 );
		SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR)pUrlWnd->m_pOldProc );
		if( pUrlWnd->m_hFont != NULL )
			DeleteObject( pUrlWnd->m_hFont );
		pUrlWnd->m_hWnd = NULL;
		pUrlWnd->m_hFont = NULL;
		pUrlWnd->m_bHilighted = FALSE;
		pUrlWnd->m_pOldProc = NULL;
		return (LRESULT)0;
	}

	return CallWindowProc( pUrlWnd->m_pOldProc, hWnd, msg, wParam, lParam );
}
//@@@ 2002.01.18 add end


