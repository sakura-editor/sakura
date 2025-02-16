/*! @file */
// 2007.10.20 kobake 書式関連
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_FORMAT_A006AC9B_ADE2_499D_9CC6_00A649F32B4F_H_
#define SAKURA_FORMAT_A006AC9B_ADE2_499D_9CC6_00A649F32B4F_H_
#pragma once

#include <string>
#include <string_view>

// 20051121 aroka
std::wstring GetDateTimeFormat( std::wstring_view format, const SYSTEMTIME& systime );
UINT32 ParseVersion( const WCHAR* ver );	//バージョン番号の解析
int CompareVersion( const WCHAR* verA, const WCHAR* verB );	//バージョン番号の比較
#endif /* SAKURA_FORMAT_A006AC9B_ADE2_499D_9CC6_00A649F32B4F_H_ */
