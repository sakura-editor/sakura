/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

#include "cxx/first_derived.hpp"
#include "cxx/com_pointer.hpp"

#include <concepts>
#include <memory>
#include <utility>

namespace cxx {

/*!
 * TComImpl - COM(IUnknown)実装テンプレート
 *
 * COM実装に必須のメモリ管理機構をテンプレート化したものです。
 *
 * オブジェクトを参照するプロセスは、使用前にAddRefします。
 * オブジェクトを参照するプロセスは、使用後にReleaseします。
 */
template <typename TargetInterface, typename... SubInterfaces>
class TComImpl : public TargetInterface, public SubInterfaces...
{
private:
	using Base = TargetInterface;
	using Me = TComImpl<Base, SubInterfaces...>;

	using ThisHolder = std::unique_ptr<Me>;

	ThisHolder  _This		= nullptr;
	LONG		_RefCount	= 0;

public:
	using com_pointer_type = com_pointer<TargetInterface>;

	/*!
	 * C++スマートポインター を COMスマートポインター に変換します。
	 */
	template <typename T>
	static com_pointer_type to_com_pointer(
		std::unique_ptr<T> sp
	)
	{
		auto p   = sp.get();
		p->_This = std::move(sp);
		return com_pointer_type(p);
	}

protected:
	/*
	 * COM 実装クラスの生成ヘルパ
	 *	- Derived はこの TComImpl から派生していること
	 *	- Args... で Derived を構築できること
	 */
	template <typename Derived, typename... Args>
	static com_pointer_type make_instance(Args&&... args)
		requires (std::derived_from<Derived, Me> && std::constructible_from<Derived, Args...>)
	{
		return to_com_pointer(std::make_unique<Derived>(std::forward<Args>(args)...));
	}

	TComImpl() = default;

public:
	TComImpl(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	TComImpl(Me&&) noexcept = default;
	Me& operator=(Me&&) noexcept = default;

	virtual ~TComImpl() = default;

#pragma region Unknown
	/*!
	 * 参照カウンターをインクリメントします。
	 *
	 * @returns 参照カウント
	 */
	IFACEMETHODIMP_(ULONG) AddRef() override
	{
		return ::InterlockedIncrement(&_RefCount);
	}

	/*!
	 * インターフェースポインターを照会します。
	 *
	 * @param riid インターフェースのIID。
	 * @param ppvObject ポインターを受け取るポインター変数。
	 */
	IFACEMETHODIMP QueryInterface(
		REFIID			riid,
		_Out_ LPVOID*	ppvObject
	) override
	{
		if (!ppvObject) return E_POINTER;

		*ppvObject = nullptr;

		return _QueryInterfaces<TargetInterface, SubInterfaces...>(riid, std::bit_cast<LPUNKNOWN*>(ppvObject));
	}

	/*!
	 * 参照カウンターをデクリメントします。
	 * 
	 * 参照カンターがゼロになった場合、オブジェクトは破棄されます。
	 *
	 * @returns 参照カウント
	 */
	IFACEMETHODIMP_(ULONG) Release() override
	{
		const auto nRefCount = ::InterlockedDecrement(&_RefCount);

		// カウントがゼロになったらオブジェクトを解放する
		if (0 == nRefCount) {
			ThisHolder pThis;
			pThis.swap(_This);
			pThis.reset();
		}

		return nRefCount;
	}

#pragma endregion

private:
	/*!
	 * インターフェースポインターを照会します。
	 *
	 * @param riid インターフェースのIID。
	 * @param ppvObject ポインターを受け取るポインター変数。
	 */
	template<typename Head, typename... Tail>
	HRESULT _QueryInterfaces(
		REFIID		riid,
		LPUNKNOWN*	ppvObject
	) noexcept
	{
		auto condition = __uuidof(Head) == riid;

		// 先頭クラスのみ追加のIIDを検証する
		if constexpr (std::is_same_v<Head, TargetInterface>)
		{
			// ターゲットがIDispatch派生の場合、IDispatchも受け入れる
			if constexpr (std::is_base_of_v<IDispatch, TargetInterface>)
			{
				condition |= __uuidof(IDispatch) == riid;
			}

			// ターゲットは常にIUnknown派生なので、IUnknownも受け入れる
			condition |= __uuidof(IUnknown) == riid;
		}

		if (condition) {
			using HeadOrDerived = first_derived_t<Head, TargetInterface, SubInterfaces...>;

			*ppvObject = static_cast<HeadOrDerived*>(this);

			static_cast<HeadOrDerived*>(*ppvObject)->AddRef();

			return S_OK;
		}

		if constexpr (0 < sizeof...(Tail))
		{
			return _QueryInterfaces<Tail...>(riid, ppvObject);
		}

		if constexpr (0 == sizeof...(Tail))
		{
			return E_NOINTERFACE;
		}
	}
};

} // namespace cxx
