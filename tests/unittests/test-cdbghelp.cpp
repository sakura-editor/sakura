/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

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

#include <eh.h>

#include "_main/global.h"
#include "extmodule/CDbgHelp.h"

#include "util/file.h"

/*!
	クラッシュダンプのテスト
 */
TEST(CDbgHelp, test)
{
	const auto dumpFile = GetIniFileName().replace_filename(L"sakura.dmp");

	CDbgHelp cDbgHelp;
	cDbgHelp.InitDll();
	ASSERT_TRUE(cDbgHelp.IsAvailable());

	try
	{
		// 何もしないセッションを実行させる
		cDbgHelp.DbgSession([]() {});

		// 例外を発生させる
		cDbgHelp.DbgSession([]() {
			throw L"何か問題が起きたぜ！";
		});

		// 例外をスローできていたらここには来ない
		FAIL();
	}
	// tryブロックで発生したあらゆる例外をキャッチして握りつぶす　←アプリコードでこれをやってはいけない
	catch (...) {
		// ダンプファイルが作成されているはず。
		EXPECT_TRUE(fexist(dumpFile.c_str()));

		// ダンプファイルを削除する
		std::filesystem::remove(dumpFile);
	}
}
