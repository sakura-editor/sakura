/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "util/os/MockKernel32.hpp"

/* static */ HANDLE MockKernel32::_CreateFileW(
	_In_ LPCWSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile
)
{
	return ::CreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile
	);
}

/* static */ DWORD MockKernel32::_GetModuleFileNameW(
	_In_opt_ HMODULE hModule,
	_Out_writes_to_( nSize, ((return < nSize) ? (return +1) : nSize) )
	LPWSTR lpFilename,
	_In_ DWORD nSize
)
{
	return ::GetModuleFileNameW(
		hModule,
		lpFilename,
		nSize
	);
}

MockKernel32::MockKernel32()
{
	// デフォルトの動作を設定する
	ON_CALL(*this, CreateFileW(_, _, _, _, _, _, _)).WillByDefault(&_CreateFileW);
	ON_CALL(*this, GetModuleFileNameW(_, _, _)).WillByDefault(&_GetModuleFileNameW);
}
