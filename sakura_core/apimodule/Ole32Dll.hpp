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
 * Ole32.DLLへの依存を抽象化するクラス
 *
 * Win32 API呼出のテストを可能にする
 *
 * * 必要な定義を Windows.h からコピペする
 * * （定義順はアルファベット順、意味的なまとまりは考慮しない。）
 * * 関数定義を virtual const に変える
 * * 関数本体でグローバルの同名関数に処理を委譲させる
 */
struct Ole32Dll
{
	virtual ~Ole32Dll() = default;

	virtual HRESULT CoCreateInstance(
		_In_ REFCLSID rclsid,
		_In_opt_ LPUNKNOWN pUnkOuter,
		_In_ DWORD dwClsContext,
		_In_ REFIID riid,
		_COM_Outptr_ _At_(*ppv, _Post_readable_size_(_Inexpressible_(varies))) LPVOID  FAR* ppv
	) const
	{
		return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	}
};

class Ole32DllClient
{
	std::shared_ptr<Ole32Dll> _Ole32Dll;

public:
	explicit Ole32DllClient(std::shared_ptr<Ole32Dll> Ole32Dll_)
		: _Ole32Dll(std::move(Ole32Dll_))
	{
	}
	virtual ~Ole32DllClient() = default;

protected:
	std::shared_ptr<Ole32Dll> GetOle32Dll() const noexcept
	{
		return _Ole32Dll;
	}
};
