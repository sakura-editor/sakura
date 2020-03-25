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

#include <string>
#include <algorithm>
#include "debug/Debug2.h" //assert_warning

/*!
 * 文字列参照型
 *
 * C-Styleの文字列ポインタを安全に扱うためのクラス。
 *
 * クラス外に確保されたメモリ領域を文字列として参照する。
 * メモリと切り離すことにより、文字列参照のための汎用クラスとして機能する。
 *
 * このクラスは無効値(=NULL)を保持することができる。
 * このクラスは文字列を変更するインターフェースを提供しない。
 */
template <typename CHAR_TYPE>
class StringRef {
	CHAR_TYPE*	m_pData;			//!< 文字列バッファを指すポインタ
	size_t		m_cchDataLength;	//!< 有効文字列長

	using Me = StringRef<CHAR_TYPE>;
	using Traits = std::char_traits<CHAR_TYPE>;

public:
	//! デフォルトコンストラクタ
	constexpr StringRef() noexcept
		: Me( NULL, 0 )
	{
	}

	/*!
	 * コンストラクタ
	 *
	 * StringRefのインスタンスを構築する
	 *
	 * @param [in] pszData C-Styleの文字列ポインタ。
	 */
	StringRef( const CHAR_TYPE* pszData ) noexcept
		: Me( pszData, 0 )
	{
		// 文字列ポインタが有効な場合、有効文字列長を求める
		if( pszData != NULL ){
			m_cchDataLength = Traits::length( pszData );
		}
	}

	/*!
	 * コンストラクタ
	 *
	 * StringRefのインスタンスを構築する
	 *
	 * @param [in] pszData C-Styleの文字列ポインタ。
	 * @param [in] cchDataLength 文字列バッファの有効文字列長。
	 */
	constexpr StringRef( const CHAR_TYPE* pszData, size_t cchDataLength ) noexcept
		: m_pData( const_cast<CHAR_TYPE*>(pszData) )
		, m_cchDataLength( cchDataLength )
	{
	}

	//! デフォルトのコピーコンストラクタを使う
	StringRef( const Me& other ) = default;
	//! デフォルトのコピー代入演算子を使う
	Me& operator = ( const Me& ) = default;

	//! デフォルトのムーブコンストラクタを使う
	StringRef( Me&& other ) = default;
	//! デフォルトのムーブ代入演算子を使う
	Me& operator = ( Me&& rhs ) = default;

	//! デフォルトのデストラクタを使う
	~StringRef( void ) = default;

	//! 文字列参照が有効かどうか判定する。
	constexpr bool IsValid() const noexcept { return c_str() != nullptr; }

	//! 文字列の参照ポインタ(C-Style)を取得する。
	constexpr const CHAR_TYPE* c_str() const noexcept { return m_pData; }

	//! 有効文字列長(文字単位)を取得する。
	constexpr size_t length() const noexcept { return m_cchDataLength; }

	/*!
	 * 任意位置の文字を取得する
	 *
	 * @remark 文字列データを変更する場合、lengthの更新を忘れずに行うこと。
	 */
	constexpr CHAR_TYPE at( size_t nIndex ) const noexcept {
		assert_warning( m_pData != NULL );
		assert_warning( nIndex < length() );
		if( nIndex < length() ) {
			return m_pData[nIndex];
		}
		return '\0';
	}

	//! 任意位置の文字を取得する
	constexpr CHAR_TYPE operator[]( size_t nIndex ) const noexcept { return at( nIndex ); }

	/*!
	 * 文字列の参照ポインタ(C-Style)を取得する。
	 *
	 * 旧CStringRefが定義していた関数。
	 * 利用箇所を修正して回るのが面倒なので一旦エイリアスを作る
	 */
	const CHAR_TYPE* GetPtr() const noexcept { return c_str(); }

	/*!
	 * 有効文字列長(文字単位)を取得する。
	 *
	 * 旧CStringRefが定義していた関数。
	 * 利用箇所を修正して回るのが面倒なので一旦エイリアスを作る
	 */
	int GetLength()	const noexcept { return static_cast<int>(length()); }

	/*!
	 * 任意位置の文字を取得する
	 *
	 * 旧CStringRefが定義していた関数。
	 * 利用箇所を修正して回るのが面倒なので一旦エイリアスを作る
	 */
	wchar_t At( size_t nIndex )	const noexcept { return at( nIndex ); }

protected:
	/*!
	 * 参照先を引数で指定された部分文字列に置き替える
	 *
	 * @param [in,opt] pData C-Styleの文字列ポインタ(NULL指定可、NUL終端不要)
	 * @param [in] cchDataLength 有効文字列長(省略不可)
	 */
	constexpr void reset( const CHAR_TYPE* pData, size_t cchDataLength ) noexcept
	{
		m_pData = const_cast<CHAR_TYPE*>(pData);
		m_cchDataLength = cchDataLength;
	}

	/*!
	 * 参照先を引数で指定された部分文字列に置き替える
	 *
	 * @param [in] rhs 文字列参照
	 */
	constexpr void reset( const Me& rhs ) noexcept
	{
		reset( rhs.c_str(), rhs.length() );
	}

	/*!
	 * 引数で指定されたインスタンスとデータを入れ替える
	 */
	constexpr void swap( Me& rhs ) noexcept
	{
		std::swap( m_pData, rhs.m_pData );
		std::swap( m_cchDataLength, rhs.m_cchDataLength );
	}

public:
	/*!
	 * 同型との比較
	 *
	 * @param rhs 比較対象
	 * @retval < 0 自身がメモリ未確保、かつ、比較対象はメモリ確保済み
	 * @retval < 0 データ値が比較対象より小さい
	 * @retval < 0 データが比較対象の先頭部分と一致する、かつ、データ長が比較対象より小さい
	 * @retval == 0 比較対象が自分自身の参照
	 * @retval == 0 比較対象と等しい
	 * @retval == 0 自身がメモリ未確保、かつ、比較対象がメモリ未確保
	 * @retval > 0 自身が確保済み、かつ、比較対象がメモリ未確保
	 * @retval > 0 データ値が比較対象より大きい
	 * @retval > 0 データの先頭部分が比較対象と一致する、かつ、データ長が比較対象より大きい
	 */
	int compare( const Me& rhs ) const noexcept
	{
		// 比較対象が自インスタンスであれば等しい
		if( this == &rhs ) return 0;

		// メモリ確保状況をチェックする
		const int lhsIsValid = static_cast<int>(IsValid());
		const int rhsIsValid = static_cast<int>(rhs.IsValid());

		// 片方でも未確保なら、メモリ内容によらず差異が確定する
		if( rhsIsValid == 0 || lhsIsValid == 0 ) return lhsIsValid - rhsIsValid;

		// データ長が短い方を基準に比較を行う
		const ptrdiff_t lhsLength = static_cast<ptrdiff_t>(length());
		const ptrdiff_t rhsLength = static_cast<ptrdiff_t>(rhs.length());
		const ptrdiff_t minLength = std::min( lhsLength, rhsLength );

		// データ長の範囲で文字列を比較する
		auto cmp = Traits::compare( c_str(), rhs.c_str(), minLength );
		if( cmp == 0 ) cmp = lhsLength - rhsLength;
		return cmp;
	}

	//! 等価比較演算子
	bool operator == ( const Me& rhs ) const noexcept { return 0 == compare( rhs ); }

	//! 否定の等価比較演算子
	bool operator != ( const Me& rhs ) const noexcept { return !(*this == rhs); }
};

/*!
 * 文字列ポインタ型との等価比較
 *
 * @param lhs 比較対象(StringRef)
 * @param rhs 比較対象(C string)
 * @retval true 等しい
 * @retval false 等しくない
 */
template <typename CHAR_TYPE>
bool operator == (
	const StringRef<CHAR_TYPE>&		lhs,
	const CHAR_TYPE*				rhs
	) noexcept
{
	const StringRef<CHAR_TYPE> tmp( rhs );
	return 0 == lhs.compare( tmp );
}

/*!
 * 文字列ポインタ型との否定の等価比較
 *
 * @param lhs 比較対象(StringRef)
 * @param rhs 比較対象(C string)
 * @retval true 等しくない
 * @retval false 等しい
 */
template <typename CHAR_TYPE>
bool operator != (
	const StringRef<CHAR_TYPE>&		lhs,
	const CHAR_TYPE*				rhs
	) noexcept
{
	return !(lhs == rhs);
}

/*!
 * 文字列ポインタ型との等価比較(引数逆転版)
 *
 * @param lhs 比較対象(C string)
 * @param rhs 比較対象(StringRef)
 * @retval true 等しい
 * @retval false 等しくない
 */
template <typename CHAR_TYPE>
bool operator == (
	const CHAR_TYPE*				lhs,
	const StringRef<CHAR_TYPE>&		rhs
	) noexcept
{
	const StringRef<CHAR_TYPE> tmp( lhs );
	return 0 == tmp.compare( rhs );
}

/*!
 * 文字列ポインタ型との否定の等価比較(引数逆転版)
 *
 * @param lhs 比較対象(C string)
 * @param rhs 比較対象(StringRef)
 * @retval true 等しくない
 * @retval false 等しい
 */
template <typename CHAR_TYPE>
bool operator != (
	const CHAR_TYPE*				lhs,
	const StringRef<CHAR_TYPE>&		rhs
	) noexcept
{
	return !(lhs == rhs);
}

/*!
 * 文字列の参照を保持する型
 *
 * 「文字列」とは、0個以上の文字で構成されるシーケンスである。
 * サクラエディタは、内部データを変則UTF16LEエンコーディングで保持する。
 * これは Windows の内部コードと酷似しているため、いわゆるW版APIに文字列として渡すことができる。
 * 本質的にはかなり特殊なコード体系なので、「コード変換」を含むコンテキストで使う場合には注意が必要。
 *
 * 変則UTF16LEエンコーディングの「変則」部分についてはCCodeBase派生クラスを参照。
 */
using CStringRefW = StringRef<wchar_t>;

/*!
 * バイトシーケンスの参照を保持する型
 *
 * 「バイトシーケンス」とは、0個以上のバイトデータで構成されるシーケンスである。
 * サクラエディタは、内部データを変則UTF16LEエンコーディングで保持する。
 * ファイルなどの外部データを内部データに取り込む、または、
 * 内部データをファイルなどの外部データに書き込む際に行う「コード変換」で利用する型である。
 *
 * この型は、単体でエンコーディング情報を保持しないため、
 * 保持されるデータのエンコーディングはコンテキストに依存する。
 *
 * この型によるデータの長期保持はおすすめしない。
 */
using CStringRefA = StringRef<unsigned char>;
