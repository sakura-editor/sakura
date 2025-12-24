/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CLAXINTEGER_B3F68913_E6AE_472E_AC4F_E8985190BD7E_H_
#define SAKURA_CLAXINTEGER_B3F68913_E6AE_472E_AC4F_E8985190BD7E_H_
#pragma once

//!型チェックの緩い整数型
class CLaxInteger {
private:
	using Me = CLaxInteger;

public:
	//! デフォルトコンストラクタ
	CLaxInteger() noexcept = default;

	//! intから構築（変換は考慮しない）
	CLaxInteger(int value) noexcept : m_value(value) {}

	CLaxInteger(const Me&) = default;
	Me& operator = (const Me&) = default;

	~CLaxInteger() noexcept = default;

	//! 代入演算子
	template<typename T> requires (std::is_integral_v<T> || std::is_convertible_v<T, Me>)
	Me& operator = (const T& n) noexcept { m_value = Me(n).m_value; return *this; }

	//! intから構築できる型への明示変換は可能とする
	template<typename T> requires std::is_constructible_v<T, int>
	explicit operator T() const noexcept { return T(m_value); }

	//暗黙の変換
	/* implicit */ operator int() const noexcept { return m_value; }
	/* implicit */ operator int&()      noexcept { return m_value; }

private:
	int m_value = 0;
};

#endif /* SAKURA_CLAXINTEGER_B3F68913_E6AE_472E_AC4F_E8985190BD7E_H_ */
