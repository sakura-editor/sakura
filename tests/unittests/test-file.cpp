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
#include <Shlwapi.h>

#include <cstdlib>
#include <fstream>

#include "config/maxdata.h"
#include "basis/primitive.h"
#include "debug/Debug2.h"
#include "basis/CMyString.h"
#include "mem/CNativeW.h"
#include "env/DLLSHAREDATA.h"
#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"
#include "CDataProfile.h"
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
 * @brief 既存コード互換用に残しておく関数のリグレッション
 */
TEST(file, Deprecated_GetExedir)
{
	// テストに使うファイル名(空でなければなんでもいい)
	constexpr const auto filename = L"README.txt";

	// 比較用関数呼び出し
	auto exeBasePath = GetExeFileName().parent_path().append(filename);

	// 戻り値取得用のバッファ
	WCHAR szBuf[_MAX_PATH];

	// 戻り値取得用のバッファを指定しない場合、何も起きない
	GetExedir(nullptr);

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

/*!
 * @brief iniファイルパスの取得(プロセス未作成時)
 */
TEST(file, GetIniFileName_OutOfProcess)
{
	// exeファイルの拡張子をiniに変えたパスが返る
	auto iniPath = GetExeFileName().replace_extension(L".ini");
	ASSERT_STREQ(iniPath.c_str(), GetIniFileName().c_str());
}

/*!
 * @brief iniファイルパスの取得(コントロールプロセス未初期化、かつ、プロファイル指定なし時)
 */
TEST(file, GetIniFileName_InProcessDefaultProfileUnInitialized)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="")");

	// exeファイルの拡張子をiniに変えたパスが返る
	auto path = GetExeFileName().replace_extension(L".ini");
	ASSERT_STREQ(path.c_str(), GetIniFileName().c_str());
}

/*!
 * @brief iniファイルパスの取得(コントロールプロセス未初期化、かつ、プロファイル指定あり時)
 */
TEST(file, GetIniFileName_InProcessNamedProfileUnInitialized)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	// exeファイルの拡張子をiniに変えたパスの最後のフォルダにプロファイル名を加えたパスが返る
	auto iniPath = GetExeFileName().replace_extension(L".ini");
	auto path = iniPath.parent_path().append(L"profile1").append(iniPath.filename().c_str());
	ASSERT_STREQ(path.c_str(), GetIniFileName().c_str());
}

/*!
 * @brief iniファイルパスの取得
 */
TEST(file, GetIniFileName_PrivateRoamingAppData)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	// マルチユーザ構成設定ファイルのパス
	auto exeIniPath = GetExeFileName().concat(L".ini");

	// 設定を書き込む
	::WritePrivateProfileString(L"Settings", L"MultiUser", L"1", exeIniPath.c_str());
	::WritePrivateProfileString(L"Settings", L"UserRootFolder", L"0", exeIniPath.c_str());
	::WritePrivateProfileString(L"Settings", L"UserSubFolder", L"", exeIniPath.c_str());

	// 実在チェック
	ASSERT_TRUE(fexist(exeIniPath.c_str()));

	// 期待値を取得する
	std::wstring expected(2048, L'\0');
	ASSERT_TRUE(ExpandEnvironmentStrings(LR"(%USERPROFILE%\AppData\Roaming\sakura\profile1\)", expected.data(), (DWORD)expected.capacity()));
	expected.assign(expected.data());
	expected += GetIniFileName().filename();

	// テスト実施
	ASSERT_STREQ(expected.c_str(), GetIniFileName().c_str());

	// INIファイルを削除する
	std::filesystem::remove(exeIniPath);

	// 削除チェック
	ASSERT_FALSE(fexist(exeIniPath.c_str()));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST(file, GetIniFileName_PrivateDesktop)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="")");

	// マルチユーザ構成設定ファイルのパス
	auto exeIniPath = GetExeFileName().concat(L".ini");

	// 設定を書き込む
	::WritePrivateProfileString(L"Settings", L"MultiUser", L"1", exeIniPath.c_str());
	::WritePrivateProfileString(L"Settings", L"UserRootFolder", L"3", exeIniPath.c_str());
	::WritePrivateProfileString(L"Settings", L"UserSubFolder", L"sakura", exeIniPath.c_str());

	// 実在チェック
	ASSERT_TRUE(fexist(exeIniPath.c_str()));

	// 期待値を取得する
	std::wstring expected(2048, L'\0');
	ASSERT_TRUE(ExpandEnvironmentStrings(LR"(%USERPROFILE%\Desktop\sakura\)", expected.data(), (DWORD)expected.capacity()));
	expected.assign(expected.data());
	expected += GetIniFileName().filename();

	// テスト実施
	ASSERT_STREQ(expected.c_str(), GetIniFileName().c_str());

	// INIファイルを削除する
	std::filesystem::remove(exeIniPath);

	// 削除チェック
	ASSERT_FALSE(fexist(exeIniPath.c_str()));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST(file, GetIniFileName_PrivateProfile)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="")");

	// マルチユーザ構成設定ファイルのパス
	auto exeIniPath = GetExeFileName().concat(L".ini");

	// 設定を書き込む
	::WritePrivateProfileString(L"Settings", L"MultiUser", L"1", exeIniPath.c_str());
	::WritePrivateProfileString(L"Settings", L"UserRootFolder", L"1", exeIniPath.c_str());
	::WritePrivateProfileString(L"Settings", L"UserSubFolder", L"sakura", exeIniPath.c_str());

	// 実在チェック
	ASSERT_TRUE(fexist(exeIniPath.c_str()));

	// 期待値を取得する
	std::wstring expected(2048, L'\0');
	ASSERT_TRUE(ExpandEnvironmentStrings(LR"(%USERPROFILE%\sakura\)", expected.data(), (DWORD)expected.capacity()));
	expected.assign(expected.data());
	expected += GetIniFileName().filename();

	// テスト実施
	ASSERT_STREQ(expected.c_str(), GetIniFileName().c_str());

	// INIファイルを削除する
	std::filesystem::remove(exeIniPath);

	// 削除チェック
	ASSERT_FALSE(fexist(exeIniPath.c_str()));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST(file, GetIniFileName_PrivateDocument)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="")");

	// マルチユーザ構成設定ファイルのパス
	auto exeIniPath = GetExeFileName().concat(L".ini");

	// 設定を書き込む
	::WritePrivateProfileString(L"Settings", L"MultiUser", L"1", exeIniPath.c_str());
	::WritePrivateProfileString(L"Settings", L"UserRootFolder", L"2", exeIniPath.c_str());
	::WritePrivateProfileString(L"Settings", L"UserSubFolder", L"sakura", exeIniPath.c_str());

	// 実在チェック
	ASSERT_TRUE(fexist(exeIniPath.c_str()));

	// 期待値を取得する
	std::wstring expected(2048, L'\0');
	ASSERT_TRUE(ExpandEnvironmentStrings(LR"(%USERPROFILE%\Documents\sakura\)", expected.data(), (DWORD)expected.capacity()));
	expected.assign(expected.data());
	expected += GetIniFileName().filename();

	// テスト実施
	ASSERT_STREQ(expected.c_str(), GetIniFileName().c_str());

	// INIファイルを削除する
	std::filesystem::remove(exeIniPath);

	// 削除チェック
	ASSERT_FALSE(fexist(exeIniPath.c_str()));
}

/*!
 * @brief 既存コード互換用に残しておく関数のリグレッション
 */
TEST(file, Deprecated_GetInidir)
{
	// テストに使うファイル名(空でなければなんでもいい)
	constexpr const auto filename = L"README.txt";

	// 比較用関数呼び出し
	auto iniBasePath = GetIniFileName().parent_path().append(filename);

	// 戻り値取得用のバッファ
	WCHAR szBuf[_MAX_PATH];

	// 戻り値取得用のバッファを指定しない場合、何も起きない
	GetInidir(nullptr);

	// iniフォルダの取得
	GetInidir(szBuf);
	::wcscat_s(szBuf, filename);
	ASSERT_STREQ(iniBasePath.c_str(), szBuf);

	// 一旦クリアする
	::wcscpy_s(szBuf, L"");

	// ini基準ファイルパスの取得
	GetInidir(szBuf, filename);
	ASSERT_STREQ(iniBasePath.c_str(), szBuf);
}

void EnsureDirectoryExist(const std::wstring& strProfileName);

/*!
 * @brief INIファイルまたはEXEファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す（INIを優先）
 */
TEST(file, GetInidirOrExedir)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	std::wstring buf(_MAX_PATH, L'\0');

	GetInidirOrExedir(buf.data(), L"", true);
	ASSERT_STREQ(GetExeFileName().replace_filename(L"").c_str(), buf.data());

	constexpr auto filename = L"test.txt";
	auto exeBasePath = GetExeFileName().parent_path().append(filename);
	auto iniBasePath = GetIniFileName().parent_path().append(filename);

	// EXE基準のファイルを作る
	CProfile().WriteProfile(exeBasePath.c_str(), L"file, GetInidirOrExedirのテスト");

	// INI基準のファイルを作る
	CProfile().WriteProfile(iniBasePath.c_str(), L"file, GetInidirOrExedirのテスト");

	// 両方あるときはINI基準のパスが変える
	GetInidirOrExedir(buf.data(), filename, true);
	ASSERT_STREQ(iniBasePath.c_str(), buf.data());

	// INI基準パスのファイルを削除する
	std::filesystem::remove(iniBasePath);
	ASSERT_FALSE(fexist(iniBasePath.c_str()));

	// EXE基準のみ存在するときはEXE基準のパスが変える
	GetInidirOrExedir(buf.data(), filename, true);
	ASSERT_STREQ(exeBasePath.c_str(), buf.data());

	// EXE基準パスのファイルを削除する
	std::filesystem::remove(exeBasePath);
	ASSERT_FALSE(fexist(exeBasePath.c_str()));

	// 両方ないときはINI基準のパスが変える
	GetInidirOrExedir(buf.data(), filename, true);
	ASSERT_STREQ(iniBasePath.c_str(), buf.data());
}

std::filesystem::path GetIniFileNameForIO(bool bWrite);

/*!
 * @brief 入出力に使うiniファイルの判定
 */
TEST(file, GetIniFileNameForIO)
{
	auto iniPath = GetExeFileName().replace_extension(L".ini");

	// 書き込みモードのとき
	ASSERT_STREQ(iniPath.c_str(), GetIniFileNameForIO(true).c_str());

	// 書き込みモードでないとき
	ASSERT_STREQ(iniPath.c_str(), GetIniFileNameForIO(false).c_str());

	// 書き込みモードでないがiniファイルが実在するとき
	CProfile().WriteProfile(iniPath.c_str(), L"file, GetIniFileNameForIOのテスト");
	ASSERT_TRUE(fexist(iniPath.c_str()));

	// テスト実施
	ASSERT_STREQ(iniPath.c_str(), GetIniFileNameForIO(false).c_str());

	// INIファイルを削除する
	std::filesystem::remove(iniPath);
	ASSERT_FALSE(fexist(iniPath.c_str()));
}

/*!
	GetExtのテスト
 */
TEST(CFilePath, GetExt)
{
	CFilePath path;

	// 最も単純なパターン
	path = L"test.txt";
	ASSERT_STREQ(L".txt", path.GetExt());
	ASSERT_STREQ(L"txt", path.GetExt(true));

	// ファイルに拡張子がないパターン
	path = L"lib\\.NET Core\\README";
	ASSERT_STREQ(L"", path.GetExt());
	ASSERT_STREQ(L"", path.GetExt(true));

	// 拡張子がない場合に返却されるポインタ値の確認
	ASSERT_EQ(path.c_str() + path.Length(), path.GetExt());

	// ファイルに拡張子がないパターン
	path = L"lib/.NET Core/README";
	ASSERT_STREQ(L"", path.GetExt());
	ASSERT_STREQ(L"", path.GetExt(true));

	// 拡張子がない場合に返却されるポインタ値の確認
	ASSERT_EQ(path.c_str() + path.Length(), path.GetExt());
}

/*!
	CFileNameManager::GetFilePathFormatのテスト
 */
TEST(CFileNameManager, GetFilePathFormat)
{
	// バッファ
	std::wstring strBuf;

	// 十分な大きさのバッファを指定
	strBuf = std::wstring(50, L'x');
	ASSERT_STREQ(LR"(C:\テンポラリ\test.txt)", CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"));

	// バッファ不足（パターンに一致した部分が切り捨てられる）
	strBuf = std::wstring(6, L'x');
	ASSERT_STREQ(LR"(C:\テンポ)", CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"));

	// バッファ不足（パターンに一致しない部分が切り捨てられる）
	strBuf = std::wstring(15, L'x');
	ASSERT_STREQ(LR"(C:\テンポラリ\test.t)", CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"));
}
