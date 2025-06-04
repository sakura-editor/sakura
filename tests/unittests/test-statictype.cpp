/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

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
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "util/StaticType.h"

#include "_main/CNormalProcess.h"

/*!
	@brief StaticVectorのテスト
 */
TEST(StaticVector, push_back)
{
	// メモリ確保失敗時に表示するメッセージボックスで、
	// 「アプリ名」を取得するためにプロセスのインスタンスが必要。
	CNormalProcess cProcess(::GetModuleHandle(nullptr), L"");

	// サイズ1の配列を用意する
	auto vec = StaticVector<long long, 1>();
	const auto& constVec = vec;
	EXPECT_EQ(0, vec.size());
	EXPECT_EQ(1, vec.max_size());

	// 1つめのデータを登録する
	vec.push_back(0xabcdef);
	EXPECT_EQ(1, vec.size());
	EXPECT_EQ(0xabcdef, vec[0]);
	EXPECT_EQ(0xabcdef, constVec[0]);

	// 飽和したのでこれ以上追加できない
	EXPECT_EQ(vec.max_size(), vec.size());

	// 追加しようとしてもできないことを確認する

#ifdef _DEBUG
	// デバッグビルドでは、正常にクラッシュする
	EXPECT_DEATH({ vec.push_back(0xffffff); }, "");
#else
	// リリースビルドでもクラッシュする
	EXPECT_THROW({ vec.push_back(0xffffff); }, std::out_of_range);
#endif

	// 追加できないので、サイズをカウントアップしてはいけない
	EXPECT_EQ(1, vec.size());
}

namespace mystring {

/*!
 * @brief StaticStringのテスト
 *
 * 引数なしで初期化する
 */
TEST(StaticString, init001)
{
	// ACT
	StaticString<4> buf;

	// ASSERT
	EXPECT_THAT(buf, StrEq(L""));
	EXPECT_THAT(buf.GetBufferCount(), 4);
	EXPECT_TRUE(buf.empty());
	EXPECT_THAT(LPWSTR(buf), StrEq(L""));
	EXPECT_THAT(LPCWSTR(buf), StrEq(L""));
	EXPECT_THAT(buf.length(), 0);
	EXPECT_THAT(buf.Length(), 0);
	EXPECT_THAT(std::size(buf), 4);
	EXPECT_THAT(buf.at(0), 0);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列ポインタで初期化する
 */
TEST(StaticString, init002)
{
	// ARRANGE
	constexpr auto& initialValue = L"初期値";

	// ACT
	StaticString<4> buf(initialValue);

	// ASSERT
	EXPECT_THAT(buf, StrEq(L"初期値"));
	EXPECT_THAT(buf.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列で初期化する
 */
TEST(StaticString, init003)
{
	// ACT
	StaticString<4> szText(L"初期値"s);

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列参照で初期化する
 */
TEST(StaticString, init004)
{
	// ACT
	StaticString<4> szText(L"初期値"sv);

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * ファイルパスで初期化する
 */
TEST(StaticString, init006)
{
	// ARRANGE
	std::filesystem::path path = L"初期値";

	// ACT
	StaticString<4> szText = path;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列ポインタで初期化する(値がnullptr)
 */
TEST(StaticString, init101)
{
	// ACT
	StaticString<4> szText = nullptr;	// SALによりコンパイラ警告が発生する

	// ASSERT
	EXPECT_THAT(szText, StrEq(L""));
	EXPECT_THAT(szText.length(), 0);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列ポインタで初期化する
 */
TEST(StaticString, init102)
{
	// ACT
	StaticString<4> buf(nullptr);

	// ASSERT
	EXPECT_THAT(buf, StrEq(L""));
	EXPECT_THAT(buf.length(), 0);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列参照で初期化する(サイズオーバー)
 */
TEST(StaticString, init104)
{
	// ACT
	StaticString<4> szText = L"長過ぎる";

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"長過ぎ"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 */
TEST(StaticString, assign001)
{
	// ARRANGE
	constexpr auto& initialValue = L"初期値";
	StaticString<4> buf(initialValue);

	// ACT
	buf = L"設定値";

	// ASSERT
	EXPECT_THAT(buf, StrEq(L"設定値"));
	EXPECT_THAT(buf.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列ポインタを代入する
 */
TEST(StaticString, assign002)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";

	// ACT
	szText = L"設定値";

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"設定値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列を代入する
 */
TEST(StaticString, assign003)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";

	// ACT
	szText = L"設定値"s;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"設定値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列参照を代入する
 */
TEST(StaticString, assign004)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";

	// ACT
	szText = L"設定値"sv;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"設定値"));
	EXPECT_THAT(szText.length(), 3);

	// ACT
	szText = std::wstring_view(szText, 1);

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"設"));
	EXPECT_THAT(szText.length(), 1);
}

/*!
 * @brief StaticStringのテスト
 *
 * 1文字を代入する
 */
TEST(StaticString, assign005)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";

	// ACT
	szText = L'a';

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"a"));
	EXPECT_THAT(szText.length(), 1);
	EXPECT_EQ(szText[0], L'a');
	EXPECT_EQ(szText[1], L'\0');
}

/*!
 * @brief StaticStringのテスト
 *
 * ファイルパスを代入する
 */
TEST(StaticString, assign006)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";
	std::filesystem::path path = L"設定値";

	// ACT
	szText = path;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"設定値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * nullptrを代入する
 */
TEST(StaticString, assign101)
{
	// ARRANGE
	constexpr auto& initialValue = L"初期値";
	StaticString<4> buf(initialValue);

	// ACT
	buf = nullptr;

	// ASSERT
	EXPECT_THAT(buf, StrEq(L""));
	EXPECT_THAT(buf.length(), 0);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列ポインタを代入する(値がnullptr)
 */
TEST(StaticString, assign102)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";
	LPCWSTR pszNull = nullptr;

	// ACT
	szText = pszNull;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L""));
	EXPECT_THAT(szText.length(), 0);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列参照を代入する(サイズオーバー)
 */
TEST(StaticString, assign104)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";

	// ACT
	szText = L"長過ぎる"sv;
	
	// ASSERT
	EXPECT_THAT(szText, StrEq(L"長過ぎ"));
	EXPECT_THAT(szText.length(), 3);

	// ACT2
	EXPECT_EQ(STRUNCATE, szText.assign(L"overflow"sv));

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"ove"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 末尾に文字列ポインタを追加する
 */
TEST(StaticString, append002)
{
	// ARRANGE
	StaticString<4> szText = L"ザク";

	// ACT
	szText += L"Ⅱ";

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"ザクⅡ"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 末尾に文字列を追加する
 */
TEST(StaticString, append003)
{
	// ARRANGE
	StaticString<4> szText = L"ザク";

	// ACT
	szText += L"Ⅱ"s;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"ザクⅡ"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 末尾に文字列参照タを追加する
 */
TEST(StaticString, append004)
{
	// ARRANGE
	StaticString<4> szText = L"ザク";

	// ACT
	szText += L"Ⅱ"sv;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"ザクⅡ"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 末尾に1文字を追加する
 */
TEST(StaticString, append005)
{
	// ARRANGE
	StaticString<4> szText = L"ザク";

	// ACT
	szText += L'Ⅱ';

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"ザクⅡ"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 末尾にnullptrを追加する
 */
TEST(StaticString, append101)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";

	// ACT
	szText += nullptr;	// SALによりコンパイラ警告が発生する

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);

	// ACT & ASSERT
	EXPECT_THAT(szText.append(nullptr), EINVAL);

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 末尾に文字列ポインタを追加する(値がnullptr)
 */
TEST(StaticString, append102)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";
	LPCWSTR pszNull = nullptr;

	// ACT
	szText += pszNull;	// SALによりコンパイラ警告が発生する

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);

	// ACT & ASSERT
	EXPECT_THAT(szText.append(pszNull), EINVAL);

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 末尾に文字列を追加する(値が空文字列)
 */
TEST(StaticString, append103)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";

	// ACT & ASSERT
	EXPECT_THAT(szText.append(L""s), EINVAL);
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);

	// ACT
	szText += L""s;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 末尾に文字列参照を追加する(サイズオーバー)
 */
TEST(StaticString, append104)
{
	// ARRANGE
	StaticString<4> szText = L"ザク";

	// ACT & ASSERT
	szText += L"Ⅱ MS-06F"sv;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"ザクⅡ"));
	EXPECT_THAT(szText.length(), 3);

	// ARRANGE
	szText = L"ザク";

	// ACT & ASSERT
	EXPECT_THAT(szText.append(L"Ⅱ MS-06F"sv), STRUNCATE);

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"ザクⅡ"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StaticStringのテスト
 *
 * 末尾に文字列ポインタを追加する(途中にNULを含む)
 */
TEST(StaticString, append202)
{
	// ARRANGE
	StaticString<4> szText = L"ザ";

	// ACT
	szText += L"ク\0Ⅱ";

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"ザク"));
	EXPECT_THAT(szText.length(), 2);
}

/*!
 * @brief StaticStringのテスト
 *
 * 末尾に文字列参照を追加する(途中にNULを含む)
 */
TEST(StaticString, append204)
{
	StaticString<4> szText = L"ザ";

	// ACT
	szText += L"ク\0Ⅱ"sv;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"ザク"));
	EXPECT_THAT(szText.length(), 2);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列長を計算する
 */
TEST(StaticString, length)
{
	// ARRANGE
	StaticString<4> szText = L"ザクⅡ";
	const wchar_t dummy = 0x2025;

	// ASSERT
	EXPECT_EQ(szText.length(), 3);
	EXPECT_EQ(szText.data()[3], 0);

	// ACT(領域全体にゴミ投入)
	auto_memset(szText.data(), dummy, int(std::size(szText)));

	// ASSERT
	EXPECT_EQ(szText.length(), 0);
	EXPECT_EQ(szText.data()[3], dummy);

	// ACT(NUL終端する)
	szText[2] = 0;

	// ASSERT
	EXPECT_EQ(szText.length(), 2);
	EXPECT_EQ(szText.data()[3], dummy);

	// ACT
	szText = dummy;

	// ASSERT
	EXPECT_EQ(szText.length(), 1);
}

/*!
 * @brief StaticStringのテスト
 *
 * 文字列が空かどうか判定する
 */
TEST(StaticString, empty)
{
	// ARRANGE
	StaticString<4> szText = L"ザクⅡ";

	// ASSERT
	EXPECT_FALSE(szText.empty());

	// ACT
	szText = nullptr;

	// ASSERT
	EXPECT_TRUE(szText.empty());
}

/*!
 * @brief StaticStringのテスト
 */
TEST(StaticString, constAt)
{
	// ARRANGE
	const StaticString<4> szText = L"初期値";

	// ASSERT
	EXPECT_THAT(szText.at(0), L'初');
	EXPECT_THAT(szText.at(1), L'期');
	EXPECT_THAT(szText.at(2), L'値');
	EXPECT_THAT(szText.at(3), '\0');

	EXPECT_THROW({ szText.at(4); }, std::out_of_range);
	EXPECT_THROW({ szText.at(5); }, std::out_of_range);
}

/*!
 * @brief StaticStringのテスト
 *
 * 添え字演算子
 */
TEST(StaticString, subscriptOperator001)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";

	// ASSERT
	EXPECT_THAT(szText[0], L'初');
	EXPECT_THAT(szText[1], L'期');
	EXPECT_THAT(szText[2], L'値');
	EXPECT_THAT(szText[3], '\0');
	EXPECT_THAT(szText[4], '\0');
}

/*!
 * @brief StaticStringのテスト
 * 
 * 添え字演算子
 */
TEST(StaticString, constSubscriptOperator001)
{
	// ARRANGE
	const StaticString<4> szText = L"初期値";

	// ASSERT
	EXPECT_THAT(szText[0], L'初');
	EXPECT_THAT(szText[1], L'期');
	EXPECT_THAT(szText[2], L'値');
	EXPECT_THAT(szText[3], '\0');
	EXPECT_THAT(szText[4], '\0');
}

/*!
 * @brief StaticStringのテスト
 *
 * ファイルパス型に変換する
 */
TEST(StaticString, toFilePath)
{
	// ARRANGE
	StaticString<4> szText = L"初期値";

	// ACT
	auto path = std::filesystem::path(szText);

	// ASSERT
	EXPECT_FALSE(path.empty());

	// ACT
	szText = nullptr;
	path = std::filesystem::path(szText);

	// ASSERT
	EXPECT_TRUE(path.empty());
}

} // namespace mystring
