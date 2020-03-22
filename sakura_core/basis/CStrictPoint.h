/*! @file
	Copyright (C) 2008, kobake
	Copyright (C) 2008-2017 SAKURA Editor Project
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

#include <Windows.h> //tagPOINT

/*!
 * 汎用ポイント型テンプレートクラス
 *
 * 型パラメータに渡す基本型を継承するクラスとして動作する。
 * 数値型のメンバ(x,y)を持つ型ならば何でも指定可能。
 *
 * tagPOINT型を渡した場合、tagPOINTを継承する。
 * メンバ(x,y)の定義型が厳密な数値型の場合、厳密なポイント型となる。
 *
 * 使用例
 * ・CLogicPoint	論理行位置と行頭からのオフセット位置を組み合わせて文字データの絶対位置を表す型
 * ・CLayoutPoint	レイアウト行位置と桁位置を組み合わせて文字の表示位置を表す型
 * ・CMyPoint		垂直方向の座標と水平方向の座標を組み合わせて描画位置を表す緩い型
 */
template <typename Base>
class CStrictPoint : public Base {
	using Me = CStrictPoint<Base>;

public:
	using Base::x;
	using Base::y;
	using UNIT_X_TYPE = decltype(Base::x);
	using UNIT_Y_TYPE = decltype(Base::y);

	/*!
	 * デフォルトコンストラクタ
	 *
	 * 新しいインスタンスを構築する。
	 */
	constexpr CStrictPoint() noexcept
		: Me( 0, 0 )
	{
	}

	/*!
	 * 値指定コンストラクタ
	 *
	 * x,yを指定して新しいインスタンスを構築する。
	 */
	template<typename X, typename Y,
		typename = std::enable_if_t< std::is_assignable_v<UNIT_X_TYPE&, X> && std::is_assignable_v<UNIT_Y_TYPE&, Y> > >
	constexpr CStrictPoint( const X xValue, const Y yValue ) noexcept
	{
		Base::x = UNIT_X_TYPE( xValue );
		Base::y = UNIT_Y_TYPE( yValue );
	}

	/*!
	 * 値指定コンストラクタ
	 *
	 * 基本型を指定して新しいインスタンスを構築する。
	 */
	CStrictPoint( const Base& rhs ) noexcept
		: Me( rhs.x, rhs.y )
	{
	}

	/*!
	 * 値指定コンストラクタ
	 *
	 * 他の型からも、「明示的に指定すれば」変換が可能
	 */
	template <class SRC>
	explicit CStrictPoint( const SRC& rhs ) noexcept
		: Me( rhs.x, rhs.y )
	{
	}

	//! デフォルトのコピーコンストラクタを使う
	CStrictPoint( const Me& ) = default;
	//! デフォルトのコピー代入演算子を使う
	Me& operator = ( const Me& ) = default;

	//! デフォルトのムーブコンストラクタを使う
	CStrictPoint( Me&& ) = default;
	//! デフォルトのムーブ代入演算子を使う
	Me& operator = ( Me&& ) = default;

	//! x,y いずれかが 0 より小さい場合に true を返す
	constexpr bool HasNegative() const
	{
		return x < 0 || y < 0;
	}

	//! x,y どちらも自然数であれば true
	constexpr bool BothNatural() const
	{
		return x >= 0 && y >= 0;
	}

	/*!
	 * 水平方向の位置を取得する。
	 */
	constexpr UNIT_X_TYPE GetX() const { return x; }

	/*!
	 * 垂直方向の位置を取得する。
	 */
	constexpr UNIT_Y_TYPE GetY() const { return y; }

	//! 廃止したい
	UNIT_X_TYPE GetX2() const { return x; }
	//! 廃止したい
	UNIT_Y_TYPE GetY2() const { return y; }
	//! 廃止したい
	Base Get() const { return *this; }
	//! 廃止したい
	POINT GetPOINT() const
	{
		POINT pt = { x, y };
		return pt;
	}

	/*!
	 * 水平方向の位置を設定する。
	 *
	 * 水平方向の基本型に代入できる型の変数であれば設定可能。
	 */
	template<typename X,
		typename = std::enable_if_t< std::is_assignable_v<UNIT_X_TYPE&, X> > >
	void SetX( const X& xValue ) { x = UNIT_X_TYPE( xValue ); }

	/*!
	 * 垂直方向の位置を設定する。
	 *
	 * 垂直方向の基本型に代入できる型の変数であれば設定可能。
	 */
	template<typename Y,
		typename = std::enable_if_t< std::is_assignable_v<UNIT_Y_TYPE&, Y> > >
	void SetY( const Y& yValue ) { y = UNIT_Y_TYPE( yValue ); }

	/*!
	 * ポイントの位置を設定する。
	 *
	 * xは、垂直方向の基本型に代入できる型の変数であれば設定可能。
	 * yは、垂直方向の基本型に代入できる型の変数であれば設定可能。
	 */
	template<typename X, typename Y,
		typename = std::enable_if_t< std::is_assignable_v<UNIT_X_TYPE&, X> && std::is_assignable_v<UNIT_Y_TYPE&, Y> > >
	void Set( const X& xValue, const Y& yValue ) {
		SetX( xValue );
		SetY( yValue );
	}

	/*!
	 * ポイントの位置を設定する。
	 *
	 * Uは、数値型のメンバx,yを持つ型でなければならない。
	 * xは、垂直方向の基本型に代入できる型の変数であれば設定可能。
	 * yは、垂直方向の基本型に代入できる型の変数であれば設定可能。
	 */
	template<typename U>
	void Set( const U& pt ) { Set( pt.x, pt.y ); }

	/*!
	 * ポイントの位置をクリアする。
	 */
	void Clear() { Set( 0, 0 ); }

	/*!
	 * ポイントの位置をずらす。
	 */
	template<typename X, typename Y>
	void Offset( const X& xValue, const Y& yValue ) { Set( x + xValue, y + yValue ); }

	/*!
	 * ポイントの位置をずらす。
	 */
	void Offset( const Base& pt ) { Set( x + pt.x, y + pt.y ); }

	//算術演算子
	Me& operator += ( const Base& rhs ) { x += rhs.x; y += rhs.y; return *this; }
	Me& operator -= ( const Base& rhs ) { x -= rhs.x; y -= rhs.y; return *this; }
	Me& operator *= ( int n ) { x *= n; y *= n; return *this; }
	Me& operator /= ( int n ) { x /= n; y /= n; return *this; }

	//算術演算子２
	Me operator + ( const Base& rhs ) const { Me ret( *this ); ret += rhs; return ret; }
	Me operator - ( const Base& rhs ) const { Me ret( *this ); ret -= rhs; return ret; }
	Me operator * ( int n ) const { Me ret( *this ); ret *= n; return ret; }
	Me operator / ( int n ) const { Me ret( *this ); ret /= n; return ret; }

	//代入演算子
	Me& operator = ( const Base& rhs ) { x = rhs.x; y = rhs.y; return *this; }

	//比較演算子
	bool operator == ( const Base& rhs ) const { return x == rhs.x && y == rhs.y; }
	bool operator != ( const Base& rhs ) const { return !(*this == rhs); }
};

// このクラスはメンバー追加禁止
static_assert(sizeof(CStrictPoint<tagPOINT>) == sizeof(tagPOINT), "size check");
