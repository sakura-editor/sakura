/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/os/Kernel32.hpp"

HANDLE Kernel32::CreateFileMappingW(
	_In_ HANDLE hFile,
	_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
	_In_ DWORD flProtect,
	_In_ DWORD dwMaximumSizeHigh,
	_In_ DWORD dwMaximumSizeLow,
	_In_opt_ LPCWSTR lpName
) const
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

HANDLE Kernel32::CreateFileW(
	_In_ LPCWSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile
) const
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

HANDLE Kernel32::CreateMutexW(
	_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
	_In_ BOOL bInitialOwner,
	_In_opt_ LPCWSTR lpName
) const
{
	return ::CreateMutexW(
		lpMutexAttributes,
		bInitialOwner,
		lpName
	);
}

HANDLE Kernel32::FindFirstFileW(
	_In_ LPCWSTR lpFileName,
	_Out_ LPWIN32_FIND_DATAW lpFindFileData
) const
{
	return ::FindFirstFileW(
		lpFileName,
		lpFindFileData
	);
}

BOOL Kernel32::FindNextFileW(
	_In_ HANDLE hFindFile,
	_Out_ LPWIN32_FIND_DATAW lpFindFileData
) const
{
	return ::FindNextFileW(
		hFindFile,
		lpFindFileData
	);
}

HRSRC Kernel32::FindResourceW(
	_In_opt_ HMODULE hModule,
	_In_ LPCWSTR lpName,
	_In_ LPCWSTR lpType
) const
{
	return ::FindResourceW(
		hModule,
		lpName,
		lpType
	);
}

DWORD Kernel32::GetCurrentDirectoryW(
	_In_ DWORD nBufferLength,
	_Out_writes_to_opt_( nBufferLength, return +1 )
	LPWSTR lpBuffer
) const
{
	return ::GetCurrentDirectoryW(
		nBufferLength,
		lpBuffer
	);
}

BOOL Kernel32::GetFileTime(
	_In_ HANDLE hFile,
	_Out_opt_ LPFILETIME lpCreationTime,
	_Out_opt_ LPFILETIME lpLastAccessTime,
	_Out_opt_ LPFILETIME lpLastWriteTime
) const
{
	return ::GetFileTime(
		hFile,
		lpCreationTime,
		lpLastAccessTime,
		lpLastWriteTime
	);
}

DWORD Kernel32::GetModuleFileNameW(
	_In_opt_ HMODULE hModule,
	_Out_writes_to_( nSize, ((return < nSize) ? (return +1) : nSize) )
	LPWSTR lpFilename,
	_In_ DWORD nSize
) const
{
	return ::GetModuleFileNameW(
		hModule,
		lpFilename,
		nSize
	);
}

FARPROC Kernel32::GetProcAddress(
	_In_ HMODULE hModule,
	_In_ LPCSTR lpProcName
) const
{
	return ::GetProcAddress(
		hModule,
		lpProcName
	);
}

UINT Kernel32::GetSystemDirectoryW(
	_Out_writes_to_opt_( uSize, return +1 )
	LPWSTR lpBuffer,
	_In_ UINT uSize
) const
{
	return ::GetSystemDirectoryW(lpBuffer, uSize);
}

HMODULE Kernel32::LoadLibraryExW(
	_In_ LPCWSTR lpLibFileName,
	_Reserved_ HANDLE hFile,
	_In_ DWORD dwFlags
) const
{
	return ::LoadLibraryExW(
		lpLibFileName,
		hFile,
		dwFlags
	);
}

HGLOBAL Kernel32::LoadResource(
	_In_opt_ HMODULE hModule,
	_In_ HRSRC hResInfo
) const
{
	return ::LoadResource(
		hModule,
		hResInfo
	);
}

LPVOID Kernel32::LockResource(
	_In_ HGLOBAL hResData
) const
{
	return ::LockResource(hResData);
}

LPVOID Kernel32::MapViewOfFile(
	_In_ HANDLE hFileMappingObject,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwFileOffsetHigh,
	_In_ DWORD dwFileOffsetLow,
	_In_ SIZE_T dwNumberOfBytesToMap
) const
{
	return ::MapViewOfFile(
		hFileMappingObject,
		dwDesiredAccess,
		dwFileOffsetHigh,
		dwFileOffsetLow,
		dwNumberOfBytesToMap
	);
}

HANDLE Kernel32::OpenFileMappingW(
	_In_ DWORD dwDesiredAccess,
	_In_ BOOL bInheritHandle,
	_In_ LPCWSTR lpName
) const
{
	return ::OpenFileMappingW(
		dwDesiredAccess,
		bInheritHandle,
		lpName
	);
}

BOOL Kernel32::ReplaceFileW(
	_In_       LPCWSTR lpReplacedFileName,
	_In_       LPCWSTR lpReplacementFileName,
	_In_opt_   LPCWSTR lpBackupFileName,
	_In_       DWORD    dwReplaceFlags,
	_Reserved_ LPVOID   lpExclude,
	_Reserved_ LPVOID  lpReserved
) const
{
	return ::ReplaceFileW(
		lpReplacedFileName,
		lpReplacementFileName,
		lpBackupFileName,
		dwReplaceFlags,
		lpExclude,
		lpReserved
	);
}

BOOL Kernel32::SetCurrentDirectoryW(
	_In_ LPCWSTR lpPathName
) const
{
	return ::SetCurrentDirectoryW(lpPathName);
}


BOOL Kernel32::SetFileTime(
	_In_ HANDLE hFile,
	_In_opt_ CONST FILETIME* lpCreationTime,
	_In_opt_ CONST FILETIME* lpLastAccessTime,
	_In_opt_ CONST FILETIME* lpLastWriteTime
) const
{
	return ::SetFileTime(
		hFile,
		lpCreationTime,
		lpLastAccessTime,
		lpLastWriteTime
	);
}

DWORD Kernel32::SizeofResource(
	_In_opt_ HMODULE hModule,
	_In_ HRSRC hResInfo
) const
{
	return ::SizeofResource(
		hModule,
		hResInfo
	);
}

DWORD Kernel32::WaitForMultipleObjects(
	_In_ DWORD nCount,
	_In_reads_( nCount ) CONST HANDLE* lpHandles,
	_In_ BOOL bWaitAll,
	_In_ DWORD dwMilliseconds
) const
{
	return ::WaitForMultipleObjects(
		nCount,
		lpHandles,
		bWaitAll,
		dwMilliseconds
	);
}

DWORD Kernel32::WaitForSingleObject(
	_In_ HANDLE hHandle,
	_In_ DWORD dwMilliseconds
) const
{
	return ::WaitForSingleObject(
		hHandle,
		dwMilliseconds
	);
}
