/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

namespace cxx {

//! 関数/メソッド引数を解析するためのメタタイプ
template<typename T>
struct args_of_function;

//! API関数ポインタの引数を解析するためのメタタイプ
template<typename R, typename... Args>
struct args_of_function<R(WINAPI *)(Args...)>
{
	using types = std::tuple<Args...>;
};

//! COMインターフェースメソッド引数を解析するためのメタタイプ
template<typename R, typename C, typename... Args>
struct args_of_function<R (STDMETHODCALLTYPE C::*)(Args...)>
{
	using types = std::tuple<Args...>;
};

//! COMインターフェースメソッド引数を数える関数
template<typename FuncDecl>
inline constexpr size_t size_of_args = std::tuple_size_v<typename args_of_function<FuncDecl>::types>;

//! N番目のAPI関数/COMインターフェースメソッド引数の型を取得するメタ関数
template<size_t N, typename FuncDecl>
using type_of_Nth_arg = std::tuple_element_t<N, typename args_of_function<FuncDecl>::types>;

template<typename FuncDecl>
using type_of_first_arg = type_of_Nth_arg<0, FuncDecl>;

template<typename FuncDecl>
using type_of_last_arg = type_of_Nth_arg<size_of_args<FuncDecl> - 1, FuncDecl>;

} // namespace cxx
