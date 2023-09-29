/*! @file */
/*
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
#include <stdexcept>
#include <gtest/gtest.h>
#include "charset/charcode.h"
#include "mem/CNativeW.h"
#include "mem/CNativeA.h"

// NULL定義補正
// 詳細はStdAfx.hを参照
#ifdef __cplusplus
# pragma warning( push )
# pragma warning( disable : 4005 )
# define NULL nullptr
# pragma warning( pop )
#endif // end of #ifdef __cplusplus

/*!
	CStringRefのテスト
 */
TEST(CStringRef, CStringRef)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;

	CStringRef v1;
	EXPECT_EQ(NULL, v1.GetPtr());
	EXPECT_EQ(0, v1.GetLength());
	EXPECT_FALSE(v1.IsValid());
	EXPECT_EQ(L'\0', v1.At(0));

	CStringRef v2(sz, cch);
	EXPECT_STREQ(sz, v2.GetPtr());
	EXPECT_EQ(cch, v2.GetLength());
	EXPECT_TRUE(v2.IsValid());
	EXPECT_EQ(L't', v2.At(0));
	EXPECT_EQ(L'e', v2.At(1));
	EXPECT_EQ(L's', v2.At(2));
	EXPECT_EQ(L't', v2.At(3));
	EXPECT_EQ(L'\0', v2.At(4));

	CNativeW cmem(sz, cch);
	CStringRef v3(cmem);
	EXPECT_STREQ(sz, v3.GetPtr());
	EXPECT_EQ(cch, v3.GetLength());
	EXPECT_TRUE(v3.IsValid());
	EXPECT_EQ(L't', v3.At(0));
	EXPECT_EQ(L'e', v3.At(1));
	EXPECT_EQ(L's', v3.At(2));
	EXPECT_EQ(L't', v3.At(3));
	EXPECT_EQ(L'\0', v3.At(4));
}

/*!
 * @brief コンストラクタ(パラメータなし)の仕様
 * @remark バッファは確保されない
 * @remark 文字列長はゼロになる
 * @remark バッファサイズはゼロになる
 */
TEST(CNativeW, ConstructWithoutParam)
{
	CNativeW value;
	ASSERT_EQ(NULL, value.GetStringPtr());
	EXPECT_EQ(0, value.GetStringLength());
	EXPECT_EQ(0, value.capacity());
}

/*!
 * @brief コンストラクタ(文字列と文字列長指定)の仕様
 * @remark バッファが確保される
 * @remark 文字列長は指定した文字列長になる
 * @remark バッファサイズは指定した文字列を格納できるサイズになる
 */
TEST(CNativeW, ConstructWithStringWithLength)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CNativeW value(sz, cch);
	ASSERT_STREQ(sz, value.GetStringPtr());
	EXPECT_EQ(cch, value.GetStringLength());
	EXPECT_LE(cch, value.capacity());
}

/*!
 * @brief コンストラクタ(文字列指定)の仕様
 * @remark バッファが確保される
 * @remark 文字列長は指定した文字列の文字列長になる
 * @remark バッファサイズは指定した文字列を格納できるサイズになる
 */
TEST(CNativeW, ConstructWithString)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CNativeW value(sz);
	ASSERT_STREQ(sz, value.GetStringPtr());
	EXPECT_EQ(cch, value.GetStringLength());
	EXPECT_LE(cch, value.capacity());
}

/*!
 * @brief コンストラクタ(空文字列)の仕様
 * @remark 普通の文字列指定と同じ挙動になる
 */
TEST(CNativeW, ConstructWithStringEmpty)
{
	constexpr const wchar_t sz[] = L"";
	CNativeW value(sz);
	ASSERT_STREQ(sz, value.GetStringPtr());
	EXPECT_EQ(0, value.GetStringLength());
	EXPECT_LE(0, value.capacity());
}

/*!
 * @brief コンストラクタ(NULL指定)の仕様
 * @remark バッファは確保されない
 * @remark 文字列長はゼロになる
 */
TEST(CNativeW, ConstructWithStringNull)
{
	CNativeW value(NULL);
	EXPECT_EQ(0, value.GetStringLength());
	EXPECT_EQ(NULL, value.GetStringPtr());

	CNativeW value2(NULL);
	EXPECT_EQ(0, value2.GetStringLength());
	EXPECT_EQ(NULL, value2.GetStringPtr());
}

/*!
 * @brief コピーコンストラクタの仕様
 * @remark 新しいインスタンスの属性はコピー元と同じになる
 * @remark コピー元バッファとは別に新しいバッファが確保される
 */
TEST(CNativeW, ConstructFromOtherByCopy)
{
	constexpr const wchar_t sz[] = L"test";
	CNativeW other(sz);
	CNativeW value(other);
	ASSERT_STREQ(other.GetStringPtr(), value.GetStringPtr());
	EXPECT_EQ(other.GetStringLength(), value.GetStringLength());
	EXPECT_EQ(other.capacity(), value.capacity());

	// コピー元バッファとは別に新しいバッファが確保される
	ASSERT_NE(other.GetStringPtr(), value.GetStringPtr());
}

/*!
 * @brief ムーブコンストラクタの仕様
 * @remark 新しいインスタンスの属性はムーブ元と同じになる
 * @remark ムーブ元は抜け殻になる
 */
TEST(CNativeW, ConstructFromOtherByMove)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CNativeW other(sz);
	CNativeW value(std::move(other));
	ASSERT_STREQ(sz, value.GetStringPtr());
	EXPECT_EQ(cch, value.GetStringLength());
	EXPECT_LE(cch, value.capacity());

	// ムーブ元は抜け殻になる
	ASSERT_EQ(NULL, other.GetStringPtr());
	EXPECT_EQ(0, other.GetStringLength());
	EXPECT_EQ(0, other.capacity());
}

/*!
 * @brief コピー代入演算子の仕様
 * @remark バッファが確保される
 * @remark 文字列長は指定した文字列の文字列長になる
 * @remark バッファサイズは指定した文字列を格納できるサイズになる
 * @remark コピー元バッファとは別に新しいバッファが確保される
 */
TEST(CNativeW, CopyFromOther)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CNativeW value;
	CNativeW other(sz);
	value = other;
	ASSERT_STREQ(sz, value.GetStringPtr());
	EXPECT_EQ(cch, value.GetStringLength());
	EXPECT_LE(cch, value.capacity());

	// コピー元バッファとは別に新しいバッファが確保される
	ASSERT_NE(other.GetStringPtr(), value.GetStringPtr());
}

/*!
 * @brief ムーブ代入演算子の仕様
 * @remark バッファが確保される
 * @remark 文字列長は指定した文字列の文字列長になる
 * @remark バッファサイズは指定した文字列を格納できるサイズになる
 * @remark ムーブ元は抜け殻になる
 */
TEST(CNativeW, MoveFromOther)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CNativeW value;
	CNativeW other(sz);
	value = std::move(other);
	ASSERT_STREQ(sz, value.GetStringPtr());
	EXPECT_EQ(cch, value.GetStringLength());
	EXPECT_LE(cch, value.capacity());

	// ムーブ元は抜け殻になる
	ASSERT_EQ(NULL, other.GetStringPtr());
	EXPECT_EQ(0, other.GetStringLength());
	EXPECT_EQ(0, other.capacity());
}

/*!
 * @brief 添字演算子の仕様
 * @remark 指定した位置にあるwchar_tを返す
 * @remark インデックス指定が範囲外の場合 NUL を返す
 * @remark 確保領域を超える位置を指定してもエラーにならない
 */
TEST(CNativeW, GetCharAtIndex)
{
	constexpr const wchar_t sz[] = L"森鷗外";
	constexpr const size_t cch = _countof(sz) - 1;
	CNativeW value(sz, cch);
	EXPECT_EQ(cch, value.GetStringLength());
	for (size_t index = 0; index < cch; ++index) {
		EXPECT_EQ(sz[index], value[index]);
	}
	ASSERT_EQ(0, value[cch]);
	EXPECT_EQ(0, value[value.capacity() + 1]);
}

/*!
 * @brief 代入演算子(文字列指定)の仕様
 * @remark バッファが確保される
 * @remark 文字列長は指定した文字列の文字列長になる
 * @remark バッファサイズは指定した文字列を格納できるサイズになる
 */
TEST(CNativeW, AssignString)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CNativeW value;
	value = sz;
	ASSERT_STREQ(sz, value.GetStringPtr());
	EXPECT_EQ(cch, value.GetStringLength());
	EXPECT_LE(cch, value.capacity());
}

/*!
 * @brief 代入演算子(NULL指定)の仕様
 * @remark バッファを確保している場合は解放される
 * @remark 文字列長はゼロになる
 */
TEST(CNativeW, AssignStringNullPointer)
{
	CNativeW value(L"test");
	value = NULL;
	EXPECT_EQ(0, value.GetStringLength());
	EXPECT_EQ(NULL, value.GetStringPtr());
}

/*!
 * @brief 代入演算子(NULL指定)の仕様
 * @remark バッファを確保している場合は解放される
 * @remark 文字列長はゼロになる
 */
TEST(CNativeW, AssignStringNullLiteral)
{
	CNativeW value(L"test");
	value = NULL;
	ASSERT_EQ(NULL, value.GetStringPtr());
	EXPECT_EQ(0, value.GetStringLength());
}

/*!
 * @brief 加算代入演算子(文字指定)の仕様
 * @remark バッファが確保される
 * @remark 文字列長は演算子呼出前の文字列長+1になる
 * @remark バッファサイズは連結された文字列を格納できるサイズになる
 */
TEST(CNativeW, AppendChar)
{
	constexpr const wchar_t sz[] = L"X";
	constexpr const size_t cch = _countof(sz) - 1;
	CNativeW value;
	value += sz[0];
	ASSERT_STREQ(sz, value.GetStringPtr());
	EXPECT_EQ(1, value.GetStringLength());
	EXPECT_LE(1, value.capacity());
}

/*!
 * @brief 加算代入演算子(文字列指定)の仕様
 * @remark バッファが確保される
 * @remark 文字列長は指定した文字列の文字列長になる
 * @remark バッファサイズは指定した文字列を格納できるサイズになる
 */
TEST(CNativeW, AppendString)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CNativeW value;
	value += sz;
	ASSERT_STREQ(sz, value.GetStringPtr());
	EXPECT_EQ(cch, value.GetStringLength());
	EXPECT_LE(cch, value.capacity());
}

/*!
 * @brief 加算代入演算子(NULL指定)の仕様
 * @remark 加算代入しても内容に変化無し
 */
TEST(CNativeW, AppendStringNullPointer)
{
	CNativeW org(L"orz");
	CNativeW value(org);
	value += NULL;
	EXPECT_EQ(value.GetStringLength(), org.GetStringLength());
	EXPECT_EQ(org, value);
}

/*!
 * @brief 独自関数AppendStringFの仕様
 * @remark 指定したフォーマットで、引数がフォーマットされる
 * @remark 指定したフォーマットがNULLの場合、例外を投げる
 * @remark 確保済みメモリが十分な場合、追加確保を行わない
 * @remark 追加される文字列が空文字列の場合、追加自体を行わない
 */
TEST(CNativeW, AppendStringWithFormatting)
{
	CNativeW value;
	value.AppendStringF(L"いちご%d%%", 100);
	ASSERT_STREQ(L"いちご100%", value.GetStringPtr());

	// フォーマットに NULL を渡したケースをテストする
	ASSERT_THROW(value.AppendStringF(std::wstring_view(NULL, 0)), std::invalid_argument);
	ASSERT_THROW(value.AppendStringF(std::wstring_view(L"ダミー", 0)), std::invalid_argument);

	// 文字列長を0にして、追加確保が行われないケースをテストする
	value = L"いちご100%"; //テスト前の初期値(念のため再代入しておく
	value._SetStringLength(0);
	value.AppendStringF(L"いちご%d%%", 25); //1文字短くなるような指定をしている
	ASSERT_EQ(L"いちご25%", value);

	// 追加フォーマットが空文字列となるケースをテストする
	value.AppendStringF(L"%s", L"");
	ASSERT_EQ(L"いちご25%", value);

	// 未確保状態からの書式化をテストする
	value = NULL; //テスト前の初期値(未確保
	value.AppendStringF( L"KEY[%03d]", 12 );
	ASSERT_EQ( L"KEY[012]", value );

	// 文字列連結(書式でmax長指定)をテストする
	value.AppendStringF( L"%.3s", L"abcdef" );
	ASSERT_EQ( L"KEY[012]abc", value );

	// 文字列連結(書式で出力長指定)をテストする
	value.AppendStringF( L"%6s", L"abc" );
	ASSERT_EQ( L"KEY[012]abc   abc", value );

	// フォーマット出力長2047字を超える条件をテストする
	{
		std::wstring longText( 2048, L'=' );
		value = NULL; //テスト前の初期値(未確保
		value.AppendStringF( L"%s", longText.c_str() );
		ASSERT_EQ( longText.c_str(), value );
	}
}

/*!
 * @brief 等価比較演算子のテスト
 *  初期値同士の等価比較を行う
 */
TEST(CNativeW, operatorEqualNull)
{
	CNativeW value, other;
	ASSERT_EQ(value, other);
}

/*!
 * @brief 等価比較演算子のテスト
 *  NULLとの等価比較を行う
 */
TEST(CNativeW, operatorEqualNullptr)
{
	CNativeW value;
	ASSERT_EQ(value, NULL);
}

/*!
 * @brief 等価比較演算子のテスト
 *  ポインタ(値がNULL)との等価比較を行う
 */
TEST(CNativeW, operatorEqualStringNull)
{
	CNativeW value;
	LPCWSTR str = NULL;
	ASSERT_EQ(value, str);
}

/*!
 * @brief 等価比較演算子のテスト
 *  値あり同士の等価比較を行う
 */
TEST(CNativeW, operatorEqualSame)
{
	CNativeW value(L"これはテストです。");
	CNativeW other(L"これはテストです。");
	ASSERT_EQ(value, other);
}

/*!
 * @brief 等価比較演算子のテスト
 *  自分自身との等価比較を行う
 */
TEST(CNativeW, operatorEqualBySelf)
{
	CNativeW value;
	ASSERT_EQ(value, value);
}

/*!
 * @brief 等価比較演算子のテスト
 *  等価演算子がfalseを返すパターンのテスト
 */
TEST(CNativeW, operatorEqualAndNotEqual)
{
	CNativeW value;
	EXPECT_TRUE(value == value);
	EXPECT_FALSE(value != value);
	ASSERT_EQ(value, value);

	CNativeW other(L"値あり");
	EXPECT_FALSE(value == other);
	EXPECT_TRUE(value != other);
	ASSERT_NE(value, other);
}

/*!
 * @brief 否定の等価比較演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *  重要なクラスなので、テスト条件ごとにケースを分ける
 *
 *  合格条件：値あり vs NULL の比較で不一致を検出できること
 */
TEST(CNativeW, operatorNotEqualSomeValueVsNull)
{
	// 値あり vs NULL
	CNativeW value(L"これはテストです。");
	CNativeW other;
	ASSERT_NE(value, other);
}

/*!
 * @brief 否定の等価比較演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *  重要なクラスなので、テスト条件ごとにケースを分ける
 *
 *  合格条件：NULL vs 値あり の比較で不一致を検出できること
 */
TEST(CNativeW, operatorNotEqualNullVsSomeValue)
{
	// NULL vs 値あり
	CNativeW value;
	CNativeW other(L"これはテストです。");
	ASSERT_NE(value, other);
}

/*!
 * @brief 否定の等価比較演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *  重要なクラスなので、テスト条件ごとにケースを分ける
 *
 *  合格条件：長さの異なる場合の比較で不一致を検出できること
 */
TEST(CNativeW, operatorNotEqualNotSameLength)
{
	// 値あり vs 値あり(文字列長が違う)
	CNativeW value(L"これはテストです。");
	CNativeW other(L"これはテスト？");
	ASSERT_NE(value, other);
}

/*!
 * @brief 否定の等価比較演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *  重要なクラスなので、テスト条件ごとにケースを分ける
 *
 *  合格条件：長さが等しく内容の異なる場合の比較で不一致を検出できること
 */
TEST(CNativeW, operatorNotEqualNotSameContent)
{
	// 値あり vs 値あり(値が違う)
	CNativeW value(L"これはテストです。");
	CNativeW other(L"これはテストです？");
	ASSERT_NE(value, other);
}

/*!
 * @brief 等価比較演算子のテスト
 *  ポインタとの等価比較を行う
 */
TEST(CNativeW, operatorEqualSameString)
{
	constexpr const wchar_t text[] = L"おっす！オラ(ry";
	CNativeW value(text);
	LPCWSTR str = text;
	ASSERT_EQ(value, str);
}

/*!
 * @brief 否定の等価比較演算子のテスト
 *  ポインタとの等価比較を行う
 */
TEST(CNativeW, operatorNotEqualAlmostSameString)
{
	CNativeW value(L"おっす！オラ(ry");
	LPCWSTR str = L"おっと！オラ(ry";
	ASSERT_NE(value, str);
}

/*!
 * @brief 否定の等価比較演算子のテスト
 *  NULLとの等価比較を行う
 */
TEST(CNativeW, operatorNotEqualNullptr)
{
	constexpr const wchar_t text[] = L"おっす！オラ(ry";
	CNativeW value(text);
	ASSERT_NE(value, NULL);
}

/*!
 * @brief 否定の等価比較演算子のテスト
 *  ポインタ(値がNULL)との等価比較を行う
 */
TEST(CNativeW, operatorNotEqualStringNull)
{
	constexpr const wchar_t text[] = L"おっす！オラ(ry";
	CNativeW value(text);
	LPCWSTR str = NULL;
	ASSERT_NE(value, str);
}

/*!
 * @brief 独自関数Replaceの仕様
 * @remark バッファが確保される
 * @remark 文字列長は0になる
 * @remark バッファサイズは空文字列を格納できるサイズになる
 */
TEST(CNativeW, ReplaceOfNullString)
{
	CNativeW value;
	value.Replace(L"置換前", L"置換後");
	ASSERT_STREQ(L"", value.GetStringPtr());
	EXPECT_EQ(0, value.GetStringLength());
	EXPECT_LE(0, value.capacity());
}

/*!
 * @brief 独自関数Replaceの仕様
 * @remark 文字列中の指定文字列が置換される
 */
TEST(CNativeW, ReplaceWithSomeChanges)
{
	CNativeW value(L"？とは違うのだよ、？とは！");
	value.Replace(L"？", L"ザク");
	ASSERT_STREQ(L"ザクとは違うのだよ、ザクとは！", value.GetStringPtr());
}

/*!
	CNativeW::Clear のデータサイズのクリアをテストする

	0. バッファが空の状態でクリアする

	1-1. 固定データを追加する
	1-2. バッファの状態を取得する
	1-3. バッファの状態をチェックする

	2-1. CNativeW をクリアする
	2-2. クリア後のバッファの状態を取得する
	2-3. クリア後のバッファの状態をチェックする

	3-1. 固定データを再追加する
	3-2. バッファの状態を取得する
	3-3. バッファの状態をチェックする
*/
TEST(CNativeW, Clear)
{
	constexpr const WCHAR*	fixedPatternStr = L"abc";
	constexpr const int		fixedPatternLen = 3;
	
	CNativeW stringW;
	
	// 0. バッファが空の状態でクリアする
	stringW.Clear();

	// 1-1. 固定データを追加する

	stringW.AppendString(fixedPatternStr);			// 固定データを追加する

	// 1-2. バッファの状態を取得する
	
	auto orgCapacity = stringW.capacity();			// データ追加後にバッファサイズを取得する
	auto orgLength   = stringW.GetStringLength();	// Clear() 前にデータサイズを取得する

	// 1-3. バッファの状態をチェックする

	EXPECT_LE(0, orgCapacity);						// データ追加後のバッファサイズを確認する
	EXPECT_EQ(orgLength, fixedPatternLen);			// データ追加後のデータサイズを確認する

	// 2-1. CNativeW をクリアする
	
	stringW.Clear();								// CNativeW をクリアする

	// 2-2. クリア後のバッファの状態を取得する

	auto newCapacity = stringW.capacity();			// Clear() 後にバッファサイズを取得する
	auto newLength   = stringW.GetStringLength();	// Clear() 後にデータサイズを取得する

	// 2-3. クリア後のバッファの状態をチェックする
	
	EXPECT_EQ(orgCapacity, newCapacity);			// Clear() 後にバッファサイズが変わっていないのを確認する
	EXPECT_EQ(newLength, 0);						// Clear() 後にデータが空なのを確認する

	// 3-1. 固定データを再追加する

	stringW.AppendString(fixedPatternStr);			// Clear() 後に固定データを再追加する

	// 3-2. バッファの状態を取得する
	
	auto newCapacity2 = stringW.capacity();			// 再追加後にバッファサイズを取得する
	auto newLength2   = stringW.GetStringLength();	// 再追加後にデータサイズを取得する

	// 3-3. バッファの状態をチェックする
	
	EXPECT_EQ(orgCapacity, newCapacity2);			// 再追加後にバッファサイズが変わっていないのを確認する
	EXPECT_EQ(newLength2, fixedPatternLen);			// 再追加後にデータサイズを確認する
}

/*!
	CNativeW を単にインスタンス化した状態ではバッファが確保されていないのを確認する。
*/
TEST(CNativeW, CheckEmpty)
{
	CNativeW stringW;

	// インスタンス化しただけではバッファが確保されないことを確認する
	EXPECT_EQ(NULL, stringW.GetStringPtr());

	// インスタンス化しただけではバッファサイズが 0 であることを確認する。
	EXPECT_EQ(0, stringW.capacity());
}

/*!
 * 同型との比較のテスト
 *
 * @remark < 0 自身がメモリ未確保、かつ、比較対象はメモリ確保済み
 * @remark < 0 データ値が比較対象より小さい
 * @remark < 0 データが比較対象の先頭部分と一致する、かつ、データ長が比較対象より小さい
 * @remark == 0 比較対象が自分自身の参照
 * @remark == 0 自身がメモリ未確保、かつ、比較対象がメモリ未確保
 * @remark > 0 自身が確保済み、かつ、比較対象がメモリ未確保
 * @remark > 0 データ値が比較対象より大きい
 * @remark > 0 データの先頭部分が比較対象と一致する、かつ、データ長が比較対象より大きい
 */
TEST(CNativeW, CompareWithCNativeW)
{
	//互いに値の異なる文字列定数を定義する
	constexpr const wchar_t szS0[]	= L"a\0b\0c";
	constexpr const wchar_t szM0[]	= L"a\0a\0c\0";
	constexpr const wchar_t szM1[]	= L"a\0b\0c\0";
	constexpr const wchar_t szM2[]	= L"a\0c\0c\0";
	constexpr const wchar_t szL0[]	= L"a\0b\0c\0d";

	// 値なしの変数と文字列定数に対応するCNativeWのインスタンスを用意する
	CNativeW cN0, cN1
		, cS0(szS0, _countof(szS0))
		, cM0(szM0, _countof(szM0))
		, cM1(szM1, _countof(szM1))
		, cM2(szM2, _countof(szM2))
		, cL0(szL0, _countof(szL0));

	// 比較
	// ASSERT_GTの判定仕様は v1 > v2
	// ASSERT_EQの判定仕様は v1 == v2(expected, actual)
	// ASSERT_LTの判定仕様は v1 < v2
	ASSERT_GT(0, cN0.Compare(cS0));
	ASSERT_GT(0, cM1.Compare(cM2));
	ASSERT_GT(0, cM1.Compare(cL0));
	ASSERT_EQ(0, cN0.Compare(cN0));
	ASSERT_EQ(0, cS0.Compare(cS0));
	ASSERT_EQ(0, cN0.Compare(cN1));
	ASSERT_LT(0, cS0.Compare(cN0));
	ASSERT_LT(0, cM1.Compare(cM0));
	ASSERT_LT(0, cM1.Compare(cS0));
}

/*!
 * 文字列ポインタ型との比較のテスト
 *
 * @remark < 0 自身がメモリ未確保、かつ、比較対象がNULL以外
 * @remark < 0 文字列値が比較対象より小さい
 * @remark == 0 自身がメモリ未確保、かつ、比較対象がNULL
 * @remark > 0 自身がメモリ確保済み、かつ、比較対象がNULL
 * @remark > 0 文字列値が比較対象より大きい
 */
TEST(CNativeW, CompareWithStringPtr)
{
	//互いに値の異なる文字列定数を定義する
	constexpr const wchar_t* pcN0 = NULL;
	constexpr const wchar_t szS0[] = L"ab";
	constexpr const wchar_t szM0[] = L"aac";
	constexpr const wchar_t szM1[] = L"abc";
	constexpr const wchar_t szM2[] = L"acc";
	constexpr const wchar_t szL0[] = L"abcd";

	// 定数に対応するCNativeWのインスタンスを用意する
	CNativeW cN0(pcN0), cM1(szM1);

	// 比較
	// ASSERT_GTの判定仕様は v1 > v2
	// ASSERT_EQの判定仕様は v1 == v2(expected, actual)
	// ASSERT_LTの判定仕様は v1 < v2
	ASSERT_GT(0, cN0.Compare(szM1));
	ASSERT_GT(0, cM1.Compare(szM2));
	ASSERT_GT(0, cM1.Compare(szL0));
	ASSERT_EQ(0, cN0.Compare(pcN0));
	ASSERT_EQ(0, cM1.Compare(szM1));
	ASSERT_LT(0, cM1.Compare(pcN0));
	ASSERT_LT(0, cM1.Compare(szM0));
	ASSERT_LT(0, cM1.Compare(szS0));
}

/*!
 * @brief グローバル加算演算子のテスト
 * @remark 1つ目の引数の末尾に2つ目の引数を連結する
 * @remark 順序逆転版も同仕様。
 */
TEST(CNativeW, globalOperatorAdd)
{
	CNativeW v1(L"前半");
	constexpr const wchar_t v2[] = L"後半";
	EXPECT_STREQ(L"前半後半", (v1 + v2).GetStringPtr());

	constexpr const wchar_t v3[] = L"前半";
	CNativeW v4(L"後半");
	EXPECT_STREQ(L"前半後半", (v3 + v4).GetStringPtr());
}

/*!
 * @brief GetSizeOfCharの仕様
 * @remark 指定した文字の符号単位数を返す。
 */
TEST(CNativeW, GetSizeOfChar)
{
	// 基本多言語面の文字ならば1を返す。
	EXPECT_EQ(CNativeW::GetSizeOfChar(L"a", 1, 0), 1);
	EXPECT_EQ(CNativeW::GetSizeOfChar(CStringRef(L"a", 1), 0), 1);
	// 範囲外なら0を返す。
	EXPECT_EQ(CNativeW::GetSizeOfChar(L"", 0, 0), 0);
	EXPECT_EQ(CNativeW::GetSizeOfChar(CStringRef(L"", 0), 0), 0);
	// 上位・下位サロゲートの組み合わせであれば2を返す。
	EXPECT_EQ(CNativeW::GetSizeOfChar(L"\xd83c\xdf38", 2, 0), 2);
	EXPECT_EQ(CNativeW::GetSizeOfChar(CStringRef(L"\xd83c\xdf38", 2), 0), 2);
	// 指定位置が下位サロゲートならその他の文字と同様に1を返す。
	EXPECT_EQ(CNativeW::GetSizeOfChar(L"\xd83c\xdf38", 2, 1), 1);
	EXPECT_EQ(CNativeW::GetSizeOfChar(CStringRef(L"\xd83c\xdf38", 2), 1), 1);
}

TEST(CNativeW, GetSizeOfChar_Empty)
{
	const auto& s = L"";
	EXPECT_EQ(0, CNativeW::GetSizeOfChar(s, _countof(s) - 1, 0));
}

TEST(CNativeW, GetSizeOfChar_SurrogatePair)
{
	// 絵文字　男性のシンボル
	const auto& s = L"\U0001f6b9";
	EXPECT_EQ(2, CNativeW::GetSizeOfChar(s, _countof(s) - 1, 0));
}

TEST(CNativeW, GetSizeOfChar_IVS)
{
	// 葛󠄀城市(先頭の文字が異体字)
	const auto& s = L"葛󠄀城市";
	EXPECT_EQ(3, CNativeW::GetSizeOfChar(s, _countof(s) - 1, 0));
}

/*!
 * @brief GetKetaOfCharの仕様
 * @remark 指定した文字の桁数を返す。
 */
TEST(CNativeW, GetKetaOfChar)
{
	// 範囲外なら0を返す。
	EXPECT_EQ(CNativeW::GetKetaOfChar(L"", 0, 0), 0);
	EXPECT_EQ(CNativeW::GetKetaOfChar(CStringRef(L"", 0), 0), 0);
	// 上位サロゲートなら2を返す。
	EXPECT_EQ(CNativeW::GetKetaOfChar(L"\xd83c\xdf38", 2, 0), 2);
	EXPECT_EQ(CNativeW::GetKetaOfChar(CStringRef(L"\xd83c\xdf38", 2), 0), 2);
	// 上位サロゲートに続く下位サロゲートであれば0を返す。
	EXPECT_EQ(CNativeW::GetKetaOfChar(L"\xd83c\xdf38", 2, 1), 0);
	EXPECT_EQ(CNativeW::GetKetaOfChar(CStringRef(L"\xd83c\xdf38", 2), 1), 0);
	// 下位サロゲートだけなら2を返す。
	EXPECT_EQ(CNativeW::GetKetaOfChar(L"\xdf38", 1, 0), 2);
	EXPECT_EQ(CNativeW::GetKetaOfChar(CStringRef(L"\xdf38", 1), 0), 2);

	// サクラエディタでは Unicode で表現できない文字コードの破壊を防ぐため、
	// 不明バイトを下位サロゲートにマップして保持している。
	// この1バイト文字は半角として扱わなければ不自然なので、
	// 上位対を持たない下位サロゲート 0xdc00 ～ 0xdcff の範囲に限り、1を返すことになっている。
	//
	// https://sourceforge.net/p/sakura-editor/patchunicode/57/
	// http://sakura-editor.sourceforge.net/cgi-bin/cyclamen/cyclamen.cgi?log=unicode&v=833
	EXPECT_EQ(CNativeW::GetKetaOfChar(L"\xdbff", 1, 0), 2);
	EXPECT_EQ(CNativeW::GetKetaOfChar(CStringRef(L"\xdbff", 1), 0), 2);
	for (wchar_t ch = 0xdc00; ch <= 0xdcff; ++ch) {
		EXPECT_EQ(CNativeW::GetKetaOfChar(&ch, 1, 0), 1);
		EXPECT_EQ(CNativeW::GetKetaOfChar(CStringRef(&ch, 1), 0), 1);
	}
	EXPECT_EQ(CNativeW::GetKetaOfChar(L"\xdd00", 1, 0), 2);
	EXPECT_EQ(CNativeW::GetKetaOfChar(CStringRef(L"\xdd00", 1), 0), 2);

	// 文字が半角なら1を返す。
	EXPECT_EQ(CNativeW::GetKetaOfChar(L"a", 1, 0), 1);
	EXPECT_EQ(CNativeW::GetKetaOfChar(CStringRef(L"a", 1), 0), 1);

	// 文字が全角なら2を返す。
	class FakeCache : public CCharWidthCache {
	public:
		bool CalcHankakuByFont(wchar_t c) override { return false; }
	} cache;
	EXPECT_EQ(CNativeW::GetKetaOfChar(L"あ", 1, 0, cache), 2);
	EXPECT_EQ(CNativeW::GetKetaOfChar(CStringRef(L"あ", 1), 0, cache), 2);
}

/*!
 * @brief GetKetaOfCharの仕様
 * @remark 指定した文字のピクセル単位幅を返す。
 */
TEST(CNativeW, GetHabaOfChar)
{
	// 範囲外なら0を返す。
	EXPECT_EQ((Int)CNativeW::GetHabaOfChar(L"", 0, 1, false, GetCharWidthCache()), 0);

	// 改行コードなら1を返す。
	EXPECT_EQ((Int)CNativeW::GetHabaOfChar(L"\r\n", 2, 0, false, GetCharWidthCache()), 1);
	EXPECT_EQ((Int)CNativeW::GetHabaOfChar(L"\r\n", 2, 1, false, GetCharWidthCache()), 1);

	// CalcPxWidthByFont で計算した結果を返す。
	class FakeCache1 : public CCharWidthCache {
	public:
		int CalcPxWidthByFont(wchar_t ch) override {
			if (ch == L'a') return 10000;
			else if (ch == L'b') return 20000;
			else return 0;
		}
	} cache1;
	EXPECT_EQ((Int)CNativeW::GetHabaOfChar(L"ab", 2, 0, false, cache1), 10000);
	EXPECT_EQ((Int)CNativeW::GetHabaOfChar(L"ab", 2, 1, false, cache1), 20000);

	// サロゲートペアの幅は CalcPxWidthByFont2 で計算する。
	// 指定された位置が下位サロゲートなら0を返す。
	class FakeCache2 : public CCharWidthCache {
	public:
		int CalcPxWidthByFont2(const wchar_t* pc2) const override {
			return 20000;
		}
	} cache2;
	EXPECT_EQ((Int)CNativeW::GetHabaOfChar(L"\xd83c\xdf38", 2, 0, false, cache2), 20000);
	EXPECT_EQ((Int)CNativeW::GetHabaOfChar(L"\xd83c\xdf38", 2, 1, false, cache2), 0);

	// サロゲートペアが片方しかないときは CalcPxWidthByFont で計算している。
	class FakeCache3 : public CCharWidthCache {
	public:
		int CalcPxWidthByFont(wchar_t c) override {
			return 10000;
		}
	} cache3;
	EXPECT_EQ((Int)CNativeW::GetHabaOfChar(L"\xd83cあ", 2, 0, false, cache3), 10000);
	EXPECT_EQ((Int)CNativeW::GetHabaOfChar(L"\xdf38あ", 2, 0, false, cache3), 10000);
	EXPECT_EQ((Int)CNativeW::GetHabaOfChar(L"あ\xdf38", 2, 1, false, cache3), 10000);
}

/*!
 * @brief GetCharNextの仕様
 */
TEST(CNativeW, GetCharNext)
{
	constexpr const wchar_t* text = L"a\xd83c\xdf38";
	// 次の文字のアドレスを返す。
	EXPECT_EQ(CNativeW::GetCharNext(text, 3, text), text + 1);
	// 上位サロゲートが渡された場合は下位サロゲートを飛ばす。
	EXPECT_EQ(CNativeW::GetCharNext(text, 3, text + 1), text + 3);
	// ポインタを進めた結果が範囲外なら &pData[nDataLen] を返す。
	EXPECT_EQ(CNativeW::GetCharNext(text, 3, text + 3), text + 3);
}

/*!
 * @brief GetCharPrevの仕様
 */
TEST(CNativeW, GetCharPrev)
{
	constexpr const wchar_t* text = L"a\xd83c\xdf38" L"d";
	// 前の文字のアドレスを返す。
	EXPECT_EQ(CNativeW::GetCharPrev(text, 4, text + 1), text);
	// 前の文字が下位サロゲートだった場合は下位サロゲートを飛ばす。
	EXPECT_EQ(CNativeW::GetCharPrev(text, 4, text + 3), text + 1);
	// ポインタを戻した結果が範囲外なら pData を返す。
	EXPECT_EQ(CNativeW::GetCharPrev(text, 4, text), text);
}

/*!
 * @brief GetCharPrevの潜在バグ評価
 */
TEST(CNativeW, GetCharPrev_Bugs_Preview)
{
	// a、カラー絵文字「男性のシンボル」、x
	constexpr const wchar_t text[] = L"a\U0001F6B9x";

	// text[0] = L'a'
	// text[1] = (\U0001F6B9 の1ワード目)
	// text[2] = (\U0001F6B9 の2ワード目)
	// text[3] = L'x'

	// textのような配列であれば問題はない
	EXPECT_EQ(&text[0], CNativeW::GetCharPrev(text, _countof(text) - 1, text + 1));
	EXPECT_EQ(&text[1], CNativeW::GetCharPrev(text, _countof(text) - 1, text + 2));
	EXPECT_EQ(&text[1], CNativeW::GetCharPrev(text, _countof(text) - 1, text + 3));
	EXPECT_EQ(&text[3], CNativeW::GetCharPrev(text, _countof(text) - 1, text + 4));

	// 配列の一部を参照した、ないし、異常データを扱う場合に問題がある。
	const auto *pText = &text[2];

	// これがバグ。範囲外アドレスを返してはならない。
	// EXPECT_EQ(&text[1], CNativeW::GetCharPrev(pText, 2, pText));

	// 対処方法 関数コメントにある仕様通りに修正する。
	ASSERT_EQ(&text[2], CNativeW::GetCharPrev(pText, 2, pText));
}
