/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/design_template.h"

#include <Windows.h>

 //! Kernel32.dll呼出をテスト可能にするDIっぽいもの
struct Kernel32 : public TSakuraSingleton<Kernel32>
{
	using Me = Kernel32;

	~Kernel32() override = default;

	virtual HANDLE CreateFileMappingW(
		_In_ HANDLE hFile,
		_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
		_In_ DWORD flProtect,
		_In_ DWORD dwMaximumSizeHigh,
		_In_ DWORD dwMaximumSizeLow,
		_In_opt_ LPCWSTR lpName
    ) const;

	virtual HANDLE CreateFileW(
		_In_ LPCWSTR lpFileName,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile
	) const;

	virtual HANDLE FindFirstFileW(
		_In_ LPCWSTR lpFileName,
		_Out_ LPWIN32_FIND_DATAW lpFindFileData
	) const;

	virtual BOOL FindNextFileW(
		_In_ HANDLE hFindFile,
		_Out_ LPWIN32_FIND_DATAW lpFindFileData
	) const;

	virtual HRSRC FindResourceW(
		_In_opt_ HMODULE hModule,
		_In_ LPCWSTR lpName,
		_In_ LPCWSTR lpType
	) const;

	virtual DWORD GetCurrentDirectoryW(
		_In_ DWORD nBufferLength,
		_Out_writes_to_opt_(nBufferLength,return + 1)
		LPWSTR lpBuffer
	) const;

	virtual BOOL GetFileTime(
		_In_ HANDLE hFile,
		_Out_opt_ LPFILETIME lpCreationTime,
		_Out_opt_ LPFILETIME lpLastAccessTime,
		_Out_opt_ LPFILETIME lpLastWriteTime
	) const;

	virtual DWORD GetModuleFileNameW(
		_In_opt_ HMODULE hModule,
		_Out_writes_to_( nSize, ((return < nSize) ? (return +1) : nSize) )
		LPWSTR lpFilename,
		_In_ DWORD nSize
	) const;

	virtual FARPROC GetProcAddress(
		_In_ HMODULE hModule,
		_In_ LPCSTR lpProcName
	) const;

	virtual UINT GetSystemDirectoryW(
		_Out_writes_to_opt_(uSize, return +1)
		LPWSTR lpBuffer,
		_In_ UINT uSize
	) const;

	virtual HMODULE LoadLibraryExW(
		_In_ LPCWSTR lpLibFileName,
		_Reserved_ HANDLE hFile,
		_In_ DWORD dwFlags
	) const;

	virtual HGLOBAL LoadResource(
		_In_opt_ HMODULE hModule,
		_In_ HRSRC hResInfo
	) const;

	virtual LPVOID LockResource(
		_In_ HGLOBAL hResData
	) const;

	virtual LPVOID MapViewOfFile(
		_In_ HANDLE hFileMappingObject,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwFileOffsetHigh,
		_In_ DWORD dwFileOffsetLow,
		_In_ SIZE_T dwNumberOfBytesToMap
	) const;

	virtual HANDLE OpenFileMappingW(
		_In_ DWORD dwDesiredAccess,
		_In_ BOOL bInheritHandle,
		_In_ LPCWSTR lpName
	) const;

	virtual BOOL ReplaceFileW(
		_In_       LPCWSTR lpReplacedFileName,
		_In_       LPCWSTR lpReplacementFileName,
		_In_opt_   LPCWSTR lpBackupFileName,
		_In_       DWORD    dwReplaceFlags,
		_Reserved_ LPVOID   lpExclude,
		_Reserved_ LPVOID  lpReserved
	) const;

	virtual BOOL SetCurrentDirectoryW(
		_In_ LPCWSTR lpPathName
	) const;

	virtual BOOL SetFileTime(
		_In_ HANDLE hFile,
		_In_opt_ CONST FILETIME* lpCreationTime,
		_In_opt_ CONST FILETIME* lpLastAccessTime,
		_In_opt_ CONST FILETIME* lpLastWriteTime
	) const;

	virtual DWORD SizeofResource(
		_In_opt_ HMODULE hModule,
		_In_ HRSRC hResInfo
	) const;
};
