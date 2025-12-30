/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

namespace cxx {

//! ラムダ式の引数を解析するためのメタタイプ
template<typename T>
struct args_of_lambda;

//! ラムダ式のoperator()を解析するためのメタタイプ
template<typename R, typename C, typename... Args>
struct args_of_lambda<R (C::*)(Args...) const>
{
	using return_type = R;
	using types = std::tuple<Args...>;
};

//! ラムダ式のoperator()を解析するためのメタタイプ
template<typename R, typename C, typename... Args>
struct args_of_lambda<R (C::*)(Args...)>
{
	using return_type = R;
	using types = std::tuple<Args...>;
};

//! ラムダ式（クロージャオブジェクト）向けのヘルパー
template<typename Lambda>
struct lambda_traits
{
	using call_type = decltype(&Lambda::operator());
	using return_type = typename args_of_lambda<call_type>::return_type;
	using types = typename args_of_lambda<call_type>::types;
};

//! N番目のラムダ式引数の型を取得するメタ関数
template<size_t N, typename Lambda>
using type_of_Nth_lambda_arg = std::tuple_element_t<N, typename lambda_traits<Lambda>::types>;

} // namespace cxx
