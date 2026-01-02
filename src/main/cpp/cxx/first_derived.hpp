/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

namespace cxx {

//! 型リストTypesからT派生型を抽出するフィルター
template<typename T, typename U>
struct filter_derived
{
	using type = std::conditional_t<std::is_base_of_v<T, U>, std::tuple<U>, std::tuple<>>;
};

//! 型リストTypesから最初のT派生型を抽出するメタクラス
template<typename T, typename... Types>
struct first_derived
{
	using type = std::tuple_element_t<0, decltype(std::tuple_cat(std::declval<typename filter_derived<T, Types>::type>()...))>;
};

//! 型リストTypesから最初のT派生型を抽出するメタ関数
template<typename T, typename... Types>
using first_derived_t = typename first_derived<T, Types...>::type;

} // namespace cxx
