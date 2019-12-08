/*! @file */
/*
	Copyright (C) 2018-2019 Sakura Editor Organization

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
#include <gtest/gtest.h>
#include "mem/CNativeW.h"
#include "mem/CNativeA.h"

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
 * @remark バッファサイズは指定した文字列長より大きくなる
 */
TEST(CNativeW, ConstructWithStringWithLength)
{
	constexpr const wchar_t sz[] = L"test";
	constexpr const size_t cch = _countof(sz) - 1;
	CNativeW value(sz, cch);
	ASSERT_STREQ(sz, value.GetStringPtr());
	EXPECT_EQ(cch, value.GetStringLength());
	EXPECT_LT(cch + 1, value.capacity());
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
	EXPECT_LT(cch + 1, value.capacity());
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
	EXPECT_LT(1, value.capacity());
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
	EXPECT_EQ(nullptr, value.GetStringPtr());

	CNativeW value2(nullptr);
	EXPECT_EQ(0, value2.GetStringLength());
	EXPECT_EQ(nullptr, value2.GetStringPtr());
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
	EXPECT_LT(cch + 1, value.capacity());

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
	EXPECT_LT(cch + 1, value.capacity());

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
	EXPECT_LT(cch + 1, value.capacity());

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
	EXPECT_LT(cch + 1, value.capacity());
}

/*!
 * @brief 代入演算子(nullptr指定)の仕様
 * @remark バッファを確保している場合は解放される
 * @remark 文字列長はゼロになる
 */
TEST(CNativeW, AssignStringNullPointer)
{
	CNativeW value(L"test");
	value = nullptr;
	EXPECT_EQ(0, value.GetStringLength());
	EXPECT_EQ(nullptr, value.GetStringPtr());
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
 * @remark バッファサイズは2以上になる
 */
TEST(CNativeW, AppendChar)
{
	constexpr const wchar_t sz[] = L"X";
	constexpr const size_t cch = _countof(sz) - 1;
	CNativeW value;
	value += sz[0];
	ASSERT_STREQ(sz, value.GetStringPtr());
	EXPECT_EQ(1, value.GetStringLength());
	EXPECT_LT(1 + 1, value.capacity());
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
	EXPECT_LT(cch + 1, value.capacity());
}

/*!
 * @brief 加算代入演算子(nullptr指定)の仕様
 * @remark 加算代入しても内容に変化無し
 */
TEST(CNativeW, AppendStringNullPointer)
{
	CNativeW org(L"orz");
	CNativeW value(org);
	value += nullptr;
	EXPECT_EQ(value.GetStringLength(), org.GetStringLength());
	EXPECT_EQ(org, value);
}

/*!
 * @brief 加算代入演算子(NULL指定)の仕様
 * @remark バッファが確保される
 * @remark 文字列長は演算子呼出前の文字列長+1になる
 */
TEST(CNativeW, AppendStringNullLiteral)
{
	CNativeW value;
#ifdef _MSC_VER
	value += NULL; // operator += (wchar_t) と解釈される
#else
	value += static_cast<wchar_t>(NULL);
#endif
	ASSERT_STREQ(L"", value.GetStringPtr());
	EXPECT_EQ(1, value.GetStringLength());
}

/*!
 * @brief 独自関数AppendStringFの仕様
 * @remark 指定したフォーマットで、引数がフォーマットされる
 */
TEST(CNativeW, AppendStringWithFormatting)
{
	CNativeW value;
	value.AppendStringF(L"いちご%d%%", 100);
	ASSERT_STREQ(L"いちご100%", value.GetStringPtr());
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
 *  nullptrとの等価比較を行う
 */
TEST(CNativeW, operatorEqualNullptr)
{
	CNativeW value;
	ASSERT_EQ(value, nullptr);
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
 *  nullptrとの等価比較を行う
 */
TEST(CNativeW, operatorNotEqualNullptr)
{
	constexpr const wchar_t text[] = L"おっす！オラ(ry";
	CNativeW value(text);
	ASSERT_NE(value, nullptr);
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
 * @remark バッファサイズは1以上になる
 */
TEST(CNativeW, ReplaceOfNullString)
{
	CNativeW value;
	value.Replace(L"置換前", L"置換後");
	ASSERT_STREQ(L"", value.GetStringPtr());
	EXPECT_EQ(0, value.GetStringLength());
	EXPECT_LT(1, value.capacity());
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

	EXPECT_GT(orgCapacity, 0);						// データ追加後のバッファサイズを確認する
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
