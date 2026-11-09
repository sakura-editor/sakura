/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include <tchar.h>
#include <Windows.h>

#include <stdexcept>

#include "basis/primitive.h"
#include "util/string_ex.h"

#include "testing/MsvcInvalidParameterHandlerDisabler.hpp"
#include "testing/MsvcReportMode.hpp"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

using namespace cxx;
using namespace testing;

// CStrictIntegerっぽい型を検出するコンセプトのテスト
static_assert(!cxx::StrictIntegerLike<int>);

#ifdef USE_STRICT_INT

static_assert(cxx::StrictIntegerLike<CLayoutInt>);
static_assert(cxx::StrictIntegerLike<CLogicInt>);

#else

static_assert(!cxx::StrictIntegerLike<CLayoutInt>);
static_assert(!cxx::StrictIntegerLike<CLogicInt>);

#endif // ifdef USE_STRICT_INT


/*!
	@brief 旧コード互換用。可能であれば使わないでください。

	代替関数は strnlen か std::char_traits<char>::length です。

	strlenに渡す文字列ポインタが以下を満たすなら「危険」でもないです。
	・非NULLであること
	・I/FのNUL終端を確約できること
 */
TEST(string_ex, auto_strlenA)
{
	constexpr auto& text = "abc";
	ASSERT_THAT(auto_strlen(text), Eq(3));
}

/*!
	@brief 旧コード互換用。可能であれば使わないでください。

	代替関数は wcsnlen か std::char_traits<wchar_t>::length です。

	wcslenに渡す文字列ポインタが以下を満たすなら「危険」でもないです。
	・非NULLであること
	・I/FのNUL終端を確約できること
 */
TEST(string_ex, auto_strlenW)
{
	constexpr auto& text = LTEXT("abc");
	ASSERT_THAT(auto_strlen(text), Eq(3));
}

TEST(string_ex, auto_strnlenA)
{
	constexpr auto& text = "abc";
	ASSERT_THAT(auto_strnlen(text, std::size(text)), Eq(3));
}

TEST(string_ex, auto_strnlenW)
{
	constexpr auto& text = LTEXT("abc");
	ASSERT_THAT(auto_strnlen(text, std::size(text)), Eq(3));
}

/*!
	@brief 旧コード互換用。可能であれば使わないでください。

	代替関数はないのでロジックを見直してください。
 */
TEST(string_ex, auto_strchrA)
{
	constexpr const char test_data[] = "abc";
	ASSERT_EQ(&test_data[1], auto_strchr(test_data, test_data[1]));
}

/*!
	@brief 旧コード互換用。可能であれば使わないでください。

	代替関数はないのでロジックを見直してください。
 */
TEST(string_ex, auto_strchrW)
{
	constexpr const WCHAR test_data[] = LTEXT("abc");
	ASSERT_EQ(&test_data[1], auto_strchr(test_data, test_data[1]));
}

/*!
	@brief 旧コード互換用。使わないでください。

	代替関数は strprintf か auto_sprintf_s です。
 */
TEST(string_ex, auto_sprintfA)
{
	char szText[_MAX_PATH];
	auto_sprintf(szText, "%s-%d", "test", 101);
	ASSERT_STREQ("test-101", szText);
}

/*!
	@brief 旧コード互換用。使わないでください。

	代替関数は strprintf か auto_sprintf_s です。
 */
TEST(string_ex, auto_sprintfW)
{
	WCHAR szText[_MAX_PATH];
	auto_sprintf(szText, LTEXT("%s-%d"), LTEXT("test"), 101);
	ASSERT_STREQ(LTEXT("test-101"), szText);
}

/*!
 * @brief 旧コード互換用。使わないでください。
 *
 * サクラエディタ独自関数です。
 *
 * 第1引数には出力先の固定長バッファを指定します。
 *
 * 固定長バッファとして扱えるなら生配列でもstd::arrayでもstd::vectorでも構いません。
 *
 * バッファ変数の宣言を変更できる場合、 strprintf への移行を検討してください。
 *
 * 書式文字列ごと変更できる場合、 std::format への移行を検討してください。
 */
TEST(string_ex, auto_sprintf_sA)
{
	std::array<char, _MAX_PATH> szText{};
	EXPECT_THAT(auto_sprintf_s(szText, "%s-%d", "test", 101), 8);
	ASSERT_THAT(std::data(szText), StrEq("test-101"));
}

TEST(string_ex, auto_sprintf_sW)
{
	SFilePath szText;
	EXPECT_THAT(auto_sprintf_s(szText, LTEXT("%s-%d"), LTEXT("test"), 101), 8);
	ASSERT_THAT(szText, StrEq(LTEXT("test-101")));
}

TEST(string_ex, ConvertPrintfArgReturnsSourcePointerA)
{
	const auto source = "test"s;

	EXPECT_THAT(
		PrintfArg::ConvertArgPhase2(PrintfArg::ConvertArgPhase1(source)),
		Eq(source.c_str())
	);
}

TEST(string_ex, ConvertPrintfArgReturnsSourcePointerW)
{
	const auto source = LTEXT("test"s);

	EXPECT_THAT(
		PrintfArg::ConvertArgPhase2(PrintfArg::ConvertArgPhase1(source)),
		Eq(source.c_str())
	);
}

TEST(string_ex, ConvertPrintfArgRejectsTemporaryBufferA)
{
	EXPECT_THAT(
		PrintfArg::ConvertArgPhase2(PrintfArg::ConvertArgPhase1("test data"sv.substr(0, 4))),
		StrEq("test")
	);
}

TEST(string_ex, ConvertPrintfArgRejectsTemporaryBufferW)
{
	EXPECT_THAT(
		PrintfArg::ConvertArgPhase2(PrintfArg::ConvertArgPhase1(LTEXT("test data"sv).substr(0, 4))),
		StrEq(LTEXT("test"))
	);
}

/*!
	@brief 独自定義のフォーマット関数。

	C関数をC++に移植する作業を簡便化する目的で作成。
	CRT関数sprintf_sに関する注意点が全て当てはまるので注意。
	例)
		"%b" など、未定義のフォーマット識別子を指定すると落ちます。
		フォーマットと引数の整合がとれていないとAV例外で落ちます。
 */
TEST(string_ex, strprintf)
{
	sakura::wstring text = strprintf(LTEXT("%s-%d"), LTEXT("test"), 101);
	ASSERT_STREQ(LTEXT("test-101"), text.c_str());
}

/*!
	@brief 独自定義のフォーマット関数(C-Style風)。
 */
TEST(string_ex, strprintfWOutputToArg)
{
	sakura::wstring text(1024, LCHAR('\0'));
	strprintf(text, LTEXT("%s-%d"), LTEXT("test"), 101);
	ASSERT_STREQ(LTEXT("test-101"), text.c_str());
}

/*!
	@brief 独自定義のフォーマット関数(C-Style風)。
 */
TEST(string_ex, strprintfAOutputToArg)
{
	std::string text(1024, '\0');
	strprintf(text, "%ls-of-active-codepage-%d", LTEXT("test"), 101);
	ASSERT_STREQ("test-of-active-codepage-101", text.c_str());
}

/*!
	@brief 独自定義のフォーマット関数(空文字出力テスト)。
 */
TEST(string_ex, strprintfEmpty)
{
	sakura::wstring text = strprintf(LTEXT("%hs"), "");
	ASSERT_TRUE(text.empty());
}

/*!
	@brief 独自定義のフォーマット関数(CLayoutInt出力テスト)。
 */
TEST(string_ex, strprintfCLayoutInt)
{
	CLayoutInt value{ 25 };
	sakura::wstring text = strprintf(LTEXT("いちご%d%%"), value);
	EXPECT_THAT(text, StrEq(LTEXT("いちご25%")));
}

/*!
	@brief 独自定義のフォーマット関数(C-Style風)。

	バッファが極端に小さい場合の確認。
	wtd::wstringのスモールバッファは7文字分。
 */
TEST(string_ex, strprintfW_small_output)
{
	sakura::wstring text;
	EXPECT_STREQ(LTEXT(""), text.c_str());

	text = strprintf(LTEXT("%d"), 1);
	EXPECT_STREQ(LTEXT("1"), text.c_str());

	text = strprintf(LTEXT("%d"), 12);
	EXPECT_STREQ(LTEXT("12"), text.c_str());

	text = strprintf(LTEXT("%d"), 123);
	EXPECT_STREQ(LTEXT("123"), text.c_str());

	text = strprintf(LTEXT("%d"), 1234);
	EXPECT_STREQ(LTEXT("1234"), text.c_str());

	text = strprintf(LTEXT("%d"), 12345);
	EXPECT_STREQ(LTEXT("12345"), text.c_str());

	text = strprintf(LTEXT("%d"), 123456);
	EXPECT_STREQ(LTEXT("123456"), text.c_str());

	text = strprintf(LTEXT("%d"), 1234567);
	EXPECT_STREQ(LTEXT("1234567"), text.c_str());
}

/*!
	@brief 独自定義のフォーマット関数(C-Style風)。

	バッファが極端に小さい場合の確認
	wtd::stringのスモールバッファは15文字分。
 */
TEST(string_ex, strprintfA_small_output)
{
	std::string text;
	EXPECT_STREQ("", text.c_str());

	text = strprintf("%d", 1);
	EXPECT_STREQ("1", text.c_str());

	text = strprintf("%d", 12);
	EXPECT_STREQ("12", text.c_str());

	text = strprintf("%d", 123);
	EXPECT_STREQ("123", text.c_str());

	text = strprintf("%d", 1234);
	EXPECT_STREQ("1234", text.c_str());

	text = strprintf("%d", 12345);
	EXPECT_STREQ("12345", text.c_str());

	text = strprintf("%d", 123456);
	EXPECT_STREQ("123456", text.c_str());

	text = strprintf("%d", 1234567);
	EXPECT_STREQ("1234567", text.c_str());

	text = strprintf("%d", 12345678);
	EXPECT_STREQ("12345678", text.c_str());

	text = strprintf("%d", 123456789);
	EXPECT_STREQ("123456789", text.c_str());

	text = strprintf("%d", 1234567890);
	EXPECT_STREQ("1234567890", text.c_str());

	text = strprintf("1234567890%d", 1);
	EXPECT_STREQ("12345678901", text.c_str());

	text = strprintf("1234567890%d", 12);
	EXPECT_STREQ("123456789012", text.c_str());

	text = strprintf("1234567890%d", 123);
	EXPECT_STREQ("1234567890123", text.c_str());

	text = strprintf("1234567890%d", 1234);
	EXPECT_STREQ("12345678901234", text.c_str());

	text = strprintf("1234567890%d", 12345);
	EXPECT_STREQ("123456789012345", text.c_str());

	text = strprintf("1234567890%d", 123456);
	EXPECT_STREQ("1234567890123456", text.c_str());
}

#if defined(_MSC_VER) && defined(_DEBUG)

TEST(string_ex, strprintfA101)
{
	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	// formatは必須。省略したらクラッシュする
	LPCSTR format = nullptr;
	const auto ret = strprintf(format, "arg");
	EXPECT_THAT(errno, Eq(EINVAL));
	EXPECT_THAT(ret, StrEq(""));
}

TEST(string_ex, strprintfW101)
{
	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	// formatは必須。省略したらクラッシュする
	LPCWSTR format = nullptr;
	const auto ret = strprintf(format, LTEXT("arg"));
	EXPECT_THAT(errno, Eq(EINVAL));
	EXPECT_THAT(ret, StrEq(LTEXT("")));
}

#endif // defined(_MSC_VER) && defined(_DEBUG)

TEST(string_ex, strprintfA102)
{
	// formatが空なら呼び出す意味はない。
	auto ret = strprintf("", nullptr);
	EXPECT_THAT(ret, StrEq(""));

	ACHAR buf[50]{};
	::sprintf_s(buf, "", nullptr);
	EXPECT_THAT((LPCSTR)buf, StrEq(""));

	LPCSTR format = "%s";
	ret = strprintf(format, (LPCSTR)nullptr);
	EXPECT_THAT(ret, StrEq("(null)"));
	::sprintf_s(buf, format, (LPCSTR)nullptr);
	EXPECT_THAT(buf, StartsWith("(null)"));
}

TEST(string_ex, strprintfW102)
{
	// formatが空なら呼び出す意味はない。
	auto ret = strprintf(LTEXT(""), nullptr);
	EXPECT_THAT(ret, StrEq(LTEXT("")));

	WCHAR buf[50]{};
	::swprintf_s(buf, LTEXT(""), nullptr);
	EXPECT_THAT((LPCWSTR)buf, StrEq(LTEXT("")));

	LPCWSTR format = LTEXT("%s");
	ret = strprintf(format, (LPCWSTR)nullptr);
	EXPECT_THAT(ret, StrEq(LTEXT("(null)")));
	::swprintf_s(buf, format, (LPCWSTR)nullptr);
	EXPECT_THAT(buf, StartsWith(LTEXT("(null)")));
}

TEST(string_ex, strprintfW103)
{
	// 書式化パラメータには部分文字列を指定できるようにした
	EXPECT_THAT(
		strprintf(LTEXT("%s"), LTEXT("test data"sv).substr(0, 4)),
		StrEq(LTEXT("test"))
	);
}

/*!
	@brief 独自定義の文字列比較関数。
 */
TEST(string_ex, strncmp_literal)
{
	constexpr const char test_data1[] = "abc";
	constexpr const char test_data2[] = "abc";
	constexpr const char test_data3[] = "xyz";
	ASSERT_EQ(0, strncmp_literal(test_data1, test_data2));
	ASSERT_GT(0, strncmp_literal(test_data1, test_data3));
	ASSERT_LT(0, strncmp_literal(test_data3, test_data1));
}

/*!
	@brief 独自定義の文字列比較関数。
 */
TEST(string_ex, wcsncmp_literal)
{
	constexpr const WCHAR test_data1[] = LTEXT("abc");
	constexpr const WCHAR test_data2[] = LTEXT("abc");
	constexpr const WCHAR test_data3[] = LTEXT("xyz");
	ASSERT_EQ(0, wcsncmp_literal(test_data1, test_data2));
	ASSERT_GT(0, wcsncmp_literal(test_data1, test_data3));
	ASSERT_LT(0, wcsncmp_literal(test_data3, test_data1));
}

/*!
	@brief 独自定義の文字列比較関数。
 */
TEST(string_ex, strnicmp_literal)
{
	constexpr const char test_data1[] = "abc";
	constexpr const char test_data2[] = "ABC";
	constexpr const char test_data3[] = "XYZ";
	ASSERT_EQ(0, strnicmp_literal(test_data1, test_data2));
	ASSERT_GT(0, strnicmp_literal(test_data1, test_data3));
	ASSERT_LT(0, strnicmp_literal(test_data3, test_data1));
}

/*!
	@brief 独自定義の文字列比較関数。
 */
TEST(string_ex, wcsnicmp_literal)
{
	constexpr const WCHAR test_data1[] = LTEXT("abc");
	constexpr const WCHAR test_data2[] = LTEXT("ABC");
	constexpr const WCHAR test_data3[] = LTEXT("XYZ");
	ASSERT_EQ(0, wcsnicmp_literal(test_data1, test_data2));
	ASSERT_GT(0, wcsnicmp_literal(test_data1, test_data3));
	ASSERT_LT(0, wcsnicmp_literal(test_data3, test_data1));
}

/*!
	@brief skr_towupper は 256 未満の文字を "English"(Windows-1252) の表で大文字にする

	U+0080～U+009F(制御文字)は変えない。U+00FF は U+0178 にする。
	空白(U+0020)と U+00E0 を同じ文字にしない(BugReport/64)。
	表を使うのは MSVC のときだけ(ほかのコンパイラーでは実行時ライブラリの towupper に任せる)なので、表の値は MSVC のときだけ確かめる。
 */
TEST(string_ex, skr_towupper)
{
	EXPECT_THAT(skr_towupper(LCHAR('a')), Eq(LCHAR('A')));
	EXPECT_THAT(skr_towupper(LCHAR('Z')), Eq(LCHAR('Z')));
	EXPECT_THAT(skr_towupper(LCHAR(' ')), Eq(LCHAR(' ')));
#ifdef _MSC_VER
	EXPECT_THAT(skr_towupper(0x009A), Eq(0x009A));
	EXPECT_THAT(skr_towupper(0x00E0), Eq(0x00C0));
	EXPECT_THAT(skr_towupper(0x00FE), Eq(0x00DE));
	EXPECT_THAT(skr_towupper(0x00FF), Eq(0x0178));
#endif // ifdef _MSC_VER
}

/*!
	@brief skr_towlower は 256 未満の文字を "English"(Windows-1252) の表で小文字にする

	U+0080～U+009F(制御文字)は変えない。
	表を使うのは MSVC のときだけ(ほかのコンパイラーでは実行時ライブラリの towlower に任せる)なので、表の値は MSVC のときだけ確かめる。
 */
TEST(string_ex, skr_towlower)
{
	EXPECT_THAT(skr_towlower(LCHAR('A')), Eq(LCHAR('a')));
	EXPECT_THAT(skr_towlower(LCHAR('z')), Eq(LCHAR('z')));
	EXPECT_THAT(skr_towlower(LCHAR(' ')), Eq(LCHAR(' ')));
#ifdef _MSC_VER
	EXPECT_THAT(skr_towlower(0x008A), Eq(0x008A));
	EXPECT_THAT(skr_towlower(0x00C0), Eq(0x00E0));
	EXPECT_THAT(skr_towlower(0x00DE), Eq(0x00FE));
	EXPECT_THAT(skr_towlower(0x00FF), Eq(0x00FF));
#endif // ifdef _MSC_VER
}
