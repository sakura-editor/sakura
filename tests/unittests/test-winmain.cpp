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

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  エディタプロセスを実行する。
 */
TEST( WinMainTest, runEditorProcess )
{
	// エディタプロセスを起動する
	ASSERT_EQ( 0, wWinMain( NULL, NULL, _T(__FILE__)  L" -MTYPE=js -M=\"ExitAll();\" -PROF=", SW_SHOWDEFAULT ) );
}
