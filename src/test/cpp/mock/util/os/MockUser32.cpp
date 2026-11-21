/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "util/os/MockUser32.hpp"

#include "util/tchar_convert.h"

/* static */ int MockUser32::_MessageBoxExW(
	_In_opt_ HWND hWnd,
	_In_opt_ LPCWSTR lpText,
	_In_opt_ LPCWSTR lpCaption,
	_In_ UINT uType,
	_In_ WORD wLanguageId
)
{
	// lpText を標準エラー出力に書き出す
	std::clog << (lpText ? cxx::to_string(lpText, CP_UTF8) : "") << std::endl;

	// いい加減な戻り値を返す。(返り値0は未定義なので本来返らない値を返している)
	return 0;
}

/* static */ DWORD MockUser32::_MsgWaitForMultipleObjects(
	_In_ DWORD nCount,
	_In_reads_opt_(nCount) CONST HANDLE* pHandles,
	_In_ BOOL fWaitAll,
	_In_ DWORD dwMilliseconds,
	_In_ DWORD dwWakeMask
)
{
	return ::MsgWaitForMultipleObjects(
		nCount,
		pHandles,
		fWaitAll,
		dwMilliseconds,
		dwWakeMask
	);
}

/* static */ BOOL MockUser32::_WinHelpW(
	_In_opt_ HWND /* hWndMain */,
	_In_opt_ LPCWSTR /* lpszHelp */,
	_In_ UINT /* uCommand */,
	_In_ ULONG_PTR /* dwData */
)
{
	return TRUE;
}

MockUser32::MockUser32()
{
	// デフォルトの動作を設定する
	ON_CALL(*this, MessageBoxExW(_, _, _, _, _)).WillByDefault(&_MessageBoxExW);
	ON_CALL(*this, MsgWaitForMultipleObjects(_, _, _, _, _)).WillByDefault(&_MsgWaitForMultipleObjects);
	ON_CALL(*this, WinHelpW(_, _, _, _)).WillByDefault(&_WinHelpW);
}
