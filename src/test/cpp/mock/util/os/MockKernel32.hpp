/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/os/Kernel32.hpp"

#include <gmock/gmock.h>

struct MockKernel32 final : public Kernel32
{
	MOCK_CONST_METHOD6(CreateFileMappingW, HANDLE(
		_In_ HANDLE hFile,
		_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
		_In_ DWORD flProtect,
		_In_ DWORD dwMaximumSizeHigh,
		_In_ DWORD dwMaximumSizeLow,
		_In_opt_ LPCWSTR lpName
	));

	MOCK_CONST_METHOD7(CreateFileW, HANDLE(
		_In_ LPCWSTR lpFileName,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile
	));

	MOCK_CONST_METHOD3(CreateMutexW, HANDLE(
		_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
		_In_ BOOL bInitialOwner,
		_In_opt_ LPCWSTR lpName
	));

	MOCK_CONST_METHOD2(FindFirstFileW, HANDLE(
		_In_ LPCWSTR lpFileName,
		_Out_ LPWIN32_FIND_DATAW lpFindFileData
	));

	MOCK_CONST_METHOD2(FindNextFileW, BOOL(
		_In_ HANDLE hFindFile,
		_Out_ LPWIN32_FIND_DATAW lpFindFileData
	));

	// FindResourceWは引数型が特殊なので
	// 代替実装「FindResourceG」を用意している。
	// モック設定は G を使って行うこと。
	HRSRC FindResourceW(
		_In_opt_ HMODULE hModule,
		_In_ LPCWSTR lpName,
		_In_ LPCWSTR lpType
	) const override
	{
		// FindResourceWの第2、第3引数には NUL終端されない LPCWSTR が渡る。
		// 普通にモック化すると GMock + MSVC がエラーを吐いてしまうので対策する。
		return FindResourceG(
			hModule,
			lpName,
			lpType
		);
	}

	MOCK_CONST_METHOD2(GetCurrentDirectoryW, DWORD(
		_In_ DWORD nBufferLength,
		_Out_writes_to_opt_(nBufferLength,return + 1)
		LPWSTR lpBuffer
	));

	MOCK_CONST_METHOD4(GetFileTime, BOOL(
		_In_ HANDLE hFile,
		_Out_opt_ LPFILETIME lpCreationTime,
		_Out_opt_ LPFILETIME lpLastAccessTime,
		_Out_opt_ LPFILETIME lpLastWriteTime
	));

	MOCK_CONST_METHOD3(GetModuleFileNameW, DWORD(
		_In_opt_ HMODULE hModule,
		_Out_writes_to_(nSize, ((return < nSize) ? (return +1) : nSize))
		LPWSTR lpFilename,
		_In_ DWORD nSize
	));

	MOCK_CONST_METHOD2(GetProcAddress, FARPROC(
		_In_ HMODULE hModule,
		_In_ LPCSTR lpProcName
	));

	MOCK_CONST_METHOD2(GetSystemDirectoryW, UINT(
		_Out_writes_to_opt_(uSize, return +1)
		LPWSTR lpBuffer,
		_In_ UINT uSize
	));

	MOCK_CONST_METHOD3(LoadLibraryExW, HMODULE(
		_In_ LPCWSTR lpLibFileName,
		_Reserved_ HANDLE hFile,
		_In_ DWORD dwFlags
	));

	MOCK_CONST_METHOD2(LoadResource, HGLOBAL(
		_In_opt_ HMODULE hModule,
		_In_ HRSRC hResInfo
	));

	MOCK_CONST_METHOD1(LockResource, LPVOID(
		_In_ HGLOBAL hResData
	));

	MOCK_CONST_METHOD5(MapViewOfFile, LPVOID(
		_In_ HANDLE hFileMappingObject,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwFileOffsetHigh,
		_In_ DWORD dwFileOffsetLow,
		_In_ SIZE_T dwNumberOfBytesToMap
	));

	MOCK_CONST_METHOD3(OpenFileMappingW, HANDLE(
		_In_ DWORD dwDesiredAccess,
		_In_ BOOL bInheritHandle,
		_In_ LPCWSTR lpName
	));

	MOCK_CONST_METHOD6(ReplaceFileW, BOOL(
		_In_       LPCWSTR lpReplacedFileName,
		_In_       LPCWSTR lpReplacementFileName,
		_In_opt_   LPCWSTR lpBackupFileName,
		_In_       DWORD    dwReplaceFlags,
		_Reserved_ LPVOID   lpExclude,
		_Reserved_ LPVOID  lpReserved
	));

	MOCK_CONST_METHOD1(SetCurrentDirectoryW, BOOL(
		_In_ LPCWSTR lpPathName
	));

	MOCK_CONST_METHOD4(SetFileTime, BOOL(
		_In_ HANDLE hFile,
		_In_opt_ CONST FILETIME* lpCreationTime,
		_In_opt_ CONST FILETIME* lpLastAccessTime,
		_In_opt_ CONST FILETIME* lpLastWriteTime
	));

	MOCK_CONST_METHOD2(SizeofResource, DWORD(
		_In_opt_ HMODULE hModule,
		_In_ HRSRC hResInfo
	));

	static HANDLE _CreateFileW(
		_In_ LPCWSTR lpFileName,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile
	);

	static HANDLE _CreateMutexW(
		_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
		_In_ BOOL bInitialOwner,
		_In_opt_ LPCWSTR lpName
	);

	static HANDLE _FindFirstFileW(
		_In_ LPCWSTR lpFileName,
		_Out_ LPWIN32_FIND_DATAW lpFindFileData
	);

	static BOOL _FindNextFileW(
		_In_ HANDLE hFindFile,
		_Out_ LPWIN32_FIND_DATAW lpFindFileData
	);

	static HRSRC _FindResourceG(
		_In_opt_ HMODULE hModule,
		_In_ LPCVOID lpName,
		_In_ LPCVOID lpType
	);

	static DWORD _GetModuleFileNameW(
		_In_opt_ HMODULE hModule,
		_Out_writes_to_( nSize, ((return < nSize) ? (return +1) : nSize) )
		LPWSTR lpFilename,
		_In_ DWORD nSize
	);

	static FARPROC _GetProcAddress(
		_In_ HMODULE hModule,
		_In_ LPCSTR lpProcName
	);

	static HMODULE _LoadLibraryExW(
		_In_ LPCWSTR lpLibFileName,
		_Reserved_ HANDLE hFile,
		_In_ DWORD dwFlags
	);

	static HGLOBAL _LoadResource(
		_In_opt_ HMODULE hModule,
		_In_ HRSRC hResInfo
	);

	static LPVOID _LockResource(
		_In_ HGLOBAL hResData
	);

	static DWORD _SizeofResource(
		_In_opt_ HMODULE hModule,
		_In_ HRSRC hResInfo
	);

	MockKernel32();

	MOCK_METHOD(
		HRSRC,
		FindResourceG,
		(HMODULE hModule, const void* lpName, const void* lpType),
		(const)
	);
};
