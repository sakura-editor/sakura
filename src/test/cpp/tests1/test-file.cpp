/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
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
#include "env/CDataProfile.h"
#include "util/file.h"

std::filesystem::path GetIniFileNameForIO(bool bWrite);

namespace cxx {

bool WritePrivateProfileStringW(
	std::wstring_view appName,
	std::wstring_view keyName,
	std::wstring_view varValue,
	const std::optional<std::filesystem::path>& iniPath = std::nullopt
) noexcept
{
	return ::WritePrivateProfileStringW(std::data(appName), std::data(keyName), std::data(varValue), iniPath.has_value() ? iniPath.value().c_str() : nullptr);
}

std::wstring ExpandEnvironmentStringsW(const std::wstring& src)
{
	std::wstring expected(2048, L'\0');
	if (const auto ret = ::ExpandEnvironmentStringsW(std::data(src), std::data(expected), DWORD(std::size(expected)))) {
		expected.resize(ret - 1);
	}
	return expected;
}

} // namespace cxx

namespace path_util {

using cxx::WritePrivateProfileStringW;
using cxx::ExpandEnvironmentStringsW;

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

TEST(file, IsValidPathAvailableChar)
{
	EXPECT_TRUE(IsValidPathAvailableChar(L"test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L".\\test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"./test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:\\test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:/test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:\\"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:/"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:\\dir\\test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:\\dir\\dir2\\test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:dir\\dir2\\test.txt"));

	EXPECT_TRUE(IsValidPathAvailableChar(L"test:001.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"\\dir\\dir2\\test:001.txt"));

	// 特別考慮：DOSデバイスパスの?はtrue
	EXPECT_TRUE(IsValidPathAvailableChar(L"\\\\?\\C:\\test.txt"));

	EXPECT_FALSE(IsValidPathAvailableChar(L"test*.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"test?.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"test\".txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"test<.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"test>.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"test|.txt"));

	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir\\test*.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir\\test?.txt"));

	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir*\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir?\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir\"\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir<\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir>\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir|\\text.txt"));

	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\*dir\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\?dir\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\di*r\\text.txt"));


	EXPECT_FALSE(IsValidPathAvailableChar(L"\\\\?\\C:\\test?.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"\\\\?\\C:\\test*.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"\\\\?\\C:\\d*ir\\test.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"\\\\?\\C:\\dir*\\test.txt"));
}

/*!
 * @brief exeファイルパスの取得
 */
TEST(file, GetExeFileName)
{
	// 標準的なコードでexeファイルのパスを取得
	std::wstring expected(_MAX_PATH, L'\0');
	::GetModuleFileNameW(nullptr, std::data(expected), DWORD(std::size(expected)));
	expected.resize(::wcsnlen(std::data(expected), std::size(expected)));

	// 関数戻り値が、標準的なコードで取得した結果と一致すること
	auto exePath = GetExeFileName();
	EXPECT_THAT(exePath, StrEq(expected));
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

	// 戻り値取得用のバッファを指定しない場合、何も起きない
	GetExedir(nullptr);

	// 戻り値取得用のバッファ
	std::wstring buffer(_MAX_PATH, L'\0');

	// exeフォルダーの取得
	GetExedir(std::data(buffer));
	buffer.resize(::wcsnlen(std::data(buffer), std::size(buffer)));
	buffer += filename;
	EXPECT_THAT(buffer, StrEq(exeBasePath.c_str()));

	// バッファを再確保する
	buffer = std::wstring(_MAX_PATH, L'\0');

	// exe基準ファイルパスの取得
	GetExedir(std::data(buffer), filename);
	EXPECT_THAT(buffer, StartsWith(exeBasePath.c_str()));
}

/*!
 * @brief iniファイルパスの取得(プロセス未作成時)
 */
TEST(file, GetIniFileName_OutOfProcess)
{
	// exeファイルの拡張子をiniに変えたパスが返る
	auto iniPath = GetExeFileName().replace_extension(L".ini");
	EXPECT_THAT(GetIniFileName(), StrEq(iniPath.c_str()));
}

/*!
 * @brief iniファイルパスの取得(コントロールプロセス未初期化、かつ、プロファイル指定なし時)
 */
TEST(file, GetIniFileName_InProcessDefaultProfileUnInitialized)
{
	// コマンドラインのインスタンスを用意する
	auto pCommandLine = std::make_unique<CCommandLine>();
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="")");

	// exeファイルの拡張子をiniに変えたパスが返る
	auto path = GetExeFileName().replace_extension(L".ini");
	EXPECT_THAT(GetIniFileName(), StrEq(path.c_str()));
}

/*!
 * @brief iniファイルパスの取得(コントロールプロセス未初期化、かつ、プロファイル指定あり時)
 */
TEST(file, GetIniFileName_InProcessNamedProfileUnInitialized)
{
	// コマンドラインのインスタンスを用意する
	auto pCommandLine = std::make_unique<CCommandLine>();
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	// exeファイルの拡張子をiniに変えたパスの最後のフォルダーにプロファイル名を加えたパスが返る
	auto iniPath = GetExeFileName().replace_extension(L".ini");
	auto path = iniPath.parent_path().append(L"profile1").append(iniPath.filename().c_str());
	EXPECT_THAT(GetIniFileName(), StrEq(path.c_str()));
}

/*!
 * マルチユーザー設定ファイルを使うテストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
class CExeIniTest : public ::testing::Test {
protected:
	/*!
	 * マルチユーザー構成設定ファイルのパス
	 */
	std::filesystem::path exeIniPath;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// マルチユーザー構成設定ファイルのパス
		exeIniPath = GetExeFileName().concat(L".ini");
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// 存在チェック
		if (std::filesystem::exists(exeIniPath)) {
			// マルチユーザー構成設定ファイルを削除する
			std::filesystem::remove(exeIniPath);
		}

		// 削除チェック
		EXPECT_FALSE(fexist(exeIniPath));
	}
};

/*!
 * @brief iniファイルパスの取得
 */
TEST_F(CExeIniTest, GetIniFileName_PrivateRoamingAppData)
{
	// 設定を書き込む
	WritePrivateProfileStringW(L"Settings", L"MultiUser", L"1", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserRootFolder", L"0", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserSubFolder", L"", exeIniPath);

	// 実在チェック
	EXPECT_TRUE(fexist(exeIniPath));

	// コマンドラインのインスタンスを用意する
	auto pCommandLine = std::make_unique<CCommandLine>();
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	// 期待値を取得する
	auto expected = ExpandEnvironmentStringsW(LR"(%USERPROFILE%\AppData\Roaming\sakura\profile1\)");
	expected += GetIniFileName().filename();

	// テスト実施
	EXPECT_THAT(GetIniFileName(), StrEq(expected));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST_F(CExeIniTest, GetIniFileName_PrivateDesktop)
{
	// 設定を書き込む
	WritePrivateProfileStringW(L"Settings", L"MultiUser", L"1", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserRootFolder", L"3", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserSubFolder", L"sakura", exeIniPath);

	// 実在チェック
	EXPECT_TRUE(fexist(exeIniPath));

	// コマンドラインのインスタンスを用意する
	auto pCommandLine = std::make_unique<CCommandLine>();
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="")");

	// 期待値を取得する
	auto expected = ExpandEnvironmentStringsW(LR"(%USERPROFILE%\Desktop\sakura\)");
	expected += GetIniFileName().filename();

	// テスト実施
	EXPECT_THAT(GetIniFileName(), StrEq(expected));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST_F(CExeIniTest, GetIniFileName_PrivateProfile)
{
	// 設定を書き込む
	WritePrivateProfileStringW(L"Settings", L"MultiUser", L"1", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserRootFolder", L"1", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserSubFolder", L"sakura", exeIniPath);

	// 実在チェック
	EXPECT_TRUE(fexist(exeIniPath));

	// コマンドラインのインスタンスを用意する
	auto pCommandLine = std::make_unique<CCommandLine>();
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="")");

	// 期待値を取得する
	auto expected = ExpandEnvironmentStringsW(LR"(%USERPROFILE%\sakura\)");
	expected += GetIniFileName().filename();

	// テスト実施
	EXPECT_THAT(GetIniFileName(), StrEq(expected));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST_F(CExeIniTest, GetIniFileName_PrivateDocument)
{
	// 設定を書き込む
	WritePrivateProfileStringW(L"Settings", L"MultiUser", L"1", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserRootFolder", L"2", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserSubFolder", L"sakura", exeIniPath);

	// 実在チェック
	EXPECT_TRUE(fexist(exeIniPath));

	// コマンドラインのインスタンスを用意する
	auto pCommandLine = std::make_unique<CCommandLine>();
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="")");

	// 期待値を取得する
	auto expected = ExpandEnvironmentStringsW(LR"(%USERPROFILE%\Documents\sakura\)");
	expected += GetIniFileName().filename();

	// テスト実施
	EXPECT_THAT(GetIniFileName(), StrEq(expected));
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

	// 戻り値取得用のバッファを指定しない場合、何も起きない
	GetInidir(nullptr);

	// 戻り値取得用のバッファ
	std::wstring buffer(_MAX_PATH, L'\0');

	// iniフォルダーの取得
	GetInidir(std::data(buffer));
	buffer.resize(::wcsnlen(std::data(buffer), std::size(buffer)));
	buffer += filename;
	EXPECT_THAT(buffer, StrEq(iniBasePath.c_str()));

	// バッファを再確保する
	buffer = std::wstring(_MAX_PATH, L'\0');

	// ini基準ファイルパスの取得
	GetInidir(std::data(buffer), filename);
	EXPECT_THAT(buffer, StartsWith(iniBasePath.c_str()));
}

/*!
 * @brief INIファイルまたはEXEファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す（INIを優先）
 */
TEST(file, GetInidirOrExedir)
{
	// コマンドラインのインスタンスを用意する
	auto pCommandLine = std::make_unique<CCommandLine>();
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	std::wstring buf(_MAX_PATH, L'\0');

	GetInidirOrExedir(buf.data(), L"", true);
	EXPECT_THAT(buf, StartsWith(GetExeFileName().replace_filename(L"").c_str()));

	constexpr auto filename = L"test.txt";
	auto exeBasePath = GetExeFileName().parent_path().append(filename);
	auto iniBasePath = GetIniFileName().parent_path().append(filename);

	// EXE基準のファイルを作る
	CProfile().WriteProfile(exeBasePath.c_str(), L"file, GetInidirOrExedirのテスト");

	// INI基準のファイルを作る
	CProfile().WriteProfile(iniBasePath.c_str(), L"file, GetInidirOrExedirのテスト");

	// 両方あるときはINI基準のパスが変える
	GetInidirOrExedir(buf.data(), filename, true);
	EXPECT_THAT(buf, StartsWith(iniBasePath.c_str()));

	// INI基準パスのファイルを削除する
	std::filesystem::remove(iniBasePath);
	EXPECT_FALSE(fexist(iniBasePath));

	// EXE基準のみ存在するときはEXE基準のパスが変える
	GetInidirOrExedir(buf.data(), filename, true);
	EXPECT_THAT(buf, StartsWith(exeBasePath.c_str()));

	// EXE基準パスのファイルを削除する
	std::filesystem::remove(exeBasePath);
	EXPECT_FALSE(fexist(exeBasePath));

	// 両方ないときはINI基準のパスが変える
	GetInidirOrExedir(buf.data(), filename, true);
	EXPECT_THAT(buf, StartsWith(iniBasePath.c_str()));
}

/*!
 * @brief 入出力に使うiniファイルの判定
 */
TEST(file, GetIniFileNameForIO)
{
	auto iniPath = GetExeFileName().replace_extension(L".ini");

	// 書き込みモードのとき
	EXPECT_THAT(GetIniFileNameForIO(true), StrEq(iniPath.c_str()));

	// 書き込みモードでないとき
	EXPECT_THAT(GetIniFileNameForIO(false), StrEq(iniPath.c_str()));

	// 書き込みモードでないがiniファイルが実在するとき
	CProfile().WriteProfile(iniPath.c_str(), L"file, GetIniFileNameForIOのテスト");
	EXPECT_TRUE(fexist(iniPath));

	// テスト実施
	EXPECT_THAT(GetIniFileNameForIO(false), StrEq(iniPath.c_str()));

	// INIファイルを削除する
	std::filesystem::remove(iniPath);
	EXPECT_FALSE(fexist(iniPath));
}

/*!
 * @brief フルパスからファイル名を取り出す
 */
TEST(file, GetFileTitlePointer)
{
	// フルパスからファイル名を取得する
	EXPECT_STREQ(L"test.txt", GetFileTitlePointer(LR"(C:\Temp\test.txt)"));

	// フルパスにファイル名が含まれていない場合
	EXPECT_STREQ(L"", GetFileTitlePointer(LR"(C:\Temp\)"));

	// フルパスに\\が含まれていない場合
	EXPECT_STREQ(L"test.txt", GetFileTitlePointer(L"test.txt"));

	// 渡したパスが無効な場合は落ちます。
	EXPECT_DEATH({ GetFileTitlePointer(nullptr); }, ".*");
}

/*!
 * @brief ディレクトリの深さを計算する
 */
TEST(file, CalcDirectoryDepth)
{
	// ドライブ文字を含むフルパス
	EXPECT_EQ(1, CalcDirectoryDepth(LR"(C:\Temp\test.txt)"));

	// 共有フォルダーを含むフルパス
	EXPECT_EQ(1, CalcDirectoryDepth(LR"(\\host\Temp\test.txt)"));

	// ドライブなしのフルパス
	EXPECT_EQ(1, CalcDirectoryDepth(LR"(\Temp\test.txt)"));

	// 相対パス（？）
	EXPECT_EQ(1, CalcDirectoryDepth(LR"(C:\Temp\.\test.txt)"));

	// 渡したパスが無効な場合は落ちます。
	EXPECT_DEATH({ CalcDirectoryDepth(nullptr); }, ".*");
}

/*!
	FileMatchScoreSepExtのテスト
 */
TEST(file, FileMatchScoreSepExt)
{
	int result = 0;

	// FileNameSepExtのテストパターン
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test.txt)",
		LR"(C:\TEMP\TEST.TXT)");
	EXPECT_THAT(result, std::size(LR"(test.txt)") - 1);

	// FileNameSepExtのテストパターン（パスにフォルダーが含まれない）
	result = FileMatchScoreSepExt(
		LR"(TEST.TXT)",
		LR"(test.txt)");
	EXPECT_THAT(result, std::size(LR"(test.txt)") - 1);

	// FileNameSepExtのテストパターン（ファイル名がない）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\.txt)",
		LR"(C:\TEMP\.txt)");
	EXPECT_THAT(result, std::size(LR"(.txt)") - 1);

	// FileNameSepExtのテストパターン（拡張子がない）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test)",
		LR"(C:\TEMP\test)");
	EXPECT_THAT(result, std::size(LR"(test)") - 1);

	// 全く同じパス同士の比較（ファイル名＋拡張子が完全一致）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test.txt)",
		LR"(C:\TEMP\TEST.TXT)");
	EXPECT_THAT(result, std::size(LR"(test.txt)") - 1);

	// 異なるパスでファイル名＋拡張子が同じ（ファイル名＋拡張子が完全一致）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP1\TEST.TXT)",
		LR"(C:\TEMP2\test.txt)");
	EXPECT_THAT(result, std::size(LR"(test.txt)") - 1);

	// ファイル名が異なる1（最長一致を取得）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test.txt)",
		LR"(C:\TEMP\TEST1.TST)");
	EXPECT_THAT(result, std::size(LR"(test)") - 1 + std::size(LR"(.t)") - 1);

	// ファイル名が異なる2（最長一致を取得）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test1.tst)",
		LR"(C:\TEMP\TEST.TXT)");
	EXPECT_THAT(result, std::size(LR"(test)") - 1 + std::size(LR"(.t)") - 1);

	// 拡張子が異なる1（最長一致を取得）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test.txt)",
		LR"(C:\TEMP\TEXT.TXTX)");
	EXPECT_THAT(result, std::size(LR"(te)") - 1 + std::size(LR"(.txt)") - 1);

	// 拡張子が異なる2（最長一致を取得）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\text.txtx)",
		LR"(C:\TEMP\TEST.TXT)");
	EXPECT_THAT(result, std::size(LR"(te)") - 1 + std::size(LR"(.txt)") - 1);

	// サロゲート文字を含む1
	result = FileMatchScoreSepExt(
		L"C:\\TEMP\\test\xD83D\xDC49\xD83D\xDC46.TST",
		L"C:\\TEMP\\TEST\xD83D\xDC49\xD83D\xDC47.txt");
	EXPECT_THAT(result, std::size(LR"(testXX)") - 1 + std::size(LR"(.t)") - 1);

	// サロゲート文字を含む2
	result = FileMatchScoreSepExt(
		L"C:\\TEMP\\TEST\xD83D\xDC49\xD83D\xDC47.txt",
		L"C:\\TEMP\\test\xD83D\xDC49\xD83D\xDC46.TST");
	EXPECT_THAT(result, std::size(LR"(testXX)") - 1 + std::size(LR"(.t)") - 1);
}

/*!
	GetExtのテスト
 */
TEST(CFilePath, GetExt)
{
	CFilePath path;

	// 最も単純なパターン
	path = L"test.txt";
	EXPECT_THAT(path.GetExt(true), StrEq(L"txt"));

	// ファイルに拡張子がないパターン
	path = L"lib\\.NET Core\\README";
	EXPECT_THAT(path.GetExt(true), StrEq(L""));

	// 拡張子がない場合に返却されるポインタ値の確認
	EXPECT_EQ(path.GetExt(), path.c_str() + path.Length());

	// ファイルに拡張子がないパターン
	path = L"lib/.NET Core/README";
	EXPECT_THAT(path.GetExt(true), StrEq(L""));

	// 拡張子がない場合に返却されるポインタ値の確認
	EXPECT_EQ(path.GetExt(), path.c_str() + path.Length());
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
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\テンポラリ\test.txt)"));

	// バッファ不足（パターンに一致した部分が切り捨てられる）
	strBuf = std::wstring(6, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\テンポ)"));

	// バッファ不足（パターンに一致しない部分が切り捨てられる）
	strBuf = std::wstring(15, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\テンポラリ\test.t)"));

	// ソースが部分文字列（十分な大きさのバッファを指定）
	strBuf = std::wstring(50, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(std::wstring_view(LR"(C:\%Temp%\test.txt.bak)", 18), strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\テンポラリ\test.txt)"));

	// ソースが部分文字列（十分な大きさのバッファを指定）
	strBuf = std::wstring(50, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(std::wstring_view(LR"(C:\test.txt\%Temp%.bak)", 18), strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\test.txt\テンポラリ)"));

	// ソースが部分文字列（置換文字が1文字アウト、十分な大きさのバッファを指定）
	strBuf = std::wstring(50, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(std::wstring_view(LR"(C:\test.txt\%Temp%.bak)", 17), strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\test.txt\%Temp)"));

	// 置換対象が部分文字列（十分な大きさのバッファを指定）
	strBuf = std::wstring(50, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, std::wstring_view(LR"(%Temp%\)", 6), L"テンポラリ"), StrEq(LR"(C:\テンポラリ\test.txt)"));

	// 置換先が部分文字列（十分な大きさのバッファを指定）
	strBuf = std::wstring(50, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, L"%Temp%", std::wstring_view(L"テンポラリってる", 5)), StrEq(LR"(C:\テンポラリ\test.txt)"));
}

TEST(CFilePath, GetDirPath001)
{
	CFilePath path(LR"(C:\Temp\test.txt)");
	EXPECT_THAT(path.GetDirPath(), StrEq(LR"(C:\Temp\)"));
}

TEST(CFilePath, GetDirPath002)
{
	CFilePath path(LR"(C:\Temp\)");	//ファイル名がない
	EXPECT_THAT(path.GetDirPath(), StrEq(LR"(C:\Temp\)"));
}

TEST(CFilePath, GetDirPath003)
{
	CFilePath path(LR"(C:\Temp)");	//末尾 \ がない
	EXPECT_THAT(path.GetDirPath(), StrEq(LR"(C:\)"));
}

TEST(CFilePath, GetDirPath101)
{
	CFilePath path(L"");	//パスが空
	EXPECT_THAT(path.GetDirPath(), StrEq(L""));
}

TEST(CFilePath, GetDirPath102)
{
	CFilePath path(L"test.txt");	//ディレクトリがない
	EXPECT_THAT(path.GetDirPath(), StrEq(L""));
}

} // namespace path_util
