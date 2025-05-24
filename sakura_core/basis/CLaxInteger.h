/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CLAXINTEGER_B3F68913_E6AE_472E_AC4F_E8985190BD7E_H_
#define SAKURA_CLAXINTEGER_B3F68913_E6AE_472E_AC4F_E8985190BD7E_H_
#pragma once

//!型チェックの緩い整数型
class CLaxInteger{
	using Me = CLaxInteger;

public:
	//コンストラクタ・デストラクタ
	CLaxInteger() noexcept = default;
	CLaxInteger(int value) { m_value = value; }
	CLaxInteger(const Me&) = default;
	Me& operator = (const Me&) = default;
	~CLaxInteger() noexcept = default;

	//暗黙の変換
	operator const int&() const{ return m_value; }
	operator       int&()      { return m_value; }

private:
	int m_value = 0;
};
#endif /* SAKURA_CLAXINTEGER_B3F68913_E6AE_472E_AC4F_E8985190BD7E_H_ */
