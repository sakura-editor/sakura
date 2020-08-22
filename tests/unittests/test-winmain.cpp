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

#include "CallWinMain.h"

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
	 * CFileNameManager::GetIniFileNameDirectを使ってtests1.iniのパスを取得する。
	 */
	WCHAR szIniFile[_MAX_PATH];

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	virtual void SetUp() {
		// INIファイルのパスを取得
		WCHAR szPrivateIniFile[_MAX_PATH];
		CFileNameManager::GetIniFileNameDirect( szPrivateIniFile, szIniFile, L"" );

		if( fexist( szIniFile ) ){
			// プロファイル設定を削除する
			std::filesystem::remove( szIniFile );
		}
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	virtual void TearDown() {
		// INIファイルを削除する
		std::filesystem::remove( szIniFile );
	}
};

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  INIファイルを出力して即終了するオプション -WQ を指定して実行、INIが出力されることを確認する。
 *  (ver2.0.0以前に混入した不具合により、ver2系では利用できないオプションなので無効化しています。）
 */
TEST_F( WinMainTest, DISABLED_runWithWriteQuit )
{
	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ CallWinMain( L"-WQ" ); exit(0); }, ::testing::ExitedWithCode(0), ".*" );
	ASSERT_TRUE( fexist( szIniFile ) );
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  コントロールプロセスを起動するオプション -NOWIN を指定して実行する。
 */
TEST_F( WinMainTest, runWithNoWin )
{
	// コントロールプロセスを起動するため、テスト実行はプロセスごと分離して行う
	auto separatedTestProc = [] {
		std::mutex mtx;
		std::condition_variable cv;
		bool initialized = false;

		// 起動したコントロールプロセスを終了させるスレッド
		auto waitingThread = std::thread([&mtx, &cv, &initialized] {
			// 初期化
			{
				std::unique_lock<std::mutex> lock( mtx );
				initialized = true;
				cv.notify_one();
			}

			// 初期化完了イベントを作成する
			std::wstring_view strInitEvent( GSTR_EVENT_SAKURA_CP_INITIALIZED );
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

			// トレイウインドウを検索する
			std::wstring_view strCEditAppName( GSTR_CEDITAPP );
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
		});
		std::unique_lock<std::mutex> lock( mtx );
		cv.wait(lock, [&initialized] { return initialized; });

		// スタートアップ情報
		STARTUPINFO si = { sizeof(STARTUPINFO), 0 };
		si.lpTitle = (LPWSTR)L"sakura control process";
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOWDEFAULT;

		WCHAR szExePath[MAX_PATH];
		::GetModuleFileNameW( NULL, szExePath, _countof(szExePath) );

		CNativeW cmemCommandLine;
		cmemCommandLine.AppendStringF( L"\"%s\" -NOWIN -PROF=\"\"", szExePath );

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

		// スレッド終了を待機する
		if( waitingThread.joinable() ){
			waitingThread.join();
		}
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc(); exit(0); }, ::testing::ExitedWithCode(0), ".*" );
	ASSERT_TRUE( fexist( szIniFile ) );
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

		// 起動したエディタプロセスの終了を待機するスレッド
		auto waitingThread = std::thread([&mtx, &cv, &initialized] {
			// 初期化
			{
				std::unique_lock<std::mutex> lock( mtx );
				initialized = true;
				cv.notify_one();
			}

			// 初期化完了イベントを作成する
			std::wstring_view strInitEvent( GSTR_EVENT_SAKURA_CP_INITIALIZED );
			auto hEvent = ::CreateEventW( NULL, TRUE, FALSE, strInitEvent.data() );
			if (!hEvent) {
				throw std::runtime_error( "create event failed." );
			}

			// イベントハンドラをスマートポインタに入れる
			handleHolder eventHolder( hEvent );

			// 初期化完了イベントを待つ
			DWORD waitEventResult = ::WaitForSingleObject( hEvent, 10000 );
			if( WAIT_TIMEOUT == waitEventResult ){
				throw std::runtime_error( "waitEvent is timeout." );
			}

			// トレイウインドウを検索する
			std::wstring_view strCEditAppName( GSTR_CEDITAPP );
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

			// プロセス終了を待つ
			DWORD dwExitCode = 0;
			if( ::GetExitCodeProcess( hControlProcess, &dwExitCode ) && dwExitCode == STILL_ACTIVE ){
				DWORD waitProcessResult = ::WaitForSingleObject( hControlProcess, INFINITE );
				if( WAIT_TIMEOUT == waitProcessResult ){
					throw std::runtime_error( "waitProcess is timeout." );
				}
			}
		});
		// スレッドの初期化完了を待機する
		std::unique_lock<std::mutex> lock( mtx );
		cv.wait(lock, [&initialized] { return initialized; });

		// 起動時実行マクロの中身を作る
		std::wstring strStartupMacro;
		strStartupMacro += L"WinCloseAll();";

		// コマンドラインを組み立てる
		std::wstring strCommandLine( _T(__FILE__)  L" -MTYPE=js" );
		strCommandLine += L" -M=\""s;
		strCommandLine += std::regex_replace( strStartupMacro, std::wregex( L"\"" ), L"\"\"" );
		strCommandLine += L"\""s;

		// エディタプロセスを起動する
		CallWinMain( strCommandLine );

		// エディタ終了を待機する
		if( waitingThread.joinable() ){
			waitingThread.join();
		}
	};

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ separatedTestProc(); exit(0); }, ::testing::ExitedWithCode(0), ".*" );
	ASSERT_TRUE( fexist( szIniFile ) );
}
