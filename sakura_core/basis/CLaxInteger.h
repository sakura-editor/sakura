/*! @file */
/*
	Copyright (C) 2018-2020 Sakura Editor Organization

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

/*!
 * 型チェックの緩い整数型
 *
 * StrictIntegerに対するLax版として作成されたクラスと思われる。
 * 基本型がint(int32_t)決め打ちになっていて64bit対応しづらいのでテンプレート化。
 *
 * このクラスはC++11のリテラル型である。
 * constexprキーワードを付けて宣言すれば、コンパイル時定数として利用できる。
 */
template <typename BASE_TYPE>
class CLaxInteger {
	//! 数値データ
	BASE_TYPE m_value;

	using Me = CLaxInteger<BASE_TYPE>;

public:
	 //! デフォルトコンストラクタは利用しない
	CLaxInteger() = delete;

	/*!
	 * 値指定コンストラクタ
	 *
	 * 指定した値を持つインスタンスを生成する。
	 * explicit指定のないコンストラクタは暗黙変換を可能にする
	 */
	constexpr CLaxInteger( const BASE_TYPE& value ) noexcept
		: m_value( value )
	{
		static_assert(std::is_integral_v<BASE_TYPE>, "unexpected usage.");
	}

	//! デフォルトのコピーコンストラクタを使う
	CLaxInteger( const Me& ) = default;
	//! デフォルトのコピー代入演算子を使う
	Me& operator = ( const Me& ) = default;

	//! デフォルトのムーブコンストラクタを使う
	CLaxInteger( Me&& ) = default;
	//! デフォルトのムーブ代入演算子を使う
	Me& operator = ( Me&& ) = default;

	//! デフォルトのデストラクタを使う
	~CLaxInteger( void ) = default;

	/*!
	 * 基本型へのキャスト演算子(const版)
	 *
	 * 基本型への暗黙変換ができるように定義する。
	 */
	constexpr operator const BASE_TYPE&() const noexcept { return m_value; }

	/*!
	 * 基本型へのキャスト演算子(非const版)
	 *
	 * 基本型への暗黙変換ができるように定義する。
	 */
	constexpr operator BASE_TYPE&() noexcept { return m_value; }
};
