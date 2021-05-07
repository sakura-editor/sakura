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

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <thread>

#include "config/maxdata.h"
#include "basis/primitive.h"
#include "debug/Debug2.h"
#include "basis/CMyString.h"
#include "mem/CNativeW.h"
#include "env/DLLSHAREDATA.h"
#include "util/file.h"
#include "config/system_constants.h"
#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"
#include "io/CZipFile.h"

#include "StartEditorProcessForTest.h"

#include "tests1_rc.h"

using namespace std::literals::string_literals;

#define RT_ZIPRES MAKEINTRESOURCE(101)

using BinarySequence = std::basic_string<std::byte>;
using BinarySequenceView = std::basic_string_view<std::byte>;

void CControlProcess_Terminate(std::wstring_view profileName);
std::filesystem::path GetTempFilePath(std::wstring_view prefix, std::wstring_view extension);
BinarySequence CopyBinaryFromResource(uint16_t nResourceId, LPCWSTR resource_type);
bool WriteBinaryToFile(BinarySequenceView bin, std::filesystem::path path);

/*!
	ファイルからバイナリデータを読み込む
 */
bool ReadBinaryFromFile(std::filesystem::path path, BinarySequence& bin)
{
	if (std::error_code ec; std::filesystem::exists(path, ec)){
		using std::ios;

		const auto fileSize = std::filesystem::file_size(path, ec);
		if (fileSize > 0) {
			bin.resize(fileSize, std::byte());
		}
		else {
			return false;
		}

		std::basic_ifstream<std::byte> is;
		is.open(path.c_str(), ios::binary);
		if (is) {
			is.read(bin.data(), bin.length());
			if (is) {
				return true;
			}
		}
	}
	return false;
}

/*!
	ファイルを比較する
 */
bool IsEqualFiles(
	std::filesystem::path path1,
	std::filesystem::path path2
)
{
	if (BinarySequence bin1; ReadBinaryFromFile(path1, bin1)) {
		if (BinarySequence bin2; ReadBinaryFromFile(path2, bin2)) {
			if (bin1 == bin2) {
				return true;
			}
		}
	}
	return false;
}

/*!
	Grepテストのためのフィクスチャクラス

	設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
	始動前に設定ファイルを削除するようにしている。
	テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
class GrepStdoutTest : public ::testing::TestWithParam<int> {
protected:
	/*!
		Grepテストのテスト用プロファイル名
	 */
	static std::wstring_view PROFILE_NAME;

	/*!
		設定ファイルのパス
	 */
	static std::filesystem::path INI_PATH;

	/*!
		テストの初期化時に1度だけ呼ばれる関数
	 */
	static void SetUpTestCase();

	/*!
		テストの終了後に1度だけ呼ばれる関数
	 */
	static void TearDownTestCase();

	/*!
		テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override;

	[[noreturn]] void StartGrepProcess(
		std::filesystem::path stdoutFilePath,
		std::optional<std::wstring_view> grepKey,
		std::optional<std::wstring_view> grepRep,
		std::optional<std::wstring_view> grepFile,
		std::optional<std::wstring_view> grepFolder,
		std::optional<ECodeType> grepCode,
		std::wstring_view opts
	) const;
	[[noreturn]] void StartReplaceFileContents(
		std::wstring_view grepKey,
		std::wstring_view grepRep,
		std::filesystem::path grepFile
	) const;
};

/*!
	Grepテストのテスト用プロファイル名
 */
std::wstring_view GrepStdoutTest::PROFILE_NAME = L"grepstdout";

/*!
	設定ファイルのパス
 */
std::filesystem::path GrepStdoutTest::INI_PATH;

/*!
	テストの初期化時に1度だけ呼ばれる関数

	GetIniFileNameを使ってtests1.iniのパスを取得する。
	zipリソースからテストデータを展開する。
 */
void GrepStdoutTest::SetUpTestCase()
{
	// コマンドラインのインスタンスを用意する
	CCommandLine commandLine;
	const auto strCommandLine = strprintf(LR"(-PROF="%s")", PROFILE_NAME.data());
	commandLine.ParseCommandLine(strCommandLine.data(), false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, strCommandLine.data());

	// INIファイルのパスを取得
	INI_PATH = GetIniFileName();

	// 一時ファイル名を生成する
	// zipファイルパスの拡張子はzipにしないと動かない。
	auto tempPath = GetTempFilePath(L"tes", L"zip");

	// リソースからzipファイルデータを抽出して一時ファイルに書き込む
	const auto bin = CopyBinaryFromResource(IDR_ZIPRES2, RT_ZIPRES);
	WriteBinaryToFile(bin, tempPath);

	// IShellDispatchを使うためにOLEを初期化する
	if (SUCCEEDED(::OleInitialize(nullptr))) {
		// zipファイルを解凍する
		CZipFile cZipFile;
		cZipFile.SetZip(tempPath.c_str());
		cZipFile.Unzip(std::filesystem::current_path().c_str());

		// 作成した一時ファイルを削除する
		std::filesystem::remove(tempPath);

		// OLEをシャットダウンする
		::OleUninitialize();
	}
}

/*!
	テストの終了後に1度だけ呼ばれる関数

	zipリソースから展開したテストデータを削除する。
 */
void GrepStdoutTest::TearDownTestCase()
{
	std::filesystem::remove_all(LR"(test-data\)");
	std::filesystem::remove_all(LR"(expect-data\)");
}

/*!
	テストが実行された直後に毎回呼ばれる関数

	INIファイルをプロファイルフォルダごと削除する
 */
void GrepStdoutTest::TearDown()
{
	// INIファイルをフォルダごと削除する
	std::filesystem::remove_all(INI_PATH.parent_path());
}

/*!
	Grepテスト専用wWinMain呼出のラッパー関数

	単体テストから Grepテスト を呼び出すためのラッパー関数です。
 */
[[noreturn]] void GrepStdoutTest::StartGrepProcess(
	std::filesystem::path stdoutFilePath,
	std::optional<std::wstring_view> grepKey,
	std::optional<std::wstring_view> grepRep,
	std::optional<std::wstring_view> grepFile,
	std::optional<std::wstring_view> grepFolder,
	std::optional<ECodeType> grepCode,
	std::wstring_view opts
) const
{
	// stdout用にファイルを作る
	HANDLE hStdoutFile = ::CreateFileW(
		stdoutFilePath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_DELETE,
		static_cast<LPSECURITY_ATTRIBUTES>(nullptr),
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY,
		static_cast<HANDLE>(nullptr)
	);

	if (hStdoutFile == INVALID_HANDLE_VALUE) {
		// ファイルを開けなかったら異常終了
		std::runtime_error("openning stdout file has failed.");
	}
	else {
		// 出力ファイルにBOMを書いておく
		DWORD dwWrite = 0;
		uint16_t bom = 0xFEFF;
		::WriteFile(hStdoutFile, &bom, sizeof(bom), &dwWrite, nullptr);
	}

	const auto hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);

	if (::SetStdHandle(STD_OUTPUT_HANDLE, hStdoutFile)) {

		// 実行中モジュールのインスタンスハンドルを取得する
		HINSTANCE hInstance = ::GetModuleHandleW(nullptr);

		// wWinMainに渡すためのコマンドラインを用意する
		std::wstring commandLine(L"-GREPMODE");
		commandLine += strprintf(LR"( -PROF="%s")", PROFILE_NAME.data());
		commandLine += strprintf(LR"( -CODE=%d)", static_cast<int>(CODE_UNICODE));
		if (grepKey.has_value()) {
			commandLine += strprintf(LR"( -GKEY="%s")", grepKey.value().data());
		}
		if (grepRep.has_value()) {
			commandLine += strprintf(LR"( -GREPR="%s")", grepRep.value().data());
		}
		if (grepFile.has_value()) {
			commandLine += strprintf(LR"( -GFILE="%s")", grepFile.value().data());
		}
		if (grepFolder.has_value()) {
			commandLine += strprintf(LR"( -GFOLDER="%s")", grepFolder.value().data());
		}
		commandLine += strprintf(LR"( -GCODE=%d)", static_cast<int>(grepCode.value_or(CODE_AUTODETECT)));

		commandLine += opts;

		// wWinMainを起動する
		const int ret = wWinMain(hInstance, nullptr, commandLine.data(), SW_SHOWDEFAULT);

		::SetStdHandle(STD_OUTPUT_HANDLE, hStdout);

		::CloseHandle(hStdoutFile);

		::exit(ret);
	}
}

/*!
	Grep置換専用wWinMain呼出のラッパー関数

	Grep結果に埋め込まれたパスを置換して、期待結果と比較できるようにするために使う。
 */
[[noreturn]] void GrepStdoutTest::StartReplaceFileContents(
	std::wstring_view grepKey,
	std::wstring_view grepRep,
	std::filesystem::path grepFile
) const
{
	// wWinMainに渡すためのコマンドラインを用意する
	std::wstring commandLine(L"-GREPMODE");
	commandLine += strprintf(LR"( -PROF="%s")", PROFILE_NAME.data());
	commandLine += strprintf(LR"( -GKEY="%s")", grepKey.data());
	commandLine += strprintf(LR"( -GREPR="%s")", grepRep.data());
	commandLine += strprintf(LR"( -GFILE="%s")", grepFile.filename().c_str());
	commandLine += strprintf(LR"( -GFOLDER="%s")", grepFile.parent_path().c_str());
	commandLine += strprintf(LR"( -GCODE=%d)", static_cast<int>(CODE_AUTODETECT));
	commandLine += LR"( -GOPT=PU1)";

	// wWinMainを起動する
	const int ret = StartEditorProcessForTest(commandLine);

	::exit(ret);
}

/*!
	正規表現Grep検索のテスト
 */
TEST_F(GrepStdoutTest, runRegexGrep001)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			LR"(\d+)",
			std::nullopt,
			L"!除外ファイル.txt;#除外フォルダ;*.*",
			LR"(test-data\)",
			std::nullopt,
			LR"( -GOPT=1PRSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runRegexGrep001.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	正規表現Grep検索のテスト
 */
TEST_F(GrepStdoutTest, runRegexGrep002)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			LR"(\d+)",
			std::nullopt,
			L"!除外ファイル.txt;#除外フォルダ;*.*",
			LR"(test-data\)",
			std::nullopt,
			LR"( -GOPT=2PRSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runRegexGrep002.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	正規表現Grep検索のテスト
 */
TEST_F(GrepStdoutTest, runRegexGrep003)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			LR"(\d+)",
			std::nullopt,
			L"!除外ファイル.txt;#除外フォルダ;*.*",
			LR"(test-data\)",
			std::nullopt,
			LR"( -GOPT=3PRSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runRegexGrep003.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	正規表現Grep検索のテスト
 */
TEST_F(GrepStdoutTest, runRegexGrep004)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			LR"(\d+)",
			std::nullopt,
			L"!除外ファイル.txt;#除外フォルダ;*.*",
			LR"(test-data\)",
			std::nullopt,
			LR"( -GOPT=1NRSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runRegexGrep004.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	ファイル検索のテスト
 */
TEST_F(GrepStdoutTest, runFileSearch001)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			std::nullopt,
			std::nullopt,
			L"*.*",
			LR"(test-data\)",
			CODE_UNICODE,
			LR"( -GOPT=1PU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*" );

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runFileSearch001.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	ファイル検索のテスト
 */
TEST_F(GrepStdoutTest, runFileSearch002)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			std::nullopt,
			std::nullopt,
			L"*.*",
			LR"(test-data\)",
			CODE_UNICODE,
			LR"( -GOPT=1PSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runFileSearch002.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	ファイル検索のテスト
 */
TEST_F(GrepStdoutTest, runFileSearch003)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			std::nullopt,
			std::nullopt,
			L"!除外ファイル.txt;#除外フォルダ;*.*",
			LR"(test-data\)",
			CODE_UNICODE,
			LR"( -GOPT=1PSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runFileSearch003.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	ファイル検索のテスト
 */
TEST_F(GrepStdoutTest, runFileSearch004)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			std::nullopt,
			std::nullopt,
			L"!除外ファイル.txt;#除外フォルダ;*.*",
			LR"(test-data\)",
			std::nullopt,
			LR"( -GOPT=1PSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runFileSearch004.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	ファイル検索のテスト
 */
TEST_F(GrepStdoutTest, runFileSearch005)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			std::nullopt,
			std::nullopt,
			L"!除外ファイル.txt;#除外フォルダ;*.*",
			LR"(test-data\)",
			std::nullopt,
			LR"( -GOPT=2PSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runFileSearch005.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	ファイル検索のテスト
 */
TEST_F(GrepStdoutTest, runFileSearch006)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			std::nullopt,
			std::nullopt,
			L"!除外ファイル.txt;#除外フォルダ;*.*",
			LR"(test-data\)",
			std::nullopt,
			LR"( -GOPT=3PSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runFileSearch006.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	ファイル検索のテスト
 */
TEST_F(GrepStdoutTest, runFileSearch007)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			std::nullopt,
			std::nullopt,
			L"!除外ファイル.txt;#除外フォルダ;*.*",
			LR"(test-data\)",
			std::nullopt,
			LR"( -GOPT=1BPSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runFileSearch007.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	ファイル検索のテスト
 */
TEST_F(GrepStdoutTest, runFileSearch008)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			std::nullopt,
			std::nullopt,
			L"!除外ファイル.txt;#除外フォルダ;*.*",
			LR"(test-data\)",
			std::nullopt,
			LR"( -GOPT=2BPSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runFileSearch008.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}

/*!
	ファイル検索のテスト
 */
TEST_F(GrepStdoutTest, runFileSearch009)
{
	constexpr auto& szResultName = L"grepresult.txt";

	const auto resultPath = std::filesystem::current_path().append(szResultName);

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc1 = [resultPath, this]() {
		// Grepプロセスを起動する
		StartGrepProcess(
			resultPath,
			std::nullopt,
			std::nullopt,
			L"!除外ファイル.txt;#除外フォルダ;*.*",
			LR"(test-data\)",
			std::nullopt,
			LR"( -GOPT=3BPSU)"
		);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc1(); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ StartReplaceFileContents(resultPath.parent_path().c_str(), LR"(%BASE_DIR%)", resultPath); }, ::testing::ExitedWithCode(0), ".*");

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(PROFILE_NAME);

	// 結果を期待値と比較する
	ASSERT_TRUE(IsEqualFiles(szResultName, LR"(expect-data\runFileSearch009.txt)"));

	// 結果ファイルを削除する
	std::filesystem::remove(resultPath);
}
