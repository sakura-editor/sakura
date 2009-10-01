// 2007.10.19 kobake
// なんかシェルっぽい機能の関数群

#pragma once


BOOL MyWinHelp(HWND hwndCaller, LPCTSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);	/* WinHelp のかわりに HtmlHelp を呼び出す */	// 2006.07.22 ryoji

/* Shell Interface系(?) */
SAKURA_CORE_API BOOL SelectDir(HWND, const TCHAR*, const TCHAR*, TCHAR* );	/* フォルダ選択ダイアログ */
SAKURA_CORE_API BOOL ResolveShortcutLink(HWND hwnd, LPCTSTR lpszLinkFile, LPTSTR lpszPath);/* ショートカット(.lnk)の解決 */

SAKURA_CORE_API HWND OpenHtmlHelp( HWND hWnd, LPCTSTR szFile, UINT uCmd, DWORD_PTR data,bool msgflag = true);
SAKURA_CORE_API DWORD NetConnect ( const TCHAR strNetWorkPass[] );

/* ヘルプの目次を表示 */
SAKURA_CORE_API void ShowWinHelpContents( HWND hwnd, LPCTSTR lpszHelp );

BOOL GetSpecialFolderPath( int nFolder, LPTSTR pszPath );	// 特殊フォルダのパスを取得する	// 2007.05.19 ryoji



int MyPropertySheet( LPPROPSHEETHEADER lppsph );	// 独自拡張プロパティシート	// 2007.05.24 ryoji


//!フォント選択ダイアログ
BOOL MySelectFont( LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner );	// 2009.10.01 ryoji ポイントサイズ（1/10ポイント単位）引数追加
