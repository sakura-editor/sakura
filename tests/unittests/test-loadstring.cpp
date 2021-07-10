/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>

#include "CSelectLang.h"
#include "String_define.h"

/*!
 * @brief リソース文字列の読込テスト
 *
 * tests1.exeに埋め込んだsakura_rc.rcの値を読み込めるかチェックするテスト。
 */
TEST(LoadString, LoadStringResource)
{
	// リソースから言語識別子のIDを読み取る
	// ここのリソース文字列値は、言語選択のキーなので変更してはならない。
	ASSERT_STREQ( L"0x0411", LS( STR_SELLANG_LANGID ) );

	// リソースから選択中言語のラベル文字列を読み取る
	// これは共通設定の選択中言語のとこに表示するラベル文字列。
	// 古いWindows APIが言語表示名を提供してなかったことに起因するリソース。
	ASSERT_STREQ( L"Japanese", LS( STR_SELLANG_NAME ) );
}

constexpr auto TEST_STRING1 = L"文字列１";
constexpr auto TEST_STRING2 = L"文字列２";

/*!
 * 関数の引数で渡された文字列が、関数外部で変更されてしまうケースの検証クラス
 */
class CBadStringArgument
{
	std::list<std::wstring_view> datum
	{
		TEST_STRING1,
		TEST_STRING2,
	};
	decltype(datum)::const_iterator current;
	std::wstring str;;

public:
	CBadStringArgument()
	{
		current = datum.cbegin();
		str = current->data();
	}

	const wchar_t* GetStr() const { return str.data(); }

	void UpdateStr() {
		++current;
		str = current->data();
	}
};

/*! #1701 不具合の再現(NG) */
TEST(CBadStringArgument, test1)
{
	CBadStringArgument arg;
	auto test = [&](const wchar_t* pszText) {
		EXPECT_STREQ(TEST_STRING1, pszText);
		arg.UpdateStr(); // 👈更新、👇が影響を受けてNG。
		EXPECT_STREQ(TEST_STRING1, pszText);
	};
	test(arg.GetStr());
}

/*! 引数をC++化してみる(NG) */
TEST(CBadStringArgument, test2)
{
	CBadStringArgument arg;
	auto test = [&](std::wstring_view text) {
		EXPECT_STREQ(TEST_STRING1, text.data());
		arg.UpdateStr(); // 👈更新、👇が影響を受けてNG。
		EXPECT_STREQ(TEST_STRING1, text.data());
	};
	test(arg.GetStr());
};

/*! 引数を const std::wstring& に変えてみる(OK) */
TEST(CBadStringArgument, test3)
{
	CBadStringArgument arg;
	auto test = [&](const std::wstring& text) {
		EXPECT_STREQ(TEST_STRING1, text.data());
		arg.UpdateStr(); // 👈更新、👇は影響を受けずOK。
		EXPECT_STREQ(TEST_STRING1, text.data());
	};
	test(arg.GetStr());
}

/*! 引数を const std::wstring_view& に変えてみる(NG) */
TEST(CBadStringArgument, test4)
{
	CBadStringArgument arg;
	// std::wstring_viewの使い方が間違っている...
	auto test = [&](const std::wstring_view& text) {
		EXPECT_STREQ(TEST_STRING1, text.data());
		arg.UpdateStr(); // 👈更新、👇が影響を受けてNG。
		EXPECT_STREQ(TEST_STRING1, text.data());
	};
	test(arg.GetStr());
}

/*! 最善手のプレビュー(OK) */
TEST(CBadStringArgument, test5)
{
	CBadStringArgument arg;
	auto test = [&](std::wstring_view text) {
		// 関数ローカルにコピーを作成
		const std::wstring copyOfText(text);
		EXPECT_STREQ(TEST_STRING1, copyOfText.data());
		arg.UpdateStr(); // 👈更新、👇は影響を受けずOK。
		EXPECT_STREQ(TEST_STRING1, copyOfText.data());
	};
	test(arg.GetStr());
}

/*! 妥協案のプレビュー(OK) */
TEST(CBadStringArgument, test6)
{
	CBadStringArgument arg;
	auto test = [&](const wchar_t* pszText) {
		// 関数ローカルにコピーを作成
		const std::wstring copyOfText(pszText ? pszText : L"");
		pszText = copyOfText.data();
		EXPECT_STREQ(TEST_STRING1, pszText);
		arg.UpdateStr(); // 👈更新、👇は影響を受けずOK。
		EXPECT_STREQ(TEST_STRING1, pszText);
	};
	test(arg.GetStr());
}
