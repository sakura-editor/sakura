/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "env/CShareData.h"

/*!
 * @brief CShareData初期化と言語切替のテスト
 */
TEST(CShareData, initShareData001)
{
	// 共有メモリのインスタンスを生成する
	const auto pcShareData = std::make_unique<CShareData>();
	ASSERT_NE(nullptr, pcShareData);

	// 共有メモリを初期化する
	ASSERT_TRUE(pcShareData->InitShareData());

	// 言語切り替えのテストを実施する
	std::vector<std::wstring> values;
	pcShareData->ConvertLangValues(values, true);
	CSelectLang::ChangeLang(L"sakura_lang_en_US.dll");
	pcShareData->ConvertLangValues(values, false);
	pcShareData->RefreshString();

	// 言語を元に戻す
	CSelectLang::ChangeLang(L"");
}
