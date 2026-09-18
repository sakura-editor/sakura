/*! @file */
// 2007.10.19 kobake
// なんかシェルっぽい機能の関数群
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_SHELL_0A8B6454_B007_46E5_9606_8D2FD7993B91_H_
#define SAKURA_SHELL_0A8B6454_B007_46E5_9606_8D2FD7993B91_H_
#pragma once

#include "util/design_template.h"

#include <Windows.h>
#include <Shlwapi.h>

//! Shell32.dll呼出をテスト可能にするDIっぽいもの
struct Shell32 : public TSakuraSingleton<Shell32>
{
	using Me = Shell32;

	~Shell32() override = default;

	virtual BOOL	ShellExecuteExW(SHELLEXECUTEINFOW* pExecInfo) const;
};

BOOL MyWinHelp(HWND hwndCaller, UINT uCommand, DWORD_PTR dwData);	/* WinHelp のかわりに HtmlHelp を呼び出す */	// 2006.07.22 ryoji

/* Shell Interface系(?) */
namespace cxx {

/*! @brief フォルダー選択ダイアログ */
BOOL SelectDir(
	_In_opt_ HWND hWnd,
	_In_z_ LPCWSTR title,
	_In_z_ LPCWSTR initialDirectory,
	std::span<WCHAR> buffer
);

} // namespace cxx

/*! @brief フォルダー選択ダイアログ */
template <basis::NullTerminatedStringConstructible<WCHAR> A1, basis::NullTerminatedStringConstructible<WCHAR> A2, basis::WritableBuffer<WCHAR> A3>
BOOL SelectDir(
	_In_opt_ HWND hWnd,
	const A1& title,
	const A2& initialDirectory,
	A3& desitination
)
{
	return cxx::SelectDir(
		hWnd,
		static_cast<LPCWSTR>(cxx::NullTerminatedString(title)),
		static_cast<LPCWSTR>(cxx::NullTerminatedString(initialDirectory)),
		static_cast<std::span<WCHAR>>(desitination)
	);
}

/*! @brief フォルダー選択ダイアログ */
template <basis::NullTerminatedStringConstructible<WCHAR> A1, basis::NullTerminatedStringConstructible<WCHAR> A2>
BOOL SelectDir(
	_In_opt_ HWND hWnd,
	const A1& title,
	const A2& initialDirectory,
	WCHAR* pszFolderName,
	size_t cchFolderNmme
)
{
	// 出力先を固定長バッファとして扱う
	auto buffer = std::span(pszFolderName, cchFolderNmme);

	return SelectDir(hWnd, title, initialDirectory, buffer);
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
