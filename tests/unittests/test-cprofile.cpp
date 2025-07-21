/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "CProfile.h"

#include <cstdlib>
#include <filesystem>

#include "util/file.h"
#include "CDataProfile.h"

/*!
 * @brief WriteProfileは指定されたパスに含まれるサブディレクトリを作成する
 */
TEST( CProfile, WriteProfileMakesSubDirectories )
{
	// サブディレクトリを含むパスを作成する
	WCHAR szIniName[_MAX_PATH]{ 0 };
	::_wfullpath( szIniName, L"test1\\test2\\test.ini", _countof(szIniName) );

	// プロファイルを書き出す
	CProfile cProfile;
	cProfile.WriteProfile( szIniName, L"WriteProfileのテスト" );

	ASSERT_TRUE( fexist( szIniName ) );

	WCHAR* p;

	// ファイルを削除
	std::filesystem::remove( szIniName );

	// フォルダーを削除
	p = ::PathFindFileNameW( szIniName );
	p[0] = L'\0';
	std::filesystem::remove( szIniName );

	// フォルダーを削除
	p = ::PathFindFileNameW( szIniName );
	p[0] = L'\0';
	std::filesystem::remove( szIniName );
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NoSection)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	// 初期状態は空なのでセクションが見つからない
	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"存在しないセクション名", L"szTest", value));
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NewSection)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"szTest", value));

	// セクションを追加
	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	// 追加されたセクションを取得
	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"szTest", value));
	ASSERT_STREQ(L"value", value.data());
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NoEntry)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"szTest", value));

	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"szTest", value));
	ASSERT_STREQ(L"value", value.data());

	// 該当するキーがないのでエントリを取得できない
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"nTest", value));
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NewEntry)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"szTest", value));

	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"szTest", value));
	ASSERT_STREQ(L"value", value.data());

	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"nTest", value));

	cProfile.SetProfileData(L"Test", L"nTest", L"109");

	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"nTest", value));
	ASSERT_STREQ(L"109", value.data());
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_int)
{
	int value = 0;
	ASSERT_TRUE(profile_data::TryParse(L"109", value));
	ASSERT_EQ(109, value);

	ASSERT_FALSE(profile_data::TryParse(L"", value));
	ASSERT_EQ(109, value);

	ASSERT_FALSE(profile_data::TryParse(L"not a number", value));
	ASSERT_EQ(109, value);

	ASSERT_TRUE(profile_data::TryParse(L"888", value));
	ASSERT_EQ(888, value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_enum)
{
	enum ETest { NONE, RED, GREEN, BLUE, };
	ETest value = NONE;
	ASSERT_TRUE(profile_data::TryParse(L"1", value));
	ASSERT_EQ(RED, value);

	ASSERT_FALSE(profile_data::TryParse(L"", value));
	ASSERT_EQ(RED, value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_bool)
{
	bool value = false;
	ASSERT_TRUE(profile_data::TryParse(L"1", value));
	ASSERT_TRUE(value);

	ASSERT_FALSE(profile_data::TryParse(L"", value));
	ASSERT_TRUE(value);

	ASSERT_FALSE(profile_data::TryParse(L"false", value));
	ASSERT_TRUE(value);

	ASSERT_TRUE(profile_data::TryParse(L"0", value));
	ASSERT_FALSE(value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_WCHAR)
{
	WCHAR value = L'\0';
	ASSERT_TRUE(profile_data::TryParse(L"|", value));
	ASSERT_EQ(L'|', value);

	ASSERT_FALSE(profile_data::TryParse(L"\U0001F51E", value));
	ASSERT_EQ(L'|', value);

	ASSERT_TRUE(profile_data::TryParse(L"", value));
	ASSERT_EQ(L'\0', value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_KEYCODE)
{
	KEYCODE value = '\0';
	ASSERT_TRUE(profile_data::TryParse(L"W", value));
	ASSERT_EQ('W', value);

	ASSERT_FALSE(profile_data::TryParse(L"漢", value));
	ASSERT_EQ('W', value);

	ASSERT_TRUE(profile_data::TryParse(L"", value));
	ASSERT_EQ('\0', value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_StringBufferW)
{
	WCHAR buffer[5]{ 0 };
	StringBufferW value(buffer);
	ASSERT_TRUE(profile_data::TryParse(L"test", value));
	ASSERT_STREQ(L"test", value.c_str());

	ASSERT_FALSE(profile_data::TryParse(L"overflow", value));
	ASSERT_STREQ(L"test", value.c_str());

	ASSERT_TRUE(profile_data::TryParse(L"", value));
	ASSERT_STREQ(L"", value.c_str());
}

/*!
 * @brief ToString(KEYCODE)のテスト
 */
TEST(profile_data, ToString_KEYCODE)
{
	std::wstring str;
	KEYCODE code;

	code = 0x80;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"", str.c_str());

	code = -1;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"", str.c_str());

	code = 0;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"", str.c_str());

	code = 1;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"\x01", str.c_str());

	code = 0x61;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"a", str.c_str());

	code = 0x7f;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"\x7f", str.c_str());
}

/*!
 * @brief IOProfileDataのテスト
 */
TEST(CDataProfile, IOProfileData)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	ASSERT_FALSE(cProfile.IOProfileData(L"Test", L"szTest", value));

	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	ASSERT_TRUE(cProfile.IOProfileData(L"Test", L"szTest", value));
	ASSERT_STREQ(L"value", value.data());

	ASSERT_FALSE(cProfile.IOProfileData(L"Test", L"nTest", value));

	cProfile.SetProfileData(L"Test", L"nTest", L"109");

	ASSERT_TRUE(cProfile.IOProfileData(L"Test", L"nTest", value));
	ASSERT_STREQ(L"109", value.data());

	int nValue = 0;
	ASSERT_TRUE(cProfile.IOProfileData(L"Test", L"nTest", nValue));
	ASSERT_EQ(109, nValue);

	ASSERT_FALSE(cProfile.IOProfileData(L"Test", L"szTest", nValue));
	ASSERT_EQ(109, nValue);
}

namespace mystring {

/*!
 * @brief StringBufferWのテスト
 * 
 * 引数なしで初期化する
 */
TEST(StringBufferW, init001)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");

	// ACT
	auto szText = StringBufferW(lf.lfFaceName);

	// ASSERT
	EXPECT_THAT(szText.c_str(), lf.lfFaceName);
	EXPECT_THAT(szText.size(), std::size(lf.lfFaceName));
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(LPWSTR(szText), StrEq(L"初期値"));
	EXPECT_THAT(LPCWSTR(szText), StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StringBufferWのテスト
 */
TEST(StringBufferW, init002)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");

	// ACT
	auto szText = StringBufferW(lf.lfFaceName, std::size(lf.lfFaceName));

	// ASSERT
	EXPECT_THAT(szText.c_str(), lf.lfFaceName);
	EXPECT_THAT(szText.size(), std::size(lf.lfFaceName));
}

/*!
 * @brief StringBufferWのテスト
 */
TEST(StringBufferW, init102)
{
	EXPECT_THROW({ StringBufferW buf(nullptr, 1); }, std::invalid_argument);

	auto nul = L'\0';
	EXPECT_THROW({ StringBufferW buf(&nul, 0);}, std::invalid_argument);
}

/*!
 * @brief StringBufferWのテスト
 */
TEST(StringBufferW, assign001)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	StringBufferW buf(lf.lfFaceName);

	// ACT
	buf = L"ＭＳ ゴシック";

	// ASSERT
	EXPECT_THAT(buf, StrEq(L"ＭＳ ゴシック"));
}


/*!
 * @brief StringBufferWのテスト
 * 
 * 文字列ポインタを代入する
 */
TEST(StringBufferW, assign002)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	auto szText = StringBufferW(lf.lfFaceName);

	// ACT
	szText = L"test";

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"test"));
	EXPECT_THAT(szText.length(), 4);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 文字列を代入する
 */
TEST(StringBufferW, assign003)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	auto szText = StringBufferW(lf.lfFaceName);

	// ACT
	szText = L"test"s;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"test"));
	EXPECT_THAT(szText.length(), 4);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 文字列参照を代入する
 */
TEST(StringBufferW, assign004)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	auto szText = StringBufferW(lf.lfFaceName);

	// ACT
	szText = L"test"sv;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"test"));
	EXPECT_THAT(szText.length(), 4);

	// ACT
	szText = std::wstring_view(szText, 1);

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"t"));
	EXPECT_THAT(szText.length(), 1);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 1文字を代入する
 */
TEST(StringBufferW, assign005)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	auto szText = StringBufferW(lf.lfFaceName);

	// ACT
	szText = L'a';

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"a"));
	EXPECT_THAT(szText.length(), 1);
	EXPECT_THAT(szText[0], L'a');
	EXPECT_THAT(szText[1], L'\0');
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 文字列を代入する
 */
TEST(StringBufferW, assign006)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	auto szText = StringBufferW(lf.lfFaceName);
	SString<4> szAlt = L"設定値";

	// ACT
	szText = szAlt;

	// ASSERT
	EXPECT_THAT(szText, StrEq(std::wstring(szAlt)));
	EXPECT_THAT(szText.length(), szAlt.length());
}

/*!
 * @brief StringBufferWのテスト
 * 
 * nullptrを代入する
 */
TEST(StringBufferW, assign101)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	auto szText = StringBufferW(lf.lfFaceName);

	// ACT
	szText = nullptr;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L""));
	EXPECT_THAT(szText.length(), 0);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 文字列ポインタを代入する(値がnullptr)
 */
TEST(StringBufferW, assign102)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	auto szText = StringBufferW(lf.lfFaceName);
	LPCWSTR pszNull = nullptr;

	// ACT
	szText = pszNull;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L""));
	EXPECT_THAT(szText.length(), 0);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 文字列参照を代入する(サイズオーバー)
 */
TEST(StringBufferW, assign104)
{
	// ARRANGE
	SString<4> szBase = L"初期値";
	auto szText = StringBufferW(szBase);

	// ACT
	szText = L"設定値1"sv;
	
	// ASSERT
	EXPECT_THAT(szText, StrEq(L"設定値"));
	EXPECT_THAT(szText.length(), 3);

	// ACT2
	EXPECT_EQ(STRUNCATE, szText.assign(L"設定値1"sv));

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"設定値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 末尾に文字列ポインタを追加する
 */
TEST(StringBufferW, append002)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	auto szText = StringBufferW(lf.lfFaceName);

	// ACT
	szText += L"よん♪";

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値よん♪"));
	EXPECT_THAT(szText.length(), 6);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 末尾に文字列を追加する
 */
TEST(StringBufferW, append003)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	auto szText = StringBufferW(lf.lfFaceName);

	// ACT
	szText += L"よん♪"s;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値よん♪"));
	EXPECT_THAT(szText.length(), 6);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 末尾に文字列参照タを追加する
 */
TEST(StringBufferW, append004)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	auto szText = StringBufferW(lf.lfFaceName);

	// ACT
	szText += L"よん♪"sv;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値よん♪"));
	EXPECT_THAT(szText.length(), 6);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 末尾に1文字を追加する
 */
TEST(StringBufferW, append005)
{
	// ARRANGE
	LOGFONT lf{};
	wcscpy_s(lf.lfFaceName, L"初期値");
	auto szText = StringBufferW(lf.lfFaceName);

	// ACT
	szText += L't';

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値t"));
	EXPECT_THAT(szText.length(), 4);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 末尾にnullptrを追加する
 */
TEST(StringBufferW, append101)
{
	// ARRANGE
	SString<4> szBase = L"初期値";
	auto szText = StringBufferW(szBase);

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
 * @brief StringBufferWのテスト
 * 
 * 末尾に文字列ポインタを追加する(値がnullptr)
 */
TEST(StringBufferW, append102)
{
	// ARRANGE
	SString<4> szBase = L"初期値";
	auto szText = StringBufferW(szBase);
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
 * @brief StringBufferWのテスト
 * 
 * 末尾に文字列を追加する(値が空文字)
 */
TEST(StringBufferW, append103)
{
	// ARRANGE
	SString<4> szBase = L"初期値";
	auto szText = StringBufferW(szBase);

	// ACT
	szText += L""s;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);

	// ACT & ASSERT
	EXPECT_THAT(szText.append(L""s), EINVAL);

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"初期値"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 末尾に文字列参照を追加する(サイズオーバー)
 */
TEST(StringBufferW, append203)
{
	// ARRANGE
	SString<4> szBase = L"ザクⅡ";
	auto szText = StringBufferW(szBase);

	// ACT & ASSERT
	EXPECT_THAT(szText.append(L"Ⅱ MS-06F"s), STRUNCATE);

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"ザクⅡ"));
	EXPECT_THAT(szText.length(), 3);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 末尾に文字列ポインタを追加する(途中にNULを含む)
 */
TEST(StringBufferW, append202)
{
	 // ARRANGE
	 SString<4> szBase = L"t";
	 auto szText = StringBufferW(szBase);

	// ACT
	szText += L"e\0t";

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"te"));
	EXPECT_THAT(szText.length(), 2);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 末尾に文字列参照を追加する(途中にNULを含む)
 */
TEST(StringBufferW, append204)
{
	 // ARRANGE
	 SString<4> szBase = L"t";
	 auto szText = StringBufferW(szBase);

	// ACT
	szText += L"e\0t"sv;

	// ASSERT
	EXPECT_THAT(szText, StrEq(L"te"));
	EXPECT_THAT(szText.length(), 2);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 文字列長を計算する
 */
TEST(StringBufferW, length)
{
	 // ARRANGE
	 SString<4> szBase = L"初期値";
	 auto szText = StringBufferW(szBase);
	 const wchar_t dummy = 0x2025;

	 // ASSERT
	 EXPECT_EQ(szText.data()[3], 0);
	 EXPECT_EQ(szText.length(), 3);

	 // ACT(領域全体にゴミ投入)
	 auto_memset(szText.data(), dummy, int(std::size(szText)));

	 // ASSERT
	 EXPECT_EQ(szText.data()[3], dummy);
	 EXPECT_EQ(szText.length(), 0);

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
 * @brief StringBufferWのテスト
 * 
 * 文字列が空かどうか判定する
 */
TEST(StringBufferW, empty)
{
	// ARRANGE
	SString<4> szBase = L"初期値";
	auto szText = StringBufferW(szBase);

	// ASSERT
	EXPECT_FALSE(szText.empty());

	// ACT
	szText = nullptr;

	// ASSERT
	EXPECT_TRUE(szText.empty());
}

/*!
 * @brief StringBufferWのテスト
 */
TEST(StringBufferW, constAt)
{
	// ARRANGE
	SString<4> szBase = L"初期値";
	const auto szText = StringBufferW(szBase);

	// ASSERT
	EXPECT_THAT(szText.at(0), L'初');
	EXPECT_THAT(szText.at(1), L'期');
	EXPECT_THAT(szText.at(2), L'値');
	EXPECT_THAT(szText.at(3), '\0');

	EXPECT_THROW({ szText.at(4); }, std::out_of_range);
	EXPECT_THROW({ szText.at(5); }, std::out_of_range);
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 添え字演算子
 */
TEST(StringBufferW, subscriptOperator001)
{
	// ARRANGE
	SString<4> szBase = L"初期値";
	auto szText = StringBufferW(szBase);

	// ASSERT
	EXPECT_THAT(szText[0], L'初');
	EXPECT_THAT(szText[1], L'期');
	EXPECT_THAT(szText[2], L'値');
	EXPECT_THAT(szText[3], '\0');
	EXPECT_THAT(szText[4], '\0');
}

/*!
 * @brief StringBufferWのテスト
 * 
 * 添え字演算子
 */
TEST(StringBufferW, constSubscriptOperator001)
{
	// ARRANGE
	SString<4> szBase = L"初期値";
	const auto szText = StringBufferW(szBase);

	// ASSERT
	EXPECT_THAT(szText[0], L'初');
	EXPECT_THAT(szText[1], L'期');
	EXPECT_THAT(szText[2], L'値');
	EXPECT_THAT(szText[3], '\0');
	EXPECT_THAT(szText[4], '\0');
}

/*!
 * @brief StringBufferWのテスト
 *
 * 文字列参照型に変換する
 */
TEST(StringBufferW, toStringView)
{
	// ARRANGE
	SString<4> szBase = L"初期値";
	auto szText = StringBufferW(szBase);

	// ASSERT
	EXPECT_THAT(std::wstring_view(szText), StrEq(L"初期値"));
}

/*!
 * @brief StringBufferWのテスト
 *
 * ファイルパス型に変換する
 */
TEST(StringBufferW, toFilePath)
{
	// ARRANGE
	SString<4> szBase = L"初期値";
	auto szText = StringBufferW(szBase);

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
