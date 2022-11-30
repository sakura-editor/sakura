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
		{ nullptr, 0 }
	};
	return RegisterEntries(table);
}

