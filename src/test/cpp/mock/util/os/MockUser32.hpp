/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/os/User32.hpp"

#include <gmock/gmock.h>

struct MockUser32 final : public User32
{
	MOCK_CONST_METHOD5(MessageBoxExW, int(
		_In_opt_ HWND hWnd,
		_In_opt_ LPCWSTR lpText,
		_In_opt_ LPCWSTR lpCaption,
		_In_ UINT uType,
		_In_ WORD wLanguageId
	));

	MOCK_CONST_METHOD7(TrackPopupMenu, BOOL(
		_In_ HMENU hMenu,
		_In_ UINT uFlags,
		_In_ int x,
		_In_ int y,
		_Reserved_ int nReserved,
		_In_ HWND hWnd,
		_Reserved_ CONST RECT* prcRect
	));

	MOCK_CONST_METHOD4(WinHelpW, BOOL(
		_In_opt_ HWND hWndMain,
		_In_opt_ LPCWSTR lpszHelp,
		_In_ UINT uCommand,
		_In_ ULONG_PTR dwData
	));

	static int _MessageBoxExW(
		_In_opt_ HWND hWnd,
		_In_opt_ LPCWSTR lpText,
		_In_opt_ LPCWSTR lpCaption,
		_In_ UINT uType,
		_In_ WORD wLanguageId
	);

	static BOOL _WinHelpW(
		_In_opt_ HWND hWndMain,
		_In_opt_ LPCWSTR lpszHelp,
		_In_ UINT uCommand,
		_In_ ULONG_PTR dwData
	);

	MockUser32();
};

