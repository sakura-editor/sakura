/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
