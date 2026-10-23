/*!	@file
	@brief Entry Point

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date	2001/06/26 genta ワード単位のGrepのためのコマンドライン処理追加
	@date	2002/01/08 aroka 処理の流れを整理、未使用コードを削除
	@date	2002/01/18 aroka 虫取り＆リリース
	@date	2009/01/07 ryoji WinMainにOleInitialize/OleUninitializeを追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, aroka
	Copyright (C) 2007, kobake
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CProcessFactory.h"

#include "basis/initialization_error.hpp"
#include "util/os.h"
#include "util/MessageBoxF.h"
#include "util/module.h"
#include "debug/CRunningTimer.h"
#include "version.h"
#include "util/std_macro.h"
#include "env/DLLSHAREDATA.h"
#include "apiwrap/DarkMode.h"

#include <clocale>

#include <ole2.h>

#pragma comment(lib, "ole32.lib") 

/*!
	Windows Entry point

	コマンドラインオプションで -NOWIN を指定するかどうかでプロセスのタイプが変わる。
		+----------+---------------------------+---------------------------+
		|-NOWIN指定|どうなる？                 |作成するProcessインスタンス|
		+----------+---------------------------+---------------------------+
		|有        |コントロールプロセスとなる |CControlProcessクラス      |
		+----------+---------------------------+---------------------------+
		|無        |エディタプロセスとなる     |CNormalProcessクラス       |
		+----------+---------------------------+---------------------------+
 *
 * @date 2007/08/16 kobake setlocale 追加。ロケールを日本語に固定。
 * @date 2009/01/07 ryoji OleInitialize/OleUninitialize 追加
 * @date 2009/09/10 syat メモリリークチェックを追加
 * @date 2014/04/24 syat DLLの検索パスからカレントディレクトリを削除する
 */
int WINAPI wWinMain(
	_In_ HINSTANCE		hInstance,		//!< handle to current instance
	_In_opt_ HINSTANCE	hPrevInstance [[maybe_unused]],	//!< handle to previous instance
	_In_ LPWSTR			lpCmdLine,		//!< pointer to command line
	_In_ int			nCmdShow		//!< show state of window
)
{
#ifdef USE_LEAK_CHECK_WITH_CRTDBG
	// 2009.9.10 syat メモリリークチェックを追加
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	MY_RUNNINGTIMER(cRunningTimer, L"WinMain" );
	{
		// 2014.04.24 DLLの検索パスからカレントディレクトリを削除する
		::SetDllDirectory( L"" );
		::SetSearchPathMode( BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT );
	}

	// Cランタイムのロケールを日本語に設定する
	::setlocale(LC_ALL, "Japanese-Japan.932");
	
	// COMライブラリを初期化する(失敗したら即終了する)
	if (FAILED(::OleInitialize(nullptr))) return static_cast<int>(EAppExitCode::AbEnd);	// 失敗したら即終了する

	//開発情報
	DEBUG_TRACE(L"-- -- WinMain -- --\n");
	DEBUG_TRACE(L"sizeof(DLLSHAREDATA) = %d\n",sizeof(DLLSHAREDATA));

	DarkMode::initDarkMode();
	DarkMode::setDarkModeConfig();
	DarkMode::setDefaultColors(true);

	// 終了コード
	EAppExitCode exitCode;

	try {
		//コマンドラインクラスのインスタンスを確保する
		CCommandLine cCommandLine;

		//プロセスの生成とメッセージループ
		const auto process = CProcessFactory(hInstance).CreateInstance(lpCmdLine);

		// プロセスの生成に失敗したら即終了する
		if (!process) return static_cast<int>(EAppExitCode::AbEnd);	// 失敗したら即終了する

		MY_TRACETIME( cRunningTimer, L"ProcessObject Created" );

		// プロセス実行(初期化してメッセージループを実行する)
		exitCode = process->Run(
			nCmdShow
		);
	}
	// アプリの初期化に失敗した場合
	catch (const basis::initialization_error& e) {
		// 例外で指定されたメッセージを表示
		ErrorBeep();
		TopErrorMessage(nullptr, e.message().c_str());
		// 例外で指定された終了コードで終了
		exitCode = e.exitCode().value_or(EAppExitCode::AppInitFailed);
	}
	// メモリ不足エラーが発生した場合
	catch (const std::bad_alloc&) {
		// L"メモリ不足です。\n終了します。"
		ErrorBeep();
		TopErrorMessage(nullptr, LS(IDS_BAD_ALLOC));
		exitCode = EAppExitCode::BadAlloc;
	}
	// その他エラーが発生した場合
	catch (const std::exception&) {
		// L"予期せぬエラーが発生しました。\n終了します。"
		ErrorBeep();
		TopErrorMessage(nullptr, LS(IDS_UNHANDLED));
		exitCode = EAppExitCode::UnExpected;
	}

	::OleUninitialize();	// 2009.01.07 ryoji 追加

	return static_cast<int>(exitCode);
}
