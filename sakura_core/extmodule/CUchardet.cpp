/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CUchardet.h"

/*!
 * @brief DLLの名前を返す
 */
LPCWSTR CUchardet::GetDllNameImp( [[maybe_unused]] int index )
{
	return L"uchardet.dll";
}

/*!
	DLLの初期化

	関数のアドレスを取得してメンバに保管する．

	@retval true 成功
	@retval false アドレス取得に失敗
*/
bool CUchardet::InitDllImp()
{
	// DLL内関数名リスト
	const ImportTable table[] = {
		{ &_uchardet_new,			"uchardet_new" },
		{ &_uchardet_delete,		"uchardet_delete" },
		{ &_uchardet_handle_data,	"uchardet_handle_data" },
		{ &_uchardet_data_end,		"uchardet_data_end" },
		{ &_uchardet_reset,			"uchardet_reset" },
		{ &_uchardet_get_charset,	"uchardet_get_charset" },
		{ nullptr, nullptr }
	};
	return RegisterEntries(table);
}

