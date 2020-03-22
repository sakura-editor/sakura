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

#include "basis/CStrictPoint.h"

/*!
 * 範囲型テンプレートクラス
 *
 * 始点と終点で定義するドキュメント範囲を表す型。
 *
 * メンバ(x,y)の定義型が厳密な数値型の場合、厳密なポイント型となる。
 *
 * 使用例
 * ・CLogicRange	論理行位置と行頭からのオフセット位置を組み合わせて文字データの絶対位置を表す型
 * ・CLayoutRange	レイアウト行位置と桁位置を組み合わせて文字の表示位置を表す型
 */
template <class PointType>
class CRangeBase{

	PointType m_ptFrom;		//!< 始点
	PointType m_ptTo;		//!< 終点

	using Me = CRangeBase<PointType>;
	using UNIT_X_TYPE = decltype(PointType::x);
	using UNIT_Y_TYPE = decltype(PointType::y);

public:
	//! デフォルトコンストラクタ
	CRangeBase() = default;

	/*!
	 * 値指定コンストラクタ
	 */
	constexpr CRangeBase(
		const PointType& ptFrom,
		const PointType& ptTo
	) noexcept
		: m_ptFrom( ptFrom )
		, m_ptTo( ptTo )
	{
	}

	//! デフォルトのコピーコンストラクタを使う
	CRangeBase( const Me& ) = default;
	//! デフォルトのコピー代入演算子を使う
	Me& operator = ( const Me& ) = default;

	//! デフォルトのムーブコンストラクタを使う
	CRangeBase( Me&& ) = default;
	//! デフォルトのムーブ代入演算子を使う
	Me& operator = ( Me&& ) = default;

	/*!
	 * 始点の水平位置を取得する。
	 */
	UNIT_X_TYPE GetFromX() const { return m_ptFrom.x; }

	/*!
	 * 始点の垂直位置を取得する。
	 */
	UNIT_Y_TYPE GetFromY() const { return m_ptFrom.y; }

	/*!
	 * 終点の水平位置を取得する。
	 */
	UNIT_X_TYPE GetToX() const { return m_ptTo.x; }

	/*!
	 * 終点の垂直位置を取得する。
	 */
	UNIT_Y_TYPE GetToY() const { return m_ptTo.y; }

	/*!
	 * 始点を取得する。
	 *
	 * 取得したポイントは変更不可。
	 */
	const PointType& GetFrom() const { return m_ptFrom; }

	/*!
	 * 終点を取得する。
	 *
	 * 取得したポイントは変更不可。
	 */
	const PointType& GetTo() const { return m_ptTo; }

	/*!
	 * 始点の水平位置を設定する。
	 *
	 * 水平方向の基本型に代入できる型の変数であれば設定可能。
	 */
	template<typename X,
		typename = std::enable_if_t< std::is_assignable_v<UNIT_X_TYPE&, X> > >
	void SetFromX( const X& fromX ) { m_ptFrom.x = fromX; }

	/*!
	 * 始点の垂直位置を設定する。
	 *
	 * 垂直方向の基本型に代入できる型の変数であれば設定可能。
	 */
	template<typename Y,
		typename = std::enable_if_t< std::is_assignable_v<UNIT_Y_TYPE&, Y> >>
	void SetFromY( const Y& fromY ) { m_ptFrom.y = fromY; }

	/*!
	 * 終点の水平位置を設定する。
	 *
	 * 水平方向の基本型に代入できる型の変数であれば設定可能。
	 */
	template<typename X,
		typename = std::enable_if_t< std::is_assignable_v<UNIT_X_TYPE&, X> > >
	void SetToX( const X& toX ) { m_ptTo.x = toX; }

	/*!
	 * 終点の垂直位置を設定する。
	 *
	 * 垂直方向の基本型に代入できる型の変数であれば設定可能。
	 */
	template<typename Y,
		typename = std::enable_if_t< std::is_assignable_v<UNIT_Y_TYPE&, Y> > >
	void SetToY( const Y& toY ) { m_ptTo.y = toY; }

	/*!
	 * 始点の位置を設定する。
	 */
	template<typename X, typename Y,
		typename = std::enable_if_t< std::is_assignable_v<UNIT_X_TYPE&, X> && std::is_assignable_v<UNIT_Y_TYPE&, Y> > >
	void SetFrom( const X& fromX, const Y& fromY ) { SetFrom( PointType( fromX, fromY ) ); }

	/*!
	 * 始点の位置を設定する。
	 */
	void SetFrom( const PointType& pt ) { m_ptFrom = pt; }

	/*!
	 * 終点の位置を設定する。
	 */
	template<typename X, typename Y,
		typename = std::enable_if_t< std::is_assignable_v<UNIT_X_TYPE&, X> && std::is_assignable_v<UNIT_Y_TYPE&, Y> > >
	void SetTo( const X& toX, const Y& toY ) { SetTo( PointType( toX, toY ) ); }

	/*!
	 * 終点の位置を設定する。
	 */
	void SetTo( const PointType& pt ) { m_ptTo = pt; }

	//! 削除したい
	PointType* GetFromPointer() { return &m_ptFrom; }
	//! 削除したい
	PointType* GetToPointer() { return &m_ptTo; }
	//! 削除したい
	void SetLine( UNIT_Y_TYPE nY ) {
		m_ptFrom.y = nY;
		m_ptTo.y = nY;
	}
	//! 削除したい
	void SetXs( UNIT_X_TYPE nXFrom, UNIT_X_TYPE nXTo ) {
		m_ptFrom.x = nXFrom;
		m_ptTo.x = nXTo;
	}
	//! 削除したい
	void Set( const PointType& pt ) {
		m_ptFrom = pt;
		m_ptTo = pt;
	}
	//! 削除したい
	void Clear( int n )
	{
		m_ptFrom.Set( UNIT_X_TYPE( n ), UNIT_Y_TYPE( n ) );
		m_ptTo.Set( UNIT_X_TYPE( n ), UNIT_Y_TYPE( n ) );
	}

	/*!
	 * 仕様変更したい
	 *
	 * 説明は「1文字しか選択してない場合」にtrueを返している。
	 * 実装は「始点と終点が一致する場合」にtrueを返している。
	 *
	 * 説明は「1文字しか選択してない場合」にtrueを返している
	 */
	constexpr bool IsOne() const {
		//! 1文字しか選択してない状態ならtrue
		return m_ptFrom == m_ptTo;
	}

	//! 範囲が行内に収まっているか？
	constexpr bool IsLineOne() const { return m_ptFrom.y == m_ptTo.y; }

	//!有効な範囲ならtrue
	constexpr bool IsValid() const {
		return m_ptFrom.BothNatural() && m_ptTo.BothNatural();
	}

	//比較
	bool operator == ( const Me& rhs ) const {
		return m_ptFrom == rhs.m_ptFrom
			&& m_ptTo == rhs.m_ptTo;
	}
	bool operator != ( const Me& rhs ) const { return !(*this == rhs); }
};
