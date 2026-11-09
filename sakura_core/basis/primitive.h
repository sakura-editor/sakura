/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_PRIMITIVE_C8059DE4_C986_492E_9C09_7F044049C481_H_
#define SAKURA_PRIMITIVE_C8059DE4_C986_492E_9C09_7F044049C481_H_
#pragma once

#include <Windows.h>
#include "config/build_config.h"

#include <concepts>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

// -- -- -- -- 文字 -- -- -- -- //

//char,wchar_t の変わりに、別名の ACHAR,WCHAR を使うと、ソース整形がしやすいケースがある。
typedef char ACHAR;

//EDIT_CHAR
typedef wchar_t WChar;      //エディタで用いるテキスト管理データ型
typedef wchar_t EDIT_CHAR;
#define _EDITL(A) LTEXT(A)

//文字コード別、文字型
typedef unsigned char	uchar_t;		//  unsigned char の別名．
typedef unsigned short	uchar16_t;		//  UTF-16 用．
typedef unsigned long	uchar32_t;		//  UTF-32 用．
typedef long			wchar32_t;

//文字列

namespace basis {

/*!
 * @brief NUL終端文字列と互換性のある型
 *
 * 対象型がc_strとlengthを提供する。
 * std::wstringなど。
 *
 *  c_str: NUL終端文字列を返す関数
 *  length: 文字列の長さを返す関数
 *
 * c_str()[length()] == '\0' が保証される。
 *
 * @tparam A 変換可能かを判定する型
 * @tparam CharT 文字型(WCHAR or CHAR)
 */
template<typename A, typename CharT>
concept NullTerminatedStringCompatible =
	(requires(const A & value) {
		{
			value.c_str()
		} noexcept -> std::convertible_to<const CharT*>;
	})
	&& (requires(const A & value) {
		{
			value.length()
		} noexcept -> std::convertible_to<size_t>;
	})
	;

/*!
 * @brief 文字列として扱える型
 *
 * 対象型をそのまま文字列として参照できる。
 *
 * @tparam A 変換可能かを判定する型
 * @tparam CharT 文字型(WCHAR or CHAR)
 */
template<typename A, typename CharT = WCHAR>
concept StringViewCompatible =
	// std::basic_string_view<CharT> に例外なく変換できること
	(requires(const A& value) {
		{
			static_cast<std::basic_string_view<CharT>>(value)
		} noexcept -> std::same_as<std::basic_string_view<CharT>>;
	})
	// 配列ではないこと
	&& (!std::is_array_v<std::remove_cvref_t<A>>)
	// ポインタではないこと
	&& (!std::is_pointer_v<std::remove_cvref_t<A>>)
	// nullptrではないこと
	&& (!std::same_as<std::remove_cvref_t<A>, std::nullptr_t>)
	;

/*!
 * @brief 文字列表現を生成できる型
 *
 * 対象型から文字列を生成できる。
 * std::filesystem::pathなど。
 *
 * @tparam A 生成可能かを判定する型
 * @tparam CharT 文字型(WCHAR or CHAR)
 */
template<typename A, typename CharT = WCHAR>
concept StringConstructible =
	// std::basic_string<CharT> を生成できる型であること
	(requires(const std::remove_cvref_t<A>& value) {
		static_cast<std::basic_string<CharT>>(value);
	})
	// std::basic_string_view<CharT> に例外なく変換できること
	&& (!StringViewCompatible<A, CharT>)
	// 配列ではないこと
	&& (!std::is_array_v<std::remove_cvref_t<A>>)
	// ポインタではないこと
	&& (!std::is_pointer_v<std::remove_cvref_t<A>>)
	// nullptrではないこと
	&& (!std::same_as<std::remove_cvref_t<A>, std::nullptr_t>)
	;

/*!
 * @brief NUL終端文字列を生成できる型
 *
 * 対象型からNUL終端文字列を生成できる。
 * cxx::NullTerminatedStringのコンストラクタに渡せる型。
 *
 * @tparam A 変換可能かを判定する型
 * @tparam CharT 文字型(WCHAR or CHAR)
 */
template <typename A, typename CharT>
concept NullTerminatedStringConstructible =
	// nullptrであること
	(std::same_as<std::remove_cvref_t<A>, std::nullptr_t>)
	// ポインタであること
	|| (
		std::is_pointer_v<std::remove_cvref_t<A>>
		&& std::convertible_to<std::remove_cvref_t<A>, const CharT*>
	)
	// 配列であること
	|| (
		std::is_array_v<std::remove_reference_t<A>>
		&& (
			std::same_as<std::remove_extent_t<std::remove_reference_t<A>>, CharT>
			|| std::same_as<std::remove_extent_t<std::remove_reference_t<A>>, const CharT>
		)
	)
	// 文字列ポインタと文字列長を取得できること
	|| NullTerminatedStringCompatible<A, CharT>
	// std::basic_string_view<CharT> に例外なく変換できること
	|| StringViewCompatible<A, CharT>
	// std::basic_string<CharT> に変換できること
	|| StringConstructible<A, CharT>
	;

/*!
 * @brief 固定長バッファとして扱える型
 *
 * 対象型をそのまま固定長バッファとして更新できる。
 * std::arrayやWCHAR(&)[N]など。
 *
 * 文字列の最終要素はNUL終端と仮定するため
 * std::wstirngは対象外にしている。
 *
 * @tparam A 変換可能かを判定する型
 * @tparam CharT 文字型(WCHAR or CHAR)
 */
template<typename A, typename CharT = WCHAR>
concept WritableBuffer =
	// std::basic_string<CharT> に変換できること
	(requires(A& value) {
		{
			std::span<CharT>(value)
		} -> std::same_as<std::span<CharT>>;
	})
	// std::basic_string<CharT>ではないこと
	&& (!std::same_as<std::remove_cvref_t<A>, std::basic_string<CharT>>)
	;

} // namespace basis

// -- -- -- -- その他 -- -- -- -- //

typedef char KEYCODE;

//int互換
#ifdef USE_STRICT_INT
	#include "CLaxInteger.h"
	typedef CLaxInteger Int;
#else
	typedef int Int;
#endif

namespace cxx {

/*!
 * @brief NUL終端文字列型
 *
 * Windows APIやCランタイムに渡せるC Stringを提供する。
 *
 * @tparam CharT 文字型(WCHAR or CHAR)
 */
template <typename CharT = WCHAR>
class NullTerminatedString
{
private:
	using Me = NullTerminatedString<CharT>;

	static constexpr auto NUL = CharT();

	static constexpr auto EMPTY_STR = &NUL;

public:
	/*!
	 * @brief NUL終端文字列を構築する
	 *
	 * 型で分岐して、NUL終端文字列を構築する。
	 */
	template <basis::NullTerminatedStringConstructible<CharT> A>
	constexpr explicit NullTerminatedString(
		const A& source
	)
	{
		// NUL終端文字列と互換性のある型
		if constexpr (basis::NullTerminatedStringCompatible<A, CharT>) {
			// 提供された文字列ポインタと長さを信頼する
			m_RawPtr = source.c_str();
			m_Length = source.length();
		}
		// ファイルパス型
		else if constexpr (std::same_as<CharT, WCHAR> && std::same_as<std::remove_cvref_t<A>, std::filesystem::path>) {
			// std::filesystem::pathはC++標準の中でも特殊。
			// ワイド・ナローに両対応で、内部保持形式は実装依存（≒OSのデフォルトに従う）。
			// Windowsはファイルパス管理をUnicode(UTF16LE)で行うのでstd::wstringがネイティブ。
			// native() は const string_type& を返すので、そのまま参照してしまう。
			m_RawPtr = source.native().c_str();
			m_Length = source.native().length();
		}
		// 配列型
		else if constexpr (std::is_array_v<std::remove_cvref_t<A>>) {
			// 配列もポインタ
			m_RawPtr = static_cast<const CharT*>(source);

			// 配列範囲でNUL終端を検索して長さとする
			const auto arraySize = std::size(source);
			const auto nulPos = std::char_traits<CharT>::find(source, arraySize, CharT());
			if (!nulPos) {
				throw std::invalid_argument("char array should be null-terminated.");
			}
			m_Length = static_cast<size_t>(nulPos - source);
		}
		// NULLポインタ型
		else if constexpr (std::same_as<std::remove_cvref_t<A>, std::nullptr_t>) {
			m_IsNull = true;
		}
		// 文字列として扱える型
		else if constexpr (basis::StringViewCompatible<A, CharT>) {
			// 引数を文字列として扱う
			auto text = static_cast<std::basic_string_view<CharT>>(source);

			// 空文字列
			if (text.empty())
			{
				// 何もしない
			}
			// NUL終端されていない場合
			else if (text.data()[text.size()])
			{
				// 内部バッファにコピーする
				m_Buffer = text;
			}
			// NUL終端されている場合
			else
			{
				// ポインタと長さをそのまま使う
				m_RawPtr = text.data();
				m_Length = text.length();
			}
		}
		// ポインタ型
		else if constexpr (std::is_pointer_v<std::remove_cvref_t<A>>) {
			// ポインタをそのまま使う
			m_RawPtr = static_cast<const CharT*>(source);

			// ポインタはNULLになり得るので分岐
			if (m_RawPtr) {
				m_Length = std::char_traits<CharT>::length(m_RawPtr);
			}
			else {
				m_IsNull = true;
			}
		}
		// その他
		else {
			// 文字列表現を生成できる
			static_assert(
				basis::StringConstructible<A, CharT>,
				"source should be StringConstructible"
			);

			m_Buffer = static_cast<std::basic_string<CharT>>(source);
		}
	}

	/*!
	 * @brief NUL終端文字列を構築する
	 *
	 * 生ポインタと長さを指定して、NUL終端文字列を構築する。
	 */
	constexpr NullTerminatedString(
		const CharT* rawPtr,
		size_t length
	)
		: m_IsNull(!rawPtr)
		, m_RawPtr(rawPtr)
		, m_Length(length)
	{
	}

	NullTerminatedString(const Me&) = delete;
	Me& operator = (const Me&) = delete;

	/*!
	 * @brief C String(NUL終端文字列)を取得する
	 *
	 * @return C String(NUL終端文字列)
	 */
	constexpr const CharT* c_str() const noexcept
	{
		if (m_IsNull) {
			return nullptr;
		}

		if (m_RawPtr) {
			return m_RawPtr;	// 保持している生ポインターを返す
		}

		if (m_Buffer.empty()) {
			return EMPTY_STR;	// 空文字列を返す
		}

		return m_Buffer.c_str();
	}

	/*!
	 * @brief C String(NUL終端文字列)がインスタンスに依存するかどうか
	 */
	[[nodiscard("Check whether c_str() depends on this object's lifetime.")]]
	bool uses_buffer() const noexcept
	{
		// NULLならバッファを使わない
		if (m_IsNull) {
			return false;
		}

		// 生ポインタを保持するときはバッファを使わない
		if (m_RawPtr) {
			return false;
		}

		// 内部バッファが空のときは未使用とみなす
		if (m_Buffer.empty()) {
			return false;
		}

		// 内部バッファを使っている
		return true;
	}

	/*!
	 * @brief 文字列長を取得する
	 *
	 * @return 文字列長（NUL終端を含まない）
	 */
	constexpr size_t length() const noexcept
	{
		// 内部バッファが空のとき
		if (m_Buffer.empty()) {
			return m_Length;
		}

		// 内部バッファの長さを返す
		return m_Buffer.length();
	}

	/*!
	 * @brief 省略可能な文字列引数に変換する
	 *
	 * @return 省略可能な文字列引数
	 */
	constexpr std::optional<std::basic_string<CharT>> optStr() const {
		if (m_IsNull) {
			return std::nullopt;
		}

		return std::make_optional(std::basic_string<CharT>(str()));
	}

	/*!
	 * @brief 文字列参照に変換する
	 *
	 * @return 文字列参照
	 */
	constexpr auto str() const noexcept
	{
		return std::basic_string_view<CharT>(c_str(), length());
	}

	/*!
	 * @brief 文字列ポインタに変換する演算子
	 *
	 * @return NUL終端文字列、または、NULLポインタ
	 */
	constexpr explicit operator const CharT*() const noexcept
	{
		return c_str();
	}

	/*!
	 * @brief 文字列に変換する演算子
	 *
	 * @return 文字列
	 */
	constexpr explicit operator std::basic_string<CharT>() const
	{
		return std::basic_string<CharT>{ str() };
	}

	/*!
	 * @brief 文字列参照に変換する演算子
	 *
	 * @return 文字列参照
	 */
	constexpr explicit operator std::basic_string_view<CharT>() const noexcept
	{
		return str();
	}

private:
	bool m_IsNull = false;

	const CharT* m_RawPtr = nullptr;
	size_t m_Length = 0;

	std::basic_string<CharT> m_Buffer{};
};

} // namespace cxx

#endif /* SAKURA_PRIMITIVE_C8059DE4_C986_492E_9C09_7F044049C481_H_ */
