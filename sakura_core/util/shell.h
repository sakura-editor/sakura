// 2007.10.19 kobake
// なんかシェルっぽい機能の関数群
/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_SHELL_A129670C_6564_4E0D_AF52_E323B0C7CA099_H_
#define SAKURA_SHELL_A129670C_6564_4E0D_AF52_E323B0C7CA099_H_

BOOL MyWinHelp(HWND hwndCaller, UINT uCommand, DWORD_PTR dwData);	/* WinHelp のかわりに HtmlHelp を呼び出す */	// 2006.07.22 ryoji

/* Shell Interface系(?) */
BOOL SelectDir(HWND, const TCHAR*, const TCHAR*, TCHAR* );	/* フォルダ選択ダイアログ */
BOOL ResolveShortcutLink(HWND hwnd, LPCTSTR lpszLinkFile, LPTSTR lpszPath);/* ショートカット(.lnk)の解決 */

HWND OpenHtmlHelp( HWND hWnd, LPCTSTR szFile, UINT uCmd, DWORD_PTR data,bool msgflag = true);
DWORD NetConnect ( const TCHAR strNetWorkPass[] );

/* ヘルプの目次を表示 */
void ShowWinHelpContents( HWND hwnd );

BOOL GetSpecialFolderPath( int nFolder, LPTSTR pszPath );	// 特殊フォルダのパスを取得する	// 2007.05.19 ryoji



INT_PTR MyPropertySheet( LPPROPSHEETHEADER lppsph );	// 独自拡張プロパティシート	// 2007.05.24 ryoji


//!フォント選択ダイアログ
BOOL MySelectFont( LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner, bool );	// 2009.10.01 ryoji ポイントサイズ（1/10ポイント単位）引数追加

#endif /* SAKURA_SHELL_A129670C_6564_4E0D_AF52_E323B0C7CA099_H_ */
/*[EOF]*/
