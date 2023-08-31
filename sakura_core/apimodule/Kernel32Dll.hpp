/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
 */
#pragma once

#include <Windows.h>

#include <memory>
 
 /*!
 * Kernel32.DLLへの依存を抽象化するクラス
 *
 * Win32 API呼出のテストを可能にする
 *
 * * 必要な定義を Windows.h からコピペする
 * * （定義順はアルファベット順、意味的なまとまりは考慮しない。）
 * * 関数定義を virtual const に変える
 * * 関数本体でグローバルの同名関数に処理を委譲させる
 */
struct Kernel32Dll
{
	virtual ~Kernel32Dll() = default;

	virtual BOOL CloseHandle(
		_In_ _Post_ptr_invalid_ HANDLE hObject) const
	{
		return ::CloseHandle(hObject);
	}

	virtual HANDLE CreateFileMappingW(
		_In_     HANDLE hFile,
		_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
		_In_     DWORD flProtect,
		_In_     DWORD dwMaximumSizeHigh,
		_In_     DWORD dwMaximumSizeLow,
		_In_opt_ LPCWSTR lpName) const
	{
		return ::CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
	}

	virtual DWORD GetLastError(
		VOID) const
	{
		return ::GetLastError();
	}

	virtual HGLOBAL GlobalAlloc(
		_In_ UINT uFlags,
		_In_ SIZE_T dwBytes) const
	{
		return ::GlobalAlloc(uFlags, dwBytes);
	}

	virtual HGLOBAL GlobalFree(
		_Frees_ptr_opt_ HGLOBAL hMem) const
	{
		return ::GlobalFree(hMem);
	}

	virtual LPVOID GlobalLock(
		_In_ HGLOBAL hMem) const
	{
		return ::GlobalLock(hMem);
	}

	virtual SIZE_T GlobalSize(
		_In_ HGLOBAL hMem) const
	{
		return ::GlobalSize(hMem);
	}

	virtual BOOL GlobalUnlock(
		_In_ HGLOBAL hMem) const
	{
		return ::GlobalUnlock(hMem);
	}

	virtual LPVOID MapViewOfFile(
		_In_ HANDLE hFileMappingObject,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwFileOffsetHigh,
		_In_ DWORD dwFileOffsetLow,
		_In_ SIZE_T dwNumberOfBytesToMap) const
	{
		return ::MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
	}

	virtual VOID Sleep(
		_In_ DWORD dwMilliseconds) const
	{
		return ::Sleep(dwMilliseconds);
	}

	virtual BOOL UnmapViewOfFile(
		_In_ LPCVOID lpBaseAddress)const
	{
		return ::UnmapViewOfFile(lpBaseAddress);
	}
};

class Kernel32DllClient
{
	std::shared_ptr<Kernel32Dll> _Kernel32Dll;

public:
	explicit Kernel32DllClient(std::shared_ptr<Kernel32Dll> Kernel32Dll_)
		: _Kernel32Dll(std::move(Kernel32Dll_))
	{
	}
	virtual ~Kernel32DllClient() = default;

protected:
	std::shared_ptr<Kernel32Dll> GetKernel32Dll() const noexcept
	{
		return _Kernel32Dll;
	}
};
