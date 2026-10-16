/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/design_template.h"

#include <Windows.h>

//! User32.dll呼出をテスト可能にするDIっぽいもの
struct User32 : public TSakuraSingleton<User32>
{
	using Me = User32;

	~User32() override = default;

	virtual int	MessageBoxExW(
		_In_opt_ HWND hWnd,
		_In_opt_ LPCWSTR lpText,
		_In_opt_ LPCWSTR lpCaption,
		_In_ UINT uType,
		_In_ WORD wLanguageId
	) const;

	virtual BOOL TrackPopupMenu(
		_In_ HMENU hMenu,
		_In_ UINT uFlags,
		_In_ int x,
		_In_ int y,
		_Reserved_ int nReserved,
		_In_ HWND hWnd,
		_Reserved_ CONST RECT* prcRect
	) const;

	virtual BOOL WinHelpW(
		_In_opt_ HWND hWndMain,
		_In_opt_ LPCWSTR lpszHelp,
		_In_ UINT uCommand,
		_In_ ULONG_PTR dwData
	) const;
};
