/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CIcu4cI18n.h"

CIcu4cI18n::CIcu4cI18n() noexcept
	: _ucsdet_open(nullptr)
	, _ucsdet_setText(nullptr)
	, _ucsdet_detect(nullptr)
	, _ucsdet_close(nullptr)
{
}

/*!
 * @brief DLLの名前を返す
 */
LPCWSTR CIcu4cI18n::GetDllNameImp( [[maybe_unused]] int index )
{
	return L"icuin66.dll"; //バージョンは固定
}

/*!
	DLLの初期化

	関数のアドレスを取得してメンバに保管する．

	@retval true 成功
	@retval false アドレス取得に失敗
*/
bool CIcu4cI18n::InitDllImp()
{
	//DLL内関数名リスト
	const ImportTable table[] = {
		{ &_ucsdet_open,		"ucsdet_open_66" },		//バージョンは固定
		{ &_ucsdet_setText,		"ucsdet_setText_66" },	//バージョンは固定
		{ &_ucsdet_detect,		"ucsdet_detect_66" },	//バージョンは固定
		{ &_ucsdet_getName,		"ucsdet_getName_66" },	//バージョンは固定
		{ &_ucsdet_close,		"ucsdet_close_66" },	//バージョンは固定
		{ nullptr, nullptr }
	};
	return RegisterEntries(table);
}
