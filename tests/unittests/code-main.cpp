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

	// テスト実行時のロケールは日本語に固定する
	const LCID lcid = 0x0411;
	SetThreadUILanguage(lcid);	// スレッドのUI言語を変更

	// コマンドラインに -PROF 指定がある場合、wWinMainを起動して終了する。
	InvokeWinMainIfNeeded(::GetCommandLineW());

	// LCIDからロケール名を取得（"ja-JP"が取れる）
	SString<LOCALE_NAME_MAX_LENGTH> szLocaleName;
	LCIDToLocaleName(lcid, szLocaleName, int(std::size(szLocaleName)), NULL);

	// TODO: 以下をコメントインする
	// szLocaleName += L".UTF-8";	// UCRTのutf8サポートを有効にする

	// Cロケールも変更
	_wsetlocale(LC_ALL, szLocaleName);

	// WinMainを起動しない場合、標準のgmock_main同様の処理を実行する。
	// InitGoogleMock は Google Test の初期化も行うため、InitGoogleTest を別に呼ぶ必要はない。
	wprintf(L"Running main() from %hs\n", __FILE__);
	testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
