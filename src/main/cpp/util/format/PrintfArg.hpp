/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "basis/primitive.h"

#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>

namespace cxx {

/*!
 * @brief CStrictIntegerっぽい型
 */
template<typename A>
concept StrictIntegerLike =
	// int から例外なく構築できること
	(std::is_nothrow_constructible_v<std::remove_cvref_t<A>, int>)
	// GetValue() メソッドを持つこと
	&& (requires(const A& value) {
		{
			value.GetValue()
		} noexcept -> std::same_as<int>;
	})
	// Int に例外なく変換できること
	&& (requires(const A& value) {
		{
			static_cast<Int>(value)
		} noexcept -> std::same_as<Int>;
	})
	;

/*!
 * @brief Cランタイム printf系関数の引数を扱う関数群
 * 
 * C言語のprintf系関数はC++標準の文字列を渡せない。
 * この実装は2段階の変換を行うことにより、C++標準の文字列をprintf系関数に渡せるようにする。
 * 
 * 1段階目： 文字列引数を文字列オブジェクトに変換する。
 * 2段階目： 文字列オブジェクトを文字列ポインタに変換する。
 *
 * NUL終端のために一時バッファが必要な場合は、1段階目の戻り値に文字列を所有させる。
 * 文字数の計算と書き込みで引数を再利用するときは、この戻り値を保持し、
 * 各CRT呼び出しの直前に2段階目の変換を行う。
 * 既存のNUL終端文字列は参照するだけなので、元の文字列も書き込み完了まで有効である必要がある。
 */
struct PrintfArg final
{
	/*!
	 * @brief sprintf系関数の文字列引数を変換する（その１）
	 *
	 * 文字列引数→文字列の変換を行う。
	 */
	template <typename CharT>
	static constexpr decltype(auto) _ConvertStringArgPhase1(
		const cxx::NullTerminatedString<CharT>& text
	)
	{
		using Result = std::variant<std::nullptr_t, std::basic_string_view<CharT>, std::basic_string<CharT>>;
		if (text.uses_buffer()) {
			// textの破棄後も使えるよう、内部バッファの内容を戻り値に所有させる。
			return Result{ std::in_place_type<std::basic_string<CharT>>, std::basic_string<CharT>(text.str()) };
		}
		if (!text.c_str()) {
			return Result{ std::in_place_type<std::nullptr_t>, nullptr };
		}
		return Result{ std::in_place_type<std::basic_string_view<CharT>>, text.str() };
	}

	/*!
	 * @brief sprintf系関数の引数を変換する（その１）
	 *
	 * sprintf系関数にC++標準の文字列を渡せない対策として作成。
	 * 文字列はNULL・非所有の参照・所有する文字列のいずれかを保持するvariantに変換する。
	 * 文字列以外は参照のまま返し、StrictIntegerの整数化は2段階目で行う。
	 */
	template <typename T>
	static constexpr decltype(auto) ConvertArgPhase1(const T& value)
	{
		if constexpr (basis::NullTerminatedStringConstructible<T, WCHAR>) {
			const auto text = cxx::NullTerminatedString<WCHAR>{ value };
			return _ConvertStringArgPhase1(text);
		}
		else if constexpr (basis::NullTerminatedStringConstructible<T, ACHAR>) {
			const auto text = cxx::NullTerminatedString<ACHAR>{ value };
			return _ConvertStringArgPhase1(text);
		}
		else {
			return value;
		}
	}

	/*!
	 * @brief sprintf系関数の文字列引数を変換する（その２）
	 *
	 * 文字列→NUL終端文字列の変換を行う。
	 * string_viewは1段階目でNUL終端を確認した文字列への参照であることを前提とする。
	 */
	template <typename CharT, typename T>
	static constexpr const CharT* _ConvertStringArgPhase2(const T& text)
	{
		if constexpr (std::same_as<T, std::nullptr_t>) {
			return nullptr;
		}
		else if constexpr (std::same_as<T, std::basic_string<CharT>>) {
			return text.c_str();
		}
		else {
			return text.data();
		}
	}

	/*!
	 * @brief sprintf系関数の引数を変換する（その２）
	 *
	 * sprintf系関数にC++標準の文字列を渡せない対策として作成。
	 * @warning 戻り値の文字列ポインタは所有権を持たない。
	 * 1段階目の戻り値と、その参照先をCRT呼び出し完了まで保持すること。
	 * 1段階目の戻り値を移動すると、所有する文字列へのポインタが無効になる場合がある。
	 */
	template <typename T>
	static constexpr decltype(auto) ConvertArgPhase2(const T& value)
	{
		using ResultW = std::variant<std::nullptr_t, std::wstring_view, std::wstring>;
		using ResultA = std::variant<std::nullptr_t, std::string_view, std::string>;
		if constexpr (std::same_as<std::remove_cvref_t<T>, ResultW>) {
			return std::visit([](const auto& text) -> const WCHAR* {
				return _ConvertStringArgPhase2<WCHAR>(text);
			}, value);
		}
		else if constexpr (std::same_as<std::remove_cvref_t<T>, ResultA>) {
			return std::visit([](const auto& text) -> const ACHAR* {
				return _ConvertStringArgPhase2<ACHAR>(text);
			}, value);
		}
		else if constexpr (cxx::StrictIntegerLike<T>) {
			return static_cast<int>(value);
		}
		else {
			return value;
		}
	}
};

}	// namespace cxx
