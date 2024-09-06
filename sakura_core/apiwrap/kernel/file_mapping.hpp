/*! @file */
/*
	Copyright (C) 2024, Sakura Editor Organization

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

#include "apiwrap/kernel/handle_closer.hpp"
#include "apiwrap/kernel/message_error.hpp"

template<typename T>
struct view_of_file_unmapper {
	void operator ()(const T* p) const
	{
		UnmapViewOfFile(p);
	}
};

template<typename T>
using viewOfFileHolder = std::unique_ptr<T, view_of_file_unmapper<T>>;

template<typename T>
struct file_mapping
{
	handleHolder        m_hFileMap = handleHolder(nullptr, handle_closer());
	viewOfFileHolder<T> m_pData    = viewOfFileHolder<T>(nullptr, view_of_file_unmapper<T>());
	bool                m_bCreated = false;

	virtual ~file_mapping() = default;

	bool CreateMapping(
		_In_     HANDLE hFile,
		_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
		_In_     DWORD flProtect,
		_In_     DWORD dwMaximumSizeHigh,
		_In_     DWORD dwMaximumSizeLow,
		std::wstring_view name
	)
	{
		const auto hFileMap = CreateFileMappingW(
			hFile,
			lpFileMappingAttributes,
			flProtect,
			dwMaximumSizeHigh,
			dwMaximumSizeLow,
			name
		);

		m_hFileMap.reset(hFileMap);
		if (!hFileMap)
		{
			throw message_error( L"CreateFileMapping()に失敗しました" );
		}

		const auto lastError = GetLastError();

		m_bCreated = ERROR_ALREADY_EXISTS != lastError;

		const auto pData = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		m_pData.reset(pData);

		return m_bCreated;
	}

	virtual HANDLE CreateFileMappingW(
		_In_     HANDLE hFile,
		_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
		_In_     DWORD flProtect,
		_In_     DWORD dwMaximumSizeHigh,
		_In_     DWORD dwMaximumSizeLow,
		std::wstring_view name
	) const
	{
		return ::CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, name.data());
	}
	virtual DWORD GetLastError() const
	{
		return ::GetLastError();
	}
	virtual T* MapViewOfFile(
		_In_ HANDLE hFileMappingObject,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwFileOffsetHigh,
		_In_ DWORD dwFileOffsetLow,
		_In_ SIZE_T dwNumberOfBytesToMap
	) const
	{
		return (T*)::MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
	}
#if 0
	virtual HANDLE OpenFileMappingW(
		_In_ DWORD dwDesiredAccess,
		_In_ BOOL bInheritHandle,
		_In_ std::wstring_view name
	) const
	{
		return ::OpenFileMappingW(dwDesiredAccess, bInheritHandle, name.data());
	}
#endif
};
