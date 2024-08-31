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
#include "pch.h"

#include "env/CShareData.h"

#include "_main/CControlProcess.h"

/*!
 * @brief CShareDataのテスト
 */
TEST( CShareData, test )
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	// 共有メモリのインスタンスを取得する
	auto pShareData = CShareData::getInstance();
	ASSERT_NE(nullptr, pShareData);

	// 共有メモリのインスタンスを初期化する
	ASSERT_TRUE(pShareData->InitShareData());

	ASSERT_TRUE(pShareData->IsPrivateSettings());

	// 言語切り替えのテストを実施する
	std::vector<std::wstring> values;
	pShareData->ConvertLangValues(values, true);
	CSelectLang::ChangeLang(L"sakura_lang_en_US.dll");
	pShareData->ConvertLangValues(values, false);
	pShareData->RefreshString();
}
