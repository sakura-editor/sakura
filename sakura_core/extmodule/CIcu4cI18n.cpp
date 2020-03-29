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
#include "StdAfx.h"
#include "CIcu4cI18n.h"

CIcu4cI18n::CIcu4cI18n() noexcept
	: _ucsdet_open(nullptr)
	, _ucsdet_setText(nullptr)
	, _ucsdet_detect(nullptr)
	, _ucsdet_close(nullptr)
{}

CIcu4cI18n::~CIcu4cI18n() noexcept {}

/*!
 * @brief DLLの名前を返す
 */
LPCWSTR CIcu4cI18n::GetDllNameImp([[maybe_unused]] int index)
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
	// DLL内関数名リスト
	const ImportTable table[] = {{&_ucsdet_open, "ucsdet_open_66"},		  //バージョンは固定
								 {&_ucsdet_setText, "ucsdet_setText_66"}, //バージョンは固定
								 {&_ucsdet_detect, "ucsdet_detect_66"},	  //バージョンは固定
								 {&_ucsdet_getName, "ucsdet_getName_66"}, //バージョンは固定
								 {&_ucsdet_close, "ucsdet_close_66"},	  //バージョンは固定
								 {NULL, 0}};
	return RegisterEntries(table);
}
