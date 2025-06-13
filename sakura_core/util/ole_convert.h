/*!	@file
	@brief OLE型（VARIANT, BSTRなど）の変換関数

*/
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_OLE_CONVERT_4D549FAC_BEAA_4E5F_ACC1_9EEEA9B8DC6F_H_
#define SAKURA_OLE_CONVERT_4D549FAC_BEAA_4E5F_ACC1_9EEEA9B8DC6F_H_
#pragma once

#include <string>
#include "_os/OleTypes.h"

bool variant_to_wstr( VARIANT v, std::wstring& wstr );	// VARIANT変数をBSTRとみなし、wstringに変換する
bool variant_to_int( VARIANT v, int& n );	// VARIANT変数を整数とみなし、intに変換する
#endif /* SAKURA_OLE_CONVERT_4D549FAC_BEAA_4E5F_ACC1_9EEEA9B8DC6F_H_ */
