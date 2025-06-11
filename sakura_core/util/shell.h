/*! @file */
// 2007.10.19 kobake
// なんかシェルっぽい機能の関数群
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_SHELL_0A8B6454_B007_46E5_9606_8D2FD7993B91_H_
#define SAKURA_SHELL_0A8B6454_B007_46E5_9606_8D2FD7993B91_H_
#pragma once

#include <Windows.h>

BOOL MyWinHelp(HWND hwndCaller, UINT uCommand, DWORD_PTR dwData);	/* WinHelp のかわりに HtmlHelp を呼び出す */	// 2006.07.22 ryoji

/* Shell Interface系(?) */
BOOL SelectDir(HWND hWnd, const WCHAR* pszTitle, const WCHAR* pszInitFolder, WCHAR* strFolderName, size_t nMaxCount );	/* フォルダー選択ダイアログ */

template <size_t nMaxCount>
BOOL SelectDir(HWND hWnd, const WCHAR* pszTitle, const WCHAR* pszInitFolder, WCHAR(&strFolderName)[nMaxCount])
{
	return SelectDir( hWnd, pszTitle, pszInitFolder, strFolderName, nMaxCount );
}

BOOL ResolveShortcutLink(HWND hwnd, LPCWSTR lpszLinkFile, LPWSTR lpszPath);/* ショートカット(.lnk)の解決 */

HWND OpenHtmlHelp( HWND hWnd, LPCWSTR szFile, UINT uCmd, DWORD_PTR data,bool msgflag = true);
DWORD NetConnect ( const WCHAR strNetWorkPass[] );

/* ヘルプの目次を表示 */
void ShowWinHelpContents( HWND hwnd );

INT_PTR MyPropertySheet( LPPROPSHEETHEADER lppsph );	// 独自拡張プロパティシート	// 2007.05.24 ryoji

//!フォント選択ダイアログ
BOOL MySelectFont( LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner, bool );	// 2009.10.01 ryoji ポイントサイズ（1/10ポイント単位）引数追加
#endif /* SAKURA_SHELL_0A8B6454_B007_46E5_9606_8D2FD7993B91_H_ */
