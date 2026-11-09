/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "util/os/User32.hpp"

BOOL User32::TrackPopupMenu(
	_In_ HMENU hMenu,
	_In_ UINT uFlags,
	_In_ int x,
	_In_ int y,
	_Reserved_ int nReserved,
	_In_ HWND hWnd,
	_Reserved_ CONST RECT* prcRect
) const
{
	return ::TrackPopupMenu(
		hMenu,
		uFlags,
		x,
		y,
		nReserved,
		hWnd,
		prcRect
	);
}

/*!
 * @brief MyWinHelpの呼出をモックできるようにするための関数
 *
 * @note 直接呼ばないでください
 */
BOOL User32::WinHelpW(
	_In_opt_ HWND,
	_In_opt_ LPCWSTR,
	_In_ UINT,
	_In_ ULONG_PTR
) const
{
	return FALSE;
}
