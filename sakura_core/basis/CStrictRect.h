/*! @file */
/*
	Copyright (C) 2008, kobake

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

#include <Windows.h> //tagRECT,tagPOINT

#include "basis/CStrictPoint.h"

template <class PointType>
class CStrictRect {
	using Me = CStrictRect<PointType>;
	using UNIT_X_TYPE = decltype(PointType::x);
	using UNIT_Y_TYPE = decltype(PointType::y);

public:
	//メンバ変数は公開
	UNIT_X_TYPE left;
	UNIT_Y_TYPE top;
	UNIT_X_TYPE right;
	UNIT_Y_TYPE bottom;

	/*!
	 * デフォルトコンストラクタ
	 *
	 * 新しいインスタンスを構築する。
	 */
	constexpr CStrictRect() noexcept
		: Me( 0, 0, 0, 0 )
	{
	}

	/*!
	 * 値指定コンストラクタ
	 *
	 * left, top, right, bottom を指定して新しいインスタンスを構築する。
	 */
	template<typename X, typename Y,
		typename = std::enable_if_t< std::is_assignable_v<UNIT_X_TYPE&, X> && std::is_assignable_v<UNIT_Y_TYPE&, Y> > >
	constexpr CStrictRect( const X xLeft, const Y yTop, const X xRight, const Y yBottom ) noexcept
		: left( xLeft )
		, top( yTop )
		, right( xRight )
		, bottom( yBottom )
	{
	}

	//! デフォルトのコピーコンストラクタを使う
	CStrictRect(const Me&) = default;
	//! デフォルトのコピー代入演算子を使う
	Me& operator = (const Me&) = default;

	//! デフォルトのムーブコンストラクタを使う
	CStrictRect(Me&&) = default;
	//! デフォルトのムーブ代入演算子を使う
	Me& operator = (Me&&) = default;

public:
	//!左上座標 (TopLeft)
	PointType UpperLeft() const
	{
		return PointType(left,top);
	}
	//!右下座標 (BottomRight)
	PointType LowerRight() const
	{
		return PointType(right,bottom);
	}

	//!ヒットチェック
	bool PtInRect( const PointType& pt ) const {
		return left <= pt.x && pt.x < right
			&& top <= pt.y && pt.y < bottom;
	}

	Me& UnionStrictRect(
		const Me& rc1,
		const Me& rc2
	)
	{
		this->left   = t_min(rc1.left  , rc2.left);
		this->top    = t_min(rc1.top   , rc2.top);
		this->right  = t_max(rc1.right , rc2.right);
		this->bottom = t_max(rc1.bottom, rc2.bottom);
		return *this;
	}
};

// このクラスはメンバー追加禁止
static_assert(sizeof(CStrictRect<tagPOINT>) == sizeof(tagRECT), "size check");
