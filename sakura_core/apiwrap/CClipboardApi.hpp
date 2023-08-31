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
#include "apimodule/Shell32Dll.hpp"
#include "apimodule/User32Dll.hpp"

#include "apiwrap/HGlobal.hpp"

#include <functional>
#include <string>
#include <string_view>

namespace apiwrap {

/*!
 * クリップボードAPIクラス
 *
 * Win32 APIによるクリップボード操作をC++から行えるようにするためのクラス。
 */
class CClipboardApi : public User32DllClient, public Kernel32DllClient, public Shell32DllClient
{
public:
	static constexpr CLIPFORMAT CF_INVALID = -1;
	static constexpr CLIPFORMAT CF_NONE    =  0;

	static UINT GetClipboardFormatW(std::wstring_view name, std::shared_ptr<User32Dll> _User32Dll = std::make_shared<User32Dll>());

	explicit CClipboardApi(std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>(), std::shared_ptr<Kernel32Dll> Kernel32Dll_ = std::make_shared<Kernel32Dll>(), std::shared_ptr<Shell32Dll> Shell32Dll_ = std::make_shared<Shell32Dll>()) noexcept;

	bool   OpenClipboard(_In_opt_ HWND hWnd, int retryCount) const;

	template<typename TElemPtr>
	HANDLE SetClipboardData(UINT uFormat, size_t cbAlloc, std::function<void(TElemPtr)> func) const
	{
		if (const HGlobal mem(GMEM_MOVEABLE | GMEM_DDESHARE, cbAlloc, GetKernel32Dll());
			mem.LockToWrite(func))
		{
			return SetClipboardData(uFormat, static_cast<HGLOBAL>(mem));
		}

		return nullptr;
	}

	template<typename TElem>
	HANDLE SetClipboardData(UINT uFormat, const TElem* pData, size_t cchData, size_t cchAlloc) const
	{
		const auto cbAlloc = cchAlloc * sizeof(TElem);
		return SetClipboardData<TElem*>(uFormat, cbAlloc, [pData, cchData, cchAlloc, cbAlloc](TElem* pLocked)
			{
				const auto cbData = cchData * sizeof(TElem);
				memcpy_s(pLocked, cbAlloc, pData, cbData);

				std::fill(&pLocked[cchData], &pLocked[cchAlloc], TElem());
			});
	}

	template<typename TElemPtr>
	bool   GetClipboardData(UINT uFormat, std::function<void(const TElemPtr, size_t)> func) const
	{
		if (const HGlobal mem(GetClipboardData(uFormat), GetKernel32Dll());
			mem.LockToRead(func))
		{
			return true;
		}

		return false;
	}

	template<typename TElem>
	bool   GetClipboardString(UINT uFormat, std::basic_string<TElem>& buffer) const
	{
		return GetClipboardData<TElem*>(uFormat, [&buffer](const TElem* pData, size_t cbData)
			{
				// バイト数を文字数に変換する
				const auto cchData = cbData / sizeof(TElem);

				// データを文字として受け取る
				buffer.assign(pData, cchData);
			});
	}

protected:
	BOOL EmptyClipboard() const
	{
		return GetUser32Dll()->EmptyClipboard();
	}

	UINT EnumClipboardFormats(
		_In_ UINT format) const
	{
		return GetUser32Dll()->EnumClipboardFormats(format);
	}

	HANDLE GetClipboardData(
		_In_ UINT uFormat) const
	{
		return GetUser32Dll()->GetClipboardData(uFormat);
	}

	BOOL IsClipboardFormatAvailable(
		_In_ UINT format) const
	{
		return GetUser32Dll()->IsClipboardFormatAvailable(format);
	}

	HANDLE SetClipboardData(
		_In_ UINT uFormat,
		_In_opt_ HANDLE hMem) const
	{
		return GetUser32Dll()->SetClipboardData(uFormat, hMem);
	}

	UINT DragQueryFileW(
		_In_ HDROP hDrop,
		_In_ UINT iFile,
		_Out_writes_opt_(cch) LPWSTR lpszFile,
		_In_ UINT cch) const
	{
		return GetShell32Dll()->DragQueryFileW(hDrop, iFile, lpszFile, cch);
	}
};

} //end of namespace apiwrap
