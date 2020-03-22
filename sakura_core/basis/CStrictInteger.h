/*! @file
	Copyright (C) 2007, kobake
	Copyright (C) 2007-2017 SAKURA Editor Project
	Copyright (C) 2018-2020 SAKURA Editor Organization

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

#include <type_traits>

#include "basis/CLaxInteger.h"
#include "debug/Debug2.h"

//! 厳格な型のカテゴリ
enum class StrictCategory : uint8_t {
	Logic,		//!< 論理単位
	Layout,		//!< レイアウト単位
	Pixel,		//!< ピクセル単位
};

//! 厳格な型の属性
enum class StrictAttribute : uint8_t {
	None,			//!< なし
	Horizontal,		//!< 水平方向(char,桁,x座標)
	Vertical,		//!< 垂直方向(line,行,y座標)
};

/*!
 * カテゴリと属性から、StrictIdを生成する。
 */
inline constexpr int MakeStrictId(
	const StrictCategory	category,
	const StrictAttribute	attribute
)
{
	auto hi = static_cast<uint8_t>(category);
	auto lo = static_cast<uint8_t>(attribute);
	return  (hi << 8) | lo;
}

/*!
 * 暗黙の変換を許さない、整数クラス
 *
 * CStrictIntegerから名称変更。
 *
 * @brief 厳格っぷりをカスタマイズ可能な整数クラス
 * 		   Integer class, which is static-type-checked strict and flexible at compile.
 *
 * @author kobake
 * @date 2007.10.16
*/
template <
	int STRICT_ID,				//!< 型を分けるための数値。
	bool ALLOW_CAST_INT,		//!< 基本型への暗黙の変換を許すかどうか
	typename BASE_TYPE			//!< 基本型
>
class CStrictInteger{
	//! 数値データ
	BASE_TYPE m_value;

private:
	using Me = CStrictInteger<
		STRICT_ID,
		ALLOW_CAST_INT,
		BASE_TYPE
	>;

public:
	//! 基本型
	using BaseType = BASE_TYPE;
	//! ゆるい型
	using LaxType = CLaxInteger<BASE_TYPE>;

	//! デフォルトコンストラクタ
	CStrictInteger() noexcept
		: m_value( 0 )
	{
	}

	/*!
	 * 値指定コンストラクタ
	 *
	 * explicit指定を付けることにより、暗黙変換を抑止する。
	 * intからの変換は、「明示的に指定したときのみ」可能
	 */
	explicit constexpr CStrictInteger( const BASE_TYPE& value ) noexcept
		: m_value( value )
	{
	}

	//! デフォルトのコピーコンストラクタを使う
	CStrictInteger( const Me& ) = default;
	//! デフォルトのコピー代入演算子を使う
	Me& operator = ( const Me& ) = default;

	//! デフォルトのムーブコンストラクタを使う
	CStrictInteger( Me&& ) = default;
	//! デフォルトのムーブ代入演算子を使う
	Me& operator = ( Me&& ) = default;

	//! デフォルトのデストラクタを使う
	~CStrictInteger( void ) = default;

	// 複合代入演算子
	constexpr Me& operator += ( const Me& rhs )		{ m_value += rhs.m_value; return *this; }
	constexpr Me& operator -= ( const Me& rhs )		{ m_value -= rhs.m_value; return *this; }
	constexpr Me& operator *= ( const Me& rhs )		{ m_value *= rhs.m_value; return *this; }
	constexpr Me& operator /= ( const Me& rhs )		{ m_value /= rhs.m_value; return *this; }
	constexpr Me& operator %= ( const Me& rhs )		{ m_value %= rhs.m_value; return *this; }

	//算術演算子２ (加算、減算は同クラス同士でしか許さない)
	constexpr Me operator + ( const Me& rhs ) const	{ Me ret( *this ); return ret += rhs; }
	constexpr Me operator - ( const Me& rhs ) const	{ Me ret( *this ); return ret -= rhs; }
	constexpr Me operator * ( const Me& rhs ) const	{ Me ret( *this ); return ret *= rhs; }
	constexpr Me operator / ( const Me& rhs ) const	{ Me ret( *this ); return ret /= rhs; }
	constexpr Me operator % ( const Me& rhs ) const	{ Me ret( *this ); return ret %= rhs; }

	/*!
	 * 前置インクリメント演算子(算術演算子３)
	 *
	 * ++n のこと。
	 */
	constexpr Me& operator ++ () { ++m_value; return *this; }

	/*!
	 * 後置インクリメント演算子(算術演算子３)
	 *
	 * n++ のこと。
	 * 実装を見て分かる通り、前置に比べてやや効率が悪い。
	 */
	constexpr Me operator ++ ( int ) { Me ret( m_value ); ++(*this); return ret; }

	/*!
	 * 前置デクリメント演算子(算術演算子３)
	 *
	 * --n のこと。
	 */
	constexpr Me& operator -- () { --m_value; return *this; }

	/*!
	 * 後置デクリメント演算子(算術演算子３)
	 *
	 * n-- のこと。
	 * 実装を見て分かる通り、前置に比べてやや効率が悪い。
	 */
	constexpr Me operator -- ( int ) { Me ret( m_value ); --(*this); return ret; }

	/*!
	 * 単項プラス演算子(算術演算子４)
	 *
	 * +n のこと。
	 * 実装を見て分かる通り、定義する意味は「ほぼ」ない。
	 */
	constexpr Me operator + () const { return Me( m_value ); }

	/*!
	 * 単項マイナス演算子(算術演算子４)
	 *
	 * -n のこと。
	 */
	constexpr Me operator - () const {
		if constexpr (std::is_signed_v<BASE_TYPE>) {
			assert_warning( std::numeric_limits<BASE_TYPE>::min() < m_value );
		}
		return Me( -m_value );
	}

	//比較演算子
	bool operator <  ( const Me& rhs ) const { return m_value <  rhs.m_value; }
	bool operator <= ( const Me& rhs ) const { return m_value <= rhs.m_value; }
	bool operator >  ( const Me& rhs ) const { return m_value >  rhs.m_value; }
	bool operator >= ( const Me& rhs ) const { return m_value >= rhs.m_value; }
	bool operator == ( const Me& rhs ) const { return m_value == rhs.m_value; }
	bool operator != ( const Me& rhs ) const { return m_value != rhs.m_value; }

public:
	/*!
	 * 代入演算子
	 *
	 * T型がLaxである、または、数値型である場合に利用可能。
	 */
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || std::is_integral_v<T> >>
	constexpr Me& operator = ( const T& rhs ) { return *this = Me( (BASE_TYPE)rhs ); }

	/*!
	 * 加算代入演算子
	 *
	 * T型がLaxである、または、数値型である場合に利用可能。
	 */
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || std::is_integral_v<T> >>
	constexpr Me& operator += ( const T& rhs ) { return *this += Me( rhs ); }

	/*!
	 * 減算代入演算子
	 *
	 * T型がLaxである、または、数値型である場合に利用可能。
	 */
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || std::is_integral_v<T> >>
	constexpr Me& operator -= ( const T& rhs ) { return *this -= Me( rhs ); }

	/*!
	 * 乗算代入演算子
	 *
	 * T型がLaxである、または、数値型である場合に利用可能。
	 */
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || std::is_integral_v<T> >>
	constexpr Me& operator *= ( const T& rhs ) { return *this *= Me( rhs ); }

	/*!
	 * 除算代入演算子
	 *
	 * T型がLaxである、または、数値型である場合に利用可能。
	 */
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || std::is_integral_v<T> >>
	constexpr Me& operator /= ( const T& rhs ) { return *this /= Me( rhs ); }

	/*!
	 * 剰余算代入演算子
	 *
	 * T型がLaxである、または、数値型である場合に利用可能。
	 */
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || std::is_integral_v<T> >>
	constexpr Me& operator %= ( const T& rhs ) { return *this %= Me( rhs ); }

	/*!
	 * 加算演算子
	 *
	 * T型がLaxである、または、数値型である場合に利用可能。
	 */
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || std::is_integral_v<T> >>
	constexpr Me  operator + ( const T& rhs ) const { Me ret( *this ); return ret += rhs; }

	/*!
	 * 減算演算子
	 *
	 * T型がLaxである、または、数値型である場合に利用可能。
	 */
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || std::is_integral_v<T> >>
	constexpr Me operator - ( const T& rhs ) const { Me ret( *this ); return ret -= rhs; }

	/*!
	 * 乗算演算子
	 *
	 * T型がLaxである、または、数値型である場合に利用可能。
	 */
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || std::is_integral_v<T> >>
	constexpr Me operator * ( const T& rhs ) const { Me ret( *this ); return ret *= rhs; }

	/*!
	 * 除算演算子
	 *
	 * T型がLaxである、または、数値型である場合に利用可能。
	 */
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || std::is_integral_v<T> >>
	constexpr Me operator / ( const T& rhs ) const { Me ret( *this ); return ret /= rhs; }

	/*!
	 * 剰余算演算子
	 *
	 * T型がLaxである、または、数値型である場合に利用可能。
	 */
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || std::is_integral_v<T> >>
	constexpr Me operator % ( const T& rhs ) const { Me ret( *this ); return ret %= rhs; }

	//比較演算子
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<BASE_TYPE> ) >> constexpr bool operator <  ( const T& rhs ) const { return *this <  Me( rhs ); }
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<BASE_TYPE> ) >> constexpr bool operator <= ( const T& rhs ) const { return *this <= Me( rhs ); }
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<BASE_TYPE> ) >> constexpr bool operator >  ( const T& rhs ) const { return *this >  Me( rhs ); }
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<BASE_TYPE> ) >> constexpr bool operator >= ( const T& rhs ) const { return *this >= Me( rhs ); }
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<BASE_TYPE> ) >> constexpr bool operator == ( const T& rhs ) const { return *this == Me( rhs ); }
	template<typename T, typename = std::enable_if_t< std::is_same_v<T, LaxType> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<BASE_TYPE> ) >> constexpr bool operator != ( const T& rhs ) const { return *this != Me( rhs ); }

	/*!
	 * キャスト演算子(基本型へのキャスト)
	 *
	 * ALLOW_CAST_INTがfalseの場合、変換が使われるとビルドエラーになる。
	 */
	constexpr operator BASE_TYPE() const {
		static_assert(ALLOW_CAST_INT, "cast to base-type is not allowed.");
		return m_value;
	}

	/*!
	 * キャスト演算子(LaxTypeへのキャスト)
	 *
	 * LaxTypeへの変換は常に許す
	 */
	constexpr operator LaxType() const { return m_value; }

	// -- -- -- -- 別種のCStrictIntegerとの演算は絶対許さん(やりたきゃintでも介してください) -- -- -- -- //
	template<int N, bool B0, typename T0> constexpr Me&  operator += ( const CStrictInteger<N, B0, T0>& rhs )       { static_assert( STRICT_ID != N, "forbidden"); return *this += rhs; }
	template<int N, bool B0, typename T0> constexpr Me&  operator -= ( const CStrictInteger<N, B0, T0>& rhs )       { static_assert( STRICT_ID != N, "forbidden"); return *this -= rhs; }
	template<int N, bool B0, typename T0> constexpr Me   operator +  ( const CStrictInteger<N, B0, T0>& rhs ) const { static_assert( STRICT_ID != N, "forbidden"); return *this +  rhs; }
	template<int N, bool B0, typename T0> constexpr Me   operator -  ( const CStrictInteger<N, B0, T0>& rhs ) const { static_assert( STRICT_ID != N, "forbidden"); return *this -  rhs; }
	template<int N, bool B0, typename T0> constexpr Me&  operator =  ( const CStrictInteger<N, B0, T0>& rhs )       { static_assert( STRICT_ID != N, "forbidden"); return *this =  rhs; }
	template<int N, bool B0, typename T0> constexpr bool operator <  ( const CStrictInteger<N, B0, T0>& rhs ) const { static_assert( STRICT_ID != N, "forbidden"); return *this <  rhs; }
	template<int N, bool B0, typename T0> constexpr bool operator <= ( const CStrictInteger<N, B0, T0>& rhs ) const { static_assert( STRICT_ID != N, "forbidden"); return *this <= rhs; }
	template<int N, bool B0, typename T0> constexpr bool operator >  ( const CStrictInteger<N, B0, T0>& rhs ) const { static_assert( STRICT_ID != N, "forbidden"); return *this >  rhs; }
	template<int N, bool B0, typename T0> constexpr bool operator >= ( const CStrictInteger<N, B0, T0>& rhs ) const { static_assert( STRICT_ID != N, "forbidden"); return *this >= rhs; }
	template<int N, bool B0, typename T0> constexpr bool operator == ( const CStrictInteger<N, B0, T0>& rhs ) const { static_assert( STRICT_ID != N, "forbidden"); return *this == rhs; }
	template<int N, bool B0, typename T0> constexpr bool operator != ( const CStrictInteger<N, B0, T0>& rhs ) const { static_assert( STRICT_ID != N, "forbidden"); return *this != rhs; }
};

//左辺がint等の場合の演算子
template<typename T, int N, bool B0, typename T0, typename = std::enable_if_t< std::is_same_v<T, CLaxInteger<T0>> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<T0> ) >> inline constexpr bool operator <  ( const T& lhs, const CStrictInteger<N, B0, T0>& rhs ) { return rhs >  lhs; }
template<typename T, int N, bool B0, typename T0, typename = std::enable_if_t< std::is_same_v<T, CLaxInteger<T0>> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<T0> ) >> inline constexpr bool operator <= ( const T& lhs, const CStrictInteger<N, B0, T0>& rhs ) { return rhs >= lhs; }
template<typename T, int N, bool B0, typename T0, typename = std::enable_if_t< std::is_same_v<T, CLaxInteger<T0>> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<T0> ) >> inline constexpr bool operator >  ( const T& lhs, const CStrictInteger<N, B0, T0>& rhs ) { return rhs <  lhs; }
template<typename T, int N, bool B0, typename T0, typename = std::enable_if_t< std::is_same_v<T, CLaxInteger<T0>> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<T0> ) >> inline constexpr bool operator >= ( const T& lhs, const CStrictInteger<N, B0, T0>& rhs ) { return rhs <=  lhs; }
template<typename T, int N, bool B0, typename T0, typename = std::enable_if_t< std::is_same_v<T, CLaxInteger<T0>> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<T0> ) >> inline constexpr bool operator == ( const T& lhs, const CStrictInteger<N, B0, T0>& rhs ) { return rhs == lhs; }
template<typename T, int N, bool B0, typename T0, typename = std::enable_if_t< std::is_same_v<T, CLaxInteger<T0>> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<T0> ) >> inline constexpr bool operator != ( const T& lhs, const CStrictInteger<N, B0, T0>& rhs ) { return rhs != lhs; }
template<typename T, int N, bool B0, typename T0, typename = std::enable_if_t< std::is_same_v<T, CLaxInteger<T0>> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<T0> ) >> inline constexpr auto operator +  ( const T& lhs, const CStrictInteger<N, B0, T0>& rhs ) { return +rhs + lhs; }
template<typename T, int N, bool B0, typename T0, typename = std::enable_if_t< std::is_same_v<T, CLaxInteger<T0>> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<T0> ) >> inline constexpr auto operator -  ( const T& lhs, const CStrictInteger<N, B0, T0>& rhs ) { return -rhs + lhs; }
template<typename T, int N, bool B0, typename T0, typename = std::enable_if_t< std::is_same_v<T, CLaxInteger<T0>> || ( std::is_integral_v<T> && std::is_signed_v<T> == std::is_signed_v<T0> ) >> inline constexpr auto operator *  ( const T& lhs, const CStrictInteger<N, B0, T0>& rhs ) { return +rhs * lhs; }
