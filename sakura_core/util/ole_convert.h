/*!	@file
	@brief OLE型（VARIANT, BSTRなど）の変換関数

*/
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_OLE_CONVERT_4D549FAC_BEAA_4E5F_ACC1_9EEEA9B8DC6F_H_
#define SAKURA_OLE_CONVERT_4D549FAC_BEAA_4E5F_ACC1_9EEEA9B8DC6F_H_
#pragma once

#include <string>
#include "_os/OleTypes.h"

bool variant_to_wstr( VARIANT v, std::wstring& wstr );	// VARIANT変数をBSTRとみなし、wstringに変換する
bool variant_to_int( VARIANT v, int& n );	// VARIANT変数を整数とみなし、intに変換する
#endif /* SAKURA_OLE_CONVERT_4D549FAC_BEAA_4E5F_ACC1_9EEEA9B8DC6F_H_ */
