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
#include <gtest/gtest.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "util/file.h"

/*!
 * @brief パスがファイル名に使えない文字を含んでいるかチェックする
 */
TEST( file, IsInvalidFilenameChars )
{
	// ファイル名に使えない文字 = "\\/:*?\"<>|"
	// このうち、\\と/はパス区切りのため実質対象外になる。
	EXPECT_FALSE(IsInvalidFilenameChars(L"test.txt"));
	EXPECT_FALSE(IsInvalidFilenameChars(L".\\test.txt"));
	EXPECT_FALSE(IsInvalidFilenameChars(L"./test.txt"));
	EXPECT_FALSE(IsInvalidFilenameChars(L"C:\\test.txt"));
	EXPECT_FALSE(IsInvalidFilenameChars(L"C:/test.txt"));
	EXPECT_FALSE(IsInvalidFilenameChars(L"C:\\"));
	EXPECT_FALSE(IsInvalidFilenameChars(L"C:/"));

	EXPECT_FALSE(IsInvalidFilenameChars(L"test:001.txt"));

	EXPECT_TRUE(IsInvalidFilenameChars(L"test*.txt"));
	EXPECT_TRUE(IsInvalidFilenameChars(L"test?.txt"));
	EXPECT_TRUE(IsInvalidFilenameChars(L"test\".txt"));
	EXPECT_TRUE(IsInvalidFilenameChars(L"test<.txt"));
	EXPECT_TRUE(IsInvalidFilenameChars(L"test>.txt"));
	EXPECT_TRUE(IsInvalidFilenameChars(L"test|.txt"));
}

/*!
 * @brief exeファイルパスの取得
 */
TEST(file, GetExeFileName)
{
	// 標準的なコードでexeファイルのパスを取得
	std::wstring path(_MAX_PATH, L'\0');
	::GetModuleFileName(nullptr, path.data(), path.capacity());

	// 関数戻り値が、標準的なコードで取得した結果と一致すること
	auto exePath = GetExeFileName();
	ASSERT_STREQ(path.data(), exePath.c_str());
}

/*!
 * @brief exeフォルダのフルパスの取得
 */
TEST(file, GetExePath_Directory)
{
	// テスト対象関数呼び出し
	auto exeDir = GetExePath(L"");

	// 戻り値はファイル名を含まない
	ASSERT_FALSE(exeDir.has_filename());

	// パスコンポーネントの最終要素は空になる(\で終わっている)
	auto lastComponent = *(--exeDir.end());
	ASSERT_STREQ(L"", lastComponent.c_str());

	// 戻り値はexeファイルパスからファイル名を取り除いたものになる
	auto exePath = GetExeFileName();
	ASSERT_STREQ(exePath.remove_filename().c_str(), exeDir.c_str());
}

/*!
 * @brief exe基準のファイルパス(フルパス)の取得
 */
TEST(file, GetExePath_FileName)
{
	// テストに使うファイル名(空でなければなんでもいい)
	constexpr const auto filename = L"README.txt";

	// テスト対象関数呼び出し
	auto exeBasePath = GetExePath(filename);

	// 戻り値はファイル名を含む
	ASSERT_TRUE(exeBasePath.has_filename());

	// 戻り値のファイル名は指定したものになっている
	ASSERT_STREQ(filename, exeBasePath.filename().c_str());

	// 戻り値の親フォルダはexeファイルパスの親フォルダと等しい
	auto exePath = GetExeFileName();
	ASSERT_STREQ(exePath.parent_path().c_str(), exeBasePath.parent_path().c_str());
}

/*!
 * @brief 既存コード互換用に残しておく関数のリグレッション
 */
TEST(file, Deprecated_GetExedir)
{
	// テストに使うファイル名(空でなければなんでもいい)
	constexpr const auto filename = L"README.txt";

	// 比較用関数呼び出し
	auto exeBasePath = GetExePath(filename);

	// 戻り値取得用のバッファ
	WCHAR szBuf[_MAX_PATH];

	// exeフォルダの取得
	GetExedir(szBuf);
	::wcscat_s(szBuf, filename);
	ASSERT_STREQ(exeBasePath.c_str(), szBuf);

	// 一旦クリアする
	::wcscpy_s(szBuf, L"");

	// exe基準ファイルパスの取得
	GetExedir(szBuf, filename);
	ASSERT_STREQ(exeBasePath.c_str(), szBuf);
}
