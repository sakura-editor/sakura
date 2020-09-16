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

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <regex>
#include <string>

#include "debug/Debug2.h"
#include "StartEditorProcessForTest.h"

/*!
 * テストコード専用wWinMain呼出のラッパー関数
 *
 * 単体テストから wWinMain を呼び出すためのラッパー関数です。
 *
 * コマンドラインでプロファイルが指定されていない場合、空指定を付加します。
 */
int StartEditorProcessForTest( const std::wstring_view& strCommandLine )
{

	// 実行中モジュールのインスタンスハンドルを取得する
	HINSTANCE hInstance = ::GetModuleHandle( NULL );

	// WinMainを起動するためのコマンドラインを組み立てる
	std::wstring strCmdBuff( strCommandLine );

	// コマンドラインに -PROF 指定がない場合は付加する
	if( !std::regex_search( strCmdBuff, std::wregex( L"-PROF\\b", std::wregex::icase ) ) ){
		strCmdBuff += L" -PROF=\"\"";
	}

	// wWinMainを起動する
	return wWinMain( hInstance, NULL, &*strCmdBuff.begin(), SW_SHOWDEFAULT );
}

/*!
 * 必要な場合にwWinMainを起動して終了する。
 *
 * コマンドラインに -PROF 指定がない場合、呼出元に制御を返す。
 * コマンドラインに -PROF 指定がある場合、wWinMainを呼出してプログラムを終了する。
 */
static void InvokeWinMainIfNeeded( char** ppArgsBegin, char** ppArgsEnd )
{
	// コマンドライン引数がない場合
	if( ppArgsBegin == ppArgsEnd ){
		return;
	}

	// コマンドラインに -PROF 指定がない場合
	if( ppArgsEnd == std::find_if( ppArgsBegin, ppArgsEnd, []( const char* arg ){ return std::regex_search( arg, std::regex( "-PROF\\b", std::regex::icase ) ); } ) ){
		return;
	}

	// 最初の引数はプログラム名なので無視する
	ppArgsBegin++;

	// wWinMainを起動するためのコマンドラインを組み立てる(バッファ長はざっくり定義。)
	wchar_t szCmdBuf[4096];
	std::wstring strCommandLine;
	std::for_each( ppArgsBegin, ppArgsEnd, [&strCommandLine, &szCmdBuf]( const auto* arg ){
		::swprintf_s( szCmdBuf, L"%hs ", arg );
		strCommandLine += szCmdBuf;
	} );

	// 末尾の空白を削る(引数0個はここに来ないのでチェックしない)
	strCommandLine.assign( strCommandLine.data(), strCommandLine.length() - 1 );

	// 実行中モジュールのインスタンスハンドルを取得する
	HINSTANCE hInstance = ::GetModuleHandleW( NULL );

	// ログ出力
	WCHAR *pszCommandLine = &*strCommandLine.begin();
	printf( "%s(%d): launching process [%ls]\n", __FILE__, __LINE__, pszCommandLine );

	// wWinMainを起動する
	int ret = wWinMain( hInstance, NULL, pszCommandLine, SW_SHOWDEFAULT );

	// ログ出力(途中でexitした場合は出力されない)
	printf( "%s(%d): leaving process   [%ls] => %d\n", __FILE__, __LINE__, pszCommandLine, ret );

	// プログラムを終了する(呼出元に制御は返らない)
	exit( ret );
}

/*!
 * テストモジュールのエントリポイント
 */
int main(int argc, char **argv) {
	// コマンドラインに -PROF 指定がある場合、wWinMainを起動して終了する。
	InvokeWinMainIfNeeded( argv, argv + argc );

	// WinMainを起動しない場合、標準のgtest_main同様の処理を実行する
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
