/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

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

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <fstream>

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

#include "StartEditorProcessForTest.h"

using namespace std::literals::string_literals;

/*!
 * HANDLE型のスマートポインタを実現するためのdeleterクラス
 */
struct handle_closer
{
	void operator()( HANDLE handle ) const
	{
		::CloseHandle( handle );
	}
};

//! HANDLE型のスマートポインタ
using handleHolder = std::unique_ptr<std::remove_pointer<HANDLE>::type, handle_closer>;

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
class WinMainTest : public ::testing::TestWithParam<const wchar_t*> {
protected:
	/*!
	 * 設定ファイルのパス
	 *
	 * GetIniFileNameを使ってtests1.iniのパスを取得する。
	 */
	std::filesystem::path iniPath;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// テスト用プロファイル名
		const std::wstring_view profileName(GetParam());

		// コマンドラインのインスタンスを用意する
		CCommandLine commandLine;
		const auto strCommandLine = strprintf(LR"(-PROF="%s")", profileName.data());
		commandLine.ParseCommandLine(strCommandLine.data(), false);

		// プロセスのインスタンスを用意する
		CControlProcess dummy(nullptr, strCommandLine.data());

		// INIファイルのパスを取得
		iniPath = GetIniFileName();

		// INIファイルを削除する
		if (fexist(iniPath.c_str())) {
			std::filesystem::remove(iniPath);
		}
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// INIファイルを削除する
		if (fexist(iniPath.c_str())) {
			std::filesystem::remove(iniPath);
		}

		// プロファイル指定がある場合、フォルダーも削除しておく
		if (const std::wstring_view profileName(GetParam()); profileName.length() > 0) {
			std::filesystem::remove(iniPath.parent_path());
		}
	}
};

/*!
 * @brief コントロールプロセスの初期化完了を待つ
 *
 * CControlProcess::WaitForInitializedとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存CProcessFactory::WaitForInitializedControlProcess()と概ね等価です。
 */
void CControlProcess_WaitForInitialized(std::wstring_view profileName)
{
	// 初期化完了イベントを作成する
	std::wstring strInitEvent( GSTR_EVENT_SAKURA_CP_INITIALIZED );
	if (profileName.length() > 0) {
		strInitEvent += profileName;
	}
	auto hEvent = ::CreateEventW( NULL, TRUE, FALSE, strInitEvent.data() );
	if (!hEvent) {
		throw std::runtime_error( "create event failed." );
	}

	// イベントハンドラをスマートポインタに入れる
	handleHolder eventHolder( hEvent );

	// 初期化完了イベントを待つ
	DWORD dwRet = ::WaitForSingleObject( hEvent, 30000 );
	if( WAIT_TIMEOUT == dwRet ){
		throw std::runtime_error( "waitEvent is timeout." );
	}
}

/*!
 * @brief コントロールプロセスを起動する
 *
 * CControlProcess::Startとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存CProcessFactory::StartControlProcess()と概ね等価です。
 */
void CControlProcess_Start(std::wstring_view profileName)
{
	// スタートアップ情報
	STARTUPINFO si = { sizeof(STARTUPINFO), 0 };
	si.lpTitle = (LPWSTR)L"sakura control process";
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWDEFAULT;

	const auto exePath = GetExeFileName();

	std::wstring strProfileName;
	if (profileName.length() > 0) {
		strProfileName = profileName;
	}

	std::wstring strCommandLine = strprintf(LR"("%s" -PROF="%s" -NOWIN)", exePath.c_str(), strProfileName.c_str());

	LPWSTR pszCommandLine = strCommandLine.data();
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
	PROCESS_INFORMATION pi;

	// コントロールプロセスを起動する
	BOOL createSuccess = ::CreateProcess(
		exePath.c_str(),	// 実行可能モジュールパス
		pszCommandLine,		// コマンドラインバッファ
		NULL,				// プロセスのセキュリティ記述子
		NULL,				// スレッドのセキュリティ記述子
		FALSE,				// ハンドルの継承オプション(継承させない)
		dwCreationFlag,		// 作成のフラグ
		NULL,				// 環境変数(変更しない)
		NULL,				// カレントディレクトリ(変更しない)
		&si,				// スタートアップ情報
		&pi					// プロセス情報(作成されたプロセス情報を格納する構造体)
	);
	if( !createSuccess ){
		throw std::runtime_error( "create process failed." );
	}

	// 開いたハンドルは使わないので閉じておく
	::CloseHandle( pi.hThread );
	::CloseHandle( pi.hProcess );

	// コントロールプロセスの初期化完了を待つ
	CControlProcess_WaitForInitialized(profileName);
}

/*!
 * @brief コントロールプロセスに終了指示を出して終了を待つ
 *
 * CControlProcess::Terminateとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存コードに該当する処理はありません。
 */
void CControlProcess_Terminate(std::wstring_view profileName)
{
	// トレイウインドウを検索する
	std::wstring strCEditAppName( GSTR_CEDITAPP );
	if (profileName.length() > 0) {
		strCEditAppName += profileName;
	}
	HWND hTrayWnd = ::FindWindow( strCEditAppName.data(), strCEditAppName.data() );
	if( !hTrayWnd ){
		// ウインドウがなければそのまま抜ける
		return;
	}

	// トレイウインドウからプロセスIDを取得する
	DWORD dwControlProcessId = 0;
	::GetWindowThreadProcessId( hTrayWnd, &dwControlProcessId );
	if( !dwControlProcessId ){
		throw std::runtime_error( "dwControlProcessId can't be retrived." );
	}

	// プロセス情報の問い合せを行うためのハンドルを開く
	HANDLE hControlProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwControlProcessId );
	if( !hControlProcess ){
		throw std::runtime_error( "hControlProcess can't be opened." );
	}

	// プロセスハンドルをスマートポインタに入れる
	handleHolder processHolder( hControlProcess );

	// トレイウインドウを閉じる
	::SendMessage( hTrayWnd, WM_CLOSE, 0, 0 );

	// プロセス終了を待つ
	DWORD dwExitCode = 0;
	if( ::GetExitCodeProcess( hControlProcess, &dwExitCode ) && dwExitCode == STILL_ACTIVE ){
		DWORD waitProcessResult = ::WaitForSingleObject( hControlProcess, INFINITE );
		if( WAIT_TIMEOUT == waitProcessResult ){
			throw std::runtime_error( "waitProcess is timeout." );
		}
	}
}

/*!
 * @brief コントロールプロセスを起動し、終了指示を出して、終了を待つ
 */
void CControlProcess_StartAndTerminate(std::wstring_view profileName)
{
	// コントロールプロセスを起動する
	CControlProcess_Start(profileName.data());

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(profileName.data());
}

/*!
 * @brief wWinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  コントロールプロセスを実行する。
 *  プロセス起動は2回行い、1回目でINI作成＆書き込み、2回目でINI読み取りを検証する。
 */
TEST_P(WinMainTest, runWithNoWin)
{
	// テスト用プロファイル名
	const auto szProfileName(GetParam());

	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	CControlProcess_StartAndTerminate(szProfileName);

	// コントロールプロセスが終了すると、INIファイルが作成される
	ASSERT_TRUE(fexist(iniPath.c_str()));

	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	CControlProcess_StartAndTerminate(szProfileName);

	// コントロールプロセスが終了すると、INIファイルが作成される
	ASSERT_TRUE(fexist(iniPath.c_str()));
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  エディタプロセスを実行する。
 */
TEST_P(WinMainTest, runEditorProcess)
{
	// テスト用プロファイル名
	const auto szProfileName(GetParam());

	// テスト用ファイル作成
	const std::wstring strFileName = std::filesystem::current_path() / L"test_1000lines.txt";
	std::wofstream fs(strFileName.c_str());
	for (int n = 1; n <= 1000; n++) {
		fs << n << std::endl;
	}
	fs.close();

	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc = [szProfileName, strFileName]() {
		// 起動時実行マクロの中身を作る
		std::wstring strStartupMacro;
		strStartupMacro += L"Down();";
		strStartupMacro += L"Up();";
		strStartupMacro += L"Right();";
		strStartupMacro += L"Left();";
		strStartupMacro += L"Outline(0);";		//アウトライン解析
		strStartupMacro += L"ShowFunckey();";	//ShowFunckey 出す
		strStartupMacro += L"ShowMiniMap();";	//ShowMiniMap 出す
		strStartupMacro += L"ShowTab();";		//ShowTab 出す
		strStartupMacro += L"SelectAll();";
		strStartupMacro += L"GoFileEnd();";
		strStartupMacro += L"GoFileTop();";
		strStartupMacro += L"ShowFunckey();";	//ShowFunckey 消す
		strStartupMacro += L"ShowMiniMap();";	//ShowMiniMap 消す
		strStartupMacro += L"ShowTab();";		//ShowTab 消す
		strStartupMacro += L"ExpandParameter('$I');";	// INIファイルパスの取得(呼ぶだけ)
		// フォントサイズ設定のテスト(ここから)
		strStartupMacro += L"SetFontSize(0, 1, 0);";	// 相対指定 - 拡大 - 対象：共通設定
		strStartupMacro += L"SetFontSize(0, -1, 0);";	// 相対指定 - 縮小 - 対象：共通設定
		strStartupMacro += L"SetFontSize(100, 0, 0);";	// 直接指定 - 対象：共通設定
		strStartupMacro += L"SetFontSize(100, 0, 1);";	// 直接指定 - 対象：タイプ別設定
		strStartupMacro += L"SetFontSize(100, 0, 2);";	// 直接指定 - 対象：一時適用
		strStartupMacro += L"SetFontSize(100, 0, 3);";	// 直接指定 - 対象が不正
		strStartupMacro += L"SetFontSize(0, 0, 0);";	// 直接指定 - フォントサイズ下限未満
		strStartupMacro += L"SetFontSize(9999, 0, 0);";	// 直接指定 - フォントサイズ上限超過
		strStartupMacro += L"SetFontSize(0, 0, 2);";	// 相対指定 - サイズ変化なし
		strStartupMacro += L"SetFontSize(0, 1, 2);";	// 相対指定 - 拡大
		strStartupMacro += L"SetFontSize(0, -1, 2);";	// 相対指定 - 縮小
		strStartupMacro += L"SetFontSize(0, 9999, 2);";	// 相対指定 - 限界まで拡大
		strStartupMacro += L"SetFontSize(0, 1, 2);";	// 相対指定 - これ以上拡大できない
		strStartupMacro += L"SetFontSize(0, -9999, 2);";// 相対指定 - 限界まで縮小
		strStartupMacro += L"SetFontSize(0, -1, 2);";	// 相対指定 - これ以上縮小できない
		strStartupMacro += L"SetFontSize(100, 0, 2);";	// 元に戻す
		// フォントサイズ設定のテスト(ここまで)
		strStartupMacro += L"Outline(2);";		//アウトライン解析を閉じる
		strStartupMacro += L"ExitAll();";		//NOTE: このコマンドにより、エディタプロセスは起動された直後に終了する。

		// コマンドラインを組み立てる
		std::wstring strCommandLine = strFileName;
		strCommandLine += strprintf(LR"( -PROF="%s")", szProfileName);
		strCommandLine += strprintf(LR"( -MTYPE=js -M="%s")", std::regex_replace( strStartupMacro, std::wregex( L"\"" ), L"\"\"" ).c_str());

		// エディタプロセスを起動する
		const int ret = StartEditorProcessForTest(strCommandLine);

		exit(ret);
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc(); }, ::testing::ExitedWithCode(0), ".*" );

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(szProfileName);

	// コントロールプロセスが終了すると、INIファイルが作成される
	ASSERT_TRUE( fexist( iniPath.c_str() ) );

	// テスト用ファイルの後始末
	std::filesystem::remove(strFileName);
}

/*!
 * @brief パラメータテストをインスタンス化する
 *  プロファイル指定なしとプロファイル指定ありの2パターンで実体化させる
 */
INSTANTIATE_TEST_CASE_P(ParameterizedTestWinMain
	, WinMainTest
	, ::testing::Values(L"", L"profile1")
);
