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
#include "basis/CStringRef.h" //StringRef
#include "debug/Debug2.h" //assert_warning

/*!
 * バッファ参照型
 *
 * 書込み可能なC-Styleの文字列ポインタを安全に扱うためのクラス。
 *
 * クラス外に確保されたメモリ領域を文字列として参照する。
 * メモリと切り離すことにより、文字列バッファのための汎用クラスとして機能する。
 *
 * このクラスは無効値(=NULL)を保持することができる。
 * このクラスはバッファ拡張のためのインターフェースを提供しない。
 */
template <typename CHAR_TYPE>
class StringBuf : public StringRef<CHAR_TYPE> {
	size_t		m_cchDataCount;		//!< 文字列バッファのサイズ

	using Me = StringBuf<CHAR_TYPE>;
	using Base = StringRef<CHAR_TYPE>;
	using Traits = std::char_traits<CHAR_TYPE>;

protected:
	/*!
	 * デフォルトコンストラクタ
	 *
	 * StringBufのインスタンスを構築する
	 *
	 * この型は、書込み可能なバッファを操作するためのものなので、
	 * 書込み先バッファを指定しないコンストラクタは提供しない。
	 */
	constexpr StringBuf() noexcept
		: Me( NULL, 0 )
	{
	}

public:
	/*!
	 * コンストラクタ
	 *
	 * StringBufのインスタンスを構築する
	 *
	 * @param [in] pszData 文字列バッファを指すポインタ
	 * @param [in] cchDataCount 文字列バッファの確保サイズ(length + 1)
	 * @param [in,opt] cchDataLength 有効文字列長(length)
	 */
	constexpr StringBuf(
		CHAR_TYPE*			pszData,
		size_t				cchDataCount,
		size_t				cchDataLength = 0
	) noexcept
		: Base( pszData, cchDataLength )
		, m_cchDataCount( cchDataCount )
	{
	}

	/*!
	 * 配列用コンストラクタ
	 *
	 * StringBufのインスタンスを構築する
	 *
	 * @param [in] szData 文字列バッファとして使う配列
	 */
	template<size_t cchDataCount>
	StringBuf( CHAR_TYPE (&szData)[cchDataCount] )
		: Me( szData, cchDataCount, 0 )
	{
		if( szData != NULL ){
			szData[m_cchDataCount - 1] = '\0';
			auto length = Traits::length( szData );
			assign( szData, length );
		}
	}

	/*!
	 * コピーコンストラクタ
	 *
	 * 引数で指定された部分文字列を参照するように設定する
	 */
	StringBuf( const Me& other )
		: Me( NULL, 0 )
	{
		*this = other;
	}

	/*!
	 * ムーブコンストラクタ
	 *
	 * 空のStringBufインスタンスを構築し、
	 * 引数で指定されたインスタンスとデータを入れ替える
	 */
	StringBuf( Me&& other ) noexcept
		: Me( NULL, 0 )
	{
		*this = std::forward<Me>( other );
	}

	//! 文字列バッファのサイズ(文字単位)を取得する
	constexpr size_t capacity() const noexcept { return m_cchDataCount; }

	/*!
	 * 参照先の文字列ポインタ(C-Style)を取得する
	 *
	 * @remark 文字列データを変更する場合、lengthの更新を忘れずに行うこと。
	 */
	CHAR_TYPE* data() noexcept {
		return const_cast<CHAR_TYPE*>(Base::c_str());
	}

protected:
	/*!
	 * 引数で指定された部分文字列を参照するように設定する
	 *
	 * @param [in] rhs 文字列参照
	 */
	constexpr void reset( const Me& rhs ) noexcept
	{
		// 親クラスをリセット
		Base::reset( rhs );

		// バッファいサイズをリセット
		m_cchDataCount = rhs.m_cchDataCount;
	}

	/*!
	 * 引数で指定されたインスタンスとデータを入れ替える
	 */
	constexpr void swap( Me& rhs ) noexcept
	{
		// 親クラスを入れ替える
		Base::swap( rhs );

		// バッファいサイズを入れ替える
		std::swap( m_cchDataCount, rhs.m_cchDataCount );
	}

public:
	/*!
	 * バッファの内容を指定した文字列で置き替える
	 *
	 * @param [in] rhs 文字列
	 */
	virtual void assign( const Base& rhs ) {
		// コピー元が自分自身なら何もせず抜ける
		if( this == &rhs ){
			return;
		}
		assign( rhs.c_str(), rhs.length() );
	}

	/*!
	 * バッファの内容を指定した文字列ポインタから始まる部分文字列で置き替える
	 *
	 * @param [in] pData 文字列ポインタ
	 * @param [in] cchDataCount 文字数
	 */
	virtual void assign( const CHAR_TYPE* pData, size_t cchDataCount ){
		assert_warning( Base::IsValid() );
		if( Base::IsValid() ){
			// コピーする文字数は、指定された文字数と許容値の小さいほう
			auto cchToCopy = std::min<size_t>( cchDataCount, capacity() - 1 );
			assert_warning( cchToCopy == cchDataCount );

			// コピーする文字数が0より大きい、かつ、コピー元が内部バッファ以外
			if( 0 < cchToCopy && pData != data() ){
				// コピー元がNULL以外
				assert_warning( pData != NULL );
				if( pData != NULL ){
					Traits::copy( data(), pData, cchToCopy );
				}
			}

			// 末尾をNUL終端して有効文字列長を更新
			data()[cchToCopy] = '\0';
			size_t cchDataLength = Traits::length( data() );
			Base::reset( data(), cchDataLength );
		}
	}

	/*!
	 * バッファの末尾に指定した文字列を追加する
	 *
	 * @param [in] rhs 文字列
	 */
	void append( const Base& rhs ) {
		append( rhs.c_str(), rhs.length() );
	}

	/*!
	 * バッファの末尾に指定した文字列ポインタから始まる部分文字列を追加する
	 *
	 * @param [in] pData 文字列ポインタ
	 * @param [in] cchDataCount 文字数
	 */
	virtual void append( const CHAR_TYPE* pData, size_t cchDataCount ) {
		assert_warning( Base::IsValid() );
		if( Base::IsValid() && 0 < cchDataCount ){
			auto length = Base::length();
			auto cchToCopy = std::min<size_t>( cchDataCount, capacity() - length - 1 );
			assert_warning( cchDataCount == cchToCopy );
			assert_warning( pData != NULL );
			assert_warning( pData != &data()[length] );
			if( pData != NULL && pData != &data()[length] && 0 < cchToCopy ){
				Traits::copy( &data()[length], pData, cchToCopy );
			}
			auto newLength = length + cchToCopy;
			data()[newLength] = '\0';
			Base::reset( data(), newLength );
		}
	}

	//! コピー代入演算子 引数で指定されたインスタンスが参照する部分文字列を見るように設定する
	constexpr Me& operator = ( const Me& rhs ) { reset( rhs ); return *this; }

	//! ムーブ代入演算子 指定されたインスタンスとデータを入れ替える
	constexpr Me& operator = ( Me&& rhs ) noexcept { swap( rhs ); return *this; }

	//! 代入演算子 バッファの内容を指定した文字列で置き替える
	Me& operator = ( const Base& rhs ) { assign( rhs ); return *this; }

	//! 加算代入演算子 バッファの末尾に指定した文字列を追加する
	Me& operator += ( const Base& rhs ) { append( rhs ); return *this; }
};

/*!
 * 文字列バッファ型
 */
using CStringBufW = StringBuf<wchar_t>;

/*!
 * バイトシーケンスのバッファ型
 */
using CStringBufA = StringBuf<unsigned char>;
