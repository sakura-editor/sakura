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

#include "apimodule/User32Dll.hpp"

#include <CommCtrl.h>
#include <windowsx.h>

#include <string>
#include <string_view>

namespace apiwrap {

bool         SetWindowTextW(_In_ HWND hWnd, std::wstring_view text, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());
bool         SetWindowTextW(_In_ HWND hWnd, _In_opt_ LPCWSTR psz, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());
std::wstring GetWindowTextW(_In_ HWND hWnd, std::wstring&& buffer, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());
std::wstring GetWindowTextW(_In_ HWND hWnd, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());

bool         SetDlgItemTextW(_In_ HWND hDlg, int nIDDlgItem, std::wstring_view text, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());
bool         SetDlgItemTextW(_In_ HWND hDlg, int nIDDlgItem, _In_opt_ LPCWSTR psz, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());
std::wstring GetDlgItemTextW(_In_ HWND hDlg, int nIDDlgItem, std::wstring&& buffer, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());
std::wstring GetDlgItemTextW(_In_ HWND hDlg, int nIDDlgItem, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());

void         SendEmLimitTextW(_In_ HWND hEdit, size_t cchLimit, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());
void         SendEmLimitTextW(_In_ HWND hDlg, int nIDDlgItem, size_t cchLimit, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());

bool         IsWndClassRegistered(std::wstring_view className, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());

template<typename TResource>
auto CopyResource(HINSTANCE hLangRsrcInstance, LPCWSTR lpName, LPCWSTR lpType, std::shared_ptr<User32Dll> _User32Dll)
{
	using BinarySequence = std::basic_string<std::byte>;

	const auto hResInfo = _User32Dll->FindResourceW(hLangRsrcInstance, lpName, lpType);
	if (!hResInfo) return BinarySequence();

	const auto hResData = _User32Dll->LoadResource(hLangRsrcInstance, hResInfo);
	if (!hResData) return BinarySequence();

	const auto pDlgTemplate = std::bit_cast<TResource>(_User32Dll->LockResource(hResData));
	if (!pDlgTemplate) return BinarySequence();

	const auto dwDlgTemplateSize = _User32Dll->SizeofResource(hLangRsrcInstance, hResInfo);

	auto buffer = std::basic_string<std::byte>(dwDlgTemplateSize, std::byte());
	auto lpDlgTemplate = std::bit_cast<TResource>(buffer.data());

	::memcpy_s(lpDlgTemplate, dwDlgTemplateSize, pDlgTemplate, dwDlgTemplateSize);

	return buffer;
}

} //end of namespace apiwrap

constexpr HBRUSH MakeHBrush(int sysColorIndex)
{
	return std::bit_cast<HBRUSH>(static_cast<size_t>(sysColorIndex + 1));
}

inline HCURSOR LoadSysCursor(_In_ LPCWSTR lpCursorName)
{
	return ::LoadCursorW(NULL, lpCursorName);
}

inline HICON LoadSysIcon(_In_ LPCWSTR lpIconName)
{
	return ::LoadIconW(NULL, lpIconName);
}
