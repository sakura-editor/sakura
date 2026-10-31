/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "util/os/MockKernel32.hpp"

/* static */ HANDLE MockKernel32::_CreateFileMappingW(
	_In_ HANDLE hFile,
	_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
	_In_ DWORD flProtect,
	_In_ DWORD dwMaximumSizeHigh,
	_In_ DWORD dwMaximumSizeLow,
	_In_opt_ LPCWSTR lpName
)
{
	return ::CreateFileMappingW(
		hFile,
		lpFileMappingAttributes,
		flProtect,
		dwMaximumSizeHigh,
		dwMaximumSizeLow,
		lpName
	);
}

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

/* static */ HANDLE MockKernel32::_CreateMutexW(
	_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
	_In_ BOOL bInitialOwner,
	_In_opt_ LPCWSTR lpName
)
{
	return ::CreateMutexW(
		lpMutexAttributes,
		bInitialOwner,
		lpName
	);
}

/* static */ HANDLE MockKernel32::_FindFirstFileW(
	_In_ LPCWSTR lpFileName,
	_Out_ LPWIN32_FIND_DATAW lpFindFileData
)
{
	return ::FindFirstFileW(
		lpFileName,
		lpFindFileData
	);
}

/* static */ BOOL MockKernel32::_FindNextFileW(
	_In_ HANDLE hFindFile,
	_Out_ LPWIN32_FIND_DATAW lpFindFileData
)
{
	return ::FindNextFileW(
		hFindFile,
		lpFindFileData
	);
}

/* static */ HRSRC MockKernel32::_FindResourceG(
	_In_opt_ HMODULE hModule,
	_In_ LPCVOID lpName,
	_In_ LPCVOID lpType
)
{
	return ::FindResourceW(
		hModule,
		(LPCWSTR)lpName,
		(LPCWSTR)lpType
	);
}

/* static */ DWORD MockKernel32::_GetCurrentDirectoryW(
	_In_ DWORD nBufferLength,
	_Out_writes_to_opt_( nBufferLength, return +1 )
	LPWSTR lpBuffer
)
{
	return ::GetCurrentDirectoryW(
		nBufferLength,
		lpBuffer
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


/* static */ FARPROC MockKernel32::_GetProcAddress(
	_In_ HMODULE hModule,
	_In_ LPCSTR lpProcName
)
{
	return ::GetProcAddress(
		hModule,
		lpProcName
	);
}

/* static */ UINT MockKernel32::_GetSystemDirectoryW(
	_Out_writes_to_opt_( uSize, return +1 )
	LPWSTR lpBuffer,
	_In_ UINT uSize
)
{
	return ::GetSystemDirectoryW(
		lpBuffer,
		uSize
	);
}

/* static */ HMODULE MockKernel32::_LoadLibraryExW(
	_In_ LPCWSTR lpLibFileName,
	_Reserved_ HANDLE hFile,
	_In_ DWORD dwFlags
)
{
	return ::LoadLibraryExW(
		lpLibFileName,
		hFile,
		dwFlags
	);
}

/* static */ HGLOBAL MockKernel32::_LoadResource(
	_In_opt_ HMODULE hModule,
	_In_ HRSRC hResInfo
)
{
	return ::LoadResource(
		hModule,
		hResInfo
	);
}

/* static */ LPVOID MockKernel32::_LockResource(
	_In_ HGLOBAL hResData
)
{
	return ::LockResource(hResData);
}

/* static */ LPVOID MockKernel32::_MapViewOfFile(
	_In_ HANDLE hFileMappingObject,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwFileOffsetHigh,
	_In_ DWORD dwFileOffsetLow,
	_In_ SIZE_T dwNumberOfBytesToMap
)
{
	return ::MapViewOfFile(
		hFileMappingObject,
		dwDesiredAccess,
		dwFileOffsetHigh,
		dwFileOffsetLow,
		dwNumberOfBytesToMap
	);
}

/* static */ HANDLE MockKernel32::_OpenFileMappingW(
	_In_ DWORD dwDesiredAccess,
	_In_ BOOL bInheritHandle,
	_In_ LPCWSTR lpName
)
{
	return ::OpenFileMappingW(
		dwDesiredAccess,
		bInheritHandle,
		lpName
	);
}

/* static */ BOOL MockKernel32::_SetCurrentDirectoryW(
	_In_ LPCWSTR lpPathName
)
{
	return ::SetCurrentDirectoryW(
		lpPathName
	);
}

/* static */ DWORD MockKernel32::_SizeofResource(
	_In_opt_ HMODULE hModule,
	_In_ HRSRC hResInfo
)
{
	return ::SizeofResource(
		hModule,
		hResInfo
	);
}

/* static */ DWORD MockKernel32::_WaitForSingleObject(
	_In_ HANDLE hHandle,
	_In_ DWORD dwMilliseconds
)
{
	return ::WaitForSingleObject(
		hHandle,
		dwMilliseconds
	);
}

MockKernel32::MockKernel32()
{
	// デフォルトの動作を設定する
	ON_CALL(*this, CreateFileMappingW(_, _, _, _, _, _)).WillByDefault(&_CreateFileMappingW);
	ON_CALL(*this, CreateFileW(_, _, _, _, _, _, _)).WillByDefault(&_CreateFileW);
	ON_CALL(*this, CreateMutexW(_, _, _)).WillByDefault(&_CreateMutexW);
	ON_CALL(*this, FindResourceG(_, _, _)).WillByDefault(&_FindResourceG);
	ON_CALL(*this, GetCurrentDirectoryW(_, _)).WillByDefault(&_GetCurrentDirectoryW);
	ON_CALL(*this, GetModuleFileNameW(_, _, _)).WillByDefault(&_GetModuleFileNameW);
	ON_CALL(*this, GetProcAddress(_, _)).WillByDefault(&_GetProcAddress);
	ON_CALL(*this, GetSystemDirectoryW(_, _)).WillByDefault(&_GetSystemDirectoryW);
	ON_CALL(*this, LoadResource(_, _)).WillByDefault(&_LoadResource);
	ON_CALL(*this, LockResource(_)).WillByDefault(&_LockResource);
	ON_CALL(*this, MapViewOfFile(_, _, _, _, _)).WillByDefault(&_MapViewOfFile);
	ON_CALL(*this, OpenFileMappingW(_, _, _)).WillByDefault(&_OpenFileMappingW);
	ON_CALL(*this, SetCurrentDirectoryW(_)).WillByDefault(&_SetCurrentDirectoryW);
	ON_CALL(*this, SizeofResource(_, _)).WillByDefault(&_SizeofResource);
	ON_CALL(*this, WaitForSingleObject(_, _)).WillByDefault(&_WaitForSingleObject);
}
