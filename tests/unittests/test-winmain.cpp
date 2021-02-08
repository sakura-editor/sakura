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

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <regex>
#include <string>
#include <thread>

#include "config/maxdata.h"
#include "basis/primitive.h"
#include "debug/Debug2.h"
#include "basis/CMyString.h"
#include "mem/CNativeW.h"
#include "env/DLLSHAREDATA.h"
#include "util/file.h"
#include "config/system_constants.h"

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
typedef std::unique_ptr<std::remove_pointer<HANDLE>::type, handle_closer> handleHolder;

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
class WinMainTest : public ::testing::Test {
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
		// INIファイルのパスを取得
		iniPath = GetIniFileName();

		if( fexist( iniPath.c_str() ) ){
			// INIファイルを削除する
			std::filesystem::remove( iniPath );
		}
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// INIファイルを削除する
		std::filesystem::remove( iniPath );
	}
};

/*!
 * @brief コントロールプロセスの初期化完了を待つ
 *
 * CControlProcess::WaitForInitializedとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存CProcessFactory::WaitForInitializedControlProcess()と概ね等価です。
 */
void CControlProcess_WaitForInitialized( LPCWSTR lpszProfileName )
{
	// 初期化完了イベントを作成する
	std::wstring strInitEvent( GSTR_EVENT_SAKURA_CP_INITIALIZED );
	if( lpszProfileName && lpszProfileName[0] ){
		strInitEvent += lpszProfileName;
	}
	auto hEvent = ::CreateEventW( NULL, TRUE, FALSE, strInitEvent.data() );
	if (!hEvent) {
		throw std::runtime_error( "create event failed." );
	}

	// イベントハンドラをスマートポインタに入れる
	handleHolder eventHolder( hEvent );

	// 初期化完了イベントを待つ
	DWORD dwRet = ::WaitForSingleObject( hEvent, 10000 );
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
void CControlProcess_Start( LPCWSTR lpszProfileName )
{
	// スタートアップ情報
	STARTUPINFO si = { sizeof(STARTUPINFO), 0 };
	si.lpTitle = (LPWSTR)L"sakura control process";
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWDEFAULT;

	WCHAR szExePath[MAX_PATH];
	::GetModuleFileNameW( NULL, szExePath, _countof(szExePath) );

	CNativeW cmemCommandLine;
	cmemCommandLine.AppendStringF( L"\"%s\" -NOWIN -PROF=\"%s\"", szExePath, lpszProfileName );

	LPWSTR pszCommandLine = cmemCommandLine.GetStringPtr();
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
	PROCESS_INFORMATION pi;

	// コントロールプロセスを起動する
	BOOL createSuccess = ::CreateProcess(
		szExePath,			// 実行可能モジュールパス
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
	CControlProcess_WaitForInitialized( lpszProfileName );
}

/*!
 * @brief コントロールプロセスに終了指示を出して終了を待つ
 *
 * CControlProcess::Terminateとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存コードに該当する処理はありません。
 */
void CControlProcess_Terminate( LPCWSTR lpszProfileName )
{
	// トレイウインドウを検索する
	std::wstring strCEditAppName( GSTR_CEDITAPP );
	if( lpszProfileName && lpszProfileName[0] ){
		strCEditAppName += lpszProfileName;
	}
	HWND hTrayWnd = ::FindWindow( strCEditAppName.data(), strCEditAppName.data() );
	if( !hTrayWnd ){
		throw std::runtime_error( "tray window is not found." );
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
 * @brief wWinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  コントロールプロセスを実行する。
 */
TEST_F( WinMainTest, runWithNoWin )
{
	// テスト用プロファイル名
	constexpr auto szProfileName = L"";

	// コントロールプロセスを起動する
	CControlProcess_Start( szProfileName );

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate( szProfileName );

	// コントロールプロセスが終了すると、INIファイルが作成される
	ASSERT_TRUE( fexist( iniPath.c_str() ) );
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  エディタプロセスを実行する。
 */
TEST_F( WinMainTest, runEditorProcess )
{
	// エディタプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc = [] {
		std::mutex mtx;
		std::condition_variable cv;
		bool initialized = false;

		// エディタプロセスが起動したコントロールプロセスの終了を待機するスレッド
		auto waitingThread = std::thread([&mtx, &cv, &initialized] {
			// 初期化
			{
				std::unique_lock<std::mutex> lock( mtx );
				initialized = true;
				cv.notify_one();
			}

			// テスト用プロファイル名
			constexpr auto szProfileName = L"";

			// コントロールプロセスの初期化完了を待つ
			CControlProcess_WaitForInitialized( szProfileName );

			// 起動時実行マクロが全部実行し終わるのを待つ
			::Sleep( 10000 );

			// コントロールプロセスに終了指示を出して終了を待つ
			CControlProcess_Terminate( szProfileName );
		});

		// スレッドの初期化完了を待機する
		std::unique_lock<std::mutex> lock( mtx );
		cv.wait(lock, [&initialized] { return initialized; });

		// 起動時実行マクロの中身を作る
		std::wstring strStartupMacro;
		strStartupMacro += L"Down();";
		strStartupMacro += L"Up();";
		strStartupMacro += L"Right();";
		strStartupMacro += L"Left();";
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

		// コマンドラインを組み立てる
		std::wstring strCommandLine( _T(__FILE__)  L" -MTYPE=js" );
		strCommandLine += L" -M=\""s;
		strCommandLine += std::regex_replace( strStartupMacro, std::wregex( L"\"" ), L"\"\"" );
		strCommandLine += L"\""s;

		// エディタプロセスを起動する
		StartEditorProcessForTest( strCommandLine );

		// エディタ終了を待機する
		if( waitingThread.joinable() ){
			waitingThread.join();
		}
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc(); exit(0); }, ::testing::ExitedWithCode(0), ".*" );

	// コントロールプロセスが終了すると、INIファイルが作成される
	ASSERT_TRUE( fexist( iniPath.c_str() ) );
}
