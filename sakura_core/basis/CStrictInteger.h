/*!	@file
	@brief 厳格っぷりをカスタマイズ可能な整数クラス
	       Integer class, which is static-type-checked strict and flexible at compile.

	@author kobake
	@date 2007.10.16
*/
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSTRICTINTEGER_5B7614A0_282F_48F6_9420_CE672061CF3E_H_
#define SAKURA_CSTRICTINTEGER_5B7614A0_282F_48F6_9420_CE672061CF3E_H_
#pragma once

#include "primitive.h" // for Int

//! 整数型、または、intにキャスト可能な型
template<typename T>
concept IntOrCastable = std::is_integral_v<T> || std::is_convertible_v<T, int>;

//! 整数型、または、Int(CLazyInt)
template<typename T>
concept IntOrLazyInt = std::is_integral_v<T> || (!std::is_same_v<Int, int> && std::is_same_v<T, Int>);

//! 暗黙の変換を許さない、整数クラス
template <
	int STRICT_ID,			//!< 型を分けるための数値。0 or 1。
	bool ALLOW_CMP_INT,		//!< intとの比較を許すかどうか
	bool ALLOW_ADDSUB_INT,	//!< intとの加減算を許すかどうか
	bool ALLOW_CAST_INT,	//!< intへの暗黙の変換を許すかどうか
	bool ALLOW_ASSIGNOP_INT	//!< intの代入を許すかどうか
>
class CStrictInteger{
private:
	using Me = CStrictInteger<
		STRICT_ID,
		ALLOW_CMP_INT,
		ALLOW_ADDSUB_INT,
		ALLOW_CAST_INT,
		ALLOW_ASSIGNOP_INT
	>;

	// -- -- -- -- 別種のCStrictIntegerとの演算は絶対許さん(やりたきゃintでも介してください) -- -- -- -- //
	template <int N0, bool B0, bool B1, bool B2, bool B3> requires (N0 != STRICT_ID) Me&  operator += (const CStrictInteger<N0, B0, B1, B2, B3>&) noexcept;
	template <int N0, bool B0, bool B1, bool B2, bool B3> requires (N0 != STRICT_ID) Me&  operator -= (const CStrictInteger<N0, B0, B1, B2, B3>&) noexcept;
	template <int N0, bool B0, bool B1, bool B2, bool B3> requires (N0 != STRICT_ID) Me   operator +  (const CStrictInteger<N0, B0, B1, B2, B3>&) const noexcept;
	template <int N0, bool B0, bool B1, bool B2, bool B3> requires (N0 != STRICT_ID) Me   operator -  (const CStrictInteger<N0, B0, B1, B2, B3>&) const noexcept;
	template <int N0, bool B0, bool B1, bool B2, bool B3> requires (N0 != STRICT_ID) Me&  operator =  (const CStrictInteger<N0, B0, B1, B2, B3>&) noexcept;
	template <int N0, bool B0, bool B1, bool B2, bool B3> requires (N0 != STRICT_ID) bool operator <  (const CStrictInteger<N0, B0, B1, B2, B3>&) const noexcept;
	template <int N0, bool B0, bool B1, bool B2, bool B3> requires (N0 != STRICT_ID) bool operator <= (const CStrictInteger<N0, B0, B1, B2, B3>&) const noexcept;
	template <int N0, bool B0, bool B1, bool B2, bool B3> requires (N0 != STRICT_ID) bool operator >  (const CStrictInteger<N0, B0, B1, B2, B3>&) const noexcept;
	template <int N0, bool B0, bool B1, bool B2, bool B3> requires (N0 != STRICT_ID) bool operator >= (const CStrictInteger<N0, B0, B1, B2, B3>&) const noexcept;
	template <int N0, bool B0, bool B1, bool B2, bool B3> requires (N0 != STRICT_ID) bool operator == (const CStrictInteger<N0, B0, B1, B2, B3>&) const noexcept;
	template <int N0, bool B0, bool B1, bool B2, bool B3> requires (N0 != STRICT_ID) bool operator != (const CStrictInteger<N0, B0, B1, B2, B3>&) const noexcept;

public:
	//コンストラクタ・デストラクタ
	CStrictInteger() = default;

	CStrictInteger(const Me&) = default;
	Me& operator = (const Me&) = default;

	~CStrictInteger() noexcept = default;

	//整数型からの変換は、「明示的に指定したときのみ」可能
	template<IntOrCastable T>
	explicit CStrictInteger(T value) noexcept
		: m_value(static_cast<int>(value))
	{
	}

	//! 値を取得する
	int		GetValue() const noexcept { return m_value; }

	//! 値を設定する
	template<IntOrCastable T>
	void	SetValue(T n) noexcept { m_value = static_cast<int>(n); }

	//算術演算子 (加算、減算は同クラス同士でしか許さない)
	Me& operator += (const Me& rhs)	noexcept { m_value += rhs.m_value; return *this; }
	Me& operator -= (const Me& rhs)	noexcept { m_value -= rhs.m_value; return *this; }
	Me& operator %= (const Me& rhs)	noexcept { m_value %= rhs.m_value; return *this; }
	template<std::integral T> Me& operator *= (T n)	noexcept { m_value *= int(n); return *this; }
	template<IntOrCastable T> Me& operator /= (T n)	noexcept { m_value /= int(n); return *this; }
	template<std::integral T> Me& operator %= (T n)	noexcept { return *this %= Me(n); }

	//算術演算子２ (加算、減算は同クラス同士でしか許さない)
	Me operator + (const Me& rhs) const noexcept { Me ret(m_value); return ret += rhs; }
	Me operator - (const Me& rhs) const noexcept { Me ret(m_value); return ret -= rhs; }
	Me operator % (const Me& rhs) const noexcept { Me ret(m_value); return ret %= rhs; }
	template<std::integral T> Me operator * (T n) const noexcept { Me ret(m_value); ret *= n; return ret; }
	template<IntOrCastable T> Me operator / (T n) const noexcept { Me ret(m_value); ret /= n; return ret; }
	template<std::integral T> Me operator % (T n) const noexcept { Me ret(m_value); ret %= n; return ret; }

	//算術演算子３
	Me&	operator ++ () noexcept    { ++m_value; return *this; }	//!< 前置インクリメント
	Me&	operator -- () noexcept    { --m_value; return *this; }	//!< 前置デクリメント
	Me	operator ++ (int) noexcept { Me prev(m_value); ++(*this); return prev; }	//!< 後置インクリメント
	Me	operator -- (int) noexcept { Me prev(m_value); --(*this); return prev; }	//!< 後置デクリメント

	//算術演算子４
	Me operator - () const noexcept { return Me(-m_value); }

	//比較演算子
	bool operator <  (const Me& rhs) const noexcept { return m_value <  rhs.m_value; }
	bool operator <= (const Me& rhs) const noexcept { return m_value <= rhs.m_value; }
	bool operator >  (const Me& rhs) const noexcept { return m_value >  rhs.m_value; }
	bool operator >= (const Me& rhs) const noexcept { return m_value >= rhs.m_value; }
	bool operator == (const Me& rhs) const noexcept { return m_value == rhs.m_value; }
	bool operator != (const Me& rhs) const noexcept { return m_value != rhs.m_value; }

	//Int(CLaxInt)への変換は常に許す
	/* implicit */ operator Int() const noexcept { return Int(m_value); }

	//! intから構築できる型への明示変換は可能とする
	template<typename T>
	requires (std::is_constructible_v<T, int>)
	explicit operator T() const noexcept {
		return T(m_value);
	}

	// -- -- -- -- ALLOW_ADDSUB_INTがtrueの場合は、intとの加減算を許す -- -- -- -- //
	template<IntOrCastable T> Me& operator += (const T& rhs) noexcept       { static_assert(ALLOW_ADDSUB_INT, "addsub not allowed."); m_value += static_cast<int>(rhs); return *this; }
	template<IntOrCastable T> Me& operator -= (const T& rhs) noexcept       { static_assert(ALLOW_ADDSUB_INT, "addsub not allowed."); m_value -= static_cast<int>(rhs); return *this; }
	template<IntOrCastable T> Me  operator +  (const T& rhs) const noexcept { static_assert(ALLOW_ADDSUB_INT, "addsub not allowed."); Me ret(m_value); return ret += rhs; }
	template<IntOrCastable T> Me  operator -  (const T& rhs) const noexcept { static_assert(ALLOW_ADDSUB_INT, "addsub not allowed."); Me ret(m_value); return ret -= rhs; }

	// -- -- -- -- ALLOW_CMP_INTがtrueの場合は、intとの比較を許す -- -- -- -- //
	template<IntOrCastable T> bool operator <  (const T& rhs) const noexcept { static_assert(ALLOW_CMP_INT, "compare not allowed."); return m_value <  static_cast<int>(rhs); }
	template<IntOrCastable T> bool operator <= (const T& rhs) const noexcept { static_assert(ALLOW_CMP_INT, "compare not allowed."); return m_value <= static_cast<int>(rhs); }
	template<IntOrCastable T> bool operator >  (const T& rhs) const noexcept { static_assert(ALLOW_CMP_INT, "compare not allowed."); return m_value >  static_cast<int>(rhs); }
	template<IntOrCastable T> bool operator >= (const T& rhs) const noexcept { static_assert(ALLOW_CMP_INT, "compare not allowed."); return m_value >= static_cast<int>(rhs); }
	template<IntOrCastable T> bool operator == (const T& rhs) const noexcept { static_assert(ALLOW_CMP_INT, "compare not allowed."); return m_value == static_cast<int>(rhs); }
	template<IntOrCastable T> bool operator != (const T& rhs) const noexcept { static_assert(ALLOW_CMP_INT, "compare not allowed."); return m_value != static_cast<int>(rhs); }

	// -- -- -- -- ALLOW_CAST_INTがtrueの場合は、intへの暗黙の変換を許す -- -- -- -- //
	/* implicit */ operator int() const noexcept requires ALLOW_CAST_INT {
		return GetValue();
	}

	// -- -- -- -- ALLOW_ASSIGNOP_INTがtrueの場合は、intの代入を許す -- -- -- -- //
	template<IntOrLazyInt T>
	Me& operator = (const T& rhs) noexcept {
		//※ALLOW_ASSIGNOP_INTがfalseの場合は、代入禁止。
		static_assert(ALLOW_ASSIGNOP_INT, "assign not allowed.");

		SetValue(rhs);

		return *this;
	}

private:
	// -- -- -- -- メンバ変数 -- -- -- -- //
	int		m_value = 0;
};

//左辺がint等の場合の演算子
#define STRICTINT_LEFT_INT_CMP(TYPE) \
	template <int N, bool B0, bool B1, bool B2, bool B3> inline bool operator <  (TYPE lhs, const CStrictInteger<N, B0, B1, B2, B3>& rhs) noexcept { return rhs >  static_cast<int>(lhs); } \
	template <int N, bool B0, bool B1, bool B2, bool B3> inline bool operator <= (TYPE lhs, const CStrictInteger<N, B0, B1, B2, B3>& rhs) noexcept { return rhs >= static_cast<int>(lhs); } \
	template <int N, bool B0, bool B1, bool B2, bool B3> inline bool operator >  (TYPE lhs, const CStrictInteger<N, B0, B1, B2, B3>& rhs) noexcept { return rhs <  static_cast<int>(lhs); } \
	template <int N, bool B0, bool B1, bool B2, bool B3> inline bool operator >= (TYPE lhs, const CStrictInteger<N, B0, B1, B2, B3>& rhs) noexcept { return rhs <= static_cast<int>(lhs); } \
	template <int N, bool B0, bool B1, bool B2, bool B3> inline bool operator == (TYPE lhs, const CStrictInteger<N, B0, B1, B2, B3>& rhs) noexcept { return rhs == static_cast<int>(lhs); } \
	template <int N, bool B0, bool B1, bool B2, bool B3> inline bool operator != (TYPE lhs, const CStrictInteger<N, B0, B1, B2, B3>& rhs) noexcept { return rhs != static_cast<int>(lhs); } \
	template <int N, bool B0, bool B1, bool B2, bool B3> inline CStrictInteger<N, B0, B1, B2, B3> operator + (TYPE lhs, const CStrictInteger<N, B0, B1, B2, B3>& rhs) noexcept { return  rhs + static_cast<int>(lhs); } \
	template <int N, bool B0, bool B1, bool B2, bool B3> inline CStrictInteger<N, B0, B1, B2, B3> operator - (TYPE lhs, const CStrictInteger<N, B0, B1, B2, B3>& rhs) noexcept { return -rhs + static_cast<int>(lhs); } \
	template <int N, bool B0, bool B1, bool B2, bool B3> inline CStrictInteger<N, B0, B1, B2, B3> operator * (TYPE lhs, const CStrictInteger<N, B0, B1, B2, B3>& rhs) noexcept { return  rhs * static_cast<int>(lhs); }

STRICTINT_LEFT_INT_CMP(int)
STRICTINT_LEFT_INT_CMP(short)
STRICTINT_LEFT_INT_CMP(size_t)
STRICTINT_LEFT_INT_CMP(LONG)

// CStrictIntegerテンプレート型を検出するための型特性
template<typename T>
struct is_strict_integer : std::false_type {};

template<int STRICT_ID, bool ALLOW_CMP_INT, bool ALLOW_ADDSUB_INT, bool ALLOW_CAST_INT, bool ALLOW_ASSIGNOP_INT>
struct is_strict_integer<CStrictInteger<STRICT_ID, ALLOW_CMP_INT, ALLOW_ADDSUB_INT, ALLOW_CAST_INT, ALLOW_ASSIGNOP_INT>> : std::true_type {};

template<typename T>
inline constexpr bool is_strict_integer_v = is_strict_integer<std::remove_cv_t<std::remove_reference_t<T>>>::value;

#endif /* SAKURA_CSTRICTINTEGER_5B7614A0_282F_48F6_9420_CE672061CF3E_H_ */
