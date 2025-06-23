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

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>

#include "basis/primitive.h"
#include "util/string_ex.h"
#include "StartEditorProcessForTest.h"

/*!
 * テストコード専用wWinMain呼出のラッパー関数
 *
 * 単体テストから wWinMain を呼び出すためのラッパー関数です。
 * コマンドラインには -PROF 指定が含まれている必要があります。
 */
int StartEditorProcessForTest(std::wstring_view commandLine)
{
	//戻り値は0が正常。適当なエラー値を指定しておく。
	int ret = -1;

	// コマンドラインに -PROF 指定がない場合は異常終了させる
	if (std::regex_search(commandLine.data(), std::wregex(LR"(-PROF\b)", std::wregex::icase))) {
		// 実行中モジュールのインスタンスハンドルを取得する
		HINSTANCE hInstance = ::GetModuleHandleW(nullptr);

		// WinMainに渡すためのコマンドライン
		std::wstring strCommandLine(commandLine);

		// ログ出力
		std::wcout << strprintf(L"%hs(%d): launching process [%s]", __FILE__, __LINE__, commandLine.data()) << std::endl;

		// wWinMainを起動する
		ret = wWinMain(hInstance, nullptr, strCommandLine.data(), SW_SHOWDEFAULT);

		// ログ出力(途中でexitした場合は出力されない)
		std::wcout << strprintf(L"%hs(%d): leaving process   [%s] => %d\n", __FILE__, __LINE__, commandLine.data(), ret) << std::endl;
	}

	return ret;
}

/*!
 * 必要な場合にwWinMainを起動して終了する。
 *
 * コマンドラインに -PROF 指定がない場合、呼出元に制御を返す。
 * コマンドラインに -PROF 指定がある場合、wWinMainを呼出してプログラムを終了する。
 */
static void InvokeWinMainIfNeeded(std::wstring_view commandLine)
{
	// コマンドライン引数がない場合
	if (commandLine.empty()) {
		return;
	}

	// コマンドラインに -PROF 指定がない場合
	if (!std::regex_search(commandLine.data(), std::wregex(LR"(-PROF\b)", std::wregex::icase))) {
		return;
	}

	// wWinMainを起動する
	const int ret = StartEditorProcessForTest(commandLine);

	// プログラムを終了する(呼出元に制御は返らない)
	exit(ret);
}

/*!
 * テストモジュールのエントリポイント
 */
int wmain(int argc, wchar_t **argv) {
	// コマンドラインに -PROF 指定がある場合、wWinMainを起動して終了する。
	InvokeWinMainIfNeeded(::GetCommandLineW());

	// WinMainを起動しない場合、標準のgmock_main同様の処理を実行する。
	// InitGoogleMock は Google Test の初期化も行うため、InitGoogleTest を別に呼ぶ必要はない。
	wprintf(L"Running main() from %hs\n", __FILE__);
	testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
