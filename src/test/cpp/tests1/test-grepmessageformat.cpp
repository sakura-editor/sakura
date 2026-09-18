/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include <tchar.h>
#include <Windows.h>

#include <string>
#include <string_view>

#include "grep/GrepMessageFormat.h"
#include "env/ShareDataTestSuite.hpp"

/*!
 * @brief lineColumnToString のテスト
 *  最小値を与えて書式を確認する
 */
TEST(GrepMessageFormat, lineColumnToStringMinimum)
{
	wchar_t strWork[64];
	ASSERT_STREQ(L"(1,1)", lineColumnToString(strWork, 1, 1));
}

/*!
 * @brief lineColumnToString のテスト
 *  複数桁の行番号・桁番号を与えて書式を確認する
 */
TEST(GrepMessageFormat, lineColumnToStringMultipleDigits)
{
	wchar_t strWork[64];
	ASSERT_STREQ(L"(1234,56)", lineColumnToString(strWork, 1234, 56));
}

/*!
 * @brief lineColumnToString のテスト
 *  int を超える行番号を与えても桁落ちしないことを確認する
 */
TEST(GrepMessageFormat, lineColumnToStringLargeLineNumber)
{
	wchar_t strWork[64];
	ASSERT_STREQ(L"(4294967296,1)", lineColumnToString(strWork, 4294967296LL, 1));
}

/*!
 * @brief OutputPathInfo のテスト
 *  出力形式3（結果のみ）では何も出力せず、フラグも変化しないこと
 */
TEST(GrepMessageFormat, OutputPathInfoResultOnlyOutputsNothing)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 3;
	gi.bGrepOutputBaseFolder = true;
	gi.bGrepSeparateFolder = true;
	bool bOutputBaseFolder = false;
	bool bOutputFolderName = false;
	BOOL bOutFileName = FALSE;

	OutputPathInfo( cmemMessage, gi, L"C:\\work\\a.txt", L"C:\\work",
		L"sub", L"sub\\a.txt", L" [SJIS]",
		bOutputBaseFolder, bOutputFolderName, bOutFileName );

	EXPECT_THAT(cmemMessage.GetStringLength(), 0);
	EXPECT_FALSE(bOutputBaseFolder);
	EXPECT_FALSE(bOutputFolderName);
	EXPECT_FALSE(bOutFileName);
}

/*!
 * @brief OutputPathInfo のテスト
 *  出力形式1でベースフォルダーを出力し、フラグが立つこと
 */
TEST(GrepMessageFormat, OutputPathInfoBaseFolderNormalStyle)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 1;
	gi.bGrepOutputBaseFolder = true;
	bool bOutputBaseFolder = false;
	bool bOutputFolderName = false;
	BOOL bOutFileName = FALSE;

	OutputPathInfo( cmemMessage, gi, L"C:\\work\\a.txt", L"C:\\work",
		L"", L"a.txt", L" [SJIS]",
		bOutputBaseFolder, bOutputFolderName, bOutFileName );

	ASSERT_STREQ(L"■\"C:\\work\"\r\n", cmemMessage.GetStringPtr());
	EXPECT_TRUE(bOutputBaseFolder);
}

/*!
 * @brief OutputPathInfo のテスト
 *  フォルダー毎表示が有効な場合はベースフォルダーの記号が変わること
 */
TEST(GrepMessageFormat, OutputPathInfoBaseFolderSeparateFolder)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 1;
	gi.bGrepOutputBaseFolder = true;
	gi.bGrepSeparateFolder = true;
	bool bOutputBaseFolder = false;
	bool bOutputFolderName = false;
	BOOL bOutFileName = FALSE;

	OutputPathInfo( cmemMessage, gi, L"C:\\work\\sub\\a.txt", L"C:\\work",
		L"sub", L"sub\\a.txt", L" [SJIS]",
		bOutputBaseFolder, bOutputFolderName, bOutFileName );

	ASSERT_STREQ(L"◎\"C:\\work\"\r\n■\"sub\"\r\n", cmemMessage.GetStringPtr());
	EXPECT_TRUE(bOutputBaseFolder);
	EXPECT_TRUE(bOutputFolderName);
}

/*!
 * @brief OutputPathInfo のテスト
 *  フォルダー名が空の場合は記号のみを出力すること
 */
TEST(GrepMessageFormat, OutputPathInfoEmptyFolderName)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 1;
	gi.bGrepSeparateFolder = true;
	bool bOutputBaseFolder = false;
	bool bOutputFolderName = false;
	BOOL bOutFileName = FALSE;

	OutputPathInfo( cmemMessage, gi, L"C:\\work\\a.txt", L"C:\\work",
		L"", L"a.txt", L" [SJIS]",
		bOutputBaseFolder, bOutputFolderName, bOutFileName );

	ASSERT_STREQ(L"■\r\n", cmemMessage.GetStringPtr());
	EXPECT_TRUE(bOutputFolderName);
}

/*!
 * @brief OutputPathInfo のテスト
 *  出力形式2（WZ風）ではファイル名を出力し、bOutFileName が立つこと
 */
TEST(GrepMessageFormat, OutputPathInfoWzStyleFileName)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 2;
	bool bOutputBaseFolder = false;
	bool bOutputFolderName = false;
	BOOL bOutFileName = FALSE;

	OutputPathInfo( cmemMessage, gi, L"C:\\work\\a.txt", L"C:\\work",
		L"", L"a.txt", L" [SJIS]",
		bOutputBaseFolder, bOutputFolderName, bOutFileName );

	ASSERT_STREQ(L"■\"C:\\work\\a.txt\" [SJIS]\r\n", cmemMessage.GetStringPtr());
	EXPECT_TRUE(bOutFileName);
}

/*!
 * @brief OutputPathInfo のテスト
 *  すでに出力済みの場合は二重に出力しないこと
 */
TEST(GrepMessageFormat, OutputPathInfoDoesNotOutputTwice)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 2;
	gi.bGrepOutputBaseFolder = true;
	gi.bGrepSeparateFolder = true;
	bool bOutputBaseFolder = true;
	bool bOutputFolderName = true;
	BOOL bOutFileName = TRUE;

	OutputPathInfo( cmemMessage, gi, L"C:\\work\\sub\\a.txt", L"C:\\work",
		L"sub", L"sub\\a.txt", L" [SJIS]",
		bOutputBaseFolder, bOutputFolderName, bOutFileName );

	EXPECT_THAT(cmemMessage.GetStringLength(), 0);
}

/*!
 * @brief OutputPathInfo のテスト
 *  出力形式2でフォルダー毎表示が有効な場合は相対パスと別記号を使うこと
 */
TEST(GrepMessageFormat, OutputPathInfoWzStyleSeparateFolder)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 2;
	gi.bGrepSeparateFolder = true;
	// ベースフォルダーとフォルダー名は出力済みにして、ファイル名の分岐だけを見る
	bool bOutputBaseFolder = true;
	bool bOutputFolderName = true;
	BOOL bOutFileName = FALSE;

	OutputPathInfo( cmemMessage, gi, L"C:\\work\\sub\\a.txt", L"C:\\work",
		L"sub", L"sub\\a.txt", L" [SJIS]",
		bOutputBaseFolder, bOutputFolderName, bOutFileName );

	ASSERT_STREQ(L"◆\"sub\\a.txt\" [SJIS]\r\n", cmemMessage.GetStringPtr());
	EXPECT_TRUE(bOutFileName);
}

/*!
 * @brief OutputPathInfo のテスト
 *  既存の内容が長い場合でもバッファを広げて追記できること
 */
TEST(GrepMessageFormat, OutputPathInfoExpandsBuffer)
{
	// 初期バッファ1024を超えるよう、あらかじめ長い内容を入れておく
	const std::wstring prefix(800, L'x');
	CNativeW cmemMessage(prefix.c_str());
	GrepInfo gi;
	gi.nGrepOutputStyle = 1;
	gi.bGrepOutputBaseFolder = true;
	bool bOutputBaseFolder = false;
	bool bOutputFolderName = false;
	BOOL bOutFileName = FALSE;

	OutputPathInfo( cmemMessage, gi, L"C:\\work\\a.txt", L"C:\\work",
		L"", L"a.txt", L" [SJIS]",
		bOutputBaseFolder, bOutputFolderName, bOutFileName );

	const std::wstring expected = prefix + L"■\"C:\\work\"\r\n";
	ASSERT_STREQ(expected.c_str(), cmemMessage.GetStringPtr());
	EXPECT_TRUE(bOutputBaseFolder);
}

/*!
 * SetGrepResult のテストのためのフィクスチャクラス
 *
 * SetGrepResult は GetDllShareData() を参照するため、共有データの用意が必要。
 */
class SetGrepResultTest : public ::testing::Test {
protected:
	static void SetUpTestSuite() { env::ShareDataTestSuite::SetUpShareData(); }
	static void TearDownTestSuite() { env::ShareDataTestSuite::TearDownShareData(); }
};

/*!
 * @brief SetGrepResult のテスト
 *  出力形式1・該当行出力の書式を確認する
 */
TEST_F(SetGrepResultTest, NormalStyleHitLine)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 1;
	gi.nGrepOutputLineType = 1;

	constexpr std::wstring_view line = L"hello\r\n";
	SetGrepResult( cmemMessage, L"a.txt", L" [SJIS]", 12, 3,
		line.data(), static_cast<int>(line.length()), 2,
		nullptr, 0, gi );

	ASSERT_STREQ(L"a.txt(12,3) [SJIS]: hello\r\n", cmemMessage.GetStringPtr());
}

/*!
 * @brief SetGrepResult のテスト
 *  ベースフォルダー表示が有効な場合は先頭に記号が付くこと
 */
TEST_F(SetGrepResultTest, NormalStyleWithBaseFolderMark)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 1;
	gi.nGrepOutputLineType = 1;
	gi.bGrepOutputBaseFolder = true;

	constexpr std::wstring_view line = L"hello\r\n";
	SetGrepResult( cmemMessage, L"a.txt", L" [SJIS]", 1, 1,
		line.data(), static_cast<int>(line.length()), 2,
		nullptr, 0, gi );

	ASSERT_STREQ(L"・a.txt(1,1) [SJIS]: hello\r\n", cmemMessage.GetStringPtr());
}

/*!
 * @brief SetGrepResult のテスト
 *  出力形式2（WZ風）の書式を確認する
 */
TEST_F(SetGrepResultTest, WzStyleHitLine)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 2;
	gi.nGrepOutputLineType = 1;

	constexpr std::wstring_view line = L"hello\r\n";
	SetGrepResult( cmemMessage, L"a.txt", L" [SJIS]", 12, 3,
		line.data(), static_cast<int>(line.length()), 2,
		nullptr, 0, gi );

	ASSERT_STREQ(L"・(    12,3    ): hello\r\n", cmemMessage.GetStringPtr());
}

/*!
 * @brief SetGrepResult のテスト
 *  出力形式3（結果のみ）では見出しが付かないこと
 */
TEST_F(SetGrepResultTest, ResultOnlyStyleHitLine)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 3;
	gi.nGrepOutputLineType = 1;

	constexpr std::wstring_view line = L"hello\r\n";
	SetGrepResult( cmemMessage, L"a.txt", L" [SJIS]", 12, 3,
		line.data(), static_cast<int>(line.length()), 2,
		nullptr, 0, gi );

	ASSERT_STREQ(L"hello\r\n", cmemMessage.GetStringPtr());
}

/*!
 * @brief SetGrepResult のテスト
 *  該当部分出力では一致した文字列だけが出力されること
 */
TEST_F(SetGrepResultTest, MatchedPartOnly)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 3;
	gi.nGrepOutputLineType = 0;

	constexpr std::wstring_view line = L"hello world\r\n";
	constexpr std::wstring_view match = L"world";
	SetGrepResult( cmemMessage, L"a.txt", L" [SJIS]", 1, 7,
		line.data(), static_cast<int>(line.length()), 2,
		match.data(), static_cast<int>(match.length()), gi );

	ASSERT_STREQ(L"world\r\n", cmemMessage.GetStringPtr());
}

/*!
 * @brief SetGrepResult のテスト
 *  該当部分が改行で終わる場合は改行を重ねて出力しないこと
 */
TEST_F(SetGrepResultTest, MatchedPartEndingWithEol)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 3;
	gi.nGrepOutputLineType = 0;

	constexpr std::wstring_view line = L"hello\r\n";
	constexpr std::wstring_view match = L"hello\r\n";
	SetGrepResult( cmemMessage, L"a.txt", L" [SJIS]", 1, 1,
		line.data(), static_cast<int>(line.length()), 2,
		match.data(), static_cast<int>(match.length()), gi );

	ASSERT_STREQ(L"hello\r\n", cmemMessage.GetStringPtr());
}

/*!
 * @brief SetGrepResult のテスト
 *  該当行が長すぎる場合は上限で切り捨てられること
 */
TEST_F(SetGrepResultTest, HitLineIsTruncated)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 1;	// ノーマルの上限は2000文字
	gi.nGrepOutputLineType = 1;

	std::wstring line(2100, L'a');
	line += L"\r\n";
	SetGrepResult( cmemMessage, L"a.txt", L" [SJIS]", 1, 1,
		line.data(), static_cast<int>(line.length()), 2,
		nullptr, 0, gi );

	const std::wstring expected =
		std::wstring(L"a.txt(1,1) [SJIS]: ") + std::wstring(2000, L'a') + L"\r\n";
	ASSERT_STREQ(expected.c_str(), cmemMessage.GetStringPtr());
}

/*!
 * @brief SetGrepResult のテスト
 *  該当部分が長すぎる場合は上限で切り捨てられること
 */
TEST_F(SetGrepResultTest, MatchedPartIsTruncated)
{
	CNativeW cmemMessage(L"");
	GrepInfo gi;
	gi.nGrepOutputStyle = 3;	// 結果のみの上限は2500文字
	gi.nGrepOutputLineType = 0;

	std::wstring line(2600, L'b');
	line += L"\r\n";
	SetGrepResult( cmemMessage, L"a.txt", L" [SJIS]", 1, 1,
		line.data(), static_cast<int>(line.length()), 2,
		line.data(), 2600, gi );

	const std::wstring expected = std::wstring(2500, L'b') + L"\r\n";
	ASSERT_STREQ(expected.c_str(), cmemMessage.GetStringPtr());
}
