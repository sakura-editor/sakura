/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>
#include <windowsx.h>
#include <Shlwapi.h>

#include <memory>

#include "env/CShareData.h"

#include "_main/CCommandLine.h"
#include "_main/CNormalProcess.h"

/*!
 * @brief CShareDataのテスト
 */
TEST( CShareData, test )
{
	// 共有メモリをインスタンス化するにはプロセスのインスタンスが必要。
	CNormalProcess cProcess(::GetModuleHandle(nullptr), L"");

	// 共有メモリのインスタンスを取得する
	auto pShareData = CShareData::getInstance();
	ASSERT_NE(nullptr, pShareData);

	// 共有メモリを初期化するにはコマンドラインのインスタンスが必要
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);

	// 共有メモリのインスタンスを初期化する
	ASSERT_TRUE(pShareData->InitShareData());

	// 言語切り替えのテストを実施する
	std::vector<std::wstring> values;
	pShareData->ConvertLangValues(values, true);
	CSelectLang::ChangeLang(L"sakura_lang_en_US.dll");
	pShareData->ConvertLangValues(values, false);
	pShareData->RefreshString();
}
