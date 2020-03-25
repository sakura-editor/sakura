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
#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>

#include "basis/CStringBuf.h"
#include <stdexcept>
#include <gtest/gtest.h>

/*!
 * @brief コンストラクタ
 */
TEST(CStringBufW, ConstructWithNormalBuffer)
{
	// テキトーなサイズのバッファから構築
	wchar_t buf[MAX_PATH] = { 0 };
	CStringBufW value( buf );

	// 構築されたバッファは元のバッファと同じ。
	EXPECT_EQ(buf, value.c_str());
	EXPECT_EQ(0, value.length());
	EXPECT_EQ(_countof( buf ), value.capacity());

	// 文字列を代入してみる。
	constexpr const wchar_t sz1[] = L"abc";
	constexpr const size_t cch = _countof(sz1) - 1;
	value = sz1;
	ASSERT_STREQ(sz1, value.c_str());
	EXPECT_EQ(cch, value.length());
	EXPECT_EQ(_countof(buf), value.capacity());

	// 文字列を追加してみる。
	constexpr const wchar_t sz2[] = L"def";
	constexpr const wchar_t sz3[] = L"abcdef";
	constexpr const size_t cch3 = _countof(sz3) - 1;
	value += sz2;
	ASSERT_STREQ(sz3, value.c_str());
	EXPECT_EQ(cch3, value.length());
	EXPECT_EQ(_countof(buf), value.capacity());
}

/*!
 * @brief コンストラクタ
 */
TEST( CStringBufW, ConstructWithNullBuffer )
{
	// NULLを指定して構築
	CStringBufW value( NULL, 0 );

	// 構築されたバッファは元のバッファと同じ。
	EXPECT_EQ( NULL, value.c_str() );
	EXPECT_EQ( 0, value.length() );
	EXPECT_EQ( 0, value.capacity() );

	// 文字列を代入してみる。(何も起きない)
	constexpr const wchar_t sz1[] = L"abc";
	value = sz1;
	EXPECT_EQ( NULL, value.c_str() );
	EXPECT_EQ( 0, value.length() );
	EXPECT_EQ( 0, value.capacity() );

	// 文字列を追加してみる。(何も起きない)
	constexpr const wchar_t sz2[] = L"def";
	value += sz2;
	EXPECT_EQ( NULL, value.c_str() );
	EXPECT_EQ( 0, value.length() );
	EXPECT_EQ( 0, value.capacity() );
}

/*!
 * @brief コピーコンストラクタの仕様
 * @remark 新しいインスタンスの属性はコピー元と同じになる
 */
TEST(CStringBufW, ConstructFromOtherByCopy)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof( sz ) - 1;
	wchar_t buf[cch] = { 0 };
	CStringBufW other( buf );
	other = sz;
	CStringBufW value(other);
	ASSERT_STREQ(other.c_str(), value.c_str());
	EXPECT_EQ(other.length(), value.length());

	// コピー元と同じ参照になる
	ASSERT_EQ( other, value );
}

/*!
 * @brief ムーブコンストラクタの仕様
 * @remark 新しいインスタンスの属性はムーブ元と同じになる
 * @remark ムーブ元は抜け殻になる
 */
TEST(CStringBufW, ConstructFromOtherByMove)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	wchar_t buf[cch + 1] = { 0 };
	CStringBufW other( buf );
	other = sz;
	CStringBufW value(std::move(other));
	ASSERT_STREQ(sz, value.c_str());
	EXPECT_EQ(cch, value.length());

	// ムーブ元は抜け殻になる
	ASSERT_EQ(NULL, other.c_str());
	EXPECT_EQ(0, other.length());
	EXPECT_EQ(0, other.capacity());
}

/*!
 * @brief コピー代入演算子の仕様
 */
TEST(CStringBufW, CopyFromOther)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	wchar_t buf[cch + 1] = { 0 };
	wchar_t tmp[cch + 1] = { 0 };
	CStringBufW value( tmp );
	CStringBufW other( buf );
	other = sz;
	value = other;
	ASSERT_STREQ(sz, value.c_str());
	EXPECT_EQ(cch, value.length());

	// コピー元と同じ参照になる
	ASSERT_EQ( other, value );
}

/*!
 * @brief ムーブ代入演算子の仕様
 * @remark ムーブ元は抜け殻にならない
 */
TEST(CStringBufW, MoveFromOther)
{
	constexpr const wchar_t sz1[] = L"test";
	wchar_t buf[_countof(sz1)] = { 0 };
	wchar_t tmp[MAX_PATH] = { 0 };
	CStringBufW value( tmp );
	CStringBufW other( buf );
	other = sz1;
	value = std::move(other);
	ASSERT_STREQ(sz1, value.c_str());
	EXPECT_EQ(_countof(sz1) - 1, value.length());
	EXPECT_EQ(_countof(buf), value.capacity());

	// ムーブ元とムーブ先のデータが入れ替わる
	ASSERT_EQ(tmp, other.c_str());
	EXPECT_EQ(0, other.length());
	EXPECT_EQ(_countof(tmp), other.capacity());
}

/*!
 * @brief コンストラクタ
 */
TEST( CStringBufW, Assign1 )
{
	// テキトーなサイズのバッファから構築
	wchar_t buf[MAX_PATH] = { 0 };
	CStringBufW cBuf( buf );

	constexpr const wchar_t sz1[] = L"abc";
	constexpr const wchar_t sz2[] = L"def";
	constexpr const wchar_t sz3[] = L"abcdef";

	// 文字列を代入する。
	cBuf = sz1;
	ASSERT_STREQ( sz1, cBuf.c_str() );
	EXPECT_EQ( _countof( sz1 ) - 1, cBuf.length() );
	EXPECT_EQ( _countof( buf ), cBuf.capacity() );

	// NULLを代入する(クリアする)。
	cBuf = NULL;
	ASSERT_EQ( buf, cBuf.c_str() );
	EXPECT_EQ( 0, cBuf.length() );
	EXPECT_EQ( _countof( buf ), cBuf.capacity() );

	// 文字列を追加する(空文字への追加は代入と同じ結果になる)。
	cBuf += sz1;
	ASSERT_STREQ( sz1, cBuf.c_str() );
	EXPECT_EQ( _countof( sz1 ) - 1, cBuf.length() );
	EXPECT_EQ( _countof( buf ), cBuf.capacity() );

	// 文字列を追加する。
	cBuf += sz2;
	ASSERT_STREQ( sz3, cBuf.c_str() );
	EXPECT_EQ( _countof( sz3 ) - 1, cBuf.length() );
	EXPECT_EQ( _countof( buf ), cBuf.capacity() );

	// NULLを追加する(何も起きない)。
	cBuf += NULL;
	ASSERT_STREQ( sz3, cBuf.c_str() );
	EXPECT_EQ( _countof( sz3 ) - 1, cBuf.length() );
	EXPECT_EQ( _countof( buf ), cBuf.capacity() );
}

