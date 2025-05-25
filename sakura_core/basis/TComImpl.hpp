/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

#include <Windows.h>

#include <comdef.h>
#include <comutil.h>
#include <oaidl.h>

#include <memory>
#include <type_traits>

/*!
 * TComImpl - COMオブジェクトの実装クラステンプレート
 * 使用上の注意:
 *   1. 生成はnewで行い、deleteはしないでください。
 *      自動変数で生成するとヒープエラーが出ます。
 *   2. 生成したらAddRef()し、不要になったらRelease()してください。
 */
template<class TargetInterface, class DeleterType = std::default_delete<TargetInterface>, std::enable_if_t<std::is_base_of_v<IUnknown, TargetInterface>, std::nullptr_t> = nullptr>
class TComImpl : public TargetInterface
{
	LONG nRefCount_ = 0;

	using Me = TComImpl<TargetInterface>;

public:
	TComImpl() = default;
	TComImpl(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	TComImpl(Me&&) = delete;
	Me& operator = (Me&&) = delete;

	IFACEMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) override
	{
		if (ppvObject == nullptr) {
			return E_POINTER;
		}

		if (IsEqualIID(iid, __uuidof(TargetInterface)) ||
			IsEqualIID(iid, IID_IUnknown))
		{
			*ppvObject = this;
			AddRef();
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	IFACEMETHODIMP_(ULONG) AddRef() override
	{
		return ::InterlockedIncrement(&nRefCount_);
	}

	IFACEMETHODIMP_(ULONG) Release() override
	{
		const LONG nRefCount = ::InterlockedDecrement(&nRefCount_);

		if (nRefCount == 0) {
			const DeleterType deletor; 
			deletor(this);
		}

		return nRefCount;
	}
};
