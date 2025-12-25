/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

#include "cxx/type_of_Nth_arg.hpp"

namespace cxx {

/*!
 * リソースハンドルを保持するスマートポインタ
 */
template<auto Deleter>
struct ResourceHolder
{
	using FuncDecl = decltype(Deleter);
	static_assert(size_of_args<FuncDecl> >= 1, "Deleter must have at least 1 argument.");

	// resource は「最後の引数」
	using resource_type = type_of_last_arg<FuncDecl>;
	static_assert(std::is_pointer_v<resource_type>, "Resource must be a pointer type.");

	// bound は「最後以外」。今回の用途的には 0 or 1 が主だが、一般化して tuple で持つ。
	static constexpr size_t bound_count = size_of_args<FuncDecl> -1;

	template<size_t... I>
	static auto make_bound_types( std::index_sequence<I...> ) -> std::tuple<type_of_Nth_arg<I, FuncDecl>...>;

	using bound_tuple_t = decltype(make_bound_types( std::make_index_sequence<bound_count>{} ));

	struct Releaser
	{
		bound_tuple_t bound{};

		void operator()(resource_type p) const noexcept
		{
			if (!p) return;

			// Deleter(bound..., p)
			auto deleter = Deleter;
			std::apply([p, deleter](auto const&... xs) noexcept { std::invoke(deleter, xs..., p); }, bound);
		}
	};

	using holder_type = std::unique_ptr<std::remove_pointer_t<resource_type>, Releaser>;
	holder_type m_Holder;

	using Me = ResourceHolder<Deleter>;

	using pointer = resource_type;
	using element_type = std::remove_pointer_t<pointer>;

	/* implicit */ ResourceHolder(resource_type t) requires (0 == bound_count)
		: m_Holder(t, Releaser{})
	{
	}

   // bound 引数あり：束縛して構築
    template<typename... Bound>
    explicit ResourceHolder(Bound&&... bound) noexcept
        requires (sizeof...(Bound) == bound_count) //  && !(1 == sizeof...(Bound) && std::same_as_v<std::remove_cvref_t<type_of_first_arg>, Me>)
        : m_Holder(nullptr, Releaser{ bound_tuple_t{ std::forward<Bound>(bound)... } })
    {
    }

	// bound 更新（OwnedResourceHolder の updateContext 相当）
	template<typename... Bound>
	void updateBound(Bound&&... bound) noexcept
		requires (sizeof...(Bound) == bound_count)
	{
		m_Holder.get_deleter().bound = bound_tuple_t{ std::forward<Bound>(bound)... };
	}

	pointer get() const noexcept { return m_Holder.get(); }
	pointer release() noexcept { return m_Holder.release(); }

	Me& operator = (resource_type t)
	{
		m_Holder.reset(t);
		return *this;
	}

	/* implicit */ operator resource_type() const noexcept { return get(); }
};

} // end of namespace cxx
