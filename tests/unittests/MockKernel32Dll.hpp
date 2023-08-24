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

#include "apimodule/Kernel32Dll.hpp"

#include <gmock/gmock.h>

struct MockKernel32Dll : public Kernel32Dll
{
	MOCK_CONST_METHOD1(CloseHandle, BOOL(
		_In_ _Post_ptr_invalid_ HANDLE hObject
	));

	MOCK_CONST_METHOD6(CreateFileMappingW, HANDLE(
		_In_     HANDLE hFile,
		_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
		_In_     DWORD flProtect,
		_In_     DWORD dwMaximumSizeHigh,
		_In_     DWORD dwMaximumSizeLow,
		_In_opt_ LPCWSTR lpName
	));

	MOCK_CONST_METHOD0(GetLastError, DWORD(
		VOID
	));

	MOCK_CONST_METHOD5(MapViewOfFile, LPVOID(
		_In_ HANDLE hFileMappingObject,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwFileOffsetHigh,
		_In_ DWORD dwFileOffsetLow,
		_In_ SIZE_T dwNumberOfBytesToMap
	));

	MOCK_CONST_METHOD1(UnmapViewOfFile, BOOL(
		_In_ LPCVOID lpBaseAddress
	));
};
