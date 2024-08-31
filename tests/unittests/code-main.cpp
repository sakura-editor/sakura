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

#include "StartEditorProcessForTest.h"

#include <spdlog/sinks/stdout_color_sinks.h>

/*!
 * テストコード専用wWinMain呼出のラッパー関数
 *
 * 単体テストから wWinMain を呼び出すためのラッパー関数です。
 * コマンドラインには -PROF 指定が含まれている必要があります。

 * @param[in] commandLine コマンドライン文字列
 * @retval 0     正常終了
 * @retval 0以外 異常終了
 */
int StartEditorProcessForTest(std::wstring_view commandLine)
{
	// ログ出力
	std::wcout << fmt::format(L"launching process [{}]", commandLine) << std::endl;

	// wWinMainに渡すためのコマンドライン
	std::wstring commandLineBuf(commandLine);

	// コマンドラインに -CODE 指定がない場合は付与する
	if (!std::regex_search(commandLineBuf, std::wregex(LR"(-CODE\b)", std::wregex::icase)) &&
		!std::regex_search(commandLineBuf, std::wregex(LR"(-NOWIN\b)", std::wregex::icase)))
	{
		commandLineBuf += L" -CODE=99"; // 指定しないとファイル名から文字コードを判定する仕様によりJIS指定になってしまう。
	}

	// 実行中モジュールのインスタンスハンドルを取得する
	const auto hInstance = GetModuleHandleW(nullptr);

	// wWinMainを起動する(戻り値は0が正常)
	const auto ret = wWinMain(hInstance, nullptr, commandLineBuf.data(), SW_SHOWDEFAULT);

	// ログ出力(例外でexitした場合は出力されない)
	std::wcout << fmt::format(L"leaving process   [{}] => {}\n", commandLine, ret) << std::endl;

	return ret;
}

/*!
 * テストモジュールのエントリポイント
 */
int main(int argc, char **argv) {
	// コマンドラインに -PROF 指定がある場合、wWinMainを起動して終了する。
	if (const auto commandLine = std::wstring_view(GetCommandLineW());
		std::regex_search(commandLine.data(), std::wregex(LR"(-PROF\b)", std::wregex::icase))) {
		return StartEditorProcessForTest(commandLine);
	}

	// WinMainを起動しない場合、標準のgmock_main同様の処理を実行する。
	// InitGoogleMock は Google Test の初期化も行うため、InitGoogleTest を別に呼ぶ必要はない。
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
