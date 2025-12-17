/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

#include <string_view>

/*!
 * テストコード専用wWinMain呼出のラッパー関数
 *
 * 単体テストから wWinMain を呼び出すためのラッパー関数です。
 *
 * wWinMain は呼出元のグローバル変数を汚してしまうため、
 * ASSERT_EXIT, ASSERT_DEATH などを使って別プロセスで実行するようにしてください。
 *
 * この関数をコントロールプロセスの起動に使用しないでください。
 * googletestでは、ASSERT_EXITで起動したプロセスの完全な終了を待機できないようです。
 * コントロールプロセスが終了する前に他のテストが実行されると期待した動作にならない場合があります。
 */
int StartEditorProcessForTest(std::wstring_view commandLine);
