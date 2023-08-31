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

#include <functional>

namespace apiwrap {

/*!
 * HGLOBAL型のスマートポインタを実現するためのdeleterクラス
 */
struct HGlobalFreeFunc : private Kernel32DllClient
{
	explicit HGlobalFreeFunc(std::shared_ptr<Kernel32Dll> Kernel32Dll_) noexcept
		: Kernel32DllClient(std::move(Kernel32Dll_))
	{
	}

	void operator()( HGLOBAL hMem ) const
	{
		GlobalFree( hMem );
	}

	HGLOBAL GlobalFree(
		_Frees_ptr_opt_ HGLOBAL hMem) const
	{
		return GetKernel32Dll()->GlobalFree(hMem);
	}
};

//! HGLOBAL型のスマートポインタ
using HGlobalHolder = std::unique_ptr<std::remove_pointer_t<HGLOBAL>, HGlobalFreeFunc>;

/*!
 * グローバルメモリクラス
 * 
 * Win32 APIによるグローバルメモリ操作をC++から行えるようにするためのクラス。
 */
class HGlobal : private Kernel32DllClient
{
private:
	using Me = HGlobal;

	HGlobalHolder _Holder;

public:
	explicit HGlobal(HGLOBAL hGlobal_, std::shared_ptr<Kernel32Dll> Kernel32Dll_ = std::make_shared<Kernel32Dll>()) noexcept;
	explicit HGlobal(UINT flags, SIZE_T bytes, std::shared_ptr<Kernel32Dll> Kernel32Dll_ = std::make_shared<Kernel32Dll>()) noexcept;

	HGlobal(const Me&)       = delete;
	Me& operator=(const Me&) = delete;

	HGLOBAL Get() const noexcept { return _Holder.get(); }

	explicit operator HGLOBAL() const noexcept { return Get(); }

	template <typename TElem>
	HGLOBAL LockToWrite(std::function<void(TElem*)> func) const
	{
		if (const auto hMem = _Holder.get())
		{
			if (auto pLocked = GlobalLock<TElem*>(hMem))
			{
				func(pLocked);

				GlobalUnlock(hMem);

				return hMem;
			}
		}

		return nullptr;
	}

	template <typename TElemPtr>
	HGLOBAL LockToRead(std::function<void(const TElemPtr, size_t)> func) const
	{
		if (const auto hMem = _Holder.get())
		{
			if (const auto pLocked = GlobalLock<TElemPtr>(hMem))
			{
				const auto cbData = GlobalSize(hMem);

				func(pLocked, cbData);

				GlobalUnlock(hMem);

				return hMem;
			}
		}

		return nullptr;
	}

protected:
	HGLOBAL GlobalAlloc(
		_In_ UINT uFlags,
		_In_ SIZE_T dwBytes) const
	{
		return GetKernel32Dll()->GlobalAlloc(uFlags, dwBytes);
	}

	template<typename TElemPtr>
	TElemPtr GlobalLock(
		_In_ HGLOBAL hMem) const
	{
		return static_cast<std::remove_pointer_t<TElemPtr>*>(GetKernel32Dll()->GlobalLock(hMem));
	}

	SIZE_T GlobalSize(
		_In_ HGLOBAL hMem) const
	{
		return GetKernel32Dll()->GlobalSize(hMem);
	}

	BOOL GlobalUnlock(
		_In_ HGLOBAL hMem) const
	{
		return GetKernel32Dll()->GlobalUnlock(hMem);
	}
};

} //end of namespace apiwrap
